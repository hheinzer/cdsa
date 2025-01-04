#pragma once

#include <assert.h>
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

#ifdef ARENA_USE_STDLIB
#pragma message("USING STDLIB MEMORY MANAGEMENT!")
static struct {
    void **ptr;
    long len;
    long cap;
} x__alloc = {0};
#endif

static Arena arena_create(long capacity) {
#ifdef ARENA_USE_STDLIB
    x__alloc.cap = 1 << 10;
    x__alloc.ptr = calloc(x__alloc.cap, sizeof(*x__alloc.ptr));
    return (Arena){0};
#endif
    Arena arena = {0};
    arena.data = malloc(capacity);
    assert(arena.data);
    arena.begin = arena.data;
    arena.end = arena.begin + capacity;
    return arena;
}

[[gnu::malloc, gnu::alloc_size(2, 3), gnu::alloc_align(4)]]
static void *arena_alloc(Arena *self, long count, long size, long align, int flags) {
#ifdef ARENA_USE_STDLIB
    if (x__alloc.len >= x__alloc.cap) {
        x__alloc.cap <<= 1;
        x__alloc.ptr = realloc(x__alloc.ptr, x__alloc.cap * sizeof(*x__alloc.ptr));
    }
    return x__alloc.ptr[x__alloc.len++] = calloc(count, size);
#endif
    long available = self->end - self->begin;
    long padding = -(uintptr_t)self->begin & (align - 1);
    assert(count <= (available - padding) / size);
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
#ifdef ARENA_USE_STDLIB
    for (long i = x__alloc.len - 1; i >= 0; i--) {
        if (x__alloc.ptr[i] == ptr) {
            return x__alloc.ptr[i] = realloc(ptr, count * size);
        }
    }
    abort();
#endif
    if (ptr == self->last) {
        self->begin = self->last;
        return arena_alloc(self, count, size, align, NOZERO);
    }
    assert(self->data <= ptr && ptr < self->last);
    void *new = arena_alloc(self, count, size, align, NOZERO);
    long total = count * size;
    long max_total = (char *)self->last - (char *)ptr;
    return memcpy(new, ptr, total < max_total ? total : max_total);
}

static void *arena_memcpy(Arena *, void *dest, const void *src, long size) {
    return memcpy(dest, src, size);
}

static void *arena_memdup(Arena *self, const void *src, long count, long size, long align) {
    return memcpy(arena_alloc(self, count, size, align, NOZERO), src, count * size);
}

static void arena_destroy(Arena *self) {
#ifdef ARENA_USE_STDLIB
    for (long i = 0; i < x__alloc.len; i++) {
        free(x__alloc.ptr[i]);
    }
    free(x__alloc.ptr);
    return;
#endif
    free(self->data);
}
