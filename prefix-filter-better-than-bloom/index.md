# Prefix Filter: Better Than Bloom


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Abstract](#h:1786f904-c305-4cb3-9883-b0040924d2b1)
- <span class="section-num">2</span> [INTRODUCTION](#h:4f4c58e1-3516-4df3-84ee-a73bb2ec5903)

</div>
<!--endtoc-->


本文为摘录，原文为： ../pdf/d/p1311-even.pdf



## <span class="section-num">1</span> Abstract {#h:1786f904-c305-4cb3-9883-b0040924d2b1}


## <span class="section-num">2</span> INTRODUCTION {#h:4f4c58e1-3516-4df3-84ee-a73bb2ec5903}

-   What is a filter

    属于近似查询 (approximate membership query), 用于判断独具是否在集合中。

    -   如果输入在集合中，则必定输出 Yes
    -   如果不在集合中，则输出 Yes 的概率不能多于 \\(\Sigma\\)

<!--listend-->

-   What are filters used for
    -   与精确查询相比
        -   更省资源， 与数据量无关，而与 &Sigma; （误报率）有关
        -   适于放在内存中，用于真正操作之前来过滤掉无效数据
            -   比如 join

