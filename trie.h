#pragma once

#include "arena.h"

typedef struct Trie Trie;
typedef struct TrieItem TrieItem;
typedef void *TrieDataCopy(void *, const void *, size_t);
typedef void TrieDataDestroy(void *);

struct Trie {
    Arena *arena;
    struct {
        long size;
        TrieDataCopy *copy;
        TrieDataDestroy *destroy;
    } data;
    long length;
    TrieItem *begin;
    TrieItem *end;
};

struct TrieItem {
    struct {
        char *data;
        long size;
    } key;
    void *data;
    TrieItem *child[4];
    TrieItem *next;
};

typedef enum {
    KEEPOLD = 1 << 1,
} TrieFlags;

#define trie_for_each(item, trie)                                 \
    for (TrieItem *item = (trie)->begin; item; item = item->next) \
        if (item->key.size)

static Trie trie_create(Arena *arena, long size) {
    Trie trie = {0};
    trie.arena = arena;
    trie.data.size = size;
    trie.data.copy = memcpy;
    return trie;
}

static uint64_t x__trie_hash_fnv1a(const char *key, long size) {
    uint64_t hash = 0xcbf29ce484222325;
    for (const char *byte = key; byte < key + size; ++byte) {
        hash ^= *byte;
        hash *= 0x00000100000001b3;
    }
    return hash;
}

static void x__trie_item_create(const Trie *self, TrieItem *item, const void *key, long size,
                                void *data) {
    item->key.data = arena_alloc(self->arena, 1, size, alignof(void *), NOZERO);
    memcpy(item->key.data, key, size);
    item->key.size = size;
    if (data && self->data.copy) {
        item->data = arena_alloc(self->arena, 1, self->data.size, alignof(void *), NOZERO);
        self->data.copy(item->data, data, self->data.size);
    } else {
        item->data = data;
    }
}

static void *trie_insert(Trie *self, const void *key, long size, void *data, int flags) {
    if (!size) {
        size = strlen(key) + 1;
    }
    TrieItem **item = &self->begin;
    for (uint64_t hash = x__trie_hash_fnv1a(key, size); *item; hash <<= 2) {
        if (!(*item)->key.size) {
            break;
        }
        if ((*item)->key.size == size && !memcmp((*item)->key.data, key, size)) {
            void *item_data = (*item)->data;
            if (!(flags & KEEPOLD)) {
                (*item)->data = data;
            }
            return item_data;
        }
        item = &(*item)->child[hash >> 62];
    }
    if (!*item) {
        *item = arena_alloc(self->arena, 1, sizeof(TrieItem), alignof(TrieItem), 0);
        if (self->end) {
            self->end->next = *item;
        }
        self->end = *item;
    }
    x__trie_item_create(self, *item, key, size, data);
    self->length += 1;
    return 0;
}

static void *trie_copy(void *_other, const void *_self, size_t) {
    Trie *other = _other;
    const Trie *self = _self;
    other->data = self->data;
    for (TrieItem *item = self->begin; item; item = item->next) {
        if (item->key.size) {
            trie_insert(other, item->key.data, item->key.size, item->data, 0);
        }
    }
    return other;
}

static Trie trie_clone(const Trie *self, Arena *arena) {
    Trie trie = {0};
    trie.arena = arena;
    trie_copy(&trie, self, 0);
    return trie;
}

static void *trie_remove(Trie *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    TrieItem *item = self->begin;
    for (uint64_t hash = x__trie_hash_fnv1a(key, size); item; hash <<= 2) {
        if (item->key.size == size && !memcmp(item->key.data, key, size)) {
            item->key.size = 0;
            self->length -= 1;
            return item->data;
        }
        item = item->child[hash >> 62];
    }
    return 0;
}

static void *trie_find(const Trie *self, const void *key, long size) {
    if (!size) {
        size = strlen(key) + 1;
    }
    TrieItem *item = self->begin;
    for (uint64_t hash = x__trie_hash_fnv1a(key, size); item; hash <<= 2) {
        if (item->key.size == size && !memcmp(item->key.data, key, size)) {
            return item->data;
        }
        item = item->child[hash >> 62];
    }
    return 0;
}

static void trie_destroy(Trie *self) {
    if (self->data.destroy) {
        for (TrieItem *item = self->begin; item; item = item->next) {
            if (item->key.size) {
                self->data.destroy(item->data);
            }
        }
    }
    *self = (Trie){0};
}
