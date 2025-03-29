# pandoc 生成 docx 中的 list 处理


上一篇 [blog](https://yangyingchao.github.io/%E7%94%A8-pandoc-%E4%BB%8E-org-%E7%94%9F%E6%88%90-docx/) 中，介绍了怎样使用 pandoc 将 `org` 文件转换成为 docx 文件，以及用到的 filter 和 word 模板。这解决了我再实际使用过程中遇到的大部分问题，比如基本的格式、表格边框、表格图片标号之类的。但当时还有一个问题没有解决，就是列表 （list） 的显示不理想。

例如下面的 org 文件 ：

```org
#+TITLE:  test
#+AUTHOR: Yang,Ying-chao
#+DATE:   2025-03-29
#+OPTIONS:  ^:nil H:5 num:t toc:2 \n:nil ::t |:t -:t f:t *:t tex:t d:(HIDE) tags:not-in-toc
#+STARTUP:  oddeven lognotestate
#+SEQ_TODO: TODO(t) INPROGRESS(i) WAITING(w@) | DONE(d) CANCELED(c@)
#+TAGS:     noexport(n)
#+EXCLUDE_TAGS: noexport
#+FILETAGS: :tag1:tag2:

Normal text
- unordered item 1
  + unordered item 2
    * unordered item 3
  + unordered item 4


Normal text
1. ordered item 1
   1. ordered item 2
2. ordered item 3

```

生成的 word 文档截图如下：

<a id="figure--fig:pand-f6d61455"></a>

{{< figure src="/ox-hugo/pand_b763fef7.png" caption="<span class=\"figure-number\">Figure 1: </span>默认生成的列表" >}}

也不能说错，但是看着有点别扭：

-   前面的 bullets 和 word 默认的不一致
-   每行的缩进太大，也和默认不一样

花时间研究了一下，发现 word 中列表不是在 `style.xml` 中定义的，而是在 `numbering.xml` 里面，而且貌似 pandocx 在生成 docx 的时候，会自动生成 `numbering.xml` ，而不使用 `custom-rerference.docx`
 文件。

那就只好生成之后再做处理了：

-   先把生成的文件解压
-   使用自定义的 `numbering.xml` 做模板
-   再补上自动生成的部分
-   最后重新 zip 成 docx

`elisp` 代码如下:

```emacs-lisp
(file (yc/generate-script :infix "update-docx" :popup 'auto
        (insert "cd " dir "\n 7z x '" output "'
cd word
mv numbering.xml numbering_bak.xml
HEAD=head
which ghead && HEAD=ghead
${HEAD} -n -3 ~/.local/share/pandoc/custom-reference/word/numbering.xml > numbering.xml
tail -n 2 numbering_bak.xml >> numbering.xml
cd ..
zip -r '" output"' *
rm -rf " dir)))
```

完整代码可在 [这里](https://github.com/yangyingchao/tubo-env/blob/master/emacs/.emacs.d/site-lisp/utils/yc-utils.el#L1806) 找到。

修改以后的效果：

<a id="figure--fig:pand-9bec26a3"></a>

{{< figure src="/ox-hugo/pand_c06bbd6a.png" caption="<span class=\"figure-number\">Figure 2: </span>修改以后的效果" >}}

