# gpdb cdb


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Data structures](#h:3a9ecefa-c67a-4512-a5d4-69adb4c9a0d6)
    - <span class="section-num">1.1</span> [Slice Table](#h:1223137a-c35a-4cca-8922-447664c4cc0b)
- <span class="section-num">2</span> [PostgresMain](#h:9a8ad5db-0d8c-4bef-bbc8-4a27cd48ef45)
    - <span class="section-num">2.1</span> [Call graph (QE):](#h:554948eb-a908-49ea-89f4-324ac2f96fe7)
    - <span class="section-num">2.2</span> [QD](#h:e1a256d3-301b-4ba1-be57-8749befb730f)
- <span class="section-num">3</span> [Receiver](#h:c37bea35-e940-428c-befb-d19fa51a1295)
- <span class="section-num">4</span> [Sender](#h:d1532cbd-394f-42b8-9331-0704d91794b7)

</div>
<!--endtoc-->



## <span class="section-num">1</span> Data structures {#h:3a9ecefa-c67a-4512-a5d4-69adb4c9a0d6}


### <span class="section-num">1.1</span> Slice Table {#h:1223137a-c35a-4cca-8922-447664c4cc0b}

```plantuml
@startuml
class SliceTable {
+ NodeTag type
+ int localSlice
+ int numSlices
+ ExecSlice slices
+ bool hasMotions
+ int instrument_options
+ uint32 ic_instance_id
}

note right of SliceTable::localSlice
Index of the slice to execute
end note

note right of SliceTable::slices
Array of slices, indexed by SliceIndex
end note


note right of SliceTable::hasMotions
Are there any Motion nodes anywhere in the plan?
end note


class ExecSlice {
+ int sliceIndex
+ int rootIndex
+ int parentIndex
+ int planNumSegments
+ List children
+ GangType gangType
+ List segments
+ struct primaryGang
+ List primaryProcesses
+ Bitmapset processesMap
}

note right of ExecSlice::primaryProcesses
A list of CDBProcess nodes corresponding to the worker
processes allocated to implement this plan slice.
end note

note right of ExecSlice::processesMap
A bitmap to identify which QE should execute this slice
end note

SliceTable o-- ExecSlice

class Gang {
+ GangType type
+ int size
+ struct db_descriptors
+ bool allocated
}

note right of Gang::db_descriptors
Array of QEs/segDBs that make up this gang.
Sorted by segment index.
end note


ExecSlice *-- Gang

class CdbProcess {
+ NodeTag type
+ char listenerAddr
+ int listenerPort
+ int pid
+ int contentid
+ int dbid
}

ExecSlice o-- CdbProcess



class SegmentDatabaseDescriptor {
+ struct segment_database_info
+ int segindex
+ int conn
+ int motionListener
+ int backendPid
+ char whoami
+ int isWriter
+ int identifier
}

Gang o-- SegmentDatabaseDescriptor



class CdbComponentDatabases {
+ CdbComponentDatabaseInfo segment_db_info
+ int total_segment_dbs
+ CdbComponentDatabaseInfo entry_db_info
+ int total_entry_dbs
+ int total_segments
+ int fts_version
+ int expand_version
+ int numActiveQEs
+ int numIdleQEs
+ int qeCounter
+ List freeCounterList
}

note right of CdbComponentDatabaseInfo::segment_db_info
array of  SegmentDatabaseInfo for segment databases
end note

note right of CdbComponentDatabaseInfo::entry_db_info
array of  SegmentDatabaseInfo for entry databases
end note


class CdbComponentDatabaseInfo {
+ struct config
+ CdbComponentDatabases cdbs
+ int hostSegs
+ List freelist
+ int numIdleQEs
+ int numActiveQEs
}

note right of CdbComponentDatabaseInfo::cdbs
point to owners
end note

CdbComponentDatabases o-- CdbComponentDatabaseInfo



class GpSegConfigEntry {
+ int dbid
+ int segindex
+ char role
+ char preferred_role
+ char mode
+ char status
+ int port
+ char hostname
+ char address
+ char datadir
+ char hostip
+ char hostaddrs
}

CdbComponentDatabaseInfo o-- GpSegConfigEntry

SegmentDatabaseDescriptor o-- CdbComponentDatabaseInfo

@enduml
```

<a id="org5c1fb5c"></a>

2![](/ox-hugo/SliceTable-gen-e3de35057480a3f67a59b81aff9b1a55.png)


#### <span class="section-num">1.1.1</span> SliceTable {#h:529c866c-a184-44fa-be5a-62154cd53821}

`SliceTable` : 由 Slice 组成的链表， Slice 组织成三类：

-   root slices: <br />
    Slice 0

-   motion slices <br />
    `1 ~ n` 为 motion slices, 每个 slice 的根为 sending motion

-   其余为 initPlans


#### <span class="section-num">1.1.2</span> ExecSlice {#h:384cef30-3dad-4d92-9f0a-a892701e42b5}

-   MPP 中，计划树 (PlanTree) 被切分成多个单独的执行单元 （又称 **Slice** ）
-   一个 Slice 在进程组 （process gang） 的一个 worker 上执行


## <span class="section-num">2</span> PostgresMain {#h:9a8ad5db-0d8c-4bef-bbc8-4a27cd48ef45}


### <span class="section-num">2.1</span> Call graph (QE): {#h:554948eb-a908-49ea-89f4-324ac2f96fe7}

```text
Main()
  PostmasterMain()
    ServerLoop()
      BackendStartup()
        BackendRun()
          PostgresMain()
            InitPostgres()
              cdb_setup()
                ensureInterconnectAddress()
                InitMotionLayerIPC()
                  InitMotionTCP()
                      setupTCPListeningSocket()
            sendQEDetails()
```

`setupTCPListeningSocket()` 会由操作系统分配端口，并返回上层。并在 `InitMotionlayerIPC()` 中存储在全局变量
`Gp_listener_port` 中，并随后在函数 `sendQEDetails(void)` 中将端口信息 "qe_listener_port"  发送给 client 。


### <span class="section-num">2.2</span> QD {#h:e1a256d3-301b-4ba1-be57-8749befb730f}

```text
standard_ExecutorStart()
  CdbDispatchPlan()
    cdbdisp_dispatchX()
      AssignGangs()
        AssignWriterGangFirst()
          AllocateGang()
            cdbgang_createGang()
              cdbgang_createGang_async()
                cdbconn_doConnectComplete()
                  cdbconn_get_motion_listener_port()
          setupCdbProcessList()
```

函数  [AssignGangs()](~/Work/gpdb/src/backend/executor/execUtils.c)  在 QD 上执行， 将 Executor 工厂分配的 gangs 分配给 slice table 中的 slices.
从而构建全局的 slice table 。该过程分成了两步：

-   [AssignWriterGangFirst()](~/Work/gpdb/src/backend/executor/execUtils.c)
    -   [AllocateGang()](~/Work/gpdb/src/backend/executor/execUtils.c)

        登录到涉及到的 segDB ， 生成会话 （进程）， 从而创建进程组。

        -   [cdbgang_createGang_async()](~/Work/gpdb/src/backend/cdb/dispatcher/cdbgang_async.c)
            -   [buildGangDefinition()](~/Work/gpdb/src/backend/cdb/dispatcher/cdbgang.c) <br />
                读取 GP 系统表， 构建 `CdbComponentDatabases` ，随后将该结构转换成为 `Gang` 结构， 并初始化其中与 connection 无关的成员。随后对其中每个 seg 发起连接。

            -   [cdbconn_doConnectStart()](~/Work/gpdb/src/backend/cdb/dispatcher/cdbgang_async.c) <br />
                填充 keywords 之后，使用 `PQconnectStartParams()` 发起连接。

            -   [cdbconn_doConnectComplete()](~/Work/gpdb/src/backend/cdb/dispatcher/cdbgang_async.c) <br />
                完成连接
                -   [cdbconn_get_motion_listener_port()](~/Work/gpdb/src/backend/cdb/dispatcher/cdbconn.c) <br />
                    与 QE 建立连接后，读取 "qe_listener_port"

    -   [setupCdbProcessList()](~/Work/gpdb/src/backend/executor/execUtils.c) <br />
        Create a list of CdbProcess and initialize with Gang information.

-   [InventorySliceTree()](~/Work/gpdb/src/backend/executor/execUtils.c) <br />
    Helper for AssignGangs takes a simple inventory of the gangs required by a slice tree.


## <span class="section-num">3</span> Receiver {#h:c37bea35-e940-428c-befb-d19fa51a1295}


## <span class="section-num">4</span> Sender {#h:d1532cbd-394f-42b8-9331-0704d91794b7}

