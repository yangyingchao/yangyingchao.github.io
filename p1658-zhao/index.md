# QueryFormer: A Tree Transformer Model for Query Plan Representation


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [INTRODUCTION](#h:183f9122-54a5-402b-a116-7c3d46e8969d)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/0/p1658-zhao.pdf

QueryFormer:

-   learning-based query planer representation model
-   with tree-structured Transformer architecture

-integrate histograms from database into query plan encoding


## <span class="section-num">1</span> INTRODUCTION {#h:183f9122-54a5-402b-a116-7c3d46e8969d}

-   Physical Query Plan As DAG (Directed Acyclic Graph)

    <a id="figure--fig:screenshot@2022-11-03-11:43:04"></a>

    {{< figure src="/ox-hugo/screenshot@2022-11-03_11:43:04.png" caption="<span class=\"figure-number\">Figure 1: </span>Example query and query plan" >}}

    -   node 表示操作
    -   edge 表示方向
    -   子节点先执行，执行结果给父节点作为输入

-   Physical Query Plan 作为机器学习的输入，用以数据库优化

