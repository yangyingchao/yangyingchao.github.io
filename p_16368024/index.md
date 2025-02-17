# Linux内核机制—spin_lock (转载)


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [`pin_lock` 概述](#h:ac3c4fa2-488d-4149-b554-d2218edc64e9)
- <span class="section-num">2</span> [相关数据结构](#h:af803ca3-6514-4ef5-a060-23dc07163a51)
    - <span class="section-num">2.1</span> [struct spinlock](#h:0bc412b4-41a6-47f9-a084-5b9c4dba03c7)
    - <span class="section-num">2.2</span> [struct qnode](#h:ff217c37-8fee-4d66-899c-1b61079fdcc9)
- <span class="section-num">3</span> [相关函数](#h:de96322f-228c-4b02-816e-2c21f180b8d8)
    - <span class="section-num">3.1</span> [初始化函数](#h:5d759b92-82ff-42b4-9d57-33c6bcbbd80b)
    - <span class="section-num">3.2</span> [上锁函数](#h:3c9c36a5-9705-4007-a997-cd9fa364e497)
    - <span class="section-num">3.3</span> [解锁函数](#h:6ed0c420-d4d7-4a49-8429-d63a29040446)
    - <span class="section-num">3.4</span> [尝试获取锁函数](#h:0328de7e-60d8-4fed-84ee-9f87ea212509)
    - <span class="section-num">3.5</span> [判断上锁状态](#h:7d4fd4ec-c540-4189-b61e-89be724f406f)
    - <span class="section-num">3.6</span> [还可以直接使用 `raw_spinlock_t` 和与其配套的一组 `raw_spin_xxx()`](#h:e5859ac4-e6ed-4d9d-aad8-55bff1770ccd)
- <span class="section-num">4</span> [锁代码分析](#h:99559801-d2f0-4850-8000-f4326f72a359)
    - <span class="section-num">4.1</span> [`spin_lock()`](#h:21c8b10c-e5e1-4d2e-9a4b-dd13c57332b5)
    - <span class="section-num">4.2</span> [spin_lock 持锁慢速路径 queued_spin_lock_slowpath():](#h:2dcaa6de-51ee-4f66-9dcc-7380cc1cd15d)
- <span class="section-num">5</span> [解锁代码分析](#h:1f8557df-11a7-4ad4-bcf7-d3a4d569749d)
    - <span class="section-num">5.1</span> [`spin_unlock()` 函数](#h:8ad2683c-f285-4321-8995-dc22478091a8)
- <span class="section-num">6</span> [总结](#h:07457cda-612a-4a0e-bd61-f9ce0c077db3)
- <span class="section-num">7</span> [补充](#h:d77f1671-6696-4f58-9f58-4011809165f8)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.cnblogs.com/hellokitty2/p/16368024.html



## <span class="section-num">1</span> `pin_lock` 概述 {#h:ac3c4fa2-488d-4149-b554-d2218edc64e9}

1.  spin lock 是一种不可休眠锁，可用于原子上下文。当获取不到锁的时候会 spin 等待，此时是 running 状态。

2.  spin lock 的发展到现在经历了四个阶段

(1) CAS

锁只有一个原子变量，执行单个 compare-and-swap 指令来获取锁。问题是没有公平可言，无法让等待最长的那个任务优先拿到锁，为了
解决这个问题引入了 ticket spinlock。

(2) ticket spinlock

类似排队叫号，只有任务手中事先领取的号和被叫到的号相等时才能持锁进入临界区。这解决了不公平的问题。但是出现叫号时，所有等
待的任务所在的 cpu 都要读取内存，刷新对应的 cache line，而只有获取锁的那个任务所在的 cpu 对 cache line 的刷新才是有意义的，锁争
抢的越激烈，无谓的开销也就越大。

(3) MCS Lock

在 ticket spinlock 的基础上做一定的修改，让多个 CPU 不再等待同一个 spinlock 变量，而是基于各自的 per-CPU 的变量进行等待，那么每个
CPU 平时只需要查询自己对应的这个变量所在的本地 cache line，仅在这个变量发生变化的时候，才需要读取内存和刷新这条 cache line，
这样就可以解决上述问题。要实现类似这样的 spinlock 的分身，其中的一种方法就是使用 MCS lock。试图获取一个 spinlock 的每个 CPU，都
有一份自己的 MCS lock。

(4) qspinlock

相比起 Linux 中只占 4 个字节的 ticket spinlock，MCS lock 多了一个指针，要多占 4（或者 8）个字节，消耗的存储空间是原来的 2-3 倍。
qspinlock 的首要目标就是改进原生的 MCS lock 结构体，尽量将原生 MCS lock 要包含的内容塞进 4 字节的空间里。

如果只有 1 个或 2 个 CPU 试图获取锁，那么只需要一个 4 字节的 qspinlock 就可以了，其所占内存的大小和 ticket spinlock 一样。当有 3 个以上
的 CPU 试图获取锁，需要一个 qspinlock 加上(N-2)个 MCS node。

qspinlock 中加入”pending”位域，如果是两个 CPU 试图获取锁，那么第二个 CPU 只需要简单地设置”pending”为 1，而不用另起炉灶创建一个
MCS node。

试图加锁的 CPU 数目超过 3 个是小概率事件，但一旦发生，使用 ticket spinlock 机制就会造成多个 CPU 的 cache line 无谓刷新的问题，而
qspinlock 可以利用 MCS node 队列来解决这个问题。

可见，使用 qspinlock 机制来实现 spinlock，具有很好的可扩展性，也就是无论当前锁的争抢程度如何，性能都可以得到保证。


## <span class="section-num">2</span> 相关数据结构 {#h:af803ca3-6514-4ef5-a060-23dc07163a51}


### <span class="section-num">2.1</span> struct spinlock {#h:0bc412b4-41a6-47f9-a084-5b9c4dba03c7}

除去使能 deug 才会有的相关成员，结构如下：

```c
typedef struct spinlock { //include/linux/spinlock_types.h
    struct raw_spinlock rlock;
} spinlock_t;

typedef struct raw_spinlock { //include/linux/spinlock_types.h
    arch_spinlock_t raw_lock;
} raw_spinlock_t;

typedef struct qspinlock { //include/asm-generic/qspinlock_types.h
    union {
        /*
         * "val"作为一个 32 位的变量，包含了三个部分："locked byte",
         * "pending"和"tail"，"tail"又细分为"tail index"和"tail cpu"
         *
         * bit0-7: locked byte
         * bit8: pending
         * bit9-15: not used(实际上算在 pending 上了，只不过只使用了 bit8)
         * bit16-17: tail index
         * bit18-31: tail cpu(+1)
         *
         * 为了方便演示和说明，下面将"tail"表示为 x, "pending"表示为 y,
         * "locked byte"表示为 z, 组成一个三元组(x,y,z)
         */
        atomic_t val;

        struct {
            u16    tail;
            u16    locked_pending;
        };
        struct {
            u8    reserved[2];
            u8    pending;
            u8    locked;
        };
    };
} arch_spinlock_t;

```

其实 `arch_spinlock_t` 结构中只是一个 val 变量，分成多个位段使用，但是为了使用方便，不用进行位于/位或的操作，将其定义成了联
合体，每个变量表示 val 中的一个位段。

spinlock 结构中只有一个类型为 `raw_spinlock` 的 rlock 成员。而 raw_spinlock 结构中只有一个类型为 qspinlock 的 raw_lock 成员。
qspinlock 结构中维护的是一个联合体。

val：为 0 表示没有人持锁，然后对其赋值为 \_Q_LOCKED_VAL(1)表示持有了该锁。


### <span class="section-num">2.2</span> struct qnode {#h:ff217c37-8fee-4d66-899c-1b61079fdcc9}

```c
struct qnode { //kernel/locking/qspinlock.c
    struct mcs_spinlock mcs;
#ifdef CONFIG_PARAVIRT_SPINLOCKS //默认不使能，没有下面成员
    long reserved[2];
#endif
};
struct mcs_spinlock { //kernel/locking/mcs_spinlock.h
    struct mcs_spinlock *next; /* 组成单链表 */
    int locked; /* 1 if lock acquired */
    int count;  /* nesting count, see qspinlock.c */
};
static DEFINE_PER_CPU_ALIGNED(struct qnode, qnodes[MAX_NODES]); //MAX_NODES=4
```

MAX_NODES=4, 因为 CPU 只能处于 4 种上下文(thread、soft irq、irq、nmi)，因此一个 CPU 最多只能同时持有 4 个 spin_lock，嵌套 4 层。本次
使用哪个 qnode 结构是由 lock-&gt;val 的 tail_cpu 和 tail idx 决定。


## <span class="section-num">3</span> 相关函数 {#h:de96322f-228c-4b02-816e-2c21f180b8d8}


### <span class="section-num">3.1</span> 初始化函数 {#h:5d759b92-82ff-42b4-9d57-33c6bcbbd80b}

```c
//定义并初始化为 unlock 状态的，名为 x 的 spin_lock 变量。
DEFINE_SPINLOCK(x);
//初始化为非持锁状态
spin_lock_init(_lock) //include/linux/spinlock.h 宏
```


### <span class="section-num">3.2</span> 上锁函数 {#h:3c9c36a5-9705-4007-a997-cd9fa364e497}

```c
//里面直接调用的是 raw_spin_lock(&lock->rlock)
static __always_inline void spin_lock(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_lock_bh(&lock->rlock)
static __always_inline void spin_lock_bh(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_lock_irq(&lock->rlock)
static __always_inline void spin_lock_irq(spinlock_t *lock) //include/linux/spinlock.h
//宏，等效于 raw_spin_lock_irqsave(spinlock_check(lock), flags)
spin_lock_irqsave(lock, flags) //include/linux/spinlock.h 宏
```


### <span class="section-num">3.3</span> 解锁函数 {#h:6ed0c420-d4d7-4a49-8429-d63a29040446}

```c
//里面直接调用的是 raw_spin_unlock(&lock->rlock)
static __always_inline void spin_unlock(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_unlock_bh(&lock->rlock)
static __always_inline void spin_unlock_bh(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_unlock_irq(&lock->rlock)
static __always_inline void spin_unlock_irq(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_unlock_irqrestore(&lock->rlock, flags)
static __always_inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags) //include/linux/spinlock.h
```


### <span class="section-num">3.4</span> 尝试获取锁函数 {#h:0328de7e-60d8-4fed-84ee-9f87ea212509}

```c
//里面直接调用的是 raw_spin_trylock(&lock->rlock)
static __always_inline int spin_trylock(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_trylock_bh(&lock->rlock)
static __always_inline int spin_trylock_bh(spinlock_t *lock) //include/linux/spinlock.h
//里面直接调用的是 raw_spin_trylock_irq(&lock->rlock)
static __always_inline int spin_trylock_irq(spinlock_t *lock) //include/linux/spinlock.h
//宏，里面直接调用的是 raw_spin_trylock_irqsave(spinlock_check(lock), flags);
spin_trylock_irqsave(lock, flags) //include/linux/spinlock.h 宏
```


### <span class="section-num">3.5</span> 判断上锁状态 {#h:7d4fd4ec-c540-4189-b61e-89be724f406f}

```c
//直接调用的是 raw_spin_is_locked(&lock->rlock), 直接返回的是 lock->val，非 0 表示 locked 的状态，0表示非 locked 状态
static __always_inline int spin_is_locked(spinlock_t *lock) //include/linux/spinlock.h
//直接调用的是 raw_spin_is_contended(&lock->rlock), 判断锁是否处于被竞争状态，也即是否有任务在等待获取锁，为真表示锁处于竞争状态
static __always_inline int spin_is_contended(spinlock_t *lock) //include/linux/spinlock.h
//判断上锁状态，等效于调用 BUG_ON(!raw_spin_is_locked(&lock->rlock))
assert_spin_locked(lock) //include/linux/spinlock.h 宏
```

raw_XXX() 函数是直接对 lock-&gt;rlock 操作，一般使用不会直接使用。

可以看到，spin lock 的使用是需要区分是何种上下文的。spin_lock() 在持锁前先关抢占，通过在
current-&gt;thread_info.preempt.count 的"preempt bit"位段上加 1 来实现。spin_lock_bh() 持锁前先关底半部，通过在"software
interrupt count" bit 位段上加 2 和在"preempt bit"位段上加 1 实现的。spin_lock_irq() 持锁之前先关本地中断，再关抢占(这里关中断
是没有直接操作 preempt.count 的位段)。spin_lock_irqsave() 持锁之前先关中断再关抢占，同时保存中断标志位。


### <span class="section-num">3.6</span> 还可以直接使用 `raw_spinlock_t` 和与其配套的一组 `raw_spin_xxx()` {#h:e5859ac4-e6ed-4d9d-aad8-55bff1770ccd}

```c
#define DEFINE_RAW_SPINLOCK(x) //include/linux/spinlock_types.h
raw_spin_lock_irqsave(lock, flags) //include/linux/spinlock.h
raw_spin_unlock_irqrestore(lock, flags) //include/linux/spinlock.h
...
```


## <span class="section-num">4</span> 锁代码分析 {#h:99559801-d2f0-4850-8000-f4326f72a359}


### <span class="section-num">4.1</span> `spin_lock()` {#h:21c8b10c-e5e1-4d2e-9a4b-dd13c57332b5}

```c
static __always_inline void spin_lock(spinlock_t *lock) //include/linux/spinlock.h
{
    raw_spin_lock(&lock->rlock);
}
#define raw_spin_lock(lock)    _raw_spin_lock(lock) //include/linux/spinlock.h
void __lockfunc _raw_spin_lock(raw_spinlock_t *lock) //kernel/locking/spinlock.c
{
    __raw_spin_lock(lock);
}
static inline void __raw_spin_lock(raw_spinlock_t *lock) //include/linux/spinlock_api_smp.h
{
    /* 关抢占 */
    preempt_disable();
    /* 默认不使能 CONFIG_LOCKDEP，是个空函数 */
    spin_acquire(&lock->dep_map, 0, 0, _RET_IP_);
    /* 默认不使能 CONFIG_LOCK_STAT,      等效于 do_raw_spin_lock(lock) */
    LOCK_CONTENDED(lock, do_raw_spin_trylock, do_raw_spin_lock);
}
static inline void do_raw_spin_lock(raw_spinlock_t *lock) __acquires(lock) //include/linux/spinlock.h
{
    /* 静态代码检查相关，忽略之 */
    __acquire(lock);
    arch_spin_lock(&lock->raw_lock);
    mmiowb_spin_lock();
}
#define arch_spin_lock(l)    queued_spin_lock(l) //include/asm-generic/qspinlock.h
/**
 * queued_spin_lock - acquire a queued spinlock
 * @lock: Pointer to queued spinlock structure, that is arch_spinlock_t.
 */
static __always_inline void queued_spin_lock(struct qspinlock *lock) //include/asm-generic/qspinlock.h
{
    u32 val = 0;
    /*
     * 参数(*v, *old, new):
     *    if (*v == *old) {*v = new; return true;}
     *    if (*v != *old) {*old = *v; return false;}
     *
     * 如果 lock->val == 0, lock->val = _Q_LOCKED_VAL; return true;
     * 也就是说 lock->val 为 0 表示没有人持锁，此时赋值为 1 表示持有了该锁
     */
    if (likely(atomic_try_cmpxchg_acquire(&lock->val, &val, _Q_LOCKED_VAL)))
        return;
    /* 若是上面持锁失败，进入持锁慢速路径，上面进行了赋值，传参 val=lock->val */
    queued_spin_lock_slowpath(lock, val);
}
```


### <span class="section-num">4.2</span> spin_lock 持锁慢速路径 queued_spin_lock_slowpath(): {#h:2dcaa6de-51ee-4f66-9dcc-7380cc1cd15d}

```c
/**
 * queued_spin_lock_slowpath - acquire the queued spinlock
 * @lock: Pointer to queued spinlock structure
 * @val: Current value of the queued spinlock 32-bit word
 *
 * val: (queue tail(bit16-31 x), pending bit(bit8 y), lock value(bit0-7 z))
 *
 *              fast     :    slow                                  :    unlock
 *                       :                                          :
 * uncontended  (0,0,0) -:--> (0,0,1) ------------------------------:--> (*,*,0)
 *                       :       | ^--------.------.             /  :
 *                       :       v           \      \            |  :
 * pending               :    (0,1,1) +--> (0,1,0)   \           |  :
 *                       :       | ^--'              |           |  :
 *                       :       v                   |           |  :
 * uncontended           :    (n,x,y) +--> (n,0,0) --'           |  :
 *   queue               :       | ^--'                          |  :
 *                       :       v                               |  :
 * contended             :    (*,x,y) +--> (*,0,0) ---> (*,0,1) -'  :
 *   queue               :         ^--'                             :
 */
/*
queued_spin_lock 传参(lock, lock->val)
*/
void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val)
{
    struct mcs_spinlock *prev, *next, *node;
    u32 old, tail;
    int idx;
    /* 32 >= 1<<14 恒不成立 */
    BUILD_BUG_ON(CONFIG_NR_CPUS >= (1U << _Q_TAIL_CPU_BITS));
    /* 默认为 false，恒不执行 */
    if (pv_enabled())
        goto pv_queue;
    /* 默认为 false，恒不执行 */
    if (virt_spin_lock(lock))
        return;
    /*
     * Wait for in-progress pending->locked hand-overs with a bounded
     * number of spins so that we guarantee forward progress. 0,1,0 -> 0,0,1
     */
    /*
     * 进入慢速路径时有任务在等待 spin lock，执行这段代码时间内它可能已经获取到 spin lock 了，
     * 这里更新下从 lock->val 读取到的值.
     */
    if (val == _Q_PENDING_VAL) { //1<<8
        int cnt = _Q_PENDING_LOOPS; //1
        /*
         * 函数作用：死循环读取 lock->val 的值，直到 arg2 为真才返回读取的结果。
         * VAL 为读取的 lock->val 的值.
         * 含义为死循环读取 lock->val 的值，直到 lock->val != _Q_PENDING_VAL(即 1<<8),
         * 或尝试读取的次数达到 cnt 次，这里 cnt 初始化为 1，其实就只是读取一次。
         */
        val = atomic_cond_read_relaxed(&lock->val, (VAL != _Q_PENDING_VAL) || !cnt--);
    }
    /*
     * If we observe any contention; queue.
     *
     * 除了 lock value(即 z)之外，还有其它位段不为 0，说明已经有任务处于 pending 等锁的状态了，
     * 那么就直接 queue，在自己的 mcs 锁上自旋，不要在 lock->val 上自旋了。
     */
    if (val & ~_Q_LOCKED_MASK) // val & ~0xff
        goto queue;
    /*
     * trylock || pending
     * 0,0,* -> 0,1,* -> 0,0,1 pending, trylock
     *
     * 代码走到这里，说明没有任务处于 pending 状态，那么设置锁的 pending 标志，标识本任务
     * 要 pending 了。
     * 函数等效于 ret = lock->val; lock->val |= _Q_PENDING_VAL(1<<8); val = ret;
     */
    val = queued_fetch_set_pending_acquire(lock);
    /*
     * If we observe contention, there is a concurrent locker.
     *
     * Undo and queue; our setting of PENDING might have made the
     * n,0,0 -> 0,0,0 transition fail and it will now be waiting
     * on @next to become !NULL.
     *
     * 在执行这段代码期间，若是有任务进入了 pending 状态(设置了 pending 标识或
     * 在 msc lock 上 spin 了)，那么也是要直接 queue 的，在自己的 msc lock 上 spin，
     * 不要在 lock->val 上 spin 了。
     * 若情况是其它任务在 mcs lock 上自旋了，就清除自己或上的 pending 标志。
     */
    if (unlikely(val & ~_Q_LOCKED_MASK)) { //~0xff
        /* Undo PENDING if we set it. */
        if (!(val & _Q_PENDING_MASK)) //_Q_PENDING_MASK = 0xff<<8
            /* 直接 lock->pending=0, 将 pending 位段清 0，看来 pending 位段只使用了 bit8 */
            clear_pending(lock);
        goto queue;
    }
    /*
     * We're pending, wait for the owner to go away.
     *
     * 0,1,1 -> 0,1,0
     *
     * this wait loop must be a load-acquire such that we match the
     * store-release that clears the locked bit and create lock
     * sequentiality; this is because not all
     * clear_pending_set_locked() implementations imply full
     * barriers.
     *
     * 代码走到这里，说明目前只有一个任务正持有锁(也可能已经释放了)，且没有任务等待。
     * 那么就在 lock->val 上自旋等待，只有锁的 owner 退出临界区。
     * 上面的这个 pending 位段为 1 还是上面本任务设置的。
     */
    if (val & _Q_LOCKED_MASK) //0xff
        atomic_cond_read_acquire(&lock->val, !(VAL & _Q_LOCKED_MASK)); //【1. 在 lock->val 上 spin】
    /*
     * take ownership and clear the pending bit.
     * 0,1,0 -> 0,0,1
     *
     * owner 退出临界区释放锁了，我们就获取锁，清除我们设置的 pending 标志位。
     * 通过将 bit0-15 设置为 1 来实现的。
     */
    clear_pending_set_locked(lock);
    /* 默认不使能 CONFIG_LOCK_EVENT_COUNTS，为空 */
    lockevent_inc(lock_pending);
    /* 这个路径下，我们就是锁的第一继承人，获取到锁后就退出了 */
    return;
    /*
     * End of pending bit optimistic spinning and beginning of MCS queuing.
     *
     * 下面就是非第一位继承人，要在自己的 msc node 节点上自旋了。
     */
queue:
    /* 原生有记录进入执行慢速路径的次数 */
    lockevent_inc(lock_slowpath);
pv_queue:
    /* 获取当前 cpu 的 spin lock 嵌套深度 */
    node = this_cpu_ptr(&qnodes[0].mcs);
    idx = node->count++; //先赋值，再嵌套计数加 1
    //对 tail(即 x)的 tail cpu 和 tail idx 位段进行编码
    tail = encode_tail(smp_processor_id(), idx);
    /*
     * 4 nodes are allocated based on the assumption that there will
     * not be nested NMIs taking spinlocks. That may not be true in
     * some architectures even though the chance of needing more than
     * 4 nodes will still be extremely unlikely. When that happens,
     * we fall back to spinning on the lock directly without using
     * any MCS node. This is not the most elegant solution, but is
     * simple enough.
     * 翻译：基于不会有嵌套 NMI 采用自旋锁的假设分配了 4 个节点。这在某
     * 些架构中可能并非如此，即使需要超过 4 的可能性仍然极小。发生这种情
     * 况时，我们会退回到直接在锁上自旋而不使用任何 MCS 节点。这不是最优
     * 雅的解决方案，但足够简单。
     * 判断了等于，说明最大只允许嵌套 3 层。这里有个统计，若为真，说明出现
     * 了 nmi 中断中持 spin lock 锁的嵌套。
     */
    if (unlikely(idx >= MAX_NODES)) {
        lockevent_inc(lock_no_node);
        /* 退回到在 lock->val 上 spin 的状态，几乎不可能进到上面 if 语句中来 */
        while (!queued_spin_trylock(lock))
            cpu_relax();
        goto release;
    }
    /* 返回当前 cpu 的 qnodes[idx].mcs 节点的地址 */
    node = grab_mcs_node(node, idx);
    /* Keep counts of non-zero index values: */
    lockevent_cond_inc(lock_use_node2 + idx - 1, idx);
    /*
     * Ensure that we increment the head node->count before initialising
     * the actual node. If the compiler is kind enough to reorder these
     * stores, then an IRQ could overwrite our assignments.
     */
    barrier();
    /*不是继承者 locked 为 0, 最尾部节点指向 NULL */
    node->locked = 0;
    node->next = NULL;
    /* 默认没有定义_GEN_PV_LOCK_SLOWPATH，是空函数 */
    pv_init_node(node);
    /*
     * We touched a (possibly) cold cacheline in the per-cpu queue node;
     * attempt the trylock once more in the hope someone let go while we
     * weren't watching.
     * 尝试获取锁，成功返回 1，失败返回 0
     */
    if (queued_spin_trylock(lock))
        goto release;
    /*
     * Ensure that the initialisation of @node is complete before we
     * publish the updated tail via xchg_tail() and potentially link
     * @node into the waitqueue via WRITE_ONCE(prev->next, node) below.
     */
    smp_wmb();
    /*
     * Publish the updated tail. We have already touched the queueing cacheline;
     * don't bother with pending stuff.
     * p,*,* -> n,*,*
     */
    /*
     * 等效于：ret=lock->tail; lock->tail=tail>>_Q_TAIL_OFFSET; old = ret<<_Q_TAIL_OFFSET
     * 也即是将 tail 值赋值给 lock->val 的 bit16-31.
     * 也就是说 lock->tail 恒指向最后一个 pending 锁的任务对应的 msc node 节点上 ###########
     */
    old = xchg_tail(lock, tail);
    next = NULL;
    /*
     * if there was a previous node; link it and wait until reaching the head of the
     * waitqueue.
     * 为真，说明之前已经有 mcs node 等待节点存在了。
     */
    if (old & _Q_TAIL_MASK) { //bit16-bit31, 也即是 tail 的掩码
        /* 获取的是 cpu 的嵌套深度 idx 为下标的节点 */
        prev = decode_tail(old); //return per_cpu_ptr(&qnodes[idx].mcs, cpu)
        /* Link @node into the waitqueue. 构建 msc 链表 */
        WRITE_ONCE(prev->next, node);
        pv_wait_node(node, prev); //空函数
        /* 死循环，spin 等待，直到 node->locked 不为 0 才退出 spin */
        arch_mcs_spin_lock_contended(&node->locked); //【2. 在 mcs node->locked 上 spin】
        /*
         * While waiting for the MCS lock, the next pointer may have
         * been set by another lock waiter. We optimistically load
         * the next pointer & prefetch the cacheline for writing
         * to reduce latency in the upcoming MCS unlock operation.
         * 翻译：在等待 MCS 锁时，下一个指针可能已被另一个锁的 waiter 设置(在另一个 cpu 上，
         * 若被设置 next 就不为 NULL)。我们乐观地加载下一个指针并为写入预取缓存线，以减少
         * 即将到来的 MCS 解锁操作的延迟。
         */
        next = READ_ONCE(node->next);
        if (next)
            prefetchw(next); //cache line 预取
    }
    /*
     * we're at the head of the waitqueue, wait for the owner & pending to
     * go away.
     *
     * *,x,y -> *,0,0
     *
     * this wait loop must use a load-acquire such that we match the
     * store-release that clears the locked bit and create lock
     * sequentiality; this is because the set_locked() function below
     * does not imply a full barrier.
     *
     * The PV pv_wait_head_or_lock function, if active, will acquire
     * the lock and return a non-zero value. So we have to skip the
     * atomic_cond_read_acquire() call. As the next PV queue head hasn't
     * been designated yet, there is no way for the locked value to become
     * _Q_SLOW_VAL. So both the set_locked() and the
     * atomic_cmpxchg_relaxed() calls will be safe.
     *
     * If PV isn't active, 0 will be returned instead.
     *
     */
    /* 没有使能，函数直接返回 0 */
    if ((val = pv_wait_head_or_lock(lock, node)))
        goto locked;
    /* 代码走到这里，当前任务就是锁的第一继承人了 */
    /*
     * 死循环等待，直到         lock->val 的 y(pending bit)和 z(locked byte)都是 0,  并返回 lock->val
     * spin_unlock 时直接将 lock->val 赋值为 0, 判断 pending bit 是为还没进入 msc spin 的尝试
     * 持锁的任务让路的应该。
     */
    val = atomic_cond_read_acquire(&lock->val, !(VAL & _Q_LOCKED_PENDING_MASK)); //【3. 在 lock->val 上 spin】
locked:
    /*
     * claim the lock:
     *
     * n,0,0 -> 0,0,1 : lock, uncontended
     * *,*,0 -> *,*,1 : lock, contended
     *
     * If the queue head is the only one in the queue (lock value == tail)
     * and nobody is pending, clear the tail code and grab the lock.
     * Otherwise, we only need to grab the lock.
     */
    /*
     * In the PV case we might already have _Q_LOCKED_VAL set, because
     * of lock stealing; therefore we must also allow:
     *
     * n,0,1 -> 0,0,1
     *
     * Note: at this point: (val & _Q_PENDING_MASK) == 0, because of the
     *       above wait condition, therefore any concurrent setting of
     *       PENDING will make the uncontended transition fail.
     */
    /*
     * 走到这里，val 的 lock 和 pending 是为 0 的，但是 tail 中可能还保存着其它等待者的信息。
     * lock->tail 指向最后一个等待获取锁的任务的 mcs node 节点，若和自己相等，说明自己
     * 是唯一一个 mcs node 节点。
     */
    if ((val & _Q_TAIL_MASK) == tail) {
        /*
         * if (lock->val == val) {lock->val = _Q_LOCKED_VAL; return true;} 持锁退出
         * if (lock->val != val) {val = lock->val; return false;} //没持锁
         */
        if (atomic_try_cmpxchg_relaxed(&lock->val, &val, _Q_LOCKED_VAL))
            goto release; /* No contention */
    }
    /* 下面就是还有新的任务等待了，有新 msc                 node 串联在链表上了 */
    /*
     * Either somebody is queued behind us or _Q_PENDING_VAL got set
     * which will then detect the remaining tail and queue behind us
     * ensuring we'll see a @next.
     */
    /* 直接赋值 lock->locked = _Q_LOCKED_VAL，即对 bit0-7 locked byte 进行赋值，已经持锁了 */
    set_locked(lock);
    /*
     * contended path; wait for next if not observed yet, release.
     * 等待已经更新 lock->tail 的新的 waiter 挂在 msc 链表上
     */
    if (!next)
        /* 死循环等待，直到 node->next 不为 NULL，通过不会等待很多次 */
        next = smp_cond_load_relaxed(&node->next, (VAL)); //【4. 在 mcs node->next 上 spin】
    /*
     * 直接 next->locked = 1, 将下一个 mcs 节点设为第一继承人，
     * 其就会退出在 node->locked 上的自旋，改为在 lock->val 上的自旋
     */
    arch_mcs_spin_unlock_contended(&next->locked);
    /*没有使能，为空 */
    pv_kick_node(lock, next);
    /* 下面就是获取到 spin_lock 锁后的逻辑了 */
release:
    /* release the node. qnodes[0].mcs.count - 1 应该是和前面的加 1 相对应的 */
    __this_cpu_dec(qnodes[0].mcs.count);
}
EXPORT_SYMBOL(queued_spin_lock_slowpath);
```

a. spin lock 持锁后就一直在临界区了，不会休眠，因此不需要记录锁的 owner。
b. 非第一继承人的 waiter 在自己的 mcs node 的 node-&gt;locked 上的自旋，而第一继承人在 lock-&gt;val 上自旋。
c. 每个 cpu 有一个 mcs node[]数组，数组中有 4 个 mcs node 成员，对应四种嵌套深度(thread、softirq、irq、nmi)，若是没有嵌套的话，
使用的恒定是 node[0].
d. 一个锁对应的 mcs node 节点构成一个单链表，以 NULL 结尾，lock-&gt;val 的 tail 域"指向"最后一个获取锁的 waiter 所对应的 mcs node 节
点。

(1) queued_spin_trylock 函数，尝试获取 spin lock。

```c
/**
 * queued_spin_trylock - try to acquire the queued spinlock
 * @lock : Pointer to queued spinlock structure
 * Return: 1 if lock acquired, 0 if failed
 */
static __always_inline int queued_spin_trylock(struct qspinlock *lock)
{
    u32 val = atomic_read(&lock->val);
    /* 这里 val 不等于 0 就退出了，若没退出走到下面就是本线程可以持锁的状态了 */
    if (unlikely(val))
        return 0;
    /*
     * 参数：(atomic_t *v, int *old, int new)
     * if (*v == *old) {*v = new; return true;}
     * if (*v != *old) {*old = *v; return false;}
     */
    return likely(atomic_try_cmpxchg_acquire(&lock->val, &val, _Q_LOCKED_VAL));
}
```


## <span class="section-num">5</span> 解锁代码分析 {#h:1f8557df-11a7-4ad4-bcf7-d3a4d569749d}


### <span class="section-num">5.1</span> `spin_unlock()` 函数 {#h:8ad2683c-f285-4321-8995-dc22478091a8}

```c
static __always_inline void spin_unlock(spinlock_t *lock) //include/linux/spinlock.h
{
    raw_spin_unlock(&lock->rlock);
}
#define raw_spin_unlock(lock)    _raw_spin_unlock(lock) //include/linux/spinlock.h
void __lockfunc _raw_spin_unlock(raw_spinlock_t *lock) //kernel/locking/spinlock.c
{
    __raw_spin_unlock(lock);
}
EXPORT_SYMBOL(_raw_spin_unlock);
static inline void __raw_spin_unlock(raw_spinlock_t *lock) //include/linux/spinlock_api_smp.h
{
    spin_release(&lock->dep_map, _RET_IP_);
    do_raw_spin_unlock(lock);
    /* unlock 后开抢占 */
    preempt_enable();
}
static inline void do_raw_spin_unlock(raw_spinlock_t *lock) __releases(lock) //include/linux/spinlock.h
{
    /* 没使能 CONFIG_MMIOWB，是个空函数 */
    mmiowb_spin_unlock();
    arch_spin_unlock(&lock->raw_lock);
    __release(lock);
}
#define arch_spin_unlock(l)    queued_spin_unlock(l) //include/asm-generic/qspinlock.h
static __always_inline void queued_spin_unlock(struct qspinlock *lock)
{
    /*
     * unlock() needs release semantics:
     * 直接将 lock->locked 赋值为 0，也即只是将 lock->val 的 bit0-8 设置为 0
     */
    smp_store_release(&lock->locked, 0);
}
```


## <span class="section-num">6</span> 总结 {#h:07457cda-612a-4a0e-bd61-f9ce0c077db3}

1.  spin lock 是不休眠锁，不像其它锁一样，没有 owner 成员保存当前持锁任务。其持锁之前是抢
    占的，使用需要区分是在哪个上下文中使用，来用对应的函数。

2.  spin lock 实现机制中考虑了中断嵌套，由于只有 4 种上下文，为每个 cpu 分配了 4 个 mcs node。

3.  当一个 spin lock 在不同 cpu 上存在竞争时，各个 cpu 的 mcs node 构成一个单链表，
    lock-&gt;tail 字段"指向"最后尝试获取锁的任务所在 cpu 对应的 mcs node 节点。第一顺位继承人
    在 lock-&gt;val 上进行自旋，非第一顺位继承人在自己的 msc node 的 locked 成员上自旋，这样
    就做了释放锁时只有一个 cpu 需要更新其 cache line。


## <span class="section-num">7</span> 补充 {#h:d77f1671-6696-4f58-9f58-4011809165f8}

1.  内核中为了 debug spin_lock 方便，对函数原型进行了 inline 和 unline 的封装

<!--listend-->

```c
//kernel/locking/spinlock.c
#ifndef CONFIG_INLINE_SPIN_LOCK //默认不使能
void __lockfunc _raw_spin_lock(raw_spinlock_t *lock)
{
    __raw_spin_lock(lock);
}
EXPORT_SYMBOL(_raw_spin_lock);
#endif
#ifdef CONFIG_UNINLINE_SPIN_UNLOCK //默认使能
void __lockfunc _raw_spin_unlock(raw_spinlock_t *lock)
{
    __raw_spin_unlock(lock);
}
EXPORT_SYMBOL(_raw_spin_unlock);
#endif
//include/linux/spinlock_api_smp.h
#ifndef CONFIG_UNINLINE_SPIN_UNLOCK
#define _raw_spin_unlock(lock) __raw_spin_unlock(lock)
#endif
#ifdef CONFIG_INLINE_SPIN_LOCK
#define _raw_spin_lock(lock) __raw_spin_lock(lock)
#endif
```

kernel/locking/spinlock.c 中还有 CONFIG_INLINE_READ_LOCK_IRQ、CONFIG_INLINE_SPIN_UNLOCK_BH 等。

1.  linux-5.15.41 中新增: 若使能 CONFIG_PREEMPT_RT(Linux RT patch 默认不使能)，则将 spin_lock 的实现改为基于 rt_mutex 的可

休眠 spin_lock 实现了。

```c
/* include\linux\spinlock_types.h */
#include <linux/rtmutex.h>
typedef struct spinlock {
    struct rt_mutex_base    lock; //5.10 的内核还是 struct raw_spinlock rlock;
} spinlock_t;
```

因此有实现以下两套 lock 和 raw lock 接口：

```c
//raw spinlock:
raw_spinlock_t lock;
raw_spin_lock_init(&lock)
raw_spin_lock(&lock);
raw_spin_unlock(&lock);
//spinlock:
spinlock_t lock;
spin_lock_init(&lock)
spin_lock(&lock);
spin_unlock(&lock);
```

在没有配置 CONFIG_PREEMPT_RT 的情况下两者实现是一样的，但是在使能了 CONFIG_PREEMPT_RT(默认不使能) 的 RT 内核中，spinlock 会
被实现为基于 rtmutex 的可休眠锁，raw spinlock 保持和之前一致的逻辑。

内核工匠中有一篇类似博客：<https://blog.csdn.net/feelabclihu/article/details/125454456>

