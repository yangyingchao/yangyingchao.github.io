# Linux 中如何安全地抹去磁盘数据


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [rm 不能安全擦除](#h:2cbbde01-1566-4ece-b655-010ab5108341)
- <span class="section-num">2</span> [shred](#h:68545edf-ca95-4f46-b212-5a6c980ccc12)
- <span class="section-num">3</span> [wipe](#h:a19812b4-93b3-4891-afdc-b90917e37962)
- <span class="section-num">4</span> [dd](#h:25009ce5-a75d-4363-bef1-79876a0318e3)

</div>
<!--endtoc-->


本文为摘录，原文为： https://mp.weixin.qq.com/s/w-pMU3_TD3dEPoW-XEde-A

<https://mp.weixin.qq.com/s/w-pMU3_TD3dEPoW-XEde-A>


## <span class="section-num">1</span> rm 不能安全擦除 {#h:2cbbde01-1566-4ece-b655-010ab5108341}

因为 rm 命令或者文件管理器删除文件只是删除指向文件系统的指针（inode），所以原始数据仍可
以使用

> 在 Linux 中，文件由指针部分（inode）和数据部分（data）组成
>
> 文件被删除的时候，文件对应的 inode 就被删除掉了，而文件的数据部分在 inode 被清除掉
> 之后，就会被覆盖并写入新的内容


## <span class="section-num">2</span> shred {#h:68545edf-ca95-4f46-b212-5a6c980ccc12}

shred 功能是重复覆盖文件，只能使用专门的硬件并且极其困难地恢复数据，所以它经常被用来安
全地擦除数据

shred 是 Linux 软件包 coreutils 的一部分，所以一般情况下是默认安装的

<a id="figure--fig:screenshot@2023-10-20-09:00:34"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:00:34.png" width="800px" >}}

默认情况下，shred 会执行三次，在执行的时候，它会将伪随机数据写入设备。

例如我们要擦除 dev/sdb 设备

<a id="figure--fig:screenshot@2023-10-20-09:01:07"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:01:07.png" width="800px" >}}

前面我们说到，shred 会执行三次。但是执行三次所需的时间太长了，我们可以通过 -n 来设置执
行次数

<a id="figure--fig:screenshot@2023-10-20-09:02:15"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:02:15.png" width="800px" >}}

我们还可以添加更多选项来提高从磁盘删除数据的可靠性

<a id="figure--fig:screenshot@2023-10-20-09:02:35"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:02:35.png" width="800px" >}}

-   &#x2013;random-source=/dev/urandom ：表示指定使用随机生成的数据覆盖磁盘。Linux 的特殊文件
    /dev/urandom 为内核的随机数生成器提供了一个接口

-   -z /dev/sdb：用 0 覆盖到目前为止的所有内容

{{< figure src="/ox-hugo/640.png" width="800px" >}}


## <span class="section-num">3</span> wipe {#h:a19812b4-93b3-4891-afdc-b90917e37962}

看这个命令的名字就可以知道，这个命令用于擦除（wipe）磁盘中的数据

wipe 命令会重写磁盘扇区并刷新缓存，这使得想要恢复数据是一件极其困难或者说不可能的事

Linux 默认没有这个工具，我们需要先安装

<a id="figure--fig:screenshot@2023-10-20-09:03:33"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:03:33.png" width="800px" >}}

这个命令很简单，后面只需加上磁盘路径即可

<a id="figure--fig:screenshot@2023-10-20-09:03:50"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:03:50.png" width="800px" >}}

ps：现在 wipe 这个工具好像安装不了了（不知道是不是弃用了）


## <span class="section-num">4</span> dd {#h:25009ce5-a75d-4363-bef1-79876a0318e3}

dd 可从标准输入或文件中读取数据，根据指定的格式来转换数据，再输出到文件、设备或标准输出

<a id="figure--fig:screenshot@2023-10-20-09:04:19"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:04:19.png" width="800px" >}}

dd 命令是 Linux 中另一种常用的擦除磁盘的方法，尽管该命令并不像 shred 和 wipe 那样明确地
用于从磁盘中擦除数据，但它是 Linux 用户广泛使用的方法

例如下面的命令可以安全可靠地擦除 /dev/sdb 设备中的数据

<a id="figure--fig:screenshot@2023-10-20-09:04:38"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:04:38.png" width="800px" >}}

-   `if=/dev/urandom` ：输入文件，我们用 /dev/urandom 生成随机数据

-   `of=/dev/sdb` ：输出文件，表示要擦除的磁盘

-   `bs` ：块大小（以字节为单位）

<a id="figure--fig:screenshot@2023-10-20-09:05:15"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:05:15.png" width="800px" >}}

当显示 “No space left on device”时，表示擦除成功完成

或者我们用 zero 字符串对磁盘进行覆盖，而不是生成随机数据

<a id="figure--fig:screenshot@2023-10-20-09:05:28"></a>

{{< figure src="/ox-hugo/screenshot@2023-10-20_09:05:28.png" width="800px" >}}

参考文章：<https://linuxiac.com/best-ways-to-securely-erase-disk-in-linux/>

