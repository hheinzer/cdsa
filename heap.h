#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Heap Heap;
typedef struct HeapItem HeapItem;
typedef int HeapDataCompare(const void *, const void *, void *);
typedef void *HeapDataCopy(Arena *, void *, const void *, long);
typedef void HeapForEach(const HeapItem *, void *);

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
};

static Heap heap_create(Arena *arena, long size, HeapDataCompare *compare) {
    Heap heap = {0};
    heap.arena = arena;
    heap.data.size = size;
    heap.data.compare = compare;
    heap.data.copy = arena_memcpy;
    return heap;
}

static void x__heap_item_create(const Heap *self, HeapItem *item, void *data) {
    if (data && self->data.copy) {
        item->data = arena_alloc(self->arena, 1, self->data.size, alignof(max_align_t), NOZERO);
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
        void *swap = item->data;
        item->data = item->parent->data;
        item->parent->data = swap;
        item = item->parent;
    }
}

static void heap_push(Heap *self, void *data, void *context) {
    HeapItem *item = arena_alloc(self->arena, 1, sizeof(HeapItem), alignof(HeapItem), 0);
    x__heap_item_create(self, item, data);
    if (self->length == 0) {
        self->begin = item;
        self->end = item;
    }
    else {
        HeapItem *parent = x__heap_find_parent(self->end);
        assert(!parent->right);
        item->parent = parent;
        if (!parent->left) {
            parent->left = item;
        }
        else {
            parent->right = item;
        }
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
        HeapItem *smallest = item;
        if (item->left && self->data.compare(item->left->data, smallest->data, context) < 0) {
            smallest = item->left;
        }
        if (item->right && self->data.compare(item->right->data, smallest->data, context) < 0) {
            smallest = item->right;
        }
        if (smallest == item) {
            break;
        }
        void *swap = item->data;
        item->data = smallest->data;  // cppcheck-suppress nullPointerRedundantCheck
        smallest->data = swap;        // cppcheck-suppress nullPointerRedundantCheck
        item = smallest;
    }
}

static void *heap_pop(Heap *self, void *context) {
    if (self->length == 0) {
        return 0;
    }
    void *data = self->begin->data;
    if (self->length == 1) {
        self->begin = 0;
        self->end = 0;
    }
    else {
        assert(self->end->parent);
        if (self->end == self->end->parent->right) {
            self->end->parent->right = 0;
        }
        else {
            self->end->parent->left = 0;
        }
        self->begin->data = self->end->data;
        self->end = x__heap_find_end(self->end);
        x__heap_sift_down(self, self->begin, context);
    }
    self->length -= 1;
    return data;
}

static void *heap_peek(const Heap *self) {
    if (self->length == 0) {
        return 0;
    }
    return self->begin->data;
}

static void x__heap_for_each(const HeapItem *item, HeapForEach *callback, void *context) {
    if (item) {
        callback(item, context);
        x__heap_for_each(item->left, callback, context);
        x__heap_for_each(item->right, callback, context);
    }
}

static void heap_for_each(const Heap *self, HeapForEach *callback, void *context) {
    x__heap_for_each(self->begin, callback, context);
}

static HeapItem *x__heap_clone(const Heap *self, const HeapItem *begin, HeapItem **end) {
    if (!begin) {
        return 0;
    }
    HeapItem *item = arena_alloc(self->arena, 1, sizeof(HeapItem), alignof(HeapItem), 0);
    x__heap_item_create(self, item, begin->data);
    *end = item;
    item->left = x__heap_clone(self, begin->left, end);
    if (item->left) {
        item->left->parent = item;
    }
    item->right = x__heap_clone(self, begin->right, end);
    if (item->right) {
        item->right->parent = item;
    }
    return item;
}

static Heap heap_clone(const Heap *self, Arena *arena) {
    if (!arena) {
        arena = self->arena;
    }
    Heap heap = {0};
    heap.arena = arena;
    heap.data = self->data;
    heap.length = self->length;
    heap.begin = x__heap_clone(self, self->begin, &heap.end);
    return heap;
}

static void x__heap_items(const HeapItem *item, void *context) {
    HeapItem **items = context;
    *((*items)++) = *item;
}

static HeapItem *heap_items(const Heap *self, Arena *arena) {
    if (!arena) {
        arena = self->arena;
    }
    HeapItem *items = arena_alloc(arena, self->length, sizeof(HeapItem), alignof(HeapItem), NOZERO);
    heap_for_each(self, x__heap_items, (void *[]){items});
    return items;
}
