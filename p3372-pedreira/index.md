# Velox: Meta’s Unified Execution Engine


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:2feba2b4-8c5e-43d1-a350-894f13c55f6d)
- <span class="section-num">2</span> [INTRODUCTION](#h:894af9fe-ef1a-4830-bb2d-66fa3658ed9a)
- <span class="section-num">3</span> [LIBRARY OVERVIEW](#h:45c2ace0-0c5d-4839-bd76-e8fc71c4a50e)
- <span class="section-num">4</span> [USE CASES](#h:02b95a98-47dd-4cc4-b09d-5e4961eb13ef)
- <span class="section-num">5</span> [DEEP DIVE](#h:28e04c87-3ab7-49ae-8dd2-2c9f60d16e72)
    - <span class="section-num">5.1</span> [<span class="org-todo todo TODO">TODO</span> Type System](#h:721c90b1-0101-4889-af09-0d80b80be023)
    - <span class="section-num">5.2</span> [Vectors](#h:c3f011fb-9660-4857-a677-9ccd96a0a439)
    - <span class="section-num">5.3</span> [Expression Eval](#h:a55899bf-019b-484b-9fa0-6ad73151ac95)
    - <span class="section-num">5.4</span> [Functions](#h:b4d3982d-525d-467a-88c2-a4ac41eabd2b)
    - <span class="section-num">5.5</span> [Operators](#h:424ea8dc-5e15-4660-be04-855440d1184e)
    - <span class="section-num">5.6</span> [Memory Management](#h:ea91d67f-7e9d-4ff3-8c27-62cc34336120)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/8/p3372-pedreira.pdf



## <span class="section-num">1</span> ABSTRACT {#h:2feba2b4-8c5e-43d1-a350-894f13c55f6d}

Velox:

-   C++ database acceleration library

-   用来：
    -   构建执行引擎
        -   应对复杂数据类型
    -   增强数据管理系统 (enhance data management system)

-   倚赖：
    -   向量化 (vectorization)
    -   自适应 (adaptivity)

-   Meta 内部已经或正在将其与其他组件集成，包括：
    -   分析型查询引擎
        -   Presto
        -   Spark
    -   流处理平台
    -   消息总线
    -   数据仓库
    -   机器学习
        -   PyTorch


## <span class="section-num">2</span> INTRODUCTION {#h:894af9fe-ef1a-4830-bb2d-66fa3658ed9a}

仅做计算，无 SQL 解析、优化器等，其价值：

-   效率
-   一致性
-   工程效率


## <span class="section-num">3</span> LIBRARY OVERVIEW {#h:45c2ace0-0c5d-4839-bd76-e8fc71c4a50e}

-   是什么
    -   开源 C++ 数据库加速库
    -   可用来加速、扩展和增强数据的计算引擎：
        -   高性能计算
        -   可重用
        -   可扩展

-   不是什么
    -   无语法前端
        -   SQL 解析
        -   全局优化等

-   也就意味着：
    -   Velox 的输入是 **已经优化好的执行计划**
    -   将执行计划在本地执行

<!--listend-->

-   Velox 的组件

<a id="figure--fig:screenshot@2022-10-11-09:31:24"></a>

{{< figure src="images/velox:-meta’s-unified-execution-engine/screenshot@2022-10-11_09:31:24.png" >}}


## <span class="section-num">4</span> USE CASES {#h:02b95a98-47dd-4cc4-b09d-5e4961eb13ef}


## <span class="section-num">5</span> DEEP DIVE {#h:28e04c87-3ab7-49ae-8dd2-2c9f60d16e72}


### <span class="org-todo todo TODO">TODO</span> <span class="section-num">5.1</span> Type System {#h:721c90b1-0101-4889-af09-0d80b80be023}

-   TypeSystem 用于表示各种数据类型：
    -   原生类型
        -   整形
        -   不同精度的浮点数类型
        -   字符串
            -   varchar
            -   varbinary
        -   日期
        -   时间戳
        -   函数 （lambda 表达式）

    -   复杂类型
        -   数组
        -   固定长度的数组
        -   maps
        -   rows, structs

-   上述类型可以嵌套，并序列化、反序列化
    -   还可以包装 C++ 的结构体

-   支持类型扩展：
    -   Find How????


### <span class="section-num">5.2</span> Vectors {#h:c3f011fb-9660-4857-a677-9ccd96a0a439}

-   Vectors 用来表示列式数据
    -   列式、编码后的数据
    -   用作组件之间的输入和输出
    -   扩展自 Apache Arrow 格式，扩展包括
        -   size （行数）
        -   type
        -   null bitmap
    -   可嵌套

<!--listend-->

-   Velox Buffers
    -   从内存池中分配出的连续空间
    -   Vector 保存在 Velox buffers 中

-   引用计数
    -   Buffer 和 Vector 都有引用计数
    -   一个 buffer 可以被多个 Vector 引用
    -   只有引用计数为 1 的数据是可变的
        -   shared vector 和 buffer 可通用 copy-on-write 技术变成可写


#### <span class="section-num">5.2.1</span> Arrow Comparison {#h:af0bc56f-4859-4da5-a2c1-f287f476d439}


### <span class="section-num">5.3</span> Expression Eval {#h:a55899bf-019b-484b-9fa0-6ad73151ac95}

-   表达式计算引擎，可用作
    -   过滤投影算子 &#x2013; 用于过滤和投影表达式
    -   TableScan 和 IO connectors: 过滤条件下推
    -   用作单独的计算组件：计算表达式

-   使用 Expression Tree 用作输入
    -   树的每个节点可能是
        -   input column
        -   常量
        -   函数调用，由函数名和一系列的参数（表达式）构成
        -   CAST 表达式：用于类型转换？
        -   lambda 函数

-   函数计算分成两个部分： 编译和执行


#### <span class="section-num">5.3.1</span> Compilation {#h:25b08418-5bdd-455b-8561-bb95feab581a}

将输入的表达式树转换成为可执行的表达式，若干运行时优化技术：

-   Common Subexpression Elimination
-   Constant Folding
-   Adaptive Conjunct Reordering


#### <span class="section-num">5.3.2</span> Evaluation. {#h:6bb78bba-3183-4e38-b9f9-1124608d46ca}


### <span class="section-num">5.4</span> Functions {#h:b4d3982d-525d-467a-88c2-a4ac41eabd2b}


#### <span class="section-num">5.4.1</span> Scalar Functions. {#h:fd105474-377e-4196-8a19-1ab3e367e7af}


#### <span class="section-num">5.4.2</span> Aggregate Functions {#h:06e0f855-5595-4139-af01-90ad7d4791f4}


### <span class="section-num">5.5</span> Operators {#h:424ea8dc-5e15-4660-be04-855440d1184e}


#### <span class="section-num">5.5.1</span> Table Scans, Filter, and Project {#h:7d269783-7fd0-4801-8d8c-e3a5b3fe2feb}


#### <span class="section-num">5.5.2</span> Aggregate and Hash Joins. {#h:246de1ad-ba83-49f2-ba36-143633b69b64}


### <span class="section-num">5.6</span> Memory Management {#h:ea91d67f-7e9d-4ff3-8c27-62cc34336120}


#### <span class="section-num">5.6.1</span> Caching {#h:292ea0f6-123b-4663-99e5-786dbbafbc4b}

