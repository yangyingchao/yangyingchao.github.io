# SQLite: Past, Present, and Future


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [INTRODUCTION](#h:bbf2c939-edb3-4108-bbc0-51bcd7e678b5)
- <span class="section-num">2</span> [ARCHITECTURE](#h:a745cd6c-c855-4878-9e9b-0a4ade66be1a)
    - <span class="section-num">2.1</span> [Modules](#h:b330e68c-83ff-4a06-a693-0aae09fdbfec)

</div>
<!--endtoc-->


本文为摘录，原文为： attachments/pdf/2/p3535-gaffney.pdf



## <span class="section-num">1</span> INTRODUCTION {#h:bbf2c939-edb3-4108-bbc0-51bcd7e678b5}

-   SQLite is embedded in the process of the host application
-   Instead of communicating with a database server across process boundaries,
    applications manage a SQLite database by calling SQLite library functions

-   several characteristics combine to make SQLite useful in a broad range of scenario:
    -   Cross-platform
        -   数据库保存在单一文件中，文件可以随意拷贝，兼容：
            -   32位/64位
            -   大端，小端架构
        -   SQLite 可以运行在很多类型的机器上

    -   Compact and self-contained
        -   sqlite 库可以从一个单一 C 文件编译
        -   库很小 `< 570KiB`
        -   倚赖很少
        -   无需额外安装和配置

    -   Reliable
        -   每一行 SQLite 的代码，有超过 600 行的测试代码
        -   测试涵盖了所有分支

    -   Fast


## <span class="section-num">2</span> ARCHITECTURE {#h:a745cd6c-c855-4878-9e9b-0a4ade66be1a}


### <span class="section-num">2.1</span> Modules {#h:b330e68c-83ff-4a06-a693-0aae09fdbfec}

如[下图](#figure--fig:screenshot@2022-10-21-17:53:09) 所示， SQLite 采用模块化设计，模块可以分成 4 组。

<a id="figure--fig:screenshot@2022-10-21-17:53:09"></a>

{{< figure src="/ox-hugo/screenshot@2022-10-21_17:53:09.png" >}}


#### <span class="section-num">2.1.1</span> SQL compiler modules {#h:53047711-ae68-4caf-830b-6b072b565ff8}

-   负责将 SQL 表达式转换成位可以被虚拟机执行的二进制代码
-   神奇的思路，
-   类似汇编代码。。。


#### <span class="section-num">2.1.2</span> Core modules {#h:35e9f215-5660-44f7-85a3-a62b546bf010}

-   负责接收和执行 SQL
-   执行引擎可视为虚拟机， 又称 Virtual Database Engine, VDBE
-   VDBE 执行 Compiler 模块生成的二进制程序

