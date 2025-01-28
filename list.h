/// @file
#pragma once

#include <assert.h>
#include <stddef.h>

#include "arena.h"

typedef struct List List;
typedef struct ListItem ListItem;

typedef int ListDataCompare(const void *, const void *, void *);  ///< Data comparison function
typedef void *ListDataCopy(Arena *, void *, const void *, long);  ///< Data copy function

/**
 * @brief Represents a doubly linked list
 */
struct List {
    Arena *arena;  ///< Pointer to an arena allocator
    struct {
        long size;                 ///< Size of the item data in bytes
        ListDataCompare *compare;  ///< Pointer to a data comparison function
        ListDataCopy *copy;        ///< Pointer to a data copy function
    } data;                        ///< Data properties
    long length;                   ///< Number of items in the list
    ListItem *begin;               ///< Pointer to first item of the list
    ListItem *end;                 ///< Pointer to last item of the list
};

/**
 * @brief Represents a single item of a list
 */
struct ListItem {
    void *data;      ///< Pointer to the item data
    ListItem *next;  ///< Pointer to the next item
    ListItem *prev;  ///< Pointer to the previous item
};

/**
 * @brief Iterate over all items of a list
 * @param item Current list item
 * @param self Pointer to a list
 */
#define list_for_each(item, self) for (auto(item) = (self)->begin; item; (item) = (item)->next)

/**
 * @brief Create a new list
 * @param arena Pointer to an arena allocator
 * @param size Size of item data in bytes (optional)
 * @param compare Pointer to a data comparison function (optional)
 * @return New list instance
 * @note If `size == 0`, the data pointers will be directly assigned rather than copied
 */
static List list_create(Arena *arena, long size, ListDataCompare *compare) {
    List list = {};
    list.arena = arena;
    list.data.size = size;
    list.data.compare = compare;
    list.data.copy = arena_memcpy;
    return list;
}

/// @private
static void x__list_item_init(const List *self, ListItem *item, void *data) {
    if (data && self->data.size) {
        item->data = arena_malloc(self->arena, 1, self->data.size, alignof(max_align_t));
        self->data.copy(self->arena, item->data, data, self->data.size);
    }
    else {
        item->data = data;
    }
}

/**
 * @brief Insert a new item into a list
 * @param self Pointer to a list
 * @param index Index of the new item
 * @param data Pointer to the item data
 * @note A negative `index` is interpreted as from the back (`self->length - index`)
 */
static void list_insert(List *self, long index, void *data) {
    assert(-self->length <= index && index <= self->length);
    ListItem *item = arena_calloc(self->arena, 1, sizeof(ListItem), alignof(ListItem));
    x__list_item_init(self, item, data);
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
        auto next = self->begin;
        if (index <= (self->length - 1) / 2) {
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

/**
 * @brief Append a new item to the back of a list
 * @param self Pointer to a list
 * @param data Pointer to the item data
 */
static void list_append(List *self, void *data) {
    list_insert(self, self->length, data);
}

/**
 * @brief Remove an item from a list
 * @param self Pointer to a list
 * @param index Index of the item to remove
 * @return Pointer to the item data, or `nullptr` if the list is empty
 * @note A negative `index` is interpreted as from the back (`self->length - index`)
 */
static void *list_pop(List *self, long index) {
    if (self->length == 0) {
        return nullptr;
    }
    assert(-self->length <= index && index < self->length);
    auto item = self->begin;
    if (self->length == 1) {
        self->begin = nullptr;
        self->end = nullptr;
    }
    else if (index == 0 || index == -self->length) {
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

/**
 * @brief Remove the first occurrence of a matching item from a list
 * @param self Pointer to a list
 * @param data Pointer to the item data to match
 * @return Pointer to the item data, or `nullptr` if no matching item is found
 * @note This function requires a data comparison function to be set
 */
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

/**
 * @brief Retrieve an item from a list
 * @param self Pointer to a list
 * @param index Index of the item to retrieve
 * @return Pointer to the item data, or `nullptr` if the list is empty
 * @note A negative `index` is interpreted as from the back (`self->length - index`)
 */
static void *list_get(const List *self, long index) {
    if (self->length == 0) {
        return nullptr;
    }
    assert(-self->length <= index && index < self->length);
    auto item = self->begin;
    index = (self->length + index) % self->length;
    if (index <= (self->length - 1) / 2) {
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

/**
 * @brief Find the first occurrence of a matching item of a list
 * @param self Pointer to a list
 * @param data Pointer to the item data to match
 * @return Pointer to the item data, or `nullptr` if no matching item is found
 * @note This function requires a data comparison function to be set
 */
static void *list_find(const List *self, const void *data) {
    assert(self->data.compare);
    list_for_each(item, self) {
        if (!self->data.compare(item->data, data, nullptr)) {
            return item->data;
        }
    }
    return nullptr;
}

/**
 * @brief Retrieve the index of the first occurrence of a matching item of a list
 * @param self Pointer to a list
 * @param data Pointer to the item data to match
 * @return Index of the matching item, or `-1` if no matching item is found
 * @note This function requires a data comparison function to be set
 */
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

/**
 * @brief Count the number of matching items of a list
 * @param self Pointer to a list
 * @param data Pointer to the item data to match
 * @return Number of matching items
 * @note This function requires a data comparison function to be set
 */
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

/// @private
static ListItem *x__list_merge_sort_split(ListItem *first) {
    auto slow = first;
    auto fast = first;
    while (fast && fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    auto second = slow->next;
    slow->next = nullptr;
    if (second) {
        second->prev = nullptr;
    }
    return second;
}

/// @private
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

/// @private
static ListItem *x__list_merge_sort(List *self, ListItem *first, void *context) {
    if (!first || !first->next) {
        return first;
    }
    auto second = x__list_merge_sort_split(first);
    first = x__list_merge_sort(self, first, context);
    second = x__list_merge_sort(self, second, context);
    return x__list_merge_sort_merge(self, first, second, context);
}

/**
 * @brief Sort the items of a list
 * @param self Pointer to a list
 * @param context Pointer to a user-provided context for the comparison function (optional)
 * @note This function requires a data comparison function to be set
 */
static void list_sort(List *self, void *context) {
    assert(self->data.compare);
    self->begin = x__list_merge_sort(self, self->begin, context);
    auto item = self->begin;
    while (item && item->next) {
        item = item->next;
    }
    self->end = item;
}

/**
 * @brief Reverse the order of items in a list
 * @param self Pointer to a list
 */
static void list_reverse(List *self) {
    for (auto item = self->begin; item; item = item->prev) {
        auto swap = item->next;
        item->next = item->prev;
        item->prev = swap;
    }
    auto swap = self->begin;
    self->begin = self->end;
    self->end = swap;
}

/**
 * @brief Create a clone of a list
 * @param self Pointer to a list
 * @param arena Pointer to an arena allocator (optional)
 * @return Cloned list instance
 * @note If no arena allocator is passed, the arena allocator of the list is used
 */
static List list_clone(const List *self, Arena *arena) {
    List list = {};
    list.arena = arena ? arena : self->arena;
    list.data = self->data;
    list_for_each(item, self) {
        list_append(&list, item->data);
    }
    return list;
}

/**
 * @brief Retrieve an array of list items
 * @param self Pointer to a list
 * @param arena Pointer to an arena allocator (optional)
 * @return Pointer to an array of items
 * @note If no arena allocator is passed, the arena allocator of the list is used
 */
static ListItem *list_items(const List *self, Arena *arena) {
    arena = arena ? arena : self->arena;
    ListItem *items = arena_malloc(arena, self->length, sizeof(ListItem), alignof(ListItem));
    long index = 0;
    list_for_each(item, self) {
        items[index++] = *item;
    }
    return items;
}
