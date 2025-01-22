#pragma once

#include "arena.h"
#include "dict.h"

typedef Dict Set;
typedef DictItem SetItem;

#define set_for_each(item, self) dict_for_each(item, self)

static Set set_create(Arena *arena) {
    return dict_create(arena, 0);
}

static bool set_insert(Set *self, const void *key, long size) {
    return !!dict_insert(self, key, size, (void *)1);
}

static bool set_remove(Set *self, const void *key, long size) {
    return !!dict_remove(self, key, size);
}

static bool set_find(const Set *self, const void *key, long size) {
    return !!dict_find(self, key, size);
}

static Set set_clone(const Set *self, Arena *arena) {
    return dict_clone(self, arena);
}

static SetItem *set_items(const Set *self, Arena *arena) {
    return dict_items(self, arena);
}
