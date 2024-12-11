#include "list.h"

#include <stdio.h>
#include <string.h>

int intcmp(const void *a, const void *b)
{
    const int *ia = a, *ib = b;
    if (*ia < *ib) return -1;
    if (*ia > *ib) return 1;
    return 0;
}

int main(void)
{
    List a = {0};

    for (long i = 0; i < 10; ++i) list_append(&a, &i, memcpy, sizeof(int));

    list_insert(&a, 3, (int[]){33}, memcpy, sizeof(int));
    list_insert(&a, -3, (int[]){-33}, memcpy, sizeof(int));

    List b = list_copy(&a, memcpy, sizeof(int));

    free(list_pop(&a, a.size / 2));
    free(list_remove(&b, (int[]){33}, intcmp));

    list_sort(&a, intcmp);
    list_reverse(&b);

    printf("a: ");
    for (const ListItem *item = a.head; item; item = item->next)
        printf("%d%s", *(int *)item->data, (item->next ? ", " : "\n"));

    printf("b: ");
    for (const ListItem *item = b.head; item; item = item->next)
        printf("%d%s", *(int *)item->data, (item->next ? ", " : "\n"));

    printf("a.index(33) = %ld\n", list_index(&a, (int[]){33}, intcmp));
    printf("b.index(33) = %ld\n", list_index(&b, (int[]){33}, intcmp));

    printf("a.find(33) = %p\n", list_find(&a, (int[]){33}, intcmp));
    printf("b.find(33) = %p\n", list_find(&b, (int[]){33}, intcmp));

    printf("a.count(33) = %ld\n", list_count(&a, (int[]){33}, intcmp));
    printf("b.count(33) = %ld\n", list_count(&b, (int[]){33}, intcmp));

    list_clear(&a, free);
    list_clear(&b, free);
}
