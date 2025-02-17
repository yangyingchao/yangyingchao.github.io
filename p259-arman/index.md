# Origami: A High-Performance Mergesort Framework


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:30779f14-6c09-4a57-b1f2-6c8b6224a589)
- <span class="section-num">2</span> [INTRODUCTION](#h:b9af59de-065f-47e9-a832-feaa4e164435)
- <span class="section-num">3</span> [PIPELINE OVERVIEW](#h:926a68c8-a7d2-4920-849c-b7ecbfc1027e)
    - <span class="section-num">3.1</span> [notation](#h:06b60011-63b4-4407-ab8d-f7118e58b203)
    - <span class="section-num">3.2</span> [better cache utilization,](#h:3fbe405a-7293-42f3-9b91-282093bc312a)
    - <span class="section-num">3.3</span> [4 Phases](#h:78f26a30-9807-4abb-97e0-5908a368852a)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/c/p259-arman.pdf



## <span class="section-num">1</span> ABSTRACT {#h:30779f14-6c09-4a57-b1f2-6c8b6224a589}

Mergesort 优点：

-   不受数据倾斜影响
-   适于通过向量化执行来进行并行处理
-   适于多线程操作操作

Origami:

-   内存归并排序算法框架
-   对每个向量化指令集，提供寄存器内排序算子 （in-register sorter），小数据量性能提供 8 倍
-   branchless streaming merge 1.5 倍提升


## <span class="section-num">2</span> INTRODUCTION {#h:b9af59de-065f-47e9-a832-feaa4e164435}

MergeSort 的优点：

-   对数据分布不敏感 (distribution insensitivity): constant time on all inputs
-   支持流式操作 (streaming operation) ， 适用于外存数据，或者分布式数据
-   适用于多核并行计算


## <span class="section-num">3</span> PIPELINE OVERVIEW {#h:926a68c8-a7d2-4920-849c-b7ecbfc1027e}


### <span class="section-num">3.1</span> notation {#h:06b60011-63b4-4407-ab8d-f7118e58b203}

-   `N` ： 待排序的元素数量
-   `C` ： 能够装进 L2 Cache 的数量 （约 `2^16 ~ 2^18` ）
-   `T` ： 线程数量，常为 CPU 核数的二倍
-   `W` ： 每个 SIMD 寄存器可以装下的元素数量 （约 `4 ~ 16` ）
-   `R` ： 每个 CPU 核上 SIMD 寄存器的个数 （通常 `16 or 32` ）
-   `B` ： 每个元素的 bits 数 （ `32, 64, or 128` ）


### <span class="section-num">3.2</span> better cache utilization, {#h:3fbe405a-7293-42f3-9b91-282093bc312a}

-   L2 Cache <br />

    -   为更好的利用 CPU Cache ， 通常将输入划分成大小为 `C` 的块 （block） ，然后在 Cache 对其排序，

    从而生成 `N/C` 个有序列表。

    -   然后使用 `k` 路合并 （ `k>=2` ） ，生成最终的有序列表


### <span class="section-num">3.3</span> 4 Phases {#h:78f26a30-9807-4abb-97e0-5908a368852a}

MergeSort 可以分解为四个阶段，标记为 `P_1 ~ P_4`


#### <span class="section-num">3.3.1</span> (P1) Tiny sorters {#h:db3bca44-ab63-41c6-a13a-136eef61de2e}

-   将整个输入转换成有序的子列的过程
-   子列长度为 `m` ， ( `m>=2=` ):
    -   子列内容为 `[im, (i+1)m)`
    -   `i = 10,1,...,C/m-1`
-   MergeSort 对 P1 阶段效率不高 （长度小于 128 时）
    -   可使用其他排序算法
        -   insert sort
        -   SIMD generalizations


#### <span class="section-num">3.3.2</span> (P2) In-cache merge {#h:a9f73f60-f31b-49d2-af8d-53bba565528a}

-   P1 结束后，每个子队列都是有序的
-   P2 由 \\(log\_2(C/m)\\) 个二元合并 (binary merge) 构成


#### <span class="section-num">3.3.3</span> (P3) Out-of-cache independent merge {#h:bf125cb3-f18b-4bf9-a01d-b0f58527be69}

