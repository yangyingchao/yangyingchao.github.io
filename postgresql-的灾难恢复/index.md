# PostgreSQL 的灾难恢复


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [What is Disaster Recovery? 什么是灾难恢复？](#what-is-disaster-recovery-什么是灾难恢复)
- <span class="section-num">2</span> [The Pillars of Business Continuity: RTO and RPO &#x2013; 业务连续性的支柱：恢复时间目标（RTO）和恢复点目标（RPO）](#the-pillars-of-business-continuity-rto-and-rpo-and-x2013-业务连续性的支柱-恢复时间目标-rto-和恢复点目标-rpo)
- <span class="section-num">3</span> [The PostgreSQL Evolution (PostgreSQL 的发展)](#the-postgresql-evolution--postgresql-的发展)
- <span class="section-num">4</span> [What is a Backup? 什么是备份？](#what-is-a-backup-什么是备份)
    - <span class="section-num">4.1</span> [Logical Backups 逻辑备份](#logical-backups-逻辑备份)
    - <span class="section-num">4.2</span> [Physical Backups 物理备份](#physical-backups-物理备份)
    - <span class="section-num">4.3</span> [Pros and Cons of Logical vs. Physical Backup: 逻辑备份与物理备份的优缺点：](#pros-and-cons-of-logical-vs-dot-physical-backup-逻辑备份与物理备份的优缺点)
- <span class="section-num">5</span> [Write-Ahead Logging (WAL) 预写日志（WAL）](#write-ahead-logging--wal--预写日志-wal)
- <span class="section-num">6</span> [Continuous Backup and WAL Archiving 持续备份和 WAL 归档](#continuous-backup-and-wal-archiving-持续备份和-wal-归档)
- <span class="section-num">7</span> [Point-in-Time Recovery (PITR) 时间点恢复 (PITR)](#point-in-time-recovery--pitr--时间点恢复--pitr)
- <span class="section-num">8</span> [High Availability vs. Disaster Recovery 高可用性与灾难恢复](#high-availability-vs-dot-disaster-recovery-高可用性与灾难恢复)
- <span class="section-num">9</span> [Conclusion 结论](#conclusion-结论)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://postgr.es/p/7i_

系统故障、硬件故障或意外数据丢失可能会毫无预警地发生。决定操作是顺利恢复还是陷入停滞的因素是灾难恢复设置的强度。
PostgreSQL 具备强大的功能，使可靠的恢复成为可能。

本文将更仔细地探讨这些组件如何在后台协同工作，以保护数据完整性、启用一致的恢复，并确保您的数据库能够从任何故障场景中恢复。


## <span class="section-num">1</span> What is Disaster Recovery? 什么是灾难恢复？ {#what-is-disaster-recovery-什么是灾难恢复}

灾难恢复（Disaster Recovery, DR）是指一套用于在发生灾难时备份和恢复数据库的实践和策略。在此上下文中，灾难指的是任何使整个数据库环境无法使用的事件，例如：

-   云区域故障 - 例如，当 AWS us-east-1 陷入瘫痪，并带走一半的互联网
-   物理灾害 - 火灾、洪水、地震，或甚至是挖掘机切断光纤线路
-   灾难性人为错误 - 比如一个有缺陷的迁移导致关键数据表损坏
-   重大安全事件 - 您必须从已知的良好备份中重建
-   停电 - 延长的停工时间影响可用性
-   硬件故障 - 磁盘、内存或服务器崩溃
-   软件故障 - 缺陷、崩溃或损坏的进程
-   网络攻击 - 勒索软件、数据泄露或恶意篡改
-   …. 以及任何您无法想象的事情！

灾难恢复的目标是在发生意外事件时，确保系统能够迅速恢复到正常的操作状态。


## <span class="section-num">2</span> The Pillars of Business Continuity: RTO and RPO &#x2013; 业务连续性的支柱：恢复时间目标（RTO）和恢复点目标（RPO） {#the-pillars-of-business-continuity-rto-and-rpo-and-x2013-业务连续性的支柱-恢复时间目标-rto-和恢复点目标-rpo}

在设计灾难恢复策略之前，我们必须了解定义它的两个指标：

-   RPO <br />
    恢复点目标 (RPO) 是指在发生故障时，您能够承受的最大数据丢失量，以时间为单位进行衡量。

    它回答的问题是：“当我们恢复时，我们将回到多远的时间？”如果您只能承受 5 分钟的数据丢失，那么您的 RPO 就是 5
    分钟。RPO 主要是一个灾难恢复指标，直接与您的备份和 WAL 归档策略相关。

<!--listend-->

-   RTO <br />
    恢复时间目标 (RTO) 是指在故障后恢复服务的最大可接受时间。

    它回答的是：“应用程序可以停机多长时间？”如果您需要在 30 分钟内重新上线，则您的 RTO 是 30 分钟。RTO
    主要是一个高可用性指标，通常通过复制和故障转移机制来解决。

这两个指标往往是相互制约的，目标是使 RTO 和 RPO 尽可能接近于零。

-   较低的 RPO 需要频繁备份和强大的 WAL 归档，这可能会带来高成本。
-   较低的 RTO 需要按需计算资源和自动化，这也增加了成本。

灾难恢复规划的艺术在于通过风险管理和成本效率分析找到这些对立力量之间的平衡。


## <span class="section-num">3</span> The PostgreSQL Evolution (PostgreSQL 的发展) {#the-postgresql-evolution--postgresql-的发展}

Postgres 的一个令人印象深刻的方面是其开箱即用的灵活性和韧性。早在 2001 年，Postgres 通过引入预写日志（WAL）实现了崩溃恢复，迈出了数据持久性的一大步。

在 2005 年，连续备份和时间点恢复的引入通过在线物理备份和 WAL 存档增强了 Postgres，使得有效的灾难恢复成为可能。

在接下来的十年中，Postgres 将其连续备份框架发展成为我们今天所知的复杂复制系统，专门为现代组织的高可用性需求量身定制。

在深入了解 Postgres 的备份和恢复基础设施之前，让我们先掌握一些基本概念。


## <span class="section-num">4</span> What is a Backup? 什么是备份？ {#what-is-a-backup-什么是备份}

备份是数据的一致副本，可以用来恢复数据库。没有备份，就没有真正的灾难恢复计划。主要有两种类型：

-   逻辑备份
-   物理备份


### <span class="section-num">4.1</span> Logical Backups 逻辑备份 {#logical-backups-逻辑备份}

逻辑备份通过将数据库的内容导出为 SQL 脚本或其他可移植格式来捕获。它是一组可以重建数据库结构的命令，包括表、模式、约束，并重新插入所有数据。

常用工具有：

-   **pg_dump** - 创建单个数据库的备份。
-   **pg_dumpall** - 捕获整个集群，包括所有数据库、角色和全局对象。


### <span class="section-num">4.2</span> Physical Backups 物理备份 {#physical-backups-物理备份}

物理备份是数据库实际文件在存储层的低级副本。它们通过直接复制底层数据文件来捕获数据库的确切状态。

常用工具包括：

-   **Pg_basebackup** - 在线物理备份的标准工具。
-   **pgBackRest** 或 **Barman** - 这些是外部工具，为大规模环境提供自动化和更好的管理。


### <span class="section-num">4.3</span> Pros and Cons of Logical vs. Physical Backup: 逻辑备份与物理备份的优缺点： {#pros-and-cons-of-logical-vs-dot-physical-backup-逻辑备份与物理备份的优缺点}

这里是逻辑备份和物理备份在功能和使用案例上的快速比较：

<a id="table--tbl:post-6ee86d96"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl:post-6ee86d96">Table 1</a>:</span>
  compare table
</div>

| 特性     || 逻辑备份 || 物理备份 |
|--------|------|------|
| 数据复制 || 是   || 是   |
| 备份单个表 || 是   || 否   |
| 版本间迁移 || 是   || 否   |
| 恢复到某个时间点 || 否   || 是   |
| 增量/差异恢复 || 否   || 是   |

简而言之，逻辑备份灵活、便携，适合迁移，而物理备份速度更快，支持时间点恢复，并且在大型生产环境中扩展效果更好。


## <span class="section-num">5</span> Write-Ahead Logging (WAL) 预写日志（WAL） {#write-ahead-logging--wal--预写日志-wal}

要理解 PostgreSQL 的恢复，我们必须理解 WAL。它是持久性的最重要组成部分。

在下面的基础设施图中，有四个主要组件：

-   共享缓冲区
-   PGDATA
-   pg_wal
-   Postgres 后端

让我们先了解这些内容。Postgres 将数据存储在一个名为 PGDATA 的目录中的 8 千字节页面内。事务日志存储在位于 _pg_wal_
目录中的预写日志（WAL）文件中。共享缓冲区充当内存缓存，以提高性能，每个客户端连接由一个称为 Postgres
后端的专用进程处理。

PostgreSQL 的黄金法则是对数据页的任何修改必须在更新（“脏”）页面写回数据文件之前记录在预写日志（WAL）中。

当后端从磁盘请求一个页面时，该页面首先被加载到共享缓冲区中，然后再返回给后端。如果后端修改了页面，更改会首先记录在预写日志（WAL）中，而不是直接在数据文件中。这些信息被写入到 WAL 段中，这就是该机制被称为预写日志记录（Write-Ahead Logging），或简称为 WAL 的原因。

<a id="figure--fig:post-d41d8cd9"></a>

{{< figure src="/ox-hugo/Postgres-backend-1.png" width="800px" >}}

预写日志（WAL）是对数据文件所做的每个确切更改的二进制表示。每个 WAL 记录包含以下关键信息：

-   事务 ID - 标识进行了更改的事务
-   页面信息 - 指定被修改的数据库页面
-   重做信息 - 描述如何重新应用更改
-   撤销信息 - 描述如何撤销更改（用于回滚）
-   CRC 校验和 - 检测损坏并确保数据完整性

每个 WAL 记录首先写入一个称为 WAL 缓冲区的小型高速内存区域。当一个事务被提交时，PostgreSQL 保证相关的 WAL
记录被物理写入 pg_wal 目录中的文件。这标志着持久性的时刻，一旦 WAL 安全地保存在磁盘上，该事务就被视为永久性。即使是突发的停电也无法抹去它。

在 WAL 写入之后，PostgreSQL 可以延迟将实际的修改过的（“脏”）数据页写回磁盘，直到一个更优的时机。PostgreSQL
通过检查点（checkpoint）来协调这个过程。检查点是一个时间点，此时所有脏数据页都被刷新到磁盘，确保数据文件与此时的 WAL 保持一致。

一旦在 WAL 文件中记录的所有更改都已应用于数据文件，该 WAL 段就不再需要用于崩溃恢复。PostgreSQL 不会删除它，而是回收该空间以供将来的 WAL 记录使用。为了灾难恢复，我们通过配置 WAL 归档利用这一机制。我们不是让 PostgreSQL
回收旧的 WAL 文件，而是将每一个都归档到一个单独、安全的位置。

简而言之，为了确保可靠的灾难恢复，保护 Postgres 数据库备份和 WAL 归档至关重要。它们共同构成了时间点恢复（PITR）的基础，并作为 Postgres 复制的基石。


## <span class="section-num">6</span> Continuous Backup and WAL Archiving 持续备份和 WAL 归档 {#continuous-backup-and-wal-archiving-持续备份和-wal-归档}

我们需要持续备份和 WAL 归档，以确保可靠的灾难恢复。现在让我们更深入地了解这些机制在实践中的工作原理。

当 Postgres 服务器运行时，它会不断生成 WAL 文件，记录数据库所做的每一个更改。这些 WAL 文件定期归档到单独的存储位置，通常是像 Amazon S3 或 Google Cloud Storage 这样的对象存储。

与此同时，PGDATA 目录中的数据文件需要作为所谓的基础备份的一部分被物理复制。Postgres 提供了一种 API，可以在数据库保持在线的情况下进行这些备份，这些被称为热物理备份。

过程很简单：

1.  调用 Postgres API 以开始备份（pg_backup_start）。

2.  开始复制 PGDATA 中的所有文件。根据您的数据库大小，此步骤可能需要几分钟甚至几个小时。

3.  在此期间，正在进行的更改仍在 WAL 中记录，确保没有任何数据丢失。这些 WAL 记录也会不断发送到 WAL 归档中。

一个重要的考虑因素是，在进行在线备份时，PGDATA 内的数据文件在被复制的过程中可能会发生变化，因为数据库仍然处于活动状态。乍一看，这可能看起来像是损坏，因为被备份的文件与数据库的当前状态并不完全同步。然而，这实际上并不是一个问题。

Postgres 的设计就是优雅地处理这种情况，通过预写日志（WAL）。对数据库所做的每一个更改首先都会记录在 WAL 中，WAL
会持续归档。因此，即使基础备份包含了一些正在更新的文件，所有后续或缺失的更改也会安全地捕获在 WAL 归档中。

在恢复过程中，Postgres 首先恢复基础备份，然后按顺序重放 WAL 文件。这个重放过程会重新应用备份开始后发生的每一个已记录的更改，将数据库恢复到一个完全一致的状态，就像从未发生过中断一样。

<a id="orgf7d7fcf"></a>

<img src="/ox-hugo/Screenshot-2025-10-09-at-8.24.02-PM-1.png" alt="Screenshot-2025-10-09-at-8.24.02-PM-1.png" width="800px" />
Image Credits:<https://shrturl.app/a2zfKi>

最后，备份过程通过 Postgres API（pg_backup_stop）信号结束复制操作，并等待最终 WAL 文件被归档。这样可以确保到此为止的所有事务都被捕获，从而完成一个一致的、可恢复的备份。为了使备份可恢复，我们必须拥有从备份开始到其完成的所有 WAL 文件。

因此，如果生产数据库运行顺利，它会随着事务的发生持续生成 WAL 文件。随着时间的推移，这些文件按照顺序进行回收和归档。我们的责任只是安排定期的基础备份，并维护连续的 WAL 归档。这两者结合确保数据库可以非常准确地恢复到任何特定的时间点。


## <span class="section-num">7</span> Point-in-Time Recovery (PITR) 时间点恢复 (PITR) {#point-in-time-recovery--pitr--时间点恢复--pitr}

时间点恢复（PITR）是最强大的功能之一，它允许我们将您的数据库恢复到过去的确切时刻。

只要我们有一个基本备份的目录和一系列连续的 WAL 文件安全存储在档案中，PostgreSQL
就可以在任何特定的时间点重建数据库状态，从您最早的基本备份结束到最新 WAL 段中捕获的最近已提交事务。

所以，想象一下，一个开发人员在 2024 年 10 月 4 日下午 2:05 意外运行了 DROP TABLE customers;。使用 PITR，我们可以将“时间倒回”到这个命令执行之前。

1.  恢复： <br />
    首先，准备一个新的 PostgreSQL 实例并将最新的基础备份恢复到其中。

2.  配置恢复： <br />
    接下来，创建一个 recovery.signal 文件，并更新 postgresql.conf 以指向实际的 WAL 存档。然后定义一个恢复目标，例如：
    ```cfg
    recovery_target_time = ‘2024-10-04 14:04:00’
    ```
    就是在意外删除前的一分钟。

3.  恢复： <br />
    当你启动 PostgreSQL 时，它进入恢复模式并开始从存档中获取 WAL 文件。它按顺序回放日志中记录的每个已提交的事务，直到达到指定的恢复目标。

4.  提升： <br />
    一旦恢复达到定义的目标时间，PostgreSQL 会暂停。然后我们提升服务器，使其成为一个完全可操作的主数据库，恢复到事件发生前的确切状态，客户表安全无虞。

通过结合基础备份和持续的 WAL 归档，PostgreSQL 赋予我们恢复数据到任意精确时刻的能力。我们所需的只是设置三件事：

-   定期基础备份
-   配置持续的 WAL 归档
-   将备份和 WAL 文件分布到多个位置，以增强全球 RPO 和 RTO 目标


## <span class="section-num">8</span> High Availability vs. Disaster Recovery 高可用性与灾难恢复 {#high-availability-vs-dot-disaster-recovery-高可用性与灾难恢复}

高可用性（HA）和灾难恢复（DR）通常是相辅相成的，但它们的目的不同。

HA 指的是拥有多个实时运行的生产数据库副本（节点），并保持同步。如果一个节点出现故障、变慢或过载，另一个节点会立即接管，以最小的干扰保持系统可用。

另一方面，DR 在整个环境发生故障时发挥作用，例如区域性停电或重大数据丢失。它依赖于备份和归档的 WAL 文件，将数据库完全恢复到某个特定时间点，通常是在不同的区域或数据中心。

想象一下，一个在 AWS us-east-1 中运行的主要 PostgreSQL 集群。如果一个节点发生故障，HA 会在几秒钟内提升一个备用节点，从而确保持续服务。但如果整个区域离线，DR 则接管：数据库从 us-west-2 的备份和 WAL 归档中恢复，恢复到停机前的精确状态。一旦主区域重新上线，就可以进行再同步。

HA 和 DR 共同形成了两层防御：

-   HA 在当地故障期间保持系统运行，
-   DR 确保在大规模灾难后的恢复。


## <span class="section-num">9</span> Conclusion 结论 {#conclusion-结论}

PostgreSQL 为企业级灾难恢复提供了坚实的基础。通过预写日志、物理基础备份和时间点恢复，它可以满足严格的 RPO 和 RTO 目标。

但真正的韧性不仅仅来自备份 - 它需要一个自动化、设计良好且定期测试的灾难恢复计划，通过定期演练实现。结合强大的高可用性设置，这确保了即使在故障情况下, 数据安全和服务仍不中断。

