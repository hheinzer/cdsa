#pragma once

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void *data;
    long size;
} Fatptr;

#define fatobj(t, ...) fatptr(&(t){__VA_ARGS__}, 1, sizeof(t))

#define countof(a) (sizeof(a) / sizeof(*(a)))
#define fatarr(t, ...) fatptr((t[]){__VA_ARGS__}, countof(((t[]){__VA_ARGS__})), sizeof(t))

#define lengthof(s) (countof(s) - 1)
#define fatstr(s) fatptr(s, lengthof(s), 1)

static Fatptr fatptr(void *data, long count, long size) {
    if (count > LONG_MAX / size) {
        abort();  // overflow
    }
    return (Fatptr){data, count * size};
}

static int fatptr_equals(Fatptr a, Fatptr b) {
    return a.size == b.size && !memcmp(a.data, b.data, a.size);
}

static int fatptr_compare(Fatptr a, Fatptr b) {
    if (a.size == b.size) {
        return memcmp(a.data, b.data, a.size);
    }
    return (a.size > b.size) - (a.size < b.size);
}

static uint64_t fatptr_hash_fnv1a(Fatptr a) {
    uint64_t hash = 0xcbf29ce484222325;
    for (char *byte = a.data; byte < (char *)a.data + a.size; ++byte) {
        hash ^= *byte;
        hash *= 0x00000100000001b3;
    }
    return hash;
}

static uint64_t fatptr_hash_djb2(Fatptr a) {
    uint64_t hash = 5381;
    for (char *byte = a.data; byte < (char *)a.data + a.size; ++byte) {
        hash = ((hash << 5) + hash) + *byte;
    }
    return hash;
}

static uint64_t fatptr_hash_sdbm(Fatptr a) {
    uint64_t hash = 0;
    for (char *byte = a.data; byte < (char *)a.data + a.size; ++byte) {
        hash = *byte + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
