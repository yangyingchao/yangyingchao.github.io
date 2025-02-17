# Linux Installation Date: How to Discover Your System's Age


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [A Universal Method](#h:7f02f7ac-11bd-455d-9d23-0363894bb19b)
- <span class="section-num">2</span> [Debian / Ubuntu](#h:abb7ea13-5f80-44df-8ed0-58ae42bb0ed6)
- <span class="section-num">3</span> [Fedora / Rocky Linux / AlmaLinux](#h:e6bf4bd0-e5df-489b-bbc4-b261df1355f6)
- <span class="section-num">4</span> [Arch Linux](#h:724b944d-9451-4519-b107-ef97dd3a88f9)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://linuxiac.com/how-to-find-linux-os-installation-date/



## <span class="section-num">1</span> A Universal Method {#h:7f02f7ac-11bd-455d-9d23-0363894bb19b}

首先，我们必须澄清一点，在 Linux 中没有专门的设置、变量、日志文件条目或类似的东西来明确存储系统安装的实际日期和时间的信息。

然而，通过确定根目录（“/”）文件系统的创建时间，可以很容易地检索到此信息。

可以借助 stat 命令来实现这一目标，该命令能够显示文件或目录的详细信息，包括创建、访问和修改时间戳、文件类型、权限等元数据。

因此，要找到您的 Linux 系统安装的确切日期和时间，请运行以下命令：

```sh
LANG=C stat / | awk '/Birth: /{print $2 " " substr($3,1,5)}'
```

当我们逐步分解这个 awk 命令时：

-   \`/Birth: /\`：这是 awk 在输入的每一行中搜索的模式。
-   \`{print $2 " " substr($3,1,5)}\`：这是 awk 在找到匹配模式的行时执行的操作。下面是每个部分的作用：
    -   \`$2\` 指代当前行的第二个字段（列），也就是日期。
    -   \`substr($3, 1, 5)\` 从当前行的第三个字段（列）中提取一个子字符串，从第一个字符开始，长度为 5，也就是时间。
    -   \`" "\` 是字符串文字，用于连接日期和时间之间的空格。

因此，该命令会搜索包含"Birth: "的行，并从相应的字段中打印出日期和时间。


## <span class="section-num">2</span> Debian / Ubuntu {#h:abb7ea13-5f80-44df-8ed0-58ae42bb0ed6}

在 Debian 和 Ubuntu 系统以及它们的所有衍生系统（如 Linux Mint 和其他系统），你可以通过显示安装程序 syslog 文件的第一行来查看它们被安装的确切日期和时间。

`sudo head -n1 /var/log/installer/syslog`


## <span class="section-num">3</span> Fedora / Rocky Linux / AlmaLinux {#h:e6bf4bd0-e5df-489b-bbc4-b261df1355f6}

在 Fedora、Red Hat Enterprise Linux 及其所有衍生版（如 Rocky Linux、AlmaLinux、Oracle Linux 等）中，我们可以通过检查“basesystem”软件包的安装日期，将其用作可靠的标记来确认操作系统的安装日期。

`sudo rpm -qi basesystem | grep -i "install date"`

请记住，然而，如果你正在进行原地升级，比如从 Fedora 38 升级到 Fedora 39，Rocky 9.1 升级到 Rocky 9.2 等等，在检索“basesystem”包信息时显示的日期将是升级时的时间。

在这种情况下，要获取最初安装的实际日期，使用文章开头提到的 stat 命令的前两种通用方法之一。


## <span class="section-num">4</span> Arch Linux {#h:724b944d-9451-4519-b107-ef97dd3a88f9}

如果你正在使用 Arch Linux 或其衍生版如 Manjaro 或 EndeavourOS，那么“pacman.log”文件的第一行将告诉你你的 Linux 系统安装的时间。

`head -n1 /var/log/pacman.log`

