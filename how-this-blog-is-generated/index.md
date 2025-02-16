# 这个博客是怎么生成的？


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [用 Org Mode + Hugo 写博客，并通过 Github Action 自动部署到 Github Pages](#h:77d4f63b-c1fa-4f57-93b5-c757e41e1a71)
    - <span class="section-num">1.1</span> [准备工作](#h:ae0cb92a-bd96-42bf-9858-deed2ae7d6e4)
    - <span class="section-num">1.2</span> [写博客](#h:a50720d1-7467-43fe-96bf-c99596239cae)
    - <span class="section-num">1.3</span> [部署篇](#h:fb08f300-8df9-412a-8a7e-0a5978f7d02a)
    - <span class="section-num">1.4</span> [遇到的问题](#h:3d56e5b1-24da-4c13-a233-241622c7a523)
- <span class="section-num">2</span> [使用 Emacs Script 自动将 org 文件导出为 Markdown](#h:3dab7760-3b4f-456e-b112-e2e740204576)
    - <span class="section-num">2.1</span> [痛点](#h:d959fa6b-ae45-41e4-8da5-fd800606a0e6)
    - <span class="section-num">2.2</span> [如何解决](#h:c5499c54-d782-4a29-b13c-e4e74d749915)
    - <span class="section-num">2.3</span> [存在的问题](#h:b08e34c0-315f-43a9-9b4a-cc087fb86323)
    - <span class="section-num">2.4</span> [遇到的问题](#h:43b5e31b-6034-4c46-bb48-782b103d0536)
- <span class="section-num">3</span> [附录](#h:7d9d2a30-9a98-4243-8da6-51eb1201b20e)
    - <span class="section-num">3.1</span> [我正在用的 github actions](#h:bbb920e4-d3fe-48e3-9221-e4c6c5b53844)
    - <span class="section-num">3.2</span> [当前用的脚本](#h:006ec282-c3c0-4055-9d39-f9f564afd922)

</div>
<!--endtoc-->

参考这里：

-   <https://superbear.github.io/post/2021/11/use-org-mode-and-hugo-to-write-blog/>
-   <https://superbear.github.io/post/2021/12/batch-export-org-files-to-markdown-with-emacs-script-and-ox-hugo/>

搬运一下，放丢。。。

以下内容转自 superbear 的博客。


## <span class="section-num">1</span> 用 Org Mode + Hugo 写博客，并通过 Github Action 自动部署到 Github Pages {#h:77d4f63b-c1fa-4f57-93b5-c757e41e1a71}


### <span class="section-num">1.1</span> 准备工作 {#h:ae0cb92a-bd96-42bf-9858-deed2ae7d6e4}


#### <span class="section-num">1.1.1</span> 创建两个 Github 仓库 {#h:be11c5fd-358f-4257-a3de-991dacf33dfa}

-   用来放 org 文件. markdown 文件及 Hugo 相关配置，可以设置成私有，如 blog。
-   Github Pages 仓库，用来放生成的静态文件。一般是{username}.github.io。


#### <span class="section-num">1.1.2</span> 安装 Hugo {#h:590f68ab-9703-405b-8365-c2b1805bb518}

-   a static site generator written in Go


#### <span class="section-num">1.1.3</span> 安装 ox-hugo {#h:16bf7be3-b1f1-480f-a90a-194c11970799}

-   将 org 文件翻译成 markdown 文件，written in Emacs Lisp。Hugo 支持 Org Mode，但据说支持得不是很好 1。
    毕竟 Emacs Lisp 处理 org 文件相关的包比 Go 多，就直接用 ox-hugo 转 markdown 的方案了。

不是 Emacs 用户？

-   可跳过 Org Mode 相关内容


#### <span class="section-num">1.1.4</span> 配置环境 {#h:221b082e-bedf-46ae-9957-6eb4272b962d}

各软件版本

-   Emacs 27.1
-   Org Mode 9.3
-   ox-hugo 20210916.1332
-   Hugo v0.89.4

-   配置 Hugo
    1.  \# 在 blog 仓库中新建 hugo site
    2.  cd /path/to/blog &amp;&amp; hugo new site hugo
    3.  \# 选个主题，并设置成子模块（后续升级/替换都比较方便）
    4.  git submodule add <https://github.com/olOwOlo/hugo-theme-even.git> hugo/themes/even
    5.  \# 修改默认配置

-   开启本地实时预览
    1.  \# 配置修改. markdown 文件改动都会被监控到
    2.  cd blog &amp;&amp; hugo &#x2013;source hugo server -D

-   配置 ox-hugo

<!--listend-->

```emacs-lisp
;;; 在 init.el 里或其他文件里加以下代码
(with-eval-after-load 'ox
  (require 'ox-hugo))
;;; 可选。全局配置，如导出目录等字段，也可以在 org 文件里进行配置
;;; M-x customize-variable org-hugo
```


### <span class="section-num">1.2</span> 写博客 {#h:a50720d1-7467-43fe-96bf-c99596239cae}

1.  新建 org 目录及文件
    1.  cd blog &amp;&amp; mkdir -p org/2021/11
    2.  \# 可选，可以不按年月日分拆分目录。拆分后方便管理，但如果用户直接访问某年的数据，页面就会 404

2.  新建 org 文件，如 test.org

<!--listend-->

```org
#+OPTIONS: author:nil ^:{}
# 告诉 ox-hugo 将导出的 markdown 文件放到哪里。注意：even 主题需要发布到 post 目录。
# see: https://ox-hugo.scripter.co/#before-you-export
#+HUGO_BASE_DIR: ../../../hugo
#+HUGO_SECTION: post/2021/11
#+HUGO_CUSTOM_FRONT_MATTER: :toc true
#+HUGO_AUTO_SET_LASTMOD: t
#+HUGO_DRAFT: false
#+DATE: [2021-11-28 Sun 19:28]
#+TITLE: 标题
#+HUGO_TAGS: tag1 tag2
#+HUGO_CATEGORIES: category1 category2
Hello，World
```

1.  导出 org 到 markdown 文件

2.  在 Emacs 里执行 C-c C-e H h（File to Md file）导出整个 org 文件到 markdown 文件，也可以导出 subtree，
    具体看个人喜好

3.  访问<http://127.0.0.1:1313/>
4.  正常情况就会看到博客界面啦
5.  提交 org 及 markdown 文件到 blog 仓库
6.  使用 Yasnippet（推荐） 下次新建文件就不用 copy 一些元数据

<!--listend-->

```snippet
# -*- mode: snippet -*-
# name: hugo
# key: hugo
# --
#+OPTIONS: author:nil ^:{}
#+HUGO_BASE_DIR: ../../../hugo
#+HUGO_SECTION: post/`(format-time-string "%Y/%m")`
#+HUGO_CUSTOM_FRONT_MATTER: :toc true
#+HUGO_AUTO_SET_LASTMOD: t
#+HUGO_DRAFT: false
#+DATE: `(format-time-string "[%Y-%m-%d %a %H:%M]")`
#+TITLE: $1
#+HUGO_TAGS: $2
#+HUGO_CATEGORIES: $3
$0
```


#### <span class="section-num">1.2.1</span> 目录结构 {#h:2a8af21d-c8c0-41f1-8925-1c7a68063f84}

```text
 <!-- blog 仓库目录结构如下 -->
  <!-- tree -a -d -L 3 -I .git -->
.
├── .github
│   └── workflows
├── hugo
│   ├── archetypes
│   ├── content
│   │   └── post
│   ├── data
│   ├── layouts
│   │   └── _internal
│   ├── static
│   └── themes
│       └── even
├── org
│   └── 2021
│       └── 11
└── scripts
```


### <span class="section-num">1.3</span> 部署篇 {#h:fb08f300-8df9-412a-8a7e-0a5978f7d02a}


#### <span class="section-num">1.3.1</span> 配置 Github Action {#h:c47c61b5-6463-4b58-a111-0c975a87abf2}

在 blog 仓库下新增.github/workflows/main.yml 文件，内容如下：

```yaml
name: GitHub Pages
on:
  push:
    branches:
      - main  # Set a branch to deploy
  pull_request:
jobs:
  deploy:
    runs-on: ubuntu-20.04
    concurrency:
      group: ${{ github.workflow }}-${{ github.ref }}
    steps:
      # 修改时区
      - name: Set Timezone
        run: sudo timedatectl set-timezone 'Asia/Shanghai'
      - uses: actions/checkout@v2
        with:
          submodules: true  # Fetch Hugo themes (true OR recursive)
          fetch-depth: 0    # Fetch all history for .GitInfo and .Lastmod
      - name: Setup Hugo
        uses: peaceiris/actions-hugo@v2
        with:
          # 建议和本地用的版本保持一致，从而获得一致的体验
          hugo-version: '0.89.4'
          # extended: true
      - name: Build
        run: |
          hugo --source hugo --minify --buildFuture --buildExpired
      - name: Deploy
        uses: peaceiris/actions-gh-pages@v3
        if: ${{ github.ref == 'refs/heads/main' }}
        with:
          # github_token: ${{ secrets.PERSONAL_ACCESS_TOKEN }}
          # publish_branch: gh-pages
          # publish_dir: hugo/public
          deploy_key: ${{ secrets.DEPLOY_KEY }}
          external_repository: superbear/superbear.github.io
          # 默认是 master 目录，github 上可修改
          publish_branch: master
          publish_dir: hugo/public
```


#### <span class="section-num">1.3.2</span> 配置 DEPLOY_KEY {#h:fe0a2943-fea6-4455-a7c4-3300df3ed776}

生成 github deploy-keys

1.  将公钥添加至{username}.github.io 仓的 Deploy keys（Settings -&gt; Deploy keys）中；
2.  将私钥添加至 blog 仓库的 secrets（Settings -&gt; Secrets）中；
3.  将本地 blog 目录的改动 push 到远程 blog 仓库 master 分支；
4.  正常情况下，一段时间之后，可以在{username}.github.io 的 master 分支下看到 hugo 生成的静态文件；
5.  没有的话，可以根据 blog 仓库的 Github Actions 日志排查下。


### <span class="section-num">1.4</span> 遇到的问题 {#h:3d56e5b1-24da-4c13-a233-241622c7a523}

1.  部署完成却没有生成文件，就只有一个.nojekyll 文件

    1  &lt;!&#x2013; actions log &#x2013;&gt;
    2  cp: no such file or directory: /home/runner/work/blog/hugo/public/\*

排查思路：排查 Github Actions 日志，以及写一些 shell 命令查看路径相关信息

1.  Hugo 生成静态文件报错，报错信息如下：

    1  - unmarshal failed: Near line 4 (last key parsed 'tags'): Array contains values of type 'Integer' and 'String', but arrays must be homogeneous.

原因：tags 的类型不一致，标签混用了数字和字符串
解决方案：去掉数字类型的标签或升级至最新版本的 hugo


## <span class="section-num">2</span> 使用 Emacs Script 自动将 org 文件导出为 Markdown {#h:3dab7760-3b4f-456e-b112-e2e740204576}

接上篇 用 Org Mode + Hugo 写博客，并通过 Github Action 自动部署到 Github Pages


### <span class="section-num">2.1</span> 痛点 {#h:d959fa6b-ae45-41e4-8da5-fd800606a0e6}

代码仓库里会同时保存 org 文件和 markdown 文件，markdown 文件其实是中间产物，不想保存。


### <span class="section-num">2.2</span> 如何解决 {#h:c5499c54-d782-4a29-b13c-e4e74d749915}

首先，Emacs 是可以执行 Emacs Script 的，写个脚本，然后在 Github Action 里执行即可。Emacs 环境哪里来？
purcell 大神已经准备好了。

废话不多说，直接上代码。

嵌入 Gist

1  #+BEGIN_EXPORT html
2  &lt;script src="<https://gist.github.com/superbear/28fb0dbbca505b5d7d83e10e35b822a4.js>"&gt;&lt;/script&gt;
3  #+END_EXPORT

等等，直接用 markdown 写是不是就没有这个痛点了？嗯😄。


### <span class="section-num">2.3</span> 存在的问题 {#h:b08e34c0-315f-43a9-9b4a-cc087fb86323}

目前是全量导出，找到指定目录下的全部 org 文件，然后转成 markdown 文件。 这样每提交一篇文章，就需要
处理全部存量文章。另外，全部文章的更新时间都会跟着变，这个和 HUGO_AUTO_SET_LASTMOD 这个 property
设置有关。详见：ox-hugo last modified 。待改成增量导出。


### <span class="section-num">2.4</span> 遇到的问题 {#h:43b5e31b-6034-4c46-bb48-782b103d0536}

1.  org 导出 markdown 文件了，但未生成静态文件 <br />

原因：时区问题。Github Action 是按 UTC 时间执行的，而文件的发布日期是东八区的，这样 Hugo 可能会看到
发布日期还未到，就不处理了 1。
解决方案：修改 Github Action job 的时区，可一并解决文件修改时间不对的问题；或修改 Hugo 运行时的配
置，增加–buildFuture 参数。


## <span class="section-num">3</span> 附录 {#h:7d9d2a30-9a98-4243-8da6-51eb1201b20e}


### <span class="section-num">3.1</span> 我正在用的 github actions {#h:bbb920e4-d3fe-48e3-9221-e4c6c5b53844}

```yaml
# Sample workflow for building and deploying a Hugo site to GitHub Pages
name: Deploy Hugo site to Pages

on:
  # Runs on pushes targeting the default branch
  push:
    branches: ["master"]

  # Allows you to run this workflow manually from the Actions tab
  workflow_dispatch:

# Sets permissions of the GITHUB_TOKEN to allow deployment to GitHub Pages
permissions:
  contents: read
  pages: write
  id-token: write

# Allow only one concurrent deployment, skipping runs queued between the run in-progress and latest queued.
# However, do NOT cancel in-progress runs as we want to allow these production deployments to complete.
concurrency:
  group: "pages"
  cancel-in-progress: false

# Default to bash
defaults:
  run:
    shell: bash

jobs:
  build:
    runs-on: ubuntu-20.04
    concurrency:
      group: ${{ github.workflow }}-${{ github.ref }}
    steps:
      - uses: actions/checkout@v2
        with:
          submodules: true  # Fetch Hugo themes (true OR recursive)
          fetch-depth: 0    # Fetch all history for .GitInfo and .Lastmod

      - name: Setup Hugo
        uses: peaceiris/actions-hugo@v2
        with:
          hugo-version: '0.115.0'
          extended: true

      - name: Setup Emacs
        uses: purcell/setup-emacs@master
        with:
          version: 29.1

      - name: Export Markdown
        run: |
          # 将org文件导出成md文件
          mkdir ~/.emacs.d
          cd script && sh batch-export-org-files-to-md-with-ox-hugo.el

      - name: Build
        run: |
          cd hugo && hugo --minify --buildFuture --buildExpired

      - name: Upload artifact
        uses: actions/upload-pages-artifact@v2
        with:
          path: ./hugo/public

      - name: Deploy
        uses: peaceiris/actions-gh-pages@v3
        # If you're changing the branch from main,
        # also change the `main` in `refs/heads/main`
        # below accordingly.
        if: github.ref == 'refs/heads/master'
        with:
          publish_branch: master
          publish_dir: hugo/public
          deploy_key: ${{ secrets.DEPLOY }}
          external_repository: yangyingchao/yangyingchao.github.io

```


### <span class="section-num">3.2</span> 当前用的脚本 {#h:006ec282-c3c0-4055-9d39-f9f564afd922}

```emacs-lisp
#!/usr/bin/env sh
:; set -e # -*- mode: emacs-lisp; lexical-binding: t -*-
:; emacs --no-site-file --script "$0" -- "$@" || __EXITCODE=$?
:; exit 0

;;; Code:
(defvar bootstrap-version)
(defvar straight-base-dir)
(defvar straight-fix-org)
(defvar straight-vc-git-default-clone-depth 1)
(defvar publish--straight-repos-dir)

(setq gc-cons-threshold 83886080 ; 80MiB
      straight-base-dir (expand-file-name "../.." (or load-file-name buffer-file-name))
      straight-fix-org t
      straight-vc-git-default-clone-depth 1
      publish--straight-repos-dir (expand-file-name "straight/repos/" straight-base-dir))

(let ((bootstrap-file (expand-file-name "straight/repos/straight.el/bootstrap.el" straight-base-dir))
      (bootstrap-version 5))
  (unless (file-exists-p bootstrap-file)
    (with-current-buffer
        (url-retrieve-synchronously
         "https://raw.githubusercontent.com/raxod502/straight.el/develop/install.el"
         'silent 'inhibit-cookies)
      (goto-char (point-max))
      (eval-print-last-sexp)))
  (load bootstrap-file nil 'nomessage))

;;; org && ox-hugo
(straight-use-package '(org :type built-in))
(straight-use-package
 '(ox-hugo :type git
           :host github
           :repo "kaushalmodi/ox-hugo"
           :nonrecursive t))

(require 'ox-hugo)

(defun yc/org-hugo--build-toc-a (content)
  "Append my markers.

These markers are used to identify original source of this note."
  (concat (or content "")
    (save-excursion
      (goto-char (point-min))
      (if (re-search-forward (rx bol ":NOTER_DOCUMENT:" (* space) (group (+ nonl)) eol))
          (let ((orig (match-string 1)))
            (format "\n\n本文为摘录，原文为： %s\n" orig))
        ""))))

(advice-add #'org-hugo--build-toc :filter-return #'yc/org-hugo--build-toc-a)

(defun tnote/export-org-file-to-md (file)
  "Export single FILE to markdown."
  (message "Checking file %s" file)
  (if (and (file-exists-p file)
           (string-equal (file-name-extension file) "org")
           (not (string-match-p (rx (or "inbox" "gtd")) file)))
            (with-current-buffer (find-file-noselect file)
        (message "    Processing file: %s" file)
        (condition-case var
            (org-hugo-export-wim-to-md t)
          (error (message "ERROR: %s" var)))
        (message "    .... done"))
    (message "    Skipping file: %s" file)))

(defun batch-export-all-org-files-to-md (dir)
  "Export all org files in directory DIR to markdown.

To perform a full export of all org files in the directory DIR to
markdown format, use this command. It should be called when a
full export is required, typically for the first time.."
  (message "DIR: %s" dir)
  (mapc #'tnote/export-org-file-to-md (directory-files-recursively dir "\\`[^.#].*\\.org\\'")))

(defun batch-export-HEAD-files-to-md ()
  "Export the files in the HEAD branch to markdown format.

This command should be called in an incremental manner to
effectively export updated files.."
  (dolist (it (cdr (string-lines (shell-command-to-string "git show --oneline --name-only HEAD"))))
    (tnote/export-org-file-to-md (expand-file-name it ".."))))

;;; export
(setq org-hugo-base-dir (concat default-directory "../hugo"))
(setq org-file-dir (concat default-directory "../org/"))

(batch-export-all-org-files-to-md org-file-dir)

```

