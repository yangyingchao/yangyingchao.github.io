# Note about PO file in PostgreSQL


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Concepts](#h:7cc071ca-9356-4d53-94ff-f0819cc97b5e)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://github.com/postgres/postgres/blob/master/doc/src/sgml/nls.sgml#L52



## <span class="section-num">1</span> Concepts {#h:7cc071ca-9356-4d53-94ff-f0819cc97b5e}

The pairs of original (English) messages and their (possibly) translated equivalents are
kept in `message catalogs` , one for each program (although related programs can share a
message catalog) and for each target language.  There are two file formats for message
catalogs: The first is the `PO` file (for Portable Object), which is a plain text file
with special syntax that translators edit.  The second is the `MO` file
(for Machine Object), which is a binary file generated from the respective PO file and
is used while the internationalized program is run.  Translators do not deal with MO
files; in fact hardly anyone does.

原始（英语）消息与它们的（可能的）翻译对应关系保存在 `消息目录` 中，每个程序都有一个（尽管相关的程序可以共享一个消息目录），对于每种目标语言也是如此。消息目录有两种文件格式：

-   第一种是 `PO` 文件（Portable Object）
    -   这是一个特殊语法的纯文本文件，
    -   由翻译人员编辑

-   第二种是 `MO` 文件（Machine Object）
    -   它是从相应的 PO 文件生成的二进制文件
    -   在国际化程序运行时使用。

翻译人员不需要处理 MO 文件；事实上，几乎没有人会处理。

The extension of the message catalog file is to no surprise either `.po` or `.mo`.  The
base name is either the name of the program it accompanies, or the language the file is
for, depending on the situation.  This is a bit confusing.  Examples are `psql.po` (PO
file for psql) or `fr.mo` (MO file in French).

消息目录文件的扩展名也不足为奇，要么是 `.po` 要么是 `.mo` 。基本名称则根据情况是程序的名称或者文件所对应的语言。这有点让人困惑。例如， `psql.po` （psql 的 PO 文件）或 `fr.mo`
（法语的 MO 文件）。

The file format of the PO files is illustrated here:

PO 文件的文件格式如下所示：

```nil
# comment

msgid "original string"
msgstr "translated string"

msgid "more original"
msgstr "another translated"
"string can be broken up like this"

...
```

The msgid lines are extracted from the program source.  (They need not be, but this is
the most common way.)  The msgstr lines are initially empty and are filled in with
useful strings by the translator.  The strings can contain C-style escape characters and
can be continued across lines as illustrated.  (The next line must start at the
beginning of the line.)

-   **msgid**  行是从程序源代码中提取的。（虽然不一定非得这样做，但这是最常见的方式。）
-   msgid 行之后的 **msgstr** 行最初是空的，会由翻译人员填入有用的字符串。 <br />
    这些字符串可以包含 C 风格的转义字符，并且可以跨行续写，就像这样。（下一行必须从行的开头开始。）

The `#` character introduces a comment.  If whitespace immediately follows the #
character, then this is a comment maintained by the translator.  There can also be
automatic comments, which have a non-whitespace character immediately following the #.
These are maintained by the various tools that operate on the PO files and are intended
to aid the translator.

-   `#` 字符引入一个注释。
-   如果 `#` 字符后面紧跟着 **空格** ，则这是译者维护的注释。
-   如果 `#` 字符后面紧跟着 **非空格** ， 则为自动注释，他们由操作 PO 文件的各种工具维护，并旨在帮助翻译者。

<!--listend-->

```nil
#. automatic comment
#: filename.c:1023
#, flags, flags
```

The `#.` style comments are extracted from the source file where the message is used.
Possibly the programmer has inserted information for the translator, such as about
expected alignment.  The #: comments indicate the exact locations where the message is
used in the source.  The translator need not look at the program source, but can if
there is doubt about the correct translation.  The #, comments contain flags that
describe the message in some way.  There are currently two flags: `fuzzy` is set if the
message has possibly been outdated because of changes in the program source.  The
translator can then verify this and possibly remove the fuzzy flag.  Note that fuzzy
messages are not made available to the end user.  The other flag is `c-format`, which
indicates that the message is a &lt;function&gt;printf&lt;/function&gt;-style format template.  This
means that the translation should also be a format string with the same number and type
of placeholders.  There are tools that can verify this, which key off the c-format flag.

-   `#.` ： 的内容是从使用消息的源文件中提取的。 可能程序员已经为翻译者插入了关于期望对齐方式等信息。
-   `#` :   指出了消息在源文件中的确切位置。翻译者无需查看程序源文件，但如果对正确翻译存在疑问，可以查看源文件。
-   `#，` ： 注释包含描述消息的标志。目前有两个标志：
    -   `fuzzy` ：
        -   如果消息可能因程序源文件的更改而过时，则会设置 `fuzzy` 标志。
        -   翻译者可以验证这一点，并可能移除 fuzzy 标志。请注意，模糊消息不向最终用户提供。
    -   `c-format` :
        -   表示消息是 `printf` 格式的模板。
        -   这意味着翻译也应该是具有相同数量和类型占位符的格式字符串。
        -   有一些工具可以验证这一点，这些工具会基于 c-format 标志进行匹配。

