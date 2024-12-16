#include "hmap.h"

#include <stdio.h>

#include "hash.h"

int main(void)
{
    // create a hmap that stores integers
    Hmap a = hmap_create(10, 0.5, strhash_fnv1a, sizeof(int), memcpy, free);

    // insert key-integer pairs 0 through 9
    const char *key[] = {"zero", "one", "two",   "three", "four",
                         "five", "six", "seven", "eight", "nine"};
    for (long i = 0; i < 10; ++i) hmap_insert(&a, key[i], &i);

    // create a copy of the hmap
    Hmap b = hmap_copy(&a);

    // remove the item with key "six" from list a
    free(hmap_remove(&b, "six"));

    // print items of hmap a
    printf("a = {");
    HmapForEach(item, &a) printf("%s: %d, ", item->key, *(int *)item->data);
    printf("}\n");

    // print items of hmap b
    printf("b = {");
    HmapForEach(item, &b) printf("%s: %d, ", item->key, *(int *)item->data);
    printf("}\n");

    // print address of item with key "six"
    printf("a.find(six) = %p\n", hmap_find(&a, "six"));
    printf("b.find(six) = %p\n", hmap_find(&b, "six"));

    // clear items
    hmap_clear(&a);
    hmap_clear(&b);
}
