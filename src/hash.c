#include "hash.h"

int main(void)
{
    const char *str = "Hello, World!";
    printf("fnv1a(str) = %zu\n", hash_fnv1a(str));
    printf("djb2(str)  = %zu\n", hash_djb2(str));
    printf("sdbm(str)  = %zu\n", hash_sdbm(str));
}
