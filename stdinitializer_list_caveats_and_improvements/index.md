# std::initializer_list in C++


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Referencing local array](#h:bfb895f8-a465-4cb7-a534-ad7148fe328d)
- <span class="section-num">2</span> [The cost of copying elements](#h:e1ac2da6-09b6-49cc-95eb-c30b134cd45c)

</div>
<!--endtoc-->


本文为摘录，原文为： https://www.cppstories.com/2023/initializer_list_improvements/



## <span class="section-num">1</span> [Referencing local array](https://www.cppstories.com/2023/initializer_list_improvements/#1-referencing-local-array) {#h:bfb895f8-a465-4cb7-a534-ad7148fe328d}

`std::initializer_list` 在使用中会转化成为常量对象的匿名局部数组：

```c++
std::initializer_list<int> wrong() { // for illustration only!
    return { 1, 2, 3, 4};
}
int main() {
    std::initializer_list<int> x = wrong();
}

```

The above code is equivalent to the following:

```c++
std::initializer_list<int> wrong() {
    const int arr[] { 1, 2, 3, 4}
    return std::initializer_list<int>{arr, arr+4};
}
int main() {
    std::initializer_list<int> x = wrong();
}
```

如果尝试编译的话，会出错：

```text
GCC:
warning: returning temporary 'initializer_list' does not extend the lifetime of the underlying array [-Winit-list-lifetime]
    5 |     return { 1, 2, 3, 4};

Or in Clang:

<source>:5:12: warning: returning address of local temporary object [-Wreturn-stack-address]
    return { 1, 2, 3, 4};
```

**结论：** <br />

-   `std::initializer_list` 是一种 “view” type
-   他引用一个局部的常量数组 it references some implementation-dependent and a local array of const values.
-   Use it mainly for passing into functions when you need a variable number of arguments of the same type.
-   If you try to return such lists and pass them around, then you risk lifetime issues


## <span class="section-num">2</span> [The cost of copying elements](https://www.cppstories.com/2023/initializer_list_improvements/#2-the-cost-of-copying-elements) {#h:e1ac2da6-09b6-49cc-95eb-c30b134cd45c}

通过初始化列表来传递参数很方便，但最好知道，当向 vector 中传递对象时候，vector 中的每个元素都会进行一次拷贝构造，应该了解其代价。

