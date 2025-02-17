# Bash Mapfile Builtin Command Help and Examples


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [语法](#h:11d958d0-32e8-40f0-8b56-88270fd85ed4)
    - <span class="section-num">1.1</span> [选项](#h:7ebb0f59-ff05-4de9-9346-346ac68e6cff)
    - <span class="section-num">1.2</span> [注意事项](#h:1ce845c9-394e-41af-a4a7-b280e219050c)
    - <span class="section-num">1.3</span> [退出状态](#h:cc5aa7f0-644e-44cf-ba26-4c8948cde1ec)
- <span class="section-num">2</span> [示例](#h:dcb6eedc-6f65-46fb-9ff8-758618b0cbb1)
    - <span class="section-num">2.1</span> [使用进程替换 (process substitution) 的 mapfile](#h:57b46aaf-3ca8-41dd-b037-b8dc76e09397)
- <span class="section-num">3</span> [Process substitution](#h:09d65bb0-ed38-465f-95dd-da3cd65308a2)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.computerhope.com/unix/bash/mapfile.htm



## <span class="section-num">1</span> 语法 {#h:11d958d0-32e8-40f0-8b56-88270fd85ed4}

```text
mapfile [-n count] [-O origin] [-s count] [-t] [-u fd]
        [-C callback [-c quantum]] [array]
```


### <span class="section-num">1.1</span> 选项 {#h:7ebb0f59-ff05-4de9-9346-346ac68e6cff}

mapfile 内建命令接受以下选项：

-   `-n count`  读取最多 count 行。如果 count 为零，则复制所有可用的行。
-   `-O origin` 从索引号 origin 开始将行写入数组 array。默认值为零。
-   `-s count`  在写入数组之前丢弃 count 行。
-   `-t`  如果任何行以换行符结尾，则去除换行符。
-   `-u fd`  从文件描述符 fd 读取行，而不是标准输入。
-   `-C callback`  每次读取量子行时执行/评估一个函数/表达式 callback。默认量子值为 1，除非使用-c 另行指定。
-   `-c quantum`  指定函数/表达式 callback 在指定了-C 时执行/评估的行数量子。
-   `array`  要写入行的数组变量的名称。如果省略了 array，则默认变量 MAPFILE 是目标。


### <span class="section-num">1.2</span> 注意事项 {#h:1ce845c9-394e-41af-a4a7-b280e219050c}

-   命令名称 readarray 可以作为命令名称 mapfile 的别名使用，其操作没有任何差异。
-   如果指定了-u 选项，mapfile 将从文件描述符 fd 而不是标准输入中读取。
-   如果未指定 array，则默认变量 MAPFILE 将用作目标数组变量。

mapfile 命令不是非常可移植的。也就是说，如果你希望确保脚本可以在多种系统上运行，不建议使用 mapfile。它主要提供方便。相同的功能可以使用 read 循环实现，尽管通常 mapfile 的性能更好。

**read** 还可以将一个字符串转换成数组： `read -a`


### <span class="section-num">1.3</span> 退出状态 {#h:cc5aa7f0-644e-44cf-ba26-4c8948cde1ec}

mapfile 命令返回 0 表示成功，或者 1 表示出现任何问题，例如提供了无效选项，目标变量是只读的或不是数组。


## <span class="section-num">2</span> 示例 {#h:dcb6eedc-6f65-46fb-9ff8-758618b0cbb1}

mapfile 命令逐行读取输入，并将每行放入数组变量中。我们提供多行输入。

我们可以使用 printf 来实现。这是一种简单的打印带有换行符的文本的方法。

在 printf 的格式字符串中，我们可以包含"\n"（后面紧跟小写字母 n 的反斜杠），以创建换行符。
("\n"是一个元字符，是表示另一个字符的字符序列，无法直接输入，例如回车键。有关完整的 bash
元字符列表，请参见 bash 引用。)

这个 printf 命令打印三行文本：

```bash
printf "Line 1\nLine 2\nLine 3\n"

Line 1
Line 2
Line 3
```

我们想使用 mapfile 将每行放入数组的一个元素中。

默认情况下，mapfile 从标准输入读取数据，所以你可能会尝试像这样将 printf 的输出通过管道传递给 mapfile：

```bash
printf "Line 1\nLine 2\nLine 3\n" | mapfile
```

你期望默认的数组变量 MAPFILE 包含这些行的值。但是如果你检查 MAPFILE 的值：

```bash
echo "${MAPFILE[@]}"
[一个空行]
```

变量是空的。为什么呢？

**管道中的每个命令都在一个子 shell 中执行**: 一个作为子进程执行的 bash 实例。每个子 shell 都有自己的环境和词法作用域 - 构成管道中每个子 shell 环境的变量不会传递到其他子 shell 中。换句话说，一个管道中的元素之间没有共享环境副作用。在上面的例子中，mapfile 正常工作，并设置了 MAPFILE 的值，但是当命令的子 shell 终止时，变量 MAPFILE 消失了。

你可以在一个包含 mapfile 命令的子 shell 中输出 MAPFILE 的值来验证这一点，方法是将两者都括在括号中：

```bash
printf "Line 1\nLine 2\nLine 3\n" | (mapfile; echo "${MAPFILE[@]}")
 Line 1
 Line 2
 Line 3
```

在上述命令中，echo 打印数组变量 MAPFILE 的所有元素，以空格分隔。由于数据中的换行符，空格出现在第 2 行和第 3 行的开头。我们用括号显式地创建了子 shell，以便保留 MAPFILE 的值足够长的时间，以便我们看到这些值。

我们可以通过使用 `-t` 来去除换行符来修复换行符：

```bash
printf "Line 1\nLine 2\nLine 3\n" | (mapfile -t; echo "${MAPFILE[@]}")
Line 1 Line 2 Line 3
```

（如果我们使用 printf，我们可以在输出中恢复换行符 - 我们将在后续示例中这样做。）

所以，mapfile 是有效的，但是数组变量对于父 shell 不可访问。然而，通常情况下，你希望
MAPFILE 变量在后续的命令中持续存在。你可以通过进程替换来实现这一点。


### <span class="section-num">2.1</span> 使用进程替换 (process substitution) 的 mapfile {#h:57b46aaf-3ca8-41dd-b037-b8dc76e09397}

通过进程替换，我们可以将输出重定向到 mapfile，而不使用管道。

```bash
mapfile -t < <(printf "Line 1\nLine 2\nLine 3")
```

让我们分别解释一下这个命令的各个部分：

-   `mapfile -t` : Mapfile 从标准输入获取输入，并从每行的末尾删除换行符(-t)。这通常是你想要的：仅将行的文本存储在数组元素中，换行符被丢弃。

-   `<` : 第一个&lt;是一个重定向字符。它期望后面跟着一个文件名或文件描述符。该文件的内容将被重定向到前一个命令的标准输入。

-   `<( ... )` : 这些字符表示进程替换，它返回一个文件描述符。括号内的命令被执行，它们的输出被分配给这个文件描述符。在任何 bash 命令中，你可以像文件名一样使用进程替换。

当你运行整个命令时，mapfile 会安静地读取我们的三行文本，并将每一行放入默认数组变量 MAPFILE 的单独元素中。

我们可以使用 printf 来打印数组的元素来验证这一点。

```bash
printf "%s" "${MAPFILE[@]}"
```

第一个参数"%s"是 printf 的格式字符串。第二个参数"${MAPFILE[@]}"由 bash 展开。数组 MAPFILE 的所有元素("@")被展开为单独的参数。（有关更多信息，请参见：在 bash 中引用数组元素。）

```bash
Line 1Line2Line 3
```

如你所见，我们的三行文本被连续打印出来。这是因为我们用-t 删除了换行符，并且 printf 默认不输出换行符。

要指定 printf 在每行之后打印一个换行符，可以在格式字符串中使用\n：

```bash
printf "%s\n" "${MAPFILE[@]}"
Line 1
Line 2
Line 3
```

要访问数组的单个元素，请用索引数字替换@。编号是从零开始的，所以 0 是第一个元素的索引，1是第二个元素的索引，依此类推：

```bash
printf "%s\n" "${MAPFILE[0]}"
Line 1
printf "%s\n" "${MAPFILE[2]}"
Line 3
```


## <span class="section-num">3</span> Process substitution {#h:09d65bb0-ed38-465f-95dd-da3cd65308a2}

Process substitution is supported on systems that support named pipes (FIFOs) or the
/dev/fd method of naming open files. It takes the form of &lt;(list) or &gt;(list). The
process list is run with its input or output connected to a FIFO or some file in
/dev/fd. The name of this file is passed as an argument to the current command as the
result of the expansion. If the &gt;(list) form is used, writing to the file provides
input for list. If the &lt;(list) form is used, the file passed as an argument should be
read to obtain the output of list.

When available, process substitution is performed simultaneously with parameter and
variable expansion, command substitution, and arithmetic expansion.

-   进程替换在支持命名管道（FIFOs）或 /dev/fd 命名打开文件的系统上得到支持。
-   它具有 `<(list)` 或 `>(list)` 的形式。
-   进程列表与其输入或输出连接到 FIFO 或/dev/fd 中的某个文件，并作为扩展的结果将此文件的名称作为参数传递给当前命令。
-   如果使用了 `>(list)` 形式，则写入该文件提供了 list 的输入。
-   如果使用了 `<(list)` 形式，则应该读取作为参数传递的文件来获取 list 的输出。
-   如果可用，进程替换将与参数和变量扩展、命令替换和算术扩展同时进行。

