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

static clock_t ts = 0;
void print_time()
{
    clock_t c = clock() - ts;
    printf ("Cost: %d, %.f ms\n", c, ((float)c*1000)/CLOCKS_PER_SEC);
}
int main(int argc, char *argv[])
{

    slist* h = generate_list();
    slist* h2 = generate_list();
    slist* h3 = generate_list();

    SHOW_LIST(h);

    printf ("1....");
    printf ("Sorting...\n");


    ts = clock();
    h = merge_sort_1(h);
    print_time();
    printf ("Sorting Done...\n");

    printf ("2....");
    printf ("Sorting...\n");

    ts = clock();
    h = merge_sort_2(h2);
    print_time();
    printf ("Sorting Done...\n");

    printf ("3....");
    printf ("Sorting...\n");

    ts = clock();
    h = merge_sort_3(h3);
    print_time();
    printf ("Sorting Done...\n");


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
