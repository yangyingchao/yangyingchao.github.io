# Configuring Linux Huge Pages for PostgreSQL Performance


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [When to use huge pages and why they matter 何时使用大页面以及它们的重要性](#when-to-use-huge-pages-and-why-they-matter-何时使用大页面以及它们的重要性)
- <span class="section-num">2</span> [Calculating how many huge pages are required 计算所需的巨大页面数量](#calculating-how-many-huge-pages-are-required-计算所需的巨大页面数量)
    - <span class="section-num">2.1</span> [PostgreSQL 15+ - use the built‑in calculator](#postgresql-15-plus-use-the-built-in-calculator)
    - <span class="section-num">2.2</span> [Pre‑15 releases - compute manually](#pre-15-releases-compute-manually)
    - <span class="section-num">2.3</span> [1 GB huge pages](#1-gb-huge-pages)
- <span class="section-num">3</span> [Allocating huge pages at the operating system level 在操作系统层面分配大页](#allocating-huge-pages-at-the-operating-system-level-在操作系统层面分配大页)
- <span class="section-num">4</span> [Disable Transparent Huge Pages (THP) 禁用透明大页 (THP)](#disable-transparent-huge-pages--thp--禁用透明大页--thp)
- <span class="section-num">5</span> [Environment‑specific considerations 环境特定的考虑因素](#environment-specific-considerations-环境特定的考虑因素)
    - <span class="section-num">5.1</span> [Kubernetes and containerized deployments](#kubernetes-and-containerized-deployments)
    - <span class="section-num">5.2</span> [Amazon RDS/Aurora PostgreSQL](#amazon-rds-aurora-postgresql)
    - <span class="section-num">5.3</span> [Azure PostgreSQL 灵活服务器](#azure-postgresql-灵活服务器)
    - <span class="section-num">5.4</span> [Windows 服务器](#windows-服务器)
- <span class="section-num">6</span> [Troubleshooting and best practices](#troubleshooting-and-best-practices)
- <span class="section-num">7</span> [Summary checklist](#summary-checklist)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://stormatics.tech/blogs/configuring-linux-huge-pages-for-postgresql

大页是 Linux 内核的一项功能，它分配更大的内存页（通常是 2MB 或 1GB，而不是正常的 4KB）。PostgreSQL
的共享缓存池和动态共享内存段通常达到数十 GB，使用大页可以减少处理器必须管理的页数。较少的页表项意味着更少的转换旁路缓存（TLB）缺失和更少的页表遍历，从而减少 CPU 开销，提高查询吞吐量和并行查询性能。
PostgreSQL 文档指出，大页“减少了开销……从而导致更小的页表和更少的 CPU 时间用于内存管理”[1]。

本博客解释了大页的重要性以及如何在 Linux 上正确配置现代 PostgreSQL 版本（v15-v18）。它还涵盖了 Kubernetes、云服务（Amazon RDS/Aurora 和 Azure Database for PostgreSQL）和 Windows 的特殊考虑，并强调了禁用透明大页（THP）。


## <span class="section-num">1</span> When to use huge pages and why they matter 何时使用大页面以及它们的重要性 {#when-to-use-huge-pages-and-why-they-matter-何时使用大页面以及它们的重要性}

-   性能优势： <br />
    使用大型共享缓存时，常规的 4 KB 页会导致许多 TLB 缺失。大页将内存分组为 2 MB 或 1 GB 的块，从而使 CPU 在管理内存上花费的时间更少，更多地专注于执行查询。 <br />

    官方文档强调大页减少了 CPU 开销[^fn:1]，而 pganalyze 指出， 在专用的 PostgreSQL
    服务器上启用大页可以在共享缓存达到数十 GB 时带来可测量的性能提升。

-   专用服务器：<br />
    由于大页保留了物理内存，因此最适合用于专用数据库服务器。<br />
    在多功能服务器上过度分配大页可能会让操作系统或其他服务困难重重。

-   使用 `huge_pages=on` 以提高清晰度： <br />
    PostgreSQL 的 **huge_pages** 参数接受 `off` 、 `try` （默认）或 `on` 。<br />
    当设置为 on 时，如果没有足够的大页可用，PostgreSQL 将拒绝启动；这会立即暴露配置错误。文档鼓励启用大页，并警告透明大页（THP）通常会降低数据库性能[1]（见§4）。


## <span class="section-num">2</span> Calculating how many huge pages are required 计算所需的巨大页面数量 {#calculating-how-many-huge-pages-are-required-计算所需的巨大页面数量}


### <span class="section-num">2.1</span> PostgreSQL 15+ - use the built‑in calculator {#postgresql-15-plus-use-the-built-in-calculator}

PostgreSQL 15 引入了 `shared_memory_size_in_huge_pages` 参数，该参数报告了持有服务器共享内存所需的大页数量。您可以在不启动服务器的情况下查询它：

对于 20 GiB 的共享缓冲区和 2 GiB 的动态共享内存

```sh
postgres --shared-buffers=20GB \
         --min-dynamic-shared-memory=2GB \
         -C shared_memory_size_in_huge_pages
```

此命令输出所需的确切数量的 2 MB 大页（例如，对于 20 GiB 的缓冲区加上 2 GiB 的动态共享内存，大约需要 `~11,499`
页）。您还可以通过 `-C dynamic_shared_memory_type` 检查动态共享内存，或通过 `min_dynamic_shared_memory`
分配特定大小，以确保并行工作进程也使用大页。


### <span class="section-num">2.2</span> Pre‑15 releases - compute manually {#pre-15-releases-compute-manually}

对于 PostgreSQL 14 及更早版本，手动计算所需的巨页数量：

1.  找到您想要运行的 Linux 系统上的巨页大小（通常为 2MB）。
2.  将 shared_buffers （以 MB 为单位）除以 2（巨页的大小）。
3.  加上 `5%` 的缓冲，以确保 PostgreSQL 始终拥有所需的巨页数量。
4.  结果数字就是您应该分配的巨页数量。


### <span class="section-num">2.3</span> 1 GB huge pages {#1-gb-huge-pages}

大多数部署使用 2 MB 页面，但一些 CPU 支持 1 GB 页面。您可以通过在 **postgresql.conf** 中设置 `huge_page_size =
1GB` 并通过 `/sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages` 分配它们。大页面进一步减少了 TLB 压力，但内存碎片使其更难分配；通常需要重启。

\*


## <span class="section-num">3</span> Allocating huge pages at the operating system level 在操作系统层面分配大页 {#allocating-huge-pages-at-the-operating-system-level-在操作系统层面分配大页}

一旦您知道所需的页面数量，请在内核中预分配这些页面。以下步骤适用于大多数 Linux 发行版（RHEL、Ubuntu、Debian、SUSE 等）：

1.  通过 sysctl 保留大页：
    ```cfg
    # 示例：分配 11,500 个两兆字节的页面
    sysctl -w vm.nr_hugepages=11500
    # 要在重启后保持，请将以下内容添加到/etc/sysctl.conf 或/etc/sysctl.d/postgres.conf：
    vm.nr_hugepages = 11500
    ```
    或者，直接写入/proc/sys/vm/nr_hugepages[7]。对于 1 GB 的页面，请写入
    `/sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages` 。

2.  允许 postgres 用户锁定大内存 <br />
    在 `/etc/sysctl.conf` 中将 `vm.hugetlb_shm_group` 设置为 postgres 用户的组 ID，以便该组内的进程可以在没有
    root 权限的情况下分配大页。同时，增加 systemd 服务文件或 `/etc/security/limits.conf` 中的
    `ulimit -l` （最大锁定内存）。

3.  检查分配情况 <br />
    在设置 vm.nr_hugepages 后，通过以下命令验证(查找 HugePages_Total 和 HugePages_Free)：
    ```sh
    grep -i huge /proc/meminfo
    ```

4.  重启 PostgreSQL <br />
    在 **_postgresql.conf_** 中设置 `huge_pages` ，选择性地设置 h `uge_page_size` ，然后重启服务器。<br />
    文档指出 **huge_pages** 可以设置为 `try` 、 `on` 或 `off` ；对于 `try` ，如果无法分配大页，PostgreSQL
    会回退到普通页面，而 `on` 则会导致启动失败，因此建议使用 `try` 来捕获配置错误[4]。<br />
    启动后，检查 `SHOW huge_pages;`  应该返回 `on` （或 2MB/1GB）。

5.  分配动态共享内存 <br />
    如果您使用并行查询或逻辑复制，请在 **_postgresql.conf_** 中设置 `min_dynamic_shared_memory`
    以预分配动态共享内存。如果没有这个，平行工作进程使用的动态段将回退到普通页面，从而削弱部分好处。自
    PostgreSQL 15 起，您可以将其包含在 `shared_memory_size_in_huge_pages` 计算中。

6.  避免过度分配 <br />
    分配过多的大页可能会使操作系统资源紧张；EDB 调整指南警告说，将 `vm.nr_hugepages`
    设置得远高于需求可能会防止其他服务启动或导致服务器无法启动。


## <span class="section-num">4</span> Disable Transparent Huge Pages (THP) 禁用透明大页 (THP) {#disable-transparent-huge-pages--thp--禁用透明大页--thp}

透明大页（thp）是一个 linux 内核特性，旨在自动使用大页。对于数据库工作负载，thp 经常导致内存碎片、高延迟和不可预测的性能。postgresql 文档和几个调优指南强烈建议禁用它[^fn:2]。

1.  Temporarily (until reboot):
    ```sh
    echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
    echo never | sudo tee /sys/kernel/mm/transparent_hugepage/defrag
    ```

2.  Persistent configuration <br />
    在内核命令行（GRUB）中添加 `transparent_hugepage=never` ，或者创建一个 systemd 服务，在启动时将 never 写入 THP 文件。<br />
    在 Kubernetes 集群中，通过机器配置设置内核启动参数或运行一个禁用 THP 的 DaemonSet。

    必须在启用 PostgreSQL 大页之前禁用 THP，因为否则内核可能会使用 THP 而不是保留的大页来映射共享内存，导致性能不一致。


## <span class="section-num">5</span> Environment‑specific considerations 环境特定的考虑因素 {#environment-specific-considerations-环境特定的考虑因素}


### <span class="section-num">5.1</span> Kubernetes and containerized deployments {#kubernetes-and-containerized-deployments}

Kubernetes Pod 不能按需分配大页；大页必须在节点级别进行保留。要使用它们：

1.  按照§3 的描述配置节点的内核（vm.nr_hugepages 或 1 GB 等价物）。
2.  在您的 pod 或 PostgresCluster 规格中，使用 resources.limits.hugepages-2Mi:(或 hugepages-1Gi)请求大页面资源。
    Crunchy Data PostgreSQL Operator 指出，您必须请求足够的页面以满足节点上所有 Postgres 实例的需求，并警告
    Kubernetes 存在一个 bug，Postgres 可能会检测到大页面并尝试使用它们，即使未被请求。他们的操作默认
    huge_pages=off，仅在请求大页面资源时设置为尝试。
3.  在集群或容器的 PostgreSQL 配置中设置 huge_page_size 和 huge_pages，以匹配请求的大小（2 MB 或 1 GB）。
4.  确保 min_dynamic_shared_memory 被设置，以便并行工作者可以使用大页面。


### <span class="section-num">5.2</span> Amazon RDS/Aurora PostgreSQL {#amazon-rds-aurora-postgresql}

AWS 在许多内存优化的 RDS 实例类型上自动使用大页。它们深入的文档建议在大型实例的 DB 参数组中启用大页并将
`huge_pages` 设置为开启。因为 RDS 管理操作系统，所以您无法自己更改 vm.nr_hugepages；只需在参数组中将
`huge_pages=on` ，并让 RDS 处理分配。对于较小的实例类型或较旧的内核，可能不支持大页；在修改参数组后，请监控 SHOW
`huge_pages_status` 。


### <span class="section-num">5.3</span> Azure PostgreSQL 灵活服务器 {#azure-postgresql-灵活服务器}

Azure 的灵活服务器使用 Linux 大页，但不暴露内核参数。根据 Azure 文档，大页会自动用于共享内存区域，无法显式控制。您可以在服务器参数中设置 `huge_pages=on` ，以确保 Postgres 需要大页，并检查 `huge_pages_status` 变量以确认。该服务默认禁用 THP。


### <span class="section-num">5.4</span> Windows 服务器 {#windows-服务器}

PostgreSQL 的大页功能依赖于 Linux 的 MAP_HUGETLB；在 Windows 上不可用。

Windows 提供一个独立的“大页”功能，可以被调用 AllocateLargePages 并需要“锁定内存中的页面”特权的应用程序使用。
PostgreSQL 当前不实现这一功能（截至版本 18），因此您无法在 Windows 上使用大页。如果您在 Windows 上需要大内存页，请在 Linux 虚拟机或容器中运行 Postgres。


## <span class="section-num">6</span> Troubleshooting and best practices {#troubleshooting-and-best-practices}

-   检查 huge_pages_status： <br />
    启动 PostgreSQL 后，查询 `SHOW huge_pages_status;` 。如果返回 enabled，则表示正在使用 huge pages。如果显示
    disabled by system 或 disabled by configuration，则表明内核缺少 huge pages 或 huge_pages 设置已关闭。SHOW
    huge_page_size; 可以显示当前使用的是 2 MB 还是 1 GB 的页面[6]。

-   Cgroups 和容器限制：<br />
    在 cgroup 限制的环境中（例如 Kubernetes），不够的 hugetlb cgroup 限制可能会导致 `huge_pages=try` 尝试分配 huge
    pages，但在第一次页面错误（SIGBUS）时崩溃。

    2024 年的邮件列表补丁提议使用 MADV_POPULATE_READ 来检测这种情况。在该补丁可用之前，请确保容器的 cgroup hugetlb
    限制足够大，或者在未请求 huge pages 时将 `huge_pages` 设置为 `off` 。

-   监控内存压力：<br />
    Huge pages 不能被交换。如果服务器经历内存压力，内核无法回收 huge pages。当数据库很少使用所有保留内存时，避免分配 huge pages。

-   调整 shared_buffers：<br />
    虽然 huge pages 降低了开销，但它们不能替代良好的内存配置。最佳实践建议将可用系统内存的至少 25% 分配给
    shared_buffers（假设是专用的数据库服务器），最大可达 40%。


## <span class="section-num">7</span> Summary checklist {#summary-checklist}

1.  使用 `postgres -C shared_memory_size_in_huge_pages` 估算所需的 huge pages（Postgres 15 及以上版本）或使用针对旧版本的手动公式。
2.  通过 `vm.nr_hugepages` （2 MB）或 `/sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages` （1 GB）预分配大页。
3.  设置内核权限（ `vm.hugetlb_shm_group` 和 `ulimit -l` ），以允许 postgres 用户锁定内存[6]。
4.  设置 `huge_pages=on` ，选择性地设置 `huge_page_size = 2MB` 或 `1GB` ，并配置 `min_dynamic_shared_memory`
    以支持并行查询。
5.  通过将 `/sys/kernel/mm/transparent_hugepage/enabled` 和 `/defrag` 设置为 never 来禁用透明大页，并通过内核启动参数保持持久。
6.  重启 PostgreSQL，验证 `SHOW huge_pages_status`;，如有必要进行调整。在测试期间使用 `try` ，待稳定后切换为 `on` 。
7.  考虑针对 Kubernetes、云服务或 Windows（其中大页不可用）进行环境特定的调整。

当配置正确时，大页可以减少 CPU 开销并提高内存密集型 PostgreSQL 工作负载的吞吐量。关键是精确计算所需的大页数量，在内核中分配它们，禁用透明大页，并让 PostgreSQL 通过 huge_pages=on 使用它们。

[^fn:1]: <https://www.postgresql.org/docs/current/runtime-config-resource.html#:~:text=%60huge_pages%60%20%28%60enum%60%29%20%20>
[^fn:2]: <https://www.percona.com/blog/transparent-huge-pages-refresher>

