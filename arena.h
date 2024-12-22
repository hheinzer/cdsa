#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void *data;
    void *last;
    char *begin;
    char *end;
} Arena;

typedef enum {
    NOZERO = 1 << 1,
} ArenaFlags;

static Arena arena_create(long capacity) {
    Arena arena = {0};
    arena.data = malloc(capacity);
    arena.begin = arena.data;
    arena.end = arena.begin ? arena.begin + capacity : 0;
    return arena;
}

[[gnu::malloc, gnu::alloc_size(2, 3), gnu::alloc_align(4)]]
static void *arena_alloc(Arena *self, long count, long size, long align, int flags) {
    long available = self->end - self->begin;
    long padding = -(uintptr_t)self->begin & (align - 1);
    if (count > (available - padding) / size) {
        abort();  // out of memory or overflow
    }
    long total = count * size;
    self->last = self->begin + padding;
    self->begin += padding + total;
    return flags & NOZERO ? self->last : memset(self->last, 0, total);
}

[[gnu::malloc, gnu::alloc_size(3, 4), gnu::alloc_align(5)]]
static void *arena_realloc(Arena *self, void *ptr, long count, long size, long align) {
    if (!ptr) {
        return arena_alloc(self, count, size, align, NOZERO);
    }
    if (ptr == self->last) {
        self->begin = self->last;
        return arena_alloc(self, count, size, align, NOZERO);
    }
    if (ptr < self->data || self->last < ptr) {
        abort();
    }
    void *new = arena_alloc(self, count, size, align, NOZERO);
    long total = count * size;
    long max_total = (char *)self->last - (char *)ptr;
    return memcpy(new, ptr, total < max_total ? total : max_total);
}

static void arena_destroy(Arena *self) {
    free(self->data);
    *self = (Arena){0};
}
