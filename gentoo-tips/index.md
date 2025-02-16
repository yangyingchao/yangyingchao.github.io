# Gentoo Tips


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Failed to login as normal user](#h:7b8e2b63-d5b8-4c2a-93bf-c865d2a8223d)

</div>
<!--endtoc-->



## <span class="section-num">1</span> Failed to login as normal user {#h:7b8e2b63-d5b8-4c2a-93bf-c865d2a8223d}

passwd 中，用户使用的 shell ， 必需是包含在 `/etc/shells` 里面的，否则会无法登录，哪怕密码没有问题。

