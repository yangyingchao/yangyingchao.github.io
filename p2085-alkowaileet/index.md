# Columnar Formats for Schemaless LSM-based Document Stores


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [INTRODUCTION](#h:bbb4a7c2-b4b5-4e59-8249-fb5bda5443a0)
- <span class="section-num">2</span> [BACKGROUND](#h:1064047d-344e-44ee-8062-5592b5fc7541)
    - <span class="section-num">2.1</span> [Apache AsterixDB](#h:7837d119-448b-4eec-ac87-912ee0df6bdf)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： attachments/pdf/f/p2085-alkowaileet.pdf

这篇论文探讨了文档存储数据库系统的数据模型灵活性限制其在列式关系数据库中进行分析工作负载的表现。作者提出了几种基于Log-Structured Merge (LSM) tree事件的技术，旨在将数据存储在列式布局中。他们扩展了Dremel格式并引入一个新的列式布局来组织和存储数据。论文还强调了在文档存储中使用查询编译技术的潜力，并介绍了他们在Apache AsterixDB中的实现和评估。实验结果显示了显著的性能增益，并在最小程度上影响了摄入性能。


## <span class="section-num">1</span> INTRODUCTION {#h:bbb4a7c2-b4b5-4e59-8249-fb5bda5443a0}

本文介绍了一种新的数据存储方案，为用户提供了以列格式存储数据时获得性能提升的同时不牺牲文档存储的灵活性。作者对Dremel格式进行了扩展以解决文档存储的灵活数据模型所带来的限制问题，从而允许具有异构类型和模式更改的值。另外，利用LSM树架构在写性能上的优势，结合[22]提出的技巧，利用LSM刷写操作来推断模式并将记录以行格式写入，然后再利用Dremel扩展格式将其作为列格式写入硬盘。整个方案既满足了用户的性能期望，也保留了文档存储的灵活性。

-   这篇文章提出了一种新的模型，可以将列保存在 LSM B+ 树的索引中。
-   在这个模型中，将B+树的叶节点拓展为“超级节点”，其中一个叶节点可以占据多个页面。
-   我们将此模型称为AsterixDB Mega-Attributes Across 或者简称 AMAX。

尽管它的名字是AMAX，但是AMAX与列的结构无关，将每一列视为一系列字节，因此只需要进行少量修改就可以被其他基于LSM的文档存储所接受。在本文中，我们评估了AMAX模型的摄取性能和查询性能。在扩展版本[23]中，我们提出了一个类似于分区属性跨度（PAX）格式[21]
的APAX格式，其中每个列占据B+树叶子页面中的连续区域（称为“小页面”）。由于篇幅限制，我们省略了APAX格式的详细信息，如有兴趣，请参阅[23]获取更多信息。


## <span class="section-num">2</span> BACKGROUND {#h:1064047d-344e-44ee-8062-5592b5fc7541}


### <span class="section-num">2.1</span> Apache AsterixDB {#h:7837d119-448b-4eec-ac87-912ee0df6bdf}


#### <span class="section-num">2.1.1</span> Storage Engine: {#h:1107d4cf-6e25-4ddd-8a1c-1842ed2f81cc}

-   其存储引擎使用基于 LSM 树的 B+ 树索引来存储记录
-   新插入的记录使用主键进行哈希分区，被插入到相应的分区的主LSM内存组件中。
-   当内存组件已满时:
    -   会将其中的记录刷新到一个新的 LSM 磁盘组件中，
    -   并在其元数据页上设置一个有效性位，释放内存组件以服务后续插入。
-   LSM 磁盘组件是不可变的:
    -   因此更新和删除操作需要插入新的条目
    -   删除操作会添加一个“反物质”条目来指示已删除具有指定键的记录
    -   更新操作只需加入一个新的记录 - 包括更新的值 - 具有与原始记录相同的键。
    -   新添加的记录将替换旧记录。因此，基于LSM的文档存储将此操作称为“upsert”，因为不支持记录中值的就地部分更新。
-   合并策略周期性地在后台将磁盘组件合并成较大的组件，以决定何时和如何进行合并。
-   删除和更新的旧版本记录在合并操作期间进行垃圾回收

