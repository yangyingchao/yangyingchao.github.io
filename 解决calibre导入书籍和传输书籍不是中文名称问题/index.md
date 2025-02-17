# 解决calibre导入书籍和传输书籍不是中文名称问题


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [解决 calibre 导入书籍和传输书籍不是中文名称问题](#解决-calibre-导入书籍和传输书籍不是中文名称问题)
- <span class="section-num">2</span> [我的解决方法](#我的解决方法)
    - <span class="section-num">2.1</span> [追加](#追加)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://mp.weixin.qq.com/s?__biz=MzA5MTgxNzM0Nw==&mid=2653286264&idx=1&sn=78ffdd914a3863f0b3927485e669bd98&chksm=8ba7db94bcd05282c10916297526edefb001f4b88ef18434c85e20d44a784893a1b91b80df37&mpshare=1&scene=1&srcid=0410rJQbgLUkbG2wSAoTpbln&sharer_shareinfo=6d065c71e8d859f972c14e5e0361382b&sharer_shareinfo_first=6d065c71e8d859f972c14e5e0361382b#rd



## <span class="section-num">1</span> 解决 calibre 导入书籍和传输书籍不是中文名称问题 {#解决-calibre-导入书籍和传输书籍不是中文名称问题}

1.  去官网下载最新版本软件，安装

2.  下载源码
    ```sh
    git clone https://github.com/kovidgoyal/calibre.git
    ```

3.  check out 相应安装版本，根据需求修改源代码。
    ```sh
    gco v5.43.0(gco = git checkout)
    ```

    -   修改 backend.py 文件，删掉 ascii_filename(）方法，解决导入书籍生成不是中文名称的问题。

    -   修改 smart_device_app 包下的 driver.py 文件，解决传输后书籍不是中文名称的问题。修改内容如下：

    {{< figure src="/ox-hugo/calibre-640.png" width="-1px" >}}

4.  创建快捷方式
    ```sh
    vim /usr/local/bin/calibre-develop
    ```
    写入内容
    ```sh
    #!/bin/sh
    export CALIBRE_DEVELOP_FROM="/Users/kovid/work/calibre/src"
    calibre-debug -g
    ```
    授予权限
    ```sh
    chmod +x /usr/local/bin/calibre-develop
    ```
    配置环境变量
    ```sh
    #calibre
    export CALIBRE=/Applications/calibre.app/Contents/MacOS
    export PATH=$PATH:$CALIBRE
    ```
    启动程序
    ```sh
    calibre-develop
    ```


## <span class="section-num">2</span> 我的解决方法 {#我的解决方法}

按照上面的方法搞了一遍，发现没起作用，没再去研究为什么，就直接修改了系统的 calibre 。。。

为了后续更新方便，把修改过程写在了 emacs 里面：

```emacs-lisp
(defun yc/fix-calibre ()
  "Fix calibre."
  (interactive)
  (let ((file (pcase system-type
                ('gnu/linux "/usr/lib64/calibre/calibre/utils/filenames.py")
                (_ (error "Platform %s not support" system-type)))))
    (if (yc/file-exists-p file)
        (with-current-buffer (yc/find-file-as-root file nil t)
          (goto-char (point-min))
          (unless (re-search-forward "# 替换为下划线" nil t)
            (goto-char (point-min))
            (search-forward "def ascii_filename(")
            (replace-match "
import re
def ascii_filename(")
            (goto-char (pos-eol))
            (insert "
    return re.sub(r'[\/\\\:\*\?\"\<\>\|]', substitute, orig)  # 替换为下划线
")
            (save-buffer))
          (kill-buffer (current-buffer)))
      (error "Can't open file: %s" file))))
```


### <span class="section-num">2.1</span> 追加 {#追加}

gentoo 里面提供的 binpkg 使用 [解决 calibre 导入书籍和传输书籍不是中文名称问题](#解决-calibre-导入书籍和传输书籍不是中文名称问题) 里提到的方法不起作用，但是如果 calibre 是从官网下载的二进制版本，则可以。。。

