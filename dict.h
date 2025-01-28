/// @file
#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Dict Dict;
typedef struct DictItem DictItem;

typedef void *DictDataCopy(Arena *, void *, const void *, long);  ///< Data copy function

static constexpr long dict_hash_shift = 2;  ///< Number of hash bits to shift out
static constexpr long dict_branch_select = 64 - dict_hash_shift;  ///< Bit mask to select branch

/**
 * @brief Represents a dictionary of key-data pairs
 */
struct Dict {
    Arena *arena;  ///< Pointer to an arena allocator
    struct {
        long size;           ///< Size of the item data in bytes
        DictDataCopy *copy;  ///< Pointer to a data copy function
    } data;                  ///< Data properties
    long length;             ///< Number of items in the dict
    DictItem *begin;         ///< Pointer to root item of the dict
    DictItem *end;           ///< Pointer to last item of the dict
};

/**
 * @brief Represents a single item of a dict
 */
struct DictItem {
    struct {
        void *data;                         ///< Pointer to the key data
        long size;                          ///< Size of the key data in bytes
    } key;                                  ///< Key properties
    void *data;                             ///< Pointer to the item data
    DictItem *child[1 << dict_hash_shift];  ///< Array of child items for hash traversal
    DictItem *next;                         ///< Pointer to the next item
};

/**
 * @brief Iterate over all items of a dict
 * @param item Current dict item
 * @param self Pointer to a dict
 */
#define dict_for_each(item, self)                                 \
    for (auto(item) = (self)->begin; item; (item) = (item)->next) \
        if ((item)->key.size)

/**
 * @brief Create a new dict
 * @param arena Pointer to an arena allocator
 * @param size Size of item data in bytes (optional)
 * @return New dict instance
 * @note If `size == 0`, the data pointers will be directly assigned rather than copied
 */
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

/**
 * @brief Insert a new item into a dict
 * @param self Pointer to a dict
 * @param key Pointer to the key data
 * @param size Size of the key data in bytes (optional)
 * @param data Pointer to the item data
 * @return Pointer to the item data if the key already exists, or `nullptr` if it is a new key
 * @note If `size == 0`, `key` is assumed to be a null-terminated string
 */
static void *dict_insert(Dict *self, const void *key, long size, void *data) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = &self->begin;
    for (auto hash = x__dict_hash_fnv1a(key, size); *item; hash <<= dict_hash_shift) {
        if (!(*item)->key.size) {
            break;
        }
        if (x__dict_key_equals(*item, key, size)) {
            return (*item)->data;
        }
        item = &(*item)->child[hash >> dict_branch_select];
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

/**
 * @brief Remove an item from a dict
 * @param self Pointer to a dict
 * @param key Pointer to the key data
 * @param size Size of the key data in bytes (optional)
 * @return Pointer to the item data, or `nullptr` if the key is not found
 * @note If `size == 0`, `key` is assumed to be a null-terminated string
 */
static void *dict_remove(Dict *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__dict_hash_fnv1a(key, size); item; hash <<= dict_hash_shift) {
        if (x__dict_key_equals(item, key, size)) {
            item->key.size = 0;
            self->length -= 1;
            return item->data;
        }
        item = item->child[hash >> dict_branch_select];
    }
    return nullptr;
}

/**
 * @brief Find an item of a dict
 * @param self Pointer to a dict
 * @param key Pointer to the key data
 * @param size Size of the key data in bytes (optional)
 * @return Pointer to the item data, or `nullptr` if the key is not found
 * @note If `size == 0`, `key` is assumed to be a null-terminated string
 */
static void *dict_find(const Dict *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__dict_hash_fnv1a(key, size); item; hash <<= dict_hash_shift) {
        if (x__dict_key_equals(item, key, size)) {
            return item->data;
        }
        item = item->child[hash >> dict_branch_select];
    }
    return nullptr;
}

/**
 * @brief Create a clone of a dict
 * @param self Pointer to a dict
 * @param arena Pointer to an arena allocator (optional)
 * @return Cloned dict instance
 * @note If no arena allocator is passed, the arena allocator of the dict is used
 */
static Dict dict_clone(const Dict *self, Arena *arena) {
    Dict dict = {};
    dict.arena = arena ? arena : self->arena;
    dict.data = self->data;
    dict_for_each(item, self) {
        dict_insert(&dict, item->key.data, item->key.size, item->data);
    }
    return dict;
}

/**
 * @brief Retrieve an array of dict items
 * @param self Pointer to a dict
 * @param arena Pointer to an arena allocator (optional)
 * @return Pointer to an array of items
 * @note If no arena allocator is passed, the arena allocator of the dict is used
 */
static DictItem *dict_items(const Dict *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    DictItem *items = arena_malloc(arena, self->length, sizeof(DictItem), alignof(DictItem));
    long index = 0;
    dict_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}
