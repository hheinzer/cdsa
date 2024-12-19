#include "array.h"

#include <stdio.h>

// integer comparison function
int intcmp(const void *a, const void *b)
{
    const int *ia = a, *ib = b;
    return (*ia > *ib) - (*ia < *ib);
}

int main(void)
{
    // create an array that stores integers
    Array a = array_create(10, sizeof(int), intcmp);

    // append integers 0 through 9
    for (int i = 0; i < 10; ++i) array_append(&a, &i);

    // insert integer 33 at position 3
    array_insert(&a, 3, (int[]){33});

    // insert integer -33 at third-to-last position
    array_insert(&a, -3, (int[]){-33});

    // create a copy of the array
    Array b = array_copy(&a);

    // remove the middle item from array a
    free(array_pop(&a, a.size / 2));

    // remove the item with a value of 33 from array a
    free(array_remove(&b, (int[]){33}));

    // sort items of array a
    array_sort(&a, 0);

    // reverse the order of array b
    array_reverse(&b);

    // print items of array a
    printf("a = [");
    ArrayForEach(item, &a) printf("%d, ", *(int *)item->data);
    printf("]\n");

    // print items of array b in reverse
    printf("reverse(b) = [");
    ArrayForEachReverse(item, &b) printf("%d, ", *(int *)item->data);
    printf("]\n");

    // print index of item with a value of 33
    printf("a.index(33) = %zu\n", array_index(&a, (int[]){33}));
    printf("b.index(33) = %zu\n", array_index(&b, (int[]){33}));

    // print address of item with a value of 33
    printf("a.find(33) = %p\n", array_find(&a, (int[]){33}));
    printf("b.find(33) = %p\n", array_find(&b, (int[]){33}));

    // print number of occurrences of item with a value of 33
    printf("a.count(33) = %ld\n", array_count(&a, (int[]){33}));
    printf("b.count(33) = %ld\n", array_count(&b, (int[]){33}));

    // clear items
    array_clear(&a);
    array_clear(&b);
}
