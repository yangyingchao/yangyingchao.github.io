# The Design and Implementation of Modern Column-Oriented Database Systems


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#h:1830f5bb-c40e-4e6c-bb49-b7c5a663eafb)
    - <span class="section-num">1.1</span> [Virtual IDs](#h:d0aa820f-c598-42ba-a0d8-3b91fc66b3c1)
    - <span class="section-num">1.2</span> [Block-oriented and vectorized processing \\\\](#h:27970ca7-fdc5-47b7-8e35-b5681d23c7e9)
    - <span class="section-num">1.3</span> [Late materialization 晚期物化](#h:316f88be-6095-440c-824d-2854f8f5ed3b)
    - <span class="section-num">1.4</span> [Column-specific compression](#h:6d9d9f56-20b9-46a0-af3c-a97c37122b77)
    - <span class="section-num">1.5</span> [Direct operation on compressed data](#h:024e71c4-1d2b-4d76-a57f-a1b4c3432355)
    - <span class="section-num">1.6</span> [Efficient join implementations](#h:a07de8d4-2df1-4961-963d-54c00cb99139)
    - <span class="section-num">1.7</span> [Redundant representation of individual columns in dif- ferent sort orders](#h:0b550a82-fc3d-4ca3-b8b2-a07cbf678b41)
    - <span class="section-num">1.8</span> [Database cracking and adaptive indexing](#h:68b92fa7-1e6b-4e68-b046-49b994359f86)
    - <span class="section-num">1.9</span> [Efficient loading architectures](#h:4705376a-f8ab-4314-b009-22b9fc0b6ef8)
- <span class="section-num">2</span> [Column-store internals and advanced techniques](#h:54e3a749-27e2-46db-8446-154b812ae2b7)
    - <span class="section-num">2.1</span> [Vectorized Processing 向量化处理](#h:0cdfb744-7374-4e23-b52e-2bf01165cf66)
    - <span class="section-num">2.2</span> [Compression](#h:32d8ed7a-1090-4fb3-a9f4-7aaeec0cb330)
    - <span class="section-num">2.3</span> [Operating Directly on Compressed Data 压缩态计算](#h:bcd324ec-1990-42c8-9b51-87ad6da5869b)
    - <span class="section-num">2.4</span> [Late Materialization](#h:7f251c9e-5241-4615-8478-dafb2890693c)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/d/The Design and Implementation of Modern Column-Oriented Database Systems (abadi-column-stores).pdf



## <span class="section-num">1</span> Introduction {#h:1830f5bb-c40e-4e6c-bb49-b7c5a663eafb}


### <span class="section-num">1.1</span> Virtual IDs {#h:d0aa820f-c598-42ba-a0d8-3b91fc66b3c1}

通过固定大小来存储数据，省掉存储 ID 的开销

{{< figure src="/ox-hugo/screenshot@2023-03-02_11:35:50.png" >}}


### <span class="section-num">1.2</span> Block-oriented and vectorized processing \\\\ {#h:27970ca7-fdc5-47b7-8e35-b5681d23c7e9}

-   CPU 效率和 Cache 使用率更高
    -   算子间传递多个 tuple 组成的 block
    -   每个 block 大小为 cache size 大小
    -   每个 block 中一般包含多个记录
    -   自动向量化（编译器 + CPU）


### <span class="section-num">1.3</span> Late materialization 晚期物化 {#h:316f88be-6095-440c-824d-2854f8f5ed3b}

-   延迟将多列 join 成宽表的时机


### <span class="section-num">1.4</span> Column-specific compression {#h:6d9d9f56-20b9-46a0-af3c-a97c37122b77}


### <span class="section-num">1.5</span> Direct operation on compressed data {#h:024e71c4-1d2b-4d76-a57f-a1b4c3432355}

尽量让数据以压缩方式存储在内存中，对其进行操作，直到必需的时候再解压给外层。


### <span class="section-num">1.6</span> Efficient join implementations {#h:a07de8d4-2df1-4961-963d-54c00cb99139}


### <span class="section-num">1.7</span> Redundant representation of individual columns in dif- ferent sort orders {#h:0b550a82-fc3d-4ca3-b8b2-a07cbf678b41}


### <span class="section-num">1.8</span> Database cracking and adaptive indexing {#h:68b92fa7-1e6b-4e68-b046-49b994359f86}


### <span class="section-num">1.9</span> Efficient loading architectures {#h:4705376a-f8ab-4314-b009-22b9fc0b6ef8}


## <span class="section-num">2</span> Column-store internals and advanced techniques {#h:54e3a749-27e2-46db-8446-154b812ae2b7}


### <span class="section-num">2.1</span> Vectorized Processing 向量化处理 {#h:0cdfb744-7374-4e23-b52e-2bf01165cf66}


### <span class="section-num">2.2</span> Compression {#h:32d8ed7a-1090-4fb3-a9f4-7aaeec0cb330}


#### <span class="section-num">2.2.1</span> Run-length Encoding {#h:7a8bef04-ec63-4b0f-98c5-28f5d6455534}


#### <span class="section-num">2.2.2</span> Bit-Vector Encoding {#h:378fdd15-95c9-430f-8cec-5b8ed03c7b37}


#### <span class="section-num">2.2.3</span> Dictionary {#h:9b466ee6-89ac-439a-bc8c-670d6992a676}


#### <span class="section-num">2.2.4</span> Frame Of Reference (FOR) {#h:2675f355-c316-4d73-ad41-d9d9ea6c2fdd}


#### <span class="section-num">2.2.5</span> The Patching Technique {#h:ab2a36d8-0a3c-4de9-8871-6a944e2feee8}


### <span class="section-num">2.3</span> Operating Directly on Compressed Data 压缩态计算 {#h:bcd324ec-1990-42c8-9b51-87ad6da5869b}

-   This benefit is magnified for compression schemes like run length encoding that combine multiple values within a column inside a single compression symbol.
-   Operating directly on compressed data requires modifica- tions to the query execution engine.


### <span class="section-num">2.4</span> Late Materialization {#h:7f251c9e-5241-4615-8478-dafb2890693c}

