# OCI Database with PostgreSQL: 完善OCI的云数据库套件以满足各种需求


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [为什么 OCI Database for PostgreSQL 脱颖而出](#h:2f8a6966-c6d0-4c53-bd48-a538dcab3522)
- <span class="section-num">2</span> [Vanilla PostgreSQL 的问题](#h:d1146bb4-c2b0-45a9-bfdd-0e5a6f5671e7)
- <span class="section-num">3</span> [OCI Database with PostgreSQL – 高级架构](#h:ed0ac907-e092-4485-942e-d97ced71c119)
    - <span class="section-num">3.1</span> [数据库优化存储 (DbOS) 的优点](#h:f3643978-df64-4c44-91e6-3eb81331394a)
    - <span class="section-num">3.2</span> [进一步的存储优化](#h:edaa1505-1793-4798-94fb-fca3c6a5e41f)
    - <span class="section-num">3.3</span> [结论](#h:da77a97b-e786-464e-aafc-82fc707ff5af)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://mp.weixin.qq.com/s?__biz=MzI3OTM3MDkyNg==&mid=2247497316&idx=1&sn=d08b84d63ab2e4b69949af9170762189&chksm=eb4a7ba5dc3df2b3ca2b3721e5614ab5711be6e984ec44173035323953d193440240f8412591&mpshare=1&scene=1&srcid=1208AtRH30qSa2LdMxzmf3sV&sharer_shareinfo=9a0858a92025bbb0e8887abc9a5eee8a&sharer_shareinfo_first=9a0858a92025bbb0e8887abc9a5eee8a#rd

对于希望在云中轻松扩展且无需更改应用程序的组织来说，OCI Database with PostgreSQL 数据库
是理想的解决方案。它提供完全的 PostgreSQL 兼容性，同时让用户利用 OCI 灵活、高性能、高可
用性的基础设施以及内置的安全性和简单的定价。此外，这种 OCI 实施还提供了卓越的可扩展性并
减少了管理需求。


## <span class="section-num">1</span> 为什么 OCI Database for PostgreSQL 脱颖而出 {#h:2f8a6966-c6d0-4c53-bd48-a538dcab3522}

-   性能提升高达 3 倍
    -   数据库优化存储
    -   将 SQL 事务处理引擎与存储层解耦
-   成本不到友商的一半

我们将开源 PostgreSQL 称为“Vanilla PostgreSQL”。


## <span class="section-num">2</span> Vanilla PostgreSQL 的问题 {#h:d1146bb4-c2b0-45a9-bfdd-0e5a6f5671e7}

{{< figure src="/ox-hugo/oci-640.png" width="800px" >}}

1.  主要故障转移期间的数据丢失 - 非零 RPO<br />
    如图 2 所示，客户通常跨可用性域复制数据库实例。

    -   此复制是从 AD1 中的主数据库异步执行到  AD2 中的“副本”的。
    -   副本数据库可能落后于主数据库。
    -   如果主数据库发生故障并且副本被提升为新的主数据库，则可能会因滞后而丢失一些数据。
    -   数据丢失量取决于升级的副本与旧主副本的落后程度。

    vanilla PostgreSQL 中解决此问题的一个解决方案是同步复制功能，但由于性能开销巨大，因此不太受欢迎。

2.  手动升级和可管理性的复杂性： <br />
    尽管您可以在不同的可用性域中设置副本以实现高可用性，但将备用副本升级为主副本是一个手
    动且复杂的过程。必须谨慎选择要升级的新候选节点，从而有效减少数据丢失，同样，将旧主节
    点重新添加回集群需要更多手动步骤。例如，旧的主数据库可能有本地提交的多余事务，需要首
    先使用 pg_rewind 等工具清除这些事务，然后才能重新加入集群。

3.  创建只读副本既昂贵又缓慢： <br />
    使用普通 PostgreSQL，创建新副本需要在主数据库上拍摄数据快照并赶上主数据库。对于可能达
    到 TB 级的大型数据库，这是一项昂贵且缓慢的操作。为了能够处理应用程序的突发读取需求，
    客户必须过度配置数据库资源。由于每个副本必须具有数据库的完整副本，因此运行多个副本的
    存储成本可能会变得昂贵。


## <span class="section-num">3</span> OCI Database with PostgreSQL – 高级架构 {#h:ed0ac907-e092-4485-942e-d97ced71c119}

在 OCI Database with PostgreSQL 数据库中，Oracle 将 **复制和持久性问题推到了新的数据库优化存储(DbOS) 层**:

-   该层是专门为实现高规模、高可用性和高性能数据库服务而构建的。DbOS 提供高度耐用的网络附加存储，
-   其中数据块在三个可用性域区域中的多个可用性域之间复制。

在单 AD 区域中，数据跨多个故障域复制。集群中的所有 PostgreSQL 节点都访问相同的网络附加存
储。每个备用副本不再需要维护自己的数据库副本。主实例写入共享存储，而备用副本实例从同一共
享存储读取并服务用户查询。

{{< figure src="/ox-hugo/oci-640.jpeg" width="800px" >}}


### <span class="section-num">3.1</span> 数据库优化存储 (DbOS) 的优点 {#h:f3643978-df64-4c44-91e6-3eb81331394a}

-   持久性（零 RPO）： <br />
    -   DbOS 在多可用性域区域中跨多个可用性域复制数据，并且可以在整个可用性域丢失的情况下幸存下来。
    -   DbOS 使用基于仲裁的复制在幕后复制数据块。<br />
    -   如果主节点发生故障，数据库可以故障转移到使用复制的 DbOS 的其他数据库节点，并且这个新
        升级的主 PostgreSQL 实例可以接管而不会丢失数据。之前在旧主数据库上提交的所有事务都存
        在于新主数据库中。
    -   由于 DbOS 层执行复制，因此无需仅仅为了持久性而运行多个副本实例。<br />
        例如，可以在没有任何副本的情况下使用 PostgreSQL 实例运行单节点 OCI 数据库，但仍然不会牺牲耐用性。
        即使在这种单节点设置中，也保证您具有零 RPO。

-   高可用性 (99.99%)：<br />
    -   使用 OCI Database with PostgreSQL 数据库，您可以在几分钟内自动将主数据库故障转移到集群中的另一个副本。
    -   主故障转移速度很快，并且对应用程序几乎是透明的，因为恢复时间目标只需要几分钟。
    -   为了透明地启用故障转移，主端点被设置为浮动 IP 地址，该地址会自动移动到新的主端点。
    -   故障转移后，应用程序会自动重新建立与新主数据库的数据库连接，无需对应用程序进行任何配置更改。
    -   与普通 PostgreSQL 不同，您在启动主故障转移时无需担心复制滞后和数据丢失。<br />
        您无需手动选择特定副本来减少数据丢失。集群中的所有实例共享相同的存储，因此在故障转
        移时，新的主实例可以保证零数据丢失。

-   弹性：
    -   由于数据库存储在所有节点之间共享，因此您可以快速创建或删除副本，以满足用户查询工作负载需求。
    -   与普通 PostgreSQL 不同，您不需要在主节点上拍摄数据快照并将其复制到副本节点来启动新的备用 PostgreSQL 实例。
    -   可以像使用 PostgreSQL 在 OCI 数据库中启动计算实例一样快速地创建备用副本。

-   只读副本的水平扩展：
    -   由于副本节点与 PostgreSQL 共享 OCI 数据库中的数据库存储，因此无论数据库集群中的副本数量有多少，您只需要维护一份数据库副本。
    -   与在云中运行普通 PostgreSQL 相比，带有 PostgreSQL 的 OCI 数据库可显着节省存储成本。
    -   此外，OCI 的 PostgreSQL 服务提供按使用量付费的数据定价模型，并具有自动扩展功能，可降低您的成本。

-   低副本延迟：
    -   复制延迟是普通 PostgreSQL 只读副本设置的一个主要挑战。
    -   由于副本必须重播并保存主副本所做的所有更改，因此很容易落后，尤其是在网络分区的情况下。
    -   通过共享存储，副本的工作量显着减少。它只需要将更改应用于其缓存中的页面，并且不必保留这些更改。
    -   使用这种架构，复制延迟通常以毫秒为单位，这使得读取查询能够近乎实时地执行或完成。

-   高效复制：
    -   OCI Database with PostgreSQL 在存储层执行复制。
    -   因此，主实例不需要将预写日志  (WAL) 记录物理传送到所有副本。
    -   相反，它通知副本有新的更改，并且各个副本直接从共享存储读取 latest  WAL 记录。
    -   这可以减少主服务器上的负载，并且可以更有效地扩展到更多数量的副本。

在我们的实验中，具有内置跨 AD 复制功能的 OCI Database with PostgreSQL 数据库比普通
PostgreSQL 中的同步复制快两倍多。


### <span class="section-num">3.2</span> 进一步的存储优化 {#h:edaa1505-1793-4798-94fb-fca3c6a5e41f}

除了共享存储优化之外，OCI Database with PostgreSQL 还实施了以下优化以进一步提高性能。

-   原子写入：
    -   DbOS 针对已知的数据库性能风险实施优化，例如消除“撕裂写入”。
    -   通常，大多数数据库需要某种针对“撕裂写入”的保护，当数据库使用的页面大小（PostgreSQL
        使用 8 KB）与底层存储的“原子写入单元”大小（通常为 512B 或 4KB）。例如，如果这是自上
        一个检查点以来对页面的第一次修改，PostgreSQL 首先将整个 8KB 页面写入 WAL，然后将该页
        面刷新到磁盘。如果页面写入被破坏，那么 PostgreSQL 会回退到使用之前在 WAL 中写入的整
        页，并且不会造成任何损害。但这种保护是有代价的——它会导致 WAL 膨胀，并且频繁的检查点
        会加剧问题，而频繁的检查点需要减少计划外故障转移期间的恢复时间。
    -   我们在 DbOS 中实现了对 PostgreSQL 页面的原子写入支持。
        -   存储层永远不会覆盖现有页面。
        -   相反，它使用日志结构化技术始终将页面写入磁盘上的新位置，并维护从逻辑文件偏移到磁盘位置的映射层。
        -   旧版本的页面会定期被垃圾收集。这避免了双重写入。

-   优化的页面缓存：
    -   带有 PostgreSQL 的 OCI 数据库使用专门构建的缓存层，这与依赖于通用 Linux 内核页面缓存的普通 PostgreSQL 不同。
    -   OCI 的页面缓存实现有很多优化，例如：
        -   专为 PostgreSQL 工作负载定制的自定义预取逻辑。
        -   避免在 PostgreSQL 共享缓冲区和页面缓存中双重缓存页面
        -   通过预取数据页加速 PostgreSQL 恢复

-   存储级备份： <br />
    在普通 Postgres 中，为了维护数据库备份，WAL 被复制到对象存储，并定期拍摄文件系统快照。
    此过程同时使用主节点上的网络和 CPU。OCI 数据库与 PostgreSQL 将备份委托给存储层，从而消
    除了备份的网络和 CPU 开销。


### <span class="section-num">3.3</span> 结论 {#h:da77a97b-e786-464e-aafc-82fc707ff5af}

正如前面详细介绍的，OCI Database with PostgreSQL 数据库在成本、性能、规模、可用性和持久
性方面提供了显着的优势。实现大部分优势的关键是基于 DbOS 和 DbFS，它们是专门为优化
PostgreSQL 以在云规模上更有效地工作而构建的。

