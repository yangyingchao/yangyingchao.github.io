#include "merge_sort.h"

#define N   65536<<4

slist* generate_list()
{
    slist* h = NULL;
    int i = 0;
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        slist* n = (slist*)malloc(sizeof(slist));
        n->next = h;
        n->val = random()%65536%1024;
        h = n;
    }
    return h;
}

slist* dup_list(slist* in)
{
    slist *h = NULL, **tail = &h;
    while (in) {
        slist* n = (slist*)malloc(sizeof(slist));
        n->val = in->val;
        n->next = NULL;
        *tail = n;
        tail = &n->next;
        in = in->next;
    }

    return h;
}

long get_time_ms()
{
    struct timespec res;
    int ret = clock_gettime(CLOCK_MONOTONIC, &res);
    return ret == -1 ? 0: res.tv_sec*1000+res.tv_nsec/1000000;
}

static long ts = 0;

void print_time()
{
    long t = get_time_ms();
    if (t)
        printf ("Cost: %ld ms\n", t - ts);
}

int main(int argc, char *argv[])
{

    slist* h = generate_list();
    slist* h2 = dup_list(h);
    slist* h3 = dup_list(h);

    SHOW_LIST(h);

    printf ("1....");
    printf ("Sorting...\n");


    ts = get_time_ms();
    h = merge_sort_1(h);
    print_time();
    printf ("Sorting Done: %ld...\n", get_sts_1());

    printf ("2....");
    printf ("Sorting...\n");

    ts = get_time_ms();
    h = merge_sort_2(h2);
    print_time();
    printf ("Sorting Done: %ld...\n", get_sts_2());

    printf ("3....");
    printf ("Sorting...\n");

    ts = get_time_ms();
    h = merge_sort_3(h3);
    print_time();
    printf ("Sorting Done: %ld...\n", get_sts_3());


    SHOW_LIST(h);
    return 0;
}

/*
  1....Sorting...
  Cost: 745286, 745 ms
  Sorting Done...
  2....Sorting...
  Cost: 570641, 571 ms
  Sorting Done...
  3....Sorting...
  Cost: 677738, 678 ms
  Sorting Done...

 */
