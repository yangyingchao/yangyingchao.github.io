# SNARF: A Learning-Enhanced Range Filter


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:815ddced-2940-4ddc-a8f1-e53eeb9c4c7d)
- <span class="section-num">2</span> [INTRODUCTION](#h:4d4ce086-8979-4ce0-8e14-e0771827e7ad)
    - <span class="section-num">2.1</span> [Range Filters](#h:06c300b0-ac2c-4743-a09c-abb6ee673074)
- <span class="section-num">3</span> [SNARF: A LEARNED FILTER](#h:9ad4603f-9ca9-4dd4-a11b-086b2d5967fb)
    - <span class="section-num">3.1</span> [SNARF Description](#h:1952086c-7e65-4eee-b93d-38bb90429836)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/a/p1632-vaidya.pdf



## <span class="section-num">1</span> ABSTRACT {#h:815ddced-2940-4ddc-a8f1-e53eeb9c4c7d}

-   **SNARF**: Sparse Numerical Array-Based Range Filters
    基于稀疏数组的范围过滤器

-   用于数值类型的范围过滤


## <span class="section-num">2</span> INTRODUCTION {#h:4d4ce086-8979-4ce0-8e14-e0771827e7ad}

-   Filters
    -   are space efficient but appropriate
    -   Answer membership queries on a set _S_

    -   Point Filters
        -   eg BloomFilter
        -   Support point queries:<br />
            "Is _x_ in the set _S_ ?"

    -   Range Filters (可用作范围过滤 where + 单表)
        -   for range query: <br />
            "Is there a key in the set S in between values `p` &amp; `q`?"

    -   允许误报


### <span class="section-num">2.1</span> Range Filters {#h:06c300b0-ac2c-4743-a09c-abb6ee673074}

-   range filters can significantly improve the performance of systems for synthetic and real-world workloads
-   e.g RocksDB, SQL Server
    -   [Adaptive Range Filters for Cold Data: Avoiding Trips to Siberia （TBD）](~/Documents/Database/pdf/f/p1714-kossmann.pdf)


## <span class="section-num">3</span> SNARF: A LEARNED FILTER {#h:9ad4603f-9ca9-4dd4-a11b-086b2d5967fb}


### <span class="section-num">3.1</span> SNARF Description {#h:1952086c-7e65-4eee-b93d-38bb90429836}


#### <span class="section-num">3.1.1</span> SNARF Construction: {#h:372a603b-d9c3-4743-aa50-752da1e4dc32}

-   给定一组 keys \\[S=\\{x\_1, x\_2, ...,x\_n\\}\\] , 构建一个可以进行范围查询的过滤器
-   SNARF 通过单调函数 \\[f\\] 将 keys 映射进一个 bit array B:
    -   B 中有 \\[|B| = K \times n\\] 个比特
-   初始化时候，数组中所有元素均为 0
-   将 B 中的某些位置置位: \\[f(x\_i) = 1\\] , 其中： \\[x\_i \in S\\]
-   隐射函数为： \\[f(x) = \lfloor MCDF(x) \times nK\rfloor \\]
    -   `MCDF` 为 S 中 key 的 CDF 经验值的单调估计

-   SNARF 使用压缩：提升空间效率：

<a id="figure--fig:screenshot@2022-10-18-18:25:55"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-18_18:25:55.png" width="800px" >}}


#### <span class="section-num">3.1.2</span> SNARF Range Query: {#h:cddfe2b4-5b71-4ebd-885f-d7c67ab06373}

