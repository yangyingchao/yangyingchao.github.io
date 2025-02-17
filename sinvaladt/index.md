# shared cache invalidation


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [prelog](#prelog)
- <span class="section-num">2</span> [Configurable parameters](#configurable-parameters)
- <span class="section-num">3</span> [SICleanupQueue](#sicleanupqueue)
    - <span class="section-num">3.1</span> [重置后的处理](#重置后的处理)
    - <span class="section-num">3.2</span> [信号的处理](#信号的处理)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://github.com/postgres/postgres/blob/master/src/backend/storage/ipc/sinval.c



## <span class="section-num">1</span> prelog {#prelog}

共享缓存失效消息在一个无限数组中存储，maxMsgNum 是下一个数组下标来存储提交的消息，
minMsgNum 是最小的数组下标，包含尚未被所有后端读取的消息，始终满足 maxMsgNum &gt;= minMsgNum。每个活跃后端都有一个 nextMsgNum 指针，指示下一个需要读取的消息；对每个后端都有
maxMsgNum &gt;= nextMsgNum &gt;= minMsgNum。

消息实际上存储在一个 MAXNUMMESSAGES 条目的循环缓冲区中。我们将 MsgNum 值转换为循环缓冲区索引，通过计算 MsgNum % MAXNUMMESSAGES。只要 maxMsgNum 不比 minMsgNum 多出超过
MAXNUMMESSAGES，缓冲区就足够大。如果缓冲区溢出，我们通过为每个已经太落后的后端设置
"reset" 标志来恢复。

为降低需要重置的概率，我们向任何看起来严重滞后的后端发送“catchup” 中断。当一个后端完成处理 catchup 中断后，它执行 SICleanupQueue，这将在需要时向滞后最远的后端发送信号。这避免了多个后端同时尝试赶上导致的不必要争用。然而，最落后的后端可能被困在无法赶上的状态。最终，它会被重置，因此不会再给其他任何人带来问题。

对共享 sinval 数组访问受两个锁的保护，SInvalReadLock 和 SInvalWriteLock。读者以共享模式获取 SInvalReadLock；这允许它们修改自己的 ProcState，但不能修改或查看其他读者的。写者总是以独占模式获取 SInvalWriteLock，以串行化向列队添加消息。最多一个这样的中断正在飞行，当一个后端完成处理 catchup 中断后，它执行 SICleanupQueue，如果需要，则将信号传递给下一个最落后的后端。

```c
  /* 共享失效结构状态， 存在于每个后端中 */
  typedef struct ProcState
  {
      /* 如果 ProcState 数组条目处于非活动状态，procPid 为零。 */
      pid_t		procPid;		// 后端的 PID，用于信号传递 */
      /* 如果 procPid == 0 或 resetState 为真，则 nextMsgNum 没有意义。 */
      int			nextMsgNum;		/* 下一个要读取的消息号 */
      bool		resetState;		/* 后端需要重置其状态 */
      bool		signaled;		// 后端已发送“追赶”信号 */
      bool		hasMessages;	// 后端有未读消息 */


      /*  */
       * 后端只发送失效信息，不接收失效信息。只有在恢复期间的启动进程才有意义，
       * 因为它不维护 relcache，但会发送失效消息以让查询后端看到模式更改。
       */
      bool		sendOnly;		// 后端只发送，不接收 */

      /*
       * 每个空闲后端槽使用的下一个LocalTransactionId。 我们将其保存在这里是因为它由ProcNumber索引，
       * 并且在设置MyProcNumber时将值从本地内存复制到其中非常方便。在活动ProcState条目中没有意义。
       */
      LocalTransactionId nextLXID;
  } ProcState;




/* 共享缓存失效内存段, 存在于共享内存中 */
typedef struct SISeg
{
    /* 通用状态信息   */
    int         minMsgNum;        /* 最早仍然需要的消息 */
    int         maxMsgNum;        // 下一个要分配的消息数字 */
    int         nextThreshold;    /* 调用 SICleanupQueue 的消息数量 */

    slock_t     msgnumLock;       // 保护 maxMsgNum 的自旋锁 */

    /* 保存共享失效消息的循环缓冲区    */
    SharedInvalidationMessage buffer[MAXNUMMESSAGES];

    /*
     * 每个后端失效状态信息
     *
     * 'procState' 有 NumProcStateSlots 个条目，并且由 pgprocno 进行索引。
     * 'numProcs' 是当前使用的插槽数，'pgprocnos' 是它们索引的稠密数组，以加快扫描所有正在使用的插槽的速度。
     *
     * 'pgprocnos' 在很大程度上与 ProcArrayStruct->pgprocnos 重复，但有了我们单独的副本，避免了
     * ProcArrayLock 上的争用，并允许我们仅跟踪参与共享缓存失效的进程。
     */
    int         numProcs;
    int         *pgprocnos;
    ProcState   procState[FLEXIBLE_ARRAY_MEMBER];
} SISeg;
```


## <span class="section-num">2</span> Configurable parameters {#configurable-parameters}

-   `MAXNUMMESSAGES` <br />
    我们可以缓冲的共享失效消息的最大数量。为了提高速度，必须是 2 的幂次方。

-   `MSGNUMWRAPAROUND` <br />
    多久将 MsgNum 变量减小一次以避免溢出。必须是 MAXNUMMESSAGES 的倍数。应该足够大。

-   `CLEANUP_MIN` <br />
    在我们打扰调用 SICleanupQueue 之前必须在缓冲区中存在的消息的最小数目。

-   `CLEANUP_QUANTUM` <br />
    一旦超过 `CLEANUP_MIN` ，调用 SICleanupQueue 的频率（以消息数计）。为了提高速度，应为 2 的幂次方。

-   `SIG_THRESHOLD` <br />
    后端必须落后多少个消息才会发送 `PROCSIG_CATCHUP_INTERRUPT` 的最小消息数量。

-   `WRITE_QUANTUM` <br />
    在 SIInsertDataEntries 的每次迭代中将推送到缓冲区的最大消息数。非关键，但应该小于 `CLEANUP_QUANTUM` ，因为我们每次迭代只考虑调用 SICleanupQueue 一次。


## <span class="section-num">3</span> SICleanupQueue {#sicleanupqueue}

删除已被所有活跃后端进程消费的消息

如果调用者持有 SInvalWriteLock，则 callerHasWriteLock 为 true。
minFree 是要释放的最小消息槽数量。

-   落后太多的会被重置 (`stateP->resetState = true`)
-   严重落后但不至于重置的，会被发送信号 `PROCSIG_CATCHUP_INTERRUPT` <br />
    一次最多对一个后端进行信号处理。

注意：由于我们在信号其他后端进程时瞬时释放写锁，退出时不保证仍有 minFree 个空闲消息槽。调用者必须重新检查并可能重试。


### <span class="section-num">3.1</span> 重置后的处理 {#重置后的处理}

```c { linenos=true, anchorlinenos=true, lineanchors=org-coderef--cd71af }
void
AcceptInvalidationMessages(void)
{
    ReceiveSharedInvalidMessages(LocalExecuteInvalidationMessage,
                                 InvalidateSystemCaches);
    // ... skipped ...
}
void
ReceiveSharedInvalidMessages(void (*invalFunction) (SharedInvalidationMessage *msg),
                             void (*resetFunction) (void))
{
    // ... skipped ...

    getResult = SIGetDataEntries(messages, MAXINVALMSGS);

    if (getResult < 0)
    {
        /* got a reset message */
        elog(DEBUG4, "cache state reset");
        SharedInvalidMessageCounter++;
        resetFunction();
        break;				/* nothing more to do */
    }

    // ... skipped ...
}

int
SIGetDataEntries(SharedInvalidationMessage *data, int datasize)
{
    // ... skipped ...
    if (stateP->resetState)
    {
        /*
         * Force reset.  We can say we have dealt with any messages added
         * since the reset, as well; and that means we should clear the
         * signaled flag, too.
         */
        stateP->nextMsgNum = max;
        stateP->resetState = false;
        stateP->signaled = false;
        LWLockRelease(SInvalReadLock);
        return -1;
    }

    // ... skipped ...
}
```

-   `ReceiveSharedInvalidMessages()` 中调用 `SIGetDataEntries()` 来取得未处理的消息
-   `SIGetDataEntries()` 发现被重置后，返回 `-1`
-   `ReceiveSharedInvalidMessages()` 检查到返回值小于 `0` ，则调用传入的回调函数
    `InvalidateSystemCaches()` 来进行清理缓存


### <span class="section-num">3.2</span> 信号的处理 {#信号的处理}

信号处理最终会调到函数 `ProcessCatchupInterrupt()` ：

```c
/*
 * ProcessCatchupInterrupt
 *
 * 处理追赶中断的部分，运行在信号处理程序之外，使其能够实际处理待处理的失效消息。
 */
void
ProcessCatchupInterrupt(void)
{
    while (catchupInterruptPending)
    {
        /*
         * 在这里我们需要让 ReceiveSharedInvalidMessages() 函数运行，它将进行必要的处理并重置
         * catchupInterruptPending 标志。如果我们正在事务中，可以直接调用 AcceptInvalidationMessages()
         * 来执行这个操作。如果不是，在这里启动并立即结束一个事务；AcceptInvalidationMessages() 的调用
         * 发生在事务启动内部。
         *
         * 很诱人地，我们可以尝试只调用 AcceptInvalidationMessages()，而不执行事务启动/停止的其他操作，
         * 我认为在正常情况下这样做是可行的；但是如果出现错误，我不确定处理会不会很顺利。
         */
        if (IsTransactionOrTransactionBlock())
        {
            elog(DEBUG4, "ProcessCatchupEvent inside transaction");
            AcceptInvalidationMessages();
        }
        else
        {
            elog(DEBUG4, "ProcessCatchupEvent outside transaction");
            StartTransactionCommand();
            CommitTransactionCommand();
        }
    }
}
```

`AcceptInvalidationMessages()` 见 [2](#org-coderef--cd71af-2) 。

