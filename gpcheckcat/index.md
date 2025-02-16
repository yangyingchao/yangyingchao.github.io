# gpcheckcat


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [概述](#h:bfa36016-e178-44c5-b78a-c8c0bd52f193)
- <span class="section-num">2</span> [pg_class](#h:85f7df11-2f89-46c4-909c-db8f5f11cd1b)
- <span class="section-num">3</span> [namespace](#h:d87adc91-5c8e-46a9-84fd-1e5415b0bc5f)
- <span class="section-num">4</span> [unique_index_violation](#h:f8543ff9-a368-452c-a51f-a5c76ba2b516)
- <span class="section-num">5</span> [duplicate](#h:4aae321e-1ac2-462c-8adf-6ed7b44069d3)

</div>
<!--endtoc-->



## <span class="section-num">1</span> 概述 {#h:bfa36016-e178-44c5-b78a-c8c0bd52f193}

GP 提供了 [gpcheckcat](https://github.com/greenplum-db/gpdb/blob/master/gpMgmt/bin/gpcheckcat) 用于在集群内检查系统表。

<a id="table--tbl-data-table"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl-data-table">Table 1</a>:</span>
  gpcheckcat
</div>

| 检查项                 || 描述                                                                      || Utility 模式 || 复合查询 || 错误等级 |
|---------------------|-------------------------------------------------------------------------|------------|------|------|
| pg_class               || Check pg_class entry that does not have any correspond pg_attribute entry || Y          || N    || NOREPAIR |
| namespace              || Check for schemas with a missing schema definition                        || Y          || N    || NOREPAIR |
| unique_index_violation || Check for violated unique indexes                                         || N          || Y    || NOREPAIR |
| duplicate              || Check for duplicate entries                                               || N          || Y    ||          |
| missing_extraneous     || Cross consistency check for missing or extraneous entries                 || N          || Y    ||          |
| inconsistent           || Cross consistency check for coordinator segment inconsistency             || N          ||      ||          |
| foreign_key            || Check foreign keys                                                        || N          ||      ||          |
|                        ||                                                                           ||            ||      ||          |

Note:

-   Utility 模式： 对应 SQL 是否执行在 utility 模式下 <br />
    有些 SQL 只能在集群中执行

-   复合查询： 下发的 SQL 是否为动态生成 <br />
    有些 SQL 为根据表定义或者其他条件拼接而成

-   错误等级：
    -   SUCCESS <br />
        success
    -   REMOVE <br />
        error, with repair script removes objects
    -   RESYNC <br />
        error, with repair script that resynchronizes objects
    -   NOREPAIR <br />
        error, no repair script


## <span class="section-num">2</span> pg_class {#h:85f7df11-2f89-46c4-909c-db8f5f11cd1b}

```sql
SELECT relname, relkind, tc.oid as oid
FROM   pg_class tc left outer join
       pg_attribute ta on (tc.oid = ta.attrelid)
WHERE  ta.attrelid is NULL and tc.relnatts != 0;
```


## <span class="section-num">3</span> namespace {#h:d87adc91-5c8e-46a9-84fd-1e5415b0bc5f}

```sql
SELECT o.catalog, o.nsp
FROM pg_namespace n right outer join
     (select 'pg_class' as catalog, relnamespace as nsp from pg_class
      union
      select 'pg_type' as catalog, typnamespace as nsp from pg_type
      union
      select 'pg_operator' as catalog, oprnamespace as nsp from pg_operator
      union
      select 'pg_proc' as catalog,pronamespace as nsp from pg_proc) o on
      (n.oid = o.nsp)
WHERE n.oid is NULL;
```


## <span class="section-num">4</span> unique_index_violation {#h:f8543ff9-a368-452c-a51f-a5c76ba2b516}

SQL 为拼接而成，分成两步：

1.  获取有唯一索引约束的表和索引
2.  逐一检查每个表是否违反了唯一约束

代码： [unique_index_violation_check.py](https://github.com/greenplum-db/gpdb/blob/master/gpMgmt/bin/gpcheckcat_modules/unique_index_violation_check.py)

```python
class UniqueIndexViolationCheck:
    unique_indexes_query = """
        select table_oid, index_name, table_name, array_agg(attname) as column_names
        from pg_attribute, (
            select pg_index.indrelid as table_oid, index_class.relname as index_name, table_class.relname as table_name, unnest(pg_index.indkey) as column_index
            from pg_index, pg_class index_class, pg_class table_class
            where pg_index.indisunique='t'
            and index_class.relnamespace = (select oid from pg_namespace where nspname = 'pg_catalog')
            and index_class.relkind = 'i'
            and index_class.oid = pg_index.indexrelid
            and table_class.oid = pg_index.indrelid
        ) as unique_catalog_index_columns
        where attnum = column_index
        and attrelid = table_oid
        group by table_oid, index_name, table_name;
    """

    def __init__(self):
        self.violated_segments_query = """
            select distinct(gp_segment_id) from (
                (select gp_segment_id, %s
                from gp_dist_random('%s')
                where (%s) is not null
                group by gp_segment_id, %s
                having count(*) > 1)
                union
                (select gp_segment_id, %s
                from %s
                where (%s) is not null
                group by gp_segment_id, %s
                having count(*) > 1)
            ) as violations
        """

    def runCheck(self, db_connection):
        unique_indexes = db_connection.query(self.unique_indexes_query).getresult()
        violations = []

        for (table_oid, index_name, table_name, column_names) in unique_indexes:
            column_names = ",".join(column_names)
            sql = self.get_violated_segments_query(table_name, column_names)
            violated_segments = db_connection.query(sql).getresult()
            if violated_segments:
                violations.append(dict(table_oid=table_oid,
                                       table_name=table_name,
                                       index_name=index_name,
                                       column_names=column_names,
                                       violated_segments=[row[0] for row in violated_segments]))

        return violations

    def get_violated_segments_query(self, table_name, column_names):
        return self.violated_segments_query % (
            column_names, table_name, column_names, column_names, column_names, table_name, column_names, column_names
        )
```


## <span class="section-num">5</span> duplicate {#h:4aae321e-1ac2-462c-8adf-6ed7b44069d3}

