# Performance Tip of the Week


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [#9: Optimizations past their prime](#h:3349e37f-2402-49e1-a891-9f1849eee5a8)
    - <span class="section-num">1.1</span> [Best practices](#h:4353c6fb-3ca3-4d32-b745-11b1fbcb7005)
    - <span class="section-num">1.2</span> [#21: Improving the efficiency of your regular expressions](#h:b840fb72-5992-44a3-ad54-225b893a0b03)

</div>
<!--endtoc-->


本文为摘录，原文为： https://abseil.io/fast



## <span class="section-num">1</span> #9: Optimizations past their prime {#h:3349e37f-2402-49e1-a891-9f1849eee5a8}


### <span class="section-num">1.1</span> Best practices {#h:4353c6fb-3ca3-4d32-b745-11b1fbcb7005}

– 可能的情况下编写清晰、惯用的代码 <br />
  Prefer writing clear, idiomatic code whenever possible.  It is not only easier to read
  and debug, but in the long run, also easier for the compiler to optimize.

这不仅易于阅读和调试，而且从长远来看，也更容易让编译器进行优化。

– 倾向使用编译器提供的函数： <br />
  Whenever you find a low-level performance optimization that requires fancy
  bit-twiddling, intrinsics code, or inline assembly, consider first whether this is
  something the compiler could do.

当你发现需要使用复杂的位运算、内嵌代码或内联汇编来进行底层性能优化时，请首先考虑这是否是编译器可以
完成的工作。

– If the code is hot, and the optimization is not something the compiler can be taught
  to perform, then: prefer portable code, possibly using hwy to generate efficient and
  portable vector code, failing that use intrinsics, failing that use inline asm (this
  should be extremely rare). Avoiding inline assembly makes the code more portable
  across microarchitectures.

如果代码运行频繁，并且优化不是编译器可以完成的工作， 有多种可选择的实现时，考虑下面的顺序：

-   请优先使用可移植的代码，（可能使用  hwy 生成高效且可移植的向量代码）
-   如果不行则使用编译器内部函数 (intrinsics) ，
-   最后使用内联汇编（这应该是极为罕见的情况）。
    -   避免使用内联汇编可使代码在微体系结构之间更易移植。

– Keep the “naive” code you are replacing. <br />
  If you are optimizing ComputeFoo, consider keeping the simple implementation in a
  REFERENCE_ComputeFoo function. This:

-   makes it easy to write a unit-test for the new implementation that ensures the two
    functions are equivalent;
-   makes it easier to write a microbenchmark; and it makes it easier to revert to the
    reference code when (not if) the machine-dependent implementation outlives its
    usefulness.

保留你要替换的“朴素”代码。如果你正在优化 ComputeFoo，考虑将简单的实现保留在
REFERENCE_ComputeFoo 函数中。这使得编写单元测试来确保两个函数是等效的变得容易；它使编
写微基准测试变得更容易；它也使得当（而不是如果）与机器相关的实现过时时，更容易恢复到参
考代码。

– Include a microbenchmark with your change. <br />
  在你的更改中包含一个微基准测试。

– When designing or changing configuration knobs, ensure that the choices stay optimal
  over time. Frequently, overriding the default can lead to suboptimal behavior when the
  default changes by pinning things in a worse-than-out-of-the-box state. Designing the
  knobs in terms of the outcome rather than specific behavior aspects can make such
  overrides easier (or even possible) to evolve.

在设计或更改配置选项时，请确保选择始终保持最优状态。通常，覆盖默认设置可能会导致次优行
为，因为将事物钉在比开箱即用更差的状态。将选项设计为以结果为基础而不是特定行为方面，可
以使这些覆盖更易于演变甚至成为可能。


### <span class="section-num">1.2</span> #21: Improving the efficiency of your regular expressions {#h:b840fb72-5992-44a3-ad54-225b893a0b03}


#### <span class="section-num">1.2.1</span> Using regular expressions: a representative sample {#h:7505eba6-e079-4ae5-9a54-b62074cc1450}


#### <span class="section-num">1.2.2</span> Writing more efficient code {#h:eee0025f-e714-4baf-a3fc-f789edc9919b}


#### <span class="section-num">1.2.3</span> Writing more efficient regular expressions {#h:29739270-c76a-4e80-b8b7-bbafb535e4d2}

