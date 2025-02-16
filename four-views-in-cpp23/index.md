# Spans, string_view, and Ranges - Four View types (C++17 to C++23) - C++ Stories


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [String View (C++17) SVG Image](#h:fac5fec9-45cc-4f3d-852b-d06a6b989983)
    - <span class="section-num">1.1</span> [`string_view` vs `const string &`](#h:b9c83c11-bb0b-4c53-83e8-cefa5214d4c2)
- <span class="section-num">2</span> [Span (C++20)](#h:e386b658-db23-445a-ab6a-06bb367950ac)
- <span class="section-num">3</span> [Range Views (C++20)](#h:faf47418-f580-4b64-9f0d-16b18af5671e)
- <span class="section-num">4</span> [MD Span (C++23)](#h:ae160436-19fe-4900-bddf-a4c4e00ccebc)
- <span class="section-num">5</span> [Comparing Spans to Range Views](#h:5ebc8985-1155-4cf3-a4ac-588b2b331ebe)
- <span class="section-num">6</span> [Summary](#h:adcb85b9-4fa7-45fa-860b-9c99c7000414)

</div>
<!--endtoc-->


本文为摘录，原文为： https://www.cppstories.com/2023/four-views-in-cpp23/

在本博客文章中，我们将探讨现代 C++中引入的几种不同的视图/引用类型。第一种是 C++17 中新增的 `string_view` 。
C++20 引入了 `std::span` 和 `ranges` 视图。最后新增的是来自 C++23 的 `std::mdspan` 。


## <span class="section-num">1</span> String View (C++17) SVG Image {#h:fac5fec9-45cc-4f3d-852b-d06a6b989983}

-   `std::string_view` 是一个指向字符串的非拥有引用。
-   提供了一种面向对象的方式来表示字符串和子字符串，
-   不需要复制或分配 std::string 带来的开销。
-   特别适用于需要临时视图的场景，大大提高了字符串处理代码的性能和表达能力。
-   视图对象不允许修改原始字符串中的字符。

以下是一个基本的示例：

```c++
#include <format>
#include <iostream>
#include <string_view>
void find_word(std::string_view text, std::string_view word) {
    size_t pos = text.find(word);
    if (pos != std::string_view::npos)
        std::cout << std::format("单词在位置{}处找到\n", pos);
    else
        std::cout << "单词未找到\n";
}
int main() {
    std::string str = "The quick brown fox jumps over the lazy dog";
    std::string_view sv = str;
    find_word(sv, "quick");
    find_word(sv, "lazy");
    find_word(sv, "hello");
}
```


### <span class="section-num">1.1</span> `string_view` vs `const string &` {#h:b9c83c11-bb0b-4c53-83e8-cefa5214d4c2}

`std::string const&` requires the data to be in a std::string, and not a raw C array, a
`char const*` returned by a C API, a `std::vector<char>` produced by some deserialization
engine, etc. The avoided format conversion avoids copying bytes, and (if the string is
longer than the SBO¹ for the particular `std::string` implementation) avoids a memory
allocation.

`std::string const&` 要求数据在 `std::string` 中，而不是在原始的 C 数组，由 C API 返回的
`char const*=，由某些反序列化引擎产生的 =std::vector<char>` 等中。避免格式转换可以避免字节的
复制，并且（如果字符串比特定的 `std::string` 实现的 SBO¹更长）还可以避免内存分配。


## <span class="section-num">2</span> Span (C++20) {#h:e386b658-db23-445a-ab6a-06bb367950ac}

C++20 将 `string_view` 进一步通过 `std::span` 类型实现更通用。它作为对一系列连续对象的非
拥有、轻量级引用。它在需要处理数组或向量的一部分而无需拥有数据所有权的函数中特别有用。

```c++
#include <iostream>
#include <span>
#include <vector>

void print(std::span<const int> sp) {
    for (int i : sp)
        std::cout << i << " ";
    std::cout << '\n';
}

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    std::span sp(arr, 5);
    std::vector nums { 10, 11, 12, 13 };

    print(sp);
    print(nums);
}
```


## <span class="section-num">3</span> Range Views (C++20) {#h:faf47418-f580-4b64-9f0d-16b18af5671e}

C++20 中与 `std::span` 同时引入的 Range Views（区间视图）为处理数据范围提供了高级可组合
的抽象。Range Views 实现了惰性求值 (lazy evaluation)，这允许操作（如过滤、转换或连接等）
以可读、声明式的方式表示，而不会立即求值，通常会提高性能表现。

```c++
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector nums = {1, 2, 3, 4, 5, 6};

    auto even_nums = nums | std::views::filter([](int n) {
        return n % 2 == 0;
    });

    for (int n : even_nums)
        std::cout << n << " ";
}

```


## <span class="section-num">4</span> MD Span (C++23) {#h:ae160436-19fe-4900-bddf-a4c4e00ccebc}

C++23 中的 `std::mdspan` 类型是一个多维度的 span。它将 span 的概念扩展到多维数据，这在数
值和科学计算中非常有价值。在这个 C++标准中，我们还对[]运算符进行了更改，我们可以编写
mat[i,j]来访问其元素（请参见多维子脚本运算符-P2128 中的更改）。

下面，您可以找到一个简单的 mdspan 示例，用于测试给定矩阵是否是方形和单位矩阵：

```c++
#include <vector>
#include <https://raw.githubusercontent.com/kokkos/mdspan/single-header/mdspan.hpp>
#include <algorithm>
#include <iostream>

int isIdentity(std::mdspan<int, std::extents<int, std::dynamic_extent, std::dynamic_extent>> matrix) {
    int rows = matrix.extent(0);
    int cols = matrix.extent(1);

    if (rows != cols)
        return false;

    for (int i = 0; i < rows; ++i)
        if (matrix[i, i] != 1)
            return false;

    return true;
}

int main() {
    std::vector<int> matrix_data = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    std::mdspan matrix = std::mdspan(matrix_data.data(), 3, 3);
    std::cout << isIdentity(matrix) << '\n';

    std::mdspan matrix2 = std::mdspan(matrix_data.data(), 3, 2);
    std::cout << isIdentity(matrix2) << '\n';
}

```


## <span class="section-num">5</span> Comparing Spans to Range Views {#h:5ebc8985-1155-4cf3-a4ac-588b2b331ebe}

尽管 std::span 和 Range Views 都是用于操作序列的现代机制，但它们有着不同的目的。

-   `std::span` 旨在提供对连续对象序列的直接、有限和安全的访问。它简单而又强大，适用于需要
    访问数据块但不需要所有权语义的功能。

-   `Range Views` 旨在操作和组合序列。它们不要求数据是连续的，允许在范围上进行延迟、串联的
    操作，这可以导致更富有表现力的代码和潜在的更好性能。

实质上，std::span 是一种用于安全的、连续数据访问的简单工具，而 Range Views 则提供了一种
更高级的、功能性的容器操作方法。


## <span class="section-num">6</span> Summary {#h:adcb85b9-4fa7-45fa-860b-9c99c7000414}

在这篇短文中，我们回顾了最近的 C++ 修订，并了解了视图类型背后的一些基础知识。在 C++ 中，我
们有三种不同的引用/视图对象类型：

-   C++17 引入了 `string_view` ，用于高效的字符串处理，只能进行只读访问，
-   C++20 中的 `std::span` 允许处理数据序列并公开读或写访问权限，
-   C++23 中的 `std::mdspan` 是 span 的多维版本。

此外，作为最后一种类型，我提到了范围视图，它是对集合的高级抽象。虽然其他三种需要连续序列，但范围视图更加灵活。

下面是一个简单的表格，总结了上述内容：

{{< figure src="/ox-hugo/fourviews_table.png" width="800px" >}}

-   (\*) 尽管你可以通过视图进行写入，但这可能不是最佳方法。
-   (\*) mdspan 还提供了一些关于内存布局的额外选项，并且还可以使用步长参数。

在这个文本中，我还展示了一些基本的示例，但请记住这只是冰山一角，并且请继续关注更多关于这些方便类型的文章。

<https://www.cppstories.com/2023/four-views-in-cpp23/>

