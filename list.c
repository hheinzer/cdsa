#include "list.h"

#include <stdio.h>

#include "dump.h"

int intcmp(const void *_a, const void *_b, void *) {
    const int *a = _a, *b = _b;
    return (*a > *b) - (*a < *b);
}

int main(void) {
    Arena arena = arena_create(1 << 20);

    List a = list_create(&arena, sizeof(int), intcmp);
    for (int i = 0; i < 10; i++) {
        list_append(&a, &i);
    }

    list_insert(&a, 3, (int[]){33});

    list_insert(&a, -3, (int[]){-33});

    List b = list_clone(&a, &arena);

    list_pop(&a, a.length / 2);

    list_remove(&b, (int[]){33});

    list_sort(&a, 0, 0);

    list_reverse(&b);

    dump(arena.data, arena.begin);
    printf("\n");

    printf("a = [");
    list_for_each(item, &a) printf("%d, ", *(int *)item->data);
    printf("]\n");

    printf("reverse(b) = [");
    list_for_each_reverse(item, &b) printf("%d, ", *(int *)item->data);
    printf("]\n");

    printf("a.index(33) = %ld\n", list_index(&a, (int[]){33}));
    printf("b.index(33) = %ld\n", list_index(&b, (int[]){33}));

    printf("a.find(33) = %p\n", list_find(&a, (int[]){33}));
    printf("b.find(33) = %p\n", list_find(&b, (int[]){33}));

    printf("a.count(33) = %ld\n", list_count(&a, (int[]){33}));
    printf("b.count(33) = %ld\n", list_count(&b, (int[]){33}));

    arena_destroy(&arena);
}
