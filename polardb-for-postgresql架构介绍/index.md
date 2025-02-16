# PolarDB for PostgreSQL架构介绍


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [传统数据库的问题](#h:f57c62f8-df86-41c9-a6e8-1048a25422f0)
- <span class="section-num">2</span> [PolarDB PostgreSQL 版云原生数据库的优势](#h:6ae075ee-3296-4003-867e-6272f982164a)
- <span class="section-num">3</span> [PolarDB PostgreSQL 版整体架构概述](#h:840385d4-1082-448d-8654-c75053a33282)
    - <span class="section-num">3.1</span> [存储计算分离架构概述](#h:3bac129c-7fec-491a-934b-88c823245954)
    - <span class="section-num">3.2</span> [HTAP 架构概述](#h:2b3aaddd-8c62-4596-b2e2-030f4ed00f3c)

</div>
<!--endtoc-->


本文为摘录，原文为： https://help.aliyun.com/zh/polardb/polardb-for-postgresql/polardb-for-postgresql-architecture/?spm=a2c4g.11186623.0.0.2e3b5fb1p0L9je

PolarDB PostgreSQL 版是一款阿里云自主研发的企业级数据库产品，采用计算存储分离架构，兼容
PostgreSQL 与 Oracle。PolarDB PostgreSQL 版的存储与计算能力均可横向扩展，具有高可靠、高可用、
弹性扩展等企业级数据库特性。同时，PolarDB PostgreSQL 版具有大规模并行计算能力，可以应对
OLTP 与 OLAP 混合负载。还具有时空、向量、搜索、图谱等多模创新特性，可以满足企业对数据处理日
新月异的新需求。

PolarDB PostgreSQL 版支持多种部署形态：存储计算分离部署、X-Paxos 三节点部署以及本地盘部署。


## <span class="section-num">1</span> 传统数据库的问题 {#h:f57c62f8-df86-41c9-a6e8-1048a25422f0}

随着用户业务数据量越来越大，业务越来越复杂，传统数据库系统面临巨大挑战，例如：

-   存储空间无法超过单机上限。
-   通过只读实例进行读扩展，每个只读实例独享一份存储，成本增加。
-   随着数据量增加，创建只读实例的耗时增加。
-   主备延迟高。


## <span class="section-num">2</span> PolarDB PostgreSQL 版云原生数据库的优势 {#h:6ae075ee-3296-4003-867e-6272f982164a}

针对上述传统数据库的问题，阿里云研发了 PolarDB PostgreSQL 版云原生数据库。采用了自主研发
的计算集群和存储集群分离的架构。具备如下优势：

-   扩展性：存储计算分离，极致弹性。
-   成本：共享一份数据，存储成本低。
-   易用性：一写多读，透明读写分离。
-   可靠性：三副本、秒级备份。

    {{< figure src="/ox-hugo/p523755.png" width="800px" >}}


## <span class="section-num">3</span> PolarDB PostgreSQL 版整体架构概述 {#h:840385d4-1082-448d-8654-c75053a33282}


### <span class="section-num">3.1</span> 存储计算分离架构概述 {#h:3bac129c-7fec-491a-934b-88c823245954}

{{< figure src="/ox-hugo/p523768.png" width="800px" >}}

PolarDB PostgreSQL 版是存储计算分离的设计，存储集群和计算集群可以分别独立扩展：

-   当计算能力不够时，可以单独扩展计算集群。
-   当存储容量不够时，可以单独扩展存储集群。

基于 Shared-Storage，主节点和多个只读节点共享一份存储数据，主节点刷脏不能再按照传统的刷
脏方式，否则会导致以下问题：

-   只读节点在存储中读取的页面，可能是比较老的版本，不符合当前的状态。
-   只读节点读取到的页面比自身内存中想要的数据要超前。
-   主节点切换到只读节点时，只读节点接管数据更新时，存储中的页面可能是旧的，需要读取日志
    重新对脏页的恢复。

对于第一个问题，我们需要有页面多版本能力；对于第二个问题，我们需要主库控制脏页的刷脏速度。


### <span class="section-num">3.2</span> HTAP 架构概述 {#h:2b3aaddd-8c62-4596-b2e2-030f4ed00f3c}

读写分离后，单个计算节点无法发挥出存储侧大 IO 带宽的优势，也无法通过增加计算资源来加速大
的查询。PolarDB PostgreSQL 版推出了基于 Shared-Storage 的 MPP 分布式并行执行，来加速在 OLTP 场
景下 OLAP 查询。

PolarDB PostgreSQL 版支持一套 OLTP 场景型的数据在以下两种计算引擎中使用：

-   单机执行引擎：处理高并发的 OLTP 型负载。
-   分布式执行引擎：处理大查询的 OLAP 型负载。

{{< figure src="/ox-hugo/p523773.png" width="800px" >}}

在使用相同的硬件资源时性能达到了传统 MPP 数仓的 90%，同时具备了 SQL 级别的弹性：在计算能
力不足时，可随时增加参与 OLAP 分析查询的 CPU，而数据无需重分布。

