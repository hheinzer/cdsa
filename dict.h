#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

// general purpose associative array using open chaining
typedef struct Dict Dict;
typedef struct DictItem DictItem;
typedef size_t DictKeyHash(const char *);
typedef void *DictDataCopy(void *, const void *, size_t);
typedef void DictDataFree(void *);

struct Dict {
    long size, capacity;
    size_t data_size;
    double load_factor;
    DictKeyHash *key_hash;
    DictDataCopy *data_copy;
    DictDataFree *data_free;
    DictItem *bucket;
};

struct DictItem {
    char *key;
    void *data;
    size_t hash;
    DictItem *next;
};

#define DictForEach(item, dict)                                                                   \
    for (DictItem *bucket = (dict)->bucket; bucket < (dict)->bucket + (dict)->capacity; ++bucket) \
        if (bucket->key)                                                                          \
            for (DictItem *item = bucket; item; item = item->next)

// create an empty dict
static Dict dict_create_full(long capacity, size_t data_size, double load_factor,
                             DictKeyHash *key_hash, DictDataCopy *data_copy,
                             DictDataFree *data_free)
{
    assert(capacity >= 0);
    assert(0 < load_factor && load_factor < 1);
    assert(key_hash);
    return (Dict){
        .capacity = capacity / load_factor + 1,
        .data_size = data_size,
        .load_factor = load_factor,
        .key_hash = key_hash,
        .data_copy = data_copy,
        .data_free = data_free,
    };
}
static Dict dict_create(long capacity, size_t data_size)
{
    return dict_create_full(capacity, data_size, 0.75, strhash_fnv1a, memcpy, free);
}

static void x__dict_create_buckets(Dict *dict)
{
    dict->bucket = calloc(dict->capacity, sizeof(*dict->bucket));
    assert(dict->bucket);
}

static void x__dict_resize_buckets(Dict *dict)
{
    assert(dict);
    const long _capacity = dict->capacity / dict->load_factor + 1;
    DictItem *_bucket = calloc(_capacity, sizeof(*_bucket));
    assert(_bucket);
    for (DictItem *bucket = dict->bucket; bucket < dict->bucket + dict->capacity; ++bucket) {
        if (!bucket->key) continue;
        for (DictItem *item = bucket, *next; item; item = next) {
            next = item->next;
            DictItem *_item = &_bucket[item->hash % _capacity];
            DictItem *_prev = 0;
            while (_item && _item->key) {
                _prev = _item;
                _item = _item->next;
            }
            if (!_item) {
                if (item == bucket) {
                    _item = malloc(sizeof(*_item));
                    assert(_item);
                    _item->key = item->key;
                    _item->data = item->data;
                    _item->hash = item->hash;
                }
                else {
                    _item = item;
                }
                _item->next = 0;
                assert(_prev);
                _prev->next = _item;
            }
            else {
                _item->key = item->key;
                _item->data = item->data;
                _item->hash = item->hash;
                if (item != bucket) free(item);
            }
        }
    }
    free(dict->bucket);
    dict->bucket = _bucket;
    dict->capacity = _capacity;
}

static void x__dict_item_create(const Dict *dict, DictItem *item, const char *key, void *data,
                                size_t hash)
{
    item->key = strdup(key);
    assert(item->key);
    if (data && dict->data_copy) {
        item->data = malloc(dict->data_size);
        assert(item->data);
        dict->data_copy(item->data, data, dict->data_size);
    }
    else {
        item->data = data;
    }
    item->hash = hash;
}

// insert an item with a given key; on collision, keep or replace data and return old data
static void *dict_insert(Dict *dict, const char *key, void *data, int keep)
{
    assert(dict);
    assert(key);
    if (!dict->bucket) x__dict_create_buckets(dict);
    if (dict->size + 1 > dict->capacity * dict->load_factor) x__dict_resize_buckets(dict);
    const size_t hash = dict->key_hash(key);
    DictItem *item = &dict->bucket[hash % dict->capacity];
    DictItem *prev = 0;
    while (item && item->key && (item->hash != hash || strcmp(item->key, key))) {
        prev = item;
        item = item->next;
    }
    if (!item) {
        item = malloc(sizeof(*item));
        assert(item);
        x__dict_item_create(dict, item, key, data, hash);
        item->next = 0;
        assert(prev);
        prev->next = item;
    }
    else if (!item->key) {
        x__dict_item_create(dict, item, key, data, hash);
    }
    else {
        void *item_data = item->data;
        if (!keep) item->data = data;
        return item_data;
    }
    dict->size += 1;
    return 0;
}

// return a copy of the dict
static Dict dict_copy(const Dict *dict)
{
    assert(dict);
    Dict copy = dict_create_full(dict->size, dict->data_size, dict->load_factor, dict->key_hash,
                                 dict->data_copy, dict->data_free);
    if (dict->size == 0) return copy;
    for (const DictItem *bucket = dict->bucket; bucket < dict->bucket + dict->capacity; ++bucket)
        if (bucket->key)
            for (const DictItem *item = bucket; item; item = item->next)
                dict_insert(&copy, item->key, item->data, 0);
    return copy;
}

// remove an item with a given key, and return its data
static void *dict_remove(Dict *dict, const char *key)
{
    assert(dict);
    assert(key);
    if (dict->size == 0) return 0;
    const size_t hash = dict->key_hash(key);
    DictItem *item = &dict->bucket[hash % dict->capacity];
    DictItem *prev = 0;
    while (item && item->key && (item->hash != hash || strcmp(item->key, key))) {
        prev = item;
        item = item->next;
    }
    if (!item || !item->key) return 0;
    void *data = item->data;
    DictItem *next = item->next;
    free(item->key);
    if (!prev) {
        if (next) {
            *item = *next;
            free(next);
        }
        else {
            memset(item, 0, sizeof(*item));
        }
    }
    else {
        free(item);
        prev->next = next;
    }
    dict->size -= 1;
    return data;
}

// return the data of an item with a given key
static void *dict_find(const Dict *dict, const char *key)
{
    assert(dict);
    assert(key);
    if (dict->size == 0) return 0;
    const size_t hash = dict->key_hash(key);
    DictItem *item = &dict->bucket[hash % dict->capacity];
    while (item && item->key && (item->hash != hash || strcmp(item->key, key))) item = item->next;
    return (!item || !item->key ? 0 : item->data);
}

// remove all items from the dict
static void dict_clear(Dict *dict)
{
    assert(dict);
    if (!dict->bucket) return;
    for (DictItem *bucket = dict->bucket; bucket < dict->bucket + dict->capacity; ++bucket) {
        if (!bucket->key) continue;
        free(bucket->key);
        if (dict->data_free) dict->data_free(bucket->data);
        for (DictItem *item = bucket->next, *next; item; item = next) {
            next = item->next;
            free(item->key);
            if (dict->data_free) dict->data_free(item->data);
            free(item);
        }
    }
    free(dict->bucket);
    dict->bucket = 0;
    dict->size = 0;
}
