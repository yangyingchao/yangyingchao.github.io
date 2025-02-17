# Linux Process States


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [The Linux Process States](#h:e18e2a33-c94e-4632-9c81-2b7c968471ca)
    - <span class="section-num">1.1</span> [Running or Runnable State (R)](#h:82e9a258-9ef3-4b9f-a6e0-43780e2f7c6b)
    - <span class="section-num">1.2</span> [Sleeping State: Interruptible (S) and Uninterruptible (D)](#h:af5ed80e-75ba-46bf-bd3c-fc469ce02284)
    - <span class="section-num">1.3</span> [Stopped State (T)](#h:928eb1a4-c3a7-45e7-a8d8-031aefe9d89e)
    - <span class="section-num">1.4</span> [Zombie State (Z)](#h:fc7301f8-68e6-4139-9663-7ea505a75521)
- <span class="section-num">2</span> [Checking Process State](#h:0985a112-1b62-4921-9635-078e0ca52dca)
    - <span class="section-num">2.1</span> [Displaying Process State Using ps](#h:d36c6068-e0db-468a-8b7f-349412d139f2)
    - <span class="section-num">2.2</span> [Using the top Command](#h:4aa374a8-e81e-499a-aa03-63c5e0949411)
    - <span class="section-num">2.3</span> [The /proc Pseudo File](#h:5838f424-949c-4b9c-9c93-e77715bfe95c)
- <span class="section-num">3</span> [Summary](#h:3fbb304e-adb8-4e74-94f9-e6a5944f1b26)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.baeldung.com/linux/process-states



## <span class="section-num">1</span> The Linux Process States {#h:e18e2a33-c94e-4632-9c81-2b7c968471ca}

-   Running or Runnable (R)
-   Uninterruptible Sleep (D)
-   Interruptable Sleep (S)
-   Stopped (T)
-   Zombie (Z)

状态机如下：

<a id="figure--fig:screenshot@2023-02-15-17:52:09"></a>

{{< figure src="/ox-hugo/screenshot@2023-02-15_17:52:09.png" width="800px" >}}

For any Linux process, their starting point is the moment they are created. For example, a parent
process can initiate a child process using the fork() system call. Once it starts, the process
goes into the running or runnable state. While the process is running, it could come into a code
path that requires it to wait for particular resources or signals before proceeding. While waiting
for the resources, the process would voluntarily give up the CPU cycles by going into one of the
two sleeping states.

Additionally, we could suspend a running process and put it into the stopped state. Usually, this
is done by sending the SIGSTOP signal to the process. A process in this state will continue to
exist until it is killed or resumed with SIGCONT.  Finally, the process completes its lifecycle
when it’s terminated and placed into a zombie state until its parent process clears it off the
process table.


### <span class="section-num">1.1</span> Running or Runnable State (R) {#h:82e9a258-9ef3-4b9f-a6e0-43780e2f7c6b}

When a new process is started, it’ll be placed into the running or runnable state. In the running
state, the process takes up a CPU core to execute its code and logic. However, the thread
scheduling algorithm might force a running process to give up its execution right. This is to
ensure each process can have a fair share of CPU resources. In this case, the process will be
placed into a run queue, and its state is now a runnable state waiting for its turn to execute.

Although the running and runnable states are distinct, they are collectively grouped into a single
state denoted by the character `R`.


### <span class="section-num">1.2</span> Sleeping State: Interruptible (S) and Uninterruptible (D) {#h:af5ed80e-75ba-46bf-bd3c-fc469ce02284}

During process execution, it might come across a portion of its code where it needs to request
external resources.  Mainly, the request for these resources is IO-based such as to read a file
from disk or make a network request. Since the process couldn’t proceed without the resources, it
would stall and do nothing. In events like these, they should give up their CPU cycles to other
tasks that are ready to run, and hence they go into a sleeping state.

There are two different sleeping states:

-   the uninterruptible sleeping state (D) and
-   the interruptible sleepingstate (S)

The `uninterruptible sleeping state (D)` will only wait for the resources to be available before
it transit into a runnable state, and it doesn’t react to any signals. On the other hand, the
`interruptible sleeping state (S)` will react to signals and the availability of resources.


### <span class="section-num">1.3</span> Stopped State (T) {#h:928eb1a4-c3a7-45e7-a8d8-031aefe9d89e}

From a running or runnable state, we could put a process into the stopped state (T) using the
`SIGSTOP` or `SIGTSTP` signals.  The difference between both signals is that we send the SIGSTOP
is programmatic, such as running `kill -STOP {pid}`.  Additionally, the process cannot ignore this
signal and will go into the stopped state. On the other hand, we send the SIGTSTP signal using the
keyboard CTRL + Z. Unlike SIGSTOP, the process can optionally ignore this signal and continue to
execute upon receiving SIGTSTP.

While in this state, we could bring back the process into a running or runnable state by sending
the SIGCONT signal.


### <span class="section-num">1.4</span> Zombie State (Z) {#h:fc7301f8-68e6-4139-9663-7ea505a75521}

When a process has completed its execution or is terminated, it’ll send the SIGCHLD signal to the
parent process and go into the zombie state. The zombie process, also known as a defunct process,
will remain in this state until the parent process clears it off from the process table. To clear
the terminated child process off the process table, the parent process must read the exit value of
the child process using the wait() or waitpid() system calls.


## <span class="section-num">2</span> Checking Process State {#h:0985a112-1b62-4921-9635-078e0ca52dca}

There are multiple ways to check the state of a process is in Linux. For example, we can use
command-line tools like ps and top to check the state of processes. Alternatively, we can consult
the pseudo status file for a particular PID.


### <span class="section-num">2.1</span> Displaying Process State Using ps {#h:d36c6068-e0db-468a-8b7f-349412d139f2}

To display process state using ps, let’s run the ps command to include a column that tells us the state of the process:

```console
$ ps a
    PID TTY      STAT   TIME COMMAND
   2234 tty2     Ssl+   0:00 /usr/lib/gdm3/gdm-x-session --run-script env GNOME_SHELL_SESSION_MODE=ubuntu /usr/bin/gnome-session --systemd --session=ubuntu
   2237 tty2     Rl+    0:07 /usr/lib/xorg/Xorg vt2 -displayfd 3 -auth /run/user/1000/gdm/Xauthority -background none -noreset -keeptty -verbose 3
   2287 tty2     Sl+    0:00 /usr/libexec/gnome-session-binary --systemd --systemd --session=ubuntu
   2982 pts/0    Ss     0:00 bash
   3467 pts/0    R+     0:00 ps a
```

The first letter of the value under the STAT column indicates the state that the process is
in. For example, the process with PID 2234 is currently in an interruptible sleeping state, as
denoted by the character S. Besides that, we can also observe that process 2237 is currently in
the running or runnable state.

Additionally, we can see that there are additional characters besides each of the state
characters. These characters indicate several attributes of the state of the process. For example,
the lower capital letter s means the process is the session leader. For a comprehensive list of
the meaning of each of the characters, we can find it on the official man page.


### <span class="section-num">2.2</span> Using the top Command {#h:4aa374a8-e81e-499a-aa03-63c5e0949411}

In Linux, the top command-line tool displays the process details in a real-time fashion. It shows
different aspects of the system, such as memory and CPU usage of individual processes. To see the
process state, let’s run top in the terminal:

```console
Tasks: 183 total,   1 running, 182 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.7 us,  1.1 sy,  0.0 ni, 97.1 id,  0.4 wa,  0.0 hi,  0.7 si,  0.0 st
MiB Mem :   3936.4 total,   1925.0 free,    850.6 used,   1160.8 buff/cache
MiB Swap:   2048.0 total,   2048.0 free,      0.0 used.   2834.2 avail Mem
    PID USER  PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
   2237 bob   20   0  252252  81740  49204 S   2.3   2.0   0:09.37 Xorg
   2519 bob   20   0 3428664 375256 125080 S   2.0   9.3   0:19.57 gnome-shell
   2909 bob   20   0  966852  49944  37308 S   1.0   1.2   0:02.28 gnome-terminal-
      1 root  20   0  103500  13312   8620 S   0.7   0.3   0:04.44 systemd
   3588 bob   20   0   20600   3936   3380 R   0.3   0.1   0:00.01 top
      2 root  20   0       0      0      0 S   0.0   0.0   0:00.00 kthreadd
      3 root   0 -20       0      0      0 I   0.0   0.0   0:00.00 rcu_gp
```

At the bottom section of the output of the top command, we can find the S column, which shows the state of each process.
Contrary to the ps command, the top command displays the state of each process without additional process attributes.


### <span class="section-num">2.3</span> The /proc Pseudo File {#h:5838f424-949c-4b9c-9c93-e77715bfe95c}

The /proc pseudo filesystem contains all the information about the processes in our system. Hence,
we could directly read the state of a process through this pseudo filesystem. The downside of this
approach is we’ll first need to know the PID of the process before we can read its state.

To obtain the state of a process, we can extract the value from its pseudo status file under /proc/{pid}/status. For
example, we can get the state of the process with PID 2519 by reading the file /proc/2519/status:

```console
$ cat /proc/2519/status | grep State
State:  S (sleeping)
```


## <span class="section-num">3</span> Summary {#h:3fbb304e-adb8-4e74-94f9-e6a5944f1b26}

In this tutorial, we’ve looked at the lifecycle of a Linux process. Furthermore, we’ve learned how
we can model a Linux process lifecycle as a finite state machine. Then, we’ve looked at the five
different states as a Linux process undergoes the entire lifecycle. Finally, we ended the article
with demonstrations on getting the Linux process state using various tools such as ps, top, and
the /proc pseudo-file.

