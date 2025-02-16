# Linux Asynchronous I/O


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#introduction)
- <span class="section-num">2</span> [I/O Models](#i-o-models)
- <span class="section-num">3</span> [AIO System Calls](#aio-system-calls)
    - <span class="section-num">3.1</span> [ABI Interface](#abi-interface)

</div>
<!--endtoc-->


本文为摘录，原文为： https://oxnz.github.io/2016/10/13/linux-aio/



## <span class="section-num">1</span> Introduction {#introduction}

异步 I/O（AIO）是一种执行 I/O 操作的方法，使得发出 I/O 请求的进程在操作完成之前不被阻塞。相反，在提交 I/O 请求后，
该进程继续执行其代码，并可以随后检查所提交请求的状态。

在 Linux 中实现异步 I/O 有几种方式：

-   内核系统调用
-   用户空间库的实现并在内部使用系统调用（libaio）
-   在用户空间完全模拟 AIO 而不依赖任何内核支持（目前为止，使用 librt，属于 libc 的一部分）


## <span class="section-num">2</span> I/O Models {#i-o-models}

| Mode         | Blocking                  | Non-blocking                  |
|--------------|---------------------------|-------------------------------|
| Synchronous  | read/write                | read/write `(O_NONBLOCK)`     |
| Asynchronous | I/O `(select/poll/epoll)` | multiplexing              AIO |


## <span class="section-num">3</span> AIO System Calls {#aio-system-calls}


### <span class="section-num">3.1</span> ABI Interface {#abi-interface}

AIO 系统调用入口点位于内核源代码中的 `fs/aio.c` 文件中。导出到用户空间的类型和常数位于 `/usr/include/linux/aio_abi.h`
头文件中。

Linux 内核仅提供了 5 个用于执行异步 I/O 的系统调用。

```c
#include <linux/aio_abi.h>
int io_setup(unsigned nr_events, aio_context_t *ctxp);
int io_destroy(aio_context_t ctx);
int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp);
int io_cancel(aio_context_t ctx, struct iocb *, struct io_event *result);
int io_getevents(aio_context_t ctx, long min_nr, long nr,
    struct io_event *events, struct timespec *timeout);

```

-   `struct iocb`: 每个提交到 AIO 上下文的 I/O 请求都由一个 I/O 控制块结构表示 - `struct iocb`
-   `io_submit()` 接受 AIO 上下文 ID、数组的大小和数组本身作为参数。 <br />
    需要注意的是，数组应包含指向 iocb 结构的指针，而不是结构本身。

`io_submit()` 的返回代码可以是以下值之一：

1.  `ret = （已提交的 iocbs 数量）` <br />

    理想情况，所有 iocbs 都被接受并正在处理。

2.  `0 < ret < （已提交的 iocbs 数量）` <br />

    `io_submit()` 系统调用会逐个处理传递数组中的 iocbs，从第一个条目开始。如果某个 iocb 的提交失败，系统会在这一
    点停止并返回失败的 iocb 的索引。无法知道具体的失败原因。然而，如果第一个 iocb 的提交失败，请参考点 C。

3.  `ret < 0`

    出现这种情况有两个原因：

    1.  在 `io_submit()` 开始迭代数组中的 iocbs 之前，出现了某些错误（例如，AIO 上下文无效）。
    2.  第一个 iocb (cbx[0]) 的提交失败。

