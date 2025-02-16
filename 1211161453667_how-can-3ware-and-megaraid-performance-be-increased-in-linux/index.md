# How can 3ware and MegaRAID performance be increased in Linux?


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Question](#h:641d66f2-5305-4f2f-a446-d78296b4a02a)
- <span class="section-num">2</span> [Answer](#h:531f86fa-2a4b-45e9-9414-8ed646ddae4a)
    - <span class="section-num">2.1</span> [For the Linux 2.6 kernel:](#h:73ac6463-9f36-49ae-93bf-6bcc9cbd4ddc)
    - <span class="section-num">2.2</span> [For the Linux 2.4 kernel:](#h:3fd1f3eb-0d39-4ee2-896c-54fe8b29d1af)

</div>
<!--endtoc-->


本文为摘录，原文为： https://www.broadcom.com/support/knowledgebase/1211161453667/how-can-3ware-and-megaraid-performance-be-increased-in-linux



## <span class="section-num">1</span> Question {#h:641d66f2-5305-4f2f-a446-d78296b4a02a}

How can LSI 3ware and LSI MegaRAID performance be increased in Linux?


## <span class="section-num">2</span> Answer {#h:531f86fa-2a4b-45e9-9414-8ed646ddae4a}


### <span class="section-num">2.1</span> For the Linux 2.6 kernel: {#h:73ac6463-9f36-49ae-93bf-6bcc9cbd4ddc}

See KB article A004958 Article ID 1211161457978
See also: Article ID 1211161479669

Third party software such as SarCheck <http://www.sarcheck.com> can also be used.


#### <span class="section-num">2.1.1</span> For Hard Disk Drives (HDDs): {#h:2a4fb8ae-a6a2-4515-9082-6dd5ea887aa2}

```sh
echo "deadline" > /sys/block/sda/queue/scheduler  (Turn on deadline I/O scheduler)
```


#### <span class="section-num">2.1.2</span> For Solid State Drives (SSDs): {#h:0c40a8ae-bb4c-452b-ac20-cd7dd087a7dd}

In addition to

```sh
echo "deadline" > /sys/block/sda/queue/scheduler (Turn on deadline I/O scheduler)
```

these will increase performance with SSDs:

```sh
echo "0" > /sys/block/sda/queue/rotational   (Turn off seek reordering)
echo "975" > /sys/block/sda/queue/nr_requests  (Turn up block layer queue depth for sda to 975)
echo "975" > /sys/block/sda/device/queue_depth (Turn up driver queue depth for sda to 975)
```

kernel OS is 2.6.18 which limits the MegaRAID controller's ability to use smp_affinity when used with motherboards with multiple processors (kernel 2.6.32 allows smp_affinity)


### <span class="section-num">2.2</span> For the Linux 2.4 kernel: {#h:3fd1f3eb-0d39-4ee2-896c-54fe8b29d1af}

Tuning Linux VM parameters may help to increase the read performance, depending your RAID type, application, and other factors. You can try this setting and see if it helps increase performance in your situation.

The settings are: To make the change without having to reboot (change will not survive after a reboot), type the following from a command prompt:

```sh
echo "2048" >/proc/sys/vm/min-readahead
echo "2048" >/proc/sys/vm/max-readahead
```

To make the change permanent, modify /etc/sysctl.conf and add the following lines:

```cfg
vm.max-readahead=2048
vm.min-readahead=2048
```

In addition, you can modify the bdflush parameter:

```sh
sysctl -w "vm.bdflush=10 500 0 0 500 3000 0 20 0"
```

Other information on Linux system tuning is available from: <http://people.redhat.com/alikins/system_tuning.html>

There are no other 3ware specific tuning parameters for Linux for read performance.

