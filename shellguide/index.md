# google shell style guide | Style guides for Google-originated open-source projects


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Background](#h:f6a55800-10d4-4052-be0f-f0d7053dd7b6)
- <span class="section-num">2</span> [Shell Files and Interpreter Invocation](#h:be0de52e-c0fc-43e7-b1f8-d99e128cef75)
    - <span class="section-num">2.1</span> [File Extensions](#h:f9b70299-d7f6-431d-a77d-2bb400685496)
    - <span class="section-num">2.2</span> [SUID/SGID](#h:7eb24edb-2c24-4fea-b278-6aa152751b06)
- <span class="section-num">3</span> [Environment](#h:fa963b24-5dd3-4380-ae40-710090f14e29)
    - <span class="section-num">3.1</span> [STDOUT vs STDERR](#h:90556c60-d3cd-465b-91c8-1e635abfee5f)
- <span class="section-num">4</span> [Comments](#h:e774be8f-ca5b-4520-a57f-c799f69b7675)
    - <span class="section-num">4.1</span> [File Header](#h:24294cf2-94e9-4f88-9afe-a56220843c64)
    - <span class="section-num">4.2</span> [Function Comments](#h:43e7e163-58c6-4932-b309-95390eea923d)
    - <span class="section-num">4.3</span> [Implementation Comments](#h:4b9e0968-747e-4428-a718-7e54939feffd)
    - <span class="section-num">4.4</span> [<span class="org-todo todo TODO">TODO</span> Comments](#h:9a26a2da-3497-46f0-a3e1-1bc5739749e4)
- <span class="section-num">5</span> [Formatting](#h:fa0aab5f-0167-4fa5-a463-ea5b965ac727)
    - <span class="section-num">5.1</span> [Indentation](#h:033bfebd-8148-4e37-bdb7-f1242f62d7a7)
    - <span class="section-num">5.2</span> [Line Length and Long Strings](#h:098c6c3a-4ae7-40bb-b75c-e37a811e28d7)
    - <span class="section-num">5.3</span> [Pipelines](#h:dcbc1b2e-9cf4-48fe-a706-7f3a39386d32)
    - <span class="section-num">5.4</span> [Loops](#h:d5df6dad-79a8-4531-af28-a3e4a797d193)
    - <span class="section-num">5.5</span> [Case statement](#h:033449c1-3dc3-4148-9de3-099eb0e1f366)
    - <span class="section-num">5.6</span> [Variable expansion](#h:ee435b39-dad7-4604-a699-738299997086)
    - <span class="section-num">5.7</span> [Quoting](#h:59bf8094-699c-4c8b-ae73-65616213a707)
- <span class="section-num">6</span> [Features and Bugs](#h:f07ebca3-d96a-4b10-8f24-043ef9cfec52)
    - <span class="section-num">6.1</span> [ShellCheck](#h:8c4ba5f5-2f9e-41df-8f4d-1d1f7c143b71)
    - <span class="section-num">6.2</span> [Command Substitution](#h:39f77101-8b9e-48d3-8378-b9286c796412)
    - <span class="section-num">6.3</span> [Test, `[ … ]`, and `\[\[ … \]\]`](#h:2cf87600-2f6d-48c2-be83-78bcd604e5da)
    - <span class="section-num">6.4</span> [Testing Strings](#h:35d9e289-38dc-42e3-a41c-6de70e4d8404)
    - <span class="section-num">6.5</span> [Wildcard Expansion of Filenames](#h:64dfe8cd-4826-44a0-aba3-7eb35ce48f26)
    - <span class="section-num">6.6</span> [Eval](#h:32bb0195-e72a-4225-90ad-7117fde8d0a2)
    - <span class="section-num">6.7</span> [Arrays](#h:2dd03710-f58f-42a5-9a44-e8018bfc560d)
    - <span class="section-num">6.8</span> [Pipes to While](#h:02047bfd-a8ec-4485-9930-17d1182370a5)
    - <span class="section-num">6.9</span> [Arithmetic](#h:2aa69bf1-223e-4c9d-902a-c195f99571a7)
- <span class="section-num">7</span> [Naming Conventions](#h:f05a3c9b-4803-4daf-9f8e-a308ec4cdd7e)
    - <span class="section-num">7.1</span> [Function Names](#h:bb847e7f-6e4d-42ef-9dbe-f21d51289ce3)
    - <span class="section-num">7.2</span> [Variable Names](#h:50d9a19c-c3c1-4f69-9933-5f6d4d4649b0)
    - <span class="section-num">7.3</span> [Constants and Environment Variable Names](#h:58b0b2a6-375f-4a97-804d-ad2fea2ee103)
    - <span class="section-num">7.4</span> [Source Filenames](#h:888f6100-39cf-4d9a-b38b-75cf38526704)
    - <span class="section-num">7.5</span> [Read-only Variables](#h:051bf4c0-5a94-4a7a-96c0-72c4c18e3547)
    - <span class="section-num">7.6</span> [Use Local Variables](#h:31bbd098-0f2e-46fd-83f5-c249dc69e0a1)
    - <span class="section-num">7.7</span> [Function Location](#h:24a30647-7c4c-491f-b703-bbf46125092d)
    - <span class="section-num">7.8</span> [main](#h:798b0a69-eb83-48ff-bcf3-a1860767c727)
- <span class="section-num">8</span> [Calling Commands](#h:04e85d27-2e7a-4bbd-bec6-5ff094ed706f)
    - <span class="section-num">8.1</span> [Checking Return Values](#h:2a0c36d5-95f1-4aae-9b18-2f44a9d992c4)
    - <span class="section-num">8.2</span> [Builtin Commands vs. External Commands](#h:eca45d0b-79dd-4e82-a638-bf072f336d79)
- <span class="section-num">9</span> [Conclusion](#h:36111ffd-58b2-49b6-85f0-8c0f3cfe80f4)

</div>
<!--endtoc-->


本文为摘录，原文为： https://google.github.io/styleguide/shellguide.html



## <span class="section-num">1</span> Background {#h:f6a55800-10d4-4052-be0f-f0d7053dd7b6}

Bash is the only shell scripting language permitted for executables.


## <span class="section-num">2</span> Shell Files and Interpreter Invocation {#h:be0de52e-c0fc-43e7-b1f8-d99e128cef75}


### <span class="section-num">2.1</span> File Extensions {#h:f9b70299-d7f6-431d-a77d-2bb400685496}

-   可执行文件:  应该没有扩展名（强烈推荐）,  或者使用 .sh 扩展名。
-   库文件: 必须使用 .sh 扩展名，且 **不应该是可执行** 的。

在执行一个程序时，不需要知道它使用的是哪种语言， 而且 shell 不需要扩展名，所以我们更倾向于不给可执行文件使用扩展名。

然而，对于库文件，知道使用的是哪种语言很重要， 有时候需要使用不同语言的相似库文件。这样，
具有相同功能但不同语言的库文件可以有相同的文件名，只是在文件名末尾加上特定于语言的后缀。


### <span class="section-num">2.2</span> SUID/SGID {#h:7eb24edb-2c24-4fea-b278-6aa152751b06}

在 shell 脚本中禁止使用 SUID 和 SGID。

由于 shell 存在许多安全问题，几乎不可能足够安全地使用 SUID/SGID。虽然 bash 确实使得运行
SUID 变得困难，但在某些平台上仍然有可能实现，这就是为什么我们明确禁止使用它的原因。

如果需要提供特权访问，请使用 sudo。


## <span class="section-num">3</span> Environment {#h:fa963b24-5dd3-4380-ae40-710090f14e29}


### <span class="section-num">3.1</span> STDOUT vs STDERR {#h:90556c60-d3cd-465b-91c8-1e635abfee5f}

所有错误信息都应该发送到 `STDERR` 。这样可以更容易区分正常状态和实际问题。

建议创建一个函数，用于打印错误信息以及其他状态信息。

```sh
err() {
  echo "[$(date +'%Y-%m-%dT%H:%M:%S%z')]: $*" >&2
}

if ! do_something; then
  err "Unable to do_something"
  exit 1
fi

```


## <span class="section-num">4</span> Comments {#h:e774be8f-ca5b-4520-a57f-c799f69b7675}


### <span class="section-num">4.1</span> File Header {#h:24294cf2-94e9-4f88-9afe-a56220843c64}

每个文件都要以其内容的描述开始。

每个文件必须包含一个顶级注释，其中包含其内容的简要概述。版权声明和作者信息是可选的。

示例:

```sh
#!/bin/bash
#
# Perform hot backups of Oracle databases.

```


### <span class="section-num">4.2</span> Function Comments {#h:43e7e163-58c6-4932-b309-95390eea923d}

-   任何不明显且不简短的函数必须注释。
-   无论长度或复杂性如何，库中的每个函数都必须注释。

通过阅读注释（和提供的自助信息）而无需阅读代码，他人应能够了解如何使用您的程序或使用库中的函数。

所有函数的注释应描述预期的 API 行为，包括：

-   函数的描述。
-   全局变量：使用和修改的全局变量列表。
-   参数：接受的参数。
-   输出：STDOUT 或 STDERR 的输出。
-   返回值：除最后一条运行命令的默认退出状态之外的返回值。

<!--listend-->

```sh
Example:

#######################################
# Cleanup files from the backup directory.
# Globals:
#   BACKUP_DIR
#   ORACLE_SID
# Arguments:
#   None
#######################################
function cleanup() {
  …
}
```


### <span class="section-num">4.3</span> Implementation Comments {#h:4b9e0968-747e-4428-a718-7e54939feffd}

-   对复杂、不明显、有趣或重要的部分加以注释

这遵循了注释的一般惯例。不要对每个地方都进行评论。如果有复杂的算法或者你正在做一些与平常
不同的事情，可以简短地加上注释。


### <span class="org-todo todo TODO">TODO</span> <span class="section-num">4.4</span> Comments {#h:9a26a2da-3497-46f0-a3e1-1bc5739749e4}

-   对于临时的、短期的解决方案或者是足够好但不完美的代码，请使用 TODO 注释。

这与 C++指南的约定一致。

TODO 应该包含全大写的 TODO 字符串，后面跟着在此问题上有最好理解背景的人的名字、电子邮件
地址或其他标识符。主要目的是拥有一致的 TODO，可以通过搜索找到如何根据请求获取更多细节的
方法。TODO 并不是一个承诺，指定的人会解决这个问题。因此，当您创建一个 TODO 时，通常会使
用您自己的名字。

```sh
# TODO(mrmonkey): Handle the unlikely edge cases (bug ####)
```


## <span class="section-num">5</span> Formatting {#h:fa0aab5f-0167-4fa5-a463-ea5b965ac727}

在修改文件时，应遵循已有的样式，但以下规定适用于任何新代码。


### <span class="section-num">5.1</span> Indentation {#h:033bfebd-8148-4e37-bdb7-f1242f62d7a7}

-   缩进两个空格，不要使用制表符。
    -   使用空行来分隔不同的块以提高可读性。缩进为两个空格。
    -   无论你做什么，都不要使用制表符。

-   对于现有的文件，请保持忠于现有的缩进方式。


### <span class="section-num">5.2</span> Line Length and Long Strings {#h:098c6c3a-4ae7-40bb-b75c-e37a811e28d7}

最大行长度是 80 个字符。

-   如果您必须编写超过 80 个字符的字符串，应尽可能使用 here 文档或嵌入换行进行操作。
-   如果字面字符串必须超过 80 个字符且无法合理拆分，则可以接受，但强烈建议找到缩短的方法。

<!--listend-->

```sh
# DO use 'here document's
cat <<END
I am an exceptionally long
string.
END

# Embedded newlines are ok too
long_string="I am an exceptionally
long string."
```


### <span class="section-num">5.3</span> Pipelines {#h:dcbc1b2e-9cf4-48fe-a706-7f3a39386d32}

-   如果一个管道能全部放在一行上，应该放在一行上。
-   如果一个管道不能:
    -   应该在每个管道段落上分开一行，
    -   管道放在新的一行上，
    -   下一个管道段落应有两个空格的缩进。
    -   这适用于使用“|”连接的一系列命令以及使用“||”和“&amp;&amp;”进行逻辑连接的组合。

<!--listend-->

```sh
# All fits on one line
command1 | command2

# Long commands
command1 \
  | command2 \
  | command3 \
  | command4
```


### <span class="section-num">5.4</span> Loops {#h:d5df6dad-79a8-4531-af28-a3e4a797d193}

将 `; do` 和 `; then` 放在与 `while` 、 `for` 或 `if` 同一行。

在 shell 中的循环有点不同，但我们遵循与声明函数时相同的原则。也就是说：

-   ; then 和 ; do 应该与 if/for/while 放在同一行
-   else 应该自成一行，
-   关闭语句应该垂直对齐于开放语句的行。

<!--listend-->

```sh
# If inside a function, consider declaring the loop variable as
# a local to avoid it leaking into the global environment:
# local dir
for dir in "${dirs_to_cleanup[@]}"; do
  if [[ -d "${dir}/${ORACLE_SID}" ]]; then
    log_date "Cleaning up old files in ${dir}/${ORACLE_SID}"
    rm "${dir}/${ORACLE_SID}/"*
    if (( $? != 0 )); then
      error_message
    fi
  else
    mkdir -p "${dir}/${ORACLE_SID}"
    if (( $? != 0 )); then
      error_message
    fi
  fi
done
```


### <span class="section-num">5.5</span> Case statement {#h:033449c1-3dc3-4148-9de3-099eb0e1f366}

-   替代缩进使用 2 个空格。
-   一行替代方案需要在模式的关闭括号后和 ;; 前加上一个空格。
-   长或多命令的替代方案应该分成多行，模式、行为和 ;; 分别放在不同的行上。

匹配表达式离 case 和 esac 缩进一个层级。多行行为再缩进一个层级。通常情况下，没有必要对匹
配表达式进行引号。模式表达式之前不应有左括号。避免使用 ;&amp; 和 ;;&amp; 符号。

```sh
case "${expression}" in
  a)
    variable="…"
    some_command "${variable}" "${other_expr}" …
    ;;
  absolute)
    actions="relative"
    another_command "${actions}" "${other_expr}" …
    ;;
  *)
    error "Unexpected expression '${expression}'"
    ;;
esac

```

只要表达式可读性没有受到影响，简单命令可以与模式和;;放在同一行上。

这通常适用于单字母选项处理。当动作无法放在一行上时，将模式放在独立的一行上，然后是动作，
然后再另起一行写;;。当与动作在同一行上时，在模式的闭括号后加一个空格，再在;;之前加一个空
格。

```sh
verbose='false'
aflag=''
bflag=''
files=''
while getopts 'abf:v' flag; do
  case "${flag}" in
    a) aflag='true' ;;
    b) bflag='true' ;;
    f) files="${OPTARG}" ;;
    v) verbose='true' ;;
    *) error "Unexpected option ${flag}" ;;
  esac
done

```


### <span class="section-num">5.6</span> Variable expansion {#h:ee435b39-dad7-4604-a699-738299997086}

按照优先顺序：

-   保持与您发现的代码一致；
-   引用您的变量 (使用  `"` )；
-   优先使用 `"${var}"` 而非 `"$var"` 。

这些是强烈推荐的准则，但并非强制性规定。然而，推荐而非强制并不意味着可以轻视或低估。


### <span class="section-num">5.7</span> Quoting {#h:59bf8094-699c-4c8b-ae73-65616213a707}

-   无论如何都要引用包含变量、命令替换、空格或 shell 元字符的字符串 <br />
    除非需要小心地取消引用扩展或它是一个 shell 内部整数（见下一条）。
-   使用数组来安全引用元素列表，尤其是命令行选项 <br />
    详见下面的数组部分。
-   可选择引用 shell 内部的只读特殊变量
    -   这些变量被定义为整数：\\(?、\\)#、$$、$!（参见 man bash）。
    -   为了一致性，更喜欢引用“命名”的内部整数变量，如 PPID 等。
-   更倾向于引用作为“单词”的字符串（而不是命令选项或路径名）。
-   永远不要引用字面整数。
-   了解 `\[\[..\]\]` ]]中模式匹配的引用规则。<br />
    详见下面的章节。
-   除非有特定的原因要使用 `$*` ，否则使用 `"$@"` ，比如只是将参数追加到消息或日志的字符串中。

<!--listend-->

```sh
# 'Single' quotes indicate that no substitution is desired.
# "Double" quotes indicate that substitution is required/tolerated.

# Simple examples

# "quote command substitutions"
# Note that quotes nested inside "$()" don't need escaping.
flag="$(some_command and its args "$@" 'quoted separately')"

# "quote variables"
echo "${flag}"

# Use arrays with quoted expansion for lists.
declare -a FLAGS
FLAGS=( --foo --bar='baz' )
readonly FLAGS
mybinary "${FLAGS[@]}"

# It's ok to not quote internal integer variables.
if (( $# > 3 )); then
  echo "ppid=${PPID}"
fi

# "never quote literal integers"
value=32
# "quote command substitutions", even when you expect integers
number="$(generate_number)"

# "prefer quoting words", not compulsory
readonly USE_INTEGER='true'

# "quote shell meta characters"
echo 'Hello stranger, and well met. Earn lots of $$$'
echo "Process $$: Done making \$\$\$."

# "command options or path names"
# ($1 is assumed to contain a value here)
grep -li Hugo /dev/null "$1"

# Less simple examples
# "quote variables, unless proven false": ccs might be empty
git send-email --to "${reviewers}" ${ccs:+"--cc" "${ccs}"}

# Positional parameter precautions: $1 might be unset
# Single quotes leave regex as-is.
grep -cP '([Ss]pecial|\|?characters*)$' ${1:+"$1"}

# For passing on arguments,
# "$@" is right almost every time, and
# $* is wrong almost every time:
#
# * $* and $@ will split on spaces, clobbering up arguments
#   that contain spaces and dropping empty strings;
# * "$@" will retain arguments as-is, so no args
#   provided will result in no args being passed on;
#   This is in most cases what you want to use for passing
#   on arguments.
# * "$*" expands to one argument, with all args joined
#   by (usually) spaces,
#   so no args provided will result in one empty string
#   being passed on.
# (Consult `man bash` for the nit-grits ;-)

(set -- 1 "2 two" "3 three tres"; echo $#; set -- "$*"; echo "$#, $@")
(set -- 1 "2 two" "3 three tres"; echo $#; set -- "$@"; echo "$#, $@")
```


## <span class="section-num">6</span> Features and Bugs {#h:f07ebca3-d96a-4b10-8f24-043ef9cfec52}


### <span class="section-num">6.1</span> ShellCheck {#h:8c4ba5f5-2f9e-41df-8f4d-1d1f7c143b71}

[ShellCheck](https://www.shellcheck.net/) 项目可帮助您识别 shell 脚本中的常见错误和警告。无论是大型还是小型脚本，都推荐
使用该工具。


### <span class="section-num">6.2</span> Command Substitution {#h:39f77101-8b9e-48d3-8378-b9286c796412}

使用 `$(command)` 代替反引号。

嵌套的反引号需要用 `\` 来转义内部的反引号。 `$(command)` 的格式在嵌套时不会改变，并且更容易读取。

Example:

```sh
# This is preferred:
var="$(command "$(command1)")"
# This is not:
var="`command \`command1\``"
```


### <span class="section-num">6.3</span> Test, `[ … ]`, and `\[\[ … \]\]` {#h:2cf87600-2f6d-48c2-be83-78bcd604e5da}

`[[ ... ]]` is preferred over `[ … ]`, `test` and `/usr/bin/[`.

`[[ ... ]]`  减少错误的发生，因为在 `[[ ... ]]` 之间没有路径名展开或者单词分割。
此外， `[[ ... ]]`  可以进行正则表达式匹配，而 `[ … ]` 不可以。


### <span class="section-num">6.4</span> Testing Strings {#h:35d9e289-38dc-42e3-a41c-6de70e4d8404}

在可能的情况下，请使用引号而不是填充字符 (filler character)。

Bash 足够聪明，能够处理测试中的空字符串。因此，考虑到代码更易读性，应该使用 test 来
处理空字符串或非空字符串，而不是使用填充字符。

```sh
# Do this:
if [[ "${my_var}" == "some_string" ]]; then
  do_something
fi

# -z (string length is zero) and -n (string length is not zero) are
# preferred over testing for an empty string
if [[ -z "${my_var}" ]]; then
  do_something
fi

# This is OK (ensure quotes on the empty side), but not preferred:
if [[ "${my_var}" == "" ]]; then
  do_something
fi
# Not this:
if [[ "${my_var}X" == "some_stringX" ]]; then
  do_something
fi

To avoid confusion about what you’re testing for, explicitly use -z or -n.

# Use this
if [[ -n "${my_var}" ]]; then
  do_something
fi
# Instead of this
if [[ "${my_var}" ]]; then
  do_something
fi
```

为了明确起见，使用==表示相等，而不是=，尽管两者都可以工作。前者鼓励使用[[，而后者可能会
与赋值混淆。但是，在[\\[&#x2026;]]中使用&lt;和&gt;时要小心，它将执行词典比较。使用((&#x2026;))或-lt 和-gt 进行数
值比较。

```sh
# Use this
if [[ "${my_var}" == "val" ]]; then
  do_something
fi
if (( my_var > 3 )); then
  do_something
fi
if [[ "${my_var}" -gt 3 ]]; then
  do_something
fi
# Instead of this
if [[ "${my_var}" = "val" ]]; then
  do_something
fi
# Probably unintended lexicographical comparison.
if [[ "${my_var}" > 3 ]]; then
  # True for 4, false for 22.
  do_something
fi
```


### <span class="section-num">6.5</span> Wildcard Expansion of Filenames {#h:64dfe8cd-4826-44a0-aba3-7eb35ce48f26}

当使用通配符扩展文件名时，请使用显式路径。

由于文件名可能以 - 开头，使用 `./*` 而不是 `*` 来扩展通配符会更安全。

```sh
# Here's the contents of the directory:
# -f  -r  somedir  somefile

# Incorrectly deletes almost everything in the directory by force
psa@bilby$ rm -v *
removed directory: `somedir'
removed `somefile'
# As opposed to:
psa@bilby$ rm -v ./*
removed `./-f'
removed `./-r'
rm: cannot remove `./somedir': Is a directory
removed `./somefile'
```


### <span class="section-num">6.6</span> Eval {#h:32bb0195-e72a-4225-90ad-7117fde8d0a2}

`eval` 应该避免使用。

当用于对变量赋值时，eval 会修改输入，并且可以设置变量，无法检查这些变量是什么。

```sh
# What does this set?
# Did it succeed? In part or whole?
eval $(set_my_variables)

# What happens if one of the returned values has a space in it?
variable="$(eval some_function)"
```


### <span class="section-num">6.7</span> Arrays {#h:2dd03710-f58f-42a5-9a44-e8018bfc560d}

-   Bash 数组应该用于存储元素列表，以避免引号引用的复杂性。
    -   这尤其适用于参数列表。不应使用数组来方便更复杂的数据结构（参见上面的使用 Shell 时）。

-   数组存储有序的字符串集合，并且可以安全地展开为命令或循环的单个元素。

-   应避免使用单个字符串作为多个命令参数，<br />
    因为这不可避免地导致作者使用 eval 或尝试在字符串中嵌套引号，这不会产生可靠或可读的结果，并且导致不必要的复杂性。

<!--listend-->

```sh
# An array is assigned using parentheses, and can be appended to
# with +=( … ).
declare -a flags
flags=(--foo --bar='baz')
flags+=(--greeting="Hello ${name}")
mybinary "${flags[@]}"
# Don’t use strings for sequences.
flags='--foo --bar=baz'
flags+=' --greeting="Hello world"'  # This won’t work as intended.
mybinary ${flags}
# Command expansions return single strings, not arrays. Avoid
# unquoted expansion in array assignments because it won’t
# work correctly if the command output contains special
# characters or whitespace.

# This expands the listing output into a string, then does special keyword
# expansion, and then whitespace splitting.  Only then is it turned into a
# list of words.  The ls command may also change behavior based on the user's
# active environment!
declare -a files=($(ls /directory))

# The get_arguments writes everything to STDOUT, but then goes through the
# same expansion process above before turning into a list of arguments.
mybinary $(get_arguments)

```


#### <span class="section-num">6.7.1</span> Arrays Pros {#h:ddb31c21-29a3-46e3-ad4f-21146f74ff4b}

-   使用数组可以在不混淆引用语义的情况下创建列表。相反地，不使用数组会导致在字符串内部错误地嵌套引用的尝试。
-   数组使得可以安全地存储包含空格的任意字符串的序列/列表。


#### <span class="section-num">6.7.2</span> Arrays Cons {#h:1d90b364-8f46-4129-ba9d-2e2240b81a0a}

使用数组可能会增加脚本的复杂性增加的风险。


#### <span class="section-num">6.7.3</span> Arrays Decision {#h:86f5e27f-5817-410f-afec-e16e1eacc75f}

应该使用数组来安全地创建和传递列表。特别是在构建一组命令参数时，应该使用数组来避免引号的混乱问题。使用引用扩展 - "${array[@]}" - 来访问数组。然而，如果需要更高级的数据操作，应该完全避免使用 shell 脚本;参见上面的内容。


### <span class="section-num">6.8</span> Pipes to While {#h:02047bfd-a8ec-4485-9930-17d1182370a5}

使用进程替换或者优先使用 bash4+里的 readarray 命令，而不是使用管道传递给 while。

管道会创建一个子 shell，所以在管道中修改的变量不会传递到父 shell。

管道传递给 while 的隐式子 shell 可能会引入难以追踪的细微错误。

```sh
last_line='NULL'
your_command | while read -r line; do
  if [[ -n "${line}" ]]; then
    last_line="${line}"
  fi
done

# This will always output 'NULL'!
echo "${last_line}"

```

使用进程替换也会创建一个子 shell。然而，它允许从子 shell 重定向到一个 while 循环，而无需
将 while（或任何其他命令）放入子 shell 中。


### <span class="section-num">6.9</span> Arithmetic {#h:2aa69bf1-223e-4c9d-902a-c195f99571a7}

-   始终使用(( … ))或\\((( … ))而不是 let 或\\)[ … ]或 expr。
-   永远不要使用$[ … ]语法，expr 命令或 let 内置命令。
-   &lt;and&gt;在\\[[]]表达式内部不执行数字比较（而执行字典比较；请参见字符串测试）。
-   不要对数值比较使用[\\[]]，而是使用 `(( … ))` 。
-   特别是在启用 set -e 的情况下。例如，set -e; i=0; (( i++ ))将导致
    shell 退出。

<!--listend-->

```sh
# Simple calculation used as text - note the use of $(( … )) within
# a string.
echo "$(( 2 + 2 )) is 4"

# When performing arithmetic comparisons for testing
if (( a < b )); then
  …
fi

# Some calculation assigned to a variable.
(( i = 10 * j + 400 ))
# This form is non-portable and deprecated
i=$[2 * 10]

# Despite appearances, 'let' isn't one of the declarative keywords,
# so unquoted assignments are subject to globbing wordsplitting.
# For the sake of simplicity, avoid 'let' and use (( … ))
let i="2 + 2"

# The expr utility is an external program and not a shell builtin.
i=$( expr 4 + 4 )

# Quoting can be error prone when using expr too.
i=$( expr 4 '*' 4 )

```


## <span class="section-num">7</span> Naming Conventions {#h:f05a3c9b-4803-4daf-9f8e-a308ec4cdd7e}


### <span class="section-num">7.1</span> Function Names {#h:bb847e7f-6e4d-42ef-9dbe-f21d51289ce3}

-   小写字母，用下划线分隔单词。
-   使用双冒号来分隔库。
-   在函数名后面需要使用括号。
-   关键字 function 是可选的，但必须在整个项目中保持一致使用。
-   如果你正在编写单个函数，请使用小写字母，并使用下划线分隔单词。
-   如果你正在编写一个包，应使用双冒号分隔包名。
-   大括号必须与函数名在同一行（与 Google 的其他语言一样），函数名与括号之间不要有空格。

<!--listend-->

```sh
# Single function
my_func() {
  …
}

# Part of a package
mypackage::my_func() {
  …
}

```

当函数名后面跟着“()”时，“function”关键字是多余的，但它能够加强对函数的快速识别。


### <span class="section-num">7.2</span> Variable Names {#h:50d9a19c-c3c1-4f69-9933-5f6d4d4649b0}

关于函数名称。

循环变量的变量名称应与您正在遍历的任何变量命名方式相似。

```sh
for zone in "${zones[@]}"; do
  something_with "${zone}"
done

```


### <span class="section-num">7.3</span> Constants and Environment Variable Names {#h:58b0b2a6-375f-4a97-804d-ad2fea2ee103}

-   所有字母大写，用下划线分隔，在文件顶部声明。
-   常量和任何导出到环境的内容都应大写。

<!--listend-->

```sh
# Constant
readonly PATH_TO_FILES='/some/path'

# Both constant and environment
declare -xr ORACLE_SID='PROD'

```

有些东西在它们第一次设置后变得恒定（例如通过 getopts）。因此，在 getopts 或基于条件设置一个
constant 是可以的，但是之后应立即将其设置为 readonly。为了清晰起见，建议使用 readonly 或
export 而不是等效的 declare 命令。

```sh
VERBOSE='false'
while getopts 'v' flag; do
  case "${flag}" in
    v) VERBOSE='true' ;;
  esac
done
readonly VERBOSE

```


### <span class="section-num">7.4</span> Source Filenames {#h:888f6100-39cf-4d9a-b38b-75cf38526704}

使用小写字母，并在需要时用下划线分隔单词。

这是为了与 Google 中的其他代码风格保持一致：maketemplate 或 make_template，而不是 make-template。


### <span class="section-num">7.5</span> Read-only Variables {#h:051bf4c0-5a94-4a7a-96c0-72c4c18e3547}

使用 readonly 或 declare -r 来确保它们是只读的。

由于全局变量在 shell 中被广泛使用，因此在处理它们时捕捉错误是很重要的。当你声明一个变量
是只读的时候，要明确表示这一点。

```sh
zip_version="$(dpkg --status zip | grep Version: | cut -d ' ' -f 2)"
if [[ -z "${zip_version}" ]]; then
  error_message
else
  readonly zip_version
fi
```


### <span class="section-num">7.6</span> Use Local Variables {#h:31bbd098-0f2e-46fd-83f5-c249dc69e0a1}

使用 local 关键字在函数内部声明具体函数变量。声明和赋值应该分开放在不同的行上。

通过使用 local 关键字在声明变量时，确保本地变量只在函数及其子函数内部可见。这样可以避免污染全局命名空间，并意外地设置对函数外部可能有重要意义的变量。

当赋值值由命令替换提供时，声明和赋值必须是分开的语句；因为 local 内建函数不会传播来自命
令替换的退出代码。

```sh
my_func2() {
  local name="$1"

  # Separate lines for declaration and assignment:
  local my_var
  my_var="$(my_func)"
  (( $? == 0 )) || return

  …
}
my_func2() {
  # DO NOT do this:
  # $? will always be zero, as it contains the exit code of 'local', not my_func
  local my_var="$(my_func)"
  (( $? == 0 )) || return

  …
}

```


### <span class="section-num">7.7</span> Function Location {#h:24a30647-7c4c-491f-b703-bbf46125092d}

将所有函数放在常量的下方，不要在函数之间隐藏可执行代码。这样做会使代码难以跟踪，并在调试
时会产生令人讨厌的意外。

如果您有函数，请将它们都放在文件的顶部附近。只有 includes、set 语句和设置常量可以在声明
函数之前完成。


### <span class="section-num">7.8</span> main {#h:798b0a69-eb83-48ff-bcf3-a1860767c727}

如果脚本足够长，至少包含一个其他函数，则需要一个名为 main 的函数。

为了方便找到程序的起始点，将主程序放在一个名为 main 的函数中，作为最底层的函数。这样可以保
持与代码库其余部分的一致性，同时还允许您将更多变量定义为本地变量（如果主代码不是函数，则
无法完成此操作）。文件中最后一个非注释行应该是对 main 的调用：


## <span class="section-num">8</span> Calling Commands {#h:04e85d27-2e7a-4bbd-bec6-5ff094ed706f}


### <span class="section-num">8.1</span> Checking Return Values {#h:2a0c36d5-95f1-4aae-9b18-2f44a9d992c4}

始终检查返回值并提供有信息的返回值。

对于未使用管道的命令，请使用 $? 或直接通过 if 语句进行检查，以保持简单。

```sh
if ! mv "${file_list[@]}" "${dest_dir}/"; then
  echo "Unable to move ${file_list[*]} to ${dest_dir}" >&2
  exit 1
fi

# Or
mv "${file_list[@]}" "${dest_dir}/"
if (( $? != 0 )); then
  echo "Unable to move ${file_list[*]} to ${dest_dir}" >&2
  exit 1
fi

```

Bash 还有一个名为 PIPESTATUS 的变量，允许检查管道中所有部分的返回代码。如果仅需要检查整个管
道的成功或失败，以下方法也是可行的：

```sh
tar -cf - ./* | ( cd "${dir}" && tar -xf - )
if (( PIPESTATUS[0] != 0 || PIPESTATUS[1] != 0 )); then
  echo "Unable to tar files to ${dir}" >&2
fi
```

然而，一旦您执行其他命令，PIPESTATUS 将被覆盖，所以如果您需要根据管道中发生错误的位置来进行不同的错误处理，您需要在运行命令后立即将 PIPESTATUS 分配给另一个变量（请记住，[ 是一个命令，会清除
 PIPESTATUS）。

```sh
tar -cf - ./* | ( cd "${DIR}" && tar -xf - )
return_codes=( "${PIPESTATUS[@]}" )
if (( return_codes[0] != 0 )); then
  do_something
fi
if (( return_codes[1] != 0 )); then
  do_something_else
fi
```


### <span class="section-num">8.2</span> Builtin Commands vs. External Commands {#h:eca45d0b-79dd-4e82-a638-bf072f336d79}

在选择调用内置 shell 和调用独立进程之间，选择内置 shell。

我们更喜欢使用内置的函数，例如 bash(1) 中的参数扩展函数，因为它更健壮和可移植（特别是与类似 sed 的工具相比较）。

```sh
# Prefer this:
addition=$(( X + Y ))
substitution="${string/#foo/bar}"
# Instead of this:
addition="$(expr "${X}" + "${Y}")"
substitution="$(echo "${string}" | sed -e 's/^foo/bar/')"
```


## <span class="section-num">9</span> Conclusion {#h:36111ffd-58b2-49b6-85f0-8c0f3cfe80f4}

Use common sense and BE CONSISTENT.

