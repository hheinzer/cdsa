#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Heap Heap;
typedef struct HeapItem HeapItem;

typedef int HeapDataCompare(const void *, const void *, void *);
typedef void *HeapDataCopy(Arena *, void *, const void *, long);

struct Heap {
    Arena *arena;
    struct {
        long size;
        HeapDataCompare *compare;
        HeapDataCopy *copy;
    } data;
    long length;
    HeapItem *begin;
    HeapItem *end;
};

struct HeapItem {
    void *data;
    HeapItem *parent;
    HeapItem *left;
    HeapItem *right;
    HeapItem *next;
};

#define heap_for_each(item, self) for (auto(item) = (self)->begin; item; (item) = (item)->next)

static Heap heap_create(Arena *arena, long size, HeapDataCompare *compare) {
    Heap heap = {};
    heap.arena = arena;
    heap.data.size = size;
    heap.data.compare = compare;
    heap.data.copy = arena_memcpy;
    return heap;
}

static void x__heap_item_create(const Heap *self, HeapItem *item, void *data) {
    if (data && self->data.size) {
        item->data = arena_malloc(self->arena, 1, self->data.size, alignof(max_align_t));
        self->data.copy(self->arena, item->data, data, self->data.size);
    }
    else {
        item->data = data;
    }
}

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

static void x__heap_sift_up(const Heap *self, HeapItem *item, void *context) {
    while (item->parent && self->data.compare(item->parent->data, item->data, context) > 0) {
        auto swap = item->data;
        item->data = item->parent->data;
        item->parent->data = swap;
        item = item->parent;
    }
}

static void heap_push(Heap *self, void *data, void *context) {
    HeapItem *item = arena_calloc(self->arena, 1, sizeof(HeapItem), alignof(HeapItem));
    x__heap_item_create(self, item, data);
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

static void *heap_peek(const Heap *self) {
    if (self->length == 0) {
        return nullptr;
    }
    return self->begin->data;
}

static Heap heap_clone(const Heap *self, void *context, Arena *arena) {
    Heap heap = {};
    heap.arena = arena ? arena : self->arena;
    heap.data = self->data;
    heap_for_each(item, self) {
        heap_push(&heap, item->data, context);
    }
    return heap;
}

static HeapItem *heap_items(const Heap *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    HeapItem *items = arena_malloc(arena, self->length, sizeof(HeapItem), alignof(HeapItem));
    long index = 0;
    heap_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}
