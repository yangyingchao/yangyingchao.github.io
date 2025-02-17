# polardb pg HTAP架构详解


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [HTAP 架构原理](#h:036ce9a9-077e-48de-bb55-5234be6c2087)
- <span class="section-num">2</span> [分布式优化器](#h:5fb147d2-245c-4846-a70f-120912f6df84)
- <span class="section-num">3</span> [算子并行化](#h:bd455b10-a8d7-45a2-a36b-ce3ffa9fb862)
- <span class="section-num">4</span> [消除数据倾斜问题](#h:9800261a-d0c9-4291-b68d-70585cab4086)
- <span class="section-num">5</span> [SQL 级别弹性扩展](#h:ce8787fe-c246-42e2-90b2-f7f85da2ec59)
- <span class="section-num">6</span> [事务一致性](#h:00c5781c-9f42-42bf-b6d3-5cd0f06d7274)
- <span class="section-num">7</span> [TPC-H 性能：加速比](#h:50f1201d-8be4-44f6-a94a-813611fe0e09)
- <span class="section-num">8</span> [TPC-H 性能：和传统 MPP 数仓对比](#h:f71c9216-f9a9-439c-ba38-301fa36ce12e)
- <span class="section-num">9</span> [分布式执行加速索引创建](#h:a08fa54c-934e-4eef-b8af-7270e0256978)
- <span class="section-num">10</span> [分布式并行执行加速多模：时空数据库](#h:417292af-a08f-42fe-87f1-b61f525ca2e3)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://help.aliyun.com/zh/polardb/polardb-for-postgresql/polardb-for-postgresql-architecture/?spm=a2c4g.11186623.0.0.2e3b5fb1p0L9je

PolarDB PostgreSQL 版读写分离后，由于底层是存储池，理论上 IO 吞吐是无限大的。而大查询只能在单个计算节点上执行，单个计算节点的 CPU/MEM/IO 是有限的，因此单个计算节点无法发挥出存储侧的大 IO 带宽的优势，也无法通过增加计算资源来加速大的查询。PolarDB PostgreSQL 版推出了基于 Shared-Storage 的 MPP 分布式并行执行，来加速在 OLTP 场景下 OLAP 查询。


## <span class="section-num">1</span> HTAP 架构原理 {#h:036ce9a9-077e-48de-bb55-5234be6c2087}

PolarDB PostgreSQL 版底层存储在不同节点上是共享的，因此不能直接像传统 MPP 一样去扫描表。
PolarDB PostgreSQL 版在原来单机执行引擎上支持了 MPP 分布式并行执行，同时对 Shared-Storage 进行了优化。 基于 Shared-Storage 的 MPP 是业界首创。原理如下：

-   Shuffle 算子屏蔽数据分布。

-   ParallelScan 算子屏蔽共享存储。

{{< figure src="/ox-hugo/p524517.png" width="800px" >}}

如图所示：

-   表 A 和表 B 做 join，并做聚合。

-   共享存储中的表仍然是单张表，并没有做物理上的分区。

-   重新设计 4 类扫描算子，使之在扫描共享存储上的表时能够分片扫描，形成 virtual partition。


## <span class="section-num">2</span> 分布式优化器 {#h:5fb147d2-245c-4846-a70f-120912f6df84}

基于社区的 GPORCA 优化器扩展了能感知共享存储特性的 Transformation Rules。使得能够探索共享存储下特有的 Plan 空间。例如，对于一个表在 PolarDB PostgreSQL 版中既可以全量的扫描，也可以分区域扫描，这个是和传统 MPP 的本质区别。如下图所示，上面灰色部分是 PolarDB PostgreSQL 版内核与 GPORCA 优化器的适配部分。下半部分是 ORCA 内核，灰色模块是在 ORCA 内核中对共享存储特性所做的扩展。

{{< figure src="/ox-hugo/p524524.png" width="800px" >}}


## <span class="section-num">3</span> 算子并行化 {#h:bd455b10-a8d7-45a2-a36b-ce3ffa9fb862}

PolarDB PostgreSQL 版中有 4 类算子需要并行化，以下介绍一个具有代表性的 Seqscan 的算子的并行化。

为了最大限度的利用存储的大 IO 带宽，在顺序扫描时，按照 4 MB 为单位做逻辑切分，将 IO 尽量打散到不同的盘上，达到所有的盘同时提供读服务的效果。这样做还有一个优势，就是每个只读节点只扫描部分表文件，则最终能缓存的表大小是所有只读节点的 BufferPool 总和。

{{< figure src="/ox-hugo/p524526.png" width="800px" >}}

下面的图表中：

-   增加只读节点，扫描性能线性提升 30 倍。

-   打开 Buffer 时，扫描从 37 分钟降到 3.75 秒。

{{< figure src="/ox-hugo/p524527.png" width="800px" >}}


## <span class="section-num">4</span> 消除数据倾斜问题 {#h:9800261a-d0c9-4291-b68d-70585cab4086}

倾斜是传统 MPP 固有的问题：

-   在 PolarDB PostgreSQL 版中，大对象的是通过 heap 表关联 TOAST​表，无论对哪个表切分都无法达到均衡。

-   另外，不同只读节点的事务、Buffer、网络、IO 负载抖动。

以上两点会导致分布执行时存在长尾进程。

{{< figure src="/ox-hugo/p524528.png" width="800px" >}}

-   协调节点内部分成 DataThread 和 ControlThread。

-   DataThread 负责收集汇总元组。

-   ControlThread 负责控制每个扫描算子的扫描进度。

-   扫描快的工作进程能多扫描逻辑的数据切片。

-   过程中需要考虑 Buffer 的亲和性。

**说明** <br />

尽管是动态分配，尽量维护 Buffer 的亲和性。另外，每个算子的上下文存储在 worker 的私有内存中，
Coordinator 不存储具体表的信息。

下面表格中，当出现大对象时，静态切分出现数据倾斜，而动态扫描仍然能够线性提升。

{{< figure src="/ox-hugo/p524529.png" width="800px" >}}


## <span class="section-num">5</span> SQL 级别弹性扩展 {#h:ce8787fe-c246-42e2-90b2-f7f85da2ec59}

利用数据共享的特点，还可以支持云原生下极致弹性的要求。将 Coordinator 全链路上各个模块所需要的外部依赖存在共享存储上，同时 worker 全链路上需要的运行时参数通过控制链路从
Coordinator 同步过来，使 Coordinator 和 worker 无状态化。

{{< figure src="/ox-hugo/p524530.png" width="800px" >}}

因此：

-   SQL 连接的任意只读节点都可以成为 Coordinator 节点，这解决了 Coordinator 单点问题。

-   支持不同的 SQL 使用不同的 CPU 数目执行，灵活的配置不同业务 SQL 配置不同的 CPU 核心数。

{{< figure src="/ox-hugo/p524531.png" width="800px" >}}


## <span class="section-num">6</span> 事务一致性 {#h:00c5781c-9f42-42bf-b6d3-5cd0f06d7274}

多个计算节点数据一致性通过等待回放和 globalsnapshot 机制来完成。等待回放保证所有 worker 能看到所需要的数据版本，而 globalsnapshot 保证了选出一个统一的版本。

{{< figure src="/ox-hugo/p524533.png" width="800px" >}}


## <span class="section-num">7</span> TPC-H 性能：加速比 {#h:50f1201d-8be4-44f6-a94a-813611fe0e09}

{{< figure src="/ox-hugo/p524534.png" width="800px" >}}

使用 1 TB 的 TPC-H 进行了测试，首先对比了 PolarDB PostgreSQL 版新的分布式并行和单机并行的性能，有 3 条 SQL 提速 60 倍，19 条 SQL 提速 10 倍以上。

{{< figure src="/ox-hugo/p524535.png" width="800px" >}}

{{< figure src="/ox-hugo/p524536.png" width="800px" >}}

此外，使用分布式执行引擎测试，增加 CPU 时的性能，可以看到，从 16 核和 128 核时性能线性提升。单看 22 条 SQL，通过增加 CPU，每条 SQL 性能线性提升。


## <span class="section-num">8</span> TPC-H 性能：和传统 MPP 数仓对比 {#h:f71c9216-f9a9-439c-ba38-301fa36ce12e}

和传统 MPP 数仓对比，同样使用 16 个节点，PolarDB PostgreSQL 版的性能是传统 MPP 数仓的 90%。

{{< figure src="/ox-hugo/p524538.png" width="800px" >}}

{{< figure src="/ox-hugo/p524539.png" width="800px" >}}

前面讲到给 PolarDB PostgreSQL 版的分布式引擎做到了弹性扩展，数据不需要充分重分布，当
dop=8 时，性能是传统 MPP 数仓的 5.6 倍。


## <span class="section-num">9</span> 分布式执行加速索引创建 {#h:a08fa54c-934e-4eef-b8af-7270e0256978}

OLTP 业务中会建大量的索引，经分析建索引过程中，80%是在排序和构建索引页，20%在写索引页。通过使用分布式并行来加速排序过程，同时流水化批量写入。

{{< figure src="/ox-hugo/p524541.png" width="800px" >}}

上述优化能够使得创建索引有 4~5 倍的提升。

{{< figure src="/ox-hugo/p524542.png" width="800px" >}}


## <span class="section-num">10</span> 分布式并行执行加速多模：时空数据库 {#h:417292af-a08f-42fe-87f1-b61f525ca2e3}

PolarDB PostgreSQL 版是对多模数据库，支持时空数据。时空数据库是计算密集型和 IO 密集型，可以借助分布式执行来加速。PolarDB PostgreSQL 版针对共享存储退出了扫描共享 RTREE 索引的功能。

{{< figure src="/ox-hugo/p524543.png" width="800px" >}}

-   数据量：40000 万，500 GB。

-   规格：5个只读节点，每个节点规格为 16 核 CPU、128 GB 内存。

-   性能：
    -   随 CPU 数目线性提升。

    -   80 核 CPU 时，提升 71 倍。

{{< figure src="/ox-hugo/p524546.png" width="800px" >}}

