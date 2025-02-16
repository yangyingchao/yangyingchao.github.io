# Coping with the TCP TIME-WAIT state on busy Linux servers


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [TL;DR](#h:871f360d-063e-46ff-9c0a-18322d91be4c)
- <span class="section-num">2</span> [About the TIME-WAIT state](#h:096c1a95-20d2-4ac1-b6f2-fac0a6b2e233)
    - <span class="section-num">2.1</span> [TCP state diagram](#h:0a4a593c-1032-4cbd-bb0d-89c43f6348c0)
    - <span class="section-num">2.2</span> [Purpose](#h:761d930e-9258-42e1-8792-75aafd6a4642)
- <span class="section-num">3</span> [Problems](#h:90387b11-15a7-43e6-9d48-031298e3c45a)
- <span class="section-num">4</span> [Other solutions](#h:fe22875f-822e-470b-a2cf-8ca13e804920)
    - <span class="section-num">4.1</span> [net.ipv4.tcp_tw_reuse](#h:27aabe51-3f44-40ac-a21e-f9de9dbb9254)
- <span class="section-num">5</span> [Another](#h:27aabe51-3f44-40ac-a21e-f9de9dbb9254)
- <span class="section-num">6</span> [总结](#h:d0c1d159-905d-4598-a15c-ffb80f1d452d)

</div>
<!--endtoc-->


本文为摘录，原文为： https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#summary



## <span class="section-num">1</span> [TL;DR](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#summary%23:~:text=TL;DR) {#h:871f360d-063e-46ff-9c0a-18322d91be4c}

-   不用启用 `net.ipv4.tcp_tw_recycle` ， 该选项已在 Linux4.12 废弃
-   多数情况下， `TIME-WAIT` 状态的 socket 无害


## <span class="section-num">2</span> [About the TIME-WAIT state](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#about-the-time-wait-state) {#h:096c1a95-20d2-4ac1-b6f2-fac0a6b2e233}


### <span class="section-num">2.1</span> [TCP state diagram](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#summary%23:~:text=TCP%20state%20diagram) {#h:0a4a593c-1032-4cbd-bb0d-89c43f6348c0}

{{< figure src="/ox-hugo/tcp-state-diagram-v2.svg" width="727px" >}}

{{< figure src="/ox-hugo/tcpfsm.png" width="562px" >}}

<http://tcpipguide.com/free/t_TCPOperationalOverviewandtheTCPFiniteStateMachineF-2.htm>

-   只有主动关闭连接一端会进入 `TIME-WAIT` 状态
-   另外一端通常会走另外一条路径，快速关闭连接
-   可以通过命令 `ss -tan`  来查看状态
    ```console
    $ ss -tan | head -5
    LISTEN     0  511             *:80              *:*
    SYN-RECV   0  0     192.0.2.145:80    203.0.113.5:35449
    SYN-RECV   0  0     192.0.2.145:80   203.0.113.27:53599
    ESTAB      0  0     192.0.2.145:80   203.0.113.27:33605
    TIME-WAIT  0  0     192.0.2.145:80   203.0.113.47:50685
    ```


### <span class="section-num">2.2</span> [Purpose](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#purpose) {#h:761d930e-9258-42e1-8792-75aafd6a4642}


## <span class="section-num">3</span> [Problems](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#problems) {#h:90387b11-15a7-43e6-9d48-031298e3c45a}


## <span class="section-num">4</span> [Other solutions](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#other-solutions) {#h:fe22875f-822e-470b-a2cf-8ca13e804920}


### <span class="section-num">4.1</span> [net.ipv4.tcp_tw_reuse](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux#netipv4tcp_tw_reuse) {#h:27aabe51-3f44-40ac-a21e-f9de9dbb9254}


## <span class="section-num">5</span> Another {#h:27aabe51-3f44-40ac-a21e-f9de9dbb9254}


## <span class="section-num">6</span> [总结](https://draveness.me/whys-the-design-tcp-time-wait/#:~:text=%E6%80%BB%E7%BB%93) {#h:d0c1d159-905d-4598-a15c-ffb80f1d452d}

在某些场景下，60 秒的等待销毁时间确实是难以接受的，例如：高并发的压力测试。当我们通过并发请
求测试远程服务的吞吐量和延迟时，本地就可能产生大量处于 TIME_WAIT 状态的 TCP 连接，在 macOS
上可以使用如下所示的命令查看活跃的连接：

