# video - How to strip audio streams from an MKV file? - Super User


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Q](#q)
- <span class="section-num">2</span> [A:](#a)
- <span class="section-num">3</span> [Further more](#further-more)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://superuser.com/questions/77504/how-to-strip-audio-streams-from-an-mkv-file



## <span class="section-num">1</span> Q {#q}

我一直在尝试从一个 MKV（Matroska）文件中删除不需要的音频流。我这样做的原因是为了避免在 Windows Media Player 中手动选择所需的流。

输入文件提供了以下的 ffmpeg 信息：

```text
流 #0.0: 视频: mpeg4, yuv420p, 704x396 [PAR 1:1 DAR 16:9], 29.98 tbr, 1k tbn, 29.98 tbc
流 #0.1(eng): 音频: aac，24000 Hz，5.1，s16
流 #0.2(jpn): 音频: aac，24000 Hz，5.1，s16
流 #0.3(eng): 字幕: 0x0000
流 #0.4(eng): 字幕: 0x0000
流 #0.5: 附件: 0x0000
流 #0.6: 附件: 0x0000
```

由于我想要的流是 0，1 和 3（sub），我的 ffmpeg 命令如下：

```sh
ffmpeg -i input.mkv -map 0:0 -map 0:1 -map 0:3 -vcodec copy -acodec libmp3lame -newsubtitle test.mkv
```

但是却奇怪地出现了错误：

至少必须指定一个输出文件

如果要删除字幕：

```sh
ffmpeg -i input.mkv -map 0:0 -map 0:1 -vcodec copy -acodec libmp3lame test.mkv
```

则会出现：

流映射数量必须与输出流的数量相匹配

我似乎不太理解“map”选项的工作原理。有人能帮我弄清楚吗？


## <span class="section-num">2</span> A: {#a}

如果你只想删除一个流而不重新编码，你可能希望使用 MKVtoolnix 包来完成（请参阅 videohelp.com
的页面）。

有一些图形用户界面可能会帮助你（在 videohelp 链接中查找）。我不确定确切的 mkvmerge 命令行可能是什么，但我认为类似这样的内容会起作用：

```text
# 首先，获取音频轨道信息，这样我们知道要保留哪个
mkvmerge -i input.mkv
File 'input.mkv': container: Matroska
Track ID 1: video (V_MPEG4/ISO/AVC)
Track ID 2: audio (A_AAC)
Track ID 3: audio (A_AAC)        <----------- 例如，我们保留这个
Track ID 4: audio (A_AAC)
mkvmerge -o output.mkv --audio-tracks 3 input.mkv
```

这样应该就可以了（很抱歉，我没有可用于测试的文件）。&#x2013;audio-tracks 选项告诉 mkvmerge 仅将列出的音轨复制到新文件中。如果你想保留第 2 和第 3 轨道，但不要第 4 轨道，可以使用&#x2013;audio-tracks 2,3。

mkvmerge 还有很多其他选项，比如设置标题、为音频添加延迟以同步等等，所以请查看 manpage 以获取详细信息。


## <span class="section-num">3</span> Further more {#further-more}

For anyone needing to automate this, I've done this:

```sh
find ./ -maxdepth 1 -type f \
     -exec mkvmerge -o ./output/"{}" \
     --audio-tracks 2 \
     --video-tracks 0 \
     --subtitle-tracks 5 \
     --default-track-flag 2 \
     --default-track-fla g 5 \
     --forced-display-flag 5 "{}" \;
```

This finds all files in current directory, include only audio track 2 video track 0, and
subtitle track 5. Also makes track 5 default subtitle and forces it. Then save each file
in ./output directoy.

