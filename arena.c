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
void parallel(Arena arena);

int main(void) {
    Arena arena = arena_create(1 << 20);
    Arena scratch = arena_scratch(&arena, 16);

    char *s = strdup(&arena, "arena");
    dump(arena.data, arena.begin);
    printf("\n");

    strdup(&scratch, "scratch");
    dump(scratch.data, scratch.begin);
    printf("\n");

    temporary(arena);
    dump(arena.data, arena.begin);
    printf("\n");

    permanent(&arena);
    dump(arena.data, arena.begin);
    printf("\n");

    parallel(arena);
    dump(arena.data, arena.begin);
    printf("\n");

    s = strapp(&arena, s, " allocator");
    dump(arena.data, arena.begin);

    arena_destroy(&arena);
}

void temporary(Arena arena) {
    char *s = strdup(&arena, "tempo");
    dump(arena.data, arena.begin);
    printf("\n");

    strapp(&arena, s, "rary");
    dump(arena.data, arena.begin);
    printf("\n");
}

void permanent(Arena *arena) {
    strdup(arena, "permanent");
    dump(arena->data, arena->begin);
    printf("\n");
}

void parallel(Arena arena) {
#pragma omp parallel num_threads(3)
    {
        Arena thread = arena_thread(&arena);
        char *s = calloc(&thread, s, 16);
        sprintf(s, "thread %d", omp_get_thread_num());
#pragma omp critical
        {
            dump(thread.data, thread.begin);
            printf("\n");
        }
    }
}
