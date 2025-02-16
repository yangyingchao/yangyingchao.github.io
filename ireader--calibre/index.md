# iReader & Calibre


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [表现](#h:57f79f38-e207-4293-a147-b02bfdf82069)
- <span class="section-num">2</span> [原因](#h:9678f102-5772-4b3e-941f-438745e83994)
- <span class="section-num">3</span> [绕过方法](#h:827d4da8-a906-4237-8c61-90b1b020c9df)

</div>
<!--endtoc-->



## <span class="section-num">1</span> 表现 {#h:57f79f38-e207-4293-a147-b02bfdf82069}

通过 Calibre 向 iReader 中传输书籍失败，报 MTP 相关错误。


## <span class="section-num">2</span> 原因 {#h:9678f102-5772-4b3e-941f-438745e83994}

不知道为什么，无法在设备存储空间中创建目录


## <span class="section-num">3</span> 绕过方法 {#h:827d4da8-a906-4237-8c61-90b1b020c9df}

1.  用文件管理器提前创建 Books目录 （不推荐）, 或者
2.  在 Calibre 中找到设备，选择配置，将传输目录指向已有的目录，例如 iTransfer

推荐第二种，配置过后会生成下面的配置文件 `~/.config/calibre/mtp_devices.json`

```json-ts
{
  "blacklist": [],
  "device-47516c97f972181": {
    "send_to": [
      "i-transfer"
    ]
  },
  "history": {
    "47516c97f972181": [
      "Ocean 2",
      "2023-08-09T01:21:19.306004+00:00"
    ]
  }
}
```

