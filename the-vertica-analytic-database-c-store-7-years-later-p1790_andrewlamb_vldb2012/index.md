# The Vertica Analytic Database- C-Store 7 Years Later


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:7d7533f5-6893-4cc5-bf49-528e2f05cb24)
- <span class="section-num">2</span> [BACKGROUND](#h:38bfadd0-2466-44b4-bcee-50563df8d649)
    - <span class="section-num">2.1</span> [Design Overview](#h:b92b62b5-cf6d-48e1-9f0d-09634656132c)
- <span class="section-num">3</span> [DATA MODEL](#h:8c127add-721e-4ce9-8210-eb475565182a)
    - <span class="section-num">3.1</span> [Projections](#h:5818aee4-1caa-47bf-ae26-56ef2b0d2bc8)
    - <span class="section-num">3.2</span> [Join Indexes](#h:9e309991-d0d5-4c52-9e3b-9aa55cd040ab)
    - <span class="section-num">3.3</span> [Prejoin Projections](#h:fc16fc4d-95f3-43b2-a0df-72e301c1c280)
    - <span class="section-num">3.4</span> [Encoding and Compression](#h:b05f07d2-c047-43a0-b329-eb3859a65070)
    - <span class="section-num">3.5</span> [Partitioning](#h:e73ca29a-39ae-43a2-a12c-41ee05b754da)
    - <span class="section-num">3.6</span> [Segmentation: Cluster Distribution](#h:62e3ee39-fe90-434c-909e-1c5877396c87)
    - <span class="section-num">3.7</span> [Read and Write Optimized Stores](#h:77c7ffc8-afd0-4118-93e3-bc54a105568f)
- <span class="section-num">4</span> [TUPLE MOVER](#h:4e7a0195-059c-4502-ad9f-57d2ccf20a8c)
- <span class="section-num">5</span> [QUERY EXECUTION](#h:fd7b1b73-4e68-4442-971e-f89255025ef2)
    - <span class="section-num">5.1</span> [Query Operators and Plan Format](#h:5dc5c12e-3fc0-4c27-aef8-7ac7c097e387)
    - <span class="section-num">5.2</span> [Query Optimization](#h:110fd5c5-957a-47b8-a225-fdbcb6ac93dc)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/3/The Vertica Analytic Database- C-Store 7 Years Later (p1790_andrewlamb_vldb2012).pdf



## <span class="section-num">1</span> ABSTRACT {#h:7d7533f5-6893-4cc5-bf49-528e2f05cb24}

-   Vertica 是 C-Store 的商业化成果


## <span class="section-num">2</span> BACKGROUND {#h:38bfadd0-2466-44b4-bcee-50563df8d649}


### <span class="section-num">2.1</span> Design Overview {#h:b92b62b5-cf6d-48e1-9f0d-09634656132c}


#### <span class="section-num">2.1.1</span> Design Goals {#h:e3e72698-a717-47d3-a742-5f668cf4dd48}

-   Designed for **analytic workloads** rather than for **transactional workloads**
    -   **Transactional workloads** are characterized by:
        -   a large number of transactions per second (e.g. thousands)
        -   事务性工作负载指的是每秒钟有大量的交易（比如数千次），
        -   each transaction involves a handful of tuples.
        -   每个交易仅涉及几个元组。
        -   most of the transactions take the form of single row insertions or modifications to existing rows.
        -   大多数交易采用单行插入或修改现有行的形式。
        -   例如，插入新的销售记录或更新银行帐户余额。

    -   **Analytic workloads** are characterized by:
        -   smaller transaction volume (e.g. tens per second),
        -   每秒钟的交易量较小（例如每秒钟几十次），
        -   each transaction examines a significant fraction of the tuples in a table.
        -   但每个交易都会检查表中相当一部分的元组。
        -   例如，跨时间和地理维度聚合销售数据以及分析网站上不同用户的行为。

-   Share nothing storage

-   尽量进行本地计算： <br />
    优化器， 执行器避免通过网络搬运大量数据

-   加载，尤其是批量加载，要快


## <span class="section-num">3</span> DATA MODEL {#h:8c127add-721e-4ce9-8210-eb475565182a}


### <span class="section-num">3.1</span> Projections {#h:5818aee4-1caa-47bf-ae26-56ef2b0d2bc8}

-   它将表数据物理地组织成投影
    -   这些投影是表属性的排序子集
    -   可以允许使用任意数量的具有不同排序顺序和表列子集的投影。


### <span class="section-num">3.2</span> Join Indexes {#h:9e309991-d0d5-4c52-9e3b-9aa55cd040ab}

未使用 C-Store 中使用的连接索引


### <span class="section-num">3.3</span> Prejoin Projections {#h:fc16fc4d-95f3-43b2-a0df-72e301c1c280}


### <span class="section-num">3.4</span> Encoding and Compression {#h:b05f07d2-c047-43a0-b329-eb3859a65070}


### <span class="section-num">3.5</span> Partitioning {#h:e73ca29a-39ae-43a2-a12c-41ee05b754da}


### <span class="section-num">3.6</span> Segmentation: Cluster Distribution {#h:62e3ee39-fe90-434c-909e-1c5877396c87}


### <span class="section-num">3.7</span> Read and Write Optimized Stores {#h:77c7ffc8-afd0-4118-93e3-bc54a105568f}

-   ROS: Read Optimized Store
-   WOS: Write Optimized Store


#### <span class="section-num">3.7.1</span> Data Modifications and Delete Vectors {#h:1c17ad0f-22c4-415d-8207-643132887a38}

-   A delete vector is a list of positions of rows that have been deleted.


## <span class="section-num">4</span> TUPLE MOVER {#h:4e7a0195-059c-4502-ad9f-57d2ccf20a8c}


## <span class="section-num">5</span> QUERY EXECUTION {#h:fd7b1b73-4e68-4442-971e-f89255025ef2}


### <span class="section-num">5.1</span> Query Operators and Plan Format {#h:5dc5c12e-3fc0-4c27-aef8-7ac7c097e387}


### <span class="section-num">5.2</span> Query Optimization {#h:110fd5c5-957a-47b8-a225-fdbcb6ac93dc}

