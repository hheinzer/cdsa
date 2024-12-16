#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/dict.h"
#include "../src/hash.h"
#include "../src/hmap.h"

void stress_dict(long n, long capacity, float load_factor, DictKeyHash hash);
void stress_hmap(long n, long capacity, float load_factor, HmapKeyHash hash);
char *number(char *str, long num);

int main(void)
{
    for (long i = 1; i < 20; ++i) {
        const clock_t t0 = clock();
        stress_dict(1000000, 0, i / 20.0, strhash_fnv1a);
        const clock_t t1 = clock();
        stress_hmap(1000000, 0, i / 20.0, strhash_fnv1a);
        const clock_t t2 = clock();
        printf("%.2f: %10.4f, %10.4f, %10.4f\n", i / 20.0, (t1 - t0) / (double)CLOCKS_PER_SEC,
               (t2 - t1) / (double)CLOCKS_PER_SEC, 1 - (t2 - t1) / (double)(t1 - t0));
    }
}

void stress_dict(long n, long capacity, float load_factor, DictKeyHash hash)
{
    char key[1024];
    Dict dict = dict_create(capacity, load_factor, hash, sizeof(long), memcpy, free);
    for (long i = 0; i < n; ++i) dict_insert(&dict, number(key, i), &i);
    for (long i = 0; i < n; ++i) free(dict_remove(&dict, number(key, i)));
    assert(dict.size == 0);
    dict_clear(&dict);
}

void stress_hmap(long n, long capacity, float load_factor, HmapKeyHash hash)
{
    char key[1024];
    Hmap hmap = hmap_create(capacity, load_factor, hash, sizeof(long), memcpy, free);
    for (long i = 0; i < n; ++i) hmap_insert(&hmap, number(key, i), &i);
    for (long i = 0; i < n; ++i) free(hmap_remove(&hmap, number(key, i)));
    assert(hmap.size == 0);
    hmap_clear(&hmap);
}

char *number(char *str, long num)
{
    sprintf(str, "%ld-%ld-%ld", num, num, num);
    return str;
}
