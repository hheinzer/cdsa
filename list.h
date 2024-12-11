#pragma once

#include <assert.h>
#include <stdlib.h>

typedef struct List List;
typedef struct ListItem ListItem;

struct List {
    long size;
    ListItem *head, *tail;
};

struct ListItem {
    void *data;
    ListItem *next, *prev;
};

static void list_insert(List *list, long i, void *data,
                        void *data_copy(void *, const void *, size_t), size_t data_size)
{
    assert(list);
    assert(-list->size <= i && i <= list->size);
    ListItem *item = calloc(1, sizeof(*item));
    assert(item);
    if (data && data_copy) {
        item->data = malloc(data_size);
        assert(item->data);
        data_copy(item->data, data, data_size);
    }
    else {
        item->data = data;
    }
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

static void list_append(List *list, void *data, void *data_copy(void *, const void *, size_t),
                        size_t data_size)
{
    list_insert(list, list->size, data, data_copy, data_size);
}

static void *list_pop(List *list, long i)
{
    assert(list);
    if (list->size == 0) return 0;
    assert(-list->size <= i && i < list->size);
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
    else if (i == list->size - 1) {
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

static void *list_remove(List *list, const void *data, int data_cmp(const void *, const void *))
{
    assert(list);
    assert(data_cmp);
    for (ListItem *item = list->head; item; item = item->next) {
        if (!data_cmp(item->data, data)) {
            if (item == list->head) {
                list->head = item->next;
                list->head->prev = 0;
            }
            else if (item == list->tail) {
                list->tail = item->prev;
                list->tail->next = 0;
            }
            else {
                item->next->prev = item->prev;
                item->prev->next = item->next;
            }
            void *data = item->data;
            free(item);
            list->size -= 1;
            return data;
        }
    }
    return 0;
}

static void list_clear(List *list, void data_free(void *))
{
    assert(list);
    for (ListItem *item = list->head, *next; item; item = next) {
        next = item->next;
        if (data_free) data_free(item->data);
        free(item);
    }
}

static long list_index(List *list, const void *data, int data_cmp(const void *, const void *))
{
    assert(list);
    assert(data_cmp);
    long i = 0;
    for (const ListItem *item = list->head; item; item = item->next, ++i)
        if (!data_cmp(item->data, data)) return i;
    return -1;
}

static void *list_find(List *list, const void *data, int data_cmp(const void *, const void *))
{
    assert(list);
    assert(data_cmp);
    for (const ListItem *item = list->head; item; item = item->next)
        if (!data_cmp(item->data, data)) return item->data;
    return 0;
}

static long list_count(List *list, const void *data, int data_cmp(const void *, const void *))
{
    assert(list);
    assert(data_cmp);
    long n = 0;
    for (const ListItem *item = list->head; item; item = item->next)
        if (!data_cmp(item->data, data)) n += 1;
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

static ListItem *x__list_merge_sort_merge(ListItem *first, ListItem *second,
                                          int data_cmp(const void *, const void *))
{
    if (!first) return second;
    if (!second) return first;
    if (data_cmp(first->data, second->data) < 0) {
        first->next = x__list_merge_sort_merge(first->next, second, data_cmp);
        if (first->next) first->next->prev = first;
        first->prev = 0;
        return first;
    }
    else {
        second->next = x__list_merge_sort_merge(first, second->next, data_cmp);
        if (second->next) second->next->prev = second;
        second->prev = 0;
        return second;
    }
}

static ListItem *x__list_merge_sort(ListItem *first, int data_cmp(const void *, const void *))
{
    if (!first || !first->next) return first;
    ListItem *second = x__list_merge_sort_split(first);
    first = x__list_merge_sort(first, data_cmp);
    second = x__list_merge_sort(second, data_cmp);
    return x__list_merge_sort_merge(first, second, data_cmp);
}

static void list_sort(List *list, int data_cmp(const void *, const void *))
{
    assert(list);
    assert(data_cmp);
    list->head = x__list_merge_sort(list->head, data_cmp);
    ListItem *item = list->head;
    while (item && item->next) item = item->next;
    list->tail = item;
}

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

static List list_copy(const List *list, void *data_copy(void *, const void *, size_t),
                      size_t data_size)
{
    assert(list);
    List copy = {0};
    for (const ListItem *item = list->head; item; item = item->next)
        list_append(&copy, item->data, data_copy, data_size);
    return copy;
}
