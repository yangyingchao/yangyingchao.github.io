<!-- GEN-TITLE: GiST Indexing -->

# Table of Contents

1.  [GiST: A Generalized Search Tree for Secondary Storage](#org5d3339c)
    1.  [Introduction](#orgb40e95b)
    2.  [The Generalized Search Tree](#orga89cf3d)
    3.  [The Keys to the GiST](#orge001fbf)
    4.  [The 4 Methods](#orgb17b8ac)
2.  [Research Paper: PDF](#org9057ac2)

Based on <https://github.com/postgres/postgres/tree/master/src/backend/access/gist> and
<http://gist.cs.berkeley.edu> .


<a id="org5d3339c"></a>

# GiST: A Generalized Search Tree for Secondary Storage


<a id="orgb40e95b"></a>

## Introduction

传统数据库使用 B+ 树，B+ Tree 可以有效降低 IO，足以应对处理字符和数字。
但现在数据库中存储的数据类型越来越多，为更好支持这些类型，当前（back to
1995&#x2026;） 有两个主流研究方向：

-   特需化的搜索树 （Specialized Search Trees）   
    人们为了解决特定问题e 人开发了很多不同的树，如：为解决空间搜索 (spatial
    search) 而开发 + R-Trees。缺点是，每增加一种新的类型，都要添加新的算法和数据
    结构，开发和维护代价都比较高。

-   搜索树支持可扩展数据类型：  
    该方法可以让现有的树支持更多中数据类型，但无法增加其支持的查询的种类，例如：
    -   `B+-Tree` ： 只要数据支持线性排序，就可以使用 B+ 树来检索数据，这增加了数
        据的可拓展性，但对查询而言， **只有** 包含等值条件和范围条件的查询才有利。
    
    -   `R Tree` ：类似，只有包含等值、重叠以及  containment 的查询才有利。

In the beginning there was the B-tree. All database search trees since the B-tree have been
variations on its theme. Recognizing this, we have developed a new kind of index called a
Generalized Search Tree (GiST), which provides the functionality of all these trees in a single
package. The GiST is an extensible data structure, which allows users to develop indices over any
kind of data, supporting any lookup over that data. This package unifies a number of popular
search trees in one data structure (the long list of potentials includes R-trees, B+-trees,
hB-trees, TV-trees, Ch-Trees, partial sum trees, ranked B+-trees, and many, many others),
eliminating the need to build multiple search trees for handling diverse applications. In addition
to unifying all these structures, the GiST has one key feature that previous trees lacked: both
data and query extensibility.

**通用搜索树:** 在数据类型和支持的查询方法上都提供了良好的扩展性，其两个特点：

-   可在一个包中提供多种树所能提供的功能，可让用户对任何数据建立索引，进行查询:   
    R-trees, B+-trees, hB-trees, TV-trees, Ch-Trees, partial sum trees, ranked B+-trees &#x2026;
-   扩展性：与自定义函数结合，提供其他纬度的搜索、查询。


<a id="orga89cf3d"></a>

## The Generalized Search Tree

定义其数据结构、不变性 (invariant properties) ，扩展方法和内置算法。


### Structure

A GiST is a balanced tree of variable fanout between kM and M

-   平衡树
-   root 节点扇出 (每个节点中的指针数量) 为 `2～M` 。
-   其他节点的扇出在 `kM` 和 `M` 之间可变 ：   
    `k` 为常量，称为 minimum fill factor of the tree：   <img src="/tmp/2019-04-28-GiST-Indexing_e68ec63adbe3c72c84be82f5449ba01bb9a91e40.png" alt="2019-04-28-GiST-Indexing_e68ec63adbe3c72c84be82f5449ba01bb9a91e40.png" />，  **BUT：** M 是个什么。。。
-   leaf node 中包含 `(p, ptr)` 对：
    -   p: predicate, use as key
    -   ptr: identifier of some tuple in database


<a id="orge001fbf"></a>

## The Keys to the GiST

和 B tree 相比：

-   共同点：
    -   都是平衡树
    -   都包含 `<Key, Pointer>` 对

-   不同点： Key 不同
    -   B tree 中 Key 通常为 INT
    -   GiST 中 Key 为用户用户定义的类的一个成员变量， represents some property that is true of all
        data items reachable from the pointer associated with the key.


<a id="orgb17b8ac"></a>

## The 4 Methods

To make a GiST work, you just have to figure out what to represent in the
keys, and then write 4 methods for the key class that help the tree do insertion, deletion, and search.

Here are the 4 methods required of the key class to make a GiST work:


### Consistent

This method lets the tree search correctly. Given a key p on a tree page, and user query q, the
Consistent method should return NO if it is certain that both p and q cannot be true for a given
data item. Otherwise it should return MAYBE.

该函数用于进行搜索。

-   函数输入为：
    -   `p` &#x2013; key of page
    -   `q` &#x2013; user query

-   输出 `/func(p, q)/` 为：
    -   `NO` ： 确定不包含
    -   `MAYBE:` 其他条件


### Union

This method consolidates information in the tree. Given a set S of entries,
this method returns a new key p which is true for all the data items below
S. A simple way to implement Union is to return a predicate equivalent to the
disjunction of the keys in S, i.e. "p1 or p2 or p3 or&#x2026;".

该函数用于联合树中的信息（节点）。

-   输入:  `S` &#x2013; 多个记录的集合
-   输出： 一个 key， 该 key 下集合内所有 item 都满足条件。


### Penalty

Given a choice of inserting a new data item in a subtree rooted by entry
`<p,tr>` , return a number representing how bad it would be to do that. Items will
get inserted down the path of least Penalty in the tree.

惩罚函数，或者称估值函数。

-   输入： `<p, tr>` 为一个子树
-   输出：一个数值，用于表示将新数据插入该子树是否合适。

GiST 会把新数据插入到惩罚最小的子树中。


### PickSplit

As in a B-tree, pages in a GiST occasionally need to be split upon insertion
of a new data item. This routine is responsible for deciding which items go to
the new page, and which ones stay on the old page.

发生分裂时，使用此函数来决定：哪些数据放到新页中，哪些数据保留在原页中。


<a id="org9057ac2"></a>

# Research Paper: [PDF](../assets/data/gist.ps.pdf)

