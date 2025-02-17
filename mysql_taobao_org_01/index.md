# 聊聊日志即数据库


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Single Page Recovery](#single-page-recovery)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： http://mysql.taobao.org/monthly/2023/11/01/

[《数据库故障恢复机制的前世今生》](http://catkang.github.io/2019/01/16/crash-recovery.html)一文中介绍过，由于磁盘的的顺序访问性能远好于随机访问，数据库设计中通常都会采用 WAL 的方式，将随机访问的数据库请求转换为顺序的日志 IO，并通过 Buffer Pool 尽量的合并并推迟真正的数据修改落盘。如果发生故障，可以通过日志的重放恢复故障发生前未刷盘的修改信息。也就是说 Log 中包含数据库恢复所需要的全部信息。

现代数据库为了追求更高的事务并发度，会：

-   显式地区分用户可见的逻辑内容和维护内部的物理结构，
-   在并发控制上支持了 Lock 和 Latch 的分层，
-   同时也在故障恢复上区分了 User Transaction 和 System Transaction。

在这种设计下， 保证数据库 D（Durable）的 Redo Log 需要能在 Crash Recovery 的过程中，在完全不感知用户事务的情况下，恢复未提交的 System Transaction。因此，Redo Log 的设计上天然就是 Page Oriented 的，也就是说每条 Redo Log 都被限制在单个 Page 中，其重放过程不需要感知用户事务的存在，也不需要关心其他的 Page。

在[《B+树数据库故障恢复概述》](http://catkang.github.io/2022/10/05/btree-crash-recovery.html)中详细的讨论过这个过程，也提到这样做的好处使得在 Crash Recovery 的过程中 Page 的恢复过程可以实现充分的并发。到这里我们就可以引出本文想要讨论的主要内容：

**已知：**

-   特性 1（完备）：Log 中包含数据库恢复所需要的全部信息；
-   特性 2（Page Oriented）：Page 通过 Log 的恢复过程只需要关心当前的 Page 本身；

那么，通过这两个特性，数据库设计能实现哪些实用和有趣的功能呢？


## <span class="section-num">1</span> Single Page Recovery {#single-page-recovery}

