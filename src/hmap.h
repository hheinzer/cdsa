#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// general purpose associative array using open chaining
typedef struct Hmap Hmap;
typedef struct HmapItem HmapItem;
typedef size_t HmapKeyHash(const char *);
typedef void *HmapDataCopy(void *, const void *, size_t);
typedef void HmapDataFree(void *);

struct Hmap {
    long size, capacity;      // number and maximum number of items
    float load_factor;        // load factor for resizing
    long max_dist;            // maximum probing distance
    HmapItem *item;           // array of items
    HmapKeyHash *key_hash;    // pointer to a function for hashing keys
    size_t data_size;         // size of item data in bytes
    HmapDataCopy *data_copy;  // pointer to a function for copying data
    HmapDataFree *data_free;  // pointer to a function for freeing data
};

struct HmapItem {
    char *key;    // pointer to key
    void *data;   // pointer to stored data
    size_t hash;  // hash value of key
};

#define HmapForEach(item, hmap)                                                         \
    for (HmapItem *item = (hmap)->item; item < (hmap)->item + (hmap)->capacity; ++item) \
        if (item->key)

// create an empty hmap
static Hmap hmap_create(long capacity, float load_factor, HmapKeyHash *key_hash, size_t data_size,
                        HmapDataCopy *data_copy, HmapDataFree *data_free)
{
    assert(capacity > 0);
    assert(0 < load_factor && load_factor < 1);
    assert(key_hash);
    return (Hmap){.capacity = capacity,
                  .load_factor = load_factor,
                  .key_hash = key_hash,
                  .data_size = data_size,
                  .data_copy = data_copy,
                  .data_free = data_free};
}

// create hmap items
static void x__hmap_create_items(Hmap *hmap)
{
    hmap->item = calloc(hmap->capacity, sizeof(*hmap->item));
    assert(hmap->item);
}

// resize hmap items
static void x__hmap_resize_items(Hmap *hmap)
{
    assert(hmap);

    // create new items
    const long _capacity = hmap->capacity / hmap->load_factor;
    HmapItem *_item = calloc(_capacity, sizeof(*_item));
    assert(_item);
    long _max_dist = 0;

    // insert old items
    for (HmapItem *item = hmap->item; item < hmap->item + hmap->capacity; ++item) {
        if (!item->key) continue;

        // find position
        long _dist = 0, _i = item->hash % _capacity;
        while (_item[_i].key) {
            _dist += 1;
            _i = (_i + 1) % _capacity;
        }

        // insert item (same key cannot occur)
        _item[_i].key = item->key;
        _item[_i].data = item->data;
        _item[_i].hash = item->hash;
        if (_dist > _max_dist) _max_dist = _dist;
    }

    // housekeeping
    free(hmap->item);
    hmap->item = _item;
    hmap->capacity = _capacity;
    hmap->max_dist = _max_dist;
}

// create an hmap item
static void x__hmap_item_create(const Hmap *hmap, HmapItem *item, const char *key, void *data,
                                size_t hash)
{
    item->key = strdup(key);
    assert(item->key);
    if (data && hmap->data_copy) {
        item->data = malloc(hmap->data_size);
        assert(item->data);
        hmap->data_copy(item->data, data, hmap->data_size);
    }
    else {
        item->data = data;
    }
    item->hash = hash;
}

// insert an item with a given key, swap data and return old data on collision
[[maybe_unused]] static void *hmap_insert(Hmap *hmap, const char *key, void *data)
{
    assert(hmap);
    assert(key);

    // housekeeping
    if (!hmap->item) x__hmap_create_items(hmap);
    if (hmap->size + 1 > hmap->capacity * hmap->load_factor) x__hmap_resize_items(hmap);

    // find position
    const size_t hash = hmap->key_hash(key);
    long dist = 0, i = hash % hmap->capacity;
    HmapItem *item = &hmap->item[i];
    while (item->key && (item->hash != hash || strcmp(item->key, key))) {
        dist += 1;
        i = (i + 1) % hmap->capacity;
        item = &hmap->item[i];
    }

    // insert item
    if (!item->key) {  // empty item: add item
        x__hmap_item_create(hmap, item, key, data, hash);
        if (dist > hmap->max_dist) hmap->max_dist = dist;
    }
    else {  // same key: swap data
        void *item_data = item->data;
        item->data = data;
        return item_data;
    }

    // housekeeping
    hmap->size += 1;
    return 0;
}

// return a copy of the hmap
[[maybe_unused]] static Hmap hmap_copy(const Hmap *hmap)
{
    assert(hmap);
    Hmap copy = hmap_create(hmap->capacity, hmap->load_factor, hmap->key_hash, hmap->data_size,
                            hmap->data_copy, hmap->data_free);
    if (hmap->size == 0) return copy;  // empty hmap
    for (const HmapItem *item = hmap->item; item < hmap->item + hmap->capacity; ++item)
        if (item->key) hmap_insert(&copy, item->key, item->data);
    return copy;
}

// remove an item with a given key, and return its data
[[maybe_unused]] static void *hmap_remove(Hmap *hmap, const char *key)
{
    assert(hmap);
    if (hmap->size == 0) return 0;  // empty hmap
    assert(key);
    const size_t hash = hmap->key_hash(key);
    long i = hash % hmap->capacity;
    HmapItem *item = &hmap->item[i];
    for (long dist = 0; dist <= hmap->max_dist; ++dist) {
        if (item->key && item->hash == hash && !strcmp(item->key, key)) {
            void *data = item->data;
            free(item->key);
            memset(item, 0, sizeof(*item));
            hmap->size -= 1;
            return data;
        }
        i = (i + 1) % hmap->capacity;
        item = &hmap->item[i];
    }
    return 0;
}

// return the data of an item with a given key
[[maybe_unused]] static void *hmap_find(const Hmap *hmap, const char *key)
{
    assert(hmap);
    if (hmap->size == 0) return 0;  // empty hmap
    assert(key);
    const size_t hash = hmap->key_hash(key);
    long i = hash % hmap->capacity;
    HmapItem *item = &hmap->item[i];
    for (long dist = 0; dist <= hmap->max_dist; ++dist) {
        if (item->key && item->hash == hash && !strcmp(item->key, key)) return item->data;
        i = (i + 1) % hmap->capacity;
        item = &hmap->item[i];
    }
    return 0;
}

// remove all items from the hmap
[[maybe_unused]] static void hmap_clear(Hmap *hmap)
{
    assert(hmap);
    if (hmap->size == 0) return;  // empty hmap
    for (HmapItem *item = hmap->item; item < hmap->item + hmap->capacity; ++item) {
        if (!item->key) continue;
        free(item->key);
        if (hmap->data_free) hmap->data_free(item->data);
    }
    hmap->size = 0;
    free(hmap->item);
    hmap->item = 0;
}
