#include "dict.h"

#include <stdio.h>

#include "hash.h"

int main(void)
{
    // create a dict that stores integers
    Dict a = dict_create(20, 0.5, strhash_fnv1a, sizeof(int), memcpy, free);

    // insert key-integer pairs 0 through 9
    const char *key[] = {"zero", "one", "two",   "three", "four",
                         "five", "six", "seven", "eight", "nine"};
    for (long i = 0; i < 10; ++i) dict_insert(&a, key[i], &i);

    // create a copy of the dict
    Dict b = dict_copy(&a);

    // remove the item with key "six" from list a
    free(dict_remove(&b, "six"));

    // print items of dict a
    printf("a = {");
    DictForEach(item, &a)
        printf("%s%s: %d", (item == a.bucket ? "" : ", "), item->key, *(int *)item->data);
    printf("}\n");

    // print items of dict b
    printf("b = {");
    DictForEach(item, &b)
        printf("%s%s: %d", (item == b.bucket ? "" : ", "), item->key, *(int *)item->data);
    printf("}\n");

    // print address of item with key "six"
    printf("a.find(33) = %p\n", dict_find(&a, "six"));
    printf("b.find(33) = %p\n", dict_find(&b, "six"));

    // clear items
    dict_clear(&a);
    dict_clear(&b);
}