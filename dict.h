#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Dict Dict;
typedef struct DictItem DictItem;
typedef void *DictDataCopy(Arena *, void *, const void *, long);

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
    DictItem *child[4];
    DictItem *next;
};

typedef enum {
    KEEPOLD = 1 << 1,
} DictFlags;

#define dict_for_each(item, dict)                                 \
    for (DictItem *item = (dict)->begin; item; item = item->next) \
        if (item->key.size)

static Dict dict_create(Arena *arena, long size) {
    Dict dict = {0};
    dict.arena = arena;
    dict.data.size = size;
    dict.data.copy = arena_memcpy;
    return dict;
}

static uint64_t x__dict_hash_fnv1a(const char *key, long size) {
    uint64_t hash = 0xcbf29ce484222325;
    for (const char *byte = key; byte < key + size; ++byte) {
        hash ^= *byte;
        hash *= 0x00000100000001b3;
    }
    return hash;
}

static int x__dict_key_equals(const DictItem *item, const void *key, long size) {
    return item->key.size == size && !memcmp(item->key.data, key, size);
}

static void x__dict_item_create(const Dict *self, DictItem *item, const void *key, long size,
                                void *data) {
    item->key.data = arena_memdup(self->arena, key, 1, size, alignof(max_align_t));
    item->key.size = size;
    if (data && self->data.copy) {
        item->data = arena_alloc(self->arena, 1, self->data.size, alignof(max_align_t), NOZERO);
        self->data.copy(self->arena, item->data, data, self->data.size);
    } else {
        item->data = data;
    }
}

static void *dict_insert(Dict *self, const void *key, long size, void *data, int flags) {
    if (!size) {
        size = strlen(key) + 1;
    }
    DictItem **item = &self->begin;
    for (uint64_t hash = x__dict_hash_fnv1a(key, size); *item; hash <<= 2) {
        if (!(*item)->key.size) {
            break;
        }
        if (x__dict_key_equals(*item, key, size)) {
            void *item_data = (*item)->data;
            if (!(flags & KEEPOLD)) {
                (*item)->data = data;
            }
            return item_data;
        }
        item = &(*item)->child[hash >> 62];
    }
    if (!*item) {
        *item = arena_alloc(self->arena, 1, sizeof(DictItem), alignof(DictItem), 0);
        if (self->end) {
            self->end->next = *item;
        }
        self->end = *item;
    }
    x__dict_item_create(self, *item, key, size, data);
    self->length += 1;
    return 0;
}

static void *dict_remove(Dict *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    DictItem *item = self->begin;
    for (uint64_t hash = x__dict_hash_fnv1a(key, size); item; hash <<= 2) {
        if (x__dict_key_equals(item, key, size)) {
            item->key.size = 0;
            self->length -= 1;
            return item->data;
        }
        item = item->child[hash >> 62];
    }
    return 0;
}

static void *dict_find(const Dict *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    DictItem *item = self->begin;
    for (uint64_t hash = x__dict_hash_fnv1a(key, size); item; hash <<= 2) {
        if (x__dict_key_equals(item, key, size)) {
            return item->data;
        }
        item = item->child[hash >> 62];
    }
    return 0;
}

static Dict dict_clone(const Dict *self, Arena *arena) {
    if (!arena) {
        arena = self->arena;
    }
    Dict dict = {0};
    dict.arena = arena;
    dict.data = self->data;
    for (DictItem *item = self->begin; item; item = item->next) {
        if (item->key.size) {
            dict_insert(&dict, item->key.data, item->key.size, item->data, 0);
        }
    }
    return dict;
}

static DictItem *dict_items(const Dict *self, Arena *arena) {
    if (!arena) {
        arena = self->arena;
    }
    DictItem *items = arena_alloc(arena, self->length, sizeof(DictItem), alignof(DictItem), NOZERO);
    long index = 0;
    for (DictItem *item = self->begin; item; item = item->next) {
        if (item->key.size) {
            items[index++] = *item;
        }
    }
    return items;
}
