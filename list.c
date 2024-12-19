#include "list.h"

#include <stdio.h>

// integer comparison function
int intcmp(const void *a, const void *b)
{
    const int *ia = a, *ib = b;
    return (*ia > *ib) - (*ia < *ib);
}

int main(void)
{
    // create a list that stores integers
    List a = list_create(sizeof(int), intcmp);

    // append integers 0 through 9
    for (int i = 0; i < 10; ++i) list_append(&a, &i);

    // insert integer 33 at position 3
    list_insert(&a, 3, (int[]){33});

    // insert integer -33 at third-to-last position
    list_insert(&a, -3, (int[]){-33});

    // create a copy of the list
    List b = list_copy(&a);

    // remove the middle item from list a
    free(list_pop(&a, a.size / 2));

    // remove the item with a value of 33 from list a
    free(list_remove(&b, (int[]){33}));

    // sort items of list a
    list_sort(&a, 0);

    // reverse the order of list b
    list_reverse(&b);

    // print items of list a
    printf("a = [");
    ListForEach(item, &a) printf("%d, ", *(int *)item->data);
    printf("]\n");

    // print items of list b in reverse
    printf("reverse(b) = [");
    ListForEachReverse(item, &b) printf("%d, ", *(int *)item->data);
    printf("]\n");

    // print index of item with a value of 33
    printf("a.index(33) = %zu\n", list_index(&a, (int[]){33}));
    printf("b.index(33) = %zu\n", list_index(&b, (int[]){33}));

    // print address of item with a value of 33
    printf("a.find(33) = %p\n", list_find(&a, (int[]){33}));
    printf("b.find(33) = %p\n", list_find(&b, (int[]){33}));

    // print number of occurrences of item with a value of 33
    printf("a.count(33) = %ld\n", list_count(&a, (int[]){33}));
    printf("b.count(33) = %ld\n", list_count(&b, (int[]){33}));

    // clear items
    list_clear(&a);
    list_clear(&b);
}
