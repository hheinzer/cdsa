/// @file
#pragma once

#include <stddef.h>

#include "arena.h"

typedef struct Set Set;
typedef struct SetItem SetItem;

static constexpr long set_hash_shift = 2;  ///< Number of hash bits to shift out
static constexpr long set_branch_select = 64 - set_hash_shift;  ///< Bit mask to select branch

/**
 * @brief Represents a set of unique keys
 */
struct Set {
    Arena *arena;    ///< Pointer to an arena allocator
    long length;     ///< Number of items in the set
    SetItem *begin;  ///< Pointer to the root item in the set
    SetItem *end;    ///< Pointer to the last item in the set
};

/**
 * @brief Represents a single item in a set
 */
struct SetItem {
    struct {
        void *data;                       ///< Pointer to the key data
        long size;                        ///< Size of the key data in bytes
    } key;                                ///< Key properties
    SetItem *child[1 << set_hash_shift];  ///< Array of child items for hash traversal
    SetItem *next;                        ///< Pointer to the next item
};

/**
 * @brief Iterate over all items in a set
 * @param item Current set item
 * @param self Pointer to a set
 */
#define set_for_each(item, self)                                  \
    for (auto(item) = (self)->begin; item; (item) = (item)->next) \
        if ((item)->key.size)

/**
 * @brief Create a new set
 * @param arena Pointer to an arena allocator
 * @return New set instance
 */
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

/**
 * @brief Insert a new item into a set
 * @param self Pointer to a set
 * @param key Pointer to the key data
 * @param size Size of the key data in bytes (optional)
 * @return `true` if the key is newly added, or `false` if it already exists
 * @note If `size == 0`, `key` is assumed to be a null-terminated string
 */
static bool set_insert(Set *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = &self->begin;
    for (auto hash = x__set_hash_fnv1a(key, size); *item; hash <<= set_hash_shift) {
        if (!(*item)->key.size) {
            break;
        }
        if (x__set_key_equals(*item, key, size)) {
            return false;
        }
        item = &(*item)->child[hash >> set_branch_select];
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

/**
 * @brief Remove an item from a set
 * @param self Pointer to a set
 * @param key Pointer to the key data
 * @param size Size of the key data in bytes (optional)
 * @return `true` if the key is successfully removed, or `false` if it is not found
 * @note If `size == 0`, `key` is assumed to be a null-terminated string
 */
static bool set_remove(Set *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__set_hash_fnv1a(key, size); item; hash <<= set_hash_shift) {
        if (x__set_key_equals(item, key, size)) {
            item->key.size = 0;
            self->length -= 1;
            return true;
        }
        item = item->child[hash >> set_branch_select];
    }
    return false;
}

/**
 * @brief Find an item of a set
 * @param self Pointer to a set
 * @param key Pointer to the key data
 * @param size Size of the key data in bytes (optional)
 * @return `true` if the key exists, or `false` if it is not found
 * @note If `size == 0`, `key` is assumed to be a null-terminated string
 */
static bool set_find(const Set *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    auto item = self->begin;
    for (auto hash = x__set_hash_fnv1a(key, size); item; hash <<= set_hash_shift) {
        if (x__set_key_equals(item, key, size)) {
            return true;
        }
        item = item->child[hash >> set_branch_select];
    }
    return false;
}

/**
 * @brief Create a clone of a set
 * @param self Pointer to a set
 * @param arena Pointer to an arena allocator (optional)
 * @return Cloned set instance
 * @note If no arena allocator is passed, the arena allocator of the set is used
 */
static Set set_clone(const Set *self, Arena *arena) {
    Set dict = {};
    dict.arena = arena ? arena : self->arena;
    set_for_each(item, self) {
        set_insert(&dict, item->key.data, item->key.size);
    }
    return dict;
}

/**
 * @brief Retrieve an array of set items
 * @param self Pointer to a set
 * @param arena Pointer to an arena allocator (optional)
 * @return Pointer to an array of items
 * @note If no arena allocator is passed, the arena allocator of the set is used
 */
static SetItem *set_items(const Set *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    SetItem *items = arena_malloc(arena, self->length, sizeof(SetItem), alignof(SetItem));
    long index = 0;
    set_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}

/**
 * @brief Compute the union of two sets
 * @param self Pointer to a set
 * @param other Pointer to another set
 * @param arena Pointer to an arena allocator (optional)
 * @return New set containing all unique items from both sets
 */
static Set set_union(const Set *self, const Set *other, Arena *arena) {
    Set set = set_clone(self, arena);
    set_for_each(item, other) {
        set_insert(&set, item->key.data, item->key.size);
    }
    return set;
}

/**
 * @brief Compute the intersection of two sets
 * @param self Pointer to a set
 * @param other Pointer to another set
 * @param arena Pointer to an arena allocator (optional)
 * @return New set containing items common to both sets
 */
static Set set_intersection(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

/**
 * @brief Compute the difference of two sets
 * @param self Pointer to a set
 * @param other Pointer to another set
 * @param arena Pointer to an arena allocator (optional)
 * @return New set containing items in the first set but not in the second
 */
static Set set_difference(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

/**
 * @brief Compute the symmetric difference of two sets
 * @param self Pointer to a set
 * @param other Pointer to another set
 * @param arena Pointer to an arena allocator (optional)
 * @return New set containing items that are in either of the sets but not in both
 */
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

/**
 * @brief Check if one set is a subset of another
 * @param self Pointer to a set
 * @param other Pointer to another set
 * @return `true` if all items in the first set are also in the second, or `false` otherwise
 */
static bool set_is_subset(const Set *self, const Set *other) {
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Check if one set is a superset of another
 * @param self Pointer to a set
 * @param other Pointer to another set
 * @return `true` if all items in the second set are also in the first, or `false` otherwise
 */
static bool set_is_superset(const Set *self, const Set *other) {
    return set_is_subset(other, self);
}
