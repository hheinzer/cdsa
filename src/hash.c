#include "hash.h"

#include <string.h>

int main(void)
{
    const char *str = "Hello, World!";

    printf("fnv1a(str) = %zu\n", memhash_fnv1a(str, strlen(str)));
    printf("fnv1a(str) = %zu\n", strhash_fnv1a(str));

    printf("fnv1a(str) = %zu\n", memhash_djb2(str, strlen(str)));
    printf("djb2(str)  = %zu\n", strhash_djb2(str));

    printf("fnv1a(str) = %zu\n", memhash_sdbm(str, strlen(str)));
    printf("sdbm(str)  = %zu\n", strhash_sdbm(str));
}
