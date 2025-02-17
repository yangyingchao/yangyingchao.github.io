# gp shared snapshot


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [DtaStrutor](#h:6d1e31ba-8f2f-4776-8ba5-2f2e7a62301f)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： ../../../Work/pg_gpdb/src/backend/utils/time/sharedsnapshot.c

在 Greenplum 中，作为切片计划的一部分，许多 PostgreSQL 进程（qExecs，QE）在单个段数据库上运行，作为同一用户 SQL 语句的一部分。属于特定用户在特定段数据库上的所有 qExecs 需要具有一致的可见性。为此，使用了一种称为“共享本地快照”（Shared Local Snapshot）的思想。共享内存数据结构 SharedSnapshotSlot 在特定数据库实例上的会话的统一流程（gang processes）之间共享会话和事务信息。这些流程被称为 SegMate 进程组。

SegMate 进程组由一个 QE（查询执行器）Writer 进程和 0 个、1个或多个 QE Reader 进程。
Greenplum 需要发明 SegMate 共享机制，因为在 Postgres 中只有一个后端，并且大部分需要的信息只是在私有内存中可用。通过 Greenplum 在数据库实例上的会话并行性，我们需要一种方式来在 SegMates 之间共享尚未提交的会话信息。这些信息包括事务快照、子事务状态、所谓的 combo-cid 映射等。

举个例子：QE（Query Executor）读取器需要使用与 QE 写入器相同的快照和命令号信息，以便可以看到 QE 写入器所写入的当前数据。在一个事务中，QE 写入器会将新数据写入共享内存缓存中。在同一个事务中的稍后阶段，QE 读取器需要识别出共享内存缓存中属于其会话事务的元组，以便能够正确执行。

另一个例子： QE（查询执行器）读取器需要知道哪些子事务对于一个会话的事务是活动的或已提交的，这样它们才能正确读取由 QE 写入器为该事务所写入的子事务数据。

所以，重点是与 QE 读者共享尚未提交的私有会话事务信息，以便 SegMate 进程组可以正确地处理该事务。我们通常认为 QE 写者/读者位于各个片段上。但是，协调器有一个特殊用途的 QE 读者，称为 Entry DB Singleton。因此，SegMate 模块也可以在协调器上工作。

每个共享快照只在段数据库中本地可见。高级 Writer 团队成员建立了一个本地事务，获得了共享快照
shmem 空间中的槽位，并将快照信息复制到共享内存中，以供其他与该段相关的 qExecs 找到。下一节详细介绍了共享内存的初始化过程，谁写入快照，如何控制以及何时读者可以读取快照，锁定等等。

-   **共享内存初始化** <br />
    -   共享内存由 postmaster 设置。
    -   QD 上每个用户连接需要一个槽来存储一组 segmate 的快照信息的数据结构。
    -   在每个槽中，QD 写入器存储由 SharedSnapshotSlot 定义的信息。Shared Memory
        Initialization: Shared memory is setup by the postmaster. One slot for every user
        connection on the QD is kind of needed to store a data structure for a set of
        segmates to store their snapshot information. In each slot QE writer stores
        information defined by SharedSnapshotSlot.

-   **PQsendMppStatement** ：<br />
    与 PQsendQuery 相同，只是它还发送一个序列化的快照和 xid。已经修改了 postgres.c 以接受这个新的协议消息。它的工作基本上与'Q'（普通查询）相同，只是从 QD（QueryDispatcher）中解开快照和 xid，并将其存储起来。在语句分发期间，所有的 QE（Query Executor）都在 QD 快照中发送。

Global Session ID: The shared snapshot shared memory is split into slots. A
set of segmates for a given user requires a single slot. The snapshot
information and other information is stored within the snapshot. A unique
session id identifies all the components in the system that are working for
a single user session. Within a single segment database this essentially
defines what it means to be "segmates."  The shared snapshot slot is
identified by this unique session id. The unique session id is sent in from
the QD as a GUC called "mpp_session_id". So the slot's field "slotid" will
store the "mpp_session_id" that WRITER to the slot will use. Readers of the
slot will find the correct slot by finding the one that has the slotid
equal to their own mpp_session_id.

-   **全局会话 ID**: <br />

    -   共享快照的共享内存被分割成槽位 (slot)。
    -   对于一个特定用户的 segmate 集合，只需要一个槽位。
    -   快照信息和其他信息都存储在快照中。
    -   唯一会话 ID 标识了系统中为单个用户会话工作的所有组件。

    在一个单一段数据库中，这基本上定义了"segmates"的含义。共享快照槽位由这个唯一会话 ID 进行标识。这个唯一会话 ID 通过名为 `mpp_session_id` 的 GUC 从 QD 发送过来。因此，槽位的字段"slotid"将存储作为 WRITER 使用的"mpp_session_id"。槽位的读取者将通过找到 slotid 与其自己的 mpp_session_id 相等的槽位来找到正确的槽位。

Single Writer: Mechanism is simplified by introducing the restriction of
only having a single qExec in a set of segmates capable of writing. Single
WRITER qExec is the only qExec amongst all of its segmates that will ever
perform database write operations.  Benefits of the approach, Single WRITER
qExec is the only member of a set of segmates that need to participate in
global transactions. Also&#x2026; only this WRITER qExec really has to do
anything during commit. Assumption seems since they are just reader qExecs
that this is not a problem. The single WRITER qExec is the only qExec that
is guaranteed to participate in every dispatched statement for a given user
(at least to that segdb). Also, it is this WRITER qExec that performs any
utility statement.

-   **单一写入者**: <br />
    通过引入只允许在一组能够写入的 segmates 中只有一个 qExec 的限制来简化机制。单一写入者 qExec
    是所有 segmates 中唯一执行数据库写入操作的 qExec。这种方法的好处是，单一写入者 qExec 是一组
    segmates 中唯一需要参与全局事务的成员。此外&#x2026;只有这个写入者 qExec 在提交时才真正需要执行任何操作。假设由于它们只是读取者 qExecs，这不是一个问题。单一写入者 qExec 是唯一一个保证参与给定用户的每个分派语句的 qExec（至少对于该 segdb 而言）。同时，这个写入者 qExec 还执行任何实用语句。

Coordinating Readers and Writers: The coordination is on when the writer
has set the snapshot such that the readers can get it and use it. In
general, we cannot assume that the writer will get to setting it before the
reader needs it and so we need to build a mechanism for the reader to (1)
know that its reading the right snapshot and (2) know when it can read.
The Mpp_session_id stored in the SharedSnapshotSlot is the piece of
information that lets the reader know it has got the right slot. And it
knows can read it when the xid and cid in the slot match the transactionid
and curid sent in from the QD in the SnapshotInfo field.  Basically QE
READERS aren't allowed to read the shared local snapshot until the shared
local snapshot has the same QD statement id as the QE Reader. i.e. the QE
WRITER updates the local snapshot and then writes the QD statement id into
the slot which identifies the "freshness" of that information. Currently QE
readers check that value and if its not been set they sleep (gasp!) for a
while. Think this approach is definitely not elegant and robust would be
great maybe to replace it with latch based approach.

-   **协同读写者** ：<br />
    当写者设置快照以便读者可以获取并使用时，协作才开始。一般来说，我们不能假设写者在读者需要之前就设置好快照，因此我们需要为读者建立机制：(1) 确定读取正确的快照 (2) 确定何时可以进行读取。存储在 SharedSnapshotSlot 中的 Mpp_session_id 是让读者知道它获得了正确的快照的信息。当快照中的 xid 和 cid 与从 SnapshotInfo 字段中来自 QD 的 transactionid 和
    curid 相匹配时，读者知道可以读取。基本上，只有当共享本地快照与 QE Reader 具有相同的 QD 语句 id 时，QE Reader 才被允许读取共享本地快照。也就是说，QE Writer 更新本地快照，然后将 QD 语句 id 写入标识该信息的槽中的情况下，QE Reader 可以进行读取。目前，QE 读者检查该值，如果未设置，则暂时休眠。认为这种方式绝对不够优雅和稳健，可能可以考虑用锁定机制替代。

Cursor handling through SharedSnapshot: Cursors are funny case because they
read through a snapshot taken when the create cursor command was executed,
not through the current snapshot. Originally, the SharedSnapshotSlot was
designed for just the current command. The default transaction isolation
mode is READ COMMITTED, which cause a new snapshot to be created each
command. Each command in an explicit transaction started with BEGIN and
completed with COMMIT, etc. So, cursors would read through the current
snapshot instead of the create cursor snapshot and see data they shouldn't
see. The problem turns out to be a little more subtle because of the
existence of QE Readers and the fact that QE Readers can be created later –
long after the create cursor command. So, the solution was to serialize the
current snapshot to a temporary file during create cursor so that
subsequently created QE Readers could get the right snapshot to use from
the temporary file and ignore the SharedSnapshotSlot.

-   **通过 SharedSnapshot 处理游标** ： <br />
    游标是一种特殊情况，因为它们是通过在执行创建游标命令时所采取快照来读取数据的，而不是当前的快照。最初，SharedSnapshotSlot 只为当前命令设计。默认的事务隔离模式是读提交（READ
    COMMITTED），这会导致每个命令都创建一个新的快照。显式事务中的每个命令都以 BEGIN 开始，以 COMMIT 完成等。因此，游标将通过当前快照而不是创建游标快照来读取数据，并且会看到不应该看到的数据。问题的复杂性在于存在 QE Reader 以及后面可能创建的 QE Reader 的事实。因此，解决方案是在创建游标时将当前快照序列化到临时文件中，以便后续创建的 QE Reader 可以从临时文件中获取正确的用于的快照，并忽略 SharedSnapshotSlot。

Sub-Transaction handling through SharedSnapshot: QE Readers need to know
which sub-transactions the QE Writer has committed and which are active so
QE Readers can see the right data. While a sub-transaction may be committed
in an active parent transaction, that data is not formally committed until
the parent commits. And, active sub-transactions are not even
sub-transaction committed yet. So, other transactions cannot see active or
committed sub-transaction work yet. Without adding special logic to a QE
Reader, it would be considered another transaction and not see the
committed or active sub-transaction work. This is because QE Readers do not
start their own transaction. We just set a few variables in the xact.c
module to fake making it look like there is a current transaction,
including which sub-transactions are active or committed. This is a
kludge. In order for the QE Reader to fake being part of the QE Writer
transaction, we put the current transaction id and the values of all active
and committed sub-transaction ids into the SharedSnapshotSlot shared-memory
structure. Since shared-memory is not dynamic, poses an arbitrary limit on
the number of sub-transaction ids we keep in the SharedSnapshotSlot
in-memory. Once this limit is exceeded the sub-transaction ids are written
to temp files on disk.  See how the TransactionIdIsCurrentTransactionId
procedure in xact.c checks to see if the backend executing is a QE Reader
(or Entry DB Singleton), and if it is, walk through the sub-transaction ids
in SharedSnapshotSlot.

-   **通过 SharedSnapshot 处理子事务** ： <br />
    QE Reader 需要知道 QE Writer 提交了哪些子事务以及哪些子事务仍然活动，以便 QE Reader 可以看到正确的数据。虽然可以在活动的父事务中提交子事务，但是该数据直到父事务提交才被正式提交。此外，活动的子事务还没有提交。因此，其他事务还不能看到活动或提交的子事务的工作。如果没有向 QE Reader 添加特殊逻辑，它将被视为另一个事务，并且不会看到已提交或活动的子事务的工作。这是因为 QE Reader 不会启动自己的事务，我们只是在 xact.c 模块中设置了一些变量，以假装有当前事务，包括哪些子事务是活动的或已经提交的。这是一个曲线解决方案。为了让 QE Reader 伪装成 QE Writer 事务的一部分，我们将当前事务 id 和所有活动和已提交的子事务 id 的值放入 SharedSnapshotSlot 共享内存结构中。由于共享内存不是动态的，在内存中对保存的子事务 id 数量施加了任意限制。一旦超过了这个限制，子事务 id 就会被写入磁盘上的临时文件中。请参阅 xact.c 中的 TransactionIdIsCurrentTransactionId 过程，该过程检查执行的后端是否为 QE Reader（或 Entry DB Singleton），如果是，则遍历 SharedSnapshotSlot 中的子事务 id。


## <span class="section-num">1</span> DtaStrutor {#h:6d1e31ba-8f2f-4776-8ba5-2f2e7a62301f}

```plantuml

class SharedSnapshotStruct {
+ int numSlots
+ int maxSlots
+ int nextSlot
+ SharedSnapshotSlot slots
+ TransactionId xips
}



class SharedSnapshotSlot {
+ int32 slotindex
+ int32 slotid
+ PGPROC writer_proc
+ PGXACT writer_xact
+ volatile distributedXid
+ volatile ready
+ uint32 segmateSync
+ SnapshotData snapshot
+ LWLock slotLock
+ volatile cur_dump_id
+ volatile dump
+ SubTransactionId fullXid
+ TimestampTz startTimestamp
}

SharedSnapshotStruct *-- SharedSnapshotSlot


class PGPROC {
+ SHM_QUEUE links
+ PGPROC procgloballist
+ PGSemaphore sem
+ int waitStatus
+ Latch procLatch
+ LocalTransactionId lxid
+ LocalDistribXactData localDistribXactData
+ int pid
+ int pgprocno
+ BackendId backendId
+ Oid databaseId
+ Oid roleId
+ int mppSessionId
+ PGPROC lockGroupLeader
+ dlist_head lockGroupMembers
+ dlist_node lockGroupLink
}

class PGXACT {
+ TransactionId xid
+ TransactionId xmin
+ uint8 vacuumFlags
+ bool overflowed
+ bool delayChkpt
+ uint8 nxids
}

class SnapshotData {
+ SnapshotType snapshot_type
+ TransactionId xmin
+ TransactionId xmax
+ TransactionId xip
+ uint32 xcnt
+ TransactionId subxip
+ int32 subxcnt
+ _Bool suboverflowed
+ _Bool takenDuringRecovery
+ _Bool copied
+ _Bool haveDistribSnapshot
+ CommandId curcid
+ uint32 speculativeToken
+ uint32 active_count
+ uint32 regd_count
+ pairingheap_node ph_node
+ TimestampTz whenTaken
+ XLogRecPtr lsn
+ DistributedSnapshotWithLocalMapping distribSnapshotWithLocalMapping
}

class LWLock {
+ int tranche
+ pg_atomic_uint32 state
+ proclist_head waiters
}

SharedSnapshotSlot *-- PGPROC
SharedSnapshotSlot *-- PGXACT
SharedSnapshotSlot *-- SnapshotData
SharedSnapshotSlot *-- LWLock
```

