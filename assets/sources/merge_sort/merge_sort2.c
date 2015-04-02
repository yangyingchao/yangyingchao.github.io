#include "merge_sort.h"
static long sts;
static  slist *merge(slist *a,  slist *b)
{
    sts ++;
    slist head, *tail = &head;
	while (a && b) {
		if (a->val <= b->val) {
			tail->next = a;
			a = a->next;
		} else {
			tail->next = b;
			b = b->next;
		}
		tail = tail->next;
	}
	tail->next = a?:b;

     return head.next;
}

slist* merge_sort_2(slist* head)
{
    sts ++;
    if (!head || !head->next)
        return head;

	slist *part[21];

	int lev;  /* index into part[] */
	int max_lev = 0;
	slist *list;
	memset(part, 0, sizeof(part));

	list = head;

	while (list) {
		slist *cur = list;
		list = list->next;
		cur->next = NULL;

		for (lev = 0; part[lev]; lev++) {
			cur = merge(part[lev], cur);
			part[lev] = NULL;
		}
		if (lev > max_lev) {
			max_lev = lev;
		}
        SHOW_LIST(cur);
		part[lev] = cur;
	}

	for (lev = 0; lev < max_lev; lev++)
		if (part[lev])
			list = merge(part[lev], list);
    SHOW_LIST(list);
    return list;
}


long get_sts_2()
{
    return sts;
}
