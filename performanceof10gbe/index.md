# Performance of 10 G Ethernet Using Commodity Hardware


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [INTRODUCTION](#h:ff062e17-a590-447b-af94-db765fe35d49)
- <span class="section-num">2</span> [NETWORK PROCESSING IN THE LINUX KERNEL](#h:79aa9e78-e621-4482-a37f-6c4e9faa51ed)
    - <span class="section-num">2.1</span> [Kernel Accounting](#h:ae958029-e093-4749-8e53-86bbf3c625c0)
    - <span class="section-num">2.2</span> [Packet Transmission 发送](#h:becafb3e-b9d7-42ce-a7f6-ef239ea75c5a)
    - <span class="section-num">2.3</span> [Packet Reception 接收](#h:89c95fde-f17c-4f48-a103-2c4f8a22c09c)
    - <span class="section-num">2.4</span> [Incorrect SoftIRQ Accounting](#h:9010c7b0-db9f-4169-8ec4-b8905c4dd54a)
- <span class="section-num">3</span> [TESTBED AND KERNEL CONFIGURATION](#h:aaaa327c-4cf8-4784-a699-88f940bd6e60)
- <span class="section-num">4</span> [UDP TRANSMISSION](#h:8168452b-6b7f-4d83-91b1-33a6df8d63fc)
- <span class="section-num">5</span> [TCP TRANSMISSION](#h:3d1a9962-b03b-481c-8098-aeaaad110f71)
- <span class="section-num">6</span> [TCP TRANSMISSION WITH ZERO-COPY](#h:b3dad9b5-5d55-4dbb-ad94-74e11ba80a91)
- <span class="section-num">7</span> [THE HARDWARE OFFLOAD FUNCTIONALITIES OF THE NETWORK ADAPTER](#h:566096d2-c034-451a-9844-517b5b69c0f0)
- <span class="section-num">8</span> [FIBRE CHANNEL TO 10 GIGABIT ETHERNET TESTS](#h:321ebff8-6056-49e5-9deb-bfc61ccb5011)
- <span class="section-num">9</span> [CONCLUSIONS](#h:77721e10-4994-4b5b-97c8-6d13ef405b68)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/3/performanceOf10GbE.pdf



## <span class="section-num">1</span> INTRODUCTION {#h:ff062e17-a590-447b-af94-db765fe35d49}

With the introduction of 10-GbE, network I/O re-entered the “fast network,
slow host” scenario that occurred with both the transitions to Fast Ethernet
and Gigabit Ethernet. Specifically, three major system bottlenecks may limit
the efficiency of high-performance I/O adapters:

-   PCI-X 总线带宽
    -   PCI-X 频率 `133MHz` ，带宽 `8.5Gb/s`
    -   已被 PCI-Express (PCIe) 替代：
        -   `8` 通道
        -   `20 Gb/s`

-   CPU 利用率
-   内存带宽


## <span class="section-num">2</span> NETWORK PROCESSING IN THE LINUX KERNEL {#h:79aa9e78-e621-4482-a37f-6c4e9faa51ed}


### <span class="section-num">2.1</span> Kernel Accounting {#h:ae958029-e093-4749-8e53-86bbf3c625c0}

-   **User**
    -   程序运行在用户态的时间

-   **System**
    -   程序运行在内核态的时间

-   **IRQ**
    -   CPU 处理硬件中断的时间

-   **SoftIRQ**
    -   CPU 处理软中断的时间


### <span class="section-num">2.2</span> Packet Transmission 发送 {#h:becafb3e-b9d7-42ce-a7f6-ef239ea75c5a}

-   发送队列
    -   每个网卡驱动维持一个数据包的发送队列
    -   内核根据 qdisc (queue discipline) 将数据包插入到队列中
        -   默认的 qdisc 为 `pfifo_fast` (paccket FIO)
        -   Linux 支持其他的策略, 如
            -   RED (Radom Early Drop)
            -   CBQ (Class Based Queuing)
            -   Others

-   link layer
    -   triggered by function `dev_queue_xmit()`, 该函数负责：
        -   将数据包根据 `qdisc` 插入到发送队列中
        -   从发送队列中取出待发送的包，调用驱动的发送函数 `hard_start_xmit()` 来发送
        -   如果因为某些原因 （如设备没有资源了），它会安排 SoftIRQ ， 然后晚些通过软中断再次发送

-   设备驱动
    -   设备驱动负责将数据从 `tx_ring` 中转移到网卡的缓冲区中
    -   该操作通过设置 DMA 映射，然后设置硬件上的特定寄存器来完成
    -   驱动无需等待发送完成：
        -   发送完成之后硬件发出硬中断 `DMAdone`
        -   内核收到中断后安排 SoftIRQ 来释放 packet 使用的内存 <br />
            释放内存操作耗时相对较长，不适合在硬中断中完成


### <span class="section-num">2.3</span> Packet Reception 接收 {#h:89c95fde-f17c-4f48-a103-2c4f8a22c09c}

接收从 NIC 开始。

-   NIC
    -   接收到一对 Ethernet frames
    -   Frames 存储在 `rx_ring`
        -   内核 reserved 空间中
        -   环形缓冲区
    -   网卡将接收到的数据放到 DMA 后，将中断信号发送给指定的 IRQ line
        -   中断控制器负责中断指定的处理器


### <span class="section-num">2.4</span> Incorrect SoftIRQ Accounting {#h:9010c7b0-db9f-4169-8ec4-b8905c4dd54a}


## <span class="section-num">3</span> TESTBED AND KERNEL CONFIGURATION {#h:aaaa327c-4cf8-4784-a699-88f940bd6e60}

-   测试机配置和网络拓扑：

    <a id="figure--fig:screenshot@2022-10-21-13:31:24"></a>

    {{< figure src="/ox-hugo/screenshot@2022-10-21_13:31:24.png" >}}

-   内核参数：

    <a id="figure--fig:screenshot@2022-10-21-13:35:13"></a>

    {{< figure src="/ox-hugo/screenshot@2022-10-21_13:35:13.png" >}}

    -   `net.core.rmem_max` ， `net.core.wmem_max` <br />
        接收端和发送端 **socket** buffer size

    -   `net.ipv4.tcp_rmem` ， `net.ipv4.tcp_wmem`  <br />
        接收端和发送端 **tcp** buffer size (min, default, max)

    -   `net.core.netdev_max_backlog` <br />
        控制软中断函数 `net_rx_action()` 每次处理的数据包的个数


## <span class="section-num">4</span> UDP TRANSMISSION {#h:8168452b-6b7f-4d83-91b1-33a6df8d63fc}

<a id="figure--fig:screenshot@2022-10-21-13:44:24"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-21_13:44:24.png" >}}

-   两个发送进程，MTU 9K 时候性能最好
-   MTU 9K 时候， packet size 8K 以上可以跑满


## <span class="section-num">5</span> TCP TRANSMISSION {#h:3d1a9962-b03b-481c-8098-aeaaad110f71}

-   shows the data transfer rate, measured as a function of the TCP send
    size. For a MTU of 1500 B, the maximum throughput achieved was around 5.5
    Gb/s, reached at the max- imum tested send size of 64 KiB. The throughput
    decreased as decreased the send size, with a change in the slope at 1500
    B. The adoption of the 9000 B MTU with TCP improved the throughput up to 7
    Gb/s.

    <a id="figure--fig:screenshot@2022-10-21-13:53:04"></a>

    {{< figure src="/ox-hugo/screenshot@2022-10-21_13:53:04.png" >}}


## <span class="section-num">6</span> TCP TRANSMISSION WITH ZERO-COPY {#h:b3dad9b5-5d55-4dbb-ad94-74e11ba80a91}

-   `sendfile()` 来省略在内核态与用户态之间的数据拷贝
-   `5.5 Gb/s` -&gt;  `8Gb/s`


## <span class="section-num">7</span> THE HARDWARE OFFLOAD FUNCTIONALITIES OF THE NETWORK ADAPTER {#h:566096d2-c034-451a-9844-517b5b69c0f0}

offload function: 内核可以将高负载的任务交给硬件去做。

-   TCP Segmentation Offload, TSO
    -   工作在发送端
    -   当 TCP 的数据包大小超过 MTU 时候，必须进行分片操作
    -   不支持 TSO 的硬件，必须由内核在 TPC 层完成
    -   支持 TSO 的硬件，则可将最大 `64K` 的数据一次性交给网卡，由网卡来完成分片

-   Large Receive Offload， LRO
    -   工作在接收端
    -   在 NIC 层将多个 TCP packets 重组成更大的数据包

-   Scatter-Gather (SG) I/O
    -   可以将不连续的内存地址通过 DMA 映射，减少内存拷贝

-   Checksum Offload, CO
    -   TCP 包的 checksum 计算

这些 offload 的效果:

-   吞吐量结果：

    <a id="figure--fig:screenshot@2022-10-21-17:21:40"></a>

    {{< figure src="/ox-hugo/screenshot@2022-10-21_17:21:40.png" >}}

-   CPU 负载：

    <a id="figure--fig:screenshot@2022-10-21-17:24:28"></a>

    {{< figure src="/ox-hugo/screenshot@2022-10-21_17:24:28.png" width="800px" >}}


## <span class="section-num">8</span> FIBRE CHANNEL TO 10 GIGABIT ETHERNET TESTS {#h:321ebff8-6056-49e5-9deb-bfc61ccb5011}


## <span class="section-num">9</span> CONCLUSIONS {#h:77721e10-4994-4b5b-97c8-6d13ef405b68}

1.  MTU matters
2.  Offload matters&#x2026;

