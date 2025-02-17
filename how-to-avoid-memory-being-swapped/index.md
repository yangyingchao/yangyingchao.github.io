# how to avoid memory being swapped (locking memory pages)


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [mlockall()](#h:a69e195a-92fb-405b-9b6c-ec86c0015e47)
- <span class="section-num">2</span> [how to do this after program started and program does not call `mlockall()` ?](#h:694a46e8-f346-48c2-a21e-a71751c8643d)
- <span class="section-num">3</span> [内核实现](#h:a99f5c47-b061-4e47-a7f0-51dcf4e3b852)
    - <span class="section-num">3.1</span> [mlockall()](#h:6ff510c5-4a77-4766-b6ec-49b25660504a)
    - <span class="section-num">3.2</span> [内存区域标记](#h:75a484c0-9716-44e8-8811-9041f9878f19)
- <span class="section-num">4</span> [Swap](#h:7ad9b325-1adc-4075-ac3b-3e92208ae6a6)
    - <span class="section-num">4.1</span> [Swap info](#h:ddbd6d4f-ac07-456e-a148-dcdeac5f975d)
    - <span class="section-num">4.2</span> [Locks](#h:1fe99bb1-aeb0-4a49-9efe-56e123310ee6)
    - <span class="section-num">4.3</span> [fork 时候复制 swap&#x2026;](#h:36e44df6-8e26-4bd4-bfc8-80df31b3614b)
    - <span class="section-num">4.4</span> [进程退出时候清理 swap 。。。](#h:cb155170-b5a3-421f-9f98-090780fbf82a)
- <span class="section-num">5</span> [RLIMIT_MEMLOCK](#h:e5964c54-29f4-42fa-af12-10ca1093f20b)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://stackoverflow.com/questions/12520499/linux-how-to-lock-the-pages-of-a-process-in-memory



## <span class="section-num">1</span> mlockall() {#h:a69e195a-92fb-405b-9b6c-ec86c0015e47}

from manpage:

```text

mlockall() and munlockall()
    mlockall() locks all pages mapped into the address space of the  calling  process.   This  in‐
    cludes the pages of the code, data, and stack segment, as well as shared libraries, user space
    kernel data, shared memory, and memory-mapped files.  All mapped pages are  guaranteed  to  be
    resident  in  RAM  when the call returns successfully; the pages are guaranteed to stay in RAM
    until later unlocked.

    The flags argument is constructed as the bitwise OR of one or more of the following constants:

    MCL_CURRENT
           Lock all pages which are currently mapped into the address space of the process.

    MCL_FUTURE
           Lock all pages which will become mapped into the address space of the  process  in  the
           future.   These  could be, for instance, new pages required by a growing heap and stack
           as well as new memory-mapped files or shared memory regions.

    MCL_ONFAULT (since Linux 4.4)
           Used together with MCL_CURRENT, MCL_FUTURE, or both.  Mark all current  (with  MCL_CUR‐
           RENT)  or  future  (with  MCL_FUTURE)  mappings to lock pages when they are faulted in.
           When used with MCL_CURRENT, all present pages are locked, but mlockall() will not fault
           in non-present pages.  When used with MCL_FUTURE, all future mappings will be marked to
           lock pages when they are faulted in, but they will not be populated by  the  lock  when
           the mapping is created.  MCL_ONFAULT must be used with either MCL_CURRENT or MCL_FUTURE
           or both.

    If MCL_FUTURE has been specified, then a later  system  call  (e.g.,  mmap(2),  sbrk(2),  mal‐
    loc(3)), may fail if it would cause the number of locked bytes to exceed the permitted maximum
    (see below).  In the same circumstances, stack growth may likewise fail: the kernel will  deny
    stack expansion and deliver a SIGSEGV signal to the process.

munlockall() unlocks all pages mapped into the address space of the calling process.

```


## <span class="section-num">2</span> how to do this after program started and program does not call `mlockall()` ? {#h:694a46e8-f346-48c2-a21e-a71751c8643d}

Make a GDB command file that contains something like this:

```sh
call mlockall(3)
detach
```

Then on the command line, find the PID of the process you want to mlock. Type:

```sh
gdb --pid [PID] --batch -x [command file]
```

If you get fancy with `pgrep` that could be:

```sh
gdb --pid $(pgrep sshd) --batch -x [command file]
```


## <span class="section-num">3</span> 内核实现 {#h:a99f5c47-b061-4e47-a7f0-51dcf4e3b852}


### <span class="section-num">3.1</span> mlockall() {#h:6ff510c5-4a77-4766-b6ec-49b25660504a}

```c
SYSCALL_DEFINE1(mlockall, int, flags)
{
    unsigned long lock_limit;
    int ret;

    if (!flags || (flags & ~(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT)) ||
        flags == MCL_ONFAULT)
        return -EINVAL;

    if (!can_do_mlock())
        return -EPERM;

    lock_limit = rlimit(RLIMIT_MEMLOCK);
    lock_limit >>= PAGE_SHIFT;

    if (mmap_write_lock_killable(current->mm))
        return -EINTR;

    ret = -ENOMEM;
    if (!(flags & MCL_CURRENT) || (current->mm->total_vm <= lock_limit) ||
        capable(CAP_IPC_LOCK))
        ret = apply_mlockall_flags(flags);
    mmap_write_unlock(current->mm);
    if (!ret && (flags & MCL_CURRENT))
        mm_populate(0, TASK_SIZE);

    return ret;
}
```


#### <span class="section-num">3.1.1</span> EPERM 的情况 {#h:d9c66345-c146-46da-968a-1daaea8804cf}

当可执行程序拥有 `CAP_IPC_LOCK` 时候，可以有权限执行 mlock, 否则的话，下列情况会返回 EPERM:

1.  RLIMIT_MEMLOCK 为 0 时候：
    ```c
    bool can_do_mlock(void)
    {
        if (rlimit(RLIMIT_MEMLOCK) != 0)
            return true;
        if (capable(CAP_IPC_LOCK))
            return true;
        return false;
    }
    ```
2.  `MCL_CURRENT` 被设置，并且当前进程已使用的内存超过了 `RLIMIT_MEMLOCK`  .


### <span class="section-num">3.2</span> 内存区域标记 {#h:75a484c0-9716-44e8-8811-9041f9878f19}


## <span class="section-num">4</span> Swap {#h:7ad9b325-1adc-4075-ac3b-3e92208ae6a6}


### <span class="section-num">4.1</span> Swap info {#h:ddbd6d4f-ac07-456e-a148-dcdeac5f975d}

```plantuml
class swap_info_struct {
    + unsigned flags
    + short prio
    + struct list
    + struct avail_lists
    + signed type
    + unsigned max
    + unsigned swap_map
    + struct cluster_info
    + struct free_clusters
    + unsigned lowest_bit
    + unsigned highest_bit
    + unsigned pages
    + unsigned inuse_pages
    + unsigned cluster_next
    + unsigned cluster_nr
    + struct percpu_cluster
    + struct curr_swap_extent
    + struct first_swap_extent
    + struct bdev
    + struct swap_file
    + unsigned old_block_size
    + spinlock_t lock
    + spinlock_t cont_lock
    + struct discard_work
    + struct discard_clusters
}


class swap_cluster_info {
+ spinlock_t lock
+ unsigned data
+ unsigned flags
}


swap_info_struct *-- swap_cluster_info


class swap_cluster_list {
+ struct head
+ struct tail
}

swap_info_struct *-- swap_cluster_list

class swap_extent {
+ struct list
+ unsigned start_page
+ unsigned nr_pages
+ sector_t start_block
}


swap_info_struct *-- swap_extent

note left of swap_extent
一个交换区范围 (swap extent range) 将交换文件的
PAGE_SIZE 大小的页的范围映射到磁盘块的范围上。
交换区范围列表映射整个交换文件。

end note


class block_device {
+ dev_t bd_dev
+ int bd_openers
+ struct bd_inode
+ struct bd_super
+ struct bd_mutex
+ void bd_claiming
+ void bd_holder
+ int bd_holders
+ bool bd_write_holder
+ struct bd_holder_disks
+ struct bd_contains
+ unsigned bd_block_size
+ u8 bd_partno
+ struct bd_part
+ unsigned bd_part_count
+ int bd_invalidated
+ struct bd_disk
+ struct bd_queue
+ struct bd_bdi
+ struct bd_list
+ unsigned bd_private
+ int bd_fsfreeze_count
+ struct bd_fsfreeze_mutex
}

swap_info_struct *-- block_device

class file {
+ union f_u
+ struct f_path
+ struct f_inode
+ const f_op
+ spinlock_t f_lock
+ enum f_write_hint
+ atomic_long_t f_count
+ unsigned f_flags
+ fmode_t f_mode
+ struct f_pos_lock
+ loff_t f_pos
+ struct f_owner
+ const f_cred
+ struct f_ra
+ u64 f_version
+ void f_security
+ void private_data
+ struct f_ep_links
+ struct f_tfile_llink
+ struct f_mapping
+ errseq_t f_wb_err
}

swap_info_struct *-- file

class address_space {
+ struct host
+ struct i_pages
+ atomic_t i_mmap_writable
+ struct i_mmap
+ struct i_mmap_rwsem
+ unsigned nrpages
+ unsigned nrexceptional
+ unsigned writeback_index
+ const a_ops
+ unsigned flags
+ spinlock_t private_lock
+ gfp_t gfp_mask
+ struct private_list
+ void private_data
+ errseq_t wb_err
}

file *-- address_space

class inode {
+ umode_t i_mode
+ unsigned i_opflags
+ kuid_t i_uid
+ kgid_t i_gid
+ unsigned i_flags
+ struct i_acl
+ struct i_default_acl
+ const i_op
+ struct i_sb
+ struct i_mapping
+ void i_security
+ unsigned i_ino
+ dev_t i_rdev
+ loff_t i_size
+ struct i_atime
+ void i_private
}

address_space *-- inode
```

```c
/*
 * MAX_SWAPFILES defines the maximum number of swaptypes: things which can
 * be swapped to.  The swap type and the offset into that swap type are
 * encoded into pte's and into pgoff_t's in the swapcache.  Using five bits
 * for the type means that the maximum number of swapcache pages is 27 bits
 * on 32-bit-pgoff_t architectures.  And that assumes that the architecture packs
 * the type/offset into the pte as 5/27 as well.
*/
/*
 * MAX_SWAPFILES 定义了最大的swaptypes数量：可以被交换到的东西。交换类型和交换类型中的偏移量都被编码到
 * pte和swapcache中的pgoff_t中。对于类型使用五位意味着在32位pgoff_t架构上，swapcache页面的最大数量为27
 * 位。并且这假设架构也将类型/偏移量打包到pte中，比例为5/27。
 */

#define MAX_SWAPFILES                              \
    ((1 << MAX_SWAPFILES_SHIFT) - SWP_DEVICE_NUM - \
    SWP_MIGRATION_NUM - SWP_HWPOISON_NUM)

  struct swap_info_struct *swap_info[MAX_SWAPFILES];
```

最多 `MAX_SWAPFILES` 个 swap 设备。


### <span class="section-num">4.2</span> Locks {#h:1fe99bb1-aeb0-4a49-9efe-56e123310ee6}


#### <span class="section-num">4.2.1</span> 全局锁 {#h:4542388b-4a23-4f9a-afc5-7d3afbbaf6c5}

```c
DEFINE_SPINLOCK(swap_lock);
```


#### <span class="section-num">4.2.2</span> 粗粒度锁 {#h:35cfd6f1-394c-4a8f-8883-aff2964a9ff3}


#### <span class="section-num">4.2.3</span> 细粒度锁 {#h:aef166e9-9c66-4f62-8668-78d4e76aeaf9}


### <span class="section-num">4.3</span> fork 时候复制 swap&#x2026; {#h:36e44df6-8e26-4bd4-bfc8-80df31b3614b}

```text
14.74%     0.00%  xxxxsql  [kernel.kallsyms]         [k] entry_SYSCALL_64_after_hwframe
        |
        ---entry_SYSCALL_64_after_hwframe
           do_syscall_64
           |
           |--7.61%--__x64_sys_exit_group
           |          do_group_exit
           |          |
           |           --7.61%--do_exit
           |                     |
           |                      --7.46%--mmput
           |                                |
           |                                 --7.45%--exit_mmap
           |                                           |
           |                                            --6.37%--unmap_vmas
           |                                                      |
           |                                                       --6.33%--unmap_page_range
           |                                                                 |
           |                                                                 |--0.98%--free_swap_and_cache
           |                                                                 |
           |                                                                  --0.76%--tlb_flush_mmu
           |
           |--3.40%--_do_fork
           |          |
           |           --3.38%--copy_process
           |                     |
           |                      --3.15%--dup_mm
           |                                |
           |                                 --2.42%--copy_page_range
           |                                           |
           |                                            --2.35%--copy_pte_range.isra.97
           |                                                      |
           |                                                       --0.98%--swap_duplicate
           |                                                                 |
           |                                                                  --0.83%--__swap_duplicate

```


#### <span class="section-num">4.3.1</span> `swap_duplicate` {#h:b1a19790-e8c8-4c2a-a0a1-13fad351664e}

-   将 swap entry 的引用计数增加 1。
-   返回 `0` : 表示成功
-   返回 `-ENOMEM`  <br />
    如果需要 `swap_count_continuation` 但无法原子地分配，
-   如果 `__swap_duplicate()` 因其他原因失败（ `-EINVAL` 或 `-ENOENT` ），如页表条目损坏，则返回 0，就像它成功一样。

<!--listend-->

```c

/*
 * Increase reference count of swap entry by 1.
 * Returns 0 for success, or -ENOMEM if a swap_count_continuation is required
 * but could not be atomically allocated.  Returns 0, just as if it succeeded,
 * if __swap_duplicate() fails for another reason (-EINVAL or -ENOENT), which
 * might occur if a page table entry has got corrupted.
 */
int swap_duplicate(swp_entry_t entry)
{
    int err = 0;

    while (!err && __swap_duplicate(entry, 1) == -ENOMEM)
        err = add_swap_count_continuation(entry, GFP_ATOMIC);
    return err;
}

```


### <span class="section-num">4.4</span> 进程退出时候清理 swap 。。。 {#h:cb155170-b5a3-421f-9f98-090780fbf82a}


## <span class="section-num">5</span> RLIMIT_MEMLOCK {#h:e5964c54-29f4-42fa-af12-10ca1093f20b}

