# BTRFS and free space - emergency response


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [我还有多少自由空间？](#我还有多少自由空间)
- <span class="section-num">2</span> [将未使用的分配转换为自由空间](#将未使用的分配转换为自由空间)
- <span class="section-num">3</span> [如果您已经用尽空间](#如果您已经用尽空间)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://ohthehugemanatee.org/blog/2019/02/11/btrfs-out-of-space-emergency-response/

我在我的根文件系统（Linux 上）上使用 BTRFS，主要是为了快速快照和恢复的功能。昨天我遇到了一个常见的问题：我的驱动器突然满了。我从系统驱动器上的 4GB 的可用空间瞬间变为 0，导致我的系统出现各种混乱。

这个问题发生在许多人身上，因为 BTRFS 与“可用的空闲空间”之间没有线性关系。有几个概念会妨碍这一点：

– 压缩：BTRFS 支持在写入时压缩数据。这显然改变了可以存储的数据量。– 50MB 的文本可能只占用驱动器上的 5MB“空间”。– 元数据：BTRFS 将数据与元数据分开存储。数据和元数据都会占用“空间”。– 块分配：BTRFS 以块的方式为数据分配空间。– 多个设备：BTRFS 支持多个设备共同工作，类似 RAID。这意味着每个文件都需要存储额外的信息。例如，RAID-1 会存储每个文件的两个副本，因此一个 50MB 的文件需要占用 100MB 的空间。– 快照：BTRFS 可以存储设备的快照，实际上类似于当前状态的差异。差异中的数据量取决于您的当前状态……因此快照本身的大小并不一致。– 嵌套卷：BTRFS 允许您将文件系统划分为“子卷”，每个子卷都可以（在某个时候）具有自己的 RAID 配置。

很容易查看驱动器并告诉我还有多少 MiB 的空间没有使用。但准确地说出在这些空间中可以写入多少数据则非常困难。因此，系统工具如 df 报告的 BRFS 卷上的“自由空间”数量可能会经历很大的波动——就像我消失的 4GiB 一样。更糟糕的是，通用工具报告的可用空间可能会误导。BTRFS 可能会耗尽空间，而 df 仍然认为您有很多空间可用。

让我们一起了解一下 BTRFS 如何存储数据，以更好地理解这个问题。然后我们可以使用一些 BTRFS 自己的工具来解决它。


## <span class="section-num">1</span> 我还有多少自由空间？ {#我还有多少自由空间}

与其使用像 df 这样的通用工具来回答这个问题，不如使用 btrfs CLI 工具获得更多细节。

BTRFS 从一个大的原始存储池开始，并在此基础上动态分配。您可以通过以下命令列出块设备中所有的设备：

```console
$ sudo btrfs fi show
Label: 'OS'  uuid: c0d21ade-5570-41a3-b0cf-a5ce219e7a8e
  Total devices 1 FS bytes used 31.74GiB
  devid    1 size 48.83GiB used 47.80GiB path /dev/nvme0n1p2
```

在这种情况下，我只有一个物理设备。您可以看到它给出了已分配字节的总数，以及总大小。在另一个文件系统中，这可能是报告给 df 的数字。BTRFS 可不如此！让我们深入了解。

```console
$ btrfs fi df /
Data, single: total=45.75GiB, used=30.56GiB
System, single: total=32.00MiB, used=16.00KiB
Metadata, single: total=2.02GiB, used=1.17GiB
GlobalReserve, single: total=89.31MiB, used=0.00B
```

这里的“总计”值是第一个命令计为“使用”的内容的细分。btrfs fi df 显示已分配空间中实际存储数据的多少，以及多少只是空的分配。在这种情况下：在我的 48GiB 设备上，47GiB 是已分配的。在分配中，31GiB 实际上是存储数据的。附注：如果您处于多驱动器的情况下，此命令会考虑 RAID 元数据。

以下是更简单的视图：

```console
$ sudo btrfs fi usage /
Overall:
    Device size:      48.83GiB
    Device allocated:     47.80GiB
    Device unallocated:      1.03GiB
    Device missing:        0.00B
    Used:       31.74GiB
    Free (estimated):     16.22GiB  (min: 16.22GiB)
    Data ratio:           1.00
    Metadata ratio:         1.00
    Global reserve:     89.31MiB  (used: 0.00B)
Data,single: Size:45.75GiB, Used:30.56GiB
   /dev/nvme0n1p2   45.75GiB
Metadata,single: Size:2.02GiB, Used:1.18GiB
   /dev/nvme0n1p2    2.02GiB
System,single: Size:32.00MiB, Used:16.00KiB
   /dev/nvme0n1p2   32.00MiB
Unallocated:
   /dev/nvme0n1p2    1.03GiB
```

这显示了在该块设备中所有设备上分配和使用的空间细分。“总体”是针对整个块设备，而“空闲（估计）”的数字是报告给 df 的内容。

这很有问题：我的大多数正常工具告诉我有 15GB 的自由空间。但是如果我再写 1GiB 的数据，BTRFS 将仍然耗尽空间。这个问题很麻烦并且很难诊断。修复起来也更困难，因为大多数解决方案需要设备上有一些额外的空间。


## <span class="section-num">2</span> 将未使用的分配转换为自由空间 {#将未使用的分配转换为自由空间}

那么，为什么 BTRFS 分配如此多的空间来存储少量数据呢？我在 47GiB 的分配中存储了 31GiB 的数据，使用/总比率为 0.66！这非常低效。这是不幸的结果，因为它是一个写时复制的文件系统——BTRFS 在每次写入时都在新分配的块中开始。然而，块大小是静态的，而文件各不相同。因此，很多时候，一个块是未完全填满的。这就是我们所抱怨的“已分配但未使用”的空间。

幸运的是，有一种方法可以解决这个问题：BTRFS 有一个工具可以“再平衡”您的文件系统。它最初是为了平衡跨多个驱动器存储的数据（因此得名）设计的。然而，在单驱动器配置中，它也很有用，可以重新平衡数据在分配中的存储。

默认情况下，平衡将重写磁盘上的所有数据。这可能是不必要的。块的填满程度会不均匀，但我们在上面看到的，平均应该约为
66％使用。因此，我们将根据数据（-d）使用情况进行过滤，并仅重新平衡使用少于 66％的块。这样将留下任何高于平均水平的部分填充块。

-   在后台运行，因为这需要很长时间。
    ```console
    $ sudo btrfs balance start -dusage=66 / &
    ```

-   检查状态
    ```console
    $ sudo btrfs balance status -v /
    Balance on '/' is running
    1 out of about 27 chunks balanced (5 considered),  96% left
    Dumping filters: flags 0x1, state 0x1,
    ```

-   或者懒惰一点，让 bash 每 60 秒报告一次状态。
    ```console
    while :; do sudo btrfs balance status -v / ; sleep 60; done
    Balance on '/' is running
    3 out of about 27 chunks balanced (12 considered),  89% left
    Dumping filters: flags 0x1, state 0x1, force is off
    DATA (flags 0x2): balancing, usage=66
    Balance on '/' is running
    4 out of about 27 chunks balanced (13 considered),  85% left
    Dumping filters: flags 0x1, state 0x1, force is off
    DATA (flags 0x2): balancing, usage=66
    ...

    #当平衡操作完成时：
    Done, had to relocate 19 out of 59 chunks
    ```

完成，需重新定位 59 块中的 19 块

一旦完成，就会有很大的区别：

```console
$ btrfs filesystem df /
Data, single: total=32.53GiB, used=30.83GiB
System, single: total=32.00MiB, used=16.00KiB
Metadata, single: total=2.02GiB, used=1.17GiB
GlobalReserve, single: total=84.67MiB, used=0.00B
```

这就是为其他用途分配的 15GiB 空间。我的使用比率现在为 0.94。好耶！在某些罕见情况下，您可能需要对元数据分配执行此操作（使用-musage 而不是上面的-dusage）。


## <span class="section-num">3</span> 如果您已经用尽空间 {#如果您已经用尽空间}

如果您已经用尽空间，就无法运行平衡！在这种情况下，您必须得聪明一点。以下是您的选择：

▶ 1) 释放空间

这比听起来要难。如果您只是删除数据，它可能会留下那些部分填满的块，因此仍然是已分配的。您真正需要的是未分配的空间。获取此空间的最简单方法是删除快照。从最旧的开始，因为它将是最大的。

一旦您有一点喘息的空间，就重新平衡小段，比如元数据。然后继续按照上面描述的方法重新平衡数据。

▶ 2) 添加一些空间

不要忘了，BTRFS 卷可以跨多个设备！我最近不得不使用这个选项。添加一个设备——一个闪存驱动器也可以，但请选择最快的东西——并将其添加到 BTRFS 卷中。

-   添加您的额外驱动器 (/dev/sda)。
    ```console
    $ sudo btrfs device add -f /dev/sda /
    ```

-   现在运行您能做的最小平衡操作。
    ```console
    $ sudo btrfs balance start -dusage=1 /
    ```
    完成，需重新定位 59 块中的 1 块

<!--listend-->

-   移除设备，并运行正式的平衡。
    ```console
    $ sudo btrfs device remove /dev/sda /
    $ sudo btrfs balance start -dusage=66 /
    ```
    完成，需重新定位 59 块中的 18 块

平衡操作通常需要很长时间——超过一个小时并不少见。在涉及缓慢的闪存介质时，会需要更长时间。因此，我在这个例子中使用了非常低的平衡过滤器（-dusage=）。我们只需要释放一点空间，以便在不参与闪存驱动器的情况下再次运行平衡。

而这个最后的选项就是我昨晚拯救我的计算机的方法。我希望这能帮助到今后遭遇类似困境的某个人。

更新：请不要这样做！来自 BTRFS 社区的友好评论者告诉我，这其实是个非常糟糕的主意，因为任何中断 RAM 的情况都会不可挽回地破坏您的文件系统。坚持上面的 USB 驱动器解决方案。感谢@Zygo 的纠正，抱歉让任何人为我的学习而受苦。

更新：现在我已经不得不这样做几次，通过向文件系统添加一个 ramdisk 来平衡一个满的文件系统要好得多。这不仅比闪存设备更快，而且在大多数情况下更可靠……而且对于我这种使用案例（开发者笔记本电脑），重要的前提条件都存在：大量的 RAM，可靠的电源。以下是步骤：

-   创建一个 ramdisk。在执行此操作之前，请确保/dev/ram0 未被使用！
    ```console
    $ sudo mknod -m 660 /dev/ram0 b 1 0
    $ sudo chown root:disk /dev/ram0
    ```

-   以具体大小挂载 ramdisk。否则，它将增长到所需的大小。
    ```console
    $ sudo mkdir /mnt/ramdisk
    $ sudo mount -t ramfs -o size=4G,maxsize=4G /dev/ram0 /mnt/ramdisk
    ```

-   在 ramdisk 上创建一个文件以用作回环设备。
    ```console
    $ sudo dd if=/dev/zero of /mnt/ramdisk/extend.img bs=4M count=1000
    $ sudo losetup -fP /mnt/ramdisk/extend.img
    ```

-   找出哪个回环设备 ID 是您的
    ```console
    $ sudo losetup -a |grep extend.img
    /dev/loop10: [5243078]:8563965 (/mnt/ramdisk/extend.img)
    ```

-   将回环设备添加到 btrfs 文件系统
    ```console
    $ sudo btrfs device add /dev/loop10 /
    ```

-   确定您的平衡比率，并按往常一样进行平衡。
    ```console
    $ sudo btrfs fi usage / |head -n 6
    Overall:
    Device size:		 400.91GiB
    Device allocated:		 396.36GiB
    Device unallocated:		   4.55GiB
    Device missing:		     0.00B
    Used:			 348.91GiB
    $ echo 'scale=2;348/396' |bc
    .87
    ```

完成，需重新定位 400 块中的 46 块

-   移除设备并销毁它。
    ```console
    $ sudo btrfs device delete /dev/loop0 /
    $ sudo losetup -d /dev/loop10
    $ sudo umount /mnt/ramdisk
    $ sudo rm -rf /dev/ram0
    ```

