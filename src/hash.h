#pragma once

#include <stdio.h>

// Fowler–Noll–Vo hash function
[[maybe_unused]] static size_t hash_fnv1a(const char *str)
{
    size_t hash = 0xcbf29ce484222325;
    int c;
    while ((c = *str++)) {
        hash ^= c;
        hash *= 0x00000100000001b3;
    }
    return hash;
}

// Daniel J. Bernstein hash function
[[maybe_unused]] static size_t hash_djb2(const char *str)
{
    size_t hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

// hash function of sdbm database library, also used by gawk
[[maybe_unused]] static size_t hash_sdbm(const char *str)
{
    size_t hash = 0;
    int c;
    while ((c = *str++)) hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}
