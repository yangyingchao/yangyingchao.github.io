# 100x Faster Query in Aurora Postgres with a lower random_page_cost


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [What is Random Page Cost?](#h:cb668550-2490-4cf4-829b-f5a60b54d00e)
- <span class="section-num">2</span> [Diving Deeper](#h:5d83a798-845a-44a1-bd42-21c0f9df4678)
- <span class="section-num">3</span> [Seeing is Believing: Before and After Examples](#h:e146b6ad-c339-44e4-9526-5b7eac05fdbd)
    - <span class="section-num">3.1</span> [Before (RPC = 4.0):](#h:bedef465-1168-44ae-81a7-15c7753fe209)
    - <span class="section-num">3.2</span> [After (RPC = 1.1):](#h:eb107038-e92d-4378-b00a-9524adc18a99)
- <span class="section-num">4</span> [can perform it on a per query/connection basis](#h:b0b819dc-d3f1-40d1-8063-ae77226960ec)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://postgr.es/p/6oe

最近我在 Postgres 中处理一些查询，发现它要么选择不使用索引执行顺序扫描，要么选择使用复合部分索引的替代索引。这
让我感到困惑，尤其是知道系统中有可以更快执行这些查询的索引时。

为什么会这样呢？

经过一些研究，我偶然发现了 `random_page_cost` 参数。


## <span class="section-num">1</span> What is Random Page Cost? {#h:cb668550-2490-4cf4-829b-f5a60b54d00e}

想象在图书馆里寻找一本特定的书。按顺序阅读书籍就像数据库中的顺序扫描，而直接跳转到所需书
籍就像随机访问。random_page_cost 反映了数据库中随机访问相对于顺序访问的成本。当
random_page_cost 较高时，Postgres 认为随机获取数据很昂贵，因此决定按顺序阅读所有内容（检查
每一本书架上的书）代替。这个设置还会影响它是否决定使用完整索引还是部分或复合索引。我们将
在下文中详细了解更多内容。


## <span class="section-num">2</span> Diving Deeper {#h:5d83a798-845a-44a1-bd42-21c0f9df4678}

魔法在于 PG 如何使用这个值。较高的 RPC（random_page_cost）会抑制索引的使用，更倾向于顺序扫
描。在 Aurora Postgres 14.11 中，默认的 random_page_cost 目前为 4.0。然而，使用现代存储技
术，随机访问的成本远低于传统旋转硬盘。通过根据这个实际情况调整 RPC，我们可以促使引擎更有
效地利用索引，可能导致查询速度显著提高。


## <span class="section-num">3</span> Seeing is Believing: Before and After Examples {#h:e146b6ad-c339-44e4-9526-5b7eac05fdbd}

让我们通过一个具体但匿名化的例子来说明降低 RPC 的影响。在这里，我将使用 EXPLAIN ANALYZE
命令来比较在相同查询上调整 RPC 前后的查询计划。这个 LATERAL JOIN 查询旨在找到与名为
table_1 的表关联的有限数量的行，按其 ID 排序，同时排除已完成的条目，其 completed_at 具有值
/非空值。

现在，有两个索引存在 - 一个是标准索引 table_2_pkey，它是在 table_2 的主键上的索引。第二个是
一个在 table_2 上的复合部分索引，其中索引在 table_1_id、id 上，并且在 (completed_at IS NULL)
条件下。


### <span class="section-num">3.1</span> Before (RPC = 4.0): {#h:bedef465-1168-44ae-81a7-15c7753fe209}

使用默认的 RPC，查询的执行时间大约为~11 秒。显然，这个时间非常长，对我们来说不可扩展。执行计划如下所示：

```sql
Nested Loop  (cost=0.42..95.02 rows=1000 width=117) (actual time=0.115..11286.991 rows=1000 loops=1)
  Buffers: shared hit=88534
  ->  Seq Scan on table_1  (cost=0.00..5.20 rows=2 width=8) (actual time=0.017..0.023 rows=2 loops=1)
        Filter: (id = ANY ('{150,250}'::bigint[]))
        Rows Removed by Filter: 14
        Buffers: shared hit=5
  ->  Limit  (cost=0.42..34.91 rows=500 width=117) (actual time=5149.715..5643.379 rows=500 loops=2)
        Buffers: shared hit=88529
        ->  Index Scan using table_2_pkey on table_2  (cost=0.42..20413.13 rows=295949 width=117) (actual time=5149.713..5643.321 rows=500 loops=2)
              Filter: ((completed_at IS NULL) AND (table_1_id = table_1.id))
              Rows Removed by Filter: 146339
              Buffers: shared hit=88529
Planning:
  Buffers: shared hit=1
Planning Time: 0.204 ms
Execution Time: 11287.091 ms

```

这里，PG 在 table_1 表上执行了顺序扫描，然后使用完整索引在 table_2 表上执行索引扫描。
这是因为 PG 确定了使用默认 RPC 值 4.0 时，复合索引的成本更高，如上所述。


### <span class="section-num">3.2</span> After (RPC = 1.1): {#h:eb107038-e92d-4378-b00a-9524adc18a99}

现在，通过降低 RPC 以更好地反映存储系统的能力，PG 选择使用复合索引，因为它认为这样更轻松。

现在 PG 能够在不到 2 毫秒的时间内执行相同的查询（!!）

```sql
Nested Loop  (cost=0.56..57.04 rows=1000 width=117) (actual time=0.063..1.401 rows=1000 loops=1)
  Buffers: shared hit=344
  ->  Index Only Scan using table_1_pkey on table_1  (cost=0.14..1.67 rows=2 width=8) (actual time=0.013..0.017 rows=2 loops=1)
        Index Cond: (id = ANY ('{150,250}'::bigint[]))
        Heap Fetches: 2
        Buffers: shared hit=3
  ->  Limit  (cost=0.42..17.69 rows=500 width=117) (actual time=0.032..0.585 rows=500 loops=2)
        Buffers: shared hit=341
        ->  Index Scan using idx_complete_table_2_on_table_1_id_id_where_completed_at_is_nul on table_2  (cost=0.42..10022.74 rows=290287 width=117) (actual time=0.030..0.531 rows=500 loops=2)
              Index Cond: (table_1_id = table_1.id)
              Buffers: shared hit=341
Planning:
  Buffers: shared hit=127
Planning Time: 0.647 ms
Execution Time: 1.496 ms


```


## <span class="section-num">4</span> can perform it on a per query/connection basis {#h:b0b819dc-d3f1-40d1-8063-ae77226960ec}

```ruby
class ApplicationRecord < ActiveRecord::Base
  # ...
  def with_minimized_page_cost(&block)
    ActiveRecord::Base.connection.exec_query("SET random_page_cost=1.1")
    yield
  ensure
    ActiveRecord::Base.connection.exec_query("RESET random_page_cost;")
  end
  # ...
end

..
ApplicationRecord.with_minimized_page_cost do
  # Perform your queries here
end

```

