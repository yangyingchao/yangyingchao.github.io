# 用 pandoc 从 org 生成 docx


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [所需软件](#所需软件)
- <span class="section-num">2</span> [方法](#方法)
    - <span class="section-num">2.1</span> [advice to org-odt-export-to-odt](#advice-to-org-odt-export-to-odt)
    - <span class="section-num">2.2</span> [用到的资源文件](#用到的资源文件)

</div>
<!--endtoc-->

最近需要写一些 word 文档，因为一直在用 Emacs 的 org-mode, 不想离开 Emacs 环境，也不想去手动调整 word 格式，所以花些时间研究了一下怎么使用 pandoc 将 org 转换成 docx 。


## <span class="section-num">1</span> 所需软件 {#所需软件}

-   [emacs](https://www.gnu.org/s/emacs/) : i live in emacs
-   [pandoc](https://github.com/jgm/pandoc/) : 文档转换界的瑞士军刀
-   [pandoc-crossref](https://github.com/lierdakil/pandoc-crossref) : 一个 pandoc 过滤器，用于对图形、方程、表格进行编号以及对它们进行交叉引用


## <span class="section-num">2</span> 方法 {#方法}


### <span class="section-num">2.1</span> advice to org-odt-export-to-odt {#advice-to-org-odt-export-to-odt}

Emacs 内置有命令 `org-odt-export-to-odt` 可以把 `org` 转换成 `odt` 格式，也可以再通过内置命令 `org-odt-convert`
来把 `odt` 转换成 `docx` ， 但这种方式生成的 word 文档在 wps 打开后，总是怪怪的，还是直接使用 pandoc 配合过滤器以及
reference-document 生成的文档看起来更舒服。所以我给 `org-odt-export-to-odt`  加了个 `advice` ，让他在转换成 `odt` 之后，直接调用 pandoc 来生成 `docx` 。之所以先生成 `odt` ，是为了在生成 `odt` 过程中 evaluate 各种 babel ，以便生成图片之类。

```emacs-lisp
(yc/defmacro defadvice! (how places symbol arglist &optional docstring &rest body)
  "Define an advice called SYMBOL and add it to PLACES.

   ARGLIST is as in `defun'.  HOW is a keyword as passed to `advice-add', and
   PLACE is the function to which to add the advice, like in `advice-add'.
   DOCSTRING and BODY are as in `defun'."
  (declare (doc-string 5) (indent defun))
  (unless (stringp docstring)
    (push docstring body)
    (setq docstring nil))
  `(progn
     (defun ,symbol ,arglist ,docstring ,@body)
     (dolist (target (ensure-list ,places))
       (advice-add target ,how #',symbol))))

(defadvice! :after 'org-odt-export-to-odt yc/org-odt-export-to-odt-a (&rest args)
  "Export to docx, then convert with pandoc. With prefix-arg, open after converted."
  (let* ((curr (buffer-file-name))
         (tmp (s-concat curr "_tmp.org"))
         (r-link (rx "[[" (group (or "fig" "tbl") ":" (+? nonl)) "]]")))
    (with-temp-file tmp
      (insert (s-replace-regexp r-link "[cite:@\\1]" (yc/buffer-content :file curr))))
    (yc/pandoc-convert-to-docx tmp (f-swap-ext curr "docx") t
      (s-contains? "toc:nil" (or (cadar (org-collect-keywords '("options") nil '("options"))) "")))))
```

这里函数 `yc/org-odt-export-to-odt-a` 在 `org-odt-export-to-odt` 后执行，它会首先使用原始 `org` 文件生成一个临时文件，并将内置的 org 格式的引用替换成 pandoc 支持更好的 cite 方式，然后再根据 org header 中声明的是否自动生成 toc 来调用
`yc/pandoc-convert-to-docx` ：

```emacs-lisp
(defun yc/pandoc-convert-to-docx (&optional ifile ofile delete no-toc)
  "Convert FILES to docx."
  (interactive)
  (let ((it (or ifile (yc/choose (yc/get-files))))
        (ofile (or ofile (f-swap-ext ifile "docx"))))
    (yc/interpret-command
      (s-join " "
        (list
         (yc/executable-find "pandoc" t) (unless no-toc "--toc") "--filter=pandoc-crossref"
         (format "--lua-filter='%s'"
           (yc/-locate-aux it "doc_filter.lua" "~/.local/share/pandoc/doc_filter.lua"))
         (format "--metadata-file='%s'"
           (yc/-locate-aux it "metadata.yaml" "~/.local/share/pandoc/metadata.yaml"))
         (format "--reference-doc='%s'"
           (yc/-locate-aux it "custom-reference.docx" "~/.local/share/pandoc/custom-reference.docx"))
         (format "'%s' --from %s+east_asian_line_breaks -t docx -o \"%s\""
           it (pcase (f-ext it) ("md" "markdown") (_ (f-ext ifile))) ofile)
         (if delete (list "; rm " ifile))))
      :infix "convert-docx" :popup 'auto :callback (lambda (f s)
                                                     (if (not s)
                                                         (error "Convert failed...")
                                                       (message "Convert finished..." )
                                                       (if current-prefix-arg
                                                           (let ((current-prefix-arg nil))
                                                             (yc/open-with-external-app ofile))))))))

```

这个函数会检查本地的一些辅助文件，最终生成了一个 shell 脚本去执行，生成的脚本类似：

```sh
/home/yyc/.emacs.d/lsp/bin/pandoc \
    --toc \
    --filter=pandoc-crossref \
    --lua-filter='/home/yyc/.local/share/pandoc/doc_filter.lua'\
    --metadata-file='/home/yyc/.local/share/pandoc/metadata.yaml' \
    --reference-doc='/home/yyc/.local/share/pandoc/custom-reference.docx' \
    'tonydeng_github_io_threat-modeling-was-conducted-based-on-STRIDE.org_tmp.org' \
    --from org+east_asian_line_breaks \
    -t docx \
    -o "tonydeng_github_io_threat-modeling-was-conducted-based-on-STRIDE.docx"

rm  tonydeng_github_io_threat-modeling-was-conducted-based-on-STRIDE.org_tmp.org
```

用到的参数的具体含义，可以直接查看 `pandoc --help` 。


### <span class="section-num">2.2</span> 用到的资源文件 {#用到的资源文件}

-   [doc_filter.lua](https://github.com/yangyingchao/MyNotes/blob/master/org/2025/02/assets/doc_filter.lua)  <br />
    自己写的一个针对 docx 输出的过滤器,主要功能：
    -   可以向 world 中插入分页符，只需要在 org 中插入下面这行即可：
        ```org
        #+LATEX: \newpage
        ```

    -   可以手动生成 toc, 只需要在 org 中插入下面这行即可：
        ```org
        #+LATEX: \tableofcontents
        ```

    -   可以手动添加多个空行：
        ```org
        #+LATEX: \vspace{90 mm}
        ```
        `10 mm` 会替换成一个空行。

    -   手动执行忽略某些内容 <br />
        如果一个 header 中包含标签 `nodocx` ，那么它的内容将不会输出到 word 中。

-   [custom-reference.docx](https://github.com/yangyingchao/MyNotes/blob/master/org/2025/02/assets/custom-reference.docx) <br />

    根据我自己的需要调整过的 word 模板，其中比较坑的表格的边框，开始时候不管怎样调，边框都是透明的，用 wps 和 libreoffice 都不行，直到最后用 ms office 调整过才生效。

-   [metadata.yaml](https://github.com/yangyingchao/MyNotes/blob/master/org/2025/02/assets/metadata.yaml) <br />
    其中含有 pandoc 和 pandoc-crossref 用的配置，主要是中文本土化。

