#include "arena.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

#define malloc(A, P, N) arena_malloc(A, N, sizeof(*(P)), alignof(typeof(*(P))))
#define calloc(A, P, N) arena_calloc(A, N, sizeof(*(P)), alignof(typeof(*(P))))
#define realloc(A, P, N) arena_realloc(A, P, N, sizeof(*(P)), alignof(typeof(*(P))))
#define memdup(A, P, N) arena_memdup(A, P, N, sizeof(*(P)), alignof(typeof(*(P))))
#define strdup(A, S) memdup(A, S, strlen(S) + 1)
#define strapp(A, S1, S2) strcat(realloc(A, S1, strlen(S1) + strlen(S2) + 1), S2)

void temporary(Arena arena);
void permanent(Arena *arena, Arena scratch);

void dump(const void *begin, const void *end);

int main(void) {
    constexpr long mega_byte = 1 << 20;
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
    printf("\n");

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

void dump(const void *begin, const void *end) {
    ASAN_UNPOISON_MEMORY_REGION(begin, (char *)end - (char *)begin);
    constexpr int offset = 16;
    printf("%-8s  %-*s %s\n", "offset", 3 * offset, "data", "ascii");
    for (const char *byte = begin; byte < (char *)end; byte += offset) {
        printf("%08tx  ", byte - (char *)begin);
        for (int i = 0; i < offset; i++) {
            if (byte + i < (char *)end) {
                printf("%02x ", (unsigned char)byte[i]);
            }
            else {
                printf("   ");
            }
        }
        printf(" ");
        for (int i = 0; i < offset; i++) {
            if (byte + i < (char *)end) {
                printf("%c", isprint(byte[i]) ? byte[i] : '.');
            }
        }
        printf("\n");
    }
}
