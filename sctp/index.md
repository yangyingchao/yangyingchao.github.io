# Stream Control Transmission Protocol (SCTP)


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [SCTP Associations](#h:2815bca8-e6bb-4b22-93b8-52266d83331a)
- <span class="section-num">2</span> [Using Multiple Interfaces](#h:a7d3a58f-21fb-4e4e-b3d1-1de8bf5291e3)
- <span class="section-num">3</span> [Streams](#h:bbf694f6-56f7-4d97-8334-74bfe30fd53d)
- <span class="section-num">4</span> [Association ID](#h:276f19c3-3843-4d1b-a1ee-3dfd0126996c)
- <span class="section-num">5</span> [Rec &amp; Send](#h:ae3adb64-a1ab-4352-aa2a-8b58a81d1bf4)
- <span class="section-num">6</span> [CDB with SCTP?](#h:d0a6f1bd-1d91-4b94-b5cd-f70950fb9e9f)
- <span class="section-num">7</span> [SCTP Interface Model](#h:fa817dc8-9b69-4412-8a1c-54067ac156b4)
    - <span class="section-num">7.1</span> [two types of SCTP sockets](#h:79c523c0-858f-44d3-a13a-8ffb6cd217ac)
    - <span class="section-num">7.2</span> [The One-to-One Style](#h:af8a22bd-336b-47ac-a22b-ba2a592aa426)
    - <span class="section-num">7.3</span> [The One-to-Many Style](#h:400f90d1-fb4b-4e76-ae90-8feff25e9124)

</div>
<!--endtoc-->


本文为摘录，原文为： https://www.linuxjournal.com/article/9749



## <span class="section-num">1</span> SCTP Associations {#h:2815bca8-e6bb-4b22-93b8-52266d83331a}

SCTP 的联合 (association) 类似于 TCP 的连接 (connection) ， 但又有不同：

-   TCP connection 在服务端和客户端之间的网络接口上建立的 **一对一** 连接
-   SCTP association 是 **多对多** ：
    -   可以在服务端的多个网口和客户端的多个网口之间建立联合 <br />
        例如，服务端和客户端各自有两个设备： Ethernet &amp; Wi-Fi , 则服务端和客户端之
        间的联合可以包括 4 路：
        -   Eth -&gt; Eth
        -   Eth -&gt; Wi-Fi
        -   Wi-Fi -&gt; Eth
        -   Wi-Fi -&gt; Wi-Fi

<!--listend-->

-   一个联合中又可以有多个 stream ， 各个 stream 之间互不干扰，不会阻塞 <br />
    stream 用 stream id 来区分，从 0 开始增加

<!--listend-->

-   每个 socket 可以用来服务多个联合 <br />
    也就是说，每个 scoket 可以用来同多个主机通讯。 每个 association 用
    association id 来区分。


## <span class="section-num">2</span> Using Multiple Interfaces {#h:a7d3a58f-21fb-4e4e-b3d1-1de8bf5291e3}

-   服务端可以通过 `sctp_bindx()` 来绑定多个接口
-   客户端可以通过 `sctp_connectx()` 来使用多个地址
-   客户端可以通过 `sctp_getpaddrs()` 来获取远端地址
-   客户端可以通过 `sctp_getladdrs()` 来获取本地地址

Once an association is set up between two endpoints, messages can be sent between them. Note that SCTP does not concern
itself with QoS (Quality-of-Service) issues, such as real-time delivery, but only with reliability issues. SCTP uses the
multihomed capabilities to try as many possible routes as possible to get messages through. So on the sending side,
there is no control over which interfaces are used; indeed, the sender might even use a scheme such as round-robin among
its interfaces for each message. However, the sending application can indicate to its SCTP stack which of the remote
peer's interface it would prefer to use, and it can tell the remote peer on which interfaces it would prefer to receive
messages. These are done by using the setsockopt() call with option type as SCTP_PRIMARY_ADDR or
SCTP_SET_PEER_PRIMARY_ADDR. Of course, if these particular addresses are not available, SCTP simply will use different
addresses in the association.

Once SCTP is told which interfaces to use, it basically looks after things itself. It uses heartbeats to keep track of
which interfaces are alive, and it switches interfaces transparently when failure occurs. This is to satisfy the design
goals of SCTP for improved reliability over TCP. Applications can give hints to the SCTP stack about which interfaces to
use, but the stack will ignore these hints on failure.


## <span class="section-num">3</span> Streams {#h:bbf694f6-56f7-4d97-8334-74bfe30fd53d}

Negotiating the Number of Streams

Each endpoint of an association will support a certain number of streams. A Linux endpoint, by default, will expect to
be able to send to ten streams, while it can receive on 65,535 streams. Other SCTP stacks may have different default
values. These values can be changed by setting the socket option SCTP_INITMSG, which takes a structure sctp_initmsg:

```c

struct sctp_initmsg {
    uint16_t sinit_num_ostreams;
    uint16_t sinit_max_ostreams;
    uint16_t sinit_max_attempts;
    uint16_t sinit_max_init_timeo;
}

```

If this socket option is used to set values, it must be done **before** an
association is made. The parameters will be sent to the peer endpoint during
association initialisation.


## <span class="section-num">4</span> Association ID {#h:276f19c3-3843-4d1b-a1ee-3dfd0126996c}

There can be many associations active at any one time—a peer can be connected
to many other peers simultaneously. This is different from TCP where only one
connection on a socket can exist and also is different from UDP where no
connections exist and messages are just sent to arbitrary peers.

When there can be many associations, you need to be able to distinguish
between them. This is done by an opaque data type called an association ID.
You need to use this sometimes, but not every time.

-   one-to-one sockets: <br />
    -   仅用一个联合， Association ID 可以忽略
    -   created a one-to-one socket:
        ```c++
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)
        ```

-   one-to-many sockets: <br />
    -   当可以通过目的端地址来标识 Association 时， ID 可以忽略
    -   其余时候需要使用 ID
    -   create one-to-many socket:
        ```c
        sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)
        ```
    -   ID 的获取：
        ```c++
        sctp_assoc_t get_associd(int sockfd, struct sockaddr *sa, socklen_t salen) {
            struct sctp_paddrinfo sp;
            int sz;

            sz = sizeof(struct sctp_paddrinfo);
            bzero(&sp, sz);
            memcpy(&sp.spinfo_address, sa, salen);
            if (sctp_opt_info(sockfd, 0, SCTP_GET_PEER_ADDR_INFO, &sp, &sz) == -1)
                perror("get assoc");
            return (sp.spinfo_assoc_id);
        }

        ```

There is no way to specify from which stream to read. This is deliberate; the
intention is that when data is ready on any stream, then you read
it. Otherwise, data could be blocked on a stream with no one to read it, which
eventually could fill up system buffers. So, you can't restrict reading to any
particular stream. But, once a read is done, you can tell which stream it has
come from by using the mechanism above.

Typically, a server that reads and handles a message will have (pseudocode) that looks like this:

```c

while (true) {
    nread = sctp_recvmsg(..., msg, ..., &sinfo, ...)
    if (nread <= 0) break;
    assoc_id = sinfo.sinfo_assoc_id;
    stream = sinfo.sinfo_stream;
    handle_mesg(assoc_id, stream, msg, nread);
}

```

This is a single-threaded read loop. It ensures that information is read, no matter what association or stream it is
sent on. The application function handle_mesg() can, of course, dispatch the message to different threads if it wants.
Writes, on the other hand can be sent from multiple threads if desired.


## <span class="section-num">5</span> Rec &amp; Send {#h:ae3adb64-a1ab-4352-aa2a-8b58a81d1bf4}

When it comes to sending messages back to all the connected clients, SCTP
makes it even easier—the flag `SCTP_SENDALL` that can can be set in the
`sctp_sndrcvinfo` field of `sctp_send()`. So a server simply needs to read a
message from any client, set the SCTP_SENDALL bit and write it back out. The
SCTP stack then will send it to all live peers! There are only a few lines of


## <span class="section-num">6</span> CDB with SCTP? {#h:d0a6f1bd-1d91-4b94-b5cd-f70950fb9e9f}

-   发送端
    -   可使用 一个 socket ， 对多个 server 创建 associations
    -   多线程并行发送
-   接收端
    -   一个 socket ， 类似 UDP 接收
    -   单线程接收，分拣
    -   回发数据？？


## <span class="section-num">7</span> SCTP Interface Model {#h:fa817dc8-9b69-4412-8a1c-54067ac156b4}

<https://www.masterraghu.com/subjects/np/introduction/unix_network_programming_v1.3/ch09.html>


### <span class="section-num">7.1</span> two types of SCTP sockets {#h:79c523c0-858f-44d3-a13a-8ffb6cd217ac}


### <span class="section-num">7.2</span> The One-to-One Style {#h:af8a22bd-336b-47ac-a22b-ba2a592aa426}

便于移植基于 TCP 协议的程序。

<a id="figure--fig:screenshot@2022-06-21-17:11:51"></a>

{{< figure src="/ox-hugo/screenshot@2022-06-21_17:11:51.png" width="800px" >}}


### <span class="section-num">7.3</span> The One-to-Many Style {#h:400f90d1-fb4b-4e76-ae90-8feff25e9124}

一对多模式可以减少 server 所使用的 socket 数量：可以使用一个 socket 来建立多个
联合。 联合通过 `association id` 来标识。 该 ID 由内核指定，对用户透明。

使用该模式需要注意：

-   当客户端关闭联合之后， server 端自动关闭，并将相关的状态从内核中移除。
-   使用 one-to-many 方式，可以在四次握手的第三个或者第四个包即开始发送数据。
-   使用 `sendto/sendmsg/sctp_sendmsg` 发送数据时候，如果联合不存在，会自动创建。
-   必须使用 `sendto/sctp_sendmsg` 来发送数据，而不能使用 `send/write`
-   发送数据时，总是使用主目的地址 （建立联合时候系统选定的地址）。可以通过设置
    flag `MSG_ADDR_OVER` 来强制使用其他地址。 该 flag 可以在 `sctp_sendmsg()` 中
    通过 `sctp_sndrcvinfo` 来设置。
-   事件通知可以通过 `SCTP_EVENTS` 来控制。

    <a id="figure--fig:screenshot@2022-06-21-17:33:56"></a>

    {{< figure src="/ox-hugo/screenshot@2022-06-21_17:33:56.png" >}}

