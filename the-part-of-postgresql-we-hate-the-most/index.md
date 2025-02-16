# The Part of PostgreSQL We Hate the Most


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [什么是多版本并发控制？](#h:3e1d4668-8e85-43fc-877a-77a2ca94abf5)
- <span class="section-num">2</span> [PostgreSQL 的多版本并发控制](#h:6d4191d2-59bb-4b0a-9d33-35f6dfb1a233)
    - <span class="section-num">2.1</span> [多版本存储](#h:65745514-9b1d-4b7c-96f9-71c9c8467a81)
    - <span class="section-num">2.2</span> [Version Vacuum](#h:c78fa7fa-85b6-4964-aef9-744751c2d2db)
- <span class="section-num">3</span> [为什么 PostgreSQL 的 MVCC 是最糟糕的](#h:4d04ba17-d382-433b-bcb8-1f0aa3362be6)
    - <span class="section-num">3.1</span> [Problem #1: Version Copying](#h:3082559f-73cc-4963-a475-f83fafea6148)
    - <span class="section-num">3.2</span> [Problem #2: Table Bloat](#h:ba891e94-db54-4062-ba24-e4fe99c89e55)
    - <span class="section-num">3.3</span> [Problem #3: Secondary Index Maintenance](#h:c1e21598-c3d4-4311-8008-f6fc9af9a5e3)
    - <span class="section-num">3.4</span> [Problem #4: Vacuum Management](#h:c28e3ac5-8660-487b-b98d-358af86e369b)
- <span class="section-num">4</span> [Concluding Remarks](#h:2bafa9d8-72e6-462c-bb3f-7e0934bcf08f)

</div>
<!--endtoc-->


本文为摘录，原文为： https://ottertune.com/blog/the-part-of-postgresql-we-hate-the-most/



## <span class="section-num">1</span> 什么是多版本并发控制？ {#h:3e1d4668-8e85-43fc-877a-77a2ca94abf5}

-   在 DBMS 中，MVCC 的目标是在可能的情况下允许多个查询同时读取和写入数据库而不相互干扰。
-   MVCC 的基本思想是，DBMS 永远不会覆盖现有行。
    -   对于每个（逻辑）行，DBMS 维护多个（物理）版本。
    -   当应用程序执行查询时，DBMS 根据某个版本排序（例如创建时间戳），确定要检索哪个版本以满足请求。

这种方法的好处是，多个查询可以读取旧版本的行，而不会被更新它的其他查询阻塞。查询在数据库快照上观察到它开始查询事务时的状态（快照隔离）。

-   这种方法消除了对显式记录锁的需求，该锁会阻止读者在写入程序修改同一项时访问数据。

构建支持 MVCC 的 DBMS 时，系统工程师必须做出几个设计决策。在高层次上，它归结为以下几点：

1.  如何存储对现有行的更新。
2.  如何在运行时为查询找到正确的行版本。
3.  如何删除不再可见的过期版本。

这些决策并不互相排斥。
对于 PostgreSQL 来说，在上世纪 80 年代决定处理第一个问题的方式导致了我们今天仍然需要解决的其他两个问题。


## <span class="section-num">2</span> PostgreSQL 的多版本并发控制 {#h:6d4191d2-59bb-4b0a-9d33-35f6dfb1a233}

PostgreSQL 的多版本并发控制（MVCC）方案的核心思想似乎很简单：<br />
当查询更新表中的现有行时，数据库管理系统会复制该行并将更改应用于这个新版本，而不是覆盖原始行。

我们将这种方法称为 **追加式版本存储方案** 。

但正如我们现在所描述的，这种方法在系统的其他部分有一些非平凡的影响。


### <span class="section-num">2.1</span> 多版本存储 {#h:65745514-9b1d-4b7c-96f9-71c9c8467a81}

postgresql 将所有的行版本存储在同一存储空间的表中。要更新现有的元组，数据库管理系统首先从表中获得一个空槽用于新
的行版本。然后，将当前版本的行内容复制到新版本，并将修改应用到新分配的版本槽中的行。

当应用程序对电影数据库执行更新查询来将《少林与武当》的发行年份从 1985 年更改为 1983 年时，
可以在下面的示例中看到这个过程：

{{< figure src="/ox-hugo/ottertune-mvcc-example1.svg" width="800px" >}}

当一个 UPDATE 查询改变表中的一个元组时，PostgreSQL 会复制该元组的原始版本，然后将更改应用于新版本。
在这个例子中，表页面 #1 中没有更多空间，因此 PostgreSQL 在表页面 #2 中创建新版本。


#### <span class="section-num">2.1.1</span> N2O (new to old, 新到旧) vs O2N （old to new, 就到新） {#h:1da8f3bd-c7db-4cbe-98fb-def1c82ff13a}

现在，使用两个代表相同逻辑行的物理版本，DBMS 需要记录这些版本的血统，以便将来可以找到它们。
MVCC DBMS 通过创建一个单链表来实现版本链。版本链只向一个方向前进，以减少存储和维护开销。
这意味着 DBMS 需要决定使用什么顺序：从新到旧（N2O）顺序还是从旧到新（O2N）顺序。

-   对于 N2O 顺序:
    -   每个元组版本指向其上一个版本，
    -   版本链的头始终是最新版本。

-   对于 O2N 顺序，

    -   每个元组版本指向其下一个版本，
    -   头是最旧的元组版本。

    O2N 方法避免了 DBMS 每次修改元组时更新索引指向较新版本的需要。
    然而，在查询处理过程中，DBMS 可能需要更长时间来找到最新版本，可能需要遍历长的版本链。

    大多数 DBMS，包括 Oracle 和 MySQL，实现了 N2O。
    但是 PostgreSQL 在使用 O2N 方面独树一帜（除了 Microsoft 的 In-Memory OLTP 引擎用于 SQL Server）。


#### <span class="section-num">2.1.2</span> PostgreSQL 如何确定记录哪些内容 {#h:09b5c4f6-6837-4fc0-b190-dbfc0fc22417}

下一个问题是 PostgreSQL 如何确定要记录哪些版本指针。
PostgreSQL 中每行的标头包含了下一个版本的元组 id 字段（ `t_tcid` ）（如果它是最新版本，则是自己的元组 id）。
因此，如下一个例子所示，当一个查询请求最新版本的行时，数据库管理系统遍历索引，定位到最旧的版本，然后跟
随指针直到找到所需的版本。

{{< figure src="/ox-hugo/ottertune-mvcc-example3.svg" width="800px" >}}

PostgreSQL 开发人员很早就意识到其 MVCC 方案存在两个问题。

-   首先，每次更新时都要创建一个完整元组的新副本是昂贵的。
-   其次，为了找到最新版本（大多数查询都需要的）而遍历整个版本链是浪费的。

为了避免遍历整个版本链，PostgreSQL 为每个数据行的物理版本在表的索引中添加一个条目。
这意味着如果一个逻辑行有五个物理版本，那么在索引中将会有（至多）五个该元组的条目！
在下面的示例中，我们可以看到=idx_name= 索引包含了分别位于不同页面上的每个 “Shaolin and Wu Tang”行的条目。
这使得可以直接访问元组的最新版本，而无需遍历长的版本链。

{{< figure src="/ox-hugo/ottertune-mvcc-example4.svg" width="800px" >}}

PostgreSQL 尝试通过在相同的磁盘页（块）中创建新的副本来避免安装多个索引条目，并将相关版本存储在多个页
面上，以减少磁盘 I/O。
这种优化被称为堆唯一元组（ **HOT** ）更新。如果更新不修改表的索引引用的任何列，
并且新版本存储在与旧版本相同的数据页上（如果该页有空间），DBMS 会使用 HOT 方法。现在在我们的例子中，
更新后索引仍然指向旧版本，并且查询通过遍历版本链来检索最新版本。
在正常操作期间，PostgreSQL 通过删除旧版本来优化此过程，以修剪版本链。


### <span class="section-num">2.2</span> Version Vacuum {#h:c78fa7fa-85b6-4964-aef9-744751c2d2db}

每当应用程序更新行时，PostgreSQL 都会复制一份。接下来的问题是系统如何删除旧版本（称为“死元组”）。

上世纪 80 年代的最初版本的 PostgreSQL 没有删除死元组的功能。这样做的想法是保留所有旧版本，
以便应用程序可以执行“时光旅行”查询，以查看数据库在特定时间点的状态（例如，在上周末结束时运行一个
SELECT 查询来检查数据库的状态）。但是从未删除死元组意味着如果应用程序删除元组，表的大小永远不会缩小。
它还意味着经常更新元组的长版本链，这将减慢查询速度，除非 PostgreSQL 添加索引条目，以便查询可以快速跳转
到正确的版本，而不是遍历整个链。但是现在，这意味着索引更大，使它们变得更慢，并增加了额外的内存压力。

为了解决这些问题，PostgreSQL 使用一个净化程序来清理表中的无效元组。这个净化程序会对自上次运行以来被修
改的表页进行顺序扫描，找出过期的版本。数据库管理系统认为一个版本"过期"是指它对所有活动事务都不可见。这
意味着没有当前事务正在访问该版本，未来的事务将使用最新的"活动"版本。因此，删除过期的版本并重新利用该空
间是安全的。

PostgreSQL 根据其配置设置定期执行此净化程序（自动净化）。除了影响所有表的净化频率的全局设置外，
PostgreSQL 还提供了在表级别配置自动净化的灵活性，以便为特定表优化该过程。用户还可以通过 VACUUM SQL 命令手
动触发净化，以优化数据库性能。


## <span class="section-num">3</span> 为什么 PostgreSQL 的 MVCC 是最糟糕的 {#h:4d04ba17-d382-433b-bcb8-1f0aa3362be6}

我们直言不讳地说：如果今天有人要构建一个新的多版本并发控制（MVCC）数据库管理系统，他们不应该像
PostgreSQL 那样做（例如，自动清理的附加存储）。在我们 [2018 年的 VLDB 论文](https://db.cs.cmu.edu/papers/2017/p781-wu.pdf) 中（也被称为“有史以来关于 MVCC 的最
佳论文”），我们没有找到另一个 DBMS 以 PostgreSQL 的方式来实现 MVCC。它的设计是 20 世纪 80 年代以前的产物，在 20
世纪 90 年代的日志结构系统模式大量出现之前就存在了。

让我们来讨论一下 PostgreSQL 的 MVCC 出现的四个问题。我们还将讨论为什么其他的 MVCC DBMS 如 Oracle 和 MySQL 可以避
免这些问题。


### <span class="section-num">3.1</span> Problem #1: Version Copying {#h:3082559f-73cc-4963-a475-f83fafea6148}

在 MVCC 中采用追加方式存储的方案中，如果一个查询更新了一个元组，DBMS 会将所有列复制到新版本中。无论查
询是否更新单个列或所有列，都会进行此复制操作。可以想象，追加方式的 MVCC 会导致大量的数据重复和增加的存储
需求。这种方法意味着 PostgreSQL 需要比其他 DBMS 更多的内存和磁盘存储来存储数据库，这意味着查询较慢且云端成
本更高。

MySQL 和 Oracle 不是将整个元组复制为新版本，而是存储新版本和当前版本之间的紧凑增量（类似于 git diff）。
使用增量意味着如果查询只是更新表中的一个元组的单个列，而该表有 1000 个列，则 DBMS 只存储一个包含对该列
更改的增量记录。另一方面，PostgreSQL 会创建一个包含查询更改的列和其他 999 个未触及列的新版本。我们将忽略
TOAST 属性，因为 PostgreSQL 对它们的处理方式不同。

曾经有一次尝试去现代化 PostgreSQL 的版本存储实现。EnterpriseDB 于 2013 年开始了 zheap 项目，以替换追加式存储
引擎以使用增量版本。不幸的是，根据我们的了解，该努力在 2021 年已经停滞不前。


### <span class="section-num">3.2</span> Problem #2: Table Bloat {#h:ba891e94-db54-4062-ba24-e4fe99c89e55}

在 PostgreSQL 中，过期的版本（即死元组）所占的空间也比增量版本更多。虽然 PostgreSQL 的自动清理
（autovacuum）最终会删除这些死元组，但是写入密集型负载可能导致它们累积得比清理工作更快，从而导致数据库
持续增长。DBMS 在查询执行期间必须加载死元组到内存中，因为系统将死元组与活元组混合在页面中。不受限制的膨
胀会通过导致 DBMS 承担更多的 IOPS，并在表扫描期间消耗更多的内存来降低查询性能。此外，由于死元组引起的不准
确的优化器统计信息可能导致查询计划不佳。

假设我们的电影表（movies table）有 1000 万个活元组和 4000 万个死元组，使得表中 80％的数据都是过时的。
还假设表中的列比我们展示的要多得多，每个元组的平均大小是 1KB。在这种情况下，活元组占据 10GB 的存储空间，
而死元组占据大约 40GB 的存储空间；表的总大小为 50GB。当一个查询在这个表上执行全表扫描时，PostgreSQL 必
须从磁盘上检索所有的 50GB 并存储在内存中，即使其中大部分是过时的。尽管 Postgres 有一种保护机制，可以避
免顺序扫描对其缓冲池缓存的污染，但它并不能帮助减少 IO 成本。

即使你确保 PostgreSQL 的自动清理（autovacuum）以规律的间隔运行，并且能够跟上你的工作负载（这并不总是容
易的，见下文），自动清理也不能回收存储空间。自动清理只会删除死元组，并在每个页面内重新定位活元组，但它
不会从磁盘中回收空白页面。

当数据库管理系统由于不存在任何元组而截断最后一个页面时，其他页面将保留在磁盘上。在上面的例子中，即使
PostgreSQL 从 movies 表中删除了 40GB 的无效元组，它仍然保留了 50GB 的分配存储空间，这来自于操作系统
（或者在 RDS 的情况下来自于 Amazon）。要重新获取和返回这样未使用的空间，必须使用 VACUUM FULL 或者
`pg_repack` 扩展将整个表重写到一个没有浪费存储的新空间中。运行这些操作都不是一个简单的任务，不能不考虑
对生产数据库性能的影响；它们消耗资源且耗时，会严重影响查询性能。下图展示了 VACUUM 和 VACUUM FULL 的工
作原理。

{{< figure src="/ox-hugo/ottertune-mvcc-vacuum.svg" width="800px" >}}

-   VACUUM <br />
    使用 PostgreSQL 的常规 VACUUM 操作，数据库管理系统仅仅从每个表页面中移除已死元组，并重新组织页面以将所有存活元组放在页面的末尾。

<!--listend-->

-   VACUUM FULL <br />
    使用 VACUUM FULL 时，PostgreSQL 会从每个页面中移除已死元组，将剩余的存活元组合并并压缩到一个新的页面（表页面#3），
    然后删除不需要的页面（表页面#1 / #2）。


### <span class="section-num">3.3</span> Problem #3: Secondary Index Maintenance {#h:c1e21598-c3d4-4311-8008-f6fc9af9a5e3}

对元组的单个更新需要 PostgreSQL 更新该表的所有索引。
更新所有索引是必要的，因为 PostgreSQL 在主索引和次要索引中都使用版本的确切物理位置。
除非 DBMS 将新版本存储在与先前版本相同的页面上（HOT 更新），否则系统将为每个更新执行此操作。

回到我们的 UPDATE 查询示例，PostgreSQL 通过将原始版本复制到一个新页面中来创建一个新版本，就像之前一样。
但它还在表的主键索引（movies_pkey）和两个次要索引（idx_director，idx_name）中插入指向新版本的条目。

{{< figure src="/ox-hugo/ottertune-mvcc-example5.svg" width="800px" >}}

PostgreSQL 索引维护操作的示例，涉及非 HOT 更新。
数据库管理系统在表页#2 中创建元组的新版本，然后在所有表的索引中插入指向该版本的新条目。

PostgreSQL 每次更新需要修改表的所有索引，这对性能有几个影响。

显然，这会使更新查询变慢，因为系统需要做更多的工作。数据库管理系统需要额外的 I/O 来遍历每个索引并插入新
的条目。访问索引会引入锁/闩锁争用，无论是在索引还是数据库管理系统的内部数据结构（如缓冲池的页表）中。
再次强调，即使查询永远不会使用它们（顺便说一下，OtterTune 可以自动找出您数据库中未使用的索引），
PostgreSQL 也会为表的所有索引进行维护工作。这些额外的读取和写入在按 IOPS 计费的 DBMS（如 Amazon Aurora）中
是有问题的。

正如上文所述，如果 PostgreSQL 可以执行 HOT 写操作，即新版本位于当前版本的同一页上，它就会避免每次更新索引。
我们对 OtterTune 客户的 PostgreSQL 数据库进行的分析显示，平均约有 46%的更新使用 HOT 优化。尽管这个数字令人印
象深刻，但仍意味着超过 50%的更新在支付这种代价。

有许多用户在处理 PostgreSQL 的 MVCC 实现的这个方面时遇到了困难。其中最著名的例子是 Uber 在 2016 年发表的一篇博
文，解释了他们为何从 Postgres 转为 MySQL。他们的写入密集工作负载在具有许多次要索引的表上遇到了严重的性能
问题。

Oracle 和 MySQL 在 MVCC 实现中并没有这个问题，因为它们的次要索引不存储新版本的物理地址，而是存储一个逻辑标
识符（如元组 ID、主键），然后 DBMS 使用该标识符来查找当前版本的物理地址。现在，这可能会使次要索引的读取速
度较慢，因为 DBMS 必须解析逻辑标识符，但这些 DBMS 在其 MVCC 实现中具有其他优势来减少开销。

旁注：Uber 的博文中关于 PostgreSQL 版本存储的说法有错误。具体来说，每个 PostgreSQL 的元组都存储了指向
新版本的指针，而不是博文中所述的指向上一个版本的指针。这导致了一个 O2N 版本链排序，而不是 Uber 错误声称的
N2O 版本链。


### <span class="section-num">3.4</span> Problem #4: Vacuum Management {#h:c28e3ac5-8660-487b-b98d-358af86e369b}

PostgreSQL 的性能非常依赖于自动清理过时数据和回收空间的 autovacuum 的效果（这就是为什么 OtterTune 在您首
次连接数据库时立即检查 autovacuum 的健康状态）。无论您使用的是 RDS，Aurora 还是 Aurora Serverless，
PostgreSQL 的所有变体都存在相同的 autovacuum 问题。

但是，要确保 PostgreSQL 的 autovacuum 尽可能地运行良好是困难的，因为它非常复杂。

PostgreSQL 针对自动清理进行了默认设置，但对于所有表格来说并不理想，尤其是对于大表格来说。例如，控制在
PostgreSQL 触发 autovacuum 之前需要更新多少百分比的表格的配置选项（autovacuum_vacuum_scale_factor）的默认
设置是 20%。这个阈值意味着如果一个表格有 1 亿条数据，数据库管理系统不会触发 autovacuum，直到查询更新至
少 2000 万条数据。因此，PostgreSQL 可能会在一个表格中保留很多死元组（从而产生 IO 和内存开销）很长时间。

PostgreSQL 中 autovacuum 的另一个问题是它可能受到长时间运行的事务的阻塞，这可能导致更多的死元组和陈旧
的统计信息积累。未能及时清理过期版本会导致许多性能问题，引发更多长时间运行的事务，从而阻塞 autovacuum
进程。这成为一个恶性循环，需要人工干预通过手动终止长时间运行的事务。考虑下面的图表，它显示了一个
OtterTune 客户数据库中两周内死元组的数量：

{{< figure src="/ox-hugo/ottertune-mvcc-deadtuples.svg" width="800px" >}}

图表中的锯齿状模式显示自动清理（autovacuum）大约每天进行一次重大清理。例如，在 2 月 14 日，数据库管理
系统（DB MS）清理了 320 万个已经死亡的元组。这个图表实际上是一个不健康的 PostgreSQL 数据库的例子。图表
清楚地显示了已经死亡元组数量的上升趋势，因为自动清理跟不上。

在 OtterTune，我们经常在客户的数据库中看到这个问题。一个 PostgreSQL RDS 实例在大量插入后因为旧的统计信
息导致一个长时间运行的查询。这个查询阻塞了自动清理更新统计信息，导致更多的长时间运行的查询。OtterTune
的自动健康检查识别出了这个问题，但管理员仍然需要手动终止查询并在大量插入后运行 ANALYZE。好消息是，长查
询的执行时间从 52 分钟减少到只有 34 秒。


## <span class="section-num">4</span> Concluding Remarks {#h:2bafa9d8-72e6-462c-bb3f-7e0934bcf08f}

在构建数据库管理系统（DBMS）时，总会面临艰难的设计决策。这些决策将导致不同工作负载下的 DBMS 性能不同。
对于 Uber 的特定写入密集工作负载来说，由于 MVCC（多版本并发控制）导致的 PostgreSQL 索引写入放大是他们
转向 MySQL 的原因。但请不要误解我们的口吻，以为我们认为你永远不应该使用 PostgreSQL。虽然它的 MV CC 实现
方式并不正确，但 PostgreSQL 仍然是我们最喜欢的 DBMS。爱一样东西就意味着愿意和它的缺陷共事（参见丹·萨维奇
的《入场费》）。

那么，如何解决 PostgreSQL 的怪癖呢？嗯，你可以花费大量的时间和精力来自行调优。祝你好运。

