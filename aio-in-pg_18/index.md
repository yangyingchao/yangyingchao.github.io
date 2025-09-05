# AIO in pg_18


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Motivation](#motivation)
    - <span class="section-num">1.1</span> [Why Asynchronous IO](#why-asynchronous-io)
    - <span class="section-num">1.2</span> [Why Direct / unbuffered IO](#why-direct-unbuffered-io)
    - <span class="section-num">1.3</span> [AIO Usage Example](#aio-usage-example)
- <span class="section-num">2</span> [Design Criteria &amp; Motivation](#design-criteria-and-motivation)
    - <span class="section-num">2.1</span> [Deadlock and Starvation Dangers due to AIO](#deadlock-and-starvation-dangers-due-to-aio)
    - <span class="section-num">2.2</span> [IO 可以在关键区段中启动](#io-可以在关键区段中启动)
    - <span class="section-num">2.3</span> [State for AIO needs to live in shared memory](#state-for-aio-needs-to-live-in-shared-memory)
- <span class="section-num">3</span> [Design of the AIO Subsystem](#design-of-the-aio-subsystem)
    - <span class="section-num">3.1</span> [AIO Methods](#aio-methods)
    - <span class="section-num">3.2</span> [AIO 句柄](#aio-句柄)
    - <span class="section-num">3.3</span> [AIO Callbacks](#aio-callbacks)
    - <span class="section-num">3.4</span> [AIO Targets](#aio-targets)
    - <span class="section-num">3.5</span> [AIO Wait References](#aio-wait-references)
    - <span class="section-num">3.6</span> [AIO Results](#aio-results)
    - <span class="section-num">3.7</span> [AIO Errors](#aio-errors)
- <span class="section-num">4</span> [Helpers](#helpers)
    - <span class="section-num">4.1</span> [读取流](#读取流)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/README.md



## <span class="section-num">1</span> Motivation {#motivation}


### <span class="section-num">1.1</span> Why Asynchronous IO {#why-asynchronous-io}

直到异步 IO 的引入，Postgres 依赖于操作系统来隐藏同步 IO 带来的成本。虽然在许多工作负载中这效果惊人地好，但在预读取和控制写回方面却没有我们想要的那么出色。

有一些重要且昂贵的操作，比如 `fdatasync()` ，操作系统无法隐藏存储延迟。这对于 WAL 写入尤其重要，因为异步发出 `fdatasync()` 或 `O_DSYNC` 写入的能力可以显著提高吞吐量。


### <span class="section-num">1.2</span> Why Direct / unbuffered IO {#why-direct-unbuffered-io}

希望使用 Direct IO 的主要原因是：

-   降低 CPU 使用率 / 提高吞吐量<br />
    特别是在现代存储中，缓冲写入由于操作系统需要使用 CPU 将数据从内核的页面缓存复制到 Postgres 缓冲池而导致瓶颈。而直接 IO 通常可以直接在存储设备和 Postgres 的缓冲缓存之间移动数据，使用 DMA。在该传输进行时，CPU
    可以自由地执行其他工作。

-   降低延迟 <br />
    直接 IO 的延迟通常比缓冲 IO 低得多，这对于因 WAL 写入延迟而瓶颈的 OLTP 工作负载具有重要影响。

-   避免操作系统缓存和 Postgres 的共享缓冲区之间的双重缓冲。

-   更好地控制脏数据写回的时机和节奏。

不使用 Direct IO 的主要原因是：

-   如果没有 AIO，直接 IO 对于大多数用途来说速度过慢，无法使用。
-   即使有 AIO，Postgres 的许多部分仍需进行修改以执行显式预取。
-   在无法将共享缓冲区设置为适当较大的情况下，例如因为有许多不同的 Postgres 实例托管在共享硬件上，性能通常会比使用缓冲 IO 时更差。


### <span class="section-num">1.3</span> AIO Usage Example {#aio-usage-example}

在许多情况下，能够从 AIO 受益的代码不必直接与 AIO 接口进行交互，而可以通过更高层次的抽象使用 AIO。

在这个例子中，缓冲区将被读取到共享缓冲区中。

```c
/*
 * 操作的结果，仅在此后端访问。
 */
PgAioReturn ioret;

/*
 * 获取 AIO 句柄，ioret 会在完成时获得结果。
 *
 * 请注意，ioret 需要在 IO 完成之前保持活动状态或
 * 当前资源所有者释放之前（即抛出错误）。
 */
PgAioHandle *ioh = pgaio_io_acquire(CurrentResourceOwner, &ioret);

/*
 * 用于等待下面发起的 IO 的引用。此
 * 引用可以存放在本地或共享内存中，任何
 * 进程都可以等待它。每个 IO 可以有任意数量的引用。
 */
PgAioWaitRef iow;

pgaio_io_get_wref(ioh, &iow);

/*
 * 安排在 IO 完成时调用共享缓冲区完成回调。
 * 此回调将更新与 AioHandle 关联的缓冲区描述符，这例如允许其他后端访问缓冲区。
 *
 * 可以向回调传递一小部分数据，例如指示在缓冲区无效时是否将其归零。
 *
 * 每个句柄可以注册多个完成回调。
 */
pgaio_io_register_callbacks(ioh, PGAIO_HCB_SHARED_BUFFER_READV, 0);

/*
 * 完成回调需要知道在 IO 完成时要更新哪些缓冲区。
 * 由于 AIO 子系统不知道缓冲区，因此必须将此信息与 AioHandle 关联，以供上面注册的完成
 * 回调使用。
 *
 * 在此示例中，我们只读取一个缓冲区，因此为 1。
 */
pgaio_io_set_handle_data_32(ioh, (uint32 *) &buffer, 1);

/*
 * 将 AIO 句柄传递给较低级别的函数。当操作缓冲区级别时，我们不知道 IO 是如何执行的，这是
 * 存储管理器实现的责任。
 *
 * 例如，md.c 需要将块号转换为段中的偏移量。
 *
 * 一旦 IO 句柄被交付给 smgrstartreadv()，则不能再使用，因为 IO 可能会立即在
 * smgrstartreadv() 下面执行，并且句柄会被重用于另一个 IO。
 *
 * 为了以高效的方式发出多个 IO，调用者可以在开始多个 IO 之前调用
 * pgaio_enter_batchmode()，然后通过 pgaio_exit_batchmode() 结束该批处理。
 * 请注意，在可能存在未提交的 IO 时，要小心，因为另一个后端可能需要等待一个未提交的 IO。
 * 如果这个后端必须等待另一个后端，则会导致一个未被检测到的死锁。有关更多详细信息，请参见 pgaio_enter_batchmode()。
 *
 * 请注意，即使在批处理模式下，IO 也可能会立即提交，例如由于达到了未提交 IO 的数量限制，甚至
 * 在 smgrstartreadv() 返回之前完成。
 */
void *page = BufferGetBlock(buffer);
smgrstartreadv(ioh, operation->smgr, forknum, blkno,
               &page, 1);

/*
 * 为了利用 AIO，在等待 IO 完成之前执行其他工作（包括
 * 提交其他 IO）是有益的。否则我们可以使用同步、阻塞 IO。
 */
perform_other_work();

/*
 * 我们做了一些其他工作，现在需要 IO 操作完成才能继续。
 */
pgaio_wref_wait(&iow);

/*
 * 此时，IO 已完成。然而，我们尚不知道它是成功
 * 还是失败。缓冲区的状态已更新，这允许其他
 * 后端使用缓冲区（如果 IO 成功），或重试 IO
 * （如果失败）。
 *
 * 请注意，如果 IO 失败，可能会发出 LOG 消息，
 * 但没有抛出 ERROR。这一点至关重要，因为另一个等待
 * 此 IO 的后端不应该看到 ERROR。
 *
 * 要检查操作是否成功，并抛出 ERROR，或在更适当的情况下发出 LOG，我们传递
给 pgaio_io_acquire() 的 PgAioReturn 将被使用。
 */
if (ioret.result.status == PGAIO_RS_ERROR)
    pgaio_result_report(ioret.result, &ioret.target_data, ERROR);

/*
 * 除了完全成功之外，IO 还可能 a) 部分成功或 b) 成功但有警告（例如由于 zero_damaged_pages）。
 * 如果我们例如尝试一次读取多个块，读取可能只是
 * 成功于前几个块。
 *
 * 如果 IO 部分成功并且此后端需要所有块都已完成，
 * 该后端需要重新发出有关其余缓冲区的 IO。
 * AIO 子系统无法对此重试进行透明处理。
 *
 * 由于此示例已经很长，并且我们只读取一个块，因此如果存在部分读取或警告，我们将直接报错。
 */
if (ioret.result.status != PGAIO_RS_OK)
    pgaio_result_report(ioret.result, &ioret.target_data, ERROR);

/*
 * IO 成功，因此我们现在可以使用缓冲区。
 */
```


## <span class="section-num">2</span> Design Criteria &amp; Motivation {#design-criteria-and-motivation}


### <span class="section-num">2.1</span> Deadlock and Starvation Dangers due to AIO {#deadlock-and-starvation-dangers-due-to-aio}

在一个 AIO 源/目标是共享资源的环境中，以天真的方式使用 AIO 很容易导致死锁，例如 postgres 的 `shared_buffers`
中的页面。

考虑一个后端在一个表上进行预读，启动多个缓冲区的 IO，超前于当前的“扫描位置”。如果该后端随后执行某个阻塞操作，或者即使只是慢，那么异步启动的读取的 IO 完成可能就不会被处理。

这种 AIO 实现通过要求 AIO 方法要么允许系统中任何后端处理 AIO 完成（例如 `io_uring` ），要么保证即使发起后端被阻塞，AIO 处理仍然会发生（例如，工作模式，将完成处理卸载到 AIO 工作器）。


### <span class="section-num">2.2</span> IO 可以在关键区段中启动 {#io-可以在关键区段中启动}

使用 AIO 进行 WAL 写入可以大大减少 WAL 日志的开销：

-   AIO 允许急切地启动 WAL 写入，使其在需要等待之前就完成
-   AIO 允许同时进行多个 WAL 刷新
-   AIO 使得使用 `O_DIRECT + O_DSYNC` 更现实，这可以减少某些操作系统和存储硬件（缓冲 IO 和没有 O_DSYNC 的直接 IO
    需要在写入及其完成后发出缓存刷新）的往返次数，而 `O_DIRECT + O_DSYNC` 可以使用单个强制单位访问（FUA）写入。

能够在关键区段中执行 IO 的需求对 AIO 子系统有实质性的设计影响。这主要是因为完成 IO（参见前一节）需要在关键区段内可行，即使待完成的 IO 本身并不是在关键区段中发出的。例如，考虑一个后端首先从共享缓冲区开始多个写入，然后开始刷新 WAL 的情况。由于同一时间内只能有有限数量的 IO
处于进行中，因此启动刷新 WAL 的 IO 可能需要首先完成先前启动的 IO。


### <span class="section-num">2.3</span> State for AIO needs to live in shared memory {#state-for-aio-needs-to-live-in-shared-memory}

因为 Postgres 使用进程模型，并且 AIO 需要能够被任何后端完成，所以 AIO 子系统的大部分状态需要存储在共享内存中。

在 `EXEC_BACKEND` 构建中，由于 ASLR 的影响，后端的可执行代码和其他进程本地状态在每个进程中的地址不一定是相同的。这意味着共享内存不能包含指向回调的指针。


## <span class="section-num">3</span> Design of the AIO Subsystem {#design-of-the-aio-subsystem}


### <span class="section-num">3.1</span> AIO Methods {#aio-methods}

为了实现可移植性和性能，实施了多种执行异步 IO 的方法，未来可能还值得添加其他方法。


#### <span class="section-num">3.1.1</span> 同步模式 {#同步模式}

`io_method=sync` 实际上并不执行异步 IO，而是在执行同步 IO 时允许使用异步 IO API。这对于调试可能是有用的。同步模式的代码也被作为回退使用，例如 [工作者模式](#worker) 使用它来执行无法由工作者执行的 IO。


#### <span class="section-num">3.1.2</span> 工作者 {#工作者}

`io_method=worker` 在 Postgres 运行的每个平台上都可用，通过将 IO 调度到几个工作进程之一，以同步方式实施异步 IO —从发起进程的视角来看。


#### <span class="section-num">3.1.3</span> `io_uring` {#io-uring}

`io_method=io_uring` 在 Linux 5.1 及以上版本可用。与工作者模式相对，它在进程内部调度所有 IO，从而降低上下文切换率/延迟。


### <span class="section-num">3.2</span> AIO 句柄 {#aio-句柄}

Postgres 异步 IO 抽象的核心 API 部分是 AIO 句柄。要执行 IO，首先必须获取一个 IO 句柄（ `pgaio_io_acquire()` ），然后“定义”它，即将一个 IO 操作与该句柄关联起来。

通常，在更高层次上获取 AIO 句柄，然后将其传递给更低层次以进行完全定义。例如，对于与共享缓冲区之间的 IO，bufmgr.c
例程获取句柄，然后通过 smgr.c、md.c 传递，最后在 fd.c 中完全定义。

在最低级别用于定义操作的函数是 `pgaio_io_start_*()` 。

因为获取 IO 句柄必须始终成功并且 AIO 句柄的数量必须受到限制，所以，只要 AIO 句柄完成，就可以重新使用它们。显然，代码需要能够响应 IO 完成。状态可以使用 AIO 完成回调更新，并且发起的后端可以提供一个后端本地变量来接收 IO
的结果。发起的后端和任何其他后端都可以使用 _AIO 引用_ 等待 IO 完成。

因为 AIO 句柄在调用 `pgaio_io_acquire()` 后并不可执行，并且 `pgaio_io_acquire()` 需要始终成功（若无 PANIC），因此只能获取单个 AIO 句柄（即，通过 `pgaio_io_acquire()` 返回），而不导致 IO 被定义（通过，可能间接，导致
`pgaio_io_start_*()` 被调用）。否则，后端可能会轻易自我死锁，耗尽所有 AIO 句柄而无法等待某些 IO 完成。

如果发现某个 AIO 句柄不再需要，例如，因为该句柄是在持有一个争用锁之前获取的，可以通过 `pgaio_io_release()`
在未被定义的情况下释放。


### <span class="section-num">3.3</span> AIO Callbacks {#aio-callbacks}

通常情况下，多个层次需要对 IO 的完成做出反应。<br />
例如，对于读操作，md.c 需要检查 IO 是否完全失败或长度是否短于所需，bufmgr.c 需要验证页面是否有效，并且 bufmgr.c
需要更新 BufferDesc 以更新缓冲区的状态。

多个层次/子系统需要对 IO 完成做出反应的事实带来了一些挑战：

-   上层不应需要了解下层的细节。<br />
    例如，bufmgr.c 不应假设 IO 会通过 md.c。因此，上层无法知道下层会认为什么是错误。

-   下层不应需要知道上层的情况。<br />
    例如，smgr API 在通过共享缓冲区时被使用，但也在绕过共享缓冲区时被使用。这意味着例如 md.c 没有能力验证校验和。

-   在 AIO 子系统中为每个可能的层组合编写代码将导致大量重复。

解决这个问题的方法是能够将多个完成回调与一个句柄关联。<br />
例如，bufmgr.c 可以有一个回调来更新 BufferDesc 状态，以及验证页面，而 md.c 可以有另一个回调来检查 IO 操作是否成功。

正如[提到的](#state-for-aio-needs-to-live-in-shared-memory)，目前共享内存无法包含函数指针。因此，完成回调不是通过函数指针直接识别，而是通过 ID（ `PgAioHandleCallbackID` ）进行识别。一个显著的额外好处是，这允许用更小的内存量（当前为单个字节）来标识回调。

除完成外，AIO 回调还用于“暂存”IO。这用于增加缓冲区引用计数，以考虑 AIO 子系统对缓冲区的引用，这对于处理发起请求的后端错误并在 IO 仍在进行时释放其自身的引脚的情况是必需的。

如[前面所解释的](#io-can-be-started-in-critical-sections)，IO 完成必须在关键部分安全执行。为了让发出 IO
的后端在失败的情况下能够错误处理，可以使用[AIO 结果](#aio-results)。


### <span class="section-num">3.4</span> AIO Targets {#aio-targets}

除了上述提到的完成回调外，每个 AIO 句柄都有一个“目标”。每个目标在 AIO 句柄内有一些空间，用于存储特定于目标的信息，并可以提供回调，以允许重新打开底层文件（在工作模式下所需）并描述 IO 操作（用于调试日志和错误消息）。

也就是说，如果 AIO 的两个不同使用情况能够以相同的方式描述正在操作的文件的身份，那么使用相同的目标是有意义的。例如，不同的 smgr 实现可以通过 RelFileLocator、ForkNumber 和 BlockNumber 来描述 IO，因此可以共享一个目标。相对而言，WAL 文件的 IO 会通过 TimeLineID 和 XLogRecPtr 来描述，因此对于 smgr 和 WAL 使用同一个目标就没有意义。


### <span class="section-num">3.5</span> AIO Wait References {#aio-wait-references}

如上所述，AIO 句柄可以在完成后立即重用，因此不能用于等待 IO 的完成。等待是通过 AIO 等待引用来实现的，这些引用不仅标识了一个 AIO 句柄，还包括句柄的“生成”。

可以使用 `pgaio_io_get_wref()` 获取对 AIO 句柄的引用，然后使用 `pgaio_wref_wait()` 进行等待。


### <span class="section-num">3.6</span> AIO Results {#aio-results}

作为 AIO 完成回调在关键区域内执行并且可能由任何后端执行完成回调不能用于，例如，发出导致 IO 错误的查询。

为了能够对失败的 IO 作出反应，发起后端可以在后端本地内存中传递一个指向 `PgAioReturn` 的指针。在重用 AIO
句柄之前， `PgAioReturn` 将填充有关 IO 的信息。这包括关于 IO 是否成功的信息（作为 `PgAioResultStatus` 的值）以及足够的信息以便在失败时引发错误（通过 `pgaio_result_report()` ，错误详细信息编码在 `PgAioResult` 中）。


### <span class="section-num">3.7</span> AIO Errors {#aio-errors}

将共享完成回调编码错误的细节为一个 `ErrorData` ，以便稍后可以抛出，这将非常方便。不幸的是，这样做将需要分配内存。虽然 elog.c 可以保证（好吧，有点）记录消息不会耗尽内存，但这仅仅是因为正在记录的消息数量非常有限。随着 AIO 的使用，可能会有大量的并发 AIO 失败。

为了避免需要预分配大量可能的内存（更不用说在共享内存中！），完成回调必须以更紧凑的格式编码错误，这种格式可以转换为错误消息。


## <span class="section-num">4</span> Helpers {#helpers}

使用低级 AIO API 在整个树中引入了太多的复杂性。大多数 AIO 的使用应该通过可重用的更高级别的助手来完成。


### <span class="section-num">4.1</span> 读取流 {#读取流}

AIO 的一个常见而且非常有益的使用场景是读取，其中知道大量待读取位置。例如，对于顺序扫描，所需读取的块集可以仅通过知道当前位置并检查缓冲映射表来确定。

[读取流](../../../include/storage/read_stream.h) 接口使得在这种使用情况下比较容易使用 AIO。

