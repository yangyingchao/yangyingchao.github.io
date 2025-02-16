# Data Structures in PG


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Hash &amp; TapeSet](#h:cc825ae9-05f0-4ce8-a53c-a0697f0c1d99)
- <span class="section-num">2</span> [Slot &amp; tuple](#h:c2c5a9a7-a0ef-4fd8-b7ec-46ada6e71726)

</div>
<!--endtoc-->



## <span class="section-num">1</span> Hash &amp; TapeSet {#h:cc825ae9-05f0-4ce8-a53c-a0697f0c1d99}

```plantuml

class LogicalTapeSet {
    + BufFile pfile
    + SharedFileSet fileset
    + int worker
    + long nBlocksAllocated
    + long nBlocksWritten
    + long nHoleBlocks
    + bool forgetFreeSpace
    + long freeBlocks
    + long nFreeBlocks
    + Size freeBlocksLen
    + bool enable_prealloc
}

class LogicalTape {
    + LogicalTapeSet tapeSet
    + bool writing
    + bool frozen
    + bool dirty
    + long firstBlockNumber
    + long curBlockNumber
    + long nextBlockNumber
    + long offsetBlockNumber
    + char buffer
    + int buffer_size
    + int max_size
    + int pos
    + int nbytes
    + long prealloc
    + int nprealloc
    + int prealloc_size
}



class BufFile {
    + int numFiles
    + File files
    + bool isInterXact
    + bool dirty
    + bool readOnly
    + FileSet fileset
    + const name
    + ResourceOwner resowner
    + int curFile
    + off_t curOffset
    + int pos
    + int nbytes
    + PGAlignedBlock buffer
}

LogicalTapeSet *-- BufFile

class HashAggSpill {
+ int npartitions
+ LogicalTape partitions
+ int64 ntuples
+ uint32 mask
+ int shift
+ hyperLogLogState hll_card
}


HashAggSpill *-- LogicalTape

LogicalTape - LogicalTapeSet
```


## <span class="section-num">2</span> Slot &amp; tuple {#h:c2c5a9a7-a0ef-4fd8-b7ec-46ada6e71726}

```plantuml

class TupleTableSlot {
  + NodeTag type
  + int tts_flags
  + AttrNumber tts_nvalid
  + const tts_ops
  + TupleDesc tts_tupleDescriptor
  + int tts_values
  + int tts_isnull
  + int tts_mcxt
  + ItemPointerData tts_tid
  + int tts_tableOid
}




class MinimalTupleData {
  + int t_len
  + char mt_padding
  + int t_infomask2
  + int t_infomask
  + int t_hoff
  + int t_bits
}


class HeapTupleHeaderData {
  + union t_choice
  + ItemPointerData t_ctid
  + int t_infomask2
  + int t_infomask
  + int t_hoff
  + int t_bits
}

class union {
  + HeapTupleFields t_heap
  + DatumTupleFields t_datum
}


HeapTupleHeaderData *-- union

class MinimalTupleTableSlot {
  + TupleTableSlot base
  + HeapTuple tuple
  + MinimalTuple mintuple
  + HeapTupleData minhdr
  + int off
}

class HeapTupleData {
  + int t_len
  + ItemPointerData t_self
  + int t_tableOid
  + HeapTupleHeader t_data
}


class VirtualTupleTableSlot {
  + TupleTableSlot base
  + char data
}

class HeapTupleTableSlot {
  + TupleTableSlot base
  + HeapTuple tuple
  + int off
  + HeapTupleData tupdata
}

class BufferHeapTupleTableSlot {
  + HeapTupleTableSlot base
  + Buffer buffer
}


TupleTableSlot <|-- MinimalTupleTableSlot
TupleTableSlot <|-- VirtualTupleTableSlot
TupleTableSlot <|-- HeapTupleTableSlot
HeapTupleTableSlot <|-- BufferHeapTupleTableSlot


MinimalTupleTableSlot *-- MinimalTupleData
HeapTupleTableSlot *-- HeapTupleData
HeapTupleData *-- HeapTupleHeaderData

```

