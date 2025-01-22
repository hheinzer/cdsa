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

static Set set_union(const Set *self, const Set *other, Arena *arena) {
    Set set = set_clone(self, arena);
    set_for_each(item, other) {
        set_insert(&set, item->key.data, item->key.size);
    }
    return set;
}

static Set set_intersection(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

static Set set_difference(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

static Set set_symmetric_difference(const Set *self, const Set *other, Arena *arena) {
    Set set = set_create(arena ? arena : self->arena);
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    set_for_each(item, other) {
        if (!set_find(self, item->key.data, item->key.size)) {
            set_insert(&set, item->key.data, item->key.size);
        }
    }
    return set;
}

static bool set_is_subset(const Set *self, const Set *other) {
    set_for_each(item, self) {
        if (!set_find(other, item->key.data, item->key.size)) {
            return false;
        }
    }
    return true;
}

static bool set_is_superset(const Set *self, const Set *other) {
    return set_is_subset(other, self);
}
