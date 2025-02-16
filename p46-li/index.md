# A Learned Query Rewrite System using Monte Carlo Tree Search


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [INTRODUCTION](#h:46536eca-7ea9-4ca8-80f5-de498e0868a9)
- <span class="section-num">2</span> [LearnedRewrite](#h:a4cf4c8f-4467-4181-b74c-5347a4209ae6)
    - <span class="section-num">2.1</span> [树形结构](#h:4e4f4b6c-db9f-46e4-8921-1ff6007daec1)
- <span class="section-num">3</span> [PRELIMINARIES](#h:44d869de-543c-466b-850e-e8f46557dcc5)
    - <span class="section-num">3.1</span> [Query Rewrite Rules](#h:5fd43385-0787-467c-9b88-873a9dca9406)
    - <span class="section-num">3.2</span> [Query Rewrite](#h:a32a0924-5b47-4885-9cea-878cb01dd027)
- <span class="section-num">4</span> [TREE SEARCH FOR QUERY REWRITE](#h:31bb8cdf-d5e0-41fe-a908-17e696dca69a)
    - <span class="section-num">4.1</span> [Overview of Policy Tree Search](#h:75f54fbc-fec3-421e-80be-b95c589828aa)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/7/p46-li.pdf

-   查询重写使用启发式算法来实现，有两个限制
    -   规则的应用顺序严重影响查询性能，但
        -   可能的重写顺序随查询涉及到的算子指数增长
        -   受限于搜索空间大小限制，很难找到最佳的顺序
    -   针对不同的查询，不同的重写规则的收益也不同
        -   当前的方法，只能应用于单个计划，而不能有效的估计查询重写的收益

-   提出了基于策略树树的查询重写框架


## <span class="section-num">1</span> INTRODUCTION {#h:46536eca-7ea9-4ca8-80f5-de498e0868a9}

-   查询重写：将一个 SQL 查询转换成为等价的、但性能更高的 SQL.

-   规则的应用顺序严重影响查询性能， 以 [下图](#figure--fig:screenshot@2022-11-09-15:47:02) 为例：
    -   `q1`
        -   采用传统的从上到下的顺序应用规则
        -   仅能应用 `O1` 和 `O3`
        -   执行时间 `>20 min`

    -   `q2`

        -   通过策略树实现 `O1,O4,O3,O5` 的顺序应用规则
        -   执行时间 `1.941s`

        性能相差 600 倍。

<a id="figure--fig:screenshot@2022-11-09-15:47:02"></a>

{{< figure src="/ox-hugo/screenshot@2022-11-09_15:47:02.png" width="800px" >}}

-   传统方法通过匹配预定义的规则顺序来重写
    -   可能会陷入局部最优解


## <span class="section-num">2</span> LearnedRewrite {#h:a4cf4c8f-4467-4181-b74c-5347a4209ae6}


### <span class="section-num">2.1</span> 树形结构 {#h:4e4f4b6c-db9f-46e4-8921-1ff6007daec1}

-   使用 **策略树** 表达可能的顺序：
    -   根节点 root: 表示输入的原始 SQL
    -   每个非根节：点表示对其父节点应用重新规则之后生成的新的查询
    -   根节点到其他节点的路径：表示重新的顺序

-   策略树的优势
    -   不同路径，可以共享相同的祖先 （已经重写的查询）
        -   避免重复应用规则
    -   可以通过蒙特卡罗树搜索 (Monte Carlo Tree Search, MCTS) 来探索策略树从而找到优化节点


## <span class="section-num">3</span> PRELIMINARIES {#h:44d869de-543c-466b-850e-e8f46557dcc5}


### <span class="section-num">3.1</span> Query Rewrite Rules {#h:5fd43385-0787-467c-9b88-873a9dca9406}


#### <span class="section-num">3.1.1</span> Input Query {#h:0ba248e2-ddf1-4270-b6be-c6478f191999}

<a id="figure--fig:screenshot@2022-11-09-18:02:29"></a>

{{< figure src="/ox-hugo/screenshot@2022-11-09_18:02:29.png" >}}


#### <span class="section-num">3.1.2</span> Query Tree {#h:bceebff8-90b2-4dc0-b2ff-836682b9f31d}

<a id="figure--fig:screenshot@2022-11-09-18:02:39"></a>

{{< figure src="/ox-hugo/screenshot@2022-11-09_18:02:39.png" >}}


#### <span class="section-num">3.1.3</span> Query Rewrite Rules {#h:2052df80-43d1-414d-b812-efc69815e732}

-   规则：针对查询的等价变换
-   定义：  \\[r = (o,c,a)\\]
    -   含义：
        -   o: operator, 算子
        -   c: condition, 条件
        -   a: rewrite action
    -   解释：对于指定的查询 `q` ，规则
        -   首先匹配到算子 `o`
        -   如果 `c` 满足，或者 `o` 是子树的 root, 则对 `q` 应用 `a` ，得到 \\[q^{(o,r)}\\] ,
        -   `q` 和 \\[q^{(o,r)}\\] 等价


#### <span class="section-num">3.1.4</span> Rewrite Benefit of Applying A Rewrite Rule {#h:b2950e60-7f0c-4b5d-b703-4c88b7f41e1d}

\\[\Delta Cost(q^{(0,r)},q) = Cost(q) - Cost(q^{(o,r)})\\] where:

-   \\[Cost(q)\\] : 重写之前的代价
-   \\[Cost(q^{(o,r)})\\] ： 重写之后的代价


#### <span class="section-num">3.1.5</span> The Rewrite Order of Applying Multiple Rewrite Rules {#h:71790411-e9d5-4abd-bad0-43130750872d}


### <span class="section-num">3.2</span> Query Rewrite {#h:a32a0924-5b47-4885-9cea-878cb01dd027}


#### <span class="section-num">3.2.1</span> Query Rewrite {#h:01c94b89-3447-42b8-8239-6d9877adab66}

-   Human-involved methods
    -   手动干预
    -   性能高
    -   分析，决策耗时长

-   Heuristic query rewrite 启发式 （如 PG）
    -   自顶向下遍历查询计划中的算子，对每个算子：
        -   如果匹配到规则，则应用规则

    -   效率更高，但有两个主要限制：
        -   应用规则的顺序是固定的 <br />
            可能会错过更好的重写顺序
        -   该方法不考虑重写的收益 <br />
            可能会导致重写无用、甚至变得更慢


#### <span class="section-num">3.2.2</span> Learning Models for Databases {#h:e60dace2-5873-411a-afb8-6eba67661c54}


#### <span class="section-num">3.2.3</span> Reinforcement Learning {#h:6c83ebc3-f01d-47f5-a983-1977da0d0ab6}


## <span class="section-num">4</span> TREE SEARCH FOR QUERY REWRITE {#h:31bb8cdf-d5e0-41fe-a908-17e696dca69a}


### <span class="section-num">4.1</span> Overview of Policy Tree Search {#h:75f54fbc-fec3-421e-80be-b95c589828aa}

-   Policy Tree: <br />
    Given a query q and a set of rewrite rules, we build a policy tree T , where
    the root node denotes the origin query q, any non-root node denotes a
    rewritten query (that transforms the query of its parent by applying a
    rewrite operation), and a leaf denotes a query that cannot be rewritten by
    any rewrite rules.

