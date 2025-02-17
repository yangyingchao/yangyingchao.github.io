# What it means when you convert between different shared_ptrs - The Old New Thing


本文为摘录(或转载)，侵删，原文为： https://devblogs.microsoft.com/oldnewthing/20230817-00/?p=108611

C++中的 `shared_ptr` 管理一个引用计数指针。通常情况下，它是一个指向对象的指针，当最后一个引用过期时将被删除。但它不一定非得是这样。

回想一下， `shared_ptr` 实际上是两个指针。

-   指向一个控制块 (control block)的指针，该控制块管理共享和弱引用计数，并在共享引用计数降至零时销毁一个对象（通常称为被管理的对象）。
-   从 `get()` 方法返回的指针，通常称为存储指针。

大多数情况下，存储的指针指向托管对象，因为当你使用原始指针构造 `shared_ptr` 或者调用
`make_shared` 时，得到的就是托管对象。但是，当托管对象和存储的指针不同的时候，有什么用处呢？

可能会希望有一个 `shared_ptr` ，其 `get()` 方法返回另一个大对象的子对象的指针。在这种情况下，托管对象是较大的对象，而存储的指针是指向子对象的指针。

```c++
struct Sample {
  int value1;
  int value2;
};

void consume(std::shared_ptr<int> pint);

std::shared_ptr<Sample> p = std::make_shared<Sample>();
consume(std::shared_ptr<int>(p, &p->value1));

// 或者更简洁地
auto p = std::make_shared<Sample>();
consume({p, &p->value1});
```

在上面的例子中，我们有一个名为 Sample 的类，其中有两个成员。我们创建了一个 `shared_ptr`
指向该类，并将其保存在 p 中。但是假设还有另一个函数需要一个 `shared_ptr<int>` 。没问题，我们可以通过重新使用控制块（参数 p）并替换新的存储指针（参数 `&p->value1` ）来将
`std::shared_ptr<Sample>` 转换为 `std::shared_ptr<int>` 。consume 函数可以使用
`shared_ptr<int>` 访问 value1 成员，并且该 `shared_ptr<int>` 的控制块防止了 Sample 被销毁，从而防止了 value1 被销毁。

这里使用了 C++-20 新增的构造函数 &#x2013; **别名构造函数** ：

```c++
shared_ptr( std::nullptr_t ptr, Deleter d, Alloc alloc );
template< class Y >   										(8)
shared_ptr( const shared_ptr<Y>& r, element_type* ptr ) noexcept;
template< class Y >   										(8)   (since C++20)
shared_ptr( shared_ptr<Y>&& r, element_type* ptr ) noexcept;
```

1.  The aliasing constructor: constructs a shared_ptr which shares ownership information with the initial value of r, but

holds an unrelated and unmanaged pointer ptr. If this shared_ptr is the last of the group to go out of scope, it will
call the stored deleter for the object originally managed by r. However, calling get() on this shared_ptr will always
return a copy of ptr. It is the responsibility of the programmer to make sure that this ptr remains valid as long as
this shared_ptr exists, such as in the typical use cases where ptr is a member of the object managed by r or is an alias
(e.g., downcast) of r.get() For the second overload taking an rvalue, r is empty and r.get() == nullptr after the call.
(since C++20)

1.  **别名构造函数：** 构造一个 `shared_ptr` ，与初始值 r 共享所有权信息，但持有一个无关且未被管理的指针 ptr。如果这个 `shared_ptr` 是组中最后一个超出作用域的对象，它将调用 r
    最初管理的对象的存储删除器。然而，在这个 `shared_ptr` 上调用 get()将始终返回 ptr 的拷贝。程序员负责确保在 `shared_ptr` 存在期间 ptr 保持有效，例如，在 ptr 是 r 所管理的对象的成员或者是 r.get() 的别名（例如下转型）的典型用例中。对于接受 rvalue 的第二个重载函数，调用之后 r 为空，r.get() == nullptr。（自 C++20 起）

通常情况下，储存的指针的生命周期应该包含在管理对象的生命周期内。这可以是一个直接的包含关系，就像我们在 value1 中所做的那样，也可以是一个更复杂的生命周期依赖链。

```c++
struct Other {
  int value;
};
struct Sample2 {
  const std::unique_ptr<Other> m_other = std::make_unique<Other>();
};

auto p = std::make_shared<Sample2>();
consume({p, &p->m_other->value});
```

在第二个例子中，我们传递给 consume()函数的 shared_ptr&lt;int&gt;的存储指针指向 Sample2 对象内部的
value 成员，而 Sample2 对象包含一个独有指针指向 Other 对象。该 shared_ptr&lt;int&gt;中的控制块控制着
Sample2 对象的生命周期，这是可以接受的，因为只要 Sample2 对象存活，Other 对象中的值也将保持存活。

现在，编译器不会检查您是否具有从托管对象到存储指针的正向生命周期控制链。您可以做一些愚蠢的事情，比如

```c++
struct Sample3
{
    std::unique_ptr<Other> m_other = std::make_unique<Other>();
};

auto p = std::make_shared<Sample3>();
consume({ p, &p->m_other->value });

p->m_other = nullptr; // 糟糕，链条断了
```

`shared_ptr<int>` 将认为它仍在保持值的活动状态，即使您断开了从 Sample3 到 Other 的链接。

或者你可以做更愚蠢的事情，比如

```c++
int unrelated;
consume({ p, &unrelated });
```

shared_ptr&lt;int&gt; 将访问 unrelated，即使它的生命周期与 Sample2 无关。如果 unrelated 被销毁，shared_ptr&lt;int&gt; 将具有一个悬空的存储指针。

这些其中托管对象与所指对象不同的 shared_ptr 对象通常被称为别名共享指针。

好的，所以我展示了一种创建别名共享指针的方法，即通过从现有的 shared_ptr 构造一个
shared_ptr（共享托管对象），并提供一个不同的存储指针。如果新的存储指针指向原始对象的基类，则 shared_ptr 具有一个转换运算符，用于创建对基类子对象的别名共享指针。

```c++
struct Base
{
};
struct Derived : Base
{
};
std::shared_ptr<Derived> p = std::make_shared<Derived>();
std::shared_ptr<Base> b = p; // 自动转换等效于
std::shared_ptr<Base> b(p, p.get());
```

如果要进行反向转换（从 Base 到 Derived），可以显式写出：

```c++
std::shared_ptr<Derived> b(p, static_cast<Derived/>(p.get()));
```

当然，这需要存储的 Base 指针确实是较大的 Derived 对象的 Base 部分的指针。

C++ 语言提供了一些助手函数，通过对另一个 `shared_ptr` 的存储指针进行转换来构造一个 `shared_ptr` 。

<a id="figure--fig:screenshot@2023-08-18-09:37:06"></a>

{{< figure src="/ox-hugo/screenshot@2023-08-18_09:37:06.png" width="800px" >}}

一切看起来都很好，直到我们看到 `dynamic_pointer_cast` ，它与使用 `dynamic_cast` 的一行代码不等价！

原因是，与其他转换不同， `dynamic_cast` 可以将非空指针更改为空指针，这在运行时类型不匹配时发生。在这种情况下， `dynamic_pointer_case` 返回一个空的 `shared_ptr` （而不是一个具有控制块但没有存储指针的 `shared_ptr` ），因为没有需要扩展生命周期的对象。

现在我们可以完成该表格：

<a id="figure--fig:screenshot@2023-08-18-09:38:00"></a>

{{< figure src="/ox-hugo/screenshot@2023-08-18_09:38:00.png" caption="<span class=\"figure-number\">Figure 2: </span>最终转换表格" width="800px" >}}

关于空指针的控制块的这种细节，说明了共享指针图中两个框在技术上是合法但奇怪的。

<a id="figure--fig:screenshot@2023-08-18-09:39:32"></a>

{{< figure src="/ox-hugo/screenshot@2023-08-18_09:39:32.png" >}}

到目前为止，我们已经处理了空的 `shared_ptr` （不管理对象且没有存储指针）和完整的
`shared_ptr` （管理对象且具有存储指针）。但还有另外两个框，我将它们命名为“神秘”和“放纵”。下次我们将看看这两个奇怪的家伙。

