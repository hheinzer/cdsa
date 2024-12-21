#include "arena.h"

#include <stddef.h>
#include <stdio.h>

#include "hexdump.h"

#define malloc(arena, size) arena_alloc(arena, 1, size, alignof(max_align_t), 0)
#define realloc(arena, ptr, size) arena_realloc(arena, ptr, size, alignof(max_align_t))
#define strdup(arena, str) strcpy(malloc(arena, strlen(str) + 1), str)

int main(void)
{
    // create a memory arena
    Arena arena = arena_create(1 << 10);

    // allocate a string
    char *a = strdup(&arena, "Hello");
    printf("a = '%s'\n", a);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    {  // create a temporary memory region (braces are not strictly needed)
        Arena scratch = arena;

        // allocate another string
        char *b = strdup(&scratch, "foo");
        printf("a = '%s'\n", a);
        printf("b = '%s'\n", b);
        hexdump(scratch.data, scratch.head - scratch.data);
        printf("\n");

        // resize last string
        b = realloc(&scratch, b, strlen(b) + 4 + 1);
        strcat(b, " bar");
        printf("a = '%s'\n", a);
        printf("b = '%s'\n", b);
        hexdump(scratch.data, scratch.head - scratch.data);
        printf("\n");
    }

    // resize the first string
    a = realloc(&arena, a, strlen(a) + 8 + 1);
    strcat(a, ", World!");
    printf("a = '%s'\n", a);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    // allocate another string
    const char *c = strdup(&arena, "Arena");
    printf("a = '%s'\n", a);
    printf("c = '%s'\n", c);
    hexdump(arena.data, arena.head - arena.data);
    printf("\n");

    // cleanup
    arena_clear(&arena);
}
