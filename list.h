#pragma once

#include <assert.h>
#include <stddef.h>

#include "arena.h"

typedef struct List List;
typedef struct ListItem ListItem;

typedef int ListDataCompare(const void *, const void *, void *);
typedef void *ListDataCopy(Arena *, void *, const void *, long);

struct List {
    Arena *arena;
    struct {
        long size;
        ListDataCompare *compare;
        ListDataCopy *copy;
    } data;
    long length;
    ListItem *begin;
    ListItem *end;
};

struct ListItem {
    void *data;
    ListItem *next;
    ListItem *prev;
};

#define list_for_each(item, self) \
    for (ListItem * (item) = (self)->begin; item; (item) = (item)->next)

static List list_create(Arena *arena, long size, ListDataCompare *compare) {
    List list = {};
    list.arena = arena;
    list.data.size = size;
    list.data.compare = compare;
    list.data.copy = arena_memcpy;
    return list;
}

static void x__list_item_create(const List *self, ListItem *item, void *data) {
    if (data && self->data.size) {
        item->data = arena_malloc(self->arena, 1, self->data.size, alignof(max_align_t));
        self->data.copy(self->arena, item->data, data, self->data.size);
    }
    else {
        item->data = data;
    }
}

static void list_insert(List *self, long index, void *data) {
    assert(-self->length <= index && index <= self->length);
    ListItem *item = arena_calloc(self->arena, 1, sizeof(ListItem), alignof(ListItem));
    x__list_item_create(self, item, data);
    if (self->length == 0) {
        self->begin = item;
        self->end = item;
    }
    else if (index == 0 || index == -self->length) {
        item->next = self->begin;
        self->begin->prev = item;
        self->begin = item;
    }
    else if (index == self->length) {
        item->prev = self->end;
        self->end->next = item;
        self->end = item;
    }
    else {
        index = (self->length + index) % self->length;
        ListItem *next = nullptr;
        if (index <= (self->length - 1) / 2) {
            next = self->begin;
            for (long i = 0; i < index; i++) {
                next = next->next;
            }
        }
        else {
            next = self->end;
            for (long i = self->length - 1; i > index; i--) {
                next = next->prev;
            }
        }
        item->next = next;
        item->prev = next->prev;
        item->next->prev = item;
        item->prev->next = item;
    }
    self->length += 1;
}

static void list_append(List *self, void *data) {
    list_insert(self, self->length, data);
}

static void *list_pop(List *self, long index) {
    if (self->length == 0) {
        return nullptr;
    }
    assert(-self->length <= index && index < self->length);
    ListItem *item = nullptr;
    if (self->length == 1) {
        item = self->begin;
        self->begin = nullptr;
        self->end = nullptr;
    }
    else if (index == 0 || index == -self->length) {
        item = self->begin;
        self->begin = item->next;
        self->begin->prev = nullptr;
    }
    else if (index == self->length - 1 || index == -1) {
        item = self->end;
        self->end = item->prev;
        self->end->next = nullptr;
    }
    else {
        index = (self->length + index) % self->length;
        if (index <= (self->length - 1) / 2) {
            item = self->begin;
            for (long i = 0; i < index; i++) {
                item = item->next;
            }
        }
        else {
            item = self->end;
            for (long i = self->length - 1; i > index; i--) {
                item = item->prev;
            }
        }
        item->next->prev = item->prev;
        item->prev->next = item->next;
    }
    self->length -= 1;
    return item->data;
}

static void *list_remove(List *self, const void *data) {
    assert(self->data.compare);
    list_for_each(item, self) {
        if (self->data.compare(item->data, data, nullptr)) {
            continue;
        }
        if (self->length == 1) {
            self->begin = nullptr;
            self->end = nullptr;
        }
        else if (item == self->begin) {
            self->begin = item->next;
            self->begin->prev = nullptr;
        }
        else if (item == self->end) {
            self->end = item->prev;
            self->end->next = nullptr;
        }
        else {
            item->next->prev = item->prev;
            item->prev->next = item->next;
        }
        self->length -= 1;
        return item->data;
    }
    return nullptr;
}

static void *list_get(const List *self, long index) {
    if (self->length == 0) {
        return nullptr;
    }
    assert(-self->length <= index && index < self->length);
    ListItem *item = nullptr;
    index = (self->length + index) % self->length;
    if (index <= (self->length - 1) / 2) {
        item = self->begin;
        for (long i = 0; i < index; i++) {
            item = item->next;
        }
    }
    else {
        item = self->end;
        for (long i = self->length - 1; i > index; i--) {
            item = item->prev;
        }
    }
    return item->data;
}

static void *list_find(const List *self, const void *data) {
    assert(self->data.compare);
    list_for_each(item, self) {
        if (!self->data.compare(item->data, data, nullptr)) {
            return item->data;
        }
    }
    return nullptr;
}

static long list_index(const List *self, const void *data) {
    assert(self->data.compare);
    long index = 0;
    list_for_each(item, self) {
        if (!self->data.compare(item->data, data, nullptr)) {
            return index;
        }
        index += 1;
    }
    return -1;
}

static long list_count(const List *self, const void *data) {
    assert(self->data.compare);
    long count = 0;
    list_for_each(item, self) {
        if (!self->data.compare(item->data, data, nullptr)) {
            count += 1;
        }
    }
    return count;
}

static ListItem *x__list_merge_sort_split(ListItem *first) {
    ListItem *slow = first;
    ListItem *fast = first;
    while (fast && fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    ListItem *second = slow->next;
    slow->next = nullptr;
    if (second) {
        second->prev = nullptr;
    }
    return second;
}

static ListItem *x__list_merge_sort_merge(List *self, ListItem *first, ListItem *second,
                                          void *context) {
    if (!first) {
        return second;
    }
    if (!second) {
        return first;
    }
    if (self->data.compare(first->data, second->data, context) < 0) {
        first->next = x__list_merge_sort_merge(self, first->next, second, context);
        if (first->next) {
            first->next->prev = first;
        }
        first->prev = nullptr;
        return first;
    }
    second->next = x__list_merge_sort_merge(self, first, second->next, context);
    if (second->next) {
        second->next->prev = second;
    }
    second->prev = nullptr;
    return second;
}

static ListItem *x__list_merge_sort(List *self, ListItem *first, void *context) {
    if (!first || !first->next) {
        return first;
    }
    ListItem *second = x__list_merge_sort_split(first);
    first = x__list_merge_sort(self, first, context);
    second = x__list_merge_sort(self, second, context);
    return x__list_merge_sort_merge(self, first, second, context);
}

static void list_sort(List *self, void *context) {
    assert(self->data.compare);
    self->begin = x__list_merge_sort(self, self->begin, context);
    ListItem *item = self->begin;
    while (item && item->next) {
        item = item->next;
    }
    self->end = item;
}

static void list_reverse(List *self) {
    for (ListItem *item = self->begin; item; item = item->prev) {
        ListItem *swap = item->next;
        item->next = item->prev;
        item->prev = swap;
    }
    ListItem *swap = self->begin;
    self->begin = self->end;
    self->end = swap;
}

static List list_clone(const List *self, Arena *arena) {
    List list = {};
    list.arena = arena ? arena : self->arena;
    list.data = self->data;
    list_for_each(item, self) {
        list_append(&list, item->data);
    }
    return list;
}

static ListItem *list_items(const List *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    ListItem *items = arena_malloc(arena, self->length, sizeof(ListItem), alignof(ListItem));
    long index = 0;
    list_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}
