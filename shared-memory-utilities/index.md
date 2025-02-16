# shared memory utilities


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [RTFM&#x2026;](#h:c32fe430-3144-4745-8d64-a0bce73720c9)
    - <span class="section-num">1.1</span> [ipcs](#h:aa00c038-6852-443e-8679-c0081f6bcbd9)
    - <span class="section-num">1.2</span> [ipcrm](#h:528e7c73-98e2-4f52-b45a-0715874093fd)
- <span class="section-num">2</span> [Check shared memory in Linux](#h:06646b4f-87e4-4591-919b-cce0fe1f3983)

</div>
<!--endtoc-->


本文为摘录，原文为： https://linuxopsys.com/topics/check-shared-memory-in-linux



## <span class="section-num">1</span> RTFM&#x2026; {#h:c32fe430-3144-4745-8d64-a0bce73720c9}


### <span class="section-num">1.1</span> ipcs {#h:aa00c038-6852-443e-8679-c0081f6bcbd9}


### <span class="section-num">1.2</span> ipcrm {#h:528e7c73-98e2-4f52-b45a-0715874093fd}


## <span class="section-num">2</span> [Check shared memory in Linux](https://linuxopsys.com/topics/check-shared-memory-in-linux#:~:text=Check%20shared%20memory%20in%20Linux) {#h:06646b4f-87e4-4591-919b-cce0fe1f3983}

Print active shared memory segments using -m option.

```text
# ipcs -m
------ Shared Memory Segments --------
key shmid owner perms bytes nattch status
0x00000000 65536 root 600 393216 2 dest
0x00000000 98305 root 600 393216 2 dest
```

where,

"dest" status means memory segment is marked to be destroyed
"nattach" field shows how many application pid's are still attached to the shared memory

The following command print information about active shared memory segments:

