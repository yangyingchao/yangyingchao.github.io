# OceanBase 707M tpmC


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#h:0a45267f-a3ae-43ad-8a8a-2a8ff4d14e35)
- <span class="section-num">2</span> [DESIGN OVERVIEW](#h:81c079a8-b5c8-4073-b6dc-4390e495b9ac)
    - <span class="section-num">2.1</span> [Goals](#h:89cbb686-47c9-439f-94af-5baf3dbdec55)
    - <span class="section-num">2.2</span> [Criteria of Design](#h:d8875fcd-41a7-4eb3-bcdb-7f4698884d41)
- <span class="section-num">3</span> [架构图](#h:1d973c3d-dfa5-4501-806a-a3691d5f5ddb)
    - <span class="section-num">3.1</span> [Application Layer](#h:8c3fca18-cdec-4d5c-a080-0d9f7ce5d187)
    - <span class="section-num">3.2</span> [Proxy Layer](#h:02cd8c19-34d7-4bbd-8ec9-c5e1df9a54b2)
    - <span class="section-num">3.3</span> [Data Service Layer](#h:dda54135-ae89-427a-88cd-a83212f504d5)
    - <span class="section-num">3.4</span> [Zones](#h:e6097928-8983-4e4e-a6e7-789f28d6ba2c)
    - <span class="section-num">3.5</span> [表](#h:e7b840bd-069f-43ac-9f26-efd8b9d23815)
    - <span class="section-num">3.6</span> [Node](#h:b613b575-2ec7-4ac9-af01-6abc2ac33597)
    - <span class="section-num">3.7</span> [SQL Engine](#h:93134f71-c553-4bdf-85d9-b635dc2e6cfa)
    - <span class="section-num">3.8</span> [Multi-tenancy](#h:3ff4162e-c44a-44bf-a591-06272e393a61)
    - <span class="section-num">3.9</span> [Resource Isolation.](#h:795aebda-ad5d-423b-bbd8-18e201a3ea05)
    - <span class="section-num">3.10</span> [Features](#h:d1d0b2ec-39d3-4860-a5e2-1fc17e06cb0a)
- <span class="section-num">4</span> [STORAGE ENGINE](#h:b529b550-1b76-4517-b51e-c2ef317a50e8)
    - <span class="section-num">4.1</span> [LSM Tree-Based Architecture](#h:2d0129de-6b8c-4123-bcae-3381d34e2426)
    - <span class="section-num">4.2</span> [Asymmetric Read and Write](#h:3b88b7f0-2a81-46b3-a504-81ead89714dc)
    - <span class="section-num">4.3</span> [Daily Incremental Major Compaction](#h:fd24213c-c8e8-47e2-b267-b650ade48456)
    - <span class="section-num">4.4</span> [Replica Type](#h:38ef652e-1cac-4458-ab8d-c5a450387a17)
- <span class="section-num">5</span> [TRANSACTION PROCESSING ENGINE](#h:daef193b-c166-40bc-897c-1ba68e6d2e7e)
    - <span class="section-num">5.1</span> [Partition and Paxos Group](#h:9ddf467b-d11e-4b8e-9056-ff15b67602fe)
    - <span class="section-num">5.2</span> [Timestamp Service](#h:af98d321-1caf-4efe-811a-12302ffb7321)
    - <span class="section-num">5.3</span> [Transaction Processing](#h:3fa0c121-d1e6-41d0-94b0-aba40baa6340)
    - <span class="section-num">5.4</span> [Isolation Level](#h:48068368-5e2c-46d7-8973-fcd6ed207b2e)
    - <span class="section-num">5.5</span> [Replicated Table](#h:9c090d3d-1284-4a59-8748-5f4f41d92fd4)
- <span class="section-num">6</span> [TPC-C BENCHMARK TEST](#h:b9272e56-7a6c-4576-bb2e-07d908598d53)
- <span class="section-num">7</span> [LESSONS IN BUILDING OCEANBASE](#h:dd8ffbcb-ac77-4683-ae5b-4bdb8f828c61)
    - <span class="section-num">7.1</span> [From NoSQL to NewSQL](#h:bba12c9f-9272-42f1-ba0f-121f5f6a19fa)
    - <span class="section-num">7.2</span> [Both cost and performance](#h:a901f7ee-f75f-41b7-aa25-cafb87bf5a98)
    - <span class="section-num">7.3</span> [Data validation](#h:c0fcd1a9-665b-43af-b2be-8b02b1349136)
    - <span class="section-num">7.4</span> [Partitioning vs. sharding](#h:823cf2bc-3ef4-4e03-b67d-154e672b80ec)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/d/p3385-xu.pdf



## <span class="section-num">1</span> Introduction {#h:0a45267f-a3ae-43ad-8a8a-2a8ff4d14e35}


## <span class="section-num">2</span> DESIGN OVERVIEW {#h:81c079a8-b5c8-4073-b6dc-4390e495b9ac}


### <span class="section-num">2.1</span> Goals {#h:89cbb686-47c9-439f-94af-5baf3dbdec55}

-   Fast scale-out (scale-in) on commodity hardware to achieve high performance and low TCO.
-   Cross-region deployment and fault tolerance.


### <span class="section-num">2.2</span> Criteria of Design {#h:d8875fcd-41a7-4eb3-bcdb-7f4698884d41}


## <span class="section-num">3</span> 架构图 {#h:1d973c3d-dfa5-4501-806a-a3691d5f5ddb}

<a id="figure--fig:p3"></a>

{{< figure src="/ox-hugo/-003-002.jpg" caption="<span class=\"figure-number\">Figure 1: </span>OceanBase 架构图" width="800px" >}}


### <span class="section-num">3.1</span> Application Layer {#h:8c3fca18-cdec-4d5c-a080-0d9f7ce5d187}


### <span class="section-num">3.2</span> Proxy Layer {#h:02cd8c19-34d7-4bbd-8ec9-c5e1df9a54b2}


### <span class="section-num">3.3</span> Data Service Layer {#h:dda54135-ae89-427a-88cd-a83212f504d5}


### <span class="section-num">3.4</span> Zones {#h:e6097928-8983-4e4e-a6e7-789f28d6ba2c}

-   Zones can be restricted to one **region** or spead over multiple regions.
-   事务通过 Paxos 在多个区域中复制 <br />
    Transactions are replicated among the zones, using Paxos.


### <span class="section-num">3.5</span> 表 {#h:e7b840bd-069f-43ac-9f26-efd8b9d23815}

-   由用户显式地进行分区，并作为数据分布和负载均衡的基本单元
-   每个分片在每个 zone 中都有一个副本


### <span class="section-num">3.6</span> Node {#h:b613b575-2ec7-4ac9-af01-6abc2ac33597}

-   每个 Node (OBServer) 与传统的 RDMS 相似。
-   从 SQL 生成执行计划
    -   如果是 local plan ， 直接执行
    -   否则， 通过两阶段提交协议来执行，
        -   此时该节点作为 coordinator
        -   当 Paxos 组中多数节点的 redo log 持久化后，才提交事务


### <span class="section-num">3.7</span> SQL Engine {#h:93134f71-c553-4bdf-85d9-b635dc2e6cfa}

<a id="figure--fig:screenshot@2022-09-09-18:10:13"></a>

{{< figure src="/ox-hugo/screenshot@2022-09-09_18:10:13.png" width="800px" >}}


#### <span class="section-num">3.7.1</span> plan cache {#h:ce19ec14-eb33-426a-a124-6aa866434e99}

-   收到请求后，仅进行简单的词法分析，然后在 plan cache 中匹配缓存的计划，找不到时候再去走完整的解析流程。
-   10x 提升。。。


### <span class="section-num">3.8</span> Multi-tenancy {#h:3ff4162e-c44a-44bf-a591-06272e393a61}


#### <span class="section-num">3.8.1</span> System Tenant 系统租户 {#h:1fa5cbc3-0369-4445-8702-0e6321751d4b}

系统内置，主要功能：

-   作为系统表的容器 &#x2013; 系统表都存储在系统租户的空间内
-   。。。
-   。。。


#### <span class="section-num">3.8.2</span> Ordinary Tenant 普通髭胡 {#h:21a3b204-d403-41d9-a8f4-5760a4505658}


### <span class="section-num">3.9</span> Resource Isolation. {#h:795aebda-ad5d-423b-bbd8-18e201a3ea05}


### <span class="section-num">3.10</span> Features {#h:d1d0b2ec-39d3-4860-a5e2-1fc17e06cb0a}


## <span class="section-num">4</span> STORAGE ENGINE {#h:b529b550-1b76-4517-b51e-c2ef317a50e8}

基于 LSM-tree, 支持非对称数据块读写，日常增量 compaction, 和不同的副本类型。

<a id="figure--fig:p3385-xu--004-003"></a>

{{< figure src="/ox-hugo/-004-003.png" caption="<span class=\"figure-number\">Figure 3: </span>OB 的存储引擎" width="800px" >}}


### <span class="section-num">4.1</span> LSM Tree-Based Architecture {#h:2d0129de-6b8c-4123-bcae-3381d34e2426}

-   数据分成两部分：
    -   静态基准数据 (static baseline data): 存储于 SSTable 中
    -   动态增量数据 (dynamic incremental data) ：  存储于  MemTable 中

-   SSTable &amp; MemTable
    -   SSTable 为只读数据，生成之后就不再发生更改
    -   MemTable 支持读写操作，只存在内存中
    -   DML 操作 MemTable
    -   MemTable 打到一定大小以后， dump 到磁盘，生成 SSTable
    -   查询针对两种存储分别进行，然后进行合并后返回给客户端
    -   针对 SSTable 实现了 Block 缓存和行缓存，以减少对基线数据的随机访问

-   Compaction
    -   Minor Compaction <br />
        -   内存中数据达到一定大小后，开始进行 minor compaction
        -   Minor Compaction 将 MemTable 转换成为 SSTable

    -   Major Compaction
        -   由系统每天进行增量的 Major Compaction
        -   Major Compaction 将 SSTable 和当天发生的变化进行合并，形成新版本的基线

-   该设计导致每次查询需要读取基准数据和增量数据， OB 做了很多优化
    -   缓存 （block 级与行级）
    -   BloomFilter: 用于进行 empty checks


### <span class="section-num">4.2</span> Asymmetric Read and Write {#h:3b88b7f0-2a81-46b3-a504-81ead89714dc}

OB 实现了非对称读写。

-   读：
    -   基本单元为 microblock
    -   4K 或者 8K
-   写：
    -   基本单元为 macroblock
    -   2MB
    -   macroblock 同时也是存储系统的分配和垃圾回收的基础单元
-   多个 microblock 组装成一个 macroblock
    -   磁盘使用更高效
    -   但造成了一定程度上的写放大


### <span class="section-num">4.3</span> Daily Incremental Major Compaction {#h:fd24213c-c8e8-47e2-b267-b650ade48456}

-   数据且分成 2MB 大小的 macroblock, Major Compaction 中：
    -   数据有修改，则重写该 block
    -   数据无修改，则在新的 baseline 中直接重用，无 IO 开销
    -   因此 Compaction 开销比 LevelDB RockDB 更小


### <span class="section-num">4.4</span> Replica Type {#h:38ef652e-1cac-4458-ab8d-c5a450387a17}

有多种类型的副本：

-   full replica 完全副本，包含
    -   基线数据
    -   增量数据
    -   redo log

-   Data replica, 数据副本
    -   包括基准数据和 relod log
    -   根据需要从完全副本拷贝 minor compactions （compacted mutabtions）
    -   重做完日志后，可以升级至完全副本
    -   与完全副本相比，节省 CPU 和内存资源：
        -   不必重做日志
        -   没有 MemTable

-   Log Replica 日志副本
    -   仅包含 redo log
    -   作为 Paxos 组的成员
    -   部署两个完全副本和一个日志副本，则：
        -   拥有高可用特性
        -   存储和内存开销大大减少

            <a id="figure--fig:screenshot@2022-09-13-10:17:00"></a>

            {{< figure src="/ox-hugo/screenshot@2022-09-13_10:17:00.png" >}}


## <span class="section-num">5</span> TRANSACTION PROCESSING ENGINE {#h:daef193b-c166-40bc-897c-1ba68e6d2e7e}


### <span class="section-num">5.1</span> Partition and Paxos Group {#h:9ddf467b-d11e-4b8e-9056-ff15b67602fe}

-   Partition 分片是数据分布、负载均衡和 Paxos 同步的基础单元
-   每个分片一个 Paxos Group


### <span class="section-num">5.2</span> Timestamp Service {#h:af98d321-1caf-4efe-811a-12302ffb7321}

-   使用 timestamp Paxos group 来实现时间戳服务的高可用
-   timestamp paxos group 的 leader 与表分片的 paxos group leader 通常放在一个区域（region）中


### <span class="section-num">5.3</span> Transaction Processing {#h:3fa0c121-d1e6-41d0-94b0-aba40baa6340}


### <span class="section-num">5.4</span> Isolation Level {#h:48068368-5e2c-46d7-8973-fcd6ed207b2e}

-   read committed: default isolation level
-   也支持 snapshot isolation


### <span class="section-num">5.5</span> Replicated Table {#h:9c090d3d-1284-4a59-8748-5f4f41d92fd4}

-   synchronously replicated table
    -   变更需要等所有节点完成
    -   慢

-   asynchronously replicated table
    -   等待 paxos group 中的多数完成即可
    -   快，但不保证所有节点的数据都为最新
    -   如果查询中某节点遇到了老版本的数据，需要访问远端副本


## <span class="section-num">6</span> TPC-C BENCHMARK TEST {#h:b9272e56-7a6c-4576-bb2e-07d908598d53}


## <span class="section-num">7</span> LESSONS IN BUILDING OCEANBASE {#h:dd8ffbcb-ac77-4683-ae5b-4bdb8f828c61}


### <span class="section-num">7.1</span> From NoSQL to NewSQL {#h:bba12c9f-9272-42f1-ba0f-121f5f6a19fa}

-   应用层不应将数据库尊为一个 key-value 存储系统来用，也不应该倚赖某些数据的高级特性
-   存储过程对某些 OLTP 应用来说仍有很大的价值
-   对于使用分布式数据库的应用来说，每个事务、每个 SQL 都应该有超时机制：分布式系统的出错率更高一些（网络，节点等原因）


### <span class="section-num">7.2</span> Both cost and performance {#h:a901f7ee-f75f-41b7-aa25-cafb87bf5a98}


### <span class="section-num">7.3</span> Data validation {#h:c0fcd1a9-665b-43af-b2be-8b02b1349136}


### <span class="section-num">7.4</span> Partitioning vs. sharding {#h:823cf2bc-3ef4-4e03-b67d-154e672b80ec}

