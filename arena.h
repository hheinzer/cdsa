#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// general purpose arena allocator
typedef struct Arena Arena;
enum ArenaSizeMultiplier : size_t { KB = (1 << 10), MB = (1 << 20), GB = (1 << 30) };

struct Arena {
    size_t region, size, capacity, alignment;
    uint8_t *data;
};

// create an empty arena
static Arena arena_create(size_t capacity, size_t alignment)
{
    assert(alignment >= sizeof(size_t));
    assert(capacity >= alignment);
    assert(capacity % alignment == 0);
    uint8_t *data = aligned_alloc(alignment, capacity);
    assert(data);
    return (Arena){
        .capacity = capacity,
        .alignment = alignment,
        .data = data,
    };
}

static void x__arena_write(const Arena *arena, size_t size)
{
    *(size_t *)(arena->data + arena->size) = size;
}

// push a new region
static void arena_region_push(Arena *arena)
{
    assert(arena);
    assert(arena->data);
    assert(arena->size + arena->alignment <= arena->capacity);
    x__arena_write(arena, arena->region);
    arena->region = arena->size;
    arena->size += arena->alignment;
}

static size_t x__arena_read(const Arena *arena)
{
    return *(size_t *)(arena->data + arena->size);
}

// pop the most recent region
static void arena_region_pop(Arena *arena)
{
    assert(arena);
    assert(arena->data);
    assert(arena->region > 0);
    arena->size = arena->region;
    arena->region = x__arena_read(arena);
}

static size_t x__arena_aligned(const Arena *arena, size_t size)
{
    return (size + arena->alignment - 1) & ~(arena->alignment - 1);
}

// allocate a block of memory
static void *arena_malloc(Arena *arena, size_t size)
{
    assert(arena);
    assert(arena->data);
    if (!size) return 0;
    const size_t aligned = x__arena_aligned(arena, size);
    assert(arena->size + arena->alignment + aligned <= arena->capacity);
    x__arena_write(arena, aligned);
    void *ptr = arena->data + arena->size + arena->alignment;
    arena->size += arena->alignment + aligned;
    return ptr;
}

// allocate a block of memory; initialize memory to zero
static void *arena_calloc(Arena *arena, size_t count, size_t size)
{
    void *ptr = arena_malloc(arena, count * size);
    return memset(ptr, 0, count * size);
}

size_t x__arena_read_size(const Arena *arena, const void *ptr)
{
    return *(size_t *)((uint8_t *)ptr - arena->alignment);
}

// return the size of a block of memory
static size_t arena_sizeof(const Arena *arena, const void *ptr)
{
    assert(arena);
    assert(arena->data);
    assert((uintptr_t)ptr - (uintptr_t)arena->data <= arena->capacity);
    return x__arena_read_size(arena, ptr);
}

// do nothing; memory can only be freed through regions
static void arena_free(Arena *arena, void *ptr)
{
    assert(arena);
    assert(arena->data);
    assert((uintptr_t)ptr - (uintptr_t)arena->data <= arena->capacity);
}

void x__arena_write_size(const Arena *arena, void *ptr, size_t size)
{
    *(size_t *)((uint8_t *)ptr - arena->alignment) = size;
}

int x__arena_is_top(const Arena *arena, const void *ptr, size_t size)
{
    return (uintptr_t)arena->data + arena->size == (uintptr_t)ptr + size;
}

// reallocate a block of memory
static void *arena_realloc(Arena *arena, void *ptr, size_t new_size)
{
    if (!ptr) return arena_malloc(arena, new_size);
    if (!new_size) {
        arena_free(arena, ptr);
        return 0;
    }
    const size_t new_aligned = x__arena_aligned(arena, new_size);
    const size_t old_aligned = arena_sizeof(arena, ptr);
    if (new_aligned <= old_aligned) {
        x__arena_write_size(arena, ptr, new_size);
        return ptr;
    }
    if (x__arena_is_top(arena, ptr, old_aligned)) {
        assert(arena->size + new_aligned - old_aligned <= arena->capacity);
        x__arena_write_size(arena, ptr, new_size);
        arena->size += new_aligned - old_aligned;
        return ptr;
    }
    void *new_ptr = arena_malloc(arena, new_size);
    memcpy(new_ptr, ptr, old_aligned);
    arena_free(arena, ptr);
    return new_ptr;
}

// remove all data from the arena
static void arena_clear(Arena *arena)
{
    assert(arena);
    free(arena->data);
    *arena = (Arena){0};
}
