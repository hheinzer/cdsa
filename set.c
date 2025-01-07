#include "set.h"

#include <stdio.h>

int main(void) {
    Arena arena = arena_create(1 << 20);

    Set a = set_create(&arena);

    char *key[] = {
        "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    };
    for (long i = 0; i < 10; i++) {
        set_insert(&a, key[i], 0);
    }

    Set b = set_clone(&a, 0);

    set_remove(&b, "six", 0);
    set_insert(&b, "ten", 0);

    printf("a = {");
    set_for_each(item, &a) {
        printf("%s, ", (char *)item->key.data);
    }
    printf("}\n");

    printf("b = {");
    for (SetItem *i = set_items(&b, 0), *ii = i; i < ii + b.length; i++) {
        printf("%s, ", (char *)i->key.data);
    }
    printf("}\n");

    printf("a.find(six) = %d\n", set_find(&a, "six", 0));
    printf("b.find(six) = %d\n", set_find(&b, "six", 0));

    printf("a.find(ten) = %d\n", set_find(&a, "ten", 0));
    printf("b.find(ten) = %d\n", set_find(&b, "ten", 0));

    arena_destroy(&arena);
}
