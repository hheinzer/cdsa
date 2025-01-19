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

static Arena arena_create(long capacity) {
    Arena arena = {0};
    arena.data = malloc(capacity);
    assert(arena.data);
    arena.begin = arena.data;
    arena.end = arena.begin + capacity;
    return arena;
}

[[gnu::malloc, gnu::alloc_size(2, 3), gnu::alloc_align(4)]]
static void *arena_malloc(Arena *self, long count, long size, long align) {
    long available = self->end - self->begin;
    long padding = -(uintptr_t)self->begin & (align - 1);
    assert(count <= (available - padding) / size);
    self->last = self->begin + padding;
    self->begin += padding + count * size;
    return self->last;
}

static void *arena_calloc(Arena *self, long count, long size, long align) {
    return memset(arena_malloc(self, count, size, align), 0, count * size);
}

static void *arena_realloc(Arena *self, void *ptr, long count, long size, long align) {
    if (!ptr) {
        return arena_malloc(self, count, size, align);
    }
    if (ptr == self->last) {
        self->begin = self->last;
        return arena_malloc(self, count, size, align);
    }
    assert(self->data <= ptr && ptr < self->last);
    void *new = arena_malloc(self, count, size, align);
    long total = count * size;
    long max_total = (char *)self->last - (char *)ptr;
    return memcpy(new, ptr, total < max_total ? total : max_total);
}

static void *arena_memcpy(Arena *, void *dest, const void *src, long size) {
    return memcpy(dest, src, size);
}

static void *arena_memdup(Arena *self, const void *src, long count, long size, long align) {
    return memcpy(arena_malloc(self, count, size, align), src, count * size);
}

static Arena arena_scratch_create(Arena *self, long capacity) {
    long available = self->end - self->begin;
    assert(available >= capacity);
    self->end -= capacity;
    Arena scratch = {0};
    scratch.data = self->end;
    scratch.begin = scratch.data;
    scratch.end = scratch.begin + capacity;
    return scratch;
}

static void arena_scratch_destroy(Arena *self, Arena scratch) {
    assert(scratch.data == self->end);
    self->end = scratch.end;
}

static void arena_destroy(Arena *self) {
    free(self->data);
}
