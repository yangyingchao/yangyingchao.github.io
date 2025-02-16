# What is the difference between elfutils and binutils


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [What is the difference between elfutils and binutils](#h:443ee8f8-59ff-491c-bcdd-72574d1de216)
- <span class="section-num">2</span> [eu-stack vs gdb](#h:bf05ca98-b7e2-484f-ad03-49782c53bd91)
- <span class="section-num">3</span> [eu-strip vs strip](#h:0c4a6f39-b67b-4eff-bda5-a6875530cb81)
- <span class="section-num">4</span> [eu-addr2line vs addr2line](#h:ee772e2d-f68c-4687-a4a8-f37b34dd8926)

</div>
<!--endtoc-->


本文为摘录，原文为： https://lynxbee.com/what-is-the-difference-between-elfutils-and-binutils/#:~:text=Ulrich%20Drepper%20who%20authored%20elfutils%20has%20mentioned%20the,available%20in%20binutils%20which%20are%20in%20%28wide%29%20use



## <span class="section-num">1</span> What is the difference between elfutils and binutils {#h:443ee8f8-59ff-491c-bcdd-72574d1de216}

根据维基百科的介绍，elfutils 被描述为“Ulrich Drepper 编写了 elfutils，以取代 GNU Binutils，专门为 Linux 开发，并仅支持 ELF 和 DWARF 格式”。

elfutils 是一个红帽项目。

elfutils（如 eu-addr2line、eu-elflint、eu-findtextrel、eu-nm、eu-readelf、eu-size、eu-strip 等）与 binutils（addr2line、nm、readelf、size、strip、objdump 等）有相当大的重叠。

elfutils 的作者 Ulrich Drepper 提到了以下差异，

-   与 binutils 相比，elfutils 工具
    -   体积更小
    -   速度更快（通常是数倍）
    -   更少的错误
    -   功能更丰富
-   一些 binutils 中没有的工具被广泛使用
    -   还有一些工具使用了 elfutils 库（如 systemtap、frysk 等）


## <span class="section-num">2</span> eu-stack vs gdb {#h:bf05ca98-b7e2-484f-ad03-49782c53bd91}

`eu-stack` 仅用作显示可执行文件或者 core 文件的堆栈，速度非常快。 gdb 是一个完整的调试器，速度相对慢。


## <span class="section-num">3</span> eu-strip vs strip {#h:0c4a6f39-b67b-4eff-bda5-a6875530cb81}

`eu-strip` 比 `strip` 更灵活，可以将抽符号表与将符号表保存一步完成：

```sh
eu-strip a.out -f a.debug
```

以上命令将 a.out 中的符号表段和调试信息段都移出到 a.debug 文件中。这样，a.out 的 size 会减小很多。
而此时，a.out 中会多一个 `.gnu_debuglink` 段，它是用来保存符号表位置的。
之后，再用 gdb 去打开并运行 a.out 时，gdb 还可以找到 a.debug 这样的符号表及调试信息文件。

使用 `strip` 也能完成同样的任务，但是需要配合名 `objcopy` 命令，需要多个步骤完成：

```sh
# test.debug 将包含调试信息和符号表； 而test将只包含调试信息
objcopy --only-keep-debug test test.debug

# 从test文件里剥离debug段
objcopy --strip-debug test

# 更彻底地，上面这句可以换成下面这句以移除所有的debug信息和符号表
strip -s test

# 在二进制文件 test 中添加 .gnu_debuglink 段以指向符号表和调试信息文件
objcopy --add-gnu-debuglink=test.debug test

# objdump 命令可以查看指定的section
objdump -s -j .gnu_debuglink test
```


## <span class="section-num">4</span> eu-addr2line vs addr2line {#h:ee772e2d-f68c-4687-a4a8-f37b34dd8926}

前者速度飞快。。。

