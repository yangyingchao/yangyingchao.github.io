# Barriers in PostgresSQL


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Barriers: 协调进程的同步屏障](#h:88e9d8e9-1ea1-450e-bd7c-cfc191923b9c)
    - <span class="section-num">1.1</span> [静态参与](#h:2d66dbfb-2295-40bf-bab2-22f8c77a3414)
- <span class="section-num">2</span> [TODO: Barriers API](#h:d5dd2d64-5c1c-470a-bc3b-d3a9acbd7356)
- <span class="section-num">3</span> [TODO: how it is used in hash jon&#x2026;](#h:4733ab46-ff4d-47aa-93eb-44dcbfba482a)

</div>
<!--endtoc-->

简单总结一下 PG 中进程同步用到的屏障：
<https://github.com/postgres/postgres/blob/master/src/backend/storage/ipc/barrier.c>


## <span class="section-num">1</span> Barriers: 协调进程的同步屏障 {#h:88e9d8e9-1ea1-450e-bd7c-cfc191923b9c}

来自维基百科[1]：

> “在并行计算中，屏障是一种同步方法。在源代码中，对于一组线程或进程而言，屏障意味着任何一个线程或进程必须在此处停止，并且在所有其他线程或进程到达该屏障之前无法继续进行。”

协调进程同步的屏障可以归纳为以下几点：

1.  并发性：多个进程同时执行可能导致数据竞争和冲突，因此需要采取措施来确保数据的一致性和正确性。
2.  同步需求不明确：不同的进程可能对同步的时间和顺序有不同的要求，导致难以找到一种通用的同步机制来满足所有进程的需求。
3.  通信开销：在进程间进行同步需要进行通信，而通信本身会带来开销，包括延迟和带宽消耗。
4.  死锁：在复杂的同步机制中，可能存在死锁的风险，即进程互相等待对方释放资源，从而导致系统无法继续执行。
5.  跨平台兼容性：在跨平台开发中，不同操作系统或编程语言的同步机制可能不兼容，限制了进程间的同步和协作。

该屏障的实现允许静态参与和动态参与两种方式：

-   静态参与
    -   参与者集合在一开始就确定（事先已知）
    -   阶段号被内部使用，但是并不一定需要客户端代码访问它，
    -   只有当声明的参与者数量达到屏障时，阶段才会前进，
    -   客户端代码应始终能够确定计算的当前阶段。

-   动态参与
    -   其中进程可以随时加入或离开
    -   可以使用一个阶段号跟踪并行算法的进展情况，
    -   当新参与者加入时，可能需要与多阶段算法的当前阶段同步

假设一个并行算法，它涉及到分开的计算阶段 A、B 和 C，每个阶段的输出在下一个阶段开始之前都是必需的。


### <span class="section-num">1.1</span> 静态参与 {#h:2d66dbfb-2295-40bf-bab2-22f8c77a3414}

在初始化为 4 个参与者的静态屏障的情况下，每个参与者都在阶段 A 上工作，然后调用
BarrierArriveAndWait 进行等待，直到所有 4 个参与者到达该点。当 BarrierArriveAndWait 返回控制权时，每个参与者都可以在 B 上工作，以此类推。因为屏障知道要期望多少个参与者，所以计算的阶段不需要标签或数字，因为每个进程的程序计数器都暗示了当前阶段。即使某些进程开始运行阶段 A 较慢，其他参与者也在等待他们，并将耐心等待在屏障处。

代码可以写成如下形式：

```c
perform_a();
BarrierArriveAndWait(&barrier, ...);
perform_b();
BarrierArriveAndWait(&barrier, ...);
perform_c();
BarrierArriveAndWait(&barrier, ...);
```

If the number of participants is not known up front, then a dynamic barrier
is needed and the number should be set to zero at initialization.  New
complications arise because the number necessarily changes over time as
participants attach and detach, and therefore phases B, C or even the end
of processing may be reached before any given participant has started
running and attached.  Therefore the client code must perform an initial
test of the phase number after attaching, because it needs to find out
which phase of the algorithm has been reached by any participants that are
already attached in order to synchronize with that work.  Once the program
counter or some other representation of current progress is synchronized
with the barrier's phase, normal control flow can be used just as in the
static case.  Our example could be written using a switch statement with
cases that fall-through, as follows:

如果事先不知道参与者的数量，那么就需要使用动态屏障，并在初始化时将数量设为零。由于参与者的附加和解除附加使数量随时间而变化，因此在某个给定的参与者开始运行和附加之前，可能会到达
B、C 阶段甚至处理结束。因此，客户端代码在附加后必须对阶段号进行初始测试，因为它需要找出已经附加的任何参与者所达到的算法阶段，以与该工作进行同步。一旦程序计数器或表示当前进度的其他表示与屏障的阶段进行了同步，就可以像静态情况下一样使用正常的控制流。我们的示例可以使用带有贯穿的 switch 语句来编写，如下所示：

```c
phase = BarrierAttach(&barrier);
switch (phase)
{
case PHASE_A:
    perform_a();
    BarrierArriveAndWait(&barrier, ...);
case PHASE_B:
    perform_b();
    BarrierArriveAndWait(&barrier, ...);
case PHASE_C:
    perform_c();
    BarrierArriveAndWait(&barrier, ...);
}
BarrierDetach(&barrier);
```

Static barriers behave similarly to POSIX's pthread_barrier_t.  Dynamic
barriers behave similarly to Java's java.util.concurrent.Phaser.

静态屏障的行为类似于 POSIX 的 `pthread_barrier` \_t。动态屏障的行为类似于 Java 的 `java.util.concurrent.Phaser` 。


## <span class="section-num">2</span> TODO: Barriers API {#h:d5dd2d64-5c1c-470a-bc3b-d3a9acbd7356}


## <span class="section-num">3</span> TODO: how it is used in hash jon&#x2026; {#h:4733ab46-ff4d-47aa-93eb-44dcbfba482a}

