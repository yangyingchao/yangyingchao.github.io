# Adaptive Range Filters for Cold Data: Avoiding Trips to Siberia


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:925d40ea-36be-4f97-92e1-5a9583172847)
- <span class="section-num">2</span> [INTRODUCTION](#h:a4fb225f-a49e-4a9e-927c-a0173212264d)
- <span class="section-num">3</span> [APPLICATION EXAMPLE](#h:a555d6ac-abe7-471a-872e-af21f9cfa13c)
    - <span class="section-num">3.1</span> [Project Siberia](#h:e89b04c8-78da-49f9-ae2b-4b9de35e7442)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/f/p1714-kossmann.pdf

对目前的架构来讲，和 **粗糙索引** 作用重叠。。


## <span class="section-num">1</span> ABSTRACT {#h:925d40ea-36be-4f97-92e1-5a9583172847}

-   **ARF**: Adaptive Range Filter, 自适应范围过滤器
-   ARF is for Range queries, while
-   BloomFilter is for Point queries


## <span class="section-num">2</span> INTRODUCTION {#h:a4fb225f-a49e-4a9e-927c-a0173212264d}


## <span class="section-num">3</span> APPLICATION EXAMPLE {#h:a555d6ac-abe7-471a-872e-af21f9cfa13c}


### <span class="section-num">3.1</span> Project Siberia {#h:e89b04c8-78da-49f9-ae2b-4b9de35e7442}

-   Siberia 是 Hekaton 项目用于管理冷数据的一个项目
    -   Hekaton 则是 SQL Server 套件中的内存数据库

<!--listend-->

-   查询处理

<a id="figure--fig:screenshot@2022-10-19-08:47:07"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-19_08:47:07.png" >}}

-   Hot Store
    -   用于查询热数据

-   Cold Store
    -   用于查询冷数据
    -   标准的 SQL Server 表

-   冷热查询对客户端透明
    -   需要时候由 QueryProcessor 对两种数据的结果进行聚合操作

-   冷数据的查询代价很高

-   filter 用于判定冷数据中是否有符合条件的数据
    -   返回 true:  可能有，需要访问冷数据
    -   返回 false: 肯定无，无需访问冷数据

