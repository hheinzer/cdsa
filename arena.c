#include "arena.h"

#include <stddef.h>
#include <stdio.h>

#include "dump.h"

#define malloc(A, P, N) arena_malloc(A, N, sizeof(*(P)), alignof(typeof(*(P))))
#define calloc(A, P, N) arena_calloc(A, N, sizeof(*(P)), alignof(typeof(*(P))))
#define realloc(A, P, N) arena_realloc(A, P, N, sizeof(*(P)), alignof(typeof(*(P))))
#define memdup(A, P, N) arena_memdup(A, P, N, sizeof(*(P)), alignof(typeof(*(P))))
#define strdup(A, S) memdup(A, S, strlen(S) + 1)
#define strapp(A, S1, S2) strcat(realloc(A, S1, strlen(S1) + strlen(S2) + 1), S2)

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
