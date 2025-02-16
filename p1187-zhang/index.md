# NBTree: a Lock-free PM-friendly Persistent B+-Tree for eADR-enabled PM Systems


本文为摘录，原文为： attachments/pdf/a/p1187-zhang.pdf

-   **PM** stands for Persistent Memory， 持久内存，它是一种非易失性内存，即使断电也能保留数据。
-   **eADR** 保证了 CPU 缓存中的数据在电源故障时会被刷新到 PM 中
    -   这使得在 eADR 启用的 PM 系统中，全局可见数据被认为是持久的
    -   不再需要显式的数据清除操作

-   **NBTree**
    -   a lock-free PM-friendly B+ Tree
    -   实现了高可扩展性和低 PM 开销
    -   NBTree is the first persistent index designed for eADR-enabled PM systems. <br />
        是第一个专为启用 eADR 的 PM 系统设计的持久索引
    -   To achieve lock-free, NBTree uses atomic primitives to serialize leaf node operations. <br />
        NBTree 使用原子操作来序列化叶节点操作
    -   NBTree proposes four novel techniques to enable lock-free access to the leaf during struc-

tural modification operations (SMO), including three-phase SMO,
sync-on-write, sync-on-read, and cooperative SMO. For inner node
operations, we develop a shift-aware search algorithm to resolve
read-write conflicts. To reduce PM overhead, NBTree decouples
the leaf nodes into a metadata layer and a key-value layer. The
metadata layer is stored in DRAM, along with the inner nodes, to
reduce PM accesses. NBTree also adopts log-structured insert and
in-place update/delete to improve cache utilization. Our evaluation
shows that NBTree achieves up to 11× higher throughput and 43×
lower 99% tail latency than state-of-the-art persistent B+ -Trees

。。eADR 技术的出现为构建无锁数据结构带来了独特的机会，释放了 PM 的全部潜力。本文提出了
NBTree，一个无锁 PM 友好型 B+树，。NBTree

。为了实现无锁，。此外，NBTree 提出
了四种新技术，在结构修改操作(SMO)期间使 Leaf 无锁访问，包括三阶段 SMO、写时同步、读时同步和
协作 SMO。对于内部节点操作，我们开发了一种具有移位感知的搜索算法来解决读写冲突。为了减少
PM 开销，NBTree 将叶节点分为元数据层和键值层。元数据层与内部节点一起存储在 DRAM 中，以减少对
PM 的访问。NBTree 还采用了基于日志的插入和原地更新/删除来提高缓存利用率。评估结果表明，
NBTree 的吞吐量比最先进的持久性 B +树高 11 倍，并且 99%的尾延迟比最先进的持久性 B+树低 43 倍。

这篇文章介绍了一种新的持久化索引结构——NBTree，它是一种基于 B+树的锁无关、PM 友好的数据结构，
旨在充分发挥 eADR 技术的性能和持久化优势。通过使用原子操作来串行化叶节点操作，NBTree 实现了
锁无关。此外，该文章还提出了四种新技术，包括三阶段 SMO、写同步、读同步和协同 SMO，以实现锁
无关的访问叶节点。对于内部节点操作，NBTree 开发了一种基于移位的搜索算法来解决读写冲突。为
了降低 PM 开销，NBTree 将叶节点分离为元数据层和键值层，其中元数据层存储在 DRAM 中，以降低 PM 访
问次数。NBTree 还采用基于日志的插入和原地更新/删除来提高缓存利用率。实验表明，NBTree 在
YCSB 工作负载下的吞吐量可以达到比现有持久化 B+树高 11 倍，99%尾延迟低 43 倍。最后，该文章还介
绍了持久性内存和 eADR 技术的优劣点以及现有持久化索引结构的常见问题。

