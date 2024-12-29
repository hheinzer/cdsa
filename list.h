#pragma once

#include <assert.h>

#include "arena.h"

typedef struct List List;
typedef struct ListItem ListItem;

struct List {
    Arena *arena;
    struct {
        long size;
        int (*compare)(const void *, const void *, void *);
        void *context;
        void *(*copy)(void *, const void *, size_t);
        void (*destroy)(void *);
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

typedef enum {
    REVERSE = 1 << 1,
} ListFlags;

#define ListForEach(item, list) for (ListItem *item = (list)->begin; item; item = item->next)

#define ListForEachReverse(item, list) for (ListItem *item = (list)->end; item; item = item->prev)

#define ListForEach2(item1, item2, list1, list2)                                    \
    for (ListItem *item1 = (list1)->begin, *item2 = (list2)->begin; item1 && item2; \
         item1 = item1->next, item2 = item2->next)

#define ListForEach2Reverse(item1, item2, list1, list2)                         \
    for (ListItem *item1 = (list1)->end, *item2 = (list2)->end; item1 && item2; \
         item1 = item1->prev, item2 = item2->prev)

static List list_create(Arena *arena, long size, int (*compare)(const void *, const void *, void *),
                        void *context) {
    List list = {0};
    list.arena = arena;
    list.data.size = size;
    list.data.compare = compare;
    list.data.context = context;
    list.data.copy = memcpy;
    return list;
}

static ListItem *x__list_item_create(const List *self, void *data) {
    ListItem *item = arena_alloc(self->arena, 1, sizeof(ListItem), alignof(ListItem), 0);
    if (data && self->data.copy) {
        item->data = arena_alloc(self->arena, 1, self->data.size, alignof(void *), NOZERO);
        self->data.copy(item->data, data, self->data.size);
    } else {
        item->data = data;
    }
    return item;
}

static void list_insert(List *self, long index, void *data) {
    if (index < -self->length || self->length < index) {
        abort();
    }
    ListItem *item = x__list_item_create(self, data);
    if (self->length == 0) {
        self->begin = item;
        self->end = item;
    } else if (index == 0 || index == -self->length) {
        item->next = self->begin;
        self->begin->prev = item;
        self->begin = item;
    } else if (index == self->length) {
        item->prev = self->end;
        self->end->next = item;
        self->end = item;
    } else {
        index = (self->length + index) % self->length;
        ListItem *next = 0;
        if (index <= (self->length - 1) / 2) {
            next = self->begin;
            for (long i = 0; i < index; i++) {
                next = next->next;
            }
        } else {
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

static void *list_copy(void *_other, const void *_self, size_t) {
    List *other = _other;
    const List *self = _self;
    other->data = self->data;
    for (ListItem *item = self->begin; item; item = item->next) {
        list_append(other, item->data);
    }
    return other;
}

static List list_clone(const List *self, Arena *arena) {
    List list = {0};
    list.arena = arena;
    list_copy(&list, self, 0);
    return list;
}

static void *list_pop(List *self, long index) {
    if (index < -self->length || self->length <= index) {
        abort();
    }
    ListItem *item = 0;
    if (self->length == 1) {
        item = self->begin;
        self->begin = 0;
        self->end = 0;
    } else if (index == 0 || index == -self->length) {
        item = self->begin;
        self->begin = item->next;
        self->begin->prev = 0;
    } else if (index == self->length - 1 || index == -1) {
        item = self->end;
        self->end = item->prev;
        self->end->next = 0;
    } else {
        index = (self->length + index) % self->length;
        if (index <= (self->length - 1) / 2) {
            item = self->begin;
            for (long i = 0; i < index; i++) {
                item = item->next;
            }
        } else {
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
    for (ListItem *item = self->begin; item; item = item->next) {
        if (self->data.compare(item->data, data, self->data.context)) {
            continue;
        }
        if (self->length == 1) {
            self->begin = 0;
            self->end = 0;
        } else if (item == self->begin) {
            self->begin = item->next;
            self->begin->prev = 0;
        } else if (item == self->end) {
            self->end = item->prev;
            self->end->next = 0;
        } else {
            item->next->prev = item->prev;
            item->prev->next = item->next;
        }
        self->length -= 1;
        return item->data;
    }
    return 0;
}

static long list_index(const List *self, const void *data) {
    assert(self->data.compare);
    long index = 0;
    for (ListItem *item = self->begin; item; item = item->next) {
        if (!self->data.compare(item->data, data, self->data.context)) {
            return index;
        }
        index += 1;
    }
    return -1;
}

static void *list_find(const List *self, const void *data) {
    assert(self->data.compare);
    for (ListItem *item = self->begin; item; item = item->next) {
        if (!self->data.compare(item->data, data, self->data.context)) {
            return item->data;
        }
    }
    return 0;
}

static long list_count(const List *self, const void *data) {
    assert(self->data.compare);
    long count = 0;
    for (ListItem *item = self->begin; item; item = item->next) {
        if (!self->data.compare(item->data, data, self->data.context)) {
            count += 1;
        }
    }
    return count;
}

static ListItem *x__list_merge_sort_split(ListItem *first) {
    ListItem *slow = first, *fast = first;
    while (fast && fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    ListItem *second = slow->next;
    slow->next = 0;
    if (second) {
        second->prev = 0;
    }
    return second;
}

static ListItem *x__list_merge_sort_merge(List *self, ListItem *first, ListItem *second,
                                          int order) {
    if (!first) {
        return second;
    }
    if (!second) {
        return first;
    }
    if (order * self->data.compare(first->data, second->data, self->data.context) < 0) {
        first->next = x__list_merge_sort_merge(self, first->next, second, order);
        if (first->next) {
            first->next->prev = first;
        }
        first->prev = 0;
        return first;
    }
    second->next = x__list_merge_sort_merge(self, first, second->next, order);
    if (second->next) {
        second->next->prev = second;
    }
    second->prev = 0;
    return second;
}

static ListItem *x__list_merge_sort(List *self, ListItem *first, int order) {
    if (!first || !first->next) {
        return first;
    }
    ListItem *second = x__list_merge_sort_split(first);
    first = x__list_merge_sort(self, first, order);
    second = x__list_merge_sort(self, second, order);
    return x__list_merge_sort_merge(self, first, second, order);
}

static void list_sort(List *self, int flags) {
    assert(self->data.compare);
    self->begin = x__list_merge_sort(self, self->begin, flags & REVERSE ? -1 : 1);
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

static void list_destroy(List *self) {
    if (self->data.destroy) {
        for (ListItem *item = self->begin; item; item = item->next) {
            self->data.destroy(item->data);
        }
    }
    *self = (List){0};
}
