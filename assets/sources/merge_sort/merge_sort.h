#ifndef _MERGE_SORT_H_
#define _MERGE_SORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef struct _slist
{
    struct _slist* next;
    int   val;
} slist;


#define SLIST_FOR_EACH(H, V)                    \
    for (V = H;                         \
         V != NULL;                             \
         V=V->next)                             \

slist* merge_sort_1(slist* h);
slist* merge_sort_2(slist* h);
slist* merge_sort_3(slist* h);

static inline void show_list(slist* h, const char* name)
{
    slist* p;
    int i = 1;
    printf("show list: %s\n", name);              \
    SLIST_FOR_EACH(h, p) {
        printf ("%03d, p: %p, val: %d\n", i++, p, p->val);
    }
    printf ("%s contains: %d.\n", name, --i);
}

long get_sts_1();
long get_sts_2();
long get_sts_3();

#define SHOW_LIST(X)

// #define SHOW_LIST(X)\
//     do {            \
//     if (X) show_list(X, #X);       \
//     } while (0)


#endif /* _MERGE_SORT_H_ */
