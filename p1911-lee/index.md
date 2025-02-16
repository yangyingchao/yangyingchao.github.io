# Your Read is Our Priority in Flash Storage


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:ebffcd16-173e-4644-a00f-8f6d92c798f0)
- <span class="section-num">2</span> [INTRODUCTION](#h:f8ebb4bf-cdee-4786-920f-9e5cca36c7ca)
- <span class="section-num">3</span> [BACKGROUND](#h:3fd376b5-2ff5-4320-b346-b0e2e679ba7f)
    - <span class="section-num">3.1</span> [I/O Asymmetry in Flash SSDs](#h:d5416d09-f7a7-4448-b41e-c5f56ce6dc40)
    - <span class="section-num">3.2</span> [RAW Protocol in DBMS Buffer](#h:5a068019-5346-450f-b2b9-bb52b0395dfe)
    - <span class="section-num">3.3</span> [RAW Protocol in Storage Buffer](#h:b1ad1102-62a1-4ebf-a3d0-5175fb229e15)
- <span class="section-num">4</span> [READ STALL IN DBMS AND RW COMMAND](#h:120954ae-7c0b-4356-9db3-2603f1c95561)
    - <span class="section-num">4.1</span> [Read Stalls inRelational DBMS Buffer](#h:1961aec8-3148-4626-8ea7-d7aa8e3630c7)
    - <span class="section-num">4.2</span> [RW Command](#h:f4ca1bee-0c63-49d3-ad37-d8b7ba336e23)
- <span class="section-num">5</span> [READ STALL IN STORAGE AND R-BUF](#h:83365df9-93ca-4225-93ac-036eb29a80cf)
    - <span class="section-num">5.1</span> [Read Stalls in SSD Buffer](#h:4af9a557-45e7-494e-8d5b-540929258590)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/e/p1911-lee.pdf



## <span class="section-num">1</span> ABSTRACT {#h:ebffcd16-173e-4644-a00f-8f6d92c798f0}

-   问题： **读阻塞**
    -   buffer manager 采用 read-after-write (RAW) 策略
    -   读写速率不同，导致 **读阻塞**
    -   RAW 分两层：
        -   DBMS buffer
        -   Storage buffer （硬件层）

-   方案：
    -   RW as new storage interface:
        -   fused read and write, RW, 读写融合
        -   buffermgr 可同时发出读写请求
        -   脏页拷贝到存储 buffer 之后 马上 read

    -   R-Buf
        -   分离读写 buffer

-   效果
    -   RW: tpcc -&gt; 3.2x
    -   RW + R-Buf: tpcc -&gt; 3.9x


## <span class="section-num">2</span> INTRODUCTION {#h:f8ebb4bf-cdee-4786-920f-9e5cca36c7ca}


## <span class="section-num">3</span> BACKGROUND {#h:3fd376b5-2ff5-4320-b346-b0e2e679ba7f}


### <span class="section-num">3.1</span> I/O Asymmetry in Flash SSDs {#h:d5416d09-f7a7-4448-b41e-c5f56ce6dc40}

-   SSD 读写非对称：
    -   读快，写慢
        -   MLC 闪存，写一个页面需要 `1500us` ，而读仅需 `50us`
    -   闪存的 GC 进一步加剧了这一现象
    -   下表为一个测试结果 （FIO）
        -   [OpenSSD](https://en.wikipedia.org/wiki/Open-channel_SSD)
            -   is a solid-state drive which does not have a firmware Flash Translation Layer implemented on the device
            -   relies on OS to manage of the physical solid-state storage
            -   more flexibility with regard to:
                1.  data placement decisions,
                2.  overprovisioning,
                3.  scheduling,
                4.  garbage collection and
                5.  wear leveling

<a id="figure--fig:screenshot@2022-10-14-18:12:49"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-14_18:12:49.png" >}}


### <span class="section-num">3.2</span> RAW Protocol in DBMS Buffer {#h:5a068019-5346-450f-b2b9-bb52b0395dfe}


### <span class="section-num">3.3</span> RAW Protocol in Storage Buffer {#h:b1ad1102-62a1-4ebf-a3d0-5175fb229e15}

-   S-Buf
    -   Shared buffer, for both read &amp; write

<!--listend-->

-   SSD architecture

    <a id="figure--fig:screenshot@2022-10-17-10:45:59"></a>

    {{< figure src="/ox-hugo/screenshot@2022-10-17_10:45:59.png" width="800px" >}}


## <span class="section-num">4</span> READ STALL IN DBMS AND RW COMMAND {#h:120954ae-7c0b-4356-9db3-2603f1c95561}


### <span class="section-num">4.1</span> Read Stalls inRelational DBMS Buffer {#h:1961aec8-3148-4626-8ea7-d7aa8e3630c7}

-   the read stall problem can be alleviated or even disappear with a large buffer
    -   PG: 会下降的更多： 61%


#### <span class="section-num">4.1.1</span> Problem Definition {#h:1a5ca1de-d6a6-486c-8980-c5f1c608985d}

-   数据库 buffer 层的 **write-then-read** ，阻挡了使用底层异步并行优化 （libaio or io_uring）


### <span class="section-num">4.2</span> RW Command {#h:f4ca1bee-0c63-49d3-ad37-d8b7ba336e23}


#### <span class="section-num">4.2.1</span> Key Idea. {#h:83e15275-818a-4f1a-84e7-f9f7dee84b52}

new block I/O command:

`RW` write a dirty page to storage, and in parallel, read the missingpage to the host in on I/O call.


#### <span class="section-num">4.2.2</span> Abstraction and Architecture. {#h:bc88385d-6ad1-455f-a85f-5e51c84924c0}

-   当前存储接口中不存在这样的命令， 作者将 `RW` 作为 NVME vendor specific command 添加

-   rw(rdLBA, wrLBA, len, buf)
    -   rdLBA/wrLBA: 读写操作的逻辑块地址
    -   len: 两个页面的大小 （假设相等）
    -   buf: host buffer 虚地址

-   当主机向控制器发出 RW 命令后， NVME Controller 并行处理 READ 和 WRITE
    -   将与 wrLBA 相关的脏页拷贝进控制器缓存 （storage buffer）
    -   马上读取 rdLBA

<a id="figure--fig:screenshot@2022-10-17-17:16:36"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-17_17:16:36.png" >}}

-   好处
    -   存储控制器内部可以并行
    -   减少 SYSCALL 和 IO 中断

<!--listend-->

-   Consistency and Durability


#### <span class="section-num">4.2.3</span> Prototype Implementation {#h:ce460c46-a392-475e-a5c0-887d22858026}

向 OpenSSD 添加了新的 RW 命令，并扩展固件代码来支持这一语义。

-   Changes in OpenSSD
    -   使用 OpenSSD
        -   OpenSSD 可以自由修改软件和硬件设计
    -   使用了 Comos+ board, 该板支持 NVMe 接口
    -   定义了新的 opcode
    -   固件上实现了自定义操作
    -   固件的读写地址不同，可以利用 SSD 的多通道特性来实现并行

-   Changes in MySQL
    -   直接使用 VFS 接口的话，不会有提升
    -   扩展 `ioctl`
        -   使之可以掠过 VFS 直接发送 RW 命令

    -   修改了 MySQL 的 buffer manager
        -   添加新的 IO 函数，  buffer manager 可通过该函数来：
            -   获取 read write 的 LBA
            -   发送 RW 命令
        -   修改了 MySQL 的 read 函数：
            -   read 返回后当前事务可以直接进行，而无需再申请 buffer <br />
                （通过操作 LRU list）


## <span class="section-num">5</span> READ STALL IN STORAGE AND R-BUF {#h:83365df9-93ca-4225-93ac-036eb29a80cf}


### <span class="section-num">5.1</span> Read Stalls in SSD Buffer {#h:4af9a557-45e7-494e-8d5b-540929258590}


#### <span class="section-num">5.1.1</span> Problem Definition {#h:df362158-d5d2-4a73-ad45-dd3ea3498de3}

