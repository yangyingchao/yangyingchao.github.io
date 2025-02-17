# All Major C++17 Features You Should Know


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Language Features](#h:5d1ffb1b-4df4-4e37-82c6-58427dc149e4)
    - <span class="section-num">1.1</span> [New auto rules for direct-list-initialization](#h:771f9d85-5896-4a45-8ce1-1ab7861af069)
    - <span class="section-num">1.2</span> [static_assert with no message](#h:ae9aa599-ee2e-4fbf-8ac2-808e6666c693)
    - <span class="section-num">1.3</span> [typename in a template template parameter](#h:791c735d-4e0a-4e1b-bea9-592ef14dcdf6)
    - <span class="section-num">1.4</span> [Nested namespace definition](#h:e5f75c48-e26e-445d-91cd-cb819d8017c4)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.cppstories.com/2017/01/cpp17features/



## <span class="section-num">1</span> [Language Features](https://www.cppstories.com/2017/01/cpp17features/#language-features) {#h:5d1ffb1b-4df4-4e37-82c6-58427dc149e4}


### <span class="section-num">1.1</span> [New auto rules for direct-list-initialization](https://www.cppstories.com/2017/01/cpp17features/#new-auto-rules-for-direct-list-initialization) {#h:771f9d85-5896-4a45-8ce1-1ab7861af069}


### <span class="section-num">1.2</span> [static_assert with no message](https://www.cppstories.com/2017/01/cpp17features/#static_assert-with-no-message) {#h:ae9aa599-ee2e-4fbf-8ac2-808e6666c693}

`static_assert()` 可以不用再写 message 了， 类似 C `assert`


### <span class="section-num">1.3</span> [typename in a template template parameter](https://www.cppstories.com/2017/01/cpp17features/#typename-in-a-template-template-parameter) {#h:791c735d-4e0a-4e1b-bea9-592ef14dcdf6}

模板中以前只能用 `class` 来声明类型，现在可以用 `typename` 了

```c++
template <template <typename...> typename Container>
//            used to be invalid ^^^^^^^^
struct foo;

foo<std::vector> my_foo;
```


### <span class="section-num">1.4</span> [Nested namespace definition](https://www.cppstories.com/2017/01/cpp17features/#nested-namespace-definition) {#h:e5f75c48-e26e-445d-91cd-cb819d8017c4}

Allows to write:

```c++
namespace A::B::C {
   // ...
}

Rather than:

namespace A {
    namespace B {
        namespace C {
            // ...
        }
    }
}

```

