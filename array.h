#pragma once

#include <assert.h>
#include <stdlib.h>

// general purpose dynamic array
typedef struct Array Array;
typedef struct ArrayItem ArrayItem;
typedef int ArrayDataCompare(const void *, const void *);
typedef void *ArrayDataCopy(void *, const void *, size_t);
typedef void ArrayDataFree(void *);

struct Array {
    long size, capacity;
    size_t data_size;
    ArrayDataCompare *data_cmp;
    ArrayDataCopy *data_copy;
    ArrayDataFree *data_free;
    ArrayItem *item;
};

struct ArrayItem {
    void *data;
};

#define ArrayForEach(item, array) \
    for (ArrayItem *item = (array)->item; item < (array)->item + (array)->size; ++item)

#define ArrayForEachReverse(item, array) \
    for (ArrayItem *item = (array)->item + (array)->size - 1; item >= (array)->item; --item)

// create an empty array
static Array array_create(long capacity, size_t data_size, ArrayDataCompare *data_cmp,
                          ArrayDataCopy *data_copy, ArrayDataFree *data_free)
{
    assert(capacity >= 0);
    return (Array){
        .capacity = capacity,
        .data_size = data_size,
        .data_cmp = data_cmp,
        .data_copy = data_copy,
        .data_free = data_free,
    };
}

static void x__array_create_items(Array *array)
{
    array->item = calloc(array->capacity, sizeof(*array->item));
    assert(array->item);
}

static void x__array_resize_items(Array *array)
{
    assert(array);
    array->capacity = 2 * array->capacity + 1;
    array->item = realloc(array->item, array->capacity * sizeof(*array->item));
    assert(array->item);
}

static void x__array_item_create(const Array *array, ArrayItem *item, void *data)
{
    if (data && array->data_copy) {
        item->data = malloc(array->data_size);
        assert(item->data);
        array->data_copy(item->data, data, array->data_size);
    }
    else {
        item->data = data;
    }
}

// insert an item at a given position
[[maybe_unused]] static void array_insert(Array *array, long i, void *data)
{
    assert(array);
    assert(-array->size <= i && i <= array->size);
    if (!array->item) x__array_create_items(array);
    if (array->size + 1 > array->capacity) x__array_resize_items(array);
    if (i != array->size) {
        i = (array->size + i) % array->size;
        for (long j = array->size; j > i; --j) array->item[j] = array->item[j - 1];
    }
    x__array_item_create(array, &array->item[i], data);
    array->size += 1;
}

// add an item to the end of the array
[[maybe_unused]] static void array_append(Array *array, void *data)
{
    array_insert(array, array->size, data);
}

// return a copy of the array
[[maybe_unused]] static Array array_copy(const Array *array)
{
    assert(array);
    Array copy = array_create(array->capacity, array->data_size, array->data_cmp, array->data_copy,
                              array->data_free);
    if (array->size == 0) return copy;
    for (const ArrayItem *item = array->item; item < array->item + array->size; ++item)
        array_append(&copy, item->data);
    return copy;
}

// remove the item at the given position in the array, and return its data
[[maybe_unused]] static void *array_pop(Array *array, long i)
{
    assert(array);
    assert(-array->size <= i && i < array->size);
    if (array->size == 0) return 0;
    i = (array->size + i) % array->size;
    void *data = array->item[i].data;
    if (i != array->size)
        for (long j = i; j < array->size; ++j) array->item[j] = array->item[j + 1];
    array->size -= 1;
    return data;
}

// remove the first item from the array whose value is equal to data, and return its data
[[maybe_unused]] static void *array_remove(Array *array, const void *data)
{
    assert(array);
    assert(array->data_cmp);
    if (array->size == 0) return 0;
    for (long i = 0; i < array->size; ++i) {
        if (!array->data_cmp(array->item[i].data, data)) {
            void *item_data = array->item[i].data;
            if (i != array->size)
                for (long j = i; j < array->size; ++j) array->item[j] = array->item[j + 1];
            array->size -= 1;
            return item_data;
        }
    }
    return 0;
}

// return zero-based index in the array of the first item whose value is equal to data
[[maybe_unused]] static size_t array_index(const Array *array, const void *data)
{
    assert(array);
    assert(array->data_cmp);
    if (array->size == 0) return 0;
    for (long i = 0; i < array->size; ++i)
        if (!array->data_cmp(array->item[i].data, data)) return i;
    return -1;
}

// return the data of the first item from the array whose value is equal to data
[[maybe_unused]] static void *array_find(const Array *array, const void *data)
{
    assert(array);
    assert(array->data_cmp);
    if (array->size == 0) return 0;
    for (long i = 0; i < array->size; ++i)
        if (!array->data_cmp(array->item[i].data, data)) return array->item[i].data;
    return 0;
}

// return the number of times data appears in the array
[[maybe_unused]] static long array_count(const Array *array, const void *data)
{
    assert(array);
    assert(array->data_cmp);
    if (array->size == 0) return 0;
    long n = 0;
    for (long i = 0; i < array->size; ++i)
        if (!array->data_cmp(array->item[i].data, data)) n += 1;
    return n;
}

static long x__array_quick_sort_partition(ArrayItem *item, long low, long high,
                                          ArrayDataCompare cmp, int order)
{
    long p = low + rand() % (high - low + 1), i = low + 1, j = high;
    ArrayItem pivot = item[p];
    item[p] = item[low];
    item[low] = pivot;
    while (1) {
        while (i <= high && order * cmp(item[i].data, pivot.data) <= 0) i += 1;
        while (j >= low && order * cmp(item[j].data, pivot.data) > 0) j -= 1;
        if (i >= j) break;
        ArrayItem swap = item[i];
        item[i] = item[j];
        item[j] = swap;
    }
    item[low] = item[j];
    item[j] = pivot;
    return j;
}

static void x__array_quick_sort(ArrayItem *item, long low, long high, ArrayDataCompare cmp,
                                int order)
{
    if (low < high) {
        long pivot = x__array_quick_sort_partition(item, low, high, cmp, order);
        x__array_quick_sort(item, low, pivot - 1, cmp, order);
        x__array_quick_sort(item, pivot + 1, high, cmp, order);
    }
}

// sort the items of the array in place
[[maybe_unused]] static void array_sort(Array *array, int reverse)
{
    assert(array);
    assert(array->data_cmp);
    if (array->size == 0) return;
    x__array_quick_sort(array->item, 0, array->size - 1, array->data_cmp, (reverse ? -1 : 1));
}

// reverse the elements of the array in place
[[maybe_unused]] static void array_reverse(Array *array)
{
    assert(array);
    if (array->size == 0) return;
    for (long i = 0, j = array->size - 1; i < array->size / 2; ++i, --j) {
        ArrayItem swap = array->item[i];
        array->item[i] = array->item[j];
        array->item[j] = swap;
    }
}

// remove all items from the array
[[maybe_unused]] static void array_clear(Array *array)
{
    assert(array);
    if (array->size == 0) return;
    if (array->data_free)
        for (long i = 0; i < array->size; ++i) array->data_free(array->item[i].data);
    free(array->item);
    array->item = 0;
    array->size = 0;
}
