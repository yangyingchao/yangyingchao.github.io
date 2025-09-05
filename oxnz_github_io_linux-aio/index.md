# Linux Asynchronous I/O


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#introduction)
- <span class="section-num">2</span> [I/O Models](#i-o-models)
- <span class="section-num">3</span> [AIO System Calls](#aio-system-calls)
    - <span class="section-num">3.1</span> [ABI Interface](#abi-interface)
    - <span class="section-num">3.2</span> [struct io_event](#struct-io-event)
    - <span class="section-num">3.3</span> [struct iocb](#struct-iocb)
    - <span class="section-num">3.4</span> [AIO Command](#aio-command)
    - <span class="section-num">3.5</span> [AIO Context](#aio-context)
    - <span class="section-num">3.6</span> [syscall()](#syscall)
    - <span class="section-num">3.7</span> [Example](#example)
- <span class="section-num">4</span> [libaio](#libaio)
    - <span class="section-num">4.1</span> [安装](#安装)
    - <span class="section-num">4.2</span> [系统调用包装器](#系统调用包装器)
    - <span class="section-num">4.3</span> [辅助函数](#辅助函数)
    - <span class="section-num">4.4</span> [接口](#接口)
- <span class="section-num">5</span> [参考资料](#参考资料)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://oxnz.github.io/2016/10/13/linux-aio/



## <span class="section-num">1</span> Introduction {#introduction}

异步 I/O（AIO）是一种执行 I/O 操作的方法，使得发出 I/O 请求的进程在操作完成之前不被阻塞。相反，在提交 I/O 请求后，该进程继续执行其代码，并可以随后检查所提交请求的状态。

在 Linux 中实现异步 I/O 有几种方式：

-   内核系统调用
-   用户空间库的实现并在内部使用系统调用（libaio）
-   在用户空间完全模拟 AIO 而不依赖任何内核支持（例如使用 librt，属于 libc 的一部分）


## <span class="section-num">2</span> I/O Models {#i-o-models}

<a id="table--tbl:oxnz-6ee86d96"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl:oxnz-6ee86d96">Table 1</a>:</span>
  IO 模型
</div>

| Mode         || Blocking                  || Non-blocking                  |
|--------------|---------------------------|-------------------------------|
| Synchronous  || read/write                || read/write `(O_NONBLOCK)`     |
| Asynchronous || I/O `(select/poll/epoll)` || multiplexing              AIO |


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

    `io_submit()` 系统调用会逐个处理传递数组中的 iocbs，从第一个条目开始。如果某个 iocb 的提交失败，系统会在这一点停止并返回失败的 iocb 的索引。无法知道具体的失败原因。然而，如果第一个 iocb 的提交失败，请参考点 C。

3.  `ret < 0`

    出现这种情况有两个原因：

    1.  在 `io_submit()` 开始迭代数组中的 iocbs 之前，出现了某些错误（例如，AIO 上下文无效）。
    2.  第一个 iocb (cbx[0]) 的提交失败。

在提交 iocb 后，我们可以在不等待 I/O 完成的情况下执行其他操作。每个完成的 I/O 请求（成功或失败）都会由内核创建一个 io_event 结构。要获取 io_events 的列表（从而获取所有已完成的 iocb），应使用 `io_getevent()` 系统调用。调用 `io_getevents()` 时，需要指定：

1.  从哪个 AIO 上下文获取事件（ctx 变量）
2.  内核应该将事件加载到的缓冲区（events 变量）
3.  想要获取的最小事件数量。 <br />
    如果当前完成的 iocbs 数量少于这个数字，io_getevents()将阻塞，直到足够的事件出现。有关如何控制阻塞时间的更多细节，请参见 e)点。

4.  想要获取的最大事件数量。这通常是事件缓冲区的大小（在我们程序中的第二个 1）

5.  如果可用事件数量不足，我们不想永远等待。可以将相对截止日期指定为最后一个参数。在这种情况下，NULL
    意味着无限等待。如果想要 io_getevents()根本不阻塞，则需要将 timespec 超时结构初始化为零秒和零纳秒。

`io_getevents` 的返回代码可以是：

1.  `ret = (最大事件数量)`

    所有适合用户提供的缓冲区的事件都从内核获得。内核中可能还有更多待处理事件。

2.  `(最小事件数量) <` ret &lt;= (最大事件数量)=

    所有当前可用事件已从内核读取，且没有发生阻塞。

3.  `0 < ret < (最小事件数量)`

    所有当前可用事件已从内核读取，我们阻塞以等待用户指定的时间。

4.  `ret = 0`

    没有可用事件 XXX:? 在这种情况下是否发生阻塞？..

5.  `ret < 0`

    发生了一个错误。


### <span class="section-num">3.2</span> struct io_event {#struct-io-event}

```c
/* read() from /dev/aio returns these structures. */
struct io_event {
        __u64           data;           /* the data field from the iocb */
        __u64           obj;            /* what iocb this event came from */
        __s64           res;            /* result code for this event */
        __s64           res2;           /* secondary result */
};
```


### <span class="section-num">3.3</span> struct iocb {#struct-iocb}

```c
/*
 * we always use a 64bit off_t when communicating
 * with userland.  its up to libraries to do the
 * proper padding and aio_error abstraction
 */

struct iocb {
        /* these are internal to the kernel/libc. */
        __u64   aio_data;       /* data to be returned in event's data */
        __u32   PADDED(aio_key, aio_reserved1);
                                /* the kernel sets aio_key to the req # */

        /* common fields */
        __u16   aio_lio_opcode; /* see IOCB_CMD_ above */
        __s16   aio_reqprio;
        __u32   aio_fildes;

        __u64   aio_buf;
        __u64   aio_nbytes;
        __s64   aio_offset;

        /* extra parameters */
        __u64   aio_reserved2;  /* TODO: use this for a (struct sigevent *) */

        /* flags for the "struct iocb" */
        __u32   aio_flags;

        /*
         * if the IOCB_FLAG_RESFD flag of "aio_flags" is set, this is an
         * eventfd to signal AIO readiness to
         */
        __u32   aio_resfd;
}; /* 64 bytes */
```


### <span class="section-num">3.4</span> AIO Command {#aio-command}

```c
# /usr/include/linux/aio_abi.h
enum {
        IOCB_CMD_PREAD = 0,
        IOCB_CMD_PWRITE = 1,
        IOCB_CMD_FSYNC = 2,
        IOCB_CMD_FDSYNC = 3,
        /* These two are experimental.
         * IOCB_CMD_PREADX = 4,
         * IOCB_CMD_POLL = 5,
         */
        IOCB_CMD_NOOP = 6,
        IOCB_CMD_PREADV = 7,
        IOCB_CMD_PWRITEV = 8,
};
```

-   `IOCB_CMD_PREAD` <br />
    定位读取；对应于 pread() 系统调用。

-   `IOCB_CMD_PWRITE` <br />
    定位写入；对应于 pwrite() 系统调用。

-   `IOCB_CMD_FSYNC` <br />
    将文件的数据和元数据与磁盘同步；对应于 fsync() 系统调用。

-   `IOCB_CMD_FDSYNC` <br />
    将文件的数据和元数据与磁盘同步，但仅写入访问已修改文件数据所需的元数据；对应于 fdatasync() 系统调用。

-   `IOCB_CMD_PREADV` <br />
    向量化定位读取，有时称为“分散输入”；对应于 preadv() 系统调用。

-   `IOCB_CMD_PWRITEV` <br />
    向量化定位写入，有时称为“聚集输出”；对应于 pwritev() 系统调用。

-   `IOCB_CMD_NOOP` <br />
    在头文件中定义，但在内核的其他地方未使用。

iocb 结构中其他字段的语义依赖于指定的命令。


### <span class="section-num">3.5</span> AIO Context {#aio-context}

AIO 上下文是一组数据结构，内核支持这些结构以执行 AIO。

每个进程可以拥有多个 AIO 上下文，因此每个进程中的每个 AIO 上下文都需要一个标识符。

一个指向 ctx 变量的指针作为第二个参数传递给 `io_setup()` ，内核用一个上下文标识符填充这个变量。有趣的是，
`aio_context_t` 实际上只是内核中定义的一个无符号长整型（linux/aio_abi.h），定义如下：

```c
typedef unsigned long aio_context_t;
```

`io_setup()` 函数的第一个参数是可以同时存在于上下文中的最大请求数量。


### <span class="section-num">3.6</span> syscall() {#syscall}

man syscall

```text
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

int syscall(int number, ...);

syscall() 是一个小型库函数，它根据指定的编号和参数调用具有汇编语言接口的系统调用。例如，当调用 C
库中没有包装函数的系统调用时，使用 syscall() 是很有用的。

syscall() 在进行系统调用之前保存 CPU 寄存器，系统调用返回时恢复寄存器，并且如果发生错误，
将系统调用返回的任何错误代码存储在 errno(3) 中。

系统调用号的符号常量可以在头文件 <sys/syscall.h> 中找到。

```


### <span class="section-num">3.7</span> Example {#example}

```c
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/aio_abi.h>

inline int io_setup(unsigned nr, aio_context_t *ctxp) {
        return syscall(__NR_io_setup, nr, ctxp);
}

inline int io_destroy(aio_context_t ctx) {
        return syscall(__NR_io_destroy, ctx);
}

inline int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) {
        return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
                struct io_event *events, struct timespec *timeout) {
        return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

int main(int argc, char *argv[]) {
        aio_context_t ctx;
        struct iocb cb;
        struct iocb *cbs[1];
        char data[4096];
        struct io_event events[1];
        int ret;
        int fd;

        fd = open("/tmp/test", O_RDWR | O_CREAT);
        if (fd < 0) {
                perror("open");
                return -1;
        }

        ctx = 0;

        ret = io_setup(128, &ctx);
        if (ret < 0) {
                perror("io_setup");
                return -1;
        }

        /* setup I/O control block */
        memset(&cb, 0, sizeof(cb));
        cb.aio_fildes = fd;
        cb.aio_lio_opcode = IOCB_CMD_PWRITE;

        /* command-specific options */
        int i;
        for (i = 0; i < 4096; ++i)
                data[i] = 'A';
        cb.aio_buf = (uint64_t)data;
        cb.aio_offset = 0;
        cb.aio_nbytes = 4096;

        cbs[0] = &cb;

        ret = io_submit(ctx, 1, cbs);
        if (ret != 1) {
                if (ret < 0) perror("io_submit");
                else fprintf(stderr, "io_submit failed\n");
                return -1;
        }

        /* get reply */
        ret = io_getevents(ctx, 1, 1, events, NULL);
        printf("events: %d\n", ret);
        ret = io_destroy(ctx);
        if (ret < 0) {
                perror("io_destroy");
                return -1;
        }
        return 0;
}
```


## <span class="section-num">4</span> libaio {#libaio}


### <span class="section-num">4.1</span> 安装 {#安装}

```sh
[oxnz@localhost aio]$ sudo yum install libaio-devel
[oxnz@localhost aio]$ rpm -ql libaio
/lib64/libaio.so.1
/lib64/libaio.so.1.0.0
/lib64/libaio.so.1.0.1
/usr/share/doc/libaio-0.3.109
/usr/share/doc/libaio-0.3.109/COPYING
/usr/share/doc/libaio-0.3.109/TODO
[oxnz@localhost aio]$ rpm -ql libaio-devel
/usr/include/libaio.h
/usr/lib64/libaio.so
```


### <span class="section-num">4.2</span> 系统调用包装器 {#系统调用包装器}

```c
// /usr/include/libaio.h //
// 实际系统调用 //
extern int io_setup(int maxevents, io_context_t /ctxp);
extern int io_destroy(io_context_t ctx);
extern int io_submit(io_context_t ctx, long nr, struct iocb /ios[]);
extern int io_cancel(io_context_t ctx, struct iocb /iocb, struct io_event /evt);
extern int io_getevents(io_context_t ctx_id, long min_nr, long nr, struct io_event /events, struct timespec /timeout);
```


### <span class="section-num">4.3</span> 辅助函数 {#辅助函数}

```text
/lib64/librt.so
/usr/include/aio.h
```


### <span class="section-num">4.4</span> 接口 {#接口}

POSIX AIO 接口包含以下函数：

-   aio_read(3) 将读取请求加入队列。这是读取操作的异步版本。
-   aio_write(3) 将写入请求加入队列。这是写入操作的异步版本。
-   aio_fsync(3) 对文件描述符上的 I/O 操作加入同步请求。这是 fsync 和 fdatasync 的异步版本。
-   aio_error(3) 获取已加入队列的 I/O 请求的错误状态。
-   aio_return(3) 获取已完成 I/O 请求的返回状态。
-   aio_suspend(3) 暂停调用者，直到一组特定的 I/O 请求完成。
-   aio_cancel(3) 尝试取消在特定文件描述符上的未处理 I/O 请求。
-   lio_listio(3) 使用单个函数调用入队多个 I/O 请求。

man 7 aio

当前 Linux 的 POSIX AIO 实现是由 glibc 在用户空间提供的。这个实现有许多限制，最显著的是维护多个线程以执行 I/O
操作是昂贵且扩展性差的。已经有一段时间进行内核状态机基础的异步 I/O 实现的工作（参见 io_submit(2)，io_setup(2)，
io_cancel(2)，io_destroy(2)，io_getevents(2)），但该实现尚未成熟到能够完全使用内核系统调用重新实现 POSIX AIO
实现的程度。


## <span class="section-num">5</span> 参考资料 {#参考资料}

-   <http://www.fsl.cs.sunysb.edu/~vass/linux-aio.txt>
-   通过异步 I/O 提升应用性能

