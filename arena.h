#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// general purpose arena allocator
typedef struct Arena Arena;

struct Arena {
    char *data, *prev, *head, *tail;
};

// create an empty arena
static Arena arena_create(long capacity)
{
    assert(capacity > 0);
    char *data = malloc(capacity);
    assert(data);
    return (Arena){
        .data = memset(data, 0, capacity),
        .head = data,
        .tail = data + capacity,
    };
}

// allocate a block of memory
[[gnu::malloc, gnu::alloc_size(2, 3), gnu::alloc_align(4)]]
static void *arena_alloc(Arena *arena, long count, long size, long align, int init)
{
    assert(arena);
    assert(arena->data);
    assert(align > 0 && (align & (align - 1)) == 0);
    const long padding = -(uintptr_t)arena->head & (align - 1);
    if (count <= 0 || size <= 0) return arena->head + padding;
    const long available = arena->tail - arena->head - padding;
    if (available < 0 || count > available / size) abort();
    arena->prev = arena->head + padding;
    arena->head = arena->prev + count * size;
    return init ? memset(arena->prev, 0, count * size) : arena->prev;
}

// reallocate a block of memory
static void *arena_realloc(Arena *arena, void *ptr, long new_size, long align)
{
    if (!ptr || new_size <= 0) return arena_alloc(arena, 1, new_size, align, 0);
    if (ptr == arena->prev) {
        const long old_size = arena->head - arena->prev;
        if (new_size <= old_size) return ptr;
        arena_alloc(arena, 1, new_size - old_size, align, 0);
        arena->prev = ptr;
        return arena->prev;
    }
    void *new_ptr = arena_alloc(arena, 1, new_size, align, 0);
    const long max_old_size = arena->prev - (char *)ptr;
    return memcpy(new_ptr, ptr, (new_size < max_old_size ? new_size : max_old_size));
}

// remove all memory from the arena
static void arena_clear(Arena *arena)
{
    assert(arena);
    free(arena->data);
    *arena = (Arena){0};
}
