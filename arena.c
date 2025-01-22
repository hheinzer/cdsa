#include "arena.h"

#include <stddef.h>
#include <stdio.h>

#include "dump.h"

static constexpr long mega_byte = 1 << 20;

#define malloc(A, P, N) arena_malloc(A, N, sizeof(*(P)), alignof(typeof(*(P))))
#define calloc(A, P, N) arena_calloc(A, N, sizeof(*(P)), alignof(typeof(*(P))))
#define realloc(A, P, N) arena_realloc(A, P, N, sizeof(*(P)), alignof(typeof(*(P))))
#define memdup(A, P, N) arena_memdup(A, P, N, sizeof(*(P)), alignof(typeof(*(P))))
#define strdup(A, S) memdup(A, S, strlen(S) + 1)
#define strapp(A, S1, S2) strcat(realloc(A, S1, strlen(S1) + strlen(S2) + 1), S2)

void temporary(Arena arena);
void permanent(Arena *arena, Arena scratch);

int main(void) {
    Arena arena = arena_create(mega_byte);

    char *string = strdup(&arena, "arena");
    dump(arena.data, arena.begin);
    printf("\n");

    temporary(arena);
    dump(arena.data, arena.begin);
    printf("\n");

    Arena scratch = arena_scratch_create(&arena, mega_byte / 2);

    permanent(&arena, scratch);
    dump(arena.data, arena.begin);
    printf("\n");

    arena_scratch_destroy(&arena, scratch);

    strapp(&arena, string, " allocator");
    dump(arena.data, arena.begin);

    printf("occupied: %ld\n", arena_occupied(&arena));
    printf("available: %ld\n", arena_available(&arena));
    arena_destroy(&arena);
}

void temporary(Arena arena) {
    char *string = strdup(&arena, "tempo");
    dump(arena.data, arena.begin);
    printf("\n");

    strapp(&arena, string, "rary");
    dump(arena.data, arena.begin);
    printf("\n");
}

void permanent(Arena *arena, Arena scratch) {
    char *string = strdup(arena, "perma");
    dump(arena->data, arena->begin);
    printf("\n");

    strdup(&scratch, "scratch");
    dump(scratch.data, scratch.begin);
    printf("\n");

    strapp(arena, string, "nent");
    dump(arena->data, arena->begin);
    printf("\n");
}
