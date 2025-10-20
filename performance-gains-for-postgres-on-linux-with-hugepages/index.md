# Performance gains for postgres on Linux with hugepages


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [实验](#实验)
- <span class="section-num">2</span> [结果](#结果)
- <span class="section-num">3</span> [启用大页](#启用大页)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.n0derunner.com/performance-gains-for-postgres-on-linux-with-hugepages/

对于这个实验，我使用的是 Linux 3.10 内核上的 Postgres v11。目标是查看使用大页可以获得什么收益。我使用“内置”基准
pgbench 来运行一组简单的查询。

由于我只对大页带来的收益感兴趣，因此选择了 pgbench 的 “-S” 参数，这意味着仅执行“选择”语句。显然，这掩盖了在污染大页时可能出现的任何成本——但这样可以使实验不必担心写入文件系统。


## <span class="section-num">1</span> 实验 {#实验}

工作站有 32GB 的内存
Postgres 被分配了 16GB 的内存，使用参数

```cfg
shared_buffers = 16384MB
```

pgbench 使用规模因子为 500 创建了一个 ~7.4GB 的数据库

```sh
pgbench -i -s 500
```

以以下方式运行实验

```sh
pgbench -c 10 -S -T 600 -P 1 pgbench
```


## <span class="section-num">2</span> 结果 {#结果}

-   默认：无大页：
    -   `tps = 62190.452850` （不包括连接建立）

-   2MB 大页
    -   `tps = 66864.410968` （不包括连接建立）
    -   比默认值高 +7.5%

-   1GB 大页
    -   `tps = 69702.358303` （不包括连接建立）
    -   比默认值高 +12%


## <span class="section-num">3</span> 启用大页 {#启用大页}

获取默认大页只需在 `/etc/sysctl.conf` 中输入一个值。为了允许 `16GB` 的大页，我使用了 `8400` 的值，然后执行 _sysctl -p_

```sh
[root@arches gary]# grep huge /etc/sysctl.conf
vm.nr_hugepages = 8400
[root@arches gary]# sysctl -p
```

要获取 1GB 的大页，内核必须在启动时进行配置，例如：

```sh
[root@arches boot]# grep CMDLINE /etc/default/grub
GRUB_CMDLINE_LINUX="rd.lvm.lv=centos/swap vconsole.font=latarcyrheb-sun16 rd.lvm.lv=centos/root crashkernel=auto vconsole.keymap=us rhgb quiet rdblacklist=nouveau default_hugepagesz=1G hugepagesz=1G
```

然后重启内核

