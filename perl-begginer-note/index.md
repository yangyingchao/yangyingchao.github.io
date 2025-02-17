# perl begginer note


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [pos2usage](#h:b6801896-14cc-40fc-ae0e-ba5a5e72ba62)
    - <span class="section-num">1.1</span> [pod2usage 是作什么的](#h:7bf6a392-7f59-4e97-8920-51f304ffc357)
    - <span class="section-num">1.2</span> [pod2usage 输出的文档，是在哪里定义的](#h:c6fe3dca-2a93-44f4-b1ac-212382383d4a)
    - <span class="section-num">1.3</span> [代码中的 =head1 是什么意思？](#h:05d56de5-9cd9-41f0-b7a0-bd64e64239b9)
- <span class="section-num">2</span> [use 是什么意思](#h:1eabc14a-0303-4f40-9313-1bcee7157522)
- <span class="section-num">3</span> [die hook](#h:b830abd9-6b0f-4741-bcd2-f7bef02eafe8)
- <span class="section-num">4</span> [常量](#h:229abac2-9ec9-48b5-8b47-9b70d05bfa80)
- <span class="section-num">5</span> [变量](#h:31eb38d0-dd42-40b6-ae7b-2e5740695928)

</div>
<!--endtoc-->

不懂 perl, 所以看代码时候问了 chatGPT 这些很基础的问题，简单整理记录一下。


## <span class="section-num">1</span> pos2usage {#h:b6801896-14cc-40fc-ae0e-ba5a5e72ba62}


### <span class="section-num">1.1</span> pod2usage 是作什么的 {#h:7bf6a392-7f59-4e97-8920-51f304ffc357}

pod2usage 是 Perl 中的一个函数，用于将指定的 POD（Plain Old Documentation）文档转换为使用说明。POD 是 Perl 语言中的一种简单文本标记格式，用于编写源代码中的文档注释和使用说明文档。pod2usage 函数可以方便地将这些注释和说明文档输出到终端或文件。

在程序中，pod2usage 主要用于输出使用说明以及命令行帮助信息。它的常用参数如下：

-   -verbose：设定输出详细程度，取值为 0、1 或 2，默认为 1；
-   -exitval：设定函数的退出值，默认为 1；
-   -output：输出的文件或句柄，默认为 \\\*STDERR。

例如，在代码中调用 pod2usage() 函数输出使用说明的时候，它默认会输出到终端，同时产生退出状态码 1，可以让主程序退出。如果使用 -verbose =&gt; 2 参数，可以输出更详细的信息。


### <span class="section-num">1.2</span> pod2usage 输出的文档，是在哪里定义的 {#h:c6fe3dca-2a93-44f4-b1ac-212382383d4a}

`pod2usage` 输出的文档通常是在 Perl 源代码文件中使用特殊的注释格式（即 POD 格式）定义的。在 Perl 中，使用 "=pod" 和 "=cut" 标记来定义这些注释段：

```perl
=pod
=head1 SYNOPSIS
Usage: script.pl [options] [file ...]
Options:
  -h --help           Show this help screen
  -v --verbose        Enable verbose output
  -d --debug          Enable debugging messages
  -f --file           Specify input file
  -o --output         Specify output file
=head1 DESCRIPTION
This script does something useful.
=cut
```

在这里，定义了一个简单的使用说明文档，包含了程序的使用方法，命令行选项和描述。pod2usage
函数会自动解析 Perl 源代码中的 POD 格式注释，将其转换为纯文本形式并输出到终端。我们可以通过对 POD 格式注释的编写和格式化，来使得使用说明文档更加完整、清晰易读。

```perl
###
####################################################################################################################################
# Usage
####################################################################################################################################
=head1 SYNOPSIS
pgaudit_analyze [options] <pg-log-path>
 Configuration Options:
   --daemon             run as a daemon (consider running under upstart or systemctl)
   --port               port that PostgreSQL is running on (defaults to 5432)
   --socket-path        PostgreSQL host or socket directory (default to system default directory)
   --log-file           location of the log file for pgaudit_analyze (defaults to /var/log/pgaudit_analyze.log)
   --user               specify postgres user instead of using pgaudit_analyze invoker
 General Options:
   --help               display usage and exit
=cut
```

这段代码是一个命令行程序的使用说明。该程序名为 "pgaudit_analyze"，可以从命令行运行。可以使用以下选项来定制程序的配置：

-   &#x2013;daemon：将程序作为守护进程运行（建议在 upstart 或 systemctl 下运行）；
-   &#x2013;port：指定 PostgreSQL 运行的端口号，默认为 5432；
-   &#x2013;socket-path：指定 PostgreSQL 的主机或套接字路径（默认为系统默认目录）；
-   &#x2013;log-file：指定 pgaudit_analyze 的日志文件位置（默认为 /var/log/pgaudit_analyze.log）；
-   &#x2013;user：指定要使用的 postgres 用户，而不是使用 pgaudit_analyze 调用者。

此外，还有一个通用选项：

-   &#x2013;help：显示使用说明并退出。


### <span class="section-num">1.3</span> 代码中的 =head1 是什么意思？ {#h:05d56de5-9cd9-41f0-b7a0-bd64e64239b9}

在 Perl 编程语言中，=head1 表示一个文档的节（section），在这个节中，标题通常为粗体。而在
Perl 中，这种文档格式通常用来生成程序或模块的文档。在这里，这段代码使用 =head1 来标记一个章节，章节标题为 SYNOPSIS。SYNOPSIS 章节通常用于描述一个脚本或程序如何使用的示例和简要的指南。


## <span class="section-num">2</span> use 是什么意思 {#h:1eabc14a-0303-4f40-9313-1bcee7157522}

在 Perl 编程语言中，use 语句用于加载一个模块（也称为库）或启用一个 pragma（编译指令）。它的一般形式为：

\`\`\`
use Module::Name;
use Module::Name qw(imported_function1 imported_function2 &#x2026;);
use Pragma;
\`\`\`

其中，Module::Name 表示要加载的模块名称，而 Pragma 则表示要启用的编译器指令。在 Perl 中，模块通常是一组封装好的函数和数据结构，而 pragma 可以用来更改编译器或运行时的行为，例如开启严格模式（strict）、开启警告（warnings）等。


#### <span class="section-num">2.0.1</span> 上面的 qw() 是做什么的 {#h:8f0de5d7-916b-4489-824c-715ec98085e9}

在 Perl 编程语言中，qw() 是一种快速创建字符串列表的语法糖，它被用于创建字符串数组。在 qw() 括号中，通过空格或制表符分割的字符串会被转换为数组的元素，而逗号则用来分隔不同的元素。与使用普通的双引号或单引号不同，使用 qw() 创建字符串数组不需要使用引号或逗号，这使得语法更加简洁。在上面的代码中，qw(all) 表示一个字符串数组，包含了一个元素 "all"。这里它的作用是告诉警告 pragma 把所有警告视为致命错误。

\###

```perl
use DBI;
use File::Basename qw(dirname);
use Getopt::Long qw(GetOptions);
use Pod::Usage;
use POSIX qw(setsid);
```

这段代码与 Perl 编程语言有关，包含了五个 use 语句。它们的含义分别是：

-   use DBI：加载 DBI 模块，这是 Perl 中用于操作数据库的标准接口。
-   use <:Basename> qw(dirname)：加载 <:Basename> 模块，并导入其中的 dirname 函数，该函数用于获取给定文件路径的目录名部分。
-   use Getopt::Long qw(GetOptions)：加载 Getopt::Long 模块，并导入其中的 GetOptions 函数，该函数用于解析命令行参数。
-   use Pod::Usage：加载 Pod::Usage 模块，它提供了把 Perl 脚本中的 POD（Plain Old Documentation）文档转换为使用说明的方法。
-   use POSIX qw(setsid)：加载 POSIX 模块，并导入其中的 setsid 函数，该函数用于创建一个新的会话并将调用进程设置为新会话的领导者。

需要注意的是，这些模块都需要使用 cpan 等工具进行安装。


## <span class="section-num">3</span> die hook {#h:b830abd9-6b0f-4741-bcd2-f7bef02eafe8}

```perl
###
####################################################################################################################################
# Handle die as a confess instead to get more detailed error information
####################################################################################################################################
$SIG{__DIE__} = sub {Carp::confess @_};
```

这段代码是将内置变量 `$SIG{__DIE__}` 重定义为一个新的匿名子过程，用于把程序的“die”行为改为
`Carp::confess` ，以便获取更详细的错误信息。当程序中调用 die 函数时，Perl 解释器会触发
`__DIE__` 信号，并尝试执行 `$SIG{__DIE__}` 中的代码。在这里，使用 Carp::confess 来代替 die
函数，它可以生成一份类似于栈追踪的调试信息，并在错误发生时输出到输出流中。这种方式可以帮助开发者更快地定位和修复程序中的错误。


## <span class="section-num">4</span> 常量 {#h:229abac2-9ec9-48b5-8b47-9b70d05bfa80}

```perl
###
use constant
{
    # Session unique fields
    LOG_FIELD_USER_NAME                 => 1,
    LOG_FIELD_DATABASE_NAME             => 2,
    LOG_FIELD_PROCESS_ID                => 3,
    LOG_FIELD_CONNECTION_FROM           => 4,
    LOG_FIELD_SESSION_ID                => 5,
    LOG_FIELD_SESSION_START_TIME        => 8,
    # Log fields
    LOG_FIELD_LOG_TIME                  => 0,
    LOG_FIELD_SESSION_LINE_NUM          => 6,
    LOG_FIELD_COMMAND_TAG               => 7,
    LOG_FIELD_VIRTUAL_TRANSACTION_ID    => 9,
    LOG_FIELD_TRANSACTION_ID            => 10,
    LOG_FIELD_ERROR_SEVERITY            => 11,
    LOG_FIELD_SQL_STATE_CODE            => 12,
    LOG_FIELD_MESSAGE                   => 13,
    LOG_FIELD_DETAIL                    => 14,
    LOG_FIELD_HINT                      => 15,
    LOG_FIELD_INTERNAL_QUERY            => 16,
    LOG_FIELD_INTERNAL_QUERY_POS        => 17,
    LOG_FIELD_CONTEXT                   => 18,
    LOG_FIELD_QUERY                     => 19,
    LOG_FIELD_QUERY_POS                 => 20,
    LOG_FIELD_LOCATION                  => 21,
    LOG_FIELD_APPLICATION_NAME          => 22
};
```

这段代码使用了 Perl 中的常量（constant）。它定义了一个匿名哈希（hash），其中每个键（key）都是一个常量，对应了 Postgres 的日志格式。 代码中使用了大括号来表示一个哈希，并使用箭头（=&gt;）来分隔键和值。其中，键是常量名，值是对应的数值，代表 Postgres 日志中的某个字段。

在这里，这个常量哈希主要用于后续代码中访问 Postgres 日志时，将字段名称转化为常量名，这样可以避免使用魔法数字（magic numbers）并提高代码的可读性和可维护性。下面是一些示例：

-   LOG_FIELD_USER_NAME 表示用户名字段在哈希中对应的键值，其值为 1；
-   LOG_FIELD_TRANSACTION_ID 表示事务 ID 字段在哈希中对应的键值，其值为 10；
-   LOG_FIELD_APPLICATION_NAME 表示应用程序名称字段在哈希中对应的键值，其值为 22。

可以通过这些常量来代替在代码中使用数字来访问 Postgres 日志字段，并且可以避免因 Postgres 日志格式更改而导致代码逻辑混乱。


## <span class="section-num">5</span> 变量 {#h:31eb38d0-dd42-40b6-ae7b-2e5740695928}

```perl
###
my %oDbHash;
my $strAuditUserName = 'pgaudit_etl';
my $strAuditSchemaName = 'pgaudit';
```

这段代码定义了三个变量。

-   %oDbHash：这是一个哈希表，也称为关联数组（associative array），用于存储数据库连接的相关信息。在程序运行期间，我们可以使用这个哈希表来保存数据库连接、游标等对象，并在程序的其他部分中共享这些对象。
-   $strAuditUserName：这是一个字符串变量，保存了 Postgres 数据库的用户名，该用户名用于访问存储日志信息的 pgaudit 表。
-   $strAuditSchemaName：这也是一个字符串变量，保存了 Postgres 数据库中存储日志信息的 schema 名称，该 schema 中包含了所有与日志分析相关的表。

在程序中，这些变量的具体使用方式会参考实际情况，在后续的代码中进行定义和赋值。


#### <span class="section-num">5.0.1</span> my 是用来作什么的 {#h:38dd33e5-b03d-4d16-acd8-3aae494cddd4}

在 Perl 编程语言中，my 关键字用于声明一个新的变量，并将其限定在当前代码块（block）或子程序（subroutine）中。如果我们没有使用 my 或其他类似的关键字来声明变量，那么 Perl 就会将其视为一个全局变量，并可以在程序的任何部分进行修改和访问。

例如，在这段代码中，使用了 my 关键字来声明三个变量，这意味着它们只能在当前代码块或子程序中使用，并且不能在其他代码块或子程序中访问或修改它们。这样可以避免因命名冲突而导致程序出错，也能让代码更加清晰明了。

\###
sub databaseGet
{
    my $strDatabaseName = shift;

if (defined($oDbHash{$strDatabaseName}))
{

return false
    if (!$oDbHash{$strDatabaseName}{log});

    return true;
}

这段代码是一个名为 databaseGet 的子程序（subroutine），接收一个参数 $strDatabaseName，用于返回需要分析的 Postgres 数据库连接设置。

在实现中，首先，使用 my 关键字声明了一个新的局部变量 $strDatabaseName 用于存储子程序接收到的参数。然后，使用 defined 函数检查 $oDbHash 哈希表是否已经包含了这个数据库的信息。

如果存在，会有两种可能：

-   如果该数据库没有配置日志记录，则返回 false；
-   否则返回 true，代表允许记录日志。

而如果 $oDbHash 中没有这个数据库的信息，则子程序会返回 undef（或空值）。在程序的其他部分，我们可以使用这个子程序来完成数据库的连接和日志记录的设置。


#### <span class="section-num">5.0.2</span> 上面的 shift 是做什么的 {#h:5c853723-27ab-4298-996e-03fe9e973e63}

\`shift\` 是用来从参数列表中取出第一个参数的函数。在这个代码中，它取出了传递给 \`databaseGet\` 函数的唯一一个参数 \`$strDatabaseName\`，并将其赋值给变量 \`$strDatabaseName\`。

所以，这个函数的首行代码 \`my $strDatabaseName = shift;\` 的作用就是将传入的 \`$strDatabaseName\` 参数值取出来，并将其存储到 \`$strDatabaseName\` 变量中。


#### <span class="section-num">5.0.3</span> if (defined($oDbHash{$strDatabaseName})) 是什么意思 {#h:7ef07a4d-29f0-4084-a88b-10b9fe8321de}

\`if (defined($oDbHash{$strDatabaseName}))\` 这行代码使用了一个 \`defined\` 函数来判断一个叫做 \`%oDbHash\` 的哈希表中是否存在 \`$strDatabaseName\` 这个键名。

具体地说，如果哈希表 \`%oDbHash\` 中存在 \`$strDatabaseName\` 这个键名，则 \`defined\` 函数会返回 true，否则返回 false。

因此，这行代码在判断当前的程序是否已经连接到名为 \`$strDatabaseName\` 的数据库。如果存在，则证明程序已经连接过这个数据库。否则，程序需要对这个数据库重新进行连接。


#### <span class="section-num">5.0.4</span> 怎样访问 perl 的哈希列表？ {#h:ca249f8e-8341-455c-a0ca-0691c2f115da}

访问 Perl 哈希列表的方式是通过使用键名来获取或者设置对应的值。

例如，如果有一个叫做 \`%hash\` 的哈希列表，我们可以使用它的键名来获取或者设置特定的值，比如：

\`\`\`perl

my %hash = (
    key1 =&gt; 'value1',
    key2 =&gt; 'value2',
    key3 =&gt; 'value3'
);

my $value = $hash{'key1'};  # $value 等于 'value1'

$hash{'key2'} = 'new_value2';
\`\`\`

需要注意的是，Perl 哈希列表中的键名和值都可以是任何标量类型，包括字符串、数字、引用等等。

\###

```perl
$oDbHash{$strDatabaseName}{hDb} = DBI->connect(
    "dbi:Pg:dbname=${strDatabaseName};port=${iPort};" .
    (defined($strSocketPath) ? "host=${strSocketPath}" : ''),
    $strDbUser, undef,
    {AutoCommit => 0, RaiseError => 1});
```

这段代码的作用是根据给定的数据库名称 \`$strDatabaseName\`，连接到一个 PostgreSQL 数据库，并将连接对象存储到一个名为 \`$oDbHash\` 的哈希列表中。

具体地说，它首先创建一个名为 \`$oDbHash\` 的哈希列表，然后将连接对象存储在 \`$oDbHash{$strDatabaseName}{hDb}\` 中，其中 \`$strDatabaseName\` 是数据库名称。这个哈希列表可以在程序的其他地方使用，以便后续操作该数据库时可以重复使用此连接。

代码中连接数据库的方式是使用 Perl 的 DBI 模块提供的 \`connect()\` 函数连接 PostgreSQL 数据库。\`connect()\` 函数的参数是一个字符串，包含要连接的数据库的名称、端口和其他一些数据库连接选项。具体来说，这个字符串的格式是 \`"dbi:Pg:dbname=\\({strDatabaseName};port=\\){iPort};host=\\({strSocketPath}"\`，其中 \`\\){strDatabaseName}\` 是数据库名称，\`\\({iPort}\` 是端口号，\`\\){strSocketPath}\` 是套接字路径（如果可用）。

值得注意的是，代码中设置了一些连接选项，包括 \`{AutoCommit =&gt; 0}\` 和 \`{RaiseError =&gt; 1}\`。\`{AutoCommit =&gt; 0}\` 表示关闭自动提交事务的选项，也即开启了事务处理；\`{RaiseError =&gt; 1}\` 表示设置了自动报错选项，也即当出现错误时，DBI 模块将自动抛出异常。


#### <span class="section-num">5.0.5</span> 简单介绍一下 DBI 模块 {#h:5d8fd3c8-672c-406e-8084-e77ea607e308}

DBI（Database Interface）是 Perl 语言中使用最广泛的数据库访问模块。它是一个规范，定义了一组指令和 API 函数，用于访问各种关系数据库。它的目标是为 Perl 提供一种数据库无关性的抽象接口，并且尽可能的对各大数据库使用相同的操作语言。

DBI 的优点：

-   简单易用，提供了可移植的接口，让用户无需关心底层数据库信息。
-   可扩展性好，支持了大量第三方连接模块（称之为 DBD，Database Drivers）来支持各大数据库。

DBI 的基本用法：

-   使用 DBI 模块中的 \`connect()\` 函数连接指定的数据库。
-   创建一个 \`Database Statement Handle\`（句柄），句柄是连接到数据库的关键对象。使用 \`prepare()\` 函数准备要执行的 SQL 语句。
-   执行 SQL 语句，获取返回结果。使用 \`execute()\` 或者 \`fetch()\` 函数执行 SQL 语句，并从结果集中获取数据。
-   完成查询操作后，使用 \`disconnect()\` 函数关闭对数据库的连接。

总的来说，DBI 是一个非常强大且使用广泛的数据库访问模块，通过它可以很方便地访问、操作各种类型的关系数据库。


#### <span class="section-num">5.0.6</span> 上面的 hDb  是什么 {#h:0d44b289-d23a-46cb-bbe0-ed2545a02c62}

\`hDb\` 是保存在 \`$oDbHash\` 哈希表中的一个键名。在这段代码中，\`$oDbHash{$strDatabaseName}{hDb}\` 是一个存储 \`DBI\` 数据库句柄（Database Handle）的哈希表元素，这个数据库句柄是后面对该数据库进行操作所必需的。也就是说，一旦成功连接到一个数据库，使用 DBI 连接返回的数据库句柄，您可以使用该句柄执行任意的 SQL 语句，进行相关的数据库操作。

可以通过使用 \`$oDbHash{$strDatabaseName}{hDb}\` 来访问保存在哈希表中的数据库句柄，以便在程序的其他部分使用它来执行数据库操作。

\###

```perl
# Check for the audit schema to determine whether or not to log this database
my $hSqlSchemaSelect = $oDbHash{$strDatabaseName}{hDb}->prepare(
    "select count(*) = 1\n" .
    "  from pg_namespace\n" .
    " where nspname = ?");
$oDbHash{$strDatabaseName}{hDb}->do("set session authorization ${strAuditUserName}");
$hSqlSchemaSelect->execute($strAuditSchemaName);
$oDbHash{$strDatabaseName}{log} = ($hSqlSchemaSelect->fetchrow_array())[0] ? true : false;
undef($hSqlSchemaSelect);
```

这段代码的作用是检查 PostgreSQL 数据库中是否存在特定的审计记录模式（audit schema）。

首先，代码使用 \`$oDbHash{$strDatabaseName}{hDb}-&gt;prepare()\` 函数为数据库连接对象 \`$oDbHash{$strDatabaseName}{hDb}\` 准备了一个 SQL 语句，用于查询数据库中是否存在指定的表。

然后，代码使用 \`$hSqlSchemaSelect-&gt;execute()\` 函数执行准备的查询语句，并传入审计记录模式的名称作为参数。

接着，代码使用 \`$hSqlSchemaSelect-&gt;fetchrow_array()\` 函数从查询结果中取得记录，并将检查到的记录数值（0 或 1）存储在 \`$oDbHash{$strDatabaseName}{log}\` 中。最后，代码使用 \`undef($hSqlSchemaSelect)\` 释放了查询语句的资源。

这个 \`$oDbHash{$strDatabaseName}{log}\` 是一个布尔值，用来判断当前数据库是否需要记录审计日志，如果该值为 true，则需要记录审计日志；否则，不需要记录审计日志。

\###

```perl
$oDbHash{$strDatabaseName}{hSqlSessionInsert} = $oDbHash{$strDatabaseName}{hDb}->prepare(
    "insert into pgaudit.session (session_id, process_id, session_start_time, user_name, application_name,\n" .
    "                             connection_from, state)\n" .
    "                     values (?, ?, ?, ?, ?, ?, ?)");
```

这段代码是为了创建一个用于插入会话信息的 SQL 语句，并使用 \`prepare()\` 函数为它获取一个数据库句柄。

具体而言，这个 SQL 语句将会在审计记录模式（audit schema）下的 \`pgaudit.session\` 表中插入会话信息，包括 \`session_id\`、\`process_id\`、\`session_start_time\`、\`user_name\`、\`application_name\`、\`connection_from\` 和 \`state\`。

然后，这个 SQL 语句使用 \`$oDbHash{$strDatabaseName}{hDb}-&gt;prepare()\` 函数，为这个数据库连接对象获取一个用于执行 SQL 语句的数据库句柄 \`$oDbHash{$strDatabaseName}{hSqlSessionInsert}\`。这样，之后就可以通过这个句柄对 \`pgaudit.session\` 表进行插入操作了。

备注：其中 \`$strDatabaseName\` 指定了要插入数据的数据库名称，\`$oDbHash\` 是一个哈希列表，用于保存数据库连接对象以及其他相关的数据。


#### <span class="section-num">5.0.7</span> perl 哈希表的 key, 有哪些类型 {#h:a7a38401-12bb-4256-81f4-2d7a1d105b4b}

Perl 哈希表的键（key）可以是任何类型的标量（scalar），包括字符串、数字、引用、文件句柄、类型等等。

注意，虽然哈希表的键可以是标量，但一个标量只能表示一个键。换句话说，哈希表里所有的键是互不相同的。

如果给哈希表赋值时使用了相同的键，则程序会将后面赋的值覆盖先前的值。除此之外，哈希表的键和值之间的关系是没有顺序之分的。


#### <span class="section-num">5.0.8</span> 继续 {#h:9bd5b4af-e920-4f89-8fff-353e64c2ce8a}

```perl
sub getline {
    my ($self, $io) = @_;
    require IO::Handle;
    $self->{_EOF} = $io->eof ? 1 : '';
```

这是一段 Perl 代码，实现了一个 \`getline\` 函数。下面是代码的作用：

-   输入参数 \`$self\` 是一个对象实例，\`$io\` 表示输入输出文件句柄。
-   引入了 \`IO::Handle\` 模块，用于检查文件是否到达结尾。
-   将 \`$io\` 文件句柄的 \`eof\` 方法调用结果赋值给对象实例 \`$self\` 的 \`_EOF\` 属性。如果文件已经到达了结尾，将 \`_EOF\` 属性设置为 1，否则将 \`_EOF\` 设置为空字符串。这个 \`_EOF\` 属性表示了文件处理器是否到达了文件结尾，并通过返回值告诉上层代码文件是否已经读取完毕。


#### <span class="section-num">5.0.9</span> @\_ 是什么意思 {#h:ea1df3dd-b6c6-42bb-a893-9daf491fa351}

\`@_\` 是一个特殊的 Perl 数组，它包含了函数或子程序的参数列表。每当你调用一个函数或子程序时，Perl 自动将传递给函数的参数列表存储在 \`@_\` 数组中。例如：

```perl
sub foo {
    my ($arg1, $arg2, $arg3) = @_;
    # ...
}
foo('hello', 42, [1, 2, 3]);
```

在上面这个例子中，=foo= 子程序被调用时，传递了三个参数 `'hello'=、=42` 和数组引用 `[1,2, 3]` ，这些参数被打包成一个列表并存储在 `@_` 数组中。在子程序中，我们可以用 `my ($arg1,
$arg2, $arg3) = @_` 的语法来获取每个参数的值。即 `$arg1` 将获得 `'hello'=，=$arg2` 将获得 `42=，=$arg3` 将获得  `[1，2，3]` 。

