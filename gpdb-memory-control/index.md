# gpdb memory control


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [<span class="org-todo todo TODO">TODO</span> VMem](#h:3ca2edd6-a67c-49c7-a59e-fcb2eea8be86)
- <span class="section-num">2</span> [Resource Group Control](#h:6dc217ce-2d94-47e0-8a0d-eaeee600085f)
    - <span class="section-num">2.1</span> [Bypass](#h:b2a8da56-79b8-4f04-ac28-5cbf019eea54)
    - <span class="section-num">2.2</span> [Fixed Mem](#h:3d341c1e-c9a0-4f50-a1c9-62f8b7cf2fae)
    - <span class="section-num">2.3</span> [Memory Limit](#h:6d42ffdf-6c48-4449-92e5-60f27205fc87)

</div>
<!--endtoc-->



## <span class="org-todo todo TODO">TODO</span> <span class="section-num">1</span> VMem {#h:3ca2edd6-a67c-49c7-a59e-fcb2eea8be86}


## <span class="section-num">2</span> Resource Group Control {#h:6dc217ce-2d94-47e0-8a0d-eaeee600085f}

-   `ResourceGroupGetQueryMemoryLimit(void)` 用于获取内存限制的绝对大小 （非百分比）


### <span class="section-num">2.1</span> Bypass {#h:b2a8da56-79b8-4f04-ac28-5cbf019eea54}

-   绕开资源限制模式

-   Enabled when:
    -   `gp_resource_group_bypass` is true: [guc_gp.c](https://github.com/greenplum-db/gpdb/blob/main/src/backend/utils/misc/guc_gp.c#L2754)
        ```c
        {
            {"gp_resource_group_bypass", PGC_USERSET, RESOURCES,
                gettext_noop("If the value is true, the query in this session will not be limited by resource group."),
                NULL
            },
            &gp_resource_group_bypass,
            false,
            check_gp_resource_group_bypass, NULL, NULL
        }
        ```

    -   Or command is one of:
        -   SET
        -   RESET
        -   SHOW


### <span class="section-num">2.2</span> Fixed Mem {#h:3d341c1e-c9a0-4f50-a1c9-62f8b7cf2fae}

固定内存大小

```c
if (gp_resgroup_memory_query_fixed_mem)
        return (uint64) gp_resgroup_memory_query_fixed_mem * 1024L;
```


### <span class="section-num">2.3</span> Memory Limit {#h:6d42ffdf-6c48-4449-92e5-60f27205fc87}

```c
queryMem = (uint64)(resgLimit *1024L *1024L / caps->concurrency);
```

