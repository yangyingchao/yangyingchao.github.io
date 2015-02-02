#include "merge_sort.h"

slist* get_middle_list(slist* h)
{
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

slist* merge(slist*a, slist* b)
{
    slist* head = NULL;
    slist* tail = NULL;
    slist* p = a;
    slist* q = b;
    while (p && q) {
        if (p->val < q->val) {
            if (!head) {
                head = p;
                tail = p;
                p = p->next;
            }
            else {
                tail->next = p;
                tail = p;
                p = p->next;
            }
        }
        else {
            if (!head) {
                head = q;
                tail = q;
                q = q->next;
            }
            else {
                tail->next = q;
                tail = q;
                q = q->next;
            }
        }
    }

    if (p)
        tail->next = p;
    if (q)
        tail->next = q;

    return head;
}


slist* merge_sort_1(slist* h)
{
    if (!h->next)
        return h;

    slist* s = get_middle_list(h);
    slist* nh = s->next;
    s->next = NULL;
    return merge(merge_sort_1(h), merge_sort_1(nh));
}
