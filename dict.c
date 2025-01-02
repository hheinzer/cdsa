#include "dict.h"

#include <stdio.h>

#include "dump.h"

int main(void) {
    Arena arena = arena_create(1 << 20);

    Dict a = dict_create(&arena, sizeof(int));

    const char *key[] = {
        "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    };
    for (long i = 0; i < 10; ++i) {
        dict_insert(&a, key[i], 0, &i, 0);
    }

    Dict b = dict_clone(&a, 0);

    dict_remove(&b, "six", 0);
    dict_insert(&b, "ten", 0, (int[]){10}, 0);

    const DictItem *b_item = dict_items(&b, 0);

    dump(arena.data, arena.begin);
    printf("\n");

    printf("a = {");
    dict_for_each(item, &a) {
        printf("%s: %d, ", (char *)item->key.data, *(int *)item->data);
    }
    printf("}\n");

    printf("b = {");
    for (long i = 0; i < b.length; i++) {
        printf("%s: %d, ", (char *)b_item[i].key.data, *(int *)b_item[i].data);
    }
    printf("}\n");

    printf("a.find(six) = %p\n", dict_find(&a, "six", 0));
    printf("b.find(six) = %p\n", dict_find(&b, "six", 0));

    printf("a.find(ten) = %p\n", dict_find(&a, "ten", 0));
    printf("b.find(ten) = %p\n", dict_find(&b, "ten", 0));

    arena_destroy(&arena);
}
