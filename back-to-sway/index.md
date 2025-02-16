# Back to Sway


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [autotiling](#h:50b10f4a-0c41-4213-a5ef-33e5fc92743d)
- <span class="section-num">2</span> [swaynagmode](#h:bad15834-89da-4d37-befe-4075355f213e)
    - <span class="section-num">2.1</span> [swaynag (sway-1.8) crash&#x2026;](#h:cc9885d3-fc6a-4157-a72a-bd4be9f08cfe)
- <span class="section-num">3</span> [chrome 启动慢](#h:95b2f90d-0f6b-4eda-9ec0-0002971b90de)

</div>
<!--endtoc-->

用了一段时间的 [hyprland](https://hyprland.org/), 其 UI 和易用性都很不错，但可能是因为还处于功能的积极开发中，稳定性还差点，
在 session 退出， 或者断开、连接新显示器时候会经常宕机， coredumpctl 里面总是一票 coredump 。。
昨天受不了了，还是换回了 [sway](https://swaywm.org/) 。这里记录几个换回来后发现的几个小问题（和增强）。


## <span class="section-num">1</span> autotiling {#h:50b10f4a-0c41-4213-a5ef-33e5fc92743d}

Hyprland 可以在创建新窗口时候自动设置窗口分割方向，很方便。 sway 没有内置这个功能，但是可以通过 [autotiling](https://github.com/nwg-piotr/autotiling) 来实现。


## <span class="section-num">2</span> swaynagmode {#h:bad15834-89da-4d37-befe-4075355f213e}

Control swaynag via keyboard shortcuts


### <span class="section-num">2.1</span> swaynag (sway-1.8) crash&#x2026; {#h:cc9885d3-fc6a-4157-a72a-bd4be9f08cfe}

最新的主干有修复，合并过来即可： [my overlay](https://github.com/yangyingchao/gentoo-overlay-tubo/tree/master/gui-wm/sway)


## <span class="section-num">3</span> chrome 启动慢 {#h:95b2f90d-0f6b-4eda-9ec0-0002971b90de}

这个是困扰了好一阵的问题，应该是 chrome 启动时候某个进程在尝试从 dbus 中读取什么东西，然后一直等到超时退出。。。

```sh
export XDG_CURRENT_DESKTOP=sway # xdg-desktop-portal
export XDG_SESSION_DESKTOP=Unity # systemd
export XDG_SESSION_TYPE=wayland # xdg/systemd

dbus-update-activation-environment XDG_CURRENT_DESKTOP XDG_SESSION_DESKTOP XDG_SESSION_TYPE
systemctl --user import-environment XDG_CURRENT_DESKTOP XDG_SESSION_DESKTOP XDG_SESSION_TYPE

dbus-run-session sway  > ~/tmp/wm.log 2>&1
```

