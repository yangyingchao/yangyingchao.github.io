# Tips of Bash/Zsh


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [展开 Bash 数组 (array) 时候， `${ARRARY[@]}` 和 `${ARRARY[*]}` 有什么区别？](#h:c2349cd1-7eb4-4ae1-9eea-323aea44ee72)
- <span class="section-num">2</span> [How do you escape characters in heredoc?](#h:f4db2d27-d344-47c3-b020-b392cc7b33d6)
    - <span class="section-num">2.1</span> [Question](#h:289ce225-b573-4798-89f3-4946fe49b3bb)
    - <span class="section-num">2.2</span> [Answer](#h:8389b13e-5ee7-4338-9e6b-4726d406d481)
- <span class="section-num">3</span> [special expansion:](#h:e6d62956-9ea2-45cb-805a-200499519c7c)
- <span class="section-num">4</span> [will .bash_profile be sourced when executing scripts?](#h:6ddc502c-c88b-4d32-9b95-10e2c0d4e77e)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://unix.stackexchange.com/questions/505949/expanding-only-certain-variables-inside-a-heredoc



## <span class="section-num">1</span> 展开 Bash 数组 (array) 时候， `${ARRARY[@]}` 和 `${ARRARY[*]}` 有什么区别？ {#h:c2349cd1-7eb4-4ae1-9eea-323aea44ee72}

INFO 中有相关说明：

> 数组的任何元素可以使用'${NAME[SUBSCRIPT]}'来引用。
>
> 这些大括号是为了避免与 shell 的文件名扩展操作符产生冲突。如果 SUBSCRIPT 为'@'或'\*'，那么
> 该词会扩展为数组 NAME 的所有成员。这些下标在词出现在双引号内时才有所不同。如果该词被双引
> 号括起来，'\\({NAME[\*]}'会扩展为单词，其值为每个数组成员之间以 IFS 变量的第一个字符分隔，
> 而'\\){NAME[@]}'会将 NAME 的每个元素扩展为独立的单词。当数组没有成员时，'${NAME[@]}'扩展为
> 空。如果双引号扩展出现在一个单词中，第一个参数的扩展将与原始单词的开头部分连接，而最后一
> 个参数的扩展将与最后部分连接。

例子：

```sh
a=('1' "2" "3" "4")
export IFS="|"

echo "${a[@]}"
echo "${a[*]}"
```

输出为：

```text
-*- mode: compilation; default-directory: "~/tmp/" -*-
1 2 3 4
1|2|3|4
```


## <span class="section-num">2</span> How do you escape characters in heredoc? {#h:f4db2d27-d344-47c3-b020-b392cc7b33d6}


### <span class="section-num">2.1</span> Question {#h:289ce225-b573-4798-89f3-4946fe49b3bb}

我正在使用一个 bash 脚本，试图阻止它尝试替换 heredoc 中的变量。如何将 heredoc 设置为 A）转义变
量名而不解析它们，或者 B）返回整个字符串而不改变它？

```bash-ts
cat > /etc/nginx/sites-available/default_php <<END
server {
    listen 80 default;
    server_name _;
    root /var/www/$host; <--- $host is a problem child
}
END
```

就现状而言，当我将它注入到文件中结束时，我得到的是这个：

```cfg
server {
    listen 80 default;
    server_name _;
    root /var/www/;
}
```


### <span class="section-num">2.2</span> Answer {#h:8389b13e-5ee7-4338-9e6b-4726d406d481}

从 bash(1) man 页:

如果 word 中的任何字符被引用，那么分隔符就是对 word 进行引号移除的结果，并且 here-document 中的行不会被展开。

```bash-ts
cat > /etc/nginx/sites-available/default_php <<"END"
```

```sh
a="this is a test"
cat <<EOF
        echo "a should be expanded: $a"
EOF

cat <<'EOF'
        echo "a should not be expanded: $a"
EOF
```


## <span class="section-num">3</span> special expansion: {#h:e6d62956-9ea2-45cb-805a-200499519c7c}

```sh

cat <<-'EOF'
### ${PARAMETER:+WORD}
`${PARAMETER:+WORD}` 是一种 bash shell 中的参数扩展语法，它的含义如下：

- 如果变量 `PARAMETER` 已经被设置并且不是空值，那么该表达式的值为 `WORD`。
- 如果变量 `PARAMETER` 没有被设置或者是空值，那么该表达式的值为为空字符串。

在这种情况下，`${PARAMETER:+WORD}` 的作用是在变量 `PARAMETER` 已经被设置并且不是空值时，返回 `WORD`；否则返回空字符串。

EOF

A=MMM
echo "1: ${A:+WORD}"
A=
echo "2: ${A:+WORD}"


cat <<-'EOF'
### ${PARAMETER:-WORD}

`${PARAMETER:-WORD}` 是一种 bash shell 中的参数扩展语法，它的含义如下：

- 如果变量 `PARAMETER` 已经被设置并且不是空值，那么该表达式的值为变量 `PARAMETER` 的值。
- 如果变量 `PARAMETER` 没有被设置或者是空值，那么该表达式的值为 `WORD`。

在这种情况下，`${PARAMETER:-WORD}` 的作用是在变量 `PARAMETER` 未设置或者为空值时提供一个默认值 `WORD`。

EOF

echo "----"
A=MMM
echo "1: ${A:-WORD}"
A=
echo "2: ${A:-WORD}"


cat <<-'EOF'

### ${PARAMETER:=WORD}

`${PARAMETER:=WORD}` 是一种 bash shell 中的参数扩展语法，它的含义如下：

- 如果变量 `PARAMETER` 已经被设置并且不是空值，那么该表达式的值为变量 `PARAMETER` 的值。
- 如果变量 `PARAMETER` 没有被设置或者是空值，那么该表达式的值为 `WORD`，并且同时将变量 `PARAMETER` 设置为 `WORD` 的值。

在这种情况下，`${PARAMETER:=WORD}` 的作用是在变量 `PARAMETER` 未设置或者为空值时，用默认值 `WORD` 初始化该变量。

EOF

echo "----"
A=MMM
echo "1: ${A:=WORD}"
A=
echo "2: ${A:=WORD}"

```


## <span class="section-num">4</span> will .bash_profile be sourced when executing scripts? {#h:6ddc502c-c88b-4d32-9b95-10e2c0d4e77e}

> .bash_profile gets called when you login and not at restart. ~/.bash_profile is only
> sourced by bash when started in interactive login mode.
> When you log in graphically, ~/.profile will be specifically sourced by the script that
> launches gnome-session (or whichever desktop environment you're using). So
> ~/.bash_profile is not sourced at all when you log in graphically.
> When you open a terminal, the terminal starts bash in (non-login) interactive mode,
> which means it will source ~/.bashrc.
> The right place for you to put these environment variables is in ~/.profile, and the
> effect should be apparent next time you log in.
> Sourcing ~/.bash_profile from ~/.bashrc is the wrong solution. It's supposed to be the
> other way around; ~/.bash_profile should source ~/.bashrc. You also need to enable crond
> service via sys v / BSD init style system.

<!--quoteend-->

> 当您登录时会调用.bash_profile，而在重新启动时不会调用。只有当 bash 以交互式登录模式启动
> 时，才会通过.bash_profile 进行资源引用。
> 当您通过图形界面登录时，~/.profile 将被启动 gnome-session（或您正在使用的其他桌面环境）
> 的脚本特别调用。因此，在图形登录时根本不会调用~/.bash_profile。
> 当您打开终端时，终端会以（非登录的）交互模式启动 bash，这意味着它将会引用~/.bashrc。
> 您应该将这些环境变量放在~/.profile 中，下次登录时效果应该就会显现出来。
> 从~/.bashrc 中引用~/.bash_profile 是错误的解决方案。正确的做法是相反的；~/.bash_profile 应
> 该引用~/.bashrc。您还需要通过 sys v / BSD init 样式系统来启用 crond 服务。

