# Chimp: Efficient Lossless Floating Point Compression for Time Series Databases


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:9d71575b-ba21-4317-b723-f2c924610db6)
- <span class="section-num">2</span> [INTRODUCTION](#h:402fb889-e9f7-4816-83ac-5edfbc857d66)
- <span class="section-num">3</span> [PRELIMINARIES](#h:4d0e1916-a961-460c-b04b-37bf987e0d1d)
    - <span class="section-num">3.1</span> [Floating Point Time Series](#h:eb874834-0cf9-4a3d-a6c2-37fe0153d2de)
    - <span class="section-num">3.2</span> [IEEE 754 Double Precision Floating Point Format](#h:87f9b9e3-a90d-404a-9c99-aaa46efe6355)
    - <span class="section-num">3.3</span> [Gorilla Compression](#h:b534c224-1e44-453d-8ac1-5d03f6b7e9a7)
- <span class="section-num">4</span> [PROPERTIES OF REAL-WORLD TIME SERIES](#h:e61dbfe2-662e-4db1-8de6-33ccd572b0a1)
    - <span class="section-num">4.1</span> [Trailing Zeros](#h:50b69b70-1b4b-4e2c-8c37-8d851fb920ce)
    - <span class="section-num">4.2</span> [Leading Zeros](#h:06702259-3ebc-4672-b24b-d7f28c4eaedd)
    - <span class="section-num">4.3</span> [Revisiting Gorilla Compression](#h:8c80901f-346f-4b1d-ba11-411436fd4516)
- <span class="section-num">5</span> [OVERVIEW](#h:3d2a78fa-6eed-4738-8d3d-5d4771119d8c)
    - <span class="section-num">5.1</span> [Our Chimp Algorithm](#h:2ff4f7f7-de49-4982-86c5-e33535c188eb)
- <span class="section-num">6</span> [效果对比](#h:7aa7a756-dc81-4885-822b-de57dc9a59aa)
    - <span class="section-num">6.1</span> [Compression size result](#h:2bd0b50a-9cc3-478d-90d2-e3a44cefe53e)
    - <span class="section-num">6.2</span> [Compression and decompression time](#h:7127ad57-9105-42b2-ac2c-0cfb641b6946)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/1/p3058-liakos.pdf



## <span class="section-num">1</span> ABSTRACT {#h:9d71575b-ba21-4317-b723-f2c924610db6}

-   时序数据难以高效存储，导致存储代价高昂。
-   **通用压缩** 技术可以减少数据大小，但给计算带来额外开销。
    -   通常不能忍受
-   通常采用快速、 **流式压缩** 将数据进行编码
    -   该做法无法完全使用压缩的潜力

-   Chimp
    -   新型流式压缩算法
    -   适用于时间序列的浮点数运算
    -   压缩比与通用算法相当，比目前标准的流式压缩比高 50%
    -   压缩、解压时间更短


## <span class="section-num">2</span> INTRODUCTION {#h:402fb889-e9f7-4816-83ac-5edfbc857d66}

-   TSMS: Time Series Management Systems
-   TSMS 压缩存储浮点数据的方法：
    -   将当前值与前一时刻的值进行异或运算 （XOR）
    -   得到的值中，大概率很多的 bit 会是 0:
        -   因为数据一般不会突然变化很大
-   我们发现，相邻数据 XOR 的结果：
    -   0 通常不会出现在结果的尾部
    -   而是出现在头部


## <span class="section-num">3</span> PRELIMINARIES {#h:4d0e1916-a961-460c-b04b-37bf987e0d1d}


### <span class="section-num">3.1</span> Floating Point Time Series {#h:eb874834-0cf9-4a3d-a6c2-37fe0153d2de}

-   时间序列， Time series （TS）
    -   一系列的数据点
    -   数据点是一对 **时间戳** 和 **值**
    -   数据点按照时间递增排序
    -   \\[TS=\langle(\\,t\_1, v\_1  )\\,,(\\,t\_2, v\_2  )\\, ,...	\rangle \\]
        -   \\(t\_i\\) 表示时间戳
        -   \\(v\_i\\) 表示值

-   Bounded Time series
    -   特殊的时间序列
    -   拥有固定个数的时间序列
        \\[TS=\langle(\\,t\_1, v\_1  )\\,,...,(\\,t\_n, v\_n )\\, 	\rangle \\]

-   Floating Point Time series
    -   特殊的时间序列
    -   \\[TS=\langle(\\,t\_1, v\_1  )\\,,(\\,t\_2, v\_2  )\\, ,...	\rangle \\] <br />
        满足：
        -   \\[v\_i \in \mathbb{R}\\]
        -   \\[i \in \mathbb{N}\\]


### <span class="section-num">3.2</span> IEEE 754 Double Precision Floating Point Format {#h:87f9b9e3-a90d-404a-9c99-aaa46efe6355}

<a id="figure--fig:screenshot@2022-10-19-10:56:21"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-19_10:56:21.png" >}}

-   双精度浮点数的格式
    -   符号位， Sign (S)
        -   1 bit
        -   `0`: 正
        -   `1`: 负
    -   指数位, biased exponent (E)
        -   11 bits
        -   偏移为 1023
    -   分数位, Fractional (F)
        -   52 bits
-   数学表示： <br />
    \\[x = (-1)^s \times 2 ^{(E-B)} \times 1.F\\]


### <span class="section-num">3.3</span> Gorilla Compression {#h:b534c224-1e44-453d-8ac1-5d03f6b7e9a7}

-   Gorilla 的变长编码：
    -   第一个数值不压缩
    -   后续数值与前一个做 XOR:
        -   结果为 `0` （即两者相等）， 则存 `0`
        -   如果不为 `0` ：则存 `1` ， 后接：
            -   控制位 `0` ： <br />
                当前有效位数在前者有效位数范围内，即：

                1.  当前值（XOR 后）的 leading zeros 的个数大于前值的 leading zeros, 且
                2.  当前值的 trailing zeros 的个数大于前值的 trailing zeros

                此情况下使用前值的信息，在控制位后进保存 XOR 后的有效数值

            -   控制位 `1` ： <br />
                -   使用接下来的 5 bits 来保存 leading zero 的个数
                -   使用接下来的 6 bits 来保存 XOR 结果的有效数值长度
                -   最后保存 XOR 结果的有效数值

<a id="figure--fig:screenshot@2022-10-20-15:11:01"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-20_15:11:01.png" width="600px" >}}

<a id="figure--fig:screenshot@2022-10-20-15:44:31"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-20_15:44:31.png" >}}


## <span class="section-num">4</span> PROPERTIES OF REAL-WORLD TIME SERIES {#h:e61dbfe2-662e-4db1-8de6-33ccd572b0a1}


### <span class="section-num">4.1</span> Trailing Zeros {#h:50b69b70-1b4b-4e2c-8c37-8d851fb920ce}


### <span class="section-num">4.2</span> Leading Zeros {#h:06702259-3ebc-4672-b24b-d7f28c4eaedd}


### <span class="section-num">4.3</span> Revisiting Gorilla Compression {#h:8c80901f-346f-4b1d-ba11-411436fd4516}


#### <span class="section-num">4.3.1</span> Flag Bits. {#h:59848c4d-b8b4-48f4-a79e-c3496fc4bfc1}

前面介绍 Gorilla 的 flag bits 有三种： `0` ， `10` 和 `11` 。其中 `0` 表示当前值和前面的值相等。
然而这种情形并不常见。

如果能够使用最少的比特位来表示最常见的情况，则应该能够提升压缩比。


#### <span class="section-num">4.3.2</span> Length of Meaningful XORed Value (Center Bits). {#h:4b0ff4ff-e36d-4311-a8ae-9d885561f080}


#### <span class="section-num">4.3.3</span> Previous Block Position. {#h:792dd029-00aa-4f31-b158-791e6ae0029a}


## <span class="section-num">5</span> OVERVIEW {#h:3d2a78fa-6eed-4738-8d3d-5d4771119d8c}


### <span class="section-num">5.1</span> Our Chimp Algorithm {#h:2ff4f7f7-de49-4982-86c5-e33535c188eb}


#### <span class="section-num">5.1.1</span> Possible Flag Sequences {#h:6fe58ee9-803c-4b85-821f-fd84ae589b56}

之前的研究表明相邻数据完全相同的概率并不大，使用最少的比特位来表示最常见的情况，能够提升压缩比。

<a id="figure--fig:screenshot@2022-10-20-15:49:41"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-20_15:49:41.png" >}}

<a id="figure--fig:screenshot@2022-10-20-15:50:47"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-20_15:50:47.png" >}}


## <span class="section-num">6</span> 效果对比 {#h:7aa7a756-dc81-4885-822b-de57dc9a59aa}


### <span class="section-num">6.1</span> Compression size result {#h:2bd0b50a-9cc3-478d-90d2-e3a44cefe53e}

<a id="figure--fig:screenshot@2022-10-20-15:53:33"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-20_15:53:33.png" width="800px" >}}


### <span class="section-num">6.2</span> Compression and decompression time {#h:7127ad57-9105-42b2-ac2c-0cfb641b6946}

<a id="figure--fig:screenshot@2022-10-20-15:53:56"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-20_15:53:56.png" >}}

