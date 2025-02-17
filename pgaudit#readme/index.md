# pgaudit/pgaudit: PostgreSQL Audit Extension


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Introduction](#h:f0206ea3-01ef-4af7-a144-a7a1a836404e)
- <span class="section-num">2</span> [Why pgAudit](#h:f5b37ef4-26b4-44a4-ba77-dc397d5e54bd)
- <span class="section-num">3</span> [Usage Considerations](#h:62f42bbe-e88a-4cf3-a2c5-d95ba4605c62)
- <span class="section-num">4</span> [Settings](#h:33bc6b27-1a27-41cd-9a02-a20adad58a9b)
    - <span class="section-num">4.1</span> [pgaudit.log](#h:89ca1d4c-6272-405a-8787-c3fa0b7585cc)
    - <span class="section-num">4.2</span> [pgaudit.log_catalog](#h:dca395cb-8831-49f2-b91b-b2ab14c01321)
- <span class="section-num">5</span> [Session Audit Logging](#h:4fc32f0b-c5c0-491b-a09c-a8fac4d0cbf5)
- <span class="section-num">6</span> [Object Audit Logging](#h:ba41086c-667f-4d95-a559-3d32bf0968c6)
- <span class="section-num">7</span> [Format](#h:370fd696-b01f-4f2f-923b-861b2a710d62)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://github.com/pgaudit/pgaudit#readme



## <span class="section-num">1</span> Introduction {#h:f0206ea3-01ef-4af7-a144-a7a1a836404e}

PostgreSQL 审计扩展（pgAudit）通过标准的 PostgreSQL 日志记录功能提供详细的会话和/或对象审计日志。

pgAudit 的目标是为 PostgreSQL 用户提供生成审计日志的能力，以满足政府、金融或 ISO 认证机构的要求。

审计是对个人或组织账户进行官方检查的行为，通常由独立机构进行。pgAudit 收集的信息被称为审计轨迹或审计日志。本文档中使用审计日志一词。


## <span class="section-num">2</span> Why pgAudit {#h:f5b37ef4-26b4-44a4-ba77-dc397d5e54bd}

标准日志记录工具可以通过设置 log_statement = all 提供基本的语句日志记录。这对于监控和其他用途是可以接受的，但不提供通常需要的详细级别的审计信息。仅仅列出对数据库执行的所有操作是不够的，还必须能够找到对审计员有兴趣的特定语句。标准日志记录工具显示用户请求的内容，而 pgAudit 则专注于数据库在满足请求时发生的详细信息。

例如，审计员可能想要验证某个特定的表是否在文档化的维护窗口内创建。这可能看起来对于 grep 来说是一个简单的任务，但如果你遇到以下模糊示例时呢：

```sql
DO $$
BEGIN
    EXECUTE 'CREATE TABLE import' || 'ant_table (id INT)';
END $$;
```

标准日志记录将给出以下结果：

```text
LOG:  statement: DO $$
BEGIN
    EXECUTE 'CREATE TABLE import' || 'ant_table (id INT)';
END $$;
```

看起来，要找到感兴趣的表可能需要对代码有一定的了解，特别是在动态创建表的情况下。这不是理想的，因为最好只需根据表名进行搜索。这就是 pgAudit 的用武之地。对于相同的输入，它将在日志中产生以下输出：

```text
AUDIT: SESSION,33,1,FUNCTION,DO,,,"DO $$
BEGIN
    EXECUTE 'CREATE TABLE import' || 'ant_table (id INT)';
END $$;"
AUDIT: SESSION,33,2,DDL,CREATE TABLE,TABLE,public.important_table,CREATE TABLE important_table (id INT)
```

不仅记录了 DO 块，而且子语句 2 包含了 CREATE TABLE 的完整文本，其中包括语句类型、对象类型和完全限定名称，使搜索变得容易。

在记录 SELECT 和 DML 语句时，pgAudit 可以配置为为每个涉及到的关系记录单独的条目。不需要解析来查找所有涉及特定表的语句。事实上，目标是语句文本主要用于深度取证，对于审计来说不是必需的。


## <span class="section-num">3</span> Usage Considerations {#h:62f42bbe-e88a-4cf3-a2c5-d95ba4605c62}

根据设置，pgAudit 可能会生成大量的日志记录。请务必仔细确定在您的环境中需要对哪些内容进行审计记录，以避免记录过多。

例如， **在 OLAP 环境中工作时，可能不明智对大型事实表的插入进行审计日志记录** :日志文件的大小很可能是插入数据的实际大小的几倍，因为日志文件以文本形式表示。由于日志通常与操作系统一起存储，这可能会导致磁盘空间迅速耗尽。在无法将审计日志限制在特定表上的情况下，请确保在测试时评估性能影响，并在日志卷上分配足够的空间。这对于 OLTP 环境也可能成立。即使插入的数量不多，审计日志的性能影响仍可能明显影响延迟。

为了限制对 SELECT 和 DML 语句进行审计日志记录的关系数量，可以考虑使用 **对象审计日志** 记录（参见对象审计）。对象审计日志记录允许选择要记录的关系，从而减少整体日志量。但是，当添加新的关系时，必须显式地将其添加到对象审计日志记录中。在这种情况下，可以考虑使用编程解决方案，其中指定的表被排除在日志记录之外，而其他表都被包括在内。


## <span class="section-num">4</span> Settings {#h:33bc6b27-1a27-41cd-9a02-a20adad58a9b}

Settings may be modified only by a superuser. Allowing normal users to change their
settings would defeat the point of an audit log.

设置只能由超级用户修改。允许普通用户更改其设置将违背审计日志的目的。

可通过下面的方式进行设置：

-   在全局范围内指定（在 postgresql.conf 或使用 ALTER SYSTEM &#x2026; SET），
-   在数据库级别指定（使用 ALTER DATABASE &#x2026; SET），
-   在角色级别指定（使用 ALTER ROLE &#x2026; SET）。

注意，设置不会通过普通角色继承来传递，并且 SET ROLE 不会更改用户的 pgAudit 设置。这是角色系统的限制，而不是 pgAudit 固有的。

**pgAudit 扩展必须加载到 shared_preload_libraries 中** 。否则，加载时会引发错误，并且不会进行审计日志记录。

此外，必须先调用 `CREATE EXTENSION pgaudit` ，然后再设置 pgaudit.log，以确保正确的
pgAudit 功能。该扩展安装了事件触发器，为 DDL 操作添加了额外的审计功能。pgAudit 在未安装该扩展的情况下可以工作，但 DDL 语句将不包含关于对象类型和名称的信息。

如果删除 pgaudit 扩展并需要重新创建，则必须首先取消设置 pgaudit.log，否则将引发错误。


### <span class="section-num">4.1</span> pgaudit.log {#h:89ca1d4c-6272-405a-8787-c3fa0b7585cc}

指定会话审计日志记录哪些类别的语句。可能的取值有：

-   READ：当源是关系或查询时，记录 SELECT 和 COPY。
-   WRITE：当目标是关系时，记录 INSERT、UPDATE、DELETE、TRUNCATE 和 COPY。
-   FUNCTION：记录函数调用和 DO 块。
-   ROLE：与角色和权限相关的语句：GRANT、REVOKE、CREATE/ALTER/DROP ROLE。
-   DDL：除了 ROLE 类别之外的所有 DDL。
-   MISC：其他命令，例如 DISCARD、FETCH、CHECKPOINT、VACUUM、SET。
-   MISC_SET：其他 SET 命令，例如 SET ROLE。
-   ALL：包括上述所有类别。

可以使用逗号分隔的列表提供多个类别，并且可以通过在类别前加上减号来排除某些类别（参见会话审计日志）。

默认值为无。


### <span class="section-num">4.2</span> pgaudit.log_catalog {#h:dca395cb-8831-49f2-b91b-b2ab14c01321}


## <span class="section-num">5</span> Session Audit Logging {#h:4fc32f0b-c5c0-491b-a09c-a8fac4d0cbf5}

Session audit logging provides detailed logs of all statements executed by a user in the
backend.


## <span class="section-num">6</span> Object Audit Logging {#h:ba41086c-667f-4d95-a559-3d32bf0968c6}

对象审计日志记录影响特定关系的语句。仅支持 SELECT、INSERT、UPDATE 和 DELETE 命令。
TRUNCATE 不包括在对象审计日志中。

对象审计日志旨在作为 pgaudit.log = 'read, write'的细粒度替代品。因此，同时使用它们可能是没有意义的，但一种可能的情况是使用会话日志来捕获每个语句，然后通过对象日志来获得有关特定关系的更多详细信息。


## <span class="section-num">7</span> Format {#h:370fd696-b01f-4f2f-923b-861b2a710d62}

审核记录会被写入到标准的日志记录工具，并以逗号分隔的格式包含以下列。如果日志记录条目的前缀部分被去除，则输出将符合 CSV 格式。

-   AUDIT_TYPE - 会话或者对象。
-   STATEMENT_ID - 该会话的唯一语句 ID。每个语句 ID 代表一个后端调用。即使某些语句没有被记录，语句 ID 也是连续的。当多个关系被记录时，可以存在多个语句 ID 的条目。
-   SUBSTATEMENT_ID - 主语句内每个子语句的连续 ID。例如，在查询中调用函数。即使某些子语句没有被记录，子语句 ID 也是连续的。当多个关系被记录时，可以存在多个子语句 ID 的条目。
-   CLASS - 例如，READ, ROLE (参见 pgaudit.log)。
-   COMMAND
    -   例如，ALTER TABLE, SELECT。
-   OBJECT_TYPE - 表，索引，视图等。适用于 SELECT、DML 和大多数 DDL 语句。
-   OBJECT_NAME - 完全限定的对象名称（例如 public.account）。适用于 SELECT、DML 和大多数 DDL 语句。
-   STATEMENT - 在后端执行的语句。
-   PARAMETER - 如果设置了 pgaudit.log_parameter 字段，则该字段将包含带引号的 CSV 格式的语句参数，如果没有参数则为&lt;none&gt;。否则，该字段为&lt;not logged&gt;。

根据您的审核日志要求，使用 `log_line_prefix` 添加其他所需的字段。一个典型的日志行前缀可能是 `'%m %u %d [%p]: '` ，它将提供每个审核日志的日期/时间、用户名、数据库名称和进程 ID。

