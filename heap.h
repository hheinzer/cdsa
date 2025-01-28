/// @file
#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Heap Heap;
typedef struct HeapItem HeapItem;

typedef int HeapDataCompare(const void *, const void *, void *);  ///< Data comparison function
typedef void *HeapDataCopy(Arena *, void *, const void *, long);  ///< Data copy function

/**
 * @brief Represents a binary heap (min-heap by default)
 */
struct Heap {
    Arena *arena;  ///< Pointer to an arena allocator
    struct {
        long size;                 ///< Size of the item data in bytes
        HeapDataCompare *compare;  ///< Pointer to a data comparison function
        HeapDataCopy *copy;        ///< Pointer to a data copy function
    } data;                        ///< Data properties
    long length;                   ///< Number of items in the heap
    HeapItem *begin;               ///< Pointer to the root item of the heap
    HeapItem *end;                 ///< Pointer to the last item in the heap
};

/**
 * @brief Represents a single item in the heap
 */
struct HeapItem {
    void *data;        ///< Pointer to the item data
    HeapItem *parent;  ///< Pointer to the parent item
    HeapItem *left;    ///< Pointer to the left child item
    HeapItem *right;   ///< Pointer to the right child item
    HeapItem *next;    ///< Pointer to the next item in insertion order
};

/**
 * @brief Iterate over all items of a heap
 * @param item Current heap item
 * @param self Pointer to a heap
 */
#define heap_for_each(item, self) for (auto(item) = (self)->begin; item; (item) = (item)->next)

/**
 * @brief Create a new heap
 * @param arena Pointer to an arena allocator
 * @param size Size of item data in bytes (optional)
 * @param compare Pointer to a data comparison function
 * @return New heap instance
 * @note If `size == 0`, the data pointers will be directly assigned rather than copied
 */
static Heap heap_create(Arena *arena, long size, HeapDataCompare *compare) {
    Heap heap = {};
    heap.arena = arena;
    heap.data.size = size;
    heap.data.compare = compare;
    heap.data.copy = arena_memcpy;
    return heap;
}

/// @private
static void x__heap_item_init(const Heap *self, HeapItem *item, void *data) {
    if (data && self->data.size) {
        item->data = arena_malloc(self->arena, 1, self->data.size, alignof(max_align_t));
        self->data.copy(self->arena, item->data, data, self->data.size);
    }
    else {
        item->data = data;
    }
}

/// @private
static HeapItem *x__heap_find_parent(HeapItem *item) {
    while (item->parent && item == item->parent->right) {
        item = item->parent;
    }
    if (item->parent) {
        if (item->parent->right) {
            item = item->parent->right;
        }
        else {
            return item->parent;
        }
    }
    while (item->left) {
        item = item->left;
    }
    return item;
}

/// @private
static void x__heap_sift_up(const Heap *self, HeapItem *item, void *context) {
    while (item->parent && self->data.compare(item->parent->data, item->data, context) > 0) {
        auto swap = item->data;
        item->data = item->parent->data;
        item->parent->data = swap;
        item = item->parent;
    }
}

/**
 * @brief Insert a new item into a heap
 * @param self Pointer to a heap
 * @param data Pointer to the item data
 * @param context Pointer to a user-provided context for the comparison function (optional)
 */
static void heap_push(Heap *self, void *data, void *context) {
    HeapItem *item = arena_calloc(self->arena, 1, sizeof(HeapItem), alignof(HeapItem));
    x__heap_item_init(self, item, data);
    if (self->length == 0) {
        self->begin = item;
        self->end = item;
    }
    else {
        auto parent = x__heap_find_parent(self->end);
        assert(!parent->right);
        item->parent = parent;
        if (!parent->left) {
            parent->left = item;
        }
        else {
            parent->right = item;
        }
        self->end->next = item;
        self->end = item;
        x__heap_sift_up(self, item, context);
    }
    self->length += 1;
}

/// @private
static HeapItem *x__heap_find_end(HeapItem *item) {
    while (item->parent && item == item->parent->left) {
        item = item->parent;
    }
    if (item->parent) {
        assert(item->parent->left);
        item = item->parent->left;
    }
    while (item->right) {
        item = item->right;
    }
    return item;
}

/// @private
static void x__heap_sift_down(const Heap *self, HeapItem *item, void *context) {
    while (item) {
        auto smallest = item;
        if (item->left && self->data.compare(item->left->data, smallest->data, context) < 0) {
            smallest = item->left;
        }
        if (item->right && self->data.compare(item->right->data, smallest->data, context) < 0) {
            smallest = item->right;
        }
        if (smallest == item) {
            break;
        }
        auto swap = item->data;
        item->data = smallest->data;  // cppcheck-suppress nullPointerRedundantCheck
        smallest->data = swap;        // cppcheck-suppress nullPointerRedundantCheck
        item = smallest;
    }
}

/**
 * @brief Remove the root item from the heap
 * @param self Pointer to a heap
 * @param context Pointer to a user-provided context for the comparison function (optional)
 * @return Pointer to the item data, or `nullptr` if the heap is empty
 */
static void *heap_pop(Heap *self, void *context) {
    if (self->length == 0) {
        return nullptr;
    }
    void *data = self->begin->data;
    if (self->length == 1) {
        self->begin = nullptr;
        self->end = nullptr;
    }
    else {
        assert(self->end->parent);
        auto end = x__heap_find_end(self->end);
        if (self->end == self->end->parent->right) {
            self->end->parent->right = nullptr;
        }
        else {
            self->end->parent->left = nullptr;
        }
        self->begin->data = self->end->data;
        self->end = end;
        self->end->next = nullptr;
        x__heap_sift_down(self, self->begin, context);
    }
    self->length -= 1;
    return data;
}

/**
 * @brief Retrieve the root item of the heap
 * @param self Pointer to a heap
 * @return Pointer to the item data, or `nullptr` if the heap is empty
 */
static void *heap_peek(const Heap *self) {
    if (self->length == 0) {
        return nullptr;
    }
    return self->begin->data;
}

/**
 * @brief Create a clone of a heap
 * @param self Pointer to a heap
 * @param context Pointer to a user-provided context for the comparison function (optional)
 * @param arena Pointer to an arena allocator (optional)
 * @return Cloned heap instance
 * @note If no arena allocator is passed, the arena allocator of the heap is used
 */
static Heap heap_clone(const Heap *self, void *context, Arena *arena) {
    Heap heap = {};
    heap.arena = arena ? arena : self->arena;
    heap.data = self->data;
    heap_for_each(item, self) {
        heap_push(&heap, item->data, context);
    }
    return heap;
}

/**
 * @brief Retrieve an array of heap items
 * @param self Pointer to a heap
 * @param arena Pointer to an arena allocator (optional)
 * @return Pointer to an array of items
 * @note If no arena allocator is passed, the arena allocator of the heap is used
 */
static HeapItem *heap_items(const Heap *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    HeapItem *items = arena_malloc(arena, self->length, sizeof(HeapItem), alignof(HeapItem));
    long index = 0;
    heap_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}
