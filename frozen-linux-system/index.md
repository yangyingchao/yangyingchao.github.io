# Ways to Deal With a Frozen Linux System


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [When everything else fails, talk with the Kernel using SysReq](#h:660feb15-1dc5-4890-a586-8f6445c1a928)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://linuxhandbook.com/frozen-linux-system/



## <span class="section-num">1</span> When everything else fails, talk with the Kernel using SysReq {#h:660feb15-1dc5-4890-a586-8f6445c1a928}

有时，甚至切换到 TTY 也无法正常工作。整个系统都死了。但是不要放弃，很可能是一种假象。Linux 在这种情况下提供了一种更多的功能。

如果在内核中编译并在文件中启用，Linux 可以为您提供直接向内核发送预设请求的方式。这在整个用户堆栈死亡的情况下非常有用。它被称为 Magic SysRq 键。在大多数发行版中默认情况下编译并启用，只有 Kernel Panic 才能阻止其工作。

要确认内核编译时已设置该选项，可以尝试在 /boot 中查找当前内核的配置文件。查找 CONFIG_MAGIC_SYSRQ 选项。如先前所述，它在大多数发行版中已编译并启用。要确认它已启用，请运行以下操作：

sysctl kernel.sysrq

在较旧的系统中，有两个选项：0和 1。它要么是关闭的，要么是打开的。在新的系统中，可以更精细地控制哪些命令已启用。

任何大于 1 的值都意味着某些命令已启用。更多详细信息可以在此处找到。

要临时设置不同的值，请运行以下命令：

`sudo sysctl -w kernel.sysrq=value`

要永久更改此值，请编辑 `/etc/sysctl.conf` 。搜索 kernel.sysrq，并设置所需的值。

测试这个功能最简单的方法就是去试。我也建议在系统稳定时学习一组按键操作并进行测试。当你执行 SysRq 命令时，内核将尝试响应。此响应只能在虚拟控制台 1 到 6 中看到。如果焦点无法从 X 移开，您将看不到响应。这并不意味着它不起作用。

可以发送许多命令。完整列表可以在这里看到。我建议记住以下内容：

| 键               | 描述                          |
|-----------------|-----------------------------|
| Alt + SysRq +  r | 退出原始模式，将控制权从 X 中移除 |
| Alt + SysRq +  e | 向所有进程发送 SigTerm 指令，让它们有机会安全退出 |
| Alt + SysRq +  i | 向所有进程发送 SigKill 指令   |
| Alt + SysRq +  k | 关闭当前虚拟控制台中的所有进程 |
| Alt + SysRq +  s | 同步所有已装载的文件系统，将所有数据刷新到磁盘上 |
| Alt + SysRq +  u | 将所有文件系统重新挂载为只读  |
| Alt + SysRq +  b | 立即重新启动系统，不同步或卸载 |
| Alt + SysRq +  o | 关闭系统                      |

这个简短的命令列表为您在处理不稳定的系统时提供了一个强大的新工具。在发送请求时，如果无法看到内核的响应，请在命令之间间隔几秒钟。当请求 HDD 操作时，请注意硬盘灯。

有两个值得记住的序列。

`Alt + SysRq + r e k`

这个序列可以用来重新启动没有响应的 X 服务器。

-   首先， 使用 `Alt + SysRq + r` 将键盘从原始模式退出。
-   然后， 使用 `Alt + SysRq + e` 请求向所有进程发送 SIGTERM 信号。
-   如果 X 服务器可以通过此请求重新启动，那么您就已经成功挽救了系统。如果 X 服务器仍然没有响应，则请使用第三个请求， `Alt + SysRq + k`  向当前虚拟控制台中的所有进程发送 SIGKILL 信号。

如果你的系统在上述操作后仍无响应，你应该继续进行以下操作序列。

`Alt + SysRq + r e i s u b`

这个命令将启动紧急重启。

– r ：退出 RAW 模式– e ：向所有进程发送 SIGTERM 信号– i ：向所有进程发送 SIGKILL 信号– s ：同步所有已挂载的文件系统– u ：应急重新挂载为只读– b ：立即重启系统

