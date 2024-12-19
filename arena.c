#include "arena.h"

#include <stdio.h>

// create global memory arena replace default allocator
Arena arena = {0};
#define region_push arena_region_push(&arena)
#define region_pop arena_region_pop(&arena)
#define malloc(size) arena_malloc(&arena, size)
#define calloc(count, size) arena_calloc(&arena, count, size)
#define realloc(ptr, size) arena_realloc(&arena, ptr, size)
#define free(ptr) arena_free(&arena, ptr)
[[gnu::constructor]] void _run_before_main(void)
{
    arena = arena_create(100 * MB, alignof(size_t));
}
[[gnu::destructor]] void _run_after_main(void)
{
    arena_clear(&arena);
}

int main(void)
{
    // allocate an array in the arena
    int *a = calloc(10, sizeof(*a));
    assert(a);
    printf("sizeof(a) = %zu\n", arena_sizeof(&arena, a));
    printf("arena.size = %zu\n\n", arena.size);

    // push a region
    region_push;
    printf("arena.size = %zu\n\n", arena.size);

    // allocate another array in the arena
    int *b = calloc(10, sizeof(*b));
    assert(b);
    printf("sizeof(b) = %zu\n", arena_sizeof(&arena, b));
    printf("arena.size = %zu\n\n", arena.size);

    // reallocate array b
    b = realloc(b, 20 * sizeof(*b));
    assert(b);
    printf("sizeof(b) = %zu\n", arena_sizeof(&arena, b));
    printf("arena.size = %zu\n\n", arena.size);

    // pop a region
    region_pop;
    printf("arena.size = %zu\n\n", arena.size);

    // allocate another array in the arena
    int *c = calloc(10, sizeof(*c));
    assert(c);
    printf("sizeof(c) = %zu\n", arena_sizeof(&arena, c));
    printf("arena.size = %zu\n\n", arena.size);
}
