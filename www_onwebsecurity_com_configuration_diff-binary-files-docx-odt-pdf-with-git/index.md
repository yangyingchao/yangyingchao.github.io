# Diff binary files like docx, odt and pdf with git


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [给 git 添加新的文本转换处理器](#给-git-添加新的文本转换处理器)
- <span class="section-num">2</span> [指示 git 为每种文件类型应用正确的处理器](#指示-git-为每种文件类型应用正确的处理器)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.onwebsecurity.com/configuration/diff-binary-files-docx-odt-pdf-with-git.html

处理二进制文件类型，如 Microsoft Word XML 格式的文档 docx、OpenDocument 文本 odt 格式和便携式文档格式 pdf，搭配
git 使用时会遇到一些困难。默认情况下，git 仅提供文本格式的差异比较。不支持以文本格式比较二进制文件。

通过简单的配置更改和一些开源的跨平台工具，git 可以适应这些格式的差异比较。


## <span class="section-num">1</span> 给 git 添加新的文本转换处理器 {#给-git-添加新的文本转换处理器}

在安装了这两个程序和封装器之后，接下来需要告诉 git 如何将二进制文件类型转换为文本格式。可以通过修改全局 git 配置来完成：

```shell
git config --global diff.docx.textconv pandoc --to=rst
git config --global diff.odt.textconv pandoc --to=rst
git config --global diff.pdf.textconv pdftostdout
```

这为每种文件类型创建了新的差异处理器。

注意

使用参数 &#x2013;to=rst 指定 pandoc 使用 reStructuredText 格式。这使得比较层级比仅使用纯文本格式更容易。


## <span class="section-num">2</span> 指示 git 为每种文件类型应用正确的处理器 {#指示-git-为每种文件类型应用正确的处理器}

最后，git 需要知道使用哪个转换处理器来处理哪种文件类型。这可以通过修改全局 gitattributes [4] 文件来完成。

`gitattributes` 文件定义了每个路径或每个文件的属性。这意味着您可以为每种文件类型指定处理器，它将自动使用正确的工具将二进制格式转换为文本格式。

`gitattributes` 文件可以在本地（每个 git 仓库）、每个系统或全局作用域中指定。全局通常是首选选项，因为这意味着每个用户只需配置一次，在每个仓库中使用。全局 `gitattributes` 文件可以在 `$HOME/.config/git/attributes` 中找到。

**注意:**

由于全局和系统 git 属性文件具有最低的优先级，它们可以在本地轻松被覆盖。这可以通过在仓库的根目录创建 `.gitattributes` 文件来完成。

以下代码片段将为每种文件类型添加正确的转换处理器到全局 git 配置：

```shell
echo "*.docx diff=docx" >> ~/.config/git/attributes
echo "*.odt diff=odt" >> ~/.config/git/attributes
echo "*.pdf diff=pdf" >> ~/.config/git/attributes
```

这就是全部内容。现在 git diff 将以纯文本格式显示 docx、odt 和 pdf 的所有更改。

任何二进制格式都可以使用 git 进行差异比较，只要有工具将二进制格式转换为纯文本。只需以相同的方式添加转换处理器和属性即可。

[1]  <https://pandoc.org/>

[2]  <http://docutils.sourceforge.net/rst.html>

[3]  <https://poppler.freedesktop.org/>

[4]  <https://git-scm.com/docs/gitattributes>

