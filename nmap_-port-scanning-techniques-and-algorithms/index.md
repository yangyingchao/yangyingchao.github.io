# NMap_ Port Scanning Techniques and Algorithms


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#introduction)
- <span class="section-num">2</span> [TCP SYN (Stealth) Scan (-sS)](#tcp-syn--stealth--scan--ss)

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


## <span class="section-num">2</span> TCP SYN (Stealth) Scan (-sS) {#tcp-syn--stealth--scan--ss}

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

