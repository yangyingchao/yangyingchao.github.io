#   调优"四剑客"的实战演练，福尔摩斯•K带你轻松优化性能
  


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [几个重要的库](#h:181c6e2a-6c88-472a-9347-59fec0ec00f1)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://mp.weixin.qq.com/s?__biz=MjM5NjgwMDMxMg==&mid=2652164045&idx=1&sn=305c8d05c0b8b41827a9d4062c44a4a8&chksm=bd03dd9a8a74548c907bc3f1ffac82db0207e14bdd6c2398fa798187b9ea449371a9f0ec1f6f&mpshare=1&scene=1&srcid=1205yZcXcCHhOMmAZpuiFyFo&sharer_sharetime=1688349494576&sharer_shareid=a1eb8d286d4dcd42a0c884a984d12f54#rd



## <span class="section-num">1</span> 几个重要的库 {#h:181c6e2a-6c88-472a-9347-59fec0ec00f1}

-   **KWR** ： <br />
    KES 自动负载信息库（Automatic Workload Repertories），每小时自动采集性能指标生成快照，建立数据库时间模型、IO 模型、等待事件、TOP SQL 和操作系统统计等性能指标，为数据库性能调优提供指导。

-   **KSH：** <br />
    KES 活跃会话历史库（Active Session History），每秒自动采样活跃会话的等待事件、SQL 语句、阻塞等会话信息，提供针对某个历史时间点的性能问题分析。

-   **KDDM：** <br />
    KES 自动诊断和建议（Automatic Database Diagnostic Monitor），基于 KWR 快照和数据库时间模型，给出内存、网络、IO 和 SQL 性能优化建议。

-   **KWR Diff：** <br />
    KWR 差异报告，通过比较 2 段时间内 KWR 性能指标的差异，发现性能变化的趋势。

