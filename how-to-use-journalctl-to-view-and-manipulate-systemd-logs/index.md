# How To Use Journalctl to View and Manipulate Systemd Logs


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Basic Log Viewing](#h:cf631f63-41b8-4f82-8544-76d72d2bdcdf)
- <span class="section-num">2</span> [Journal Filtering by Time](#h:a95fc26a-5a39-458f-9a0f-bbbddcbab960)
    - <span class="section-num">2.1</span> [Displaying Logs from the Current Boot](#h:692298a1-3c38-4603-9577-94123cec0d63)
    - <span class="section-num">2.2</span> [Filtering by Message Interest](#h:d6a93171-ab71-47be-b173-951f3cc5675a)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.digitalocean.com/community/tutorials/how-to-use-journalctl-to-view-and-manipulate-systemd-logs



## <span class="section-num">1</span> Basic Log Viewing {#h:cf631f63-41b8-4f82-8544-76d72d2bdcdf}

要查看 journald 守护程序收集的日志，请使用 journalctl 命令。

当单独使用时，系统中的每个日志条目都会在翻页器（通常是 less）中显示，供您浏览。最旧的条目将显示在顶部：

```text
journalctl

Output
-- Logs begin at Tue 2015-02-03 21:48:52 UTC, end at Tue 2015-02-03 22:29:38 UTC. --
Feb 03 21:48:52 localhost.localdomain systemd-journal[243]: Runtime journal is using 6.2M (max allowed 49.
Feb 03 21:48:52 localhost.localdomain systemd-journal[243]: Runtime journal is using 6.2M (max allowed 49.
Feb 03 21:48:52 localhost.localdomain systemd-journald[139]: Received SIGTERM from PID 1 (systemd).
Feb 03 21:48:52 localhost.localdomain kernel: audit: type=1404 audit(1423000132.274:2): enforcing=1 old_en
Feb 03 21:48:52 localhost.localdomain kernel: SELinux: 2048 avtab hash slots, 104131 rules.
Feb 03 21:48:52 localhost.localdomain kernel: SELinux: 2048 avtab hash slots, 104131 rules.
Feb 03 21:48:52 localhost.localdomain kernel: input: ImExPS/2 Generic Explorer Mouse as /devices/platform/
Feb 03 21:48:52 localhost.localdomain kernel: SELinux:  8 users, 102 roles, 4976 types, 294 bools, 1 sens,
Feb 03 21:48:52 localhost.localdomain kernel: SELinux:  83 classes, 104131 rules

. . .
```

你可能会注意到，当前显示的时间戳都是使用本地时间。现在我们已经在系统上正确地设置了本地时间，因此每条日志条目都可以使用本地时间显示。所有日志都使用这些新信息进行显示。

如果你想显示 UTC 时间戳，可以使用&#x2013;utc 标志： `journalctl --utc`


#### <span class="section-num">1.0.1</span> Displaying Kernel Messages {#h:c4ccd84c-9681-4ddb-93a0-0a1f1de5e4ea}

内核消息通常可以在 dmesg 输出中找到，并且也可以从日志中检索。

要仅显示这些消息，我们可以在命令中添加 `-k=或 =--dmesg` 标志：

```sh
journalctl -k
```

默认情况下，这将显示当前启动的内核消息。您可以使用先前讨论过的正常引导选择标志指定其他引导。例如，要获取五个启动前的消息，您可以键入：

```sh
journalctl -k -b -5
```


## <span class="section-num">2</span> Journal Filtering by Time {#h:a95fc26a-5a39-458f-9a0f-bbbddcbab960}


### <span class="section-num">2.1</span> Displaying Logs from the Current Boot {#h:692298a1-3c38-4603-9577-94123cec0d63}

-   journalctl -b
-   journalctl &#x2013;list-boots
-   journalctl -b -1


#### <span class="section-num">2.1.1</span> Time Windows {#h:9848389e-a8bf-4fb5-acf2-8e8d5a6c02f4}

-   journalctl &#x2013;since "2015-01-10 17:15:00"
-   journalctl &#x2013;since "2015-01-10" &#x2013;until "2015-01-11 03:00"


### <span class="section-num">2.2</span> Filtering by Message Interest {#h:d6a93171-ab71-47be-b173-951f3cc5675a}


#### <span class="section-num">2.2.1</span> By Unit {#h:b4688189-5838-407a-9730-705f81f22dad}

-   journalctl -u nginx.service
-   journalctl -u nginx.service &#x2013;since today
-   journalctl -u nginx.service -u php-fpm.service &#x2013;since today


#### <span class="section-num">2.2.2</span> By Process, User, or Group ID {#h:861a1506-8460-4fc3-bd7c-011f42a63112}

内核消息通常在 dmesg 的输出中找到，也可以从日志中检索出来。

要仅显示这些消息，我们可以在命令中添加-k 或&#x2013;dmesg 标志：

`journalctl -k`

默认情况下，这将显示来自当前启动的内核消息。您可以使用之前讨论的标准启动选择标志指定替代启动。例如，要获取五个启动前的消息，可以键入：

`journalctl -k -b -5`


#### <span class="section-num">2.2.3</span> By Priority {#h:9ba0bf60-421b-4ee3-a8b5-a9f3ed93e38b}

系统管理员经常感兴趣的一个过滤器是消息优先级。虽然记录详细信息通常很有用，但在实际处理可用信息时，低优先级的日志可能会分散注意力，使人感到困惑。您可以使用 journalctl 命令使用-p 选项仅显示指定优先级或更高优先级的消息。这允许您过滤掉较低优先级的消息。例如，您可以键入以下内容仅显示在错误级别或更高级别记录的条目：

`journalctl -p err -b`

这将向您显示所有标记为错误、关键、警报或紧急的消息。journal 实现了标准的 syslog 消息级别。您可以使用优先级名称或其相应的数字值。按照从最高到最低优先级的顺序，它们是：

– 0：emerg
– 1：alert
– 2：crit
– 3：err
– 4：warning
– 5：notice
– 6：info
– 7：debug

上述数字或名称可以在-p 选项中互换使用。选择一个优先级将显示标记为指定级别及其以上级别的消息。

