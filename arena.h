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

// push a new region
static void arena_region_push(Arena *arena)
{
    assert(arena && arena->data);
    assert(arena->size + arena->alignment <= arena->capacity);
    *(size_t *)(arena->data + arena->size) = arena->region;
    arena->region = arena->size;
    arena->size += arena->alignment;
}

// pop the most recent region
static void arena_region_pop(Arena *arena)
{
    assert(arena && arena->data);
    assert(arena->region > 0);
    arena->size = arena->region;
    arena->region = *(size_t *)(arena->data + arena->size);
}

static size_t x__arena_aligned_size(const Arena *arena, size_t size)
{
    return (size + arena->alignment - 1) & ~(arena->alignment - 1);
}

// allocate a block of memory
static void *arena_malloc(Arena *arena, size_t size)
{
    assert(arena && arena->data);
    const size_t aligned = x__arena_aligned_size(arena, size);
    assert(arena->size + aligned + arena->alignment <= arena->capacity);
    uint8_t *ptr = arena->data + arena->size;
    *(size_t *)ptr = size;
    ptr += arena->alignment;
    assert((uintptr_t)ptr % arena->alignment == 0);
    arena->size += aligned + arena->alignment;
    return ptr;
}

// allocate a block of memory; initialize memory to zero
static void *arena_calloc(Arena *arena, size_t count, size_t size)
{
    void *ptr = arena_malloc(arena, count * size);
    return memset(ptr, 0, count * size);
}

// return the size of a block of memory
static size_t arena_sizeof(Arena *arena, const void *ptr)
{
    assert(arena && arena->data);
    assert((uintptr_t)ptr - (uintptr_t)arena->data <= arena->capacity);
    return *(size_t *)((uint8_t *)ptr - arena->alignment);
}

// do nothing; memory can only be freed through regions
static void arena_free(Arena *arena, void *ptr)
{
    assert(arena && arena->data);
    assert((uintptr_t)ptr - (uintptr_t)arena->data <= arena->capacity);
}

// reallocate a block of memory
static void *arena_realloc(Arena *arena, void *ptr, size_t new_size)
{
    if (!ptr) return arena_malloc(arena, new_size);
    if (!new_size) {
        arena_free(arena, ptr);
        return 0;
    }
    const size_t old_size = arena_sizeof(arena, ptr);
    if (new_size <= old_size) return ptr;
    const size_t old_aligned = x__arena_aligned_size(arena, old_size);
    if ((uintptr_t)ptr + old_aligned == (uintptr_t)arena->data + arena->size) {
        const size_t new_aligned = x__arena_aligned_size(arena, new_size);
        assert(arena->size + new_aligned - old_aligned <= arena->capacity);
        *(size_t *)((uint8_t *)ptr - arena->alignment) = new_size;
        arena->size += new_aligned - old_aligned;
        return ptr;
    }
    void *new_ptr = arena_malloc(arena, new_size);
    memcpy(new_ptr, ptr, old_size);
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
