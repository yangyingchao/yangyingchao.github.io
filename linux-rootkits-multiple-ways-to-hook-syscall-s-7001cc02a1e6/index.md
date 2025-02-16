# Linux Rootkits — Multiple ways to hook syscall(s)


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Syscall Table hijacking — The good old way, 系统调用表劫持](#h:98e4d628-ec24-45db-b9b1-fff7eba4a0af)
- <span class="section-num">2</span> [sys_close — The brute force method](#h:45798689-8b14-43ec-8000-bbecea4d896c)
- <span class="section-num">3</span> [VFS hooking](#h:889b5097-0190-4799-a9c6-5cb7d50ad082)
- <span class="section-num">4</span> [The ftrace helper method](#h:ccc260a9-1238-4340-9ad4-0bd94a9a1d1e)

</div>
<!--endtoc-->


本文为摘录，原文为： https://foxtrot-sq.medium.com/linux-rootkits-multiple-ways-to-hook-syscall-s-7001cc02a1e6

我们所见到的大多数恶意软件攻击中使用的 rootkit 都是开源的，其行为几乎与在系统中运行的正常
进程相同（隐藏和挂钩）。从行为上来看，它们与正常进程几乎没有任何区别。在本文中，我们将探
讨一些现有的方法，根据不同的 Linux 内核版本来挂钩系统调用。

我们将使用多种方式进行研究：

-   Syscall 表劫持 &#x2013; 传统的方法
-   Sys_close —— 暴力方法
-   VFS 挂钩
-   ftrace 助手方法


## <span class="section-num">1</span> Syscall Table hijacking — The good old way, 系统调用表劫持 {#h:98e4d628-ec24-45db-b9b1-fff7eba4a0af}

我们将讨论的第一种 hook syscall 的方法是通过 syscall 表劫持。想要劫持 syscall 表并从中 hook syscall，需要对表具有写访问权限。
`syscall table` 是 **syscall ID** 和 **其实现的内核地址** 之间的映射。
为了修改表格，我们必须从只读获得写入权限。 这可以通过处理器中的控制寄存器来实现，该控制寄存器决定 CPU 的行为。
要修改 syscall 表， `cr0` 寄存器的 `WP` 位被设置为 `0` ，我们只需将其更改为 `1` 即可。

{{< figure src="/ox-hugo/0*ebkS9fCnoi6kCj_0.png" width="800px" >}}

现在，我们可以轻松修改系统调用表。
在 Linux 系统中， **/proc/kallsyms** 中包含了内核系统调用表（或 sys_call_table）的映射。

为了通过代码找到表格，我们将使用 syscall `kallsyms_lookup_name()` ，该函数在 kallsyms.h
中定义，用于获取函数的地址。在 Linux 内核 4.4 版本中，kallsyms_lookup_name()调用被导出，
我们可以使用它来定位 sys_call_table 的地址:

{{< figure src="/ox-hugo/0*SvmaA3_yha30zuq_.png" width="620px" >}}

一旦系统调用表的地址被确定，我们可以像下图所示轻松修改表中的条目。

{{< figure src="/ox-hugo/0*qf1DTRJXFGkpdlha.png" width="800px" >}}

使用 kallsyms_lookup_name 来查找 sys_call_table():

{{< figure src="/ox-hugo/0*MeXVwAxFyFDSyvnK.png" width="800px" >}}

Inside my_read function

{{< figure src="/ox-hugo/0*Bkk_KNBUrF4QFZXY.png" width="670px" >}}

让我们把 Rootkit 加载进系统：

{{< figure src="/ox-hugo/0*hsviOjxtwOJIVpUU.png" width="800px" >}}

Rootkit 成功地被加载了。因为我们勾住了 read()函数，所以我们得到了大量的日志记录。


## <span class="section-num">2</span> sys_close — The brute force method {#h:45798689-8b14-43ec-8000-bbecea4d896c}

该方法其实还是在获取 syscall_table

sys_close() 系统调用在旧版本的内核中（我们这里使用的是 4.4 版本）被导出，我们可以通过它
来扫描内核内存中的 sys_call_table() 的地址。这是一种通过导出的函数查找系统调用表地址的简
单方法。下面的代码片段展示了我们尝试定位 sys_call_table 的扫描代码。

{{< figure src="/ox-hugo/0*wEGycGk6hDqqqki7.png" width="800px" >}}

上面的代码片段非常简单。首先我们只是将 sys_close 的地址存储在 offset 变量中，然后我们只是扫
描内存以匹配表中的//NR_close 条目与 sys_close()。一旦匹配，我们就返回 sys_call_table 的地址
（即上面的 sct）。

现在，我们只需要用我们自己的调用替换原来的 read()系统调用。

{{< figure src="/ox-hugo/0*RzyU-Af3Z8qBIeFw.png" width="701px" >}}

这个钩子函数会打印出下面的信息:

{{< figure src="/ox-hugo/0*UXqcFdPl_3KK6tVt.png" width="800px" >}}

编译完成后，我们加载了 rootkit，可以看到 hooking 成功了。

{{< figure src="/ox-hugo/0*ViS5T23E39miIEfb.png" width="800px" >}}


## <span class="section-num">3</span> VFS hooking {#h:889b5097-0190-4799-a9c6-5cb7d50ad082}

我在阅读这篇[报告](https://media.defense.gov/2020/Aug/13/2002476465/-1/-1/0/CSA_DROVORUB_RUSSIAN_GRU_MALWARE_AUG_2020.PDF)时听说了这种技术。

Linux 中的虚拟文件系统（VFS）是一个抽象层，它类似于胶水，使诸如 open()、read() 和
write() 等系统调用无论文件系统或底层物理介质的差异都能正常工作。通过 VFS，一个内核可以挂
载多种不同类型的文件系统。

VFS 包含 4 个主要对象：

1.  超级块对象（存储在磁盘上的特殊扇区）
2.  inode 对象
3.  dentry 对象
4.  文件对象

下面的图片展示了 Linux 中 VFS 实现的高层概述：

<a id="figure--fig:screenshot@2023-09-21-16:04:03"></a>

{{< figure src="/ox-hugo/screenshot@2023-09-21_16:04:03.png" >}}

1.  每当一个进程在系统中启动时，它有各种打开文件的文件对象。
    每个文件对象结构都包含一个称为文件路径（类型为 struct path）的字段，其中包含 dentry。

<!--listend-->

1.  可以将 dentry 简单地理解为像/var/www/html 这样的路径。
    在路径中，var、www、html 和/都是 dentry 对象。这些对象是由 VFS 动态创建的。
    dentry 对象本身是一个结构体，其中包含了关联的 inode 信息（比如以上路径中的 var）。

<!--listend-->

1.  inode 对象代表内核操作文件或目录所需的所有信息。

<!--listend-->

1.  inode 操作结构（inode 对象结构内的字段）包含各种操作，如创建、symlink、mkdir 等。

我们感兴趣的字段是查找。我们将钩住这个字段/调用，下图显示了查找钩子和替换代码片段。我们
从修改 cr0 寄存器开始，以便替换我们的值。我们选择的目标是/proc，所以如果任何进程通过
lookup()在/proc 中查找任何文件，我们的函数都会被调用。

在上面的图像中：- fp 是/proc 的文
件对象。- fp 文件对象结构包含 f_path，该结构是一个 dentry 对象。- dentry 结构包含指向 fp
对象 inode 结构的 d_inode。- inode 结构内部的 i_op 是实际的操作结构，其中包含操作如创建、
symlink、mkdir、查找。内核使用这个结构来操作文件和目录。


## <span class="section-num">4</span> The ftrace helper method {#h:ccc260a9-1238-4340-9ad4-0bd94a9a1d1e}

这是一种相当新的技术，适用于最新的内核（Linux 内核 &gt;=5.7）。我们知道，在较新的内核中不再
导出 kallsyms_lookup_name。使用 kallsyms_lookup_name，我们在早期 rootkit 中定位了
sys_call_table()。

ftrace 帮助库使用 kprobe 通过 kallsyms_lookup_name 来解析符号地址。这样，我们就可以利用
ftrace 库来钩住 syscall。该技术在[此处](https://xcellerator.github.io/posts/linux_rootkits_11/)有更详细的解释。我们将只查看来自 ftrace 库的某些结构的
代码片段和工作原理。

我们可以看到下面的示例，使用 kprobe 解决 kallsyms_lookup_name()的使用。

在上面的图像中，我们要挂钩（我们的情况中是 mkdir）的名称是 syscall，hook_mkdir 是我们的
挂钩函数，而 orig_mkdir 是我们要保存 mkdir 的位置（供以后使用）。

使用 kallsysms_lookup_name 来解析 mkdir()地址。

解析完成 mkdir()之后，其地址被保存在 ftrace_hook 结构体的 address 字段中。ftrace_hook 结必要的修改。我们将在以后的博客中介绍监视功能。
构体中另一个重要的字段是 ops 字段。在 ftrace_hook 结构体中，ops 结构体包含.func 字段，可
以在我们的目标 syscall（sys_mkdir）被调用时将其分配给回调函数。因此，我们分配.func 与
fh_ftrace_thunk（我们的回调函数）如下所示：

ftrace 是 Linux 中非常有用的工具，我们不仅可以钩取系统调用，还可以监视并阻止对我们文件进
行某些不必要的修改。我们在以后的博客中将介绍其监视功能。

