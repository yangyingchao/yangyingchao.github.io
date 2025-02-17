# The mapping of oid and relfilenode in PG


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Relfilenode of ordinary table](#h:1f7ebd85-1d1f-499e-9871-86c20f4dee99)
- <span class="section-num">2</span> [Relfilenode of Nail tables](#h:26af2114-0247-479c-acda-e29244cda12e)
- <span class="section-num">3</span> [Nail table Relfilenode storage mechanism](#h:e436403d-4da6-4ab0-945c-547dc01b1c7d)
- <span class="section-num">4</span> [Summary](#h:b40df967-b768-4822-96e4-8277122284f4)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://www.highgo.ca/2021/01/12/the-mapping-of-oid-and-relfilenode-in-pg/

A table in PostgreSQL has a relfilenode value, which specifies the file name of the table on disk (except foreign table
and partition table). In general, this value can be found in the relfilenode field of the pg_class table, but there are
some specific tables whose query result in the relfilenode field is 0. This blog will explore the kernel processing of
these special tables relfilenode.


## <span class="section-num">1</span> Relfilenode of ordinary table {#h:1f7ebd85-1d1f-499e-9871-86c20f4dee99}

When a normal table is created in PostgreSQL, the relfilenode value of the table is stored in pg_class system table. As
it can be seen in this example that when a table is created, it’s OID and relfilenode value are both 16808. You can also
find a file on the disk with the same name as the Relfilenode node value, in-fact the data inserted for the table is
actually stored in the same file.

postgres=# create table t2(i int);
CREATE TABLE
postgres=# select oid,relname,relfilenode from pg_class where relname = 't2';
  oid  | relname | relfilenode
-------~~---------~~--------&#x2013;&#x2014;
 16808 | t2      |       16808
(1 row)

postgres=# \q
movead@movead-PC:/h2/pgpgpg/bin$ ll ../data/base/12835/16808
-rw--&#x2013;&#x2014;+ 1 movead movead 0 12月 31 17:11 ../data/base/12835/16808
movead@movead-PC:/h2/pgpgpg/bin$

After we perform operations such as truncate, vacuum full, etc. on a table, the data in this table will be rewritten and
the value of relfilenode for this table will be changed. The following test shows that after truncate, the relfilenode
of the t2 table has changed from 16808 to 16811.

postgres=# truncate t2;
TRUNCATE TABLE
postgres=# select oid,relname,relfilenode from pg_class where relname = 't2';
  oid  | relname | relfilenode
-------~~---------~~--------&#x2013;&#x2014;
 16808 | t2      |       16811
(1 row)

postgres=# checkpoint;
CHECKPOINT
postgres=# \q
movead@movead-PC:/h2/pgpgpg/bin$ ll ../data/base/12835/16808
ls: 无法访问'../data/base/12835/16808': 没有那个文件或目录
movead@movead-PC:/h2/pgpgpg/bin$ ll ../data/base/12835/16811
-rw--&#x2013;&#x2014;+ 1 movead movead 0 12月 31 17:16 ../data/base/12835/16811
movead@movead-PC:/h2/pgpgpg/bin$


## <span class="section-num">2</span> Relfilenode of Nail tables {#h:26af2114-0247-479c-acda-e29244cda12e}

postgres=# select oid, relname, relfilenode,reltablespace
from pg_class
where relfilenode = 0 and relkind = 'r'
order by reltablespace;
 oid  |        relname        | relfilenode | reltablespace
------~~-----------------------~~--------&#x2013;&#x2014;+----------&#x2013;&#x2014;
 1247 | pg_type               |           0 |             0
 1255 | pg_proc               |           0 |             0
 1249 | pg_attribute          |           0 |             0
 1259 | pg_class              |           0 |             0
 3592 | pg_shseclabel         |           0 |          1664
 1262 | pg_database           |           0 |          1664
 2964 | pg_db_role_setting    |           0 |          1664
 1213 | pg_tablespace         |           0 |          1664
 1261 | pg_auth_members       |           0 |          1664
 1214 | pg_shdepend           |           0 |          1664
 2396 | pg_shdescription      |           0 |          1664
 1260 | pg_authid             |           0 |          1664
 6000 | pg_replication_origin |           0 |          1664
 6100 | pg_subscription       |           0 |          1664
(14 rows)

postgres=#

It can be seen from the above query that the relfilenode of these tables queried from the pg_class catalog is 0. Among
them, pg_type, pg_proc, pg_attribute, and pg_class are non-shared tables, and they are called Nail tables in the kernel.
The remaining tables are shared tables in the pg_global tablespace.

The purpose of relfilenode field in the pg_class table is to tell the user about the file name of a given table stored
on the disk. For example, when we query the t2 table, the system must get the relfilenode from the pg_class catalog,
then find the file on the disk, open it and scan it. But if we want to query the file name of the pg_class table on
disk, where should we find its relfilenode? A set of functional interfaces is provided in PostgreSQL to convert oid and
relfilenode.

postgres=# select pg_relation_filenode(1259);
 pg_relation_filenode

---

                16475
(1 row)

postgres=# select pg_filenode_relation(0,16475);
 pg_filenode_relation

---

 pg_class
(1 row)

postgres=# select pg_filenode_relation(0,16475)::oid;
 pg_filenode_relation

---

                 1259
(1 row)

postgres=#

By call pg_relation_filenode(), oid can be converted to relfilenode, and with pg_filenode_relation(), relfilenode can be
converted to oid.

Since the corresponding relationship between oid and relfilenode for shared and nail tables are not stored in the
pg_class table, how does PostgreSQL save this mapping relationship?

nail\_


## <span class="section-num">3</span> Nail table Relfilenode storage mechanism {#h:e436403d-4da6-4ab0-945c-547dc01b1c7d}

After research, it is found that there are pg_filenode.map files in the data directory, as shown below.

movead@movead-PC:/h2/pgpgpg/data/base/12835$ ll pg_filenode.map
-rw--&#x2013;&#x2014;+ 1 movead movead 512 12月 31 15:10 pg_filenode.map
movead@movead-PC:/h2/pgpgpg/data/base/12835$

movead@movead-PC:/h2/pgpgpg/data/global$ ll pg_filenode.map
-rw--&#x2013;&#x2014;+ 1 movead movead 512 12月 31 15:10 pg_filenode.map
movead@movead-PC:/h2/pgpgpg/data/global$

The mapping relationship between oid and relfilenode for shared tables is stored in pg_filenode.map file in the global
directory, and the mapping relationship between oid and relfilenode for nail tables in the database with OID 12835 is
stored in the 12835 directory.

The struct of the pg_filenode.map file is look like as blow:

typedef struct RelMapping
{
    Oid         mapoid;         _\* OID of a catalog \*_
    Oid         mapfilenode;    _\* its filenode number \*_
} RelMapping;

typedef struct RelMapFile
{
    int32       magic;          _\* always RELMAPPER_FILEMAGIC \*_
    int32       num_mappings;   _\* number of valid RelMapping entries \*_
    RelMapping  mappings[MAX_MAPPINGS];
    pg_crc32c   crc;            _\* CRC of all above \*_
    int32       pad;            _\* to make the struct size be 512 exactly \*_
} RelMapFile;


## <span class="section-num">4</span> Summary {#h:b40df967-b768-4822-96e4-8277122284f4}

This blog mainly explains the two different manifestations of oid and relfilenode mapping in PostgreSQL. Just remember
that using pg_relation_filenode() will always get the correct result, and query from the pg_class system table may get
the wrong result.

\*
Movead Li

Movead.Li is kernel development of Highgo Software. Since joining into Highgo Software in 2016, Movead takes the most
time on researching the code of Postgres and is good at ‘Write Ahead Log’ and ‘Database Backup And Recovery’. Base on
the experience Movead has two open-source software on the Postgres database. One is Walminer which can analyze history
wal file to SQL. The other one is pg_lightool which can do a single table or block recovery base on base backup and
walfiles or walfiles only.

Hello

Now he has joined the HighGo community team and hopes to make more contributions to the community in the future.

