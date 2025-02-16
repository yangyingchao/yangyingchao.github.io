# GPDB: Configuring Your Systems


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [IP Fragmentation Settings](#h:3127b791-8437-4d53-bf04-17e651151969)
- <span class="section-num">2</span> [Networking](#h:8109178d-d6f2-4bf1-9213-38024fbf79b1)

</div>
<!--endtoc-->


本文为摘录，原文为： https://docs.vmware.com/en/VMware-Tanzu-Greenplum/6/greenplum-database/GUID-install_guide-prep_os.html



## <span class="section-num">1</span> [IP Fragmentation Settings](https://docs.vmware.com/en/VMware-Tanzu-Greenplum/6/greenplum-database/GUID-install_guide-prep_os.html#:~:text=IP%20Fragmentation%20Settings) {#h:3127b791-8437-4d53-bf04-17e651151969}

IP Fragmentation Settings

When the Greenplum Database interconnect uses UDP (the default), the network interface
card controls IP packet fragmentation and reassemblies.

If the UDP message size is larger than the size of the maximum transmission unit (MTU) of
a network, the IP layer fragments the message. (Refer to Networking later in this topic
for more information about MTU sizes for Greenplum Database.) The receiver must store the
fragments in a buffer before it can reorganize and reassemble the message.

The following sysctl.conf operating system parameters control the reassembly process:

> OS Parameter  Description
> net.ipv4.ipfrag_high_thresh  The maximum amount of memory used to reassemble IP
>   fragments before the kernel starts to remove fragments
>   to free up resources. The default value is 4194304
>   bytes (4MB).
> net.ipv4.ipfrag_low_thresh  The minimum amount of memory used to reassemble IP
>   fragments. The default value is 3145728 bytes (3MB).
>   (Deprecated after kernel version 4.17.)
> net.ipv4.ipfrag_time  The maximum amount of time (in seconds) to keep an IP
>   fragment in memory. The default value is 30.

The recommended settings for these parameters for Greenplum Database follow:

```cfg
net.ipv4.ipfrag_high_thresh = 41943040
net.ipv4.ipfrag_low_thresh = 31457280
net.ipv4.ipfrag_time = 60
```


## <span class="section-num">2</span> [Networking](https://docs.vmware.com/en/VMware-Tanzu-Greenplum/6/greenplum-database/GUID-install_guide-prep_os.html#networking%23:~:text=Networking) {#h:8109178d-d6f2-4bf1-9213-38024fbf79b1}

The maximum transmission unit (MTU) of a network specifies the size (in bytes) of the largest data packet/frame accepted
by a network-connected device. A jumbo frame is a frame that contains more than the standard MTU of 1500 bytes.

Greenplum Database utilizes 3 distinct MTU settings:

– The Greenplum Database `gp_max_packet_size` server configuration parameter. The default max packet size is 8192. This
  default assumes a jumbo frame MTU.
– The operating system MTU setting.
– The rack switch MTU setting.

These settings are connected, in that they should always be either the same,
or close to the same, value, or otherwise in the order of :
 `Greenplum < OS < switch for MTU size`.

9000 is a common supported setting for switches, and is the recommended OS and
rack switch MTU setting for your Greenplum Database hosts.

