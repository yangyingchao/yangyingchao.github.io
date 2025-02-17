# Hybrid Blockchain Database Systems: Design and Performance


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [ABSTRACT](#h:9f78e1f8-2b12-4076-aac1-e9e70da3d5c4)
- <span class="section-num">2</span> [INTRODUCTION](#h:9a37547d-d53e-4acb-870b-1a94f6f35306)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/e/p1092-loghin.pdf



## <span class="section-num">1</span> ABSTRACT {#h:9f78e1f8-2b12-4076-aac1-e9e70da3d5c4}

Abbrs:

-   CFT: crash fault-tolerant
-   BFT: [byzantine fault-tolerant](https://river.com/learn/terms/b/byzantine-fault-tolerance-bft/) <br />
    Byzantine Fault Tolerance (BFT) is a trait of decentralized, permissionless
    systems which are capable of successfully identifying and rejecting
    dishonest or faulty information. Byzantine fault tolerant systems have
    successfully solved the Byzantine Generals Problem and are robust against
    sybil attacks.


## <span class="section-num">2</span> INTRODUCTION {#h:9a37547d-d53e-4acb-870b-1a94f6f35306}

-   学术界出现了集成分布式数据库与区块链特性的系统
-   这些系统称为 hybrid blockchain database systems, 混合区块数据库系统，包含：
    -   向区块链中增加数据库特性
        -   改进性能和易用性
    -   在分布式数据库中实现区块链特性
        -   提升安全性

-   BigchainDB introduces two optimizations
    -   Blockchain Pipelining
        -   allows nodes to vote for a new block while the current block of the ledger is still undecided.
        -   在当前账本的 block 没有确定的时候，就可以为新 block 投票？
        -   避免由等待 block 完全提交而阻塞产生新的 block
    -   支持并行验证
        -   可以在多核 CPU 上并行验证事务
        -   作者说，然并卵。。。

