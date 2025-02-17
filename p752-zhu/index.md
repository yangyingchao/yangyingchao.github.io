# Cardinality Estimation in DBMS: A Comprehensive Benchmark Evaluation


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:99f62695-bea1-44e6-a3cd-a59d66b52b61)
- <span class="section-num">2</span> [INTRODUCTION](#h:c4df1980-9a57-45c5-8df2-1cc7599852e8)
- <span class="section-num">3</span> [PRELIMINARIES AND BACKGROUND](#h:d1e8dc3c-7673-4a57-8b58-a136863d747e)
- <span class="section-num">4</span> [EVALUATION PLAN](#h:16d48905-7e57-4b5f-9ddf-d8df956c4adc)
    - <span class="section-num">4.1</span> [CardEst Algorithms](#h:9580240e-b925-462d-89f7-e3e1fd6a1f9c)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/2/p752-zhu.pdf



## <span class="section-num">1</span> ABSTRACT {#h:99f62695-bea1-44e6-a3cd-a59d66b52b61}

-   we compare the effectiveness of CardEst methods in a real DBMS
-   We establish a new benchmark for CardEst, which:
    -   contains a new complex realworld dataset STATS and
    -   a diverse query workload STATS-CEB.

-   We integrate multiple most representative CardEst methods into PostgreSQL,
    -   and evaluate their true effectiveness in improving query plan quality,
    -   and other important aspects affecting their applicability.

-   We obtain key findings for the CardEst methods, under different data and query settings
-   We propose a new metric P-Error to evaluate the performance of CardEst methods
    -   which overcomes the limitation of Q-Error and
    -   is able to reflect the overall end-to-end performance of CardEst methods.

We have made all of the benchmark data and evaluation code publicly available at
<https://github.com/Nathaniel-Han/End-to-End-CardEst-Benchmark>.


## <span class="section-num">2</span> INTRODUCTION {#h:c4df1980-9a57-45c5-8df2-1cc7599852e8}

-   Performance of CardEst has a critical impact on the quality of the query plans.
-   The core task of CardEst: build a compact model capturing data and/or query information.

-   Traditional CardEst Method, mainly use two traditional CardEst methods:
    -   histogram (PG, SQL Server)
    -   sampling  (MySQL, MariaDB)

-   Learned methods (ML) for CardEst
    -   可以分为两类：
        -   query-driven （查询驱动）： 将特征化的查询映射到它们的基数
        -   data-driven （数据驱动） ： 直接对所有属性的联合分布进行建模

    -   与传统的基数估计相比，更加准确：
        -   模型更复杂
        -   粒度更细

    -   尽管如此，仍有两个短处：
        -   用于训练的数据和查询负载，并不能够很好的代表实际查询
            -   Therefore, the existing works are not sufficient to reflect the behavior of
                CardEst methods on complex real-world data with high skewness and correlations
                and multi-table queries with various join forms and conditions.

        -   大部分的评估并未展示 CardEst 方法在查询优化器上的端到端改进
            -   现有的工作通常评估 CardEst 方法的算法级指标，如估计精度和推理延迟
            -   这些指标仅能评估 CardEst 算法本身的质量，而不能用于评估这些方法在数据库中起到的真实作用，原因有二：
                1.  评估的准确不代表作出的计划就好 <br />
                    不同的子查询对整个计划影响很大，一个评估很准确的算法，也可能因错估导致关键的评估而导致很差的计划
                2.

<!--listend-->

-   We establish a new benchmark for CardEst that can represent real-world settings.


## <span class="section-num">3</span> PRELIMINARIES AND BACKGROUND {#h:d1e8dc3c-7673-4a57-8b58-a136863d747e}

In this section, we introduce some preliminaries and background, including a formal
definition of the cardinality estimation problem, a brief review on representative
CardEst algorithms and a short analysis on existing CardEst benchmarks.

The CardEst
problem requires estimating Card as accurately as possible without executing Q on T
. CardEst solutions for n. Whereas, CardEst methods for “LIKE” queries concern on
applying NLP techniques to summarize semantic information in strings.

Statistical CardEst methods can not well support “LIKE” queries. Traditional CardEst methods, such as histogram and sampling , are widely applied in DBMS and generally based on simplified assumptions and heuristics. Lots of variants are proposed later to enhance their performance. DBMS .

Pos tId=

Po kernel-based methods , index based methods and random walk based methods . Some other work, such as the sketch based method , explores a new routine for CardEst. ML-based query-driven CardEst methods try to learn a model to map each featurized query Q to its cardinality Card directly. Some ML-enhanced methods improve the performance of CardEst methods by using more complex models such as DNNs or gradient boosted trees .

ML-based data-driven CardEst methods are independent of the queries. To this end, we establish our benchmark on a new real-world dataset with a hand-picked query workload. It overcomes the drawbacks of existing CardEst benchmarks and fully fulfills the properties listed above. We describe the details on the data and workload settings in the following content.

It is an anonymized dump of user-contributed content on the Stats Stack Exchange network. STATS consumes 658MB storage space with 8 tables and 71 n. Whereas, STATS has richer and more diverse join types varying in the number of joined tables , join forms , and join keys non-equal joins as they rarely occur in practice and many CardEst algorithms process them in the same way as many-to-many joins. We manually check and retain each join template if it has occurred in the log data of StackExchangeor has its real-world semantics.

In the second phrase after deriving these 70 join templates, we generate 146 queries with 1 − 4 queries for each template as the testing query workload STATS-CEB. We make sure all the generated filter predicates reflect real-world semantics and diversify in multiple perspectives. The largest query in STATS-CEB has true cardinality of 20 billion, which is 3 larger than that of the JOB-LIGHT benchmark. predicates join type true cardinality range.


## <span class="section-num">4</span> EVALUATION PLAN {#h:16d48905-7e57-4b5f-9ddf-d8df956c4adc}

We aim to evaluate how CardEst algorithms behave in a real DBMS, including the
end-to-end improvement on optimizing query plans and other practicality aspects, on
our new benchmark.


### <span class="section-num">4.1</span> CardEst Algorithms {#h:9580240e-b925-462d-89f7-e3e1fd6a1f9c}

We identify and choose twelve representative CardEst algorithms across the three classes:

-   traditional
-   ML-based, query-driven
-   ML-based, data-driven

PostgreSQL refers to the histogram-based CardEst method used in the well-known
DBMS PostgreSQL

We use the implementation provided by authors in to generate the sketches and partition the data with 4096 buckets. Different from , we apply the same setting as to populate the bound sketches for queries with selection predicates, which would improve the estimation accuracy and generate better plans. We do not compare with the other variants of traditional methods as they do not exhibit significantly better performance or provide open-source implementation. Specifically, the tree-structured SPN contains sum node to split PT to multiple PT ′ on tuple subset T ′ T , product node to decompose PT to S PT for independent set of attributes S and leaf node if PT is a univariate PDF.

Then, the multi-leaf node is used to model the multivariate PDF PT UAE extends the UAE-Q method by unifiying both query and data information using the auto-regression model. It is a representative work aiming at closing the gap between data-driven and query-driven CardEst methods. Otherwise, we run a grid search to explore the combination of value that largely improves the endto-end performance on a validation set of queries. Notice that, there have also been proposed some CardEst modules that are optimized together with other components in a query optimizer in an end-to-end manner.

We do not compare with them as they do not fit our evaluation framework. To make our evaluation more realistic and convincing, we integrate each CardEst algorithm into the query optimizer of PostgreSQL , a well-recognized open-source DBMS.

Implementation and System Settings

CardEst method can be directly reflected by the end-to-end query runtime with their injected cardinality estimation. For example, the subplan queries A, B, and C only touch a single table, their CardEst results may affect the selection of table-scan methods, i.e. Joining A ▷◁ B with C or A ▷◁ C with B, and the join method, i.e. Therefore, the effects of a CardEst method on the final query execution plan are entirely decided by its estimation results over the sub-plan query space.

“calc_joinrel_size_estimate” in the planner of PostgreSQL to derive the sub-plan query space for each query in the workload. Then, we call each CardEst method to estimate the cardinalities of the sub-plan queries and inject the estimations back into PostgreSQL. Afterward, we run the compiler of PostgreSQL on Q to generate the plan. It will directly read the injected cardinalities produced by each method.

CardEst method without a large modification on the source code of PostgreSQL. SQL query using any CardEst method. Intel Xeon Updating speed reflects the time cost for CardEst models update to fit the data changes. For real-world settings, this metric plays an important role as its underlying data always updates with tuples insertions and deletions.

Besides these metrics, proposed some qualitative metrics related to the stability, usage, and deployment of CardEst algorithms and made a comprehensive analysis. In the following, we first evaluate the overall endto-end performance of all methods in Section 5.

HOW GOOD ARE CARDEST METHODS?

In this section, we first thoroughly investigate the true effectiveness of the aforementioned CardEst methods in improving query plan quality. Our evaluation focuses on a static environment where data in the system has read-only access. This setting is ubiquitous and critical for commercial DBMS, especially in OLAP workloads of data warehouses .

Overall End-to-End Performance

We evaluate the end-to-end performance on both JOB-LIGHT and STATS-CEB benchmarks for all CardEst methods including two baselines PostgreSQL and TrueCard shown in Table 3.

each of these CardEst methods. The astonishing performance of these ML-based data-driven CardEst methods come from their accurate characterization of data distributions and reasonable independence assumption over joined tables. Traditional histogram and sampling based methods have worse performance than PostgreSQL whereas the new traditional approach is significantly better. The query-driven CardEst methods’ performance is not stable.

They rely on a large amount of executed queries as training data and the testing query workload should follow the same distribution as the training workload to produce an accurate estimation . We observe that the execution time for CardEst method that can outperform PostgreSQL on JOB-LIGHT are all roughly 3.2h, which is very close to the minimal execution time of TrueCard. Section 3, the data distributions in the simplified IMDB dataset and the JOB-LIGHT queries are relatively simple. However, their performance differences on STATS are very drastic because the STATS dataset is much more challenging with high attribute correlations and various join types.

Therefore, the STATS-CEB benchmark can help expose the advantages and drawbacks of these methods. MultiHist and UniSample use the join uniformity assumption to estimate join queries, whose estimation error grows rapidly for queries joining more tables. However, as the cardinality increases with the number of joined tables, the relatively small sample size can not effectively capture the data distribution, leading to large estimation error. Therefore, these methods tend to yield poor join orders and long-running query plans.

The PostgreSQL produces more accurate estimations because of its high-quality implementation and fine-grained optimizations on join queries. The new traditional method PessEst has a significant improvement over the PostgreSQL because it can compute the upper bound on estimated cardinalities to avoid expensive physical join plans. As a result, the estimation accuracy, so as the execution time, of PessEst largely improves.

Analysis of ML-based Query-driven CardEst Methods: Overall the query-driven methods have comparable performance to the

PostgreSQL baseline. Specifically, MSCN can slightly outperform the PostgreSQL , LW-XGB has much slower query runtime, and LWNN has comparable performance. The unsatisfactory performance of these methods could be due to the following reasons. These methods are essentially trying to fit the probability distributions of all possible joins in the schema, which has superexponential complexity.

Besides, it is unreasonable to assume that a CardEst method can have access to this amount of executed queries in reality. The well-known workload shift issue states that query-driven methods trained one query workload will not likely produce an accurate prediction on a different workload . Therefore, the training and testing workload of these methods have different distributions. NeuroCardE , methods , do consistently outperform PostgreSQL by 7−13% on JOB-LIGHT.

NeuroCardE , the other three improve the PostgreSQL by 37−48% on

STATS-CEB. Their performance indicates that data-driven methods could serve as a practical counterpart of the PostgreSQL CardEst component. We conjecture that an effective CardEst method should make appropriate independent assumptions for large datasets. JOB-LIGHT to STATS-CEB benchmark for the following reasons.

First, the STATS dataset contains significantly more attributes with larger domain size, which can be detrimental to NeuroCardE ’s underlying deep auto-regressive models . Specifically, the full outer join size can get up to 3 1016 and an affordable training data sample size would be no larger than 3 108 . Third, the join keys in STATS dataset have very skewed distribution. Therefore, NeuroCardE can hardly capture the correct data distributions especially for join tables with small cardinalities.

All other three data-driven CardEst methods can significantly outperform the PostgreSQL baseline because their models are not constructed on the full outer join of all tables. Then, BayesCard, DeepDB, and FLAT build a model to represent the distribution of the corresponding small part. This approach solves the drawback of NeuroCardE , yields relatively accurate estimation, and produces effective query execution plans. DeepDB mostly because the STATS dataset is highly correlated, so the FSPN in FLAT has a more accurate representation of the data distribution than the SPN in DeepDB.

On the other hand, BayesCard has an even more accurate representation of data distribution and yields the best end-to-end time for most queries in STATS-CEB.

Analysis of Different Query Settings

In this section, we further examine to what extent the CardEst methods improve over PostgreSQL on various query types, i. Since JOB-LIGHT workload does not contain queries with very diverse types and the ML-based data-driven methods do not show significant difference on these queries, we only investigate queries on STATS-CEB. This observation suggests that the estimation qualities of these SOTA methods decline for queries joining more tables. In fact, the fanout join estimation approach adopted by all these methods sacrifices accuracy for efficiency by assuming some tables are independent of others.

This estimation error may accumulate for queries joining a large number of tables, leading to sub-optimal query plans. different cardinalities and investigate when a CardEst method could go wrong. The execution time of Q57 for TrueCard and FLAT is 1.90h and 1.92h, while the time for BayesCard is 3.23h. When choosing the join method in the root node of execution plans for Q57, BayesCard underestimates the final join size and chooses the “merge join” physical operation.

Ops: Hash join

UserId=u. UserId.

FLAT Plan b

“hash join” operation, which is twice as faster as the “merge join”. Since the final join operation takes up 99% of the total execution time, FLAT significantly outperforms BayesCard on this query. Generally, the query with larger true cardinality requires a longer time to execute. A bad estimation on these large subplan queries can have a detrimental result on the overall runtime, whereas a series of good estimations on small sub-plan queries will not influence the runtime as much.

Therefore, the estimation accuracy of sub-plan queries with very large true cardinalities dominate the overall quality of the query plan. As shown in Figure 2 BayesCard can generate the optimal join order of Q57 because of its near-perfect estimation of all sub-plan queries except for the one at the root node. Surprisingly, FLAT’s plan is roughly twice faster to execute than BayesCard’s plan due to the dominant large sub-plan query at the root node.

METHODS MATTER?

We only compare the recently proposed CardEst methods, which have been proved to significantly improve the PostgreSQL baseline, namely PessEst, MSCN, NeuroCardE , BayesCard, DeepDB, and FLAT.

O7: Inference latency can have a significant impact on the

Although their execution time on OLTP workload is faster than PostgreSQL, they have worse endto-end performance because of the long planning time. For OLAP workload of STATS-CEB, the CardEst methods’ planning time is much shorter than their execution time because OLAP workload contains extremely long-run queries. Therefore, we believe that CardEst methods targeting different workloads should fulfill different objectives.

NeuroCardE

Figure 3 reports the average inference latencies of all sub-queries in the workload for each method.

The newly proposed inference algorithms on BN provide BayesCard with a very fast and stable inference speed on both benchmarks.

Model Deployment

Figure 3 reports the model size and training time of all aforementioned methods. BayesCard, are generally interpretable and predictable, thus easy to debug for DBMS analytics. More importantly, a CardEst method friendly for system deployment should have faster training time and lightweight model size and BayesCard has the dominant advantage over the other ML-based data-driven methods in these two aspects because of its underlying Bayesian model. Specifically, from both training time and model size aspects, these methods can be ranked as BayesCard &gt; DeepDB &gt; FLAT &gt; NeuroCardE .

BayesCard preserves its underlying BN’s structure and only incrementally updates the model parameters. BayesCard’s underlying BN’s structure captures the inherent causality, which is unlikely to change when data changes. Therefore, BayesCard can preserve its original accuracy after model update .

Q-Error is a well-known metric to evaluate the quality of different

CardEst methods. Q-Error = max. Q-Error penalizes both overestimation and underestimation of the true cardinality. However, existing works have not investigated whether Q-Error is good evaluation metric for CardEst.

Execution Time the Q-Errors of

MSCN are significantly worse than PostgreSQL, but the execution time of MSCN largely outperforms it. This is particularly important as the DB communities have made great efforts in purely optimizing the Q-Error of CardEst methods, but sometimes neglect the ultimate goal of CardEst in DBMS. However, the Q-Error metric could not distinguish this difference and regard the estimation errors of all queries equally. This would cause the phenomenon that a more accurate estimation measured by Q-Error may lead to a worse query execution plan in reality.

For Q-Error, an estimation 1 for true cardinality of 10 has the same Q-Error as an estimation 1011 for true cardinality 1012 . For example, in Figure 2, the overall Q-Error of BayesCard over all sub-plan queries of Q57 is better than FLAT. However, only for the root query which matters most importantly to the query execution time, BayesCard fails to correctly estimate and leads to a much slower plan. For Q-Error, an underestimation 109 for true cardinality 1010 is the same as an overestimation of 1011 .

These two estimations are very likely to lead to different plans with drastically different execution time. As a result, Q-Error does not consider the importance of different sub-plan queries and may mislead the query plan generation. &#x2026;

An Alternative Metric: P-Error

Obviously, the best way to evaluate the quality of a CardEst method is to directly record its query execution time on some benchmark datasets and query workloads . A desirable metric should be fast to compute and simultaneously correlated with the query execution time. In the following, we propose the P-Error metric to fulfill this goal and quantitatively demonstrate that P-Error can be a possible substitute for Q-Error. Note that, given a query plan, the cost model of a DBMS could output an estimated cost, which is designed to directly reflect the actual execution time.

During the actual execution of this query plan, the true cardinalities of all sub-plan queries along this plan will be instantiated.

In P-Error, the effectiveness of a CardEst method’s estimation

The impact on the estimation error of each sub-plan query is reflected by its importance in generating the query plan P . However, this is not an issue as PPCP is identical to different CardEst methods, we could always compare their relative performance using P-Error no matter P is optimal or not. We can roughly see that methods with better runtime tend to have smaller P-Error . We also compute the correlation coefficients between the query execution time and Q-Error/P-Error.

Q-Error distribution w.r.t. Whereas, the value between 50% and 90% percentiles of P-Error distribution w.r.t. This indicates that P-Error is a better correspondence to the query execution time than Q-Error. In addition, P-Error is more convenient as it outputs a single value on the plan cost level whereas Q-Error outputs a value for each sub-plan query of Q. Therefore, P-Error makes an attempt to overcome the limitations of Q-Error and is shown to be more suitable to measure the actual performance of CardEst methods.

DISCUSSIONS AND CONCLUSIONS

This new benchmark helps to clearly identify the pros and cons of different CardEst methods. In addition, we propose the new metric P-Error as a potential substitute for the well-known Q-Error. DBMS community with a holistic view of the CardEst problem and help researchers design more effective and efficient CardEst methods. Admittedly, the query-driven methods are more general because they can handle complex string “LIKE” queries.

Therefore, researchers should develop CardEst methods that can produce accurate estimation for queries with large cardinalities instead of fine-grained estimation on extremely small ones. Since learning one large data-driven model on the full outer join of all tables has poor scalability, we believe an effective CardEst method should make appropriate independent assumptions and advocate researchers follow and improve the fanout methods first proposed by DeepDB . ML-based query-driven methods are inherently impractical for dynamic DBs with frequent data updates. CardEst methods with fast inference speed and effective update algorithms is also very important.

Alternatively, the newly proposed P-Error metric has better correspondence to the query performance and could serve as a better optimization objective for future researches.

REFERENCES

In SIGMOD. Exact cardinality query optimization for optimizer testing. In Proceedings of the 22nd acm sigkdd international conference on knowledge discovery and data mining.

IEEE transactions on Information Theory 14, 3 , Transaction Processing Performance Council. Transaction Processing Performance Council. SIGMOD Record 30, 2 , 199–210. Row Estimation Examples.

<//www.postgresql.org/docs/current/row-estimation-examples.html> . Selectivity estimation for range predicates using lightweight models. Compressed histograms with arbitrary bucket layouts for selectivity estimation. Masked autoencoder for distribution estimation.

Selectivity estimation using probabilistic models. Selectivity estimators for multidimensional range queries over real attributes.

The VLDB Journal 14, 2 , 137–154. An approach based on bayesian networks for query selectivity estimation. Multi-attribute selectivity estimation using deep learning. Self-tuning, gpu-accelerated kernel density models for multidimensional selectivity estimation. IEEE TKDE 27, 9 , 2377–2389. Variable Skipping for Autoregressive Range Density Estimation. Preventing bad plans by bounding the impact of cardinality estimation errors. VLDB Endowment 2, 1 , 982–993. Management of data.

In Proceedings of the 2020 ACM SIGMOD International Conference on Management

In 2020 IEEE 36th International Conference on Data Engineering Workshops. IEEE, 154–157. Conference on Performance Evaluation and Benchmarking. In 2018 IEEE 34th International Conference on Data Engineering . IEEE, 1453–1464. Selectivity estimation without the attribute value independence assumption. In VLDB, Vol.

Proceedings of the VLDB Endowment 14, 4 , 471–484. VLDB  Vladimir Svetnik, Andy Liaw, Christopher Tong, J Christopher Culberson, Robert P Sheridan, and Bradley P Feuston. Exact cardinality query optimization with bounded execution cost.

Lightweight graphical models for selectivity estimation without independence assumptions. A multi-dimensional histogram for selectivity estimation and fast approximate query answering. In Proceedings of the 2003 conference of the Centre for Advanced Studies on Collaborative research.

Peizhi

Data and Queries for Cardinality Estimation. SIGMOD International Conference on Management of Data.

