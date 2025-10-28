# NMap_ Port Scanning Techniques and Algorithms


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#introduction)
- <span class="section-num">2</span> [TCP SYN (Stealth) Scan (-sS) 半开放扫描](#tcp-syn--stealth--scan--ss--半开放扫描)
    - <span class="section-num">2.1</span> [开放端口的扫描](#开放端口的扫描)
    - <span class="section-num">2.2</span> [关闭端口的扫描](#关闭端口的扫描)
    - <span class="section-num">2.3</span> [过滤掉的端口的扫描](#过滤掉的端口的扫描)
- <span class="section-num">3</span> [TCP Connect Scan (-sT)](#tcp-connect-scan--st)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://nmap.org/book/scan-methods.html



## <span class="section-num">1</span> Introduction {#introduction}

上一章对 Nmap 进行端口扫描的内容进行了总体描述，包括在“选择扫描技术”部分中对 Nmap 支持的扫描类型的简要概述。本章将深入描述这些扫描类型。每种扫描类型都提供了典型的使用场景和说明，同时还附有网络数据包跟踪示例，展示它们的工作原理。接着，将讨论超扫描算法（大多数扫描方法使用的算法），重点是可以调整以提高性能的各个方面。

大多数扫描类型仅对特权用户可用。这是因为它们发送和接收原始 IP 数据包（甚至以太网帧），这在 Unix 系统上需要 root 权限。在 Windows 上不需要使用管理员账户，因为在 Npcap 已经加载到操作系统中时，Nmap 可以在该平台上为非特权用户工作。当 Nmap
在 1997 年发布时，要求 root 权限是一个严重的限制，因为许多用户只能访问共享的 shell 账户。现在，情况有所不同。计算机变得更加便宜，更多的人拥有始终在线的直接互联网接入，桌面 Unix 系统（包括 Linux 和 Mac OS X）变得普遍。现在提供了 Nmap 的 Windows 版本，使其可以在更多的桌面上运行。由于这些原因，用户很少需要在有限的共享 shell
账户中运行 Nmap。这是幸运的，因为特权选项使 Nmap 变得更加强大和灵活。

在讨论 Nmap 如何处理探针响应时，许多章节通过类型和代码号码讨论 ICMP 错误消息。类型和代码各是 ICMP 头中的八位字段，用于描述消息的目的。Nmap 的端口扫描技术只关注 ICMP 类型 3，这是一种不可达目的地消息。图 5.1 展示了此类数据包的 ICMP
头布局（它封装在 IP 数据包的数据部分中，如图 1“IPv4 头”所示）。

<a id="figure--fig:nmap-f121206b"></a>

{{< figure src="/ox-hugo/icmp-7c4eb651.png" caption="<span class=\"figure-number\">Figure 1: </span>ICMPv4 destination unreachable header layout" >}}

有十六个代码表示不同的不可达目的地消息。所有这些代码在表 5.1 中列出，不过 Nmap 只关注代码 0–3、9、10 和 13，这些代码用星号标记。

<a id="table--tbl:nmap-6ee86d96"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl:nmap-6ee86d96">Table 1</a>:</span>
  ICMP destination unreachable (type 3) code values
</div>

| **代码** || **描述**        |
|--------|---------------|
| 0/     || 网络不可达      |
| 1/     || 主机不可达      |
| 2/     || 协议不可达      |
| 3/     || 端口不可达      |
| 4      || 需要分片但设置了不分片位 |
| 5      || 源路线失败      |
| 6      || 目标网络未知    |
| 7      || 目标主机未知    |
| 8      || 源主机被隔离（过时） |
| 9/     || 目标网络因管理原因被禁止 |
| 10/    || 目标主机因管理原因被禁止 |
| 11     || 类型服务（TOS）不可达 |
| 12     || TOS 不可达主机  |
| 13/    || 由于过滤，通信因管理原因被禁止 |
| 14     || 主机优先级违规  |
| 15     || 优先级截止生效  |


## <span class="section-num">2</span> TCP SYN (Stealth) Scan (-sS) 半开放扫描 {#tcp-syn--stealth--scan--ss--半开放扫描}

SYN 扫描是默认且最受欢迎的扫描选项。它可以快速执行，在没有侵入式防火墙干扰的快速网络上每秒扫描成千上万个端口。
SYN 扫描相对不引人注目且隐蔽， **因为它从不完成 TCP 连接** 。它也适用于任何符合标准的 TCP 栈，而不依赖于特定平台的特异性，正如 Nmap 的 FIN/NULL/Xmas、Maimon 和空闲扫描那样。它还允许清晰、可靠地区分开放、关闭和过滤状态。

SYN 扫描可以通过向 Nmap 传递 `-sS` 选项来请求。它需要原始数据包权限，并且在这些权限可用时，默认的 TCP
扫描就是它。因此，当以 root 或管理员身份运行 Nmap 时，通常省略 `-sS` 选项。这个默认的 SYN 扫描行为在示例 5.1
中展示，该示例找到每个三个主要状态中的一个端口。

```shell
krad# nmap -p22,113,139 scanme.nmap.org

Starting Nmap ( https://nmap.org )
Nmap scan report for scanme.nmap.org (64.13.134.52)
PORT    STATE    SERVICE
22/tcp  open     ssh
113/tcp closed   auth
139/tcp filtered netbios-ssn

Nmap done: 1 IP address (1 host up) scanned in 1.35 seconds
```

虽然 SYN 扫描在没有任何底层 TCP 知识的情况下使用起来相当简单，但理解这一技术在解释不寻常的结果时会有所帮助。


### <span class="section-num">2.1</span> 开放端口的扫描 {#开放端口的扫描}

首先，针对开放端口 22 的行为如图 [2](#figure--fig:nmap-28e943bb) 所示。

<a id="figure--fig:nmap-28e943bb"></a>

{{< figure src="/ox-hugo/Ereet_Packet_Trace_Syn_Open.png" caption="<span class=\"figure-number\">Figure 2: </span>SYN scan of open port 22" >}}

-   正如这个例子所示，Nmap 首先向端口 22 发送一个 SYN 标志位被设置的 TCP 数据包（如果你忘记了数据包头的样子，请查看 [3](#figure--fig:nmap-f4d49110)  ，“TCP 头”）。这是任何合法连接尝试中 TCP 三次握手的第一步。

-   由于目标端口是开放的，Scanme 发送一个带有 `SYN` 和 `ACK` 标志的回应，迈出了第二步。

-   `RST` <br />
    在正常连接中，Ereet 的机器（名为 krad）会通过发送一个 ACK 数据包来完成三次握手以确认 `SYN/ACK` 。然而，Nmap
    并不需要这样做，因为 SYN/ACK 响应已经告诉它端口是开放的。如果 Nmap 完成了连接，它就必须考虑关闭连接。这通常涉及另一个握手，使用 FIN 数据包而不是 SYN。因此，发送 ACK 并不是个好主意，但仍然需要做些什么。如果
    SYN/ACK 完全被忽略，Scanme 将假设它被丢弃并继续重新发送。因为我们不想建立完整的连接，所以合适的回应是 `RST`
    数据包，如图中所示。 这告诉 Scanme 忘掉（重置）尝试连接。

Nmap 可以轻松地发送这个 RST 数据包，但实际上并不需要这样。运行在 krad 上的操作系统也接收到 SYN/ACK，它并不期望这个回应，因为 Nmap 是自己构造了 SYN 探测。因此，操作系统以 RST 数据包来回应意外的 SYN/ACK。章节中描述的所有 RST 数据包的 ACK 位也被设置，因为它们总是用于回应（并确认）接收到的数据包。因此，该位在 RST
数据包中并没有明确显示。由于三次握手从未完成，SYN 扫描有时被称为 **半开放扫描** 。

<a id="figure--fig:nmap-f4d49110"></a>

{{< figure src="/ox-hugo/tcp_header.jpg" caption="<span class=\"figure-number\">Figure 3: </span>tcp header" width="800px" >}}


### <span class="section-num">2.2</span> 关闭端口的扫描 {#关闭端口的扫描}

[4](#figure--fig:nmap-caa7eb53) 展示了 Nmap 如何判断端口 `113` 是关闭的。这比开放端口的情况还要简单。第一步始终是相同的——Nmap
向 Scanme 发送 `SYN` 探测。但是，返回的不是 `SYN/ACK` ，而是 `RST` 。这就确定了——端口是关闭的。关于这个端口不再需要更多的通信。

<a id="figure--fig:nmap-caa7eb53"></a>

{{< figure src="/ox-hugo/Ereet_Packet_Trace_Syn_Closed.png" caption="<span class=\"figure-number\">Figure 4: </span>SYN scan of closed port 113" >}}


### <span class="section-num">2.3</span> 过滤掉的端口的扫描 {#过滤掉的端口的扫描}

最后，Ereet 向我们展示了过滤端口在图 [5](#figure--fig:nmap-d41d8cd9) 中如何表现于 Nmap。

首先，像往常一样发送初始的 `SYN` ，但 Nmap 没有见到任何回复。响应可能只是比较慢。根据之前的响应（或时间默认值），
Nmap 知道要等待多长时间，最终放弃接收。当一个端口没有响应时，通常表示该端口被过滤（被防火墙设备阻挡，或者主机可能已关闭），但这一测试并不能得出确定的结论。也许端口是开放的，但探测或响应被简单地丢弃了。网络可能会不稳定。因此，Nmap 通过重新发送 SYN 探测再次尝试。在又一个超时期后，Nmap 放弃并标记该端口为过滤。在这种情况下，只尝试了一次重传。如“扫描代码和算法”部分所述，Nmap 会仔细记录数据包丢失统计信息，并在扫描不可靠网络时尝试更多的重传。

<a id="figure--fig:nmap-d41d8cd9"></a>

{{< figure src="/ox-hugo/Ereet_Packet_Trace_Syn_Filtered.png" caption="<span class=\"figure-number\">Figure 5: </span>SYN scan of filtered port 139" >}}

如果 Nmap 收到某些 ICMP 错误消息，它也会认为端口被过滤。表 5.2 展示了 Nmap 如何根据对 SYN 探测的响应分配端口状态。

<a id="table--tbl:nmap-cdd7b2a6"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl:nmap-cdd7b2a6">Table 2</a>:</span>
  How Nmap interprets responses to a SYN probe
</div>

| **Probe Response**                                          || **Assigned State** |
|-------------------------------------------------------------|--------------------|
| TCP SYN/ACK response                                        || open               |
| TCP RST response                                            || closed             |
| No response received (even after retransmissions)           || filtered           |
| ICMP unreachable error (type 3, code 1, 2, 3, 9, 10, or 13) || filtered           |

虽然这一部分中的漂亮插图在有它们时非常有用，但当你在指定其他所需命令行标志时添加 `--packet-trace` 选项，Nmap
会准确报告它在数据包级别上所做的事情。这是新手理解 Nmap 行为的好方法，尤其是在 Ereet 不在身边帮忙的情况下。即使是高级用户在 Nmap 产生他们不期望的结果时也会觉得这很方便。你可能还想通过 `-d` （或甚至 `-d5` ）来提高调试级别。然后扫描为你的目的所需的最少数量的端口和主机，否则你可能会得到几百万行输出。示例 5.2 重复了
Ereet 的三端口 SYN 扫描，并启用了数据包追踪（输出已编辑以减少冗长）。阅读命令行，然后通过弄清楚在继续阅读之前将发送哪些数据包来测试自己。然后，当你阅读追踪信息直到“该 SYN 隐形扫描花费了 1.25
秒”时，你应该从 RCVD 行中知道端口状态表将会是什么样的，然后再继续阅读。

```text
Example 5.2. Using --packet-trace to understand a SYN scan
krad# nmap -d --packet-trace -p22,113,139 scanme.nmap.org

Starting Nmap ( https://nmap.org )
SENT (0.0130s) ICMP krad > scanme echo request (type=8/code=0) ttl=52 id=1829
SENT (0.0160s) TCP krad:63541 > scanme:80 A iplen=40 seq=91911070 ack=99850910
RCVD (0.0280s) ICMP scanme > krad echo reply (type=0/code=0) iplen=28
We got a ping packet back from scanme: id = 48821 seq = 714 checksum = 16000
massping done:  num_hosts: 1  num_responses: 1
Initiating SYN Stealth Scan against scanme.nmap.org (scanme) [3 ports] at 00:53
SENT (0.1340s) TCP krad:63517 > scanme:113 S iplen=40 seq=10438635
SENT (0.1370s) TCP krad:63517 > scanme:22 S iplen=40 seq=10438635
SENT (0.1400s) TCP krad:63517 > scanme:139 S iplen=40 seq=10438635
RCVD (0.1460s) TCP scanme:113 > krad:63517 RA iplen=40 seq=0 ack=10438636
RCVD (0.1510s) TCP scanme:22 > krad:63517 SA iplen=44 seq=75897108 ack=10438636
SENT (1.2550s) TCP krad:63518 > scanme:139 S iplen=40 seq=10373098 win=3072
The SYN Stealth Scan took 1.25s to scan 3 total ports.
Nmap scan report for scanme.nmap.org (64.13.134.52)
PORT    STATE    SERVICE
22/tcp  open     ssh
113/tcp closed   auth
139/tcp filtered netbios-ssn

Nmap done: 1 IP address (1 host up) scanned in 1.40 seconds

```

SYN 扫描长期以来被称为隐形扫描，因为它比 TCP 连接扫描（接下来讨论的内容）更为微妙，后者是在 Nmap
发布之前最常见的扫描类型。尽管有这个称呼，但不要指望默认的 SYN 扫描能够在敏感网络中不被发现。广泛部署的入侵检测系统甚至个人防火墙都非常能够检测到默认的 SYN 扫描。更有效的隐蔽扫描技术将在第 10 章《检测和规避防火墙和入侵检测系统》中展示。


## <span class="section-num">3</span> TCP Connect Scan (-sT) {#tcp-connect-scan--st}

