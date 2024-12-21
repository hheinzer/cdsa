#include "hash.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *str = "Hello, World!";

    printf("fnv1a(str) = %lu\n", memhash_fnv1a(str, strlen(str)));
    printf("fnv1a(str) = %lu\n", strhash_fnv1a(str));

    printf("fnv1a(str) = %lu\n", memhash_djb2(str, strlen(str)));
    printf("djb2(str)  = %lu\n", strhash_djb2(str));

    printf("fnv1a(str) = %lu\n", memhash_sdbm(str, strlen(str)));
    printf("sdbm(str)  = %lu\n", strhash_sdbm(str));
}
