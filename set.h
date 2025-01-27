#pragma once

#include "arena.h"

typedef struct Set Set;
typedef struct SetItem SetItem;

static constexpr long x__set_hash_shift = 2;
static constexpr long x__set_hash_select = 64 - x__set_hash_shift;

struct Set {
    Arena *arena;
    long length;
    SetItem *begin;
    SetItem *end;
};

struct SetItem {
    struct {
        void *data;
        long size;
    } key;
    SetItem *child[1 << x__set_hash_shift];
    SetItem *next;
};

#define set_for_each(item, self)                                  \
    for (auto(item) = (self)->begin; item; (item) = (item)->next) \
        if ((item)->key.size)

static Set set_create(Arena *arena) {
    Set set = {};
    set.arena = arena;
    return set;
}

/// @private
static uint64_t x__set_hash_fnv1a(const char *key, long size) {
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
static bool x__set_key_equals(const SetItem *item, const void *key, long size) {
    return item->key.size == size && !memcmp(item->key.data, key, size);
}

/// @private
static void x__set_item_init(const Set *self, SetItem *item, const void *key, long size) {
    item->key.data = arena_memdup(self->arena, key, 1, size, alignof(max_align_t));
    item->key.size = size;
}

static bool set_insert(Set *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = &self->begin;
    for (auto hash = x__set_hash_fnv1a(key, size); *item; hash <<= x__set_hash_shift) {
        if (!(*item)->key.size) {
            break;
        }
        if (x__set_key_equals(*item, key, size)) {
            return false;
        }
        item = &(*item)->child[hash >> x__set_hash_select];
    }
    if (!*item) {
        *item = arena_calloc(self->arena, 1, sizeof(SetItem), alignof(SetItem));
        if (self->end) {
            self->end->next = *item;
        }
        self->end = *item;
    }
    x__set_item_init(self, *item, key, size);
    self->length += 1;
    return true;
}

static bool set_remove(Set *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__set_hash_fnv1a(key, size); item; hash <<= x__set_hash_shift) {
        if (x__set_key_equals(item, key, size)) {
            item->key.size = 0;
            self->length -= 1;
            return true;
        }
        item = item->child[hash >> x__set_hash_select];
    }
    return false;
}

static bool set_find(const Set *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__set_hash_fnv1a(key, size); item; hash <<= x__set_hash_shift) {
        if (x__set_key_equals(item, key, size)) {
            return true;
        }
        item = item->child[hash >> x__set_hash_select];
    }
    return false;
}

static Set set_clone(const Set *self, Arena *arena) {
    Set dict = {};
    dict.arena = arena ? arena : self->arena;
    set_for_each(item, self) {
        set_insert(&dict, item->key.data, item->key.size);
    }
    return dict;
}

static SetItem *set_items(const Set *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    SetItem *items = arena_malloc(arena, self->length, sizeof(SetItem), alignof(SetItem));
    long index = 0;
    set_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}

static Set set_union(const Set *self, const Set *other, Arena *arena) {
    Set set = set_clone(self, arena);
    set_for_each(item, other) {
        set_insert(&set, item->key.data, item->key.size);
    }
    return set;
}

static Set set_intersection(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

static Set set_difference(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

static Set set_symmetric_difference(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    set_for_each(item, other) {
        if (!set_find(self, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

static bool set_is_subset(const Set *self, const Set *other) {
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            return false;
        }
    }
    return true;
}

static bool set_is_superset(const Set *self, const Set *other) {
    return set_is_subset(other, self);
}
