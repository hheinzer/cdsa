#include "arena.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#include "hexdump.h"

// create global memory arena replace default allocator
Arena arena = {0};
#define region_push arena_region_push(&arena)
#define region_pop arena_region_pop(&arena)
#define malloc(size) arena_malloc(&arena, size)
#define calloc(count, size) arena_calloc(&arena, count, size)
#define realloc(ptr, size) arena_realloc(&arena, ptr, size)
#define free(ptr) arena_free(&arena, ptr)
#define memdup(ptr, size) memcpy(arena_malloc(&arena, size), ptr, size)
#define strdup(str) strcpy(arena_malloc(&arena, strlen(str) + 1), str)
[[gnu::constructor(1)]] void _arena_create(void)
{
    arena = arena_create(MB, alignof(max_align_t));
}
[[gnu::destructor(1)]] void _arena_clear(void)
{
    arena_clear(&arena);
}

int main(void)
{
    // allocate a string
    const char *a = strdup("first string");
    printf("sizeof(%s) = %zu\n", a, arena_sizeof(&arena, a));
    printf("arena.size = %zu\n\n", arena.size);

    // push a region
    region_push;
    printf("arena.size = %zu\n\n", arena.size);

    // allocate another string
    char *b = strdup("second string");
    printf("sizeof(%s) = %zu\n", b, arena_sizeof(&arena, b));
    printf("arena.size = %zu\n\n", arena.size);

    // reallocate string
    b = realloc(b, 2 * arena_sizeof(&arena, b));
    strcat(b, ", third string");
    printf("sizeof(%s) = %zu\n", b, arena_sizeof(&arena, b));
    printf("arena.size = %zu\n\n", arena.size);

    // print arena buffer
    hexdump(arena.data, arena.size);
    printf("\n");

    // pop a region
    region_pop;
    printf("arena.size = %zu\n\n", arena.size);

    // allocate another array in the arena
    const char *c = strdup("fourth string");
    printf("sizeof(%s) = %zu\n", c, arena_sizeof(&arena, c));
    printf("arena.size = %zu\n\n", arena.size);

    // print arena buffer
    hexdump(arena.data, arena.size);
}
