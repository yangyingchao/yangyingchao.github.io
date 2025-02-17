# cary huang:  PostgreSQL’s Planner – Simple Scan Paths vs Plans


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#introduction)
- <span class="section-num">2</span> [Where it all start](#where-it-all-start)
- <span class="section-num">3</span> [What happens behind `pg_plan_query`](#what-happens-behind-pg-plan-query)
- <span class="section-num">4</span> [set_base_rel_sizes()](#set-base-rel-sizes)
- <span class="section-num">5</span> [set_base_rel_pathlist()](#set-base-rel-pathlist)
- <span class="section-num">6</span> [generate_gather_paths](#generate-gather-paths)
- <span class="section-num">7</span> [get_cheapest_fractional_path and create_plan](#get-cheapest-fractional-path-and-create-plan)
- <span class="section-num">8</span> [Examine the Plan](#examine-the-plan)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://postgr.es/p/6s9



## <span class="section-num">1</span> Introduction {#introduction}

当你向 PostgreSQL 发送查询时，通常会经历查询处理的各个阶段，并在最后返回结果。这些阶段被称为：

-   解析（Parse）
-   分析（Analyze）
-   重写（Rewrite）
-   规划（Plan）
-   执行（Execute）

在这篇博客中，我们将只关注“规划”阶段或者“规划器”模块，因为这或许是最有趣或者最复杂的阶段。我将分享我对规划器模块的理解，探究其内部运作以处理一个简单的顺序扫描。这将基于 PostgreSQL
16。

虽然规划器的目标非常简单，即从一系列可选路径中识别最快的“路径”，并据此制订一个“方案”，以便“执行器”模块在下一个阶段执行。然而， **要识别最快的“路径”正是使规划器变得复杂的原因** 。


## <span class="section-num">2</span> Where it all start {#where-it-all-start}

在 postgres.c 中的 `exec_simple_query()` 函数是查询处理阶段发生的地方。我们将关注它落在 `pg_plan_query()` 之后发生了什么。我只会提到它将会落在的重要函数。

{{< figure src="/ox-hugo/highgo-training-Copy-of-Page-12.drawio-1.png" width="800px" >}}


## <span class="section-num">3</span> What happens behind `pg_plan_query` {#what-happens-behind-pg-plan-query}

实际上会发生很多事情，比如：

-   标识子查询、分区表、外键表、连接等等
-   借助表访问方法估计所有涉及表的大小
-   确定完成查询的所有可能路径
    -   顺序扫描、索引扫描、TID 扫描、并行工作线程等

-   在所有路径中找到最佳路径，通常是成本最低的
-   制定一个执行计划。

对于单个表的简单 SELECT 查询，不涉及任何连接或子查询，pg_plan_query 下的近似调用栈如下可视化描述：

这个甘特图被极大地简化，但展示了规划模块的若干关键要素。如果字太小，你可能需要下载图片以得到更好的视觉效果。接下来将详细解释带有蓝色星标记的区块。

{{< figure src="/ox-hugo/highgo-training-Page-20.drawio-4.png" width="800px" >}}


## <span class="section-num">4</span> set_base_rel_sizes() {#set-base-rel-sizes}

:NOTER_DOCUMENT: <https://postgr.es/p/6s9>
:NOTER_OPEN: eww
:NOTER_PAGE: 2076
:END:

正如其名称所示，这是用于估算与所有关系（表、视图、索引等）有关的大小的主要入口点。 大小包括预估的行数（元组）和列数。 这些信息通常必须从“堆访问方法”中获取，其中它将可以访问“缓冲区管理器”和“存储管理器”以提供大小的估算。

总大小将是所有涉及表的大小之和。 这对后续的“成本估算”阶段非常重要。


## <span class="section-num">5</span> set_base_rel_pathlist() {#set-base-rel-pathlist}

对于简单表的简单顺序扫描，程序会最终到达这里。其他更复杂的查询将使用不同的路径构建技术。有关其他路径构建技术，请参考 allpaths.c 中的“set_rel_pathlist()”。

目前，默认添加了 4 个扫描路径：

-   顺序扫描 <br />
    逐个顺序扫描所有内容

-   部分顺序扫描 <br />
    -   这是以“部分”添加的，因为它将由“gather”节点聚合，该节点在下一阶段考虑。这本质上意味着是一个并行顺序扫描

    -   仅当关系或查询是并行安全时才添加

-   索引扫描 <br />
    如果表有索引，可以考虑作为潜在路径

-   tid 扫描 <br />
    如果查询包含范围限制子句（WHERE ctid &gt; '(1,30)' AND ctid &lt; '(30,5)'），则 tid 扫描可能是一个选项

所有这些都需要一些成本，这些成本是由元组或页面的数量以及每个元组/页面的成本因素来估算的，如下配置:

```cfg
# - Planner Cost Constants -

seq_page_cost = 1.0                    # measured on an arbitrary scale
random_page_cost = 4.0                 # same scale as above
cpu_tuple_cost = 0.01                  # same scale as above
cpu_index_tuple_cost = 0.005           # same scale as above
cpu_operator_cost = 0.0025             # same scale as above
parallel_setup_cost = 1000.0           # same scale as above
parallel_tuple_cost = 0.1              # same scale as above
```

不同的路径方法具有不同的成本计算，它们会调用以下方法来计算“启动成本”和“运行成本”：

-   cost_seqscan()
-   cost_indexscan()
-   cost_tidscan()

你可以在选择最理想路径制定计划时从根本上影响规划者的决策。例如，如果你希望规划者更频繁地使用并行扫描，你可以考虑降低每个并行扫描元组的成本，通过调小"parallel_tuple_cost"，比如设为 0.001。

"add_path" 函数被调用以将路径添加到潜在路径列表中，但请记住规划器的路径构建机制确实有一种排出机制。这意味着如果我们打算添加一条明显优于已添加的其他路径的路径，它可能会删除所有现有路径并接受新路径。同样，如果要添加的路径明显更差，则根本不会被添加。

如果规划程序认为并行顺序扫描是安全的，则会调用“add_partial_path”。这种顺序扫描是“partial”（部分的），因为需要对其进行收集和聚合，以形成最终结果，从而导致额外的成本，因此并行性可能并不总是理想的。以下是一个经验法则：

-   如果 PostgreSQL 需要扫描大量数据，但我们只需要其中的少量数据，则并行性可以帮助
-   如果 PostgreSQL 需要扫描大量数据，而其中大部分是我们需要的数据，则并行性可能会更慢。


## <span class="section-num">6</span> generate_gather_paths {#generate-gather-paths}

如果已经添加了一些部分路径，通常是顺序扫描子路径，则调用此例程。这个例程添加了一个名为“gather”的新路径类型，其中包含一个名为“顺序扫描”的子路径。gather 路径必须考虑每个子路径的成本，以及从并行工作程序获取元组和在最终形式中对数据进行聚合的成本。


## <span class="section-num">7</span> get_cheapest_fractional_path and create_plan {#get-cheapest-fractional-path-and-create-plan}

一旦所有可能的路径候选项都被添加完毕，将调用此函数来选择最便宜的路径，即总成本最低的路径。然后选择的路径将被输入到“create_plan”中，其中路径（以及子路径（如果有））将会递归地创建，并制定成执行器能理解和执行的最终计划结构。


## <span class="section-num">8</span> Examine the Plan {#examine-the-plan}

我们可以在查询之前使用 EXPLAIN ANALYZE 来检查规划器选择的最便宜的计划及其成本细节。以下示例是一个包含 1 个名为“gather”的主计划及 1 个局部计划“sequential scan”的查询计划，该计划有 2
个 worker。您可以通过箭头（-&gt;）来判断它代表一个子路径。

```sql
postgres=# explain analyze select * from test where a > 500000 and a <600000;
                          QUERY PLAN
------------------------------------------------------------
 Gather  (cost=1000.00..329718.40 rows=112390 width=36) (actual time=62.362..5106.295 rows=99999 loops=1)
   Workers Planned: 2
   Workers Launched: 2
   ->  Parallel Seq Scan on test  (cost=0.00..317479.40 rows=46829 width=36) (actual time=58.020..3416.544 rows=33333 loops=3)
         Filter: ((a > 500000) AND (a < 600000))
         Rows Removed by Filter: 13300000
 Planning Time: 0.489 ms
 Execution Time: 5110.030 ms
(8 rows)

```

如果规划器选择了顺序扫描主路径而没有选择任何子路径，查询计划会如下所示：

```sql
postgres=# explain analyze select * from test where a > 500000;
                          QUERY PLAN
------------------------------------------------------------
 Seq Scan on test  (cost=0.00..676994.40 rows=39571047 width=6) (actual time=0.011..7852.896 rows=39500000 loops=1)
   Filter: (a > 500000)
   Rows Removed by Filter: 500000
 Planning Time: 0.115 ms
 Execution Time: 9318.773 ms
(5 rows)
```

