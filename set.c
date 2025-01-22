#include "set.h"

#include <stdio.h>

static constexpr long mega_byte = 1 << 20;

#define countof(A) ((long)(sizeof(A) / sizeof(*(A))))

void set_print(const Set *set, const char *name);

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

    Set set_or_clone = set_union(&set, &clone, nullptr);
    set_print(&set_or_clone, "set_or_clone");
    Set set_and_clone = set_intersection(&set, &clone, nullptr);
    set_print(&set_and_clone, "set_and_clone");
    Set set_minus_clone = set_difference(&set, &clone, nullptr);
    set_print(&set_minus_clone, "set_minus_clone");
    Set set_xor_clone = set_symmetric_difference(&set, &clone, nullptr);
    set_print(&set_xor_clone, "set_xor_clone");

    printf("set.is_subset(clone) = %d\n", set_is_subset(&set, &clone));
    printf("set.is_superset(clone) = %d\n", set_is_superset(&set, &clone));

    arena_destroy(&arena);
}

void set_print(const Set *set, const char *name) {
    printf("%s = {", name);
    set_for_each(item, set) {
        printf("%s, ", (char *)item->key.data);
    }
    printf("}\n");
}
