# 威胁建模框架与技术：从STRIDE到NIST-CSDN博客


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [简要介绍](#简要介绍)
- <span class="section-num">2</span> [威胁建模流程和步骤](#威胁建模流程和步骤)
- <span class="section-num">3</span> [威胁建模技术](#威胁建模技术)
    - <span class="section-num">3.1</span> [数据流程图](#数据流程图)
    - <span class="section-num">3.2</span> [过程流程图](#过程流程图)
    - <span class="section-num">3.3</span> [攻击树](#攻击树)
- <span class="section-num">4</span> [威胁建模框架和方法](#威胁建模框架和方法)
    - <span class="section-num">4.1</span> [STRIDE 威胁建模](#stride-威胁建模)
    - <span class="section-num">4.2</span> [DREAD 威胁建模](#dread-威胁建模)
    - <span class="section-num">4.3</span> [PASTA 威胁建模](#pasta-威胁建模)
    - <span class="section-num">4.4</span> [VAST 威胁建模](#vast-威胁建模)
    - <span class="section-num">4.5</span> [Trike 威胁建模](#trike-威胁建模)
    - <span class="section-num">4.6</span> [OCTAVE 威胁建模](#octave-威胁建模)
    - <span class="section-num">4.7</span> [NIST 威胁建模](#nist-威胁建模)
- <span class="section-num">5</span> [威胁建模最佳实践](#威胁建模最佳实践)
- <span class="section-num">6</span> [威胁建模工具](#威胁建模工具)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://blog.csdn.net/weixin_43965597/article/details/122882763



## <span class="section-num">1</span> 简要介绍 {#简要介绍}

了解威胁建模框架、方法和工具可以帮助更好地识别、量化和排序面临的威胁。
威胁建模是一个结构化的过程，IT 专业人员可以通过该过程识别潜在的安全威胁和漏洞，
量化每个威胁的严重性，并确定技术的优先级以缓解攻击并保护 IT 资产。威胁模型的重要之处在于它是系统的和结构化的。
威胁建模人员将执行一系列具体步骤，以全面了解他们试图保护的 IT 环境，识别漏洞和潜在攻击者。


## <span class="section-num">2</span> 威胁建模流程和步骤 {#威胁建模流程和步骤}

每种的威胁建模方法都包含一系列步骤，不同的建模方法的每个步骤存在着细微差别。首先，看一下所有这些方法共有的基本逻辑流程。
对威胁建模过程最简洁明了的概述指出，威胁模型的目的是回答四个问题：

1.  我们面对的是什么？
2.  可能会有哪些问题（威胁）？
3.  我们该怎么做？
4.  我们做得好吗？

反过来，威胁建模过程应包括四个主要步骤，每个步骤都会为这些问题之一提供答案。

1.  分解应用程序或基础架构
2.  确定威胁
3.  确定对策和缓解措施
4.  排序威胁

为了准确了解这些步骤中的每个步骤，我们需要讨论构成威胁建模基础的特定技术。


## <span class="section-num">3</span> 威胁建模技术 {#威胁建模技术}

上面列出的步骤中，最陌生的术语可能是分解（Decompose）。分解应用程序或基础架构意味着什么？

广义上讲，分解应用程序包括 **了解应用程序及其与外部实体的交互方式** 。这涉及:

-   创建用例，以了解应用程序的使用方式，
-   确定入口点以查看潜在的攻击者可以在哪里与应用程序进行交互，
-   确定资产（即攻击者可能会感兴趣的项目/区域），
-   并标识表示应用程序将授予外部实体的访问权限的信任级别。

    （这里专门谈论应用程序安全性，但是显然，从广义上讲，这也适用于对基础结构的看法。）


### <span class="section-num">3.1</span> 数据流程图 {#数据流程图}

分解应用程序的一种技术是 **构建数据流程图** 。
这是 1970 年代开发的一种方法，以可视方式展示数据如何在应用程序或系统中移动，以及各个组件在何处更改或存储数据。

其中 **信任边界** 是在 2000 年代初期添加进来的概念，特指数据流中的卡点，在该点上需要对数据进行验证，
然后数据才能被接收该数据的实体使用。信任边界是用数据流程图进行威胁建模的关键。

下图是在线银行应用程序的数据流程图；虚线表示信任边界，数据可能会在信任边界被更改，因此需要采取安全措施。

{{< figure src="/ox-hugo/8325497b61e28beab395fc96f9ba1d7d.png" width="800px" >}}


### <span class="section-num">3.2</span> 过程流程图 {#过程流程图}

由于数据流程图是由系统工程师而不是安全专家开发的，因此它们包含了许多威胁建模不需要的开销。
数据流程图的一种替代方法是过程流程图。
两者在总体概念上相似，但后者更加精简，并且侧重于用户和执行代码在系统中的移动方式，更紧密地反映了攻击者的思维方式（例如下图）。

{{< figure src="/ox-hugo/9a09b62b7d0a725e2c07c462d7ae81fb.png" width="800px" >}}


### <span class="section-num">3.3</span> 攻击树 {#攻击树}

绘制攻击树也是一种威胁建模技术，当您确定要对应用程序或基础结构潜在威胁的阶段时，它就变得非常重要。
攻击树由 90 年代后期的信息安全传奇人物布鲁斯·施耐尔（Bruce Schneier）开创。

它们由代表不同事件的一系列父节点和子节点组成，子节点是必须满足的条件才能使父节点为真。
根节点（图中的最高父节点）是攻击的总体目标。借助攻击树，威胁建模者可以看到必须组合哪些情况才能使威
胁成功。下图显示了一个简单的银行应用攻击树，说明了病毒可能成功感染文件的不同方式。

{{< figure src="/ox-hugo/feabf179c2185d2d36e0eaab59a163e8.png" width="800px" >}}

下图是 Hackinthebox 从攻击者的角度构建的攻击树示例，可以帮助了解自己所面临的威胁。

{{< figure src="/ox-hugo/59f5072a0923be7c6d7c42aba495dea4.png" width="800px" >}}


## <span class="section-num">4</span> 威胁建模框架和方法 {#威胁建模框架和方法}

威胁建模的各种结构化方法通常称为框架或方法论（本文中这两个术语基本上可以互换使用）。目前的威胁建模框架
和方法有很多，我们挑几个最流行的介绍如下：

7 种顶级威胁建模方法

1.  STRIDE
2.  DREAD
3.  PASTA
4.  VAST
5.  Trike
6.  OCTAVE
7.  NIST


### <span class="section-num">4.1</span> STRIDE 威胁建模 {#stride-威胁建模}

如上所述，STRIDE 是威胁建模的祖父，最早于 90 年代末在 Microsoft 开发。

STRIDE 代表六种威胁，每种都对 CIA 三要素构成威胁，具体如下：

1.  Spoofing 欺骗或冒充他人或计算机，影响真实性
2.  Tampering 篡改数据，这会破坏完整性
3.  Repudiation 抵赖，或无法将执行的操作关联到操作者，违反了不可抵赖性
4.  Information Disclosure 信息泄露，违反机密性
5.  Denial of Service 拒绝服务，这违反了可用性
6.  Elevation of Privilege 特权提升，违反授权


### <span class="section-num">4.2</span> DREAD 威胁建模 {#dread-威胁建模}

DREAD 被认为是 STRIDE 模型的一个附加组件，该模型使建模人员可以在确定威胁后对其进行排名。

对于每个潜在威胁，DREAD 代表六个问题,这些问题中的每一个都得到 1-3 分的评分：

1.  Damage 潜在损害：如果利用漏洞，造成的损害有多大？
2.  Reproducibility 重现性：重现攻击有多容易？
3.  Exploitability 可利用性：发动攻击有多容易？
4.  Affexted users 受影响的用户：大概影响了多少用户？
5.  Discoverability 可发现性：查找漏洞有多容易？


### <span class="section-num">4.3</span> PASTA 威胁建模 {#pasta-威胁建模}

PASTA 代表攻击模拟和威胁分析过程，它是一个七步骤过程，致力于使技术安全要求与业务目标保持一致。每个步骤
都非常复杂，由几个子步骤组成，但是总体顺序如下：

1.  定义目标

2.  定义技术范围

3.  应用程序分解

4.  威胁分析

5.  漏洞和弱点分析

6.  攻击建模

7.  风险与影响分析


### <span class="section-num">4.4</span> VAST 威胁建模 {#vast-威胁建模}

VAST 代表可视化，敏捷威胁建模。该模型是 ThreatModeler（自动威胁建模平台）的基础，该平台可以区分应用程序
和运营威胁模型。VAST 专为集成到围绕 devops 构建的工作流中而设计。


### <span class="section-num">4.5</span> Trike 威胁建模 {#trike-威胁建模}

Trike 是用于威胁建模和风险评估的框架的开源工具，它基于防御的角度，而不是试图模仿攻击者的思维过程。使用
Trike，您可以为要防御的系统建模，并通过 CRUD 的角度评估每个组件，也就是说，谁可以创建，读取，更新或删除
该实体。通过遍历数据流程图来识别威胁，每种威胁仅分为两类：拒绝服务或特权提升。


### <span class="section-num">4.6</span> OCTAVE 威胁建模 {#octave-威胁建模}

OCTAVE 代表“运营关键威胁，资产和脆弱性评估”，是卡耐基梅隆大学开发的一种威胁建模方法，其重点是组织风险而
不是技术风险。它包括三个阶段：

1.  建立基于资产的威胁配置文件

2.  识别基础架构漏洞

3.  制定安全策略和计划


### <span class="section-num">4.7</span> NIST 威胁建模 {#nist-威胁建模}

美国国家标准技术研究院拥有自己的以数据为中心的威胁建模方法，该方法包括四个步骤：

1.  系统和数据识别和表征

2.  识别并选择要包含在模型中的攻击媒介

3.  表征缓解攻击媒介的安全控件

4.  分析威胁模型

该 NIST 草案还包括一个方法的具体应用实例。
如果您正在寻找威胁建模示例，那么这是一个很棒的文档，可以阅读该文档以了解流程如何工作。


## <span class="section-num">5</span> 威胁建模最佳实践 {#威胁建模最佳实践}

无论选择哪种框架，都应遵循一些实践方法。但最重要的（通常也是很难做到的）是将威胁建模作为系统开发过程中
的优先事项。如果能在项目开发伊始就做到这一点，以后可以省去很多麻烦，因为安全性可以被深深“植入”到应用程
序或系统中。

另一个最佳实践是不要将应用程序和系统彼此隔离。Michael Santarcangelo 写道： “如果各种威胁模型以相同的方
式相互连接，并且应用程序和组件作为 IT 系统的一部分进行交互，那么结果将是一个全面的攻击面，CISO 可以使用该
攻击面来理解整个企业的整体威胁组合。”


## <span class="section-num">6</span> 威胁建模工具 {#威胁建模工具}

需要指出的是，上面列出的两种方法（VAST 和 Trike）实际上是围绕特定的软件工具构建的。还有一些支持其他方法
的工具，例如，Microsoft 提供了免费的威胁建模工具，而 OWASP Foundation 也推出了自己的桌面和 web 版本的威胁建
模工具——Threat Dragon³。

实际上，这里描述的许多方法都是概念性的，并未与任何软件联系在一起。攻击树或数据流程图可以用笔和纸绘制。
正如 Luca Bongiorni 的演讲所解释的那样，用于威胁建模的一些最受欢迎的工具是 Microsoft Visio 和 Excel。开始为
基础架构进行威胁建模的门槛很低，但回报却很高。

参考资料：

【1】最早的威胁建模论文：

<https://www.microsoft.com/security/blog/2009/08/27/the-threats-to-our-products/>

【2】Uncover Security Design Flaws Using The STRIDE Approach：

<https://docs.microsoft.com/en-us/archive/msdn-magazine/2006/november/uncover-security-design-flaws-using-the-stride-approach>

【3】OWASP 威胁建模工具：

Threat Dragon：<https://owasp.org/www-project-threat-dragon/>

