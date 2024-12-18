#include "set.h"

#include <stdio.h>

#include "hash.h"

int main(void)
{
    // create a set that stores integers
    Set a = set_create(10, sizeof(int), 0.75, memhash_fnv1a, memcpy, free);

    // insert key-integer pairs 0 through 9
    for (long i = 0; i < 10; ++i) set_insert(&a, &i, 0);

    // create a copy of the set
    Set b = set_copy(&a);

    // remove the item with key "six" from list a
    free(set_remove(&b, (int[]){6}));

    // print items of set a
    printf("a = {");
    SetForEach(item, &a) printf("%d, ", *(int *)item->data);
    printf("}\n");

    // print items of set b
    printf("b = {");
    SetForEach(item, &b) printf("%d, ", *(int *)item->data);
    printf("}\n");

    // print address of item with key "six"
    printf("a.find(6) = %p\n", set_find(&a, (int[]){6}));
    printf("b.find(6) = %p\n", set_find(&b, (int[]){6}));

    // clear items
    set_clear(&a);
    set_clear(&b);
}
