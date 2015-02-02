#include "merge_sort.h"

static long sts = 0;
static slist* get_middle_list(slist* h)
{
    sts ++;
    slist* p = h;
    slist* q = h;
    slist* k = q;
    while (p) {
        p = p->next;
        if (p) {
            p = p->next;
        }
        k = q;
        q = q->next;
    }
    return k;
}

static slist* merge(slist*a, slist* b)
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


slist* merge_sort_3(slist* h)
{
    sts ++;
    if (!h || !h->next)
        return h;

    slist* s = get_middle_list(h);
    slist* nh = s->next;
    s->next = NULL;
    return merge(merge_sort_3(h), merge_sort_3(nh));
}

long get_sts_3()
{
    return sts;
}
