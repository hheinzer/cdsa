#include "set.h"

#include <stdio.h>

static constexpr long mega_byte = 1 << 20;

#define countof(A) ((long)(sizeof(A) / sizeof(*(A))))

int main(void) {
    Arena arena = arena_create(mega_byte);

    Set set = set_create(&arena);

    char *key[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    for (long i = 0; i < countof(key); i++) {
        set_insert(&set, key[i], 0);
    }

    Set clone = set_clone(&set, nullptr);

    set_remove(&clone, "six", 0);
    set_insert(&clone, "ten", 0);

    printf("a = {");
    set_for_each(item, &set) {
        printf("%s, ", (char *)item->key.data);
    }
    printf("}\n");

    printf("b = {");
    SetItem *items = set_items(&clone, nullptr);
    for (SetItem *item = items; item < items + clone.length; item++) {
        printf("%s, ", (char *)item->key.data);
    }
    printf("}\n");

    printf("set.find(six) = %d\n", set_find(&set, "six", 0));
    printf("clone.find(six) = %d\n", set_find(&clone, "six", 0));

    printf("set.find(ten) = %d\n", set_find(&set, "ten", 0));
    printf("clone.find(ten) = %d\n", set_find(&clone, "ten", 0));

    arena_destroy(&arena);
}
