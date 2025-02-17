# UDO: Universal Database Optimization using Reinforcement Learning


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:6b1b4a85-ca06-4a51-99d2-2f06dcb728fc)
- <span class="section-num">2</span> [INTRODUCTION](#h:c5af75eb-2734-48dd-9281-7ce95ec73184)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/7/p3402-wang.pdf



## <span class="section-num">1</span> ABSTRACT {#h:6b1b4a85-ca06-4a51-99d2-2f06dcb728fc}

-   UDO:
    -   Universal Database Optimization, 通用数据库优化
    -   为数据库的某种工作负载所做的特殊优化
    -   包含多种调优：从索引选择到系统参数

-   基于强化学习
    -   达到近似优化配置
    -   创建不同的配置，并通过实际执行评估不同的配置的效果
        -   而不是简单的代价模型


## <span class="section-num">2</span> INTRODUCTION {#h:c5af75eb-2734-48dd-9281-7ce95ec73184}

-   迭代模型
    -   选择配置
    -   创建
    -   评估
    -   迭代

-   增强学习
    -   用来决定后续使用哪个配置

