#include "arena.h"

#include <stddef.h>
#include <stdio.h>

#include "dump.h"

#define calloc(a, p, n) arena_alloc(a, n, sizeof(*(p)), alignof(typeof(*(p))), 0)
#define realloc(a, p, n) arena_realloc(a, p, n, sizeof(*(p)), alignof(typeof(*(p))))
#define memdup(a, p, n) arena_memdup(a, p, n, sizeof(*(p)), alignof(typeof(*(p))))
#define strdup(a, s) memdup(a, s, strlen(s) + 1)
#define strapp(a, s, ss) strcat(realloc(a, s, strlen(s) + strlen(ss) + 1), ss)

void temporary(Arena arena);
void permanent(Arena *arena);

int main(void) {
    Arena arena = arena_create(1 << 20);

    char *s = strdup(&arena, "Hello");
    dump(arena.data, arena.begin);
    printf("\n");

    Arena scratch = arena_scratch(&arena, 1 << 4);
    strdup(&scratch, "scratch");
    dump(arena.data, arena.begin);
    printf("\n");

    temporary(arena);
    dump(arena.data, arena.begin);
    printf("\n");

    permanent(&arena);
    dump(arena.data, arena.begin);
    printf("\n");

    s = strapp(&arena, s, ", World!");
    dump(arena.data, arena.begin);

    arena_destroy(&arena);
}

void temporary(Arena arena) {
    char *s = strdup(&arena, "foo");
    dump(arena.data, arena.begin);
    printf("\n");

    strapp(&arena, s, ", bar");
    dump(arena.data, arena.begin);
    printf("\n");
}

void permanent(Arena *arena) {
    strdup(arena, "arena allocator");
    dump(arena->data, arena->begin);
    printf("\n");
}
