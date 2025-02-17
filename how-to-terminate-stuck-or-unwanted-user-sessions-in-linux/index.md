# How to Terminate Stuck or Unwanted User Sessions in Linux


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Linux 中的 TTY 和 PTS 是什么？](#h:630c60b8-137a-4521-84fd-8c01eb6c2e48)
- <span class="section-num">2</span> [如何在 Linux 中终止被卡住/不需要的用户会话](#h:78ab4e18-5d26-4a48-84a3-7d3129debc4e)
    - <span class="section-num">2.1</span> [Terminate User Session by TTY](#h:ef209893-46c7-448a-90a8-8eeffeb11171)
    - <span class="section-num">2.2</span> [按进程 ID 终止用户会话](#h:fb2a4d0e-261f-43d3-b207-b366e4f2d4e2)
    - <span class="section-num">2.3</span> [Conclusion](#h:d7fd8a22-d7ec-4841-9ae7-54f2f7892626)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://linuxiac.com/how-to-terminate-user-session-in-linux/



## <span class="section-num">1</span> Linux 中的 TTY 和 PTS 是什么？ {#h:630c60b8-137a-4521-84fd-8c01eb6c2e48}

在 Linux 中，"PTS"和"TTY"都指不同的终端设备，用于用户与操作系统之间的通信。它们作为输入命令和从系统接收输出的接口。

-   TTY (Teletype, 电传打字机)
    -   "TTY"最初是指电传打字机，一种类似打字机的设备，用于计算机早期的输入和输出。
    -   在现代 Linux 系统中，TTY 代表物理或虚拟控制台，用户可以直接与系统交互。

-   PTS（Pseudo-Terminal Slave, 伪终端从属）

    -   PTS 代表伪终端从属。
    -   它是一种虚拟终端，模拟硬件终端，
    -   但没有直接连接到任何物理设备
    -   系统创建和管理它以支持终端复用、远程登录和各种交互应用程序。

    例如，当您使用 **终端仿真器应用程序** （如 GNOME 的终端或 KDE 的 Konsole）来获取对 Linux 系
    统的 Shell 访问权限时，通常会使用伪终端。

    这些 **仿真器** 作为伪终端的“主”端，而终端内运行的 Shell 或进程则作为“从”端。

    当您打开多个终端窗口或选项卡时，每个窗口或选项卡对应一个单独的 PTS。例如，如果您打开了
    三个终端窗口，则它们可能会被标识为 `/dev/pts/0` 、 `/dev/pts/1` 和 `/dev/pts/2` 。

总之，TTY 和 PTS 都是 Linux 中的终端设备，允许用户与操作系统交互。TTY 代表物理或虚拟控制
台，而 PTS 用于终端仿真器，并提供终端复用和远程登录的附加功能。


## <span class="section-num">2</span> 如何在 Linux 中终止被卡住/不需要的用户会话 {#h:78ab4e18-5d26-4a48-84a3-7d3129debc4e}

你可以用两种方式来完成这个任务，我们将在下面分别介绍这两种方式。


### <span class="section-num">2.1</span> Terminate User Session by TTY {#h:ef209893-46c7-448a-90a8-8eeffeb11171}

我们将使用 w 命令获取有关我们 Linux 系统上已登录用户的信息。它显示有关当前已登录用户及其活动的信息。

当你运行 w 命令时，它为每个用户提供以下细节的摘要：

-   **USER**: 已登录用户的用户名。
-   **TTY**: 与用户会话关联的终端名称或设备（例如，/dev/tty1，pts/0）。
-   **FROM**: 用户登录的远程主机或 IP 地址。如果用户本地登录，则显示 TTY 的名称或“ - ”符号。
-   **LOGIN@**: 用户登录的日期和时间。
-   **IDLE**: 用户会话的非活动时间长度。如果用户主动使用终端，则会显示“旧”。
-   **JCPU**: 与用户会话相关的所有进程使用的总 CPU 时间。
-   **PCPU**: 用户当前进程使用的 CPU 时间。
-   **WHAT**: 由用户或终端关联进程执行的命令。

下面是 w 命令的输出示例：

{{< figure src="/ox-hugo/terminate-user-session-2.jpg" width="800px" >}}

从上面这个例子的输出中可以看到，有三个登录用户——两个本地用户和一个远程用户。

为了终止名为 “linuxiac” 远程登录用户的会话，我们将在 Linux 中使用 `pkill` 命令，并使用选项 `“-KILL”` ，
该选项意味着 Linux 进程必须立即被终止（而不是优雅地关闭）。请使用 `“-t”` 标志指定 TTY 的名称。

`pkill -KILL -t pts/1`

{{< figure src="/ox-hugo/terminate-user-session-7.jpg" width="800px" >}}

就是这样。从使用 w 命令进行重新检查可以看到，用户在我们的系统上的会话已立即终止。


### <span class="section-num">2.2</span> 按进程 ID 终止用户会话 {#h:fb2a4d0e-261f-43d3-b207-b366e4f2d4e2}

我们将展示第二种方法，即使用进程 ID 终止用户会话。为此，再次执行 w 命令以获取已登录用户的列表以及它们关联的 TTY/PTS。
然后，一旦我们确定了 TTY/PTS 会话，使用 `"ps"` 命令和 `"-ft"` 参数来查找它的 PID：

`ps -ft [TTY/PTS]`

最后，使用 `kill` 命令和  `-9`  （无条件终止一个进程）开关，传递进程 ID。例如：

`kill -9 4374`

{{< figure src="/ox-hugo/terminate-user-session-5.jpg" width="800px" >}}

因此，用户会话立即被终止。


### <span class="section-num">2.3</span> Conclusion {#h:d7fd8a22-d7ec-4841-9ae7-54f2f7892626}

在 Linux 系统管理领域中，了解如何终止卡住或不需要的用户会话是至关重要的。在本文中，我们探讨了两种有效的方法来完成此任务：TTY / PTS 或进程 ID。

-   TTY / PTS 方法提供了一个简单的方式来终止用户会话。
-   通过进程 ID 终止会话提供了一种更精确和直接的方法；使用哪种方法完全取决于您自己的决定。

请记住，无论您选择 TTY / PTS 方法还是进程 ID 方法，行使谨慎是至关重要的。确保终止正确的用户会话或进程，以防止意外数据丢失或意外后果。

需要更多详细信息吗？请查阅“pkill”和“kill”命令的手册页面。

<https://linuxiac.com/how-to-terminate-user-session-in-linux/>

