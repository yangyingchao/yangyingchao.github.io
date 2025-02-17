# New Wine in an Old Bottle: Data-Aware Hash Functions for Bloom Filter


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [INTRODUCTION](#h:c589c3c7-e91c-477d-8ee8-7566656fbd63)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/4/p1924-bhattacharya.pdf

FPR: Fault Positive Rate

Partitioned Bloom Filter ,

-   方法
    -   将 BloomFilter 划分成 segments
    -   每个 segment 使用简单的、基于投影的哈希函数，
        -   函数通过数据计算得来

-   效果
    -   减少误报 （两个数量级）
    -   相同误报率的话，提升 50% 的压缩率


## <span class="section-num">1</span> INTRODUCTION {#h:c589c3c7-e91c-477d-8ee8-7566656fbd63}

-   Projection Hash Bloom Filter (PHBF)

