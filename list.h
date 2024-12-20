#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// general purpose doubly linked list
typedef struct List List;
typedef struct ListItem ListItem;
typedef int ListDataCompare(const void *, const void *);
typedef void *ListDataCopy(void *, const void *, size_t);
typedef void ListDataFree(void *);

struct List {
    long size;
    size_t data_size;
    ListDataCompare *data_cmp;
    ListDataCopy *data_copy;
    ListDataFree *data_free;
    ListItem *head, *tail;
};

struct ListItem {
    void *data;
    ListItem *next, *prev;
};

#define ListForEach(item, list) for (ListItem *item = (list)->head; item; item = item->next)

#define ListForEachReverse(item, list) for (ListItem *item = (list)->tail; item; item = item->prev)

// create an empty list
static List list_create_full(size_t data_size, ListDataCompare *data_cmp, ListDataCopy *data_copy,
                             ListDataFree *data_free)
{
    return (List){
        .data_size = data_size,
        .data_cmp = data_cmp,
        .data_copy = data_copy,
        .data_free = data_free,
    };
}
static List list_create(size_t data_size, ListDataCompare *data_cmp)
{
    return list_create_full(data_size, data_cmp, memcpy, free);
}

static ListItem *x__list_item_create(const List *list, void *data)
{
    ListItem *item = malloc(sizeof(*item));
    assert(item);
    if (data && list->data_copy) {
        item->data = malloc(list->data_size);
        assert(item->data);
        list->data_copy(item->data, data, list->data_size);
    }
    else {
        item->data = data;
    }
    item->next = 0;
    item->prev = 0;
    return item;
}

// insert an item at a given position
static void list_insert(List *list, long i, void *data)
{
    assert(list);
    assert(-list->size <= i && i <= list->size);
    ListItem *item = x__list_item_create(list, data);
    if (list->size == 0) {
        list->head = item;
        list->tail = item;
    }
    else if (i == 0 || i == -list->size) {
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
    }
    else if (i == list->size) {
        item->prev = list->tail;
        list->tail->next = item;
        list->tail = item;
    }
    else {
        i = (list->size + i) % list->size;
        ListItem *next = 0;
        if (i <= list->size - 1 - i) {
            next = list->head;
            for (long n = 0; n < i; ++n) next = next->next;
        }
        else {
            next = list->tail;
            for (long n = 0; n < list->size - 1 - i; ++n) next = next->prev;
        }
        item->next = next;
        item->prev = next->prev;
        item->next->prev = item;
        item->prev->next = item;
    }
    list->size += 1;
}

// add an item to the end of the list
static void list_append(List *list, void *data)
{
    list_insert(list, list->size, data);
}

// return a copy of the list
static List list_copy(const List *list)
{
    assert(list);
    List copy = list_create_full(list->data_size, list->data_cmp, list->data_copy, list->data_free);
    if (list->size == 0) return copy;
    for (const ListItem *item = list->head; item; item = item->next) list_append(&copy, item->data);
    return copy;
}

// remove the item at the given position in the list, and return its data
static void *list_pop(List *list, long i)
{
    assert(list);
    assert(-list->size <= i && i < list->size);
    if (list->size == 0) return 0;
    ListItem *item = 0;
    if (list->size == 1) {
        item = list->head;
        list->head = 0;
        list->tail = 0;
    }
    else if (i == 0 || i == -list->size) {
        item = list->head;
        list->head = item->next;
        list->head->prev = 0;
    }
    else if (i == list->size - 1 || i == -1) {
        item = list->tail;
        list->tail = item->prev;
        list->tail->next = 0;
    }
    else {
        i = (list->size + i) % list->size;
        if (i <= list->size - 1 - i) {
            item = list->head;
            for (long n = 0; n < i; ++n) item = item->next;
        }
        else {
            item = list->tail;
            for (long n = 0; n < list->size - 1 - i; ++n) item = item->prev;
        }
        item->next->prev = item->prev;
        item->prev->next = item->next;
    }
    void *data = item->data;
    free(item);
    list->size -= 1;
    return data;
}

// remove the first item from the list whose value is equal to data, and return its data
static void *list_remove(List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    if (list->size == 0) return 0;
    for (ListItem *item = list->head; item; item = item->next) {
        if (!list->data_cmp(item->data, data)) {
            if (item == list->head) {
                list->head = item->next;
                if (list->head) list->head->prev = 0;
            }
            else if (item == list->tail) {
                list->tail = item->prev;
                list->tail->next = 0;
            }
            else {
                item->next->prev = item->prev;
                item->prev->next = item->next;
            }
            void *item_data = item->data;
            free(item);
            list->size -= 1;
            return item_data;
        }
    }
    return 0;
}

// return zero-based index in the list of the first item whose value is equal to data
static size_t list_index(const List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    if (list->size == 0) return -1;
    long i = 0;
    for (const ListItem *item = list->head; item; item = item->next, ++i)
        if (!list->data_cmp(item->data, data)) return i;
    return -1;
}

// return the data of the first item from the list whose value is equal to data
static void *list_find(const List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    if (list->size == 0) return 0;
    for (const ListItem *item = list->head; item; item = item->next)
        if (!list->data_cmp(item->data, data)) return item->data;
    return 0;
}

// return the number of times data appears in the list
static long list_count(const List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    if (list->size == 0) return 0;
    long n = 0;
    for (const ListItem *item = list->head; item; item = item->next)
        if (!list->data_cmp(item->data, data)) n += 1;
    return n;
}

static ListItem *x__list_merge_sort_split(ListItem *first)
{
    ListItem *slow = first, *fast = first;
    while (fast && fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    ListItem *second = slow->next;
    slow->next = 0;
    if (second) second->prev = 0;
    return second;
}

static ListItem *x__list_merge_sort_merge(ListItem *first, ListItem *second, ListDataCompare cmp,
                                          int order)
{
    if (!first) return second;
    if (!second) return first;
    if (order * cmp(first->data, second->data) < 0) {
        first->next = x__list_merge_sort_merge(first->next, second, cmp, order);
        if (first->next) first->next->prev = first;
        first->prev = 0;
        return first;
    }
    second->next = x__list_merge_sort_merge(first, second->next, cmp, order);
    if (second->next) second->next->prev = second;
    second->prev = 0;
    return second;
}

static ListItem *x__list_merge_sort(ListItem *first, ListDataCompare cmp, int order)
{
    if (!first || !first->next) return first;
    ListItem *second = x__list_merge_sort_split(first);
    first = x__list_merge_sort(first, cmp, order);
    second = x__list_merge_sort(second, cmp, order);
    return x__list_merge_sort_merge(first, second, cmp, order);
}

// sort the items of the list in place
static void list_sort(List *list, int reverse)
{
    assert(list);
    assert(list->data_cmp);
    if (list->size == 0) return;
    list->head = x__list_merge_sort(list->head, list->data_cmp, (reverse ? -1 : 1));
    ListItem *item = list->head;
    while (item && item->next) item = item->next;
    list->tail = item;
}

// reverse the elements of the list in place
static void list_reverse(List *list)
{
    assert(list);
    if (list->size == 0) return;
    ListItem *item = list->head;
    while (item) {
        ListItem *swap = item->prev;
        item->prev = item->next;
        item->next = swap;
        item = item->prev;
    }
    ListItem *swap = list->head;
    list->head = list->tail;
    list->tail = swap;
}

// remove all items from the list
static void list_clear(List *list)
{
    assert(list);
    if (list->size == 0) return;
    for (ListItem *item = list->head, *next; item; item = next) {
        next = item->next;
        if (list->data_free) list->data_free(item->data);
        free(item);
    }
    *list = (List){0};
}
