# Lock out user after three failed login attempts


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Lock out user after three failed login attempts](#h:38aae4aa-a802-4cff-9ffb-1ecebc230218)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://wiki.archlinux.org/title/Security#Lock_out_user_after_three_failed_login_attempts



## <span class="section-num">1</span> Lock out user after three failed login attempts {#h:38aae4aa-a802-4cff-9ffb-1ecebc230218}

截至 pambase 20200721.1-2 版本，pam_faillock.so 已默认启用，当用户在 15 分钟内尝试 3 次登录失败后会将其锁定 10 分钟（参见 FS＃67644）。锁定仅适用于密码验证（例如登录和 sudo），通过 SSH 的公钥验证仍然被接受。为了防止完全的拒绝服务，对于默认情况下 root 用户将不会被锁定。

要解除用户的锁定，请执行以下操作：

$ faillock &#x2013;user username &#x2013;reset

默认情况下，锁定机制是每个用户一个文件，位于 /run/faillock/。删除或清空文件将解锁该用户——该目录由 root 拥有，但文件由用户拥有，因此 faillock 命令仅清空文件，因此不需要 root。

模块 pam_faillock.so 可以通过文件 `/etc/security/faillock.conf` 进行配置。锁定参数如下：

-   `unlock_time` —— 锁定时间（秒为单位，默认为 10 分钟）
-   `fail_interval` —— 允许登录失败引起锁定的时间（以秒为单位，默认值为 15 分钟）
-   `deny` —— 锁定之前的登录失败次数（默认值为 3）

提示：锁定的主要目的是放慢暴力攻击的速度，使其变得不可行。因此，如果由于密码输错而导致锁定过于频繁，则可以放宽尝试次数以而不是减少锁定时间。

注意：deny = 0 将完全禁用锁定机制。

默认情况下，所有用户锁定在重新启动后都会失效。如果攻击者可以重新启动计算机，则使锁定持续存在更为安全。要使锁定持久存在，请将/etc/security/faillock.conf 中的 dir 参数更改为 /var/lib/faillock。

更改不需要重新启动即可生效。请参阅 faillock.conf(5) 以获取更多配置选项，例如启用 root 帐户的锁定，禁用集中登录（例如 LDAP）等。

