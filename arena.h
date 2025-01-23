#pragma once

#include <assert.h>
#include <sanitizer/asan_interface.h>
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
    Arena arena = {};
    arena.data = malloc(capacity);
    ASAN_POISON_MEMORY_REGION(arena.data, capacity);
    assert(arena.data);
    arena.begin = arena.data;
    arena.end = arena.begin + capacity;
    return arena;
}

static long arena_occupied(const Arena *self) {
    return self->begin - (char *)self->data;
}

static long arena_available(const Arena *self) {
    return self->end - self->begin;
}

static Arena arena_scratch_create(Arena *self, long capacity) {
    assert(arena_available(self) >= capacity);
    self->end -= capacity;
    Arena scratch = {};
    scratch.data = self->end;
    scratch.begin = scratch.data;
    scratch.end = scratch.begin + capacity;
    return scratch;
}

static long x__arena_padding(const Arena *self, long align) {
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
    constexpr long min_padding = 8;
    constexpr long min_alignment = 8;
    align = align < min_alignment ? min_alignment : align;
    long padding = -(uintptr_t)self->begin & (align - 1);
    return padding < min_padding ? padding + align : padding;
#else
    return -(uintptr_t)self->begin & (align - 1);
#endif
}

[[gnu::malloc, gnu::alloc_size(2, 3), gnu::alloc_align(4)]]
static void *arena_malloc(Arena *self, long count, long size, long align) {
    long padding = x__arena_padding(self, align);
    assert(count <= (arena_available(self) - padding) / size);
    long total = count * size;
    self->last = self->begin + padding;
    ASAN_UNPOISON_MEMORY_REGION(self->last, total);
    self->begin += padding + total;
    ASAN_POISON_MEMORY_REGION(self->begin, arena_available(self));
    return self->last;
}

static void *arena_calloc(Arena *self, long count, long size, long align) {
    return memset(arena_malloc(self, count, size, align), 0, count * size);
}

static void *x__arena_grow_last(Arena *self, long count, long size) {
    self->begin = self->last;
    assert(count <= arena_available(self) / size);
    long total = count * size;
    ASAN_UNPOISON_MEMORY_REGION(self->last, total);
    self->begin += total;
    ASAN_POISON_MEMORY_REGION(self->begin, arena_available(self));
    return self->last;
}

static void *arena_realloc(Arena *self, void *ptr, long count, long size, long align) {
    if (!ptr) {
        return arena_malloc(self, count, size, align);
    }
    if (ptr == self->last) {
        return x__arena_grow_last(self, count, size);
    }
    assert(self->data <= ptr && ptr < self->last);
    void *new_ptr = arena_malloc(self, count, size, align);
    long total = count * size;
    long max_old_total = (char *)self->last - (char *)ptr;
    long size_copy = total < max_old_total ? total : max_old_total;
    ASAN_UNPOISON_MEMORY_REGION(ptr, size_copy);
    memcpy(new_ptr, ptr, size_copy);
    // we cannot poison ptr because we don't know its size and there is at least one valid pointer
    // between ptr and new_ptr, otherwise we would be in the ptr == self->last branch
    return new_ptr;
}

static void *arena_memcpy(Arena *, void *dest, const void *src, long size) {
    return memcpy(dest, src, size);
}

static void *arena_memdup(Arena *self, const void *src, long count, long size, long align) {
    return memcpy(arena_malloc(self, count, size, align), src, count * size);
}

static void arena_scratch_destroy(Arena *self, Arena scratch) {
    assert(scratch.data == self->end);
    self->end = scratch.end;
}

static void arena_destroy(Arena *self) {
    free(self->data);
}
