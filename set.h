#pragma once

#include "arena.h"
#include "dict.h"

typedef struct Dict Set;
typedef struct DictItem SetItem;
typedef DictForEach SetForEach;

static Set set_create(Arena *arena) {
    Set set = dict_create(arena, 0);
    set.data.copy = 0;
    return set;
}

static int set_insert(Set *self, const void *key, long size) {
    return !!dict_insert(self, key, size, (void *)1, KEEPOLD);
}

static int set_remove(Set *self, const void *key, long size) {
    return !!dict_remove(self, key, size);
}

static int set_find(const Set *self, const void *key, long size) {
    return !!dict_find(self, key, size);
}

static void set_for_each(const Set *self, SetForEach *callback, void *context) {
    dict_for_each(self, callback, context);
}

static Set set_clone(const Set *self, Arena *arena) {
    return dict_clone(self, arena);
}

static SetItem *set_items(const Set *self, Arena *arena) {
    return dict_items(self, arena);
}
