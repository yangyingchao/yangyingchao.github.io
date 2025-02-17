# （转载）Hugo 添加 Giscus 评论


本文为摘录，原文为： https://stilig.me/posts/hugo-adds-giscus/

转载，侵删， 原文： <https://stilig.me/posts/hugo-adds-giscus/>

捣鼓了这么多天，看见别人的博客都有评论功能，十分羡慕，所以我在想有什么轻量又简单的方法，能让我的博客也能有评论功能，而
这时我找到了 Giscus。

Giscus 是一个由 Github Discussions 驱动的评论系统，无需自己单独配置，直接白嫖 Github 的资源即可，而且 UI 和功能都十分的
合适我，配合 LoveIt 的配置可以做到十分美观。
 配置 Github 仓库

-   如下图，创建一个公开的 Github 仓库

<a id="figure--fig:PicGo%2F202306190903900"></a>

{{< figure src="./images/PicGo%2F202306190903900.png" width="800px" >}}

-   找到 Settings -&gt; General -&gt; Features -&gt; Discussions 勾选，为仓库启动 Discussions 功能 ，如下图

<a id="org507288d"></a>

<img src="./images/PicGo%2F202306190909575.png" alt="PicGo%2F202306190909575.png" width="800px" />
pic-02

---

使配置好的仓库安装 Giscus

-   点击这里，我们将会看到下面的界面，我们点击安装

<a id="figure--fig:PicGo%2F202306190919754"></a>

{{< figure src="./images/PicGo%2F202306190919754.png" width="800px" >}}

-   点击安装后，要选择一个仓库，如下图，选择我们之前创造的仓库即可

<a id="orgc56ba0b"></a>

<img src="./images/PicGo%2F202306190928971.png" alt="PicGo%2F202306190928971.png" width="800px" />
pic-04

我遇到的问题

我创了个新号打算试试，但是发现点击安装后直接 404 ，估计是对新创的号有限制吧。

---

从官网获取配置信息

接下来，我们只需到 Giscus 官网获取配置信息，然后将配置信息填到 Hugo 的配置文件中即可。但是由于主题的不同，所以配置文件
的填写也不同，这里以 LoveIt 为例。

-   来到 Giscus 官网
    -   填写你的仓库名，如下图

<a id="org473dabb"></a>

<img src="./images/PicGo%2F202306191023678.png" alt="PicGo%2F202306191023678.png" width="800px" />
pic-05

-   选择页面与嵌入的 discussion 之间的映射关系，如下图

<a id="orged9797a"></a>

![](./images/PicGo%2F202306191027361.png)
pic-06

直接选第一个就行

-   选择 Discussion 分类，如下图

<a id="figure--fig:PicGo%2F202306191029555"></a>

{{< figure src="./images/PicGo%2F202306191029555.png" width="800px" >}}

我们选择 Announcements 类型即可，官方也是这样推荐的，因为这样便于管理。

-   其他选项默认，我们往下滑，找到配置文件，如下图

<a id="org452e221"></a>

<img src="./images/PicGo%2F202306191354061.png" alt="PicGo%2F202306191354061.png" width="800px" />
pic-08

我们要记下 data-repo，data-repo-id，data-category，data-category-id，data-mapping 这几个值。

-   修改 Hugo 配置文件

    注意

    这里使用的 Hugo 一定要是最新的版本，不然是不支持 Giscus 的（比如 release 的版本就是不支持的，因此要下载源码）。

    -   打开配置文件 config.toml，找到# 评论系统设置的第一个 enable 参量，将其改为 true，如下图：

<a id="orgc36dffa"></a>

![](./images/PicGo%2F202306191403367.png)
pic-09

-   找到# giscus comment 评论系统设置,并把其配置按照下面代码块修改。

     1  [params.page.comment.giscus]
     2    # 你可以参考官方文档来使用下列配置
     3    enable = true
     4    repo = "&lt;your_repo&gt;"
     5    repoId = "&lt;your_repoId&gt;"
     6    category = "&lt;your_category&gt;"
     7    categoryId = "&lt;your_categoryId&gt;"
     8    # &lt;your_repo&gt; 对应官网的 data-repo
     9    # &lt;your_repoId&gt; 对应官网的 data-repo-id
    10    # &lt;your_category&gt; 对应官网的 data-category
    11    # &lt;your_categoryId&gt; 对应官网的 data-category-id
    12
    13    # 为空时自动适配当前主题 i18n 配置
    14    lang = ""
    15    mapping = "&lt;your_mapping&gt;"
    16    # &lt;your_mapping&gt; 对应官网的 data-mapping
    17    reactionsEnabled = "1"
    18    emitMetadata = "0"
    19    inputPosition = "bottom"
    20    lazyLoading = false
    21    lightTheme = "light"
    22    darkTheme = "dark_dimmed"

    配置好后,就可以开启 Giscus 评论系统了。

**参考文章：**

1.  Hugo 博客引入 Giscus 评论系统
2.  迁移博客评论系统从 Utteranc.es 到 Giscus
3.  Giscus 官网

