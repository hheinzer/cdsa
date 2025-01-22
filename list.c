#include "list.h"

#include <stdio.h>

static constexpr long mega_byte = 1 << 20;

int intcmp(const void *_lhs, const void *_rhs, void *) {
    const int *lhs = _lhs;
    const int *rhs = _rhs;
    return (*lhs > *rhs) - (*lhs < *rhs);
}

int main(void) {
    Arena arena = arena_create(mega_byte);

    List list = list_create(&arena, sizeof(int), intcmp);

    for (int i = 0; i < 10; i++) {
        list_append(&list, &i);
    }

    list_insert(&list, 3, &(int){33});

    list_insert(&list, -3, &(int){-33});

    List clone = list_clone(&list, nullptr);

    list_pop(&list, list.length / 2);

    list_remove(&clone, &(int){33});

    list_sort(&list, nullptr);

    list_reverse(&clone);

    printf("a = [");
    list_for_each(item, &list) {
        printf("%d, ", *(int *)item->data);
    }
    printf("]\n");

    printf("b = [");
    ListItem *items = list_items(&clone, nullptr);
    for (ListItem *item = items; item < items + clone.length; item++) {
        printf("%d, ", *(int *)item->data);
    }
    printf("]\n");

    printf("list.get(10) = %d\n", *(int *)list_get(&list, 10));
    printf("clone.get(10) = %d\n", *(int *)list_get(&clone, 10));

    printf("list.index(33) = %ld\n", list_index(&list, (int[]){33}));
    printf("clone.index(33) = %ld\n", list_index(&clone, (int[]){33}));

    printf("list.find(33) = %p\n", list_find(&list, (int[]){33}));
    printf("clone.find(33) = %p\n", list_find(&clone, (int[]){33}));

    printf("list.count(33) = %ld\n", list_count(&list, (int[]){33}));
    printf("clone.count(33) = %ld\n", list_count(&clone, (int[]){33}));

    arena_destroy(&arena);
}
