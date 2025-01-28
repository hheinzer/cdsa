/// @file
#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Represents an arena allocator
 *
 * An arena manages a fixed block of memory, enabling fast allocations without explicit cleanup.
 * Memory is allocated sequentially, allowing for recycling of previously allocated regions.
 */
typedef struct {
    void *data;   ///< Pointer to the base memory region
    void *last;   ///< Pointer to the last allocated object
    char *begin;  ///< Pointer to the beginning of the available memory
    char *end;    ///< Pointer to the end of the available memory
} Arena;

/**
 * @brief Create a new arena
 * @param capacity Size of the base memory region in bytes
 * @return New arena instance
 * @note The arena must be destroyed using `arena_destroy()` to prevent memory leaks
 */
static Arena arena_create(long capacity) {
    Arena arena = {};
    arena.data = malloc(capacity);
    assert(arena.data);
    arena.begin = arena.data;
    arena.end = arena.begin + capacity;
    return arena;
}

/**
 * @brief Get the size of the occupied memory region
 * @param self Pointer to an arena
 * @return Size of occupied memory in bytes
 */
static long arena_occupied(const Arena *self) {
    return self->begin - (char *)self->data;
}

/**
 * @brief Get the size of the available memory region
 * @param self Pointer to an arena
 * @return Size of available memory in bytes
 */
static long arena_available(const Arena *self) {
    return self->end - self->begin;
}

/**
 * @brief Create a new scratch arena at the end of the available space of the parent arena
 * @param self Pointer to the parent arena
 * @param capacity Size of the base memory region in bytes
 * @return New scratch arena instance
 */
static Arena arena_scratch_create(Arena *self, long capacity) {
    assert(arena_available(self) >= capacity);
    self->end -= capacity;
    Arena scratch = {};
    scratch.data = self->end;
    scratch.begin = scratch.data;
    scratch.end = scratch.begin + capacity;
    return scratch;
}

/**
 * @brief Allocate uninitialized memory from an arena
 * @param self Pointer to an arena
 * @param count Number of objects
 * @param size Size of a single object in bytes
 * @param align Requested alignment in bytes
 * @return Pointer to the allocated memory
 * @note This function will trigger an assertion failure if the arena runs out of memory
 */
[[gnu::malloc, gnu::alloc_size(2, 3), gnu::alloc_align(4)]]
static void *arena_malloc(Arena *self, long count, long size, long align) {
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
    return aligned_alloc(align, count * size);
#endif
    long padding = -(uintptr_t)self->begin & (align - 1);
    assert(count <= (arena_available(self) - padding) / size);
    self->last = self->begin + padding;
    self->begin += padding + count * size;
    return self->last;
}

/**
 * @brief Allocate and zero-initialize memory from an arena
 * @param self Pointer to an arena
 * @param count Number of objects
 * @param size Size of a single object in bytes
 * @param align Requested alignment in bytes
 * @return Pointer to the allocated memory
 */
static void *arena_calloc(Arena *self, long count, long size, long align) {
    return memset(arena_malloc(self, count, size, align), 0, count * size);
}

/**
 * @brief Reallocate memory from an arena
 * @param self Pointer to an arena
 * @param ptr Pointer to the previously allocated memory (optional)
 * @param count Number of objects
 * @param size Size of a single object in bytes
 * @param align Requested alignment in bytes
 * @return Pointer to the allocated memory
 * @note If `ptr` is a null pointer, the function behaves like `arena_malloc()`
 * @note If `ptr` is the last allocated object of the arena, the alignment will be unchanged
 */
static void *arena_realloc(Arena *self, void *ptr, long count, long size, long align) {
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
    return realloc(ptr, count * size);
#endif
    if (!ptr) {
        return arena_malloc(self, count, size, align);
    }
    if (ptr == self->last) {
        self->begin = self->last;
        assert(count <= arena_available(self) / size);
        self->begin += count * size;
        return self->last;
    }
    assert(self->data <= ptr && ptr < self->last);
    void *new_ptr = arena_malloc(self, count, size, align);
    long total = count * size;
    long max_old_total = (char *)self->last - (char *)ptr;
    return memcpy(new_ptr, ptr, total < max_old_total ? total : max_old_total);
}

/**
 * @brief Copy memory from one location to another within an arena
 * @param dest Pointer to the destination
 * @param src Pointer to the source
 * @param size Number of bytes to copy
 * @return Pointer to the destination
 */
static void *arena_memcpy(Arena *, void *restrict dest, const void *restrict src, long size) {
    return memcpy(dest, src, size);
}

/**
 * @brief Duplicate memory within the arena
 * @param self Pointer to an arena
 * @param src Pointer to the source
 * @param count Number of objects
 * @param size Size of a single object in bytes
 * @param align Requested alignment in bytes
 * @return Pointer to the duplicated memory
 */
static void *arena_memdup(Arena *self, const void *src, long count, long size, long align) {
    return memcpy(arena_malloc(self, count, size, align), src, count * size);
}

/**
 * @brief Destroy a scratch arena and return its space to the parent arena
 * @param self Pointer to the parent arena
 * @param scratch Scratch arena to destroy
 * @note Scratch arenas must be destroyed in inverse creation order
 */
static void arena_scratch_destroy(Arena *self, Arena scratch) {
    assert(scratch.data == self->end);
    self->end = scratch.end;
}

/**
 * @brief Destroy an arena and free its base memory region
 * @param self Pointer to an arena
 */
static void arena_destroy(Arena *self) {
    free(self->data);
}

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
const char *__asan_default_options() {
    return "detect_leaks=0";
}
#endif
