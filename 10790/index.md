# 云数据库 PostgreSQL 日志管理及分析-操作指南-文档中心-腾讯云


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [慢查询](#h:6019344d-20aa-4954-b9ed-429bc6cf3cdc)
    - <span class="section-num">1.1</span> [功能说明](#h:d92eb969-c035-4657-ac55-97449699f838)
    - <span class="section-num">1.2</span> [监控视图](#h:48d6dc25-ade1-49c2-9ce0-01d0ecbd8a70)
    - <span class="section-num">1.3</span> [慢 SQL 列表](#h:07fca319-fb97-4c7b-a284-91bad80ce140)
    - <span class="section-num">1.4</span> [慢 SQL 统计分析](#h:e8ff9b3e-464b-436e-8cee-3fabc34a65b1)
    - <span class="section-num">1.5</span> [功能说明](#h:ec0e6696-823d-4eb2-8916-b979e9aa045b)
- <span class="section-num">2</span> [错误日志](#h:674b234e-48fd-42bf-a2ce-904622df68ef)
- <span class="section-num">3</span> [错误日志默认设置](#h:224bdb02-4654-4c42-963f-6e8ca391f4e9)
- <span class="section-num">4</span> [运行日志管理](#h:62c155e2-aeaa-4b3e-9e5e-e1c5ac54a187)
    - <span class="section-num">4.1</span> [PG_LOG 介绍](#h:a3fb6be9-2579-4a55-a635-798f91063de3)
    - <span class="section-num">4.2</span> [修改 PG_LOG 保留时长](#h:f07d52de-34f0-4c36-84ed-b1592578e807)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://cloud.tencent.com/document/product/409/10790

摘自：<https://cloud.tencent.com/document/product/409/49546>


## <span class="section-num">1</span> 慢查询 {#h:6019344d-20aa-4954-b9ed-429bc6cf3cdc}


### <span class="section-num">1.1</span> 功能说明 {#h:d92eb969-c035-4657-ac55-97449699f838}

默认数据库对超过 1s 的 SQL 语句查询判断为 “ **慢查询** ”，对应语句称为“ **慢查询语句** （俗
称 **慢 SQL** ）”，而数据库管理员（DBA）对慢查询语句进行分析并找到慢查询出现原因的过程叫做
**慢查询分析** 。﻿云数据库 PostgreSQL 控制台 在实例管理页的性能优化模块下，提供慢查询分析能
力。如下图：

{{< figure src="/ox-hugo/64ec9ef692ae5c01e9e4db61bac23b00.png" width="800px" >}}


### <span class="section-num">1.2</span> 监控视图 {#h:48d6dc25-ade1-49c2-9ce0-01d0ecbd8a70}

控制台中的两个图表为监控视图，可以直观方便地查看数据库慢 SQL 的相关信息。 慢查询与其他监
控组合视图：支持通过图表化形式查看慢查询指标以及与其他指标联合对比查看的能力。可以支持的
其他指标有：

-   CPU 利用率，
-   QPS，
-   请求数，
-   读请求数，
-   写请求数，
-   其他请求数，
-   缓冲区缓存命中率
-   平均执行时延。

慢 SQL 耗时分布： 可以通过不同时间段的慢 SQL 进行查看慢查询主要分布在什么时段区间内。


### <span class="section-num">1.3</span> 慢 SQL 列表 {#h:07fca319-fb97-4c7b-a284-91bad80ce140}

慢 SQL 列表可以实时查看数据库中所产生的慢 SQL 语句，列表根据时间降序排列，最新产生的慢
SQL 会自动生成显示在第一行。 支持查看的慢 SQL 字段：执行时间，慢 SQL 语句，总耗时，客户
端 IP 地址，数据库名，执行用户。

注意： 慢 SQL 列表默认保存最近 7 天的慢 SQL 数据与最大 50GiB 的日志总量，当哪一个条件率先满足，
则会自动删除超出要求的日志。当单条慢 SQL 大于 20KB，将无法从控制台上查看，请 提交工单，联
系腾讯云为您提供。


### <span class="section-num">1.4</span> 慢 SQL 统计分析 {#h:e8ff9b3e-464b-436e-8cee-3fabc34a65b1}

慢 SQL 统计分析可以根据指定时间范围内的所有慢 SQL 经过系统的参数抽象后，对同类 SQL 进行聚合分析得到的慢 SQL 分析信息。其
中包含多种字段信息。

-   **最后执行时间：** 在统计范围内，该抽象语句最后一次出现的时间，由于某些预计执行较长，我们统一按语句执行的 begin_time 记录。
-   **抽象后的 SQL 语句：** 去掉慢 SQL 中常数后的语句，抽象后的语句可以将同类同性质的语句进行汇总统计，以方便您更好的分析。
-   **数据库：** 该语句调用了哪个数据库。
-   **账号：** 该语句是使用哪个账号来运行的。
-   **客户端 IP 地址：** 语句在哪些客户端上执行过。
-   **首次执行时间：** 在统计范围内，该慢 SQL 第一次出现的时间（抽象后汇总后，可能有很多条记录）。
-   **执行总时间：** 在统计范围内，慢查询语句查询总耗时。
-   **平均执行时间：** 慢查询语句总时间除以总次数的平均时间。
-   **最小执行时间：** 所有该条抽象语句中，慢查询语句出现的最小时间；用于帮助判断该语句是否偶发性事件。
-   **最大执行时间：** 所有该条抽象语句中，慢查询语句出现的最大时间；用于帮助判断该语句是否偶发性事件。
-   **总时间占比：** 在统计范围内，慢查询语句占所有慢查询语句时间的占比。


### <span class="section-num">1.5</span> 功能说明 {#h:ec0e6696-823d-4eb2-8916-b979e9aa045b}


## <span class="section-num">2</span> 错误日志 {#h:674b234e-48fd-42bf-a2ce-904622df68ef}

数据库运行过程中，因操作、SQL、系统运行错误等原因产生的日志叫做错误日志，错误日志常用于
开发者找出业务系统或数据库出现问题的原因。

云数据库 PostgreSQL 在实例管理页的性能优化模块下，提供错误日志查看能力。如下图：

{{< figure src="/ox-hugo/3cc9ca2b764a79032fa9055618df448f.png" width="800px" >}}


## <span class="section-num">3</span> 错误日志默认设置 {#h:224bdb02-4654-4c42-963f-6e8ca391f4e9}

-   错误日志功能： 默认开启
-   错误日志记录级别： `log_min_error_statement=ERROR`
-   分析数据输出延迟： `1分钟 - 5分钟`
-   日志记录时长：  `7天` （最大最近 10000 条）


## <span class="section-num">4</span> 运行日志管理 {#h:62c155e2-aeaa-4b3e-9e5e-e1c5ac54a187}

:PROPERTIES:
:CUSTOM_ID: h:080465e3-1261-4a03-8ee6-0562db8aced3
:NOTER_DOCUMENT: <https://cloud.tencent.com/document/product/409/95763>
:NOTER_OPEN: eww
:NOTER_PAGE: 1


### <span class="section-num">4.1</span> PG_LOG 介绍 {#h:a3fb6be9-2579-4a55-a635-798f91063de3}

pg_log 一般是记录数据库的状态信息，例如：错误信息、慢查询 SQL、数据库的启动关闭信息等。
该日志会按大小和时间自动切割，当前云数据库 PostgreSQL 的 pg_log 默认保留 30 天。pg_log 会
占用数据库实例的存储空间，您可以基于实际需要来修改保留时长。

**说明：**
数据库实例的慢日志和错误日志默认保留 7 天，修改 pg_log 保留时长不影响慢日志和错误日志保留
时长。


### <span class="section-num">4.2</span> 修改 PG_LOG 保留时长 {#h:f07d52de-34f0-4c36-84ed-b1592578e807}

您可以基于实际需要修改 pg_log 保留时长，系统当前支持 7 天和 30 天两种。具体操作如下：

1.  登录 云数据库 PostgreSQL 控制台。
2.  在实例列表找到需要修改的实例，单击操作 &gt; 管理，进入实例详情。
3.  在实例详情页找到参数设置，在右上方搜索框中搜索需要设置的参数 log_filename 并更新。

log_filename 参数说明如下：

| 参数值               | 说明                        |
|-------------------|---------------------------|
| postgresql_%a_%H.log | 选择该值，日志保留 7 天。   |
| postgresql_%d_%H.log | 选择该值，日志保留 30 天，系统默认保留 30 天。 |

