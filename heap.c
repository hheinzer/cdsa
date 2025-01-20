#include "heap.h"

#include <stdio.h>

static constexpr long mega_byte = 1 << 20;

int intcmp(const void *_a, const void *_b, void *) {
    const int *a = _a;
    const int *b = _b;
    return (*a > *b) - (*a < *b);
}

int main(void) {
    Arena arena = arena_create(mega_byte);

    Heap heap = heap_create(&arena, sizeof(int), intcmp);

    for (int i = 9; i >= 0; i--) {
        heap_push(&heap, &i, nullptr);
    }

    Heap clone = heap_clone(&heap, nullptr, nullptr);

    heap_pop(&clone, nullptr);

    printf("a = [");
    heap_for_each(item, &heap) {
        printf("%d, ", *(int *)item->data);
    }
    printf("]\n");

    printf("b = [");
    HeapItem *items = heap_items(&clone, nullptr);
    for (HeapItem *item = items; item < items + clone.length; item++) {
        printf("%d, ", *(int *)item->data);
    }
    printf("]\n");

    printf("heap.peek() = %d\n", *(int *)heap_peek(&heap));
    printf("clone.peek() = %d\n", *(int *)heap_peek(&clone));

    arena_destroy(&arena);
}
