# C++ String Conversion: Exploring std::from_chars in C++17 to C++26


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Before C++17](#before-c-plus-plus-17)
- <span class="section-num">2</span> [Converting From Characters to Numbers: from_chars](#converting-from-characters-to-numbers-from-chars)
    - <span class="section-num">2.1</span> [示例](#示例)
- <span class="section-num">3</span> [性能](#性能)
- <span class="section-num">4</span> [C++23 更新](#c-plus-plus-23-更新)
- <span class="section-num">5</span> [C++26 更新](#c-plus-plus-26-更新)
- <span class="section-num">6</span> [C++ 中对 std::from_chars 的编译器支持](#c-plus-plus-中对-std-from-chars-的编译器支持)
- <span class="section-num">7</span> [总结](#总结)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.cppstories.com/2018/12/fromchars/

随着引入 C++17，C++标准库通过添加 `std::from_chars` 来扩展了将文本转换为数字的功能。这个低级、高性能的 API 相较于以前的方法（如 atoi 和 stringstream）提供了显著的优势。在本文中，我们将探讨从 C++17 到 C++26 的字符串转换程序的演变，重点突出了诸如 constexpr 支持和改进的错误处理等关键改进。让我们深入了解细节，看看 `std::from_chars` 如何可以改变您对字符串转换的方法。


## <span class="section-num">1</span> Before C++17 {#before-c-plus-plus-17}

-   sprintf / snprintf
-   sscanf
-   atol
-   strtol
-   strstream
-   stringstream
-   to_string
-   stoi 等函数

而在 C++17 中，新增了一种选项： `std::from_chars` ！为什么需要新方法？难道旧方法不够好吗？

新的转换例程具有以下特点：

-   不抛出异常
-   不分配内存
-   不支持本地化
-   内存安全
-   错误报告提供有关转换结果的额外信息

虽然 API 可能不太友好，但很容易封装为外观模式。

一个简单的示例：

```c++
const std::string str { "12345678901234" };
int value = 0;
std::from_chars(str.data(), str.data() + str.size(), value);
// 省略了错误检查...
```


## <span class="section-num">2</span> Converting From Characters to Numbers: from_chars {#converting-from-characters-to-numbers-from-chars}

`std::from_chars` ，在 &lt;charconv&gt; 头文件中可用，是一组重载函数：用于整数类型和浮点类型。

对于整数类型，我们有以下函数：

```c++
std::from_chars_result from_chars(const char/ first,
                                  const char/ last,
                                  TYPE &value,
                                  int base = 10);
```

其中，TYPE 展开为所有可用的有符号和无符号整数类型以及 char。

base 可以是从 2 到 36 的数字。

然后是浮点数版本：

```c++
std::from_chars_result from_chars(const char/ first,
                   const char/ last,
                   FLOAT_TYPE& value,
                   std::chars_format fmt = std::chars_format::general);
```

`FLOAT_TYPE` 展开为 `float` 、 `double` 或 `long double` 。

`chars_format` 是一个枚举，包含以下值：

-   scientific、
-   fixed、
-   hex 和
-   general（是 fixed 和 scientific 的组合）。

所有这些函数（对于整数和浮点数）的返回值是 `from_chars_result` ：

```c++
struct from_chars_result {
    const char/ ptr;
    std::errc ec;
};
```

`from_chars_result` 包含有关转换过程的信息。

以下是总结：

<a id="table--tbl-data-table"></a>
<div class="table-caption">
  <span class="table-number"><a href="#table--tbl-data-table">Table 1</a>:</span>
  caption
</div>

| 返回条件 || from_chars_result 的状态                                                      |
|------|----------------------------------------------------------------------------|
| 成功 || ptr 指向第一个不匹配模式的字符，或者如果所有字符匹配则为 last 的值，并且 ec 为值初始化状态。 |
| 无效转换 || ptr 等于 first，ec 等于 std::errc::invalid_argument。value 未被修改。         |
| 超出范围 || 数字太大，无法装入值类型。ec 等于 std::errc::result_out_of_range，ptr 指向第一个不匹配的字符。value 未被修改。 |


### <span class="section-num">2.1</span> 示例 {#示例}

以下是使用 `from_chars` 将字符串转换为数字的两个示例，一个是整数一个是浮点数。


#### <span class="section-num">2.1.1</span> 整数类型 {#整数类型}

```c++
#include <charconv> // from_char, to_char
#include <string>
#include <iostream>
int main() {
    const std::string str { "12345678901234" };
    int value = 0;
    const auto res = std::from_chars(str.data(),
                                     str.data() + str.size(),
                                     value);
    if (res.ec == std::errc())
    {
        std::cout << "value: " << value
                  << ", distance: " << res.ptr - str.data() << '\n';
    }
    else if (res.ec == std::errc::invalid_argument)
    {
        std::cout << "invalid argument!\n";
    }
    else if (res.ec == std::errc::result_out_of_range)
    {
        std::cout << "out of range! res.ptr distance: "
                  << res.ptr - str.data() << '\n';
    }
}
```

这个示例很简单，它将一个字符串 str 传递给 from_chars，然后在可能的情况下显示结果并提供附加信息。

你的任务是在编译器资源管理器中运行这段代码。

"12345678901234" 能够装入这个数字吗？或者你是否看到一些来自转换 API 的错误？


#### <span class="section-num">2.1.2</span> 浮点数 {#浮点数}

这里是浮点数版本：

```c++
#include <charconv> // from_char, to_char
#include <string>
#include <iostream>
int main() {
    const std::string str { "16.78" };
    double value = 0;
    const auto format = std::chars_format::general;
    const auto res = std::from_chars(str.data(),
                                 str.data() + str.size(),
                                 value,
                                 format);
    if (res.ec == std::errc())
    {
        std::cout << "value: " << value
                  << ", distance: " << res.ptr - str.data() << '\n';
    }
    else if (res.ec == std::errc::invalid_argument)
    {
        std::cout << "invalid argument!\n";
    }
    else if (res.ec == std::errc::result_out_of_range)
    {
        std::cout << "out of range! res.ptr distance: "
                  << res.ptr - str.data() << '\n';
    }
}
```


## <span class="section-num">3</span> 性能 {#性能}

我做了一些基准测试，新例程速度非常快！

一些数字：

-   在 GCC 上，它比 stoi 快大约 4.5 倍，比 atoi 快大约 2.2 倍，比 istringstream 快近 50 倍。
-   在 Clang 上，它比 stoi 快大约 3.5 倍，比 atoi 快 2.7 倍，比 istringstream 快 60 倍！
-   MSVC 的性能比 stoi 快约 3 倍，比 atoi 快约 2 倍，比 istringstream 快近 50 倍。

你可以在我的 C++17 书籍中找到这些结果：《C++17 详细信息》。


## <span class="section-num">4</span> C++23 更新 {#c-plus-plus-23-更新}

在 C++23 中，我们为我们的函数得到了一个改进：P2291

`std::to_chars()` 和 `std::from_chars()` 的整数重载现在是 `constexpr` 。

它已经在 GCC 13、Clang 16 和 MSVC 19.34 中实现。

与 std::optional 一起，它还可以在 constexpr 上下文中工作，我们可以创建以下示例：

```c++
#include <charconv>
#include <optional>
#include <string_view>
constexpr std::optional<int> to_int(std::string_view sv)
{
    int value {};
    const auto ret = std::from_chars(sv.begin(), sv.end(), value);
    if (ret.ec == std::errc{})
        return value;
    return {};
};
int main() {
    static_assert(to_int("hello") == std::nullopt);
    static_assert(to_int("10") == 10);
}
```

在编译器资源管理器中运行


## <span class="section-num">5</span> C++26 更新 {#c-plus-plus-26-更新}

这项工作尚未完成，在 C++26 中看起来我们将有更多的添加：

查看 P2497R0，这个提案已经被接受并包含在 C++26 的工作草案中：

对 &lt;charconv&gt; 函数成功或失败的测试

这一功能已在 GCC 14 和 Clang 18 中实现。

简而言之，from_chars_result（以及 to_chars_result）获得了一个 bool 转换运算符：

```c++
constexpr explicit operator bool() const noexcept;
```

它必须返回 `ec =` std::errc{}= 。

这意味着我们的代码可能会更简单：

```c++
if (res.ec == std::errc()) { ... }
```

可以变成：

```c++
if (res) { ... }
```

例如：

```c++
// ...
const auto res = std::from_chars(str.data(),
                                 str.data() + str.size(),
                                 value,
                                 format);
if (res)
{
    std::cout << "value: " << value
              << ", distance: " << res.ptr - str.data() << '\n';
}
// ...
```


## <span class="section-num">6</span> C++ 中对 std::from_chars 的编译器支持 {#c-plus-plus-中对-std-from-chars-的编译器支持}

-   Visual Studio：<br />
    完全支持 std::from_chars 是在 Visual Studio 2019 版本 16.4 中引入的，浮点支持从 VS 2017 版本 15.7 开始。Visual Studio 2022 包含了 C++23 功能，如对整数重载的 constexpr 支持。
-   GCC：<br />
    从 GCC 11.0 开始，std::from_chars 提供完整支持，包括整数和浮点转换。最新的 GCC 版本，比如 GCC 13，包含 constexpr 整数支持。
-   Clang：<br />
    Clang 7.0 引入了对整数转换的初始支持。Clang 16 及以上支持整数重载的 constexpr。

获取最准确和最新的信息，请查看 CppReference、编译器支持。


## <span class="section-num">7</span> 总结 {#总结}

如果你想将文本转换为数字，并且不需要任何额外的功能，如区域设置支持，那么 std::from_chars 可能是最好的选择。它提供了很好的性能，而且更重要的是，你将获得关于转换过程的大量信息（例如扫描了多少个字符）。

这些例程在解析 JSON 文件、三维文本模型表示（如 OBJ 文件格式）等方面可能特别有用。

而且新函数甚至可以在编译时使用（截至 C++23），并且在 C++26 中具有更好的错误检查功能。

