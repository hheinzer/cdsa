#include "arena.h"

#include <stddef.h>
#include <stdio.h>

#include "dump.h"

#define alloc(a, n, t) arena_alloc(a, n, sizeof(t), alignof(typeof(t)), 0)
#define realloc(a, p, n, t) arena_realloc(a, p, n, sizeof(t), alignof(typeof(t)))
#define strdup(a, s) strcpy(alloc(a, strlen(s) + 1, *s), s)
#define strapp(a, s, ss) strcat(realloc(a, s, strlen(s) + strlen(ss) + 1, *s), ss)

void temporary(Arena arena);
void permanent(Arena *arena);

int main(void) {
    Arena arena = arena_create(1 << 10);

    char *s1 = strdup(&arena, "Hello");
    dump(arena.data, arena.begin);
    printf("\n");

    temporary(arena);
    dump(arena.data, arena.begin);
    printf("\n");

    permanent(&arena);
    dump(arena.data, arena.begin);
    printf("\n");

    strapp(&arena, s1, ", World!");
    dump(arena.data, arena.begin);

    arena_destroy(&arena);
}

void temporary(Arena arena) {
    char *s2 = strdup(&arena, "foo");
    dump(arena.data, arena.begin);
    printf("\n");

    strapp(&arena, s2, ", bar");
    dump(arena.data, arena.begin);
    printf("\n");
}

void permanent(Arena *arena) {
    strdup(arena, "arena allocator");
    dump(arena->data, arena->begin);
    printf("\n");
}
