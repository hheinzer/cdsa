#pragma once

#include <assert.h>
#include <stdlib.h>

// general purpose doubly linked list
typedef struct List List;
typedef struct ListItem ListItem;
typedef void *ListDataCopy(void *, const void *, size_t);
typedef int ListDataCompare(const void *, const void *);
typedef void ListDataFree(void *);

struct List {
    size_t data_size;           // size of item data in bytes
    ListDataCopy *data_copy;    // pointer to a function for copying data
    ListDataCompare *data_cmp;  // pointer to a function for comparing data
    ListDataFree *data_free;    // pointer to a function for freeing data
    long size;                  // number of items in list
    ListItem *head, *tail;      // pointers to first and last item
};

struct ListItem {
    void *data;             // pointer to stored data
    ListItem *next, *prev;  // pointers to next and previous item
};

// create an empty list
static List list_create(size_t data_size, ListDataCopy data_copy, ListDataCompare data_cmp,
                        ListDataFree data_free)
{
    return (List){.data_size = data_size,
                  .data_copy = data_copy,
                  .data_cmp = data_cmp,
                  .data_free = data_free};
}

// insert an item at a given position
static void list_insert(List *list, long i, void *data)
{
    assert(list);
    assert(-list->size <= i && i <= list->size);

    // create item
    ListItem *item = calloc(1, sizeof(*item));
    assert(item);
    if (data && list->data_copy) {
        item->data = malloc(list->data_size);
        assert(item->data);
        list->data_copy(item->data, data, list->data_size);
    }
    else {
        item->data = data;
    }

    // insert item
    if (list->size == 0) {  // empty list
        list->head = item;
        list->tail = item;
    }
    else if (i == 0 || i == -list->size) {  // item is first item
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
    }
    else if (i == list->size) {  // insert is last item
        item->prev = list->tail;
        list->tail->next = item;
        list->tail = item;
    }
    else {
        i = (list->size + i) % list->size;  // convert possibly negative position
        ListItem *next = 0;
        if (i <= list->size - 1 - i) {  // search forward
            next = list->head;
            for (long n = 0; n < i; ++n) next = next->next;
        }
        else {  // search backward
            next = list->tail;
            for (long n = 0; n < list->size - 1 - i; ++n) next = next->prev;
        }
        item->next = next;
        item->prev = next->prev;
        item->next->prev = item;
        item->prev->next = item;
    }

    // housekeeping
    list->size += 1;
}

// add an item to the end of the list
static void list_append(List *list, void *data)
{
    list_insert(list, list->size, data);
}

// remove the item at the given position in the list, and return its data
static void *list_pop(List *list, long i)
{
    assert(list);
    if (list->size == 0) return 0;  // empty list
    assert(-list->size <= i && i < list->size);

    // pop item
    ListItem *item = 0;
    if (list->size == 1) {  // item is first and only item
        item = list->head;
        list->head = 0;
        list->tail = 0;
    }
    else if (i == 0 || i == -list->size) {  // item is first item
        item = list->head;
        list->head = item->next;
        list->head->prev = 0;
    }
    else if (i == list->size - 1) {  // item is last item
        item = list->tail;
        list->tail = item->prev;
        list->tail->next = 0;
    }
    else {
        i = (list->size + i) % list->size;  // convert possibly negative position
        if (i <= list->size - 1 - i) {      // search forward
            item = list->head;
            for (long n = 0; n < i; ++n) item = item->next;
        }
        else {  // search backward
            item = list->tail;
            for (long n = 0; n < list->size - 1 - i; ++n) item = item->prev;
        }
        item->next->prev = item->prev;
        item->prev->next = item->next;
    }

    // housekeeping
    list->size -= 1;

    // return item
    void *data = item->data;
    free(item);
    return data;
}

// remove the first item from the list whose value is equal to data
static void *list_remove(List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);

    // remove item
    for (ListItem *item = list->head; item; item = item->next) {
        if (!list->data_cmp(item->data, data)) {
            if (item == list->head) {  // item is first item
                list->head = item->next;
                list->head->prev = 0;
            }
            else if (item == list->tail) {  // item is last item
                list->tail = item->prev;
                list->tail->next = 0;
            }
            else {
                item->next->prev = item->prev;
                item->prev->next = item->next;
            }

            // housekeeping
            list->size -= 1;

            // return item
            void *data = item->data;
            free(item);
            return data;
        }
    }

    // no matching item
    return 0;
}

// remove all items from the list
static void list_clear(List *list)
{
    assert(list);
    for (ListItem *item = list->head, *next; item; item = next) {
        next = item->next;
        if (list->data_free) list->data_free(item->data);
        free(item);
    }
}

// return zero-based index in the list of the first item whose value is equal to data
static long list_index(List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    long i = 0;
    for (const ListItem *item = list->head; item; item = item->next, ++i)
        if (!list->data_cmp(item->data, data)) return i;
    return -1;
}

// return the first item from the list whose value is equal to data
static void *list_find(List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    for (const ListItem *item = list->head; item; item = item->next)
        if (!list->data_cmp(item->data, data)) return item->data;
    return 0;
}

// return the number of times data appears in the list
static long list_count(List *list, const void *data)
{
    assert(list);
    assert(list->data_cmp);
    long n = 0;
    for (const ListItem *item = list->head; item; item = item->next)
        if (!list->data_cmp(item->data, data)) n += 1;
    return n;
}

// split a linked list into two halves
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

// merge two sorted list into one sorted list
static ListItem *x__list_merge_sort_merge(ListItem *first, ListItem *second,
                                          int data_cmp(const void *, const void *), int order)
{
    if (!first) return second;
    if (!second) return first;
    if (order * data_cmp(first->data, second->data) < 0) {
        first->next = x__list_merge_sort_merge(first->next, second, data_cmp, order);
        if (first->next) first->next->prev = first;
        first->prev = 0;
        return first;
    }
    else {
        second->next = x__list_merge_sort_merge(first, second->next, data_cmp, order);
        if (second->next) second->next->prev = second;
        second->prev = 0;
        return second;
    }
}

// perform merge sort on list
static ListItem *x__list_merge_sort(ListItem *first, int data_cmp(const void *, const void *),
                                    int order)
{
    if (!first || !first->next) return first;
    ListItem *second = x__list_merge_sort_split(first);
    first = x__list_merge_sort(first, data_cmp, order);
    second = x__list_merge_sort(second, data_cmp, order);
    return x__list_merge_sort_merge(first, second, data_cmp, order);
}

// sort the items of the list in place
static void list_sort(List *list, int reverse)
{
    assert(list);
    assert(list->data_cmp);
    list->head = x__list_merge_sort(list->head, list->data_cmp, (reverse ? -1 : 1));
    ListItem *item = list->head;
    while (item && item->next) item = item->next;
    list->tail = item;
}

// reverse the elements of the list in place
static void list_reverse(List *list)
{
    assert(list);
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

// return a copy of the list
static List list_copy(const List *list)
{
    assert(list);
    List copy = list_create(list->data_size, list->data_copy, list->data_cmp, list->data_free);
    for (const ListItem *item = list->head; item; item = item->next) list_append(&copy, item->data);
    return copy;
}
