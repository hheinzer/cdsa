#include "arena.h"

#include <stddef.h>
#include <stdio.h>

#include "hexdump.h"

#define malloc(size) arena_alloc(&arena, 1, size, alignof(max_align_t), 0)
#define calloc(count, size) arena_alloc(&arena, count, size, alignof(max_align_t), 1)
#define realloc(ptr, size) arena_realloc(&arena, ptr, size, alignof(max_align_t))
#define strdup(str) strcpy(malloc(strlen(str) + 1), str)

int main(void)
{
    Arena arena = arena_create(1 << 10);

    char *a = strdup("Hello");
    printf("a = '%s'\n", a);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    char *b = strdup("foo");
    printf("a = '%s'\n", a);
    printf("b = '%s'\n", b);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    b = realloc(b, strlen(b) + 4 + 1);
    strcat(b, " bar");
    printf("a = '%s'\n", a);
    printf("b = '%s'\n", b);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    a = realloc(a, strlen(a) + 8 + 1);
    strcat(a, ", World!");
    printf("a = '%s'\n", a);
    printf("b = '%s'\n", b);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    const char *c = strdup("Arena");
    printf("a = '%s'\n", a);
    printf("b = '%s'\n", b);
    printf("c = '%s'\n", c);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    arena_clear(&arena);
}
