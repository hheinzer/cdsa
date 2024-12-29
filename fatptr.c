#include "fatptr.h"

#include <stdio.h>

#include "dump.h"

typedef struct {
    int a;
    double b;
    char c[8];
} Object;

void object_print(const Object *o, uint64_t hash) {
    printf("%d %g %s (0x%0lx)\n", o->a, o->b, o->c, hash);
}

void array_print(const int *a, long size, uint64_t hash) {
    for (long i = 0; i < size / (long)sizeof(*a); i++) {
        printf("%d ", a[i]);
    }
    printf("(0x%0lx)\n", hash);
}

void string_print(const char *s, int size, uint64_t hash) {
    printf("%.*s (0x%0lx)\n", size, s, hash);
}

int main(void) {
    // shows "Use of uninitialised value of size 8" under valgrind because
    // padding bytes between entries a and b are not initialized
    Fatptr o = fatobj(Object, .b = 1.23456789, .a = 0x12345678, .c = "1234567");
    object_print(o.data, fatptr_hash_fnv1a(o));
    dump(o.data, (char *)o.data + o.size);
    printf("\n");

    Fatptr a = fatarr(int, 14, -15, 23, 111);
    array_print(a.data, a.size, fatptr_hash_djb2(a));
    dump(a.data, (char *)a.data + a.size);
    printf("\n");

    Fatptr s1 = fatstr("Hello, World!");
    string_print(s1.data, s1.size, fatptr_hash_sdbm(s1));
    dump(s1.data, (char *)s1.data + s1.size);
    printf("\n");

    Fatptr s2 = fatstr("Hello, World.");
    string_print(s2.data, s2.size, fatptr_hash_fnv1a(s2));
    string_print(s2.data, s2.size, fatptr_hash_djb2(s2));
    string_print(s2.data, s2.size, fatptr_hash_sdbm(s2));
    printf("%d\n", fatptr_equals(s1, s2));
    printf("%d\n", fatptr_compare(s1, s2));
}
