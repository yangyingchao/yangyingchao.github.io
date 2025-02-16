# Manu: A Cloud Native Vector Database Management System


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:5628f2fa-3be2-4312-8bb1-d8da17d2d39a)
- <span class="section-num">2</span> [INTRODUCTION](#h:21580be0-85a4-4596-838b-f76c80a9b8c8)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/8/p3548-yan.pdf



## <span class="section-num">1</span> ABSTRACT {#h:5628f2fa-3be2-4312-8bb1-d8da17d2d39a}

下一代向量数据库应该有的特性：

-   可长期演进
-   tunable consistency
-   good elasticity
-   high performance

Manu 通过放宽数据模型和一致性约束来换取上述特性：

-   使用 WAL 和 binlog 作为服务的骨干
-   写组件作为日志的发布者
-   只读组件作为日志的订阅者，只读组件包括：
    -   分析
    -   搜索
-   使用 MVCC 和增量一致模型来简化组件之间的通讯和协作


## <span class="section-num">2</span> INTRODUCTION {#h:21580be0-85a4-4596-838b-f76c80a9b8c8}

