# cgroup memo


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Threads (Tasks (threads) versus processes)](#h:6aa67997-fe0b-4ae0-a304-d51dad6f82fd)
    - <span class="section-num">1.1</span> [cgroup v2 线程限制](#h:5fc61e35-bc73-442b-a063-74b0de3cf159)

</div>
<!--endtoc-->



## <span class="section-num">1</span> Threads (Tasks (threads) versus processes) {#h:6aa67997-fe0b-4ae0-a304-d51dad6f82fd}

cgroup v1 可以单独操作进程中的线程, `man cgroups` :

> In cgroups v1, a distinction is drawn between processes and tasks.  In this view, a process can con‐ sist of
> multiple tasks (more commonly called threads, from a user-space perspective, and called such in the
> remainder of this man page).  In cgroups v1, it is possible to independently manipulate the cgroup
> memberships of the threads in a process.

v2 中起初移除了线程支持，然后在后期又将其中的功能 “有限的” 加了回来：

> The cgroups v1 ability to split threads across different cgroups caused problems in some cases.  For example, it
> made no sense for the memory controller, since all of the threads of a process share a single address space.
> Because of these problems, the ability to independently manipulate the cgroup memberships of the threads in a
> process was removed in the initial cgroups v2 implementation, and subsequently restored in a more limited form
> (see the discussion of "thread mode" below).


### <span class="section-num">1.1</span> cgroup v2 线程限制 {#h:5fc61e35-bc73-442b-a063-74b0de3cf159}

Linux 4.14 为 cgroup v2 增加了线程模式，该模式可以：

> •  The  creation of threaded subtrees in which the threads of a process may be spread across cgroups inside
>    the tree.  (A threaded subtree may contain multiple multithreaded processes.)
>
> •  The concept of threaded controllers, which can distribute resources across the  cgroups  in  a  threaded
>    subtree.
>
> •  A  relaxation of the "no internal processes rule", so that, within a threaded subtree, a cgroup can both
>    contain member threads and exercise resource control over child cgroups.

