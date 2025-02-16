# What is the current time around the world? Utilizing std::chrono with time zones in C++23


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Improving the print](#improving-the-print)
- <span class="section-num">2</span> [Getting time in my current time zone 获取我当前时区的时间](#getting-time-in-my-current-time-zone-获取我当前时区的时间)
- <span class="section-num">3</span> [Daylight saving time 夏令时](#daylight-saving-time-夏令时)
- <span class="section-num">4</span> [Summary](#summary)

</div>
<!--endtoc-->


本文为摘录，原文为： https://www.cppstories.com/2024/chrono_dates_zones/

在这篇博客文章中，我们将探讨如何使用 std::chrono 处理日期，包括时区。我们将利用该库的最新特性来检索各个时区的当前时间，同时考虑夏令时的变化。此外，我们还将结合 C++23 中引入的新功能，例如增强的打印函数等。

让我们从以下代码开始，它将打印当前的日期和时间：

```C++
#include <chrono>
#include <print>

int main() {
    auto now = std::chrono::system_clock::now();
    std::print("now is {}", now);
}
```

在我的会话中，我得到以下结果：

```text
Now is 2024-11-01 11:44:06.374573703
```

这很简单，但我们处于哪个时区？我们可以以更短的形式打印出来吗？我们如何检查世界各地的时间？夏令时呢？

第一件事：时间输出。


## <span class="section-num">1</span> Improving the print {#improving-the-print}

如您所见，我使用了 C++23 中的 `std::print` ，这在 GCC 14 中可用！默认情况下，
使用 `std::chrono::system_clock::now()` 获得的时间点会打印完整信息，
但我们可以更改并调整它。例如：

-   基本日期和时间格式设置 Basic Date and Time Formatting
    -   **仅显示日期：**  `{0:%F}` 将日期格式化为 `YYYY-MM-DD` 。
    -   **使用 hh:mm:ss 格式显示时间：** `{0:%T}` 将时间格式化为 `HH:MM:SS` 。
    -   **显示月份和/或星期几的名称：** `{0:%A, %B}` 将格式化为星期几，月份。

-   Advanced Formatting Options
    -   Year and Month:
        -   `{0:%Y}`: Full year (e.g., 2024).
        -   `{0:%y}`: Last two digits of the year (e.g., 24).
        -   `{0:%B}`: Full month name (e.g., November).
        -   `{0:%b}`: Abbreviated month name (e.g., Nov).

    -   Day and Week:
        -   `{0:%d}`: Day of the month, zero-padded (e.g., 01).
        -   `{0:%A}`: Full weekday name (e.g., Friday).
        -   `{0:%a}`: Abbreviated weekday name (e.g., Fri).

    -   Time of Day:
        -   `{0:%H}`: Hour (24-hour clock), zero-padded (e.g., 14).
        -   `{0:%I}`: Hour (12-hour clock), zero-padded (e.g., 02).
        -   `{0:%M}`: Minute, zero-padded (e.g., 05).
        -   `{0:%S}`: Second, zero-padded (e.g., 09).
        -   `{0:%p}`: AM/PM designation.

    -   Time Zone:
        -   `{0:%Z}`: Time zone abbreviation (e.g., UTC).
        -   `{0:%z}`: Offset from UTC (e.g., +0000).

Here is an example demonstrating some of these formatting options:

```C++
#include <chrono>
#include <print>
int main() {
    auto now = std::chrono::system_clock::now();
    std::print("Full date and time: {0:%Y-%m-%d %H:%M:%S}\n", now);
    std::print("Date only: {0:%F}\n", now);
    std::print("Time only: {0:%T}\n", now);
    std::print("Day of the week: {0:%A}\n", now);
    std::print("Month name: {0:%B}\n", now);
    std::print("12-hour clock with AM/PM: {0:%I:%M:%S %p}\n", now);
    std::print("ISO 8601 format: {0:%FT%T%z}\n", now);
}
```

Run @Compiler Explorer

Example output:

```text
Full date and time: 2024-11-02 19:35:16.296643881
Date only: 2024-11-02
Time only: 19:35:16.296643881
Day of the week: Saturday
Month name: November
12-hour clock with AM/PM: 07:35:16.296643881 PM
ISO 8601 format: 2024-11-02T19:35:16.296643881+0000
```


## <span class="section-num">2</span> Getting time in my current time zone 获取我当前时区的时间 {#getting-time-in-my-current-time-zone-获取我当前时区的时间}

```C++
#include <chrono>
#include <print>

int main() {
    const auto now = std::chrono::system_clock::now();
    auto zt_local = std::chrono::zoned_time{ std::chrono::current_zone(), now };
    std::print("now is {} UTC and local is: {}\n", now, zt_local);

    constexpr std::string_view Warsaw{ "Europe/Warsaw" };
    constexpr std::string_view NewYork{ "America/New_York" };
    constexpr std::string_view Tokyo{ "Asia/Tokyo" };

    try
    {
        const std::chrono::zoned_time zt_w{Warsaw, now};
        std::print("Warsaw: {0:%F} {0:%R}\n", zt_w);
        const std::chrono::zoned_time zt_ny{NewYork, now};
        std::print("New York: {0:%F} {0:%R}\n", zt_ny);
        const std::chrono::zoned_time zt_t{Tokyo, now};
        std::print("Tokyo: {0:%F} {0:%R}\n", zt_t);
    }
    catch (std::runtime_error& ex)
    {
        std::print("Error: {}", ex.what());
    }
}


```

Example output:

```text
now is 2024-11-01 19:24:51.319170338 UTC and local is: 2024-11-01 19:24:51.319170338
Warsaw: 2024-11-01 20:24
New York: 2024-11-01 15:24
Tokyo: 2024-11-02 04:24
```

让我们现在深入细节：

1.  获取当前时间：
    -   我们通过使用 `std::chrono::system_clock::now()` 来获取当前时间。
        这将给我们 UTC 的当前时间。

2.  本地时区：
    -   我们使用 `std::chrono::current_zone()` 创建一个 `std::chrono::zoned_time` 对象，
        该对象会自动检测代码运行所在系统的本地时区。这使我们能够将 UTC 时间转换为本地时区。

3.  时区转换：
    -   我们使用 std=::string_view= 定义三个时区：华沙、纽约和东京。
        这些时区是使用各自的 IANA 时区标识符进行指定的。

    -   对于每个时区，我们创建一个 `std::chrono::zoned_time` 对象，
        将当前的 UTC 时间转换为指定的时区。

4.  打印结果：
    -   我们使用 `std::print` 来显示 UTC 和本地时区的当前时间。

    -   对于每个指定的时区（华沙、纽约、东京），
        我们使用格式说明符 `{0:%F}` 打印日期（ `YYYY-MM-DD` ）和 `{0:%R}` 打印时间（ `HH:MM` ）。

5.  错误处理：
    – 代码被包裹在一个 `try-catch` 块中，以处理任何潜在的运行时错误，例如无效的时区标识符。


## <span class="section-num">3</span> Daylight saving time 夏令时 {#daylight-saving-time-夏令时}

我们甚至可以检查与我们当前时区相关的一些额外信息。
我们可以从 `zoned_time` 中获取信息并打印出开始/结束、偏移量等信息：

```cpp
#include <chrono>
#include <print>
#include <iostream>

int main() {
    try
    {
        const auto now = std::chrono::floor<std::chrono::minutes>(std::chrono::system_clock::now());
        auto zt_local = std::chrono::zoned_time{ "Europe/Warsaw", now };
        std::print("现在是 {} UTC，华沙是: {}\n", now, zt_local);

        auto info = zt_local.get_info();
        std::print("本地时间信息: \n缩写: {},\n 开始 {}, 结束 {}, \n偏移 {}, 夏令时 {}\n",
                          info.abbrev, info.begin, info.end, info.offset, info.save);
    }
    catch (std::runtime_error& ex)
    {
        std::print("错误: {}", ex.what());
    }
}
```

和夏令时信息：

```cpp
#include <chrono>
#include <print>

void printInfo(std::chrono::sys_days sd, std::string_view zone) {
    auto zt_local = std::chrono::zoned_time{ "Europe/Warsaw", std::chrono::sys_days{sd} };
    auto info = zt_local.get_info();
    std::print("在 {:%F} 的 {} 的时间信息:\n缩写: {},\n开始 {}, 结束 {}, \n偏移 {}, 夏令时 {}\n",
                sd, zone, info.abbrev, info.begin, info.end, info.offset, info.save);
}

int main() {
    try
    {
        printInfo(std::chrono::year{ 2024 } / 9 / 14, "Europe/Warsaw");
        printInfo(std::chrono::year{ 2024 } / 11 / 14, "Europe/Warsaw");
    }
    catch (std::runtime_error& ex)
    {
        std::print("错误: {}", ex.what());
    }
}
```

我得到了：

```text
在 Europe/Warsaw 的时间信息 2024-09-14:
缩写: CEST,
开始 2024-03-31 01:00:00, 结束 2024-10-27 01:00:00,
偏移 7200s, 夏令时 60min
在 Europe/Warsaw 的时间信息 2024-11-14:
缩写: CET,
开始 2024-10-27 01:00:00, 结束 2025-03-30 01:00:00,
偏移 3600s, 夏令时 0min
```

如您所见，我们获得了以下信息：

– info.abbrev - 时区缩写名，例如 CET、PT 等
– info.begin, info.end - 给定时区的开始和结束日期（以 sys_seconds 为单位），通常在夏令时转换时会发生变化
– info.offset - 相对于 UTC 时间的偏移量
– info.save - 如果不为零，则表示时区处于夏令时


## <span class="section-num">4</span> Summary {#summary}

这真是一趟旅程！我们从简单的当前时间显示开始，逐渐过渡到更复杂的场景，包括 C++20 和 C++23 中提供的各种格式选项，
以及时区。最后，我们探讨了如何检查特定时间点的细节，并检查与其时区相关的信息。

