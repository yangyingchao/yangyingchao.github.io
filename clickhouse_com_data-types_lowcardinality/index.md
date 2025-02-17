# LowCardinality(T) | ClickHouse Docs


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [LowCardinality(T)](#lowcardinality--t)
    - <span class="section-num">1.1</span> [Syntax​](#syntax)
    - <span class="section-num">1.2</span> [Description​](#description)
    - <span class="section-num">1.3</span> [Example​](#example)
- <span class="section-num">2</span> [Use LowCardinality](#use-lowcardinality)
- <span class="section-num">3</span> [string optimization](#string-optimization)
    - <span class="section-num">3.1</span> [String storage in ClickHouse](#string-storage-in-clickhouse)
    - <span class="section-num">3.2</span> [Dictionary encoded strings](#dictionary-encoded-strings)
    - <span class="section-num">3.3</span> [Summary](#summary)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://clickhouse.com/docs/en/sql-reference/data-types/lowcardinality



## <span class="section-num">1</span> LowCardinality(T) {#lowcardinality--t}

<https://clickhouse.com/docs/en/sql-reference/data-types/lowcardinality>

Changes the internal representation of other data types to be dictionary-encoded.


### <span class="section-num">1.1</span> Syntax​ {#syntax}

```sql
LowCardinality(data_type)
```

Parameters:

-   `data_type` <br />
    -   `String`, `FixedString`, `Date`, `DateTime`, and `numbers` **excepting** `Decimal`.
    -   LowCardinality is not efficient for some data types, see the `allow_suspicious_low_cardinality_types` setting description.


### <span class="section-num">1.2</span> Description​ {#description}

LowCardinality is a superstructure that changes a data storage method and rules of data processing. ClickHouse
applies dictionary coding to LowCardinality-columns. Operating with dictionary encoded data significantly
increases performance of SELECT queries for many applications.

LowCardinality 是一种改变数据存储方法和数据处理规则的超级结构。ClickHouse 对 LowCardinality 列应用字典编码。使用字典编码的数据操作显著提高了许多应用程序的 SELECT 查询性能。

The efficiency of using LowCardinality data type depends on data diversity. If a dictionary contains less than
10,000 distinct values, then ClickHouse mostly shows higher efficiency of data reading and storing. If a
dictionary contains more than 100,000 distinct values, then ClickHouse can perform worse in comparison with
using ordinary data types.

使用 LowCardinality 数据类型的效率取决于数据的多样性。如果字典包含少于 10,000 个不同的值，那么 ClickHouse 在数据读取和存储方面通常显示出更高的效率。如果字典包含超过 100,000 个不同的值，那么 ClickHouse 的性能可能会比使用普通数据类型差。

Consider using LowCardinality instead of Enum when working with strings. LowCardinality provides more
flexibility in use and often reveals the same or higher efficiency.

在处理字符串时，可以考虑使用 `LowCardinality` 代替 `Enum` 。 `LowCardinality` 在使用上提供了更多的灵活性，并且通常展现出相同或更高的效率。


### <span class="section-num">1.3</span> Example​ {#example}

Create a table with a LowCardinality-column:

```sql
CREATE TABLE lc_t
(
    `id` UInt16,
    `strings` LowCardinality(String)
)
ENGINE = MergeTree()
ORDER BY id
```


## <span class="section-num">2</span> Use LowCardinality {#use-lowcardinality}

<https://www.tinybird.co/clickhouse/knowledge-base/low-cardinality>

The fewer bytes you read from disk, the faster your query.

If your column contains a limited set of repeated values (otherwise known as low cardinality, or low
unique-ness) you can optimize the storage of the value of this column by using the LowCardinality type.

The LowCardinality type encapsulates other data types, creating a dictionary of possible column values. Rather
than storing the raw value many, many times, instead, the column value is a key that points to the raw value in
a dictionary.

For example, if you had ColumnA with 5 possible unique values:

```sql
'Possible String Value 1'
'Possible String Value 2'
'Possible String Value 3'
'Possible String Value 4'
'Possible String Value 5'
```

Your table may contain 1 million rows, and 300,000 rows have the value 'Possible String Value 1' for ColumnA. If
you use a standard STRING type, you will store the entire value 'Possible String Value 1' 300,000 times, which
uses 25 bytes each time, for a total of 7.5 megabytes.

To optimize, you can set the column type to LowCardinality. Every unique value will be stored once, in a
dictionary table, for example:

```sql
1: 'Possible String Value 1'
2: 'Possible String Value 2'
3: 'Possible String Value 3'
4: 'Possible String Value 4'
5: 'Possible String Value 5'
```

Now, rather than each of the 300,000 rows storing 'Possible String Value 1' for ColumnA, instead, each of these
rows stores the key 1. When selecting ColumnA, you would retrieve the value for key 1 from the dictionary of
values.  Storing 1 as a 4 byte integer, 300,000 times, would result in a total of 1.2 megabytes, or a 6.25x
reduction in bytes.

Here's an example:

```sql
DROP DATABASE IF EXISTS lc;
CREATE DATABASE lc;
CREATE TABLE lc.base
ENGINE = MergeTree
ORDER BY (n1, s1) AS
SELECT
    number n1,
    [
        'The tattered work gloves speak of the many hours of hard labor he endured throughout his life.',
        'The glacier came alive as the climbers hiked closer.',
        'Nancy was proud that she ran a tight shipwreck.',
        'The ants enjoyed the barbecue more than the family.',
        'The bug was having an excellent day until he hit the windshield.'
    ][(rand() % 5) + 1] s1
FROM numbers(2000000);
CREATE TABLE lc.lowcard
ENGINE = MergeTree
ORDER BY (n1, s1) AS
SELECT
    n1,
    toLowCardinality(s1) s1
FROM lc.base;

select formatReadableSize(sum(bytes_on_disk)), count() from system.parts WHERE table = 'base' and active FORMAT PrettyCompact;
select formatReadableSize(sum(bytes_on_disk)), count() from system.parts WHERE table = 'lowcard' and active FORMAT PrettyCompact;
```

You can already see that the table using the LowCardinality column requires less disk space. You can confirm
this by doing a query and checking the statistics returned by ClickHouse.

```sql
localhost :) SELECT * FROM lc.base WHERE n1 = 10 ;

┌─n1─┬─s1───────────────────────────────────────────────────┐
│ 10 │ The glacier came alive as the climbers hiked closer. │
└────┴──────────────────────────────────────────────────────┘
1 rows in set. Elapsed: 0.006 sec. Processed 8.19 thousand rows, 646.57 KB (1.39 million rows/s., 110.00 MB/s.)
```

```sql
localhost :) SELECT * FROM lc.lowcard WHERE n1 = 10 ;
┌─n1─┬─s1───────────────────────────────────────────────────┐
│ 10 │ The glacier came alive as the climbers hiked closer. │
└────┴──────────────────────────────────────────────────────┘
1 rows in set. Elapsed: 0.004 sec. Processed 8.19 thousand rows, 74.09 KB (1.83 million rows/s., 16.51 MB/s.)
```

As expected, both queries are reading the same number of rows but the one not using LowCardinality is reading
almost 9 times more data!


## <span class="section-num">3</span> string optimization {#string-optimization}


### <span class="section-num">3.1</span> String storage in ClickHouse {#string-storage-in-clickhouse}

Downloadable from: <https://raw.githubusercontent.com/ClickHouse/clickhouse-presentations/master/meetup19/string_optimization.pdf>


#### <span class="section-num">3.1.1</span> External dictionaries {#external-dictionaries}

Store strings in a dictionary, indices in a table

-   Advantages
    -   Dynamically changeable set of strings
    -   No alterations (no problems)
    -   A variety of dictionary sources

-   Disadvantages
    -   Bulky (explicit) syntax
    -   Difficult to optimize
    -   Delayed updates from external source


#### <span class="section-num">3.1.2</span> Local dictionaries {#local-dictionaries}

-   Getting rid of global dictionaries <br />
    No synchronization — no problem
-   Store dictionaries locally
    -   Per block (in memory)
    -   Per part (on file system)
    -   In caches (during query processing)


### <span class="section-num">3.2</span> Dictionary encoded strings {#dictionary-encoded-strings}


#### <span class="section-num">3.2.1</span> StringWithDictionary {#stringwithdictionary}

-   Datatype for dictionary encoded strings
    -   Serialization
    -   Representation in memory
    -   Data processing

-   Content:
    -   Dictionary
    -   Column with positions
    -   Reversed index

        <a id="figure--fig:screenshot@2025-01-03-20:23:29"></a>

        {{< figure src="/ox-hugo/7e2dfb78239_screenshot@2025-01-03_20:23:29.png" >}}


#### <span class="section-num">3.2.2</span> LowCardinality(Type) {#lowcardinality--type}

Is a general datatype with dictionary encoding

-   Is implemented for strings, numbers, Date, DateTime, Nullable.
-   StringWithDictionary is an alias for LowCardinality(String).
-   Remains for some functions

<a id="figure--fig:screenshot@2025-01-03-20:25:28"></a>

{{< figure src="/ox-hugo/7e2dfb78239_screenshot@2025-01-03_20:25:28.png" width="800px" >}}


#### <span class="section-num">3.2.3</span> Queries optimizations {#queries-optimizations}

-   Implemented
    -   Functions executed on dictionaries if it’s possible
    -   Calculations are cached for same dictionaries
    -   GROUP BY optimization

-   To be done<br />
    Specializations for aggregate functions

<a id="figure--fig:screenshot@2025-01-03-20:28:36"></a>

{{< figure src="/ox-hugo/7e2dfb78239_screenshot@2025-01-03_20:28:36.png" >}}


### <span class="section-num">3.3</span> Summary {#summary}

-   LowCardinality type is available in last release
    -   Experimental ( `set allow_experimental_low_cardinality_type = 1` to enable)
    -   Test performance on your dataset
    -   Just replace String with StringWithDictionary

-   Goals
    -   Make datatype with dictionary better than String in all cases
    -   Implicitly replace String with StringWithDictionary

