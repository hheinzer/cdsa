#include "heap.h"

#include <stdio.h>

#define ABS(a) ((a) >= 0 ? (a) : -(a))

int main(void)
{
    // create a heap that stores integers
    Heap a = heap_create(10, sizeof(int));

    // append integers 0 through 9, with priority -|i - 2.1|
    for (int i = 0; i < 10; ++i) heap_push(&a, -ABS(i - 2.1), &i);

    // create a copy of the heap
    Heap b = heap_copy(&a);

    // pop highest priority item from heap b
    free(heap_pop(&b));

    // print items of array a
    printf("a = [");
    HeapForEach(item, &a) printf("%g: %d, ", item->priority, *(int *)item->data);
    printf("]\n");

    // print items of array b in reverse
    printf("b = [");
    HeapForEach(item, &b) printf("%g: %d, ", item->priority, *(int *)item->data);
    printf("]\n");

    // peek at the highest priority item
    printf("a.peek() = %d\n", *(int *)heap_peek(&a));
    printf("b.peek() = %d\n", *(int *)heap_peek(&b));

    // clear items
    heap_clear(&a);
    heap_clear(&b);
}
