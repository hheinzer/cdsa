#include "dict.h"

#include <stdio.h>

static constexpr long mega_byte = 1 << 20;

#define countof(A) ((long)(sizeof(A) / sizeof(*(A))))

int main(void) {
    Arena arena = arena_create(mega_byte);

    Dict dict = dict_create(&arena, sizeof(int));

    char *key[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    for (long i = 0; i < countof(key); i++) {
        dict_insert(&dict, key[i], 0, &i);
    }

    Dict clone = dict_clone(&dict, nullptr);

    dict_remove(&clone, "six", 0);
    dict_insert(&clone, "ten", 0, &(int){10});

    printf("a = {");
    dict_for_each(item, &dict) {
        printf("%s: %d, ", (char *)item->key.data, *(int *)item->data);
    }
    printf("}\n");

    printf("b = {");
    DictItem *items = dict_items(&clone, nullptr);
    for (DictItem *item = items; item < items + clone.length; item++) {
        printf("%s: %d, ", (char *)item->key.data, *(int *)item->data);
    }
    printf("}\n");

    printf("dict.find(six) = %p\n", dict_find(&dict, "six", 0));
    printf("clone.find(six) = %p\n", dict_find(&clone, "six", 0));

    printf("dict.find(ten) = %p\n", dict_find(&dict, "ten", 0));
    printf("clone.find(ten) = %p\n", dict_find(&clone, "ten", 0));

    arena_destroy(&arena);
}
