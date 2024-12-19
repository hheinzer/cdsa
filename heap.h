#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// general purpose priority queue
typedef struct Heap Heap;
typedef struct HeapItem HeapItem;
typedef void *HeapDataCopy(void *, const void *, size_t);
typedef void HeapDataFree(void *);

struct Heap {
    long size, capacity;
    size_t data_size;
    HeapDataCopy *data_copy;
    HeapDataFree *data_free;
    HeapItem *item;
};

struct HeapItem {
    double priority;
    void *data;
};

#define HeapForEach(item, heap) \
    for (HeapItem *item = (heap)->item; item < (heap)->item + (heap)->size; ++item)

// create an empty heap
[[maybe_unused]] static Heap heap_create_full(long capacity, size_t data_size,
                                              HeapDataCopy *data_copy, HeapDataFree *data_free)
{
    assert(capacity >= 0);
    return (Heap){
        .capacity = capacity,
        .data_size = data_size,
        .data_copy = data_copy,
        .data_free = data_free,
    };
}
[[maybe_unused]] static Heap heap_create(long capacity, size_t data_size)
{
    return heap_create_full(capacity, data_size, memcpy, free);
}

static void x__heap_create_items(Heap *heap)
{
    heap->item = calloc(heap->capacity, sizeof(*heap->item));
    assert(heap->item);
}

static void x__heap_resize_items(Heap *heap)
{
    assert(heap);
    heap->capacity = 2 * heap->capacity + 1;
    heap->item = realloc(heap->item, heap->capacity * sizeof(*heap->item));
    assert(heap->item);
}

static long x__heap_sift_up(HeapItem *item, long size, double priority)
{
    long i = size;
    while (i > 0) {
        const long parent = (i - 1) / 2;
        if (priority <= item[parent].priority) break;
        item[i] = item[parent];
        i = parent;
    }
    return i;
}

static void x__heap_item_create(const Heap *heap, HeapItem *item, double priority, void *data)
{
    item->priority = priority;
    if (heap->data_copy) {
        item->data = malloc(heap->data_size);
        assert(item->data);
        heap->data_copy(item->data, data, heap->data_size);
    }
    else {
        item->data = data;
    }
}

// push an item with a given priority
[[maybe_unused]] static void heap_push(Heap *heap, double priority, void *data)
{
    assert(heap);
    if (!heap->item) x__heap_create_items(heap);
    if (heap->size + 1 > heap->capacity) x__heap_resize_items(heap);
    const long i = x__heap_sift_up(heap->item, heap->size, priority);
    x__heap_item_create(heap, &heap->item[i], priority, data);
    heap->size += 1;
}

// return a copy of the heap
[[maybe_unused]] static Heap heap_copy(const Heap *heap)
{
    assert(heap);
    Heap copy = heap_create_full(heap->capacity, heap->data_size, heap->data_copy, heap->data_free);
    if (heap->size == 0) return copy;
    for (const HeapItem *item = heap->item; item < heap->item + heap->size; ++item)
        heap_push(&copy, item->priority, item->data);
    return copy;
}

static long x__heap_sift_down(HeapItem *item, long size, double priority)
{
    long i = 0, left;
    while ((left = 2 * i + 1) < size) {
        const long right = left + 1;
        const long largest =
            (right < size && item[right].priority > item[left].priority ? right : left);
        if (priority >= item[largest].priority) break;
        item[i] = item[largest];
        i = largest;
    }
    return i;
}

// remove the item with the highest priority, and return its data
[[maybe_unused]] static void *heap_pop(Heap *heap)
{
    assert(heap);
    if (heap->size == 0) return 0;
    void *data = heap->item[0].data;
    heap->size -= 1;
    if (heap->size > 0) {
        const double priority = heap->item[heap->size].priority;
        const long i = x__heap_sift_down(heap->item, heap->size, priority);
        heap->item[i] = heap->item[heap->size];
    }
    return data;
}

// return the data of the first item with the highest priority
[[maybe_unused]] static void *heap_peek(const Heap *heap)
{
    assert(heap);
    if (heap->size == 0) return 0;
    return heap->item[0].data;
}

// remove all items from the heap
[[maybe_unused]] static void heap_clear(Heap *heap)
{
    assert(heap);
    if (heap->size == 0) return;
    if (heap->data_free)
        for (long i = 0; i < heap->size; ++i) heap->data_free(heap->item[i].data);
    free(heap->item);
    heap->item = 0;
    heap->size = 0;
}
