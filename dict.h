#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Dict Dict;
typedef struct DictItem DictItem;

typedef void *DictDataCopy(Arena *, void *, const void *, long);

static constexpr long x__dict_hash_shift = 2;
static constexpr long x__dict_hash_select = 64 - x__dict_hash_shift;

struct Dict {
    Arena *arena;
    struct {
        long size;
        DictDataCopy *copy;
    } data;
    long length;
    DictItem *begin;
    DictItem *end;
};

struct DictItem {
    struct {
        void *data;
        long size;
    } key;
    void *data;
    DictItem *child[1 << x__dict_hash_shift];
    DictItem *next;
};

#define dict_for_each(item, self)                                 \
    for (auto(item) = (self)->begin; item; (item) = (item)->next) \
        if ((item)->key.size)

static Dict dict_create(Arena *arena, long size) {
    Dict dict = {};
    dict.arena = arena;
    dict.data.size = size;
    dict.data.copy = arena_memcpy;
    return dict;
}

/// @private
static uint64_t x__dict_hash_fnv1a(const char *key, long size) {
    constexpr uint64_t basis = 0xcbf29ce484222325;
    constexpr uint64_t prime = 0x00000100000001b3;
    uint64_t hash = basis;
    for (auto byte = key; byte < key + size; byte++) {
        hash ^= *byte;
        hash *= prime;
    }
    return hash;
}

/// @private
static bool x__dict_key_equals(const DictItem *item, const void *key, long size) {
    return item->key.size == size && !memcmp(item->key.data, key, size);
}

/// @private
static void x__dict_item_init(const Dict *self, DictItem *item, const void *key, long size,
                              void *data) {
    item->key.data = arena_memdup(self->arena, key, 1, size, alignof(max_align_t));
    item->key.size = size;
    if (data && self->data.size) {
        item->data = arena_malloc(self->arena, 1, self->data.size, alignof(max_align_t));
        self->data.copy(self->arena, item->data, data, self->data.size);
    }
    else {
        item->data = data;
    }
}

static void *dict_insert(Dict *self, const void *key, long size, void *data) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = &self->begin;
    for (auto hash = x__dict_hash_fnv1a(key, size); *item; hash <<= x__dict_hash_shift) {
        if (!(*item)->key.size) {
            break;
        }
        if (x__dict_key_equals(*item, key, size)) {
            return (*item)->data;
        }
        item = &(*item)->child[hash >> x__dict_hash_select];
    }
    if (!*item) {
        *item = arena_calloc(self->arena, 1, sizeof(DictItem), alignof(DictItem));
        if (self->end) {
            self->end->next = *item;
        }
        self->end = *item;
    }
    x__dict_item_init(self, *item, key, size, data);
    self->length += 1;
    return nullptr;
}

static void *dict_remove(Dict *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__dict_hash_fnv1a(key, size); item; hash <<= x__dict_hash_shift) {
        if (x__dict_key_equals(item, key, size)) {
            item->key.size = 0;
            self->length -= 1;
            return item->data;
        }
        item = item->child[hash >> x__dict_hash_select];
    }
    return nullptr;
}

static void *dict_find(const Dict *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__dict_hash_fnv1a(key, size); item; hash <<= x__dict_hash_shift) {
        if (x__dict_key_equals(item, key, size)) {
            return item->data;
        }
        item = item->child[hash >> x__dict_hash_select];
    }
    return nullptr;
}

static Dict dict_clone(const Dict *self, Arena *arena) {
    Dict dict = {};
    dict.arena = arena ? arena : self->arena;
    dict.data = self->data;
    dict_for_each(item, self) {
        dict_insert(&dict, item->key.data, item->key.size, item->data);
    }
    return dict;
}

static DictItem *dict_items(const Dict *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    DictItem *items = arena_malloc(arena, self->length, sizeof(DictItem), alignof(DictItem));
    long index = 0;
    dict_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}
