# Compiler considerations — The Linux Kernel 5.7.0+ documentation


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Interprocedural optimization](#h:4fd016cf-5c6d-455b-9d00-b939b023b4b6)
- <span class="section-num">2</span> [GCC function suffixes](#h:548d6856-3e29-4e21-b186-2a7c202abb46)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://people.redhat.com/~jolawren/klp-compiler-notes/livepatch/compiler-considerations.html



## <span class="section-num">1</span> Interprocedural optimization {#h:4fd016cf-5c6d-455b-9d00-b939b023b4b6}

函数内联可能是影响动态修补最常见的编译器优化。在一个简单的例子中，内联将原始代码转换为：

```c
foo() { ... [ foo implementation ] ... }
bar() { ... foo() ... }
```

变为：

```c
bar() { ... [ foo implementation ] ... }
```

内联类似于宏展开，然而编译器可能会内联它认为值得的情况（同时保留其他情况下的原始调用/返回语义），甚至部分内联函数的部分（见下文“GCC 函数后缀”中的冷函数）。

要安全地对上述示例中的 foo()进行动态修补，需要考虑到所有调用它的地方。对于那些编译器已经内联了 foo()的调用者，动态修补应该包含一个调用新的被修补内联函数版本的调用函数的新版本，使得它：

1.  调用内联函数的新的被修补版本，或者
2.  提供调用者的更新版本，其中包含其自己的内联并更新的内联函数版本


## <span class="section-num">2</span> GCC function suffixes {#h:548d6856-3e29-4e21-b186-2a7c202abb46}

GCC 可能会根据应用的优化将原始、复制和克隆函数进行重命名。以下是编译器可能应用于内核函数的名称后缀的部分列表：

-   `冷子函数(Cold subfunctions)` ： `.code` 或者 `.cold.<N>` ：<br />
    由属性或优化确定不太可能执行的函数（子函数）的部分。

    例如， `irq_do_set_affinity()` 的不太可能执行的部分可能移出到子函数
    `irq_do_set_affinity.cold.49()` 。从 GCC 9 开始，编号后缀已被移除。因此，在上一个例子中，冷子函数简单地为 `irq_do_set_affinity.cold()` 。

-   `部分内:联` `.part.<N>` ：<br />
    从原始函数主体中分离出来的函数部分，在整体内联决策上有所改进。 函数 `cdev_put()` 提供了一个有趣的部分克隆的例子。GCC 会将源函数：
    ```c
    void cdev_put(struct cdev *p)
    {
            if (p) {
                    struct module *owner = p->owner;
                    kobject_put(&p->kobj);
                    module_put(owner);
            }
    }
    ```
    拆成两个函数： `cdev_put.part.0()` 为部分内联， `cdev_put` 会调用 `cdev_put.part.0()`

将 `cdev_put()` 中的条件测试和 `cdev_put.part.0()` 中的 `kobject_put()` 和 `module_put()` 调用分成两个函数：

```asm
<cdev_put>:
       e8 bb 60 73 00          callq  ffffffff81a01a10 <__fentry__>
       48 85 ff                test   %rdi,%rdi
       74 05                   je     ffffffff812cb95f <cdev_put+0xf>
       e9 a1 fc ff ff          jmpq   ffffffff812cb600 <cdev_put.part.0>
       c3                      retq
<cdev_put.part.0>:
       e8 0b 64 73 00          callq  ffffffff81a01a10 <__fentry__>
       53                      push   %rbx
       48 8b 5f 60             mov    0x60(%rdi),%rbx
       e8 a1 54 5a 00          callq  ffffffff81870ab0 <kobject_put>
       48 89 df                mov    %rbx,%rdi
       5b                      pop    %rbx
       e9 b8 5c e8 ff          jmpq   ffffffff811512d0 <module_put>
       0f 1f 84 00 00 00 00    nopl   0x0(%rax,%rax,1)
       00
```

-   `常量传播 (Constant propagation)` : `.constprop.<N>` : <br />
    function copies to enable constant propagation when conflicting arguments  exist.

    For example, consider `cpumask_weight()` and its copies for
    `cpumask_weight(cpu_possible_mask)` and `cpumask_weight (__cpu_online_mask)`.

    Note how the .constprop copies implicitly assign the function parameter:

<!--listend-->

```asm
<cpumask_weight>:
       8b 35 1e 7d 3e 01       mov    0x13e7d1e(%rip),%esi
       e9 55 6e 3f 00          jmpq   ffffffff8141d2b0 <__bitmap_weight>
<cpumask_weight.constprop.28>:
       8b 35 79 cf 1c 01       mov    0x11ccf79(%rip),%esi
       48 c7 c7 80 db 40 82    mov    $0xffffffff8240db80,%rdi
                         R_X86_64_32S  __cpu_possible_mask
       e9 a9 c0 1d 00          jmpq   ffffffff8141d2b0 <__bitmap_weight>
<cpumask_weight.constprop.108>:
       8b 35 de 69 32 01       mov    0x13269de(%rip),%esi
       48 c7 c7 80 d7 40 82    mov    $0xffffffff8240d780,%rdi
                         R_X86_64_32S  __cpu_online_mask
       e9 0e 5b 33 00          jmpq   ffffffff8141d2b0 <__bitmap_weight>
```

-   IPA-SRA : .isra.0 : TODO

