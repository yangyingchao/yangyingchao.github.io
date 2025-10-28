# NMap_ Port Scanning Techniques and Algorithms


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#introduction)
- <span class="section-num">2</span> [TCP SYN (Stealth) Scan (-sS) 半开放扫描](#tcp-syn--stealth--scan--ss--半开放扫描)
    - <span class="section-num">2.1</span> [开放端口的扫描](#开放端口的扫描)
    - <span class="section-num">2.2</span> [关闭端口的扫描](#关闭端口的扫描)
    - <span class="section-num">2.3</span> [过滤掉的端口的扫描](#过滤掉的端口的扫描)
- <span class="section-num">3</span> [TCP Connect Scan (-sT) 连接扫描](#tcp-connect-scan--st--连接扫描)
- <span class="section-num">4</span> [UDP Scan (-sU)](#udp-scan--su)
    - <span class="section-num">4.1</span> [区分开放的 UDP 端口和被过滤的 UDP 端口](#区分开放的-udp-端口和被过滤的-udp-端口)
    - <span class="section-num">4.2</span> [加快 UDP 扫描速度 Speeding Up UDP Scans](#加快-udp-扫描速度-speeding-up-udp-scans)
- <span class="section-num">5</span> [TCP FIN, NULL, and Xmas Scans (-sF, -sN, -sX)](#tcp-fin-null-and-xmas-scans--sf-sn-sx)

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

| **代码** | **描述**        |
|--------|---------------|
| 0/     | 网络不可达      |
| 1/     | 主机不可达      |
| 2/     | 协议不可达      |
| 3/     | 端口不可达      |
| 4      | 需要分片但设置了不分片位 |
| 5      | 源路线失败      |
| 6      | 目标网络未知    |
| 7      | 目标主机未知    |
| 8      | 源主机被隔离（过时） |
| 9/     | 目标网络因管理原因被禁止 |
| 10/    | 目标主机因管理原因被禁止 |
| 11     | 类型服务（TOS）不可达 |
| 12     | TOS 不可达主机  |
| 13/    | 由于过滤，通信因管理原因被禁止 |
| 14     | 主机优先级违规  |
| 15     | 优先级截止生效  |


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

最后，Ereet 向我们展示了过滤端口在图 [6](#figure--fig:nmap-d41d8cd9) 中如何表现于 Nmap。

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

| **Probe Response**                                          | **Assigned State** |
|-------------------------------------------------------------|--------------------|
| TCP SYN/ACK response                                        | open               |
| TCP RST response                                            | closed             |
| No response received (even after retransmissions)           | filtered           |
| ICMP unreachable error (type 3, code 1, 2, 3, 9, 10, or 13) | filtered           |

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


## <span class="section-num">3</span> TCP Connect Scan (-sT) 连接扫描 {#tcp-connect-scan--st--连接扫描}

如果用户没有原始数据包权限或在扫描 IPv6 网络时，=SYN= 扫描不可用时, 此时 nmap 会默认使用 TCP 连接扫描。Nmap
请求底层操作系统通过发出 connect 系统调用与目标机器和端口建立连接，但它不是从网络上读取原始数据包响应，而是使用该 API 获取每个连接尝试的状态信息。这个和 FTP 反向扫描（章节名为“TCP FTP 反向扫描 (-b)”）是唯一可供非特权用户使用的扫描类型。

当 `SYN` 扫描可用时，通常是更好的选择。Nmap 对高层连接调用的控制能力低于对原始数据包的控制，这使得其效率较低。系统调用会完成对开放目标端口的连接，而不是像 `SYN` 扫描那样执行半开重置。这不仅需要更长时间并且需要更多的数据包来获取相同的信息，而且目标机器更可能记录下该连接。一些合适的入侵检测系统（IDS）会捕获这两种情况，但大多数机器并没有这样的报警系统。在普通的 Unix 系统上，当 Nmap
连接并在没有发送数据的情况下关闭连接时，许多服务会在 syslog 中添加一条记录，有时还会产生一个难以理解的错误信息。尽管这种情况不常见，但在发生时，某些非常糟糕的服务会崩溃。当管理员在她的日志中看到来自单个系统的一系列连接尝试时，她应该意识到她已经被连接扫描了。

[6](#figure--fig:nmap-d41d8cd9) 显示了针对 scanme.nmap.org 上开放端口 22 的连接扫描的过程。回想一下，在图 [2](#figure--fig:nmap-28e943bb)
中，针对开放端口 `22` 的“同步扫描”仅需要三条数据包。对于开放端口的具体行为取决于 Nmap
运行的平台和另一端监听的服务，但这个五个数据包的示例是典型的。

<a id="figure--fig:nmap-d41d8cd9"></a>

{{< figure src="/ox-hugo/Ereet_Packet_Trace_Connect_Open.png" caption="<span class=\"figure-number\">Figure 6: </span>Connect scan of open port 22" >}}

前两步（SYN 和 SYN/ACK）与 SYN 扫描完全相同。然后，krad 不是用 RST 数据包中止半开连接，而是用自己的 ACK
数据包确认 SYN/ACK，从而完成连接。在这种情况下，Scanme 甚至有时间通过现在已开启的连接发送其 SSH 横幅字符串（ `SSH-1.99-OpenSSH_3.1p1\n` ）。一旦 Nmap 从其主机操作系统收到连接成功的信号，它就会终止连接。TCP
连接通常会以另一个涉及 FIN 标志的握手结束，但 Nmap 请求主机操作系统立即用 RST 数据包终止连接。

While this connect scan example took almost twice as many packets as a SYN scan, the bandwidth differences are rarely so
substantial. The vast majority of ports in a large scan will be closed or filtered. The packet traces for those are the
same as described for SYN scan in Figure 5.3, “SYN scan of closed port 113” and Figure 5.4, “SYN scan of filtered port
139”. Only open ports generate more network traffic.

尽管这个连接扫描示例使用的数据包几乎是 SYN 扫描的两倍，但带宽差异通常并没有如此显著。在一次大规模扫描中，绝大多数端口会是关闭或过滤状态。那些端口的数据包跟踪与图 [4](#figure--fig:nmap-caa7eb53) 中的“关闭端口 113 的 SYN 扫描”和图
[6](#figure--fig:nmap-d41d8cd9) 中的“过滤端口 139 的 SYN 扫描”所描述的相同。只有开放端口会产生更多的网络流量。

The output of a connect scan doesn't differ significantly from a SYN scan. Example 5.3 shows a connect scan of Scanme.
The -sT option could have been omitted since Nmap is being run from a non-privileged account so connect scan is the
default type.

连接扫描的输出与 SYN 扫描并没有显著差异。下例显示了对 Scanme 的连接扫描。由于 Nmap 是从非特权账户运行的，因此可以省略 `-sT` 选项，因为连接扫描是默认类型：

```text
Example 5.3. Connect scan example
krad~> nmap -T4 -sT scanme.nmap.org
Starting Nmap ( https://nmap.org )
Nmap scan report for scanme.nmap.org (64.13.134.52)
Not shown: 994 filtered ports
PORT    STATE  SERVICE
22/tcp  open   ssh
25/tcp  closed smtp
53/tcp  open   domain
70/tcp  closed gopher
80/tcp  open   http
113/tcp closed auth
Nmap done: 1 IP address (1 host up) scanned in 4.74 seconds
```


## <span class="section-num">4</span> UDP Scan (-sU) {#udp-scan--su}

虽然互联网上大多数流行服务运行在 TCP 协议上，但 UDP 服务也被广泛部署。DNS、SNMP 和 DHCP（注册端口 `53` 、
`161/162` 和 `67/68` ）是其中三个最常见的服务。由于 UDP 扫描通常比 TCP 更慢和更困难，因此一些安全审计员会忽略这些端口。这是一个错误，因为可被利用的 UDP 服务相当常见，而攻击者当然不会忽视整个协议。幸运的是，Nmap 可以帮助盘点 UDP 端口。

UDP 扫描通过 `-sU` 选项激活。它可以与 TCP 扫描类型（例如 SYN 扫描 `-sS` ）结合使用，以便在同一次扫描中检查这两种协议。

UDP 扫描通过向每个目标端口发送一个 UDP 数据包来工作。对于大多数端口，这个数据包将是空的（没有负载），但对于一些常用端口，将发送特定协议的负载。根据响应或没有响应的情况，端口被分配到下表 [3](#table--tbl:nmap-e4814b5a) 中的四种状态之一。

<a id="table--tbl:nmap-e4814b5a"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl:nmap-e4814b5a">Table 3</a>:</span>
  How Nmap interprets responses to a UDP probe
</div>

| **Probe Response**                                              | **Assigned State** |
|-----------------------------------------------------------------|--------------------|
| Any UDP response from target port (unusual)                     | open               |
| No response received (even after retransmissions)               | open/filtered      |
| ICMP port unreachable error (type 3, code 3)                    | closed             |
| Other ICMP unreachable errors (type 3, code 1, 2, 9, 10, or 13) | filtered           |

该表中最引人注目的元素可能是 `open|filtered` 状态。这是 UDP 扫描面临的最大挑战之一： **开放端口很少对空的探测包作出响应** 。

对于 Nmap 有协议特定负载的端口，更可能收到响应并标记为开放，但对于其余端口，目标 TCP/IP
栈会将空数据包传递给正在监听的应用程序，而该应用程序通常会立即将其丢弃，认为它是无效的。如果所有其他状态的端口都能响应，那么可以通过排除法推断出开放端口。不幸的是，防火墙和过滤设备也会在不响应的情况下丢弃数据包。因此，当 Nmap 在多次尝试后未收到响应时，它无法判断该端口是开放还是被过滤。Nmap 发布时，过滤设备还相对较少，因此 Nmap 可以（并确实）简单地假设该端口是开放的。现在互联网的防护更加严格，因此 Nmap 在 2004 年（版本 3.70）进行了更改，将无响应的 UDP
端口报告为 open|filtered。

我们可以在示例 5.4 中看到这一点，该示例展示了 Ereet 扫描名为 Felix 的 Linux 主机。

Example 5.4. UDP scan example

```text
krad# nmap -sU -v felix
Starting Nmap ( https://nmap.org )
Nmap scan report for felix.nmap.org (192.168.0.42)
(The 997 ports scanned but not shown below are in state: closed)
PORT    STATE         SERVICE
53/udp  open|filtered domain
67/udp  open|filtered dhcpserver
111/udp open|filtered rpcbind
MAC Address: 00:02:E3:14:11:02 (Lite-on Communications)
Nmap done: 1 IP address (1 host up) scanned in 999.25 seconds
```

对 Felix 的扫描展示了 open|filtered 模糊性问题以及另一个问题：UDP 扫描可能很慢。在这种情况下，扫描一千个端口花费了将近 17 分钟，这主要是由于 Felix 和大多数其他 Linux 系统执行的 ICMP 响应速率限制。Nmap
提供了一些方法来处理这两个问题，以下两个章节将对此进行描述。


### <span class="section-num">4.1</span> 区分开放的 UDP 端口和被过滤的 UDP 端口 {#区分开放的-udp-端口和被过滤的-udp-端口}

在 Felix 扫描的情况下，除了三个 open|filtered 端口外，其他的都是关闭的。因此，这次扫描仍然成功地将潜在的开放端口缩小到几个。这并不总是如此。示例 5.5 显示了针对 heavily filtered 站点 Scanme
的 UDP 扫描。

Example 5.5. UDP scan example

```text
krad# nmap -sU -T4 scanme.nmap.org
Starting Nmap ( https://nmap.org )
All 1000 scanned ports on scanme.nmap.org (64.13.134.52) are open|filtered
Nmap done: 1 IP address (1 host up) scanned in 5.50 seconds
```

在这种情况下，扫描完全没有缩小开放端口的范围。所有 1000 个端口都是 open|filtered。需要采用新的策略。

表 [3](#table--tbl:nmap-e4814b5a) “Nmap 如何解释 UDP 探测的响应” 显示，当 Nmap 未能从其 UDP 探测中收到任何响应时，
`open|filtered` 状态就会出现。然而，它也显示，在少数情况下，监听特定端口的 UDP 服务会作出响应，从而证明该端口是开放的。这些服务不常响应的原因是，Nmap 发送的空数据包被视为无效。不幸的是，UDP
服务通常会定义自己的数据包结构，而不是遵循某种通用格式，这使得 Nmap 无法始终如一地发送。SNMP 数据包与 SunRPC、
DHCP 或 DNS 请求数据包看起来截然不同。

为了向每个流行的 UDP 服务发送正确的数据包，Nmap 需要一个大型数据库来定义它们的探测格式。幸运的是，Nmap
具备这一点，使用的是 nmap-service-probes，这是一部分在第 7 章《服务和应用版本检测》中描述的服务和版本检测子系统。

当启用版本扫描（使用 `-sV` 或 `-A` ）时，Nmap 将向每个 `open|filtered` 端口（以及已知的开放端口）发送 UDP 探测。如果任何探测包从 open|filtered 端口获得响应，则该状态会被更改为开放。本次在 Felix 扫描中添加 -sV 的结果在示例
5.6 中显示。

Example 5.6. Improving Felix's UDP scan results with version detection

```text
krad# nmap -sUV -F felix.nmap.org
Starting Nmap ( https://nmap.org )
Nmap scan report for felix.nmap.org (192.168.0.42)
Not shown: 997 closed ports
PORT    STATE         SERVICE    VERSION
53/udp  open          domain     ISC BIND 9.2.1
67/udp  open|filtered dhcpserver
111/udp open          rpcbind    2 (rpc #100000)
MAC Address: 00:02:E3:14:11:02 (Lite-on Communications)
Nmap done: 1 IP address (1 host up) scanned in 1037.57 seconds
```

这次新的扫描显示端口 `111` 和 `53` 确实是开放的。然而，该系统并不完美——端口 67 仍然是 `open|filtered` 。在这个特定案例中，端口是开放的，但 Nmap 没有针对 DHCP 的有效版本探测。另一个棘手的服务是 SNMP，它通常只在给定正确的社区字符串时才会响应。许多设备配置了公共社区字符串，但并非所有设备都是如此。虽然这些结果并不完美，但了解三个测试端口中两个的真实状态仍然有帮助。

在成功澄清了 Felix 的结果后，Ereet 将注意力转回到 Scanme，后者上次显示所有端口均为 `open|filtered` 。他再次尝试版本检测，如示例 5.7 所示。

Example 5.7. Improving Scanme's UDP scan results with version detection

```text
krad# nmap -sUV -T4 scanme.nmap.org
Starting Nmap ( https://nmap.org )
Nmap scan report for scanme.nmap.org (64.13.134.52)
Not shown: 999 open|filtered ports
PORT   STATE SERVICE VERSION
53/udp open  domain  ISC BIND 9.3.4
Nmap done: 1 IP address (1 host up) scanned in 3691.89 seconds
```

虽然这种版本检测技术是 Nmap 自动澄清 `open|filtered` 端口的唯一方法，但也可以尝试一些手动技巧。有时，专用的
traceroute 可以提供帮助。你可以针对已知开放的 TCP 或 UDP 端口使用 Nmap 或 Nping 等工具进行 traceroute。然后，尝试针对可疑的 UDP 端口执行相同的操作。跳数的差异可以帮助区分开放和被过滤的端口。Ereet 在示例 5.8 中对 Scanme
进行了尝试。第一个命令对已知开放的端口 `53` 进行了 UDP traceroute。第二个命令对假定关闭的端口 `54`
进行了相同的操作。为了节省空间，前几个跳数已被省略。

Example 5.8. Attempting to disambiguate UDP ports with TTL discrepancies

```text
krad# nping --udp --traceroute -c 13 -p 53 scanme.nmap.org
Starting Nping ( https://nmap.org/nping )
SENT (7.0370s) UDP 192.168.0.21:53 > 64.13.134.52:53 ttl=8 id=4826 iplen=28
RCVD (7.1010s) ICMP 4.69.134.222 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=248 id=38454 iplen=56
SENT (8.0400s) UDP 192.168.0.21:53 > 64.13.134.52:53 ttl=9 id=38166 iplen=28
RCVD (8.1050s) ICMP 4.68.18.204 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=247 id=39583 iplen=56
SENT (9.0420s) UDP 192.168.0.21:53 > 64.13.134.52:53 ttl=10 id=6788 iplen=28
RCVD (9.1080s) ICMP 4.59.4.78 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=246 id=59897 iplen=56
SENT (10.0440s) UDP 192.168.0.21:53 > 64.13.134.52:53 ttl=11 id=366 iplen=28
RCVD (10.1100s) ICMP 69.36.239.221 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=243 id=42710 iplen=56
SENT (11.0470s) UDP 192.168.0.21:53 > 64.13.134.52:53 ttl=12 id=63478 iplen=28
SENT (12.0490s) UDP 192.168.0.21:53 > 64.13.134.52:53 ttl=13 id=56653 iplen=28
Max rtt: 73.003ms | Min rtt: 0.540ms | Avg rtt: 48.731ms
Raw packets sent: 13 (364B) | Rcvd: 10 (560B) | Lost: 3 (23.08%)
Tx time: 12.02836s | Tx bytes/s: 30.26 | Tx pkts/s: 1.08
Rx time: 13.02994s | Rx bytes/s: 42.98 | Rx pkts/s: 0.77
Nping done: 1 IP address pinged in 13.05 seconds
krad# nping --udp --traceroute -c 13 -p 54 scanme.nmap.org
Starting Nping ( https://nmap.org/nping )
SENT (7.0370s) UDP 192.168.0.21:53 > 64.13.134.52:54 ttl=8 id=56481 iplen=28
RCVD (7.1130s) ICMP 4.69.134.214 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=248 id=22437 iplen=56
SENT (8.0400s) UDP 192.168.0.21:53 > 64.13.134.52:54 ttl=9 id=23264 iplen=28
RCVD (8.1060s) ICMP 4.68.18.76 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=247 id=50214 iplen=56
SENT (9.0430s) UDP 192.168.0.21:53 > 64.13.134.52:54 ttl=10 id=9101 iplen=28
RCVD (9.1070s) ICMP 4.59.4.78 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=246 id=880 iplen=56
SENT (10.0450s) UDP 192.168.0.21:53 > 64.13.134.52:54 ttl=11 id=35344 iplen=28
RCVD (10.1110s) ICMP 69.36.239.221 > 192.168.0.21 TTL=0 during transit (type=11/code=0) ttl=243 id=44617 iplen=56
SENT (11.0470s) UDP 192.168.0.21:53 > 64.13.134.52:54 ttl=12 id=53857 iplen=28
SENT (12.0490s) UDP 192.168.0.21:53 > 64.13.134.52:54 ttl=13 id=986 iplen=28
Max rtt: 76.488ms | Min rtt: 0.546ms | Avg rtt: 48.480ms
Raw packets sent: 13 (364B) | Rcvd: 11 (616B) | Lost: 2 (15.38%)
Tx time: 12.02908s | Tx bytes/s: 30.26 | Tx pkts/s: 1.08
Rx time: 13.03165s | Rx bytes/s: 47.27 | Rx pkts/s: 0.84
Nping done: 1 IP address pinged in 13.05 seconds
```

在这个例子中，Ereet 只是能够到达开放和关闭端口的第十一跳。因此，无法利用这些结果来区分该主机的端口状态。这个方式值得一试，而且在许多情况下确实有效。它在筛选防火墙位于目标主机前面一两跳的情况下更有可能奏效。另一方面，Scanme
正在运行自己的基于 Linux 的 iptables 主机防火墙。因此，在过滤和开放端口之间的跳数没有差异。

另一种技术是针对常见端口尝试应用程序特定的工具。例如，可以针对端口 161 尝试暴力破解 SNMP 社区字符串。随着 Nmap
的版本检测探测数据库的不断增长，用外部专用工具来补充其结果的需求正在减少。对于特殊情况，例如具有自定义社区字符串的 SNMP 设备，它们仍然会很有用。


### <span class="section-num">4.2</span> 加快 UDP 扫描速度 Speeding Up UDP Scans {#加快-udp-扫描速度-speeding-up-udp-scans}

进行 UDP 扫描的另一个重大挑战是快速完成。开放和被过滤的端口很少会发送任何响应，这导致 Nmap 超时，然后进行重传以防探测包或响应丢失。关闭端口通常是一个更大的问题。它们通常会发送 ICMP 端口不可达的错误消息。但是，与关闭的 TCP 端口在响应 SYN 或连接扫描时发送的 RST 数据包不同，许多主机默认情况下会对 ICMP
端口不可达消息进行速率限制。Linux 和 Solaris 在这方面特别严格。例如，Felix 上的 Linux 2.4.20
内核将目的地不可达消息限制为每秒一个（在 net/ipv4/icmp.c 中）。这就解释了为什么示例 5.4 “UDP 扫描示例”的扫描速度如此缓慢。

Nmap 会检测到速率限制，并相应地减缓速度，以避免用无用的数据包淹没网络，这些数据包会被目标机器丢弃。不幸的是，
Linux 风格的每秒一个数据包的限制使得 65,536 个端口的扫描需要超过 18 小时。以下是一些提高 UDP 扫描性能的建议。另外，请阅读第 6 章《优化 Nmap 性能》，以获取更详细的讨论和一般建议。

-   **增加主机并行性** <br />
    如果 Nmap 每秒仅从单个目标主机接收到一个端口不可达错误，则通过同时扫描 100 个这样的主机，它每秒可以接收 100
    个错误消息。通过将一个较大的值（例如 100）传递给 &#x2013;min-hostgroup 来实现这一点。

-   **首先扫描常用端口** <br />
    很少有 UDP 端口号是常用的。对最常见的 100 个 UDP 端口进行扫描（使用 -F 选项）将快速完成。然后，您可以在后台启动为期几天的 65K 端口扫描的同时，调查这些结果。

-   **在版本检测扫描中添加 &#x2013;version-intensity 0** <br />
    正如前面一节所提到的，版本检测（-sV）通常需要区分开放和被过滤的 UDP 端口。版本检测相对较慢，因为它涉及向每个发现的开放或 open|filtered 端口发送大量特定于应用协议的探测包。指定 &#x2013;version-intensity 0
    将指示 Nmap 仅尝试最有可能对给定端口号有效的探测包。它通过使用来自 nmap-service-probes 文件的数据来实现。这个选项的性能影响是显著的，在本节稍后将进行演示。

-   **在防火墙后面扫描** <br />
    与 TCP 一样，数据包过滤器会显著降低扫描速度。许多现代防火墙使得设置数据包速率限制变得简单。如果您可以通过从防火墙后面发起扫描而不是穿越它来绕过这个问题，请这样做。

-   **使用 &#x2013;host-timeout 跳过慢速主机** <br />
    被 ICMP 速率限制的主机扫描所需的时间可能比那些快速响应每个探测包并返回目的不可达数据包的主机多几个数量级。指定一个最大扫描时间（例如 15m 意味着 15 分钟）会使 Nmap
    在未能在指定时间内完成扫描的情况下放弃对个别主机的扫描。这使您能够快速扫描所有响应的主机。然后，您可以在后台处理那些慢速主机。

-   **使用 -v 并放松心情** <br />
    启用详细模式 (-v) 后，Nmap 会提供每个主机扫描完成的预计时间。没有必要密切关注它。您可以去睡觉，前往您最喜欢的酒吧，读一本书，完成其他工作，或者其他方式消遣自己，而 Nmap 将不辞辛劳地为您进行扫描。

优化 UDP 扫描的完美例子是示例 5.7，“通过版本检测改善 Scanme 的 UDP 扫描结果”。这次扫描获取了所需的数据，但对这个主机的扫描耗时超过一个小时！在示例 5.9 中，Ereet 再次运行了该扫描。这一次，他添加了 `-F` 和
`--version-intensity 0` 选项，将耗时一小时的扫描缩短到 13 秒！然而，仍然检测到相同的关键信息（在端口 53 上运行的
ISC Bind 守护进程）。

```text
Example 5.9. Optimizing UDP Scan Time
krad# nmap -sUV -T4 -F --version-intensity 0 scanme.nmap.org
Starting Nmap ( https://nmap.org )
Nmap scan report for scanme.nmap.org (64.13.134.52)
Not shown: 99 open|filtered ports
PORT   STATE SERVICE VERSION
53/udp open  domain  ISC BIND 9.3.4
Nmap done: 1 IP address (1 host up) scanned in 12.92 seconds
```


## <span class="section-num">5</span> TCP FIN, NULL, and Xmas Scans (-sF, -sN, -sX) {#tcp-fin-null-and-xmas-scans--sf-sn-sx}

