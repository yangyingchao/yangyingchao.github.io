# Ulimit conflict with PAM and Systemd


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [`ulimit` 是做什么的](#h:4f03ce15-57e7-4a81-9960-895d5e88acb4)
- <span class="section-num">2</span> [ulimit 从哪里读取配置文件](#h:09c51108-3b02-4b55-9d68-399352d01d07)
- <span class="section-num">3</span> [How PAM Modules related to  /etc/security/limit.d/\*.conf](#h:9f953590-6cd4-4b57-9a09-cd2ecbed0f7d)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://mydbops.wordpress.com/2017/12/10/ulimit-conflict-with-pam-and-systemd%E2%80%8B%E2%80%8B%E2%80%8B/#content



## <span class="section-num">1</span> `ulimit` 是做什么的 {#h:4f03ce15-57e7-4a81-9960-895d5e88acb4}

User limits command, limit the use of system-wide resources.


## <span class="section-num">2</span> ulimit 从哪里读取配置文件 {#h:09c51108-3b02-4b55-9d68-399352d01d07}

{{< figure src="images/ulimit-conflict-with-pam-and-systemd​​​/screen-shot-2017-12-08-at-3-24-21-pm.png" width="800px" >}}

-   从下面的文件、目录中读取配置文件
    -   /etc/security/limits.conf
    -   /etc/security/limits.d/\*.conf

-   默认从前者读取，然后用后面的文件中读取来覆盖前者。

-   目录 _etc/security/limits.d_ 中的文件，按照 C locale 的顺序来读取、解析并应用


## <span class="section-num">3</span> How PAM Modules related to  /etc/security/limit.d/\*.conf {#h:9f953590-6cd4-4b57-9a09-cd2ecbed0f7d}

-   The pam_limits PAM module sets limits on the system resources that can be obtained in a user session.
-   When accessing a system via ssh through sshd the /etc.pam.d/sshd policy file is consulted.

