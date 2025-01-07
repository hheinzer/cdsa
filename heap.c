#include "heap.h"

#include <stdio.h>

int intcmp(const void *_a, const void *_b, void *) {
    const int *a = _a, *b = _b;
    return (*a > *b) - (*a < *b);
}

int main(void) {
    Arena arena = arena_create(1 << 20);

    Heap a = heap_create(&arena, sizeof(int), intcmp);
    for (int i = 9; i >= 0; i--) {
        heap_push(&a, &i, 0);
    }

    Heap b = heap_clone(&a, 0, 0);

    heap_pop(&b, 0);

    printf("a = [");
    heap_for_each(item, &a) {
        printf("%d, ", *(int *)item->data);
    }
    printf("]\n");

    printf("b = [");
    for (HeapItem *items = heap_items(&b, 0), *item = items; item < items + b.length; item++) {
        printf("%d, ", *(int *)item->data);
    }
    printf("]\n");

    printf("a.peek() = %d\n", *(int *)heap_peek(&a));
    printf("b.peek() = %d\n", *(int *)heap_peek(&b));

    arena_destroy(&arena);
}
