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
    const long n = 1'000'000, capacity = n;
    for (float load_factor = 0.05; load_factor < 1.0; load_factor += 0.05) {
        stress_dict(n, capacity, load_factor, strhash_fnv1a);
        const clock_t t0 = clock();
        stress_dict(n, capacity, load_factor, strhash_fnv1a);
        const clock_t t1 = clock();

        stress_hmap(n, capacity, load_factor, strhash_fnv1a);
        const clock_t t2 = clock();
        stress_hmap(n, capacity, load_factor, strhash_fnv1a);
        const clock_t t3 = clock();

        printf("%.2f: %10.4f, %10.4f, %10.4f\n", load_factor, (t1 - t0) / (double)CLOCKS_PER_SEC,
               (t3 - t2) / (double)CLOCKS_PER_SEC, 1 - (t3 - t2) / (double)(t1 - t0));
    }
    // Compiled with 'clang -std-c23 -march=native -Ofast -flto=auto -DNDEBUG' I get:
    // 0.05:     0.5689,     0.5045,     0.1132
    // 0.10:     0.4696,     0.4192,     0.1072
    // 0.15:     0.4330,     0.3857,     0.1093
    // 0.20:     0.4172,     0.3675,     0.1190
    // 0.25:     0.4144,     0.3550,     0.1432
    // 0.30:     0.4112,     0.3434,     0.1650
    // 0.35:     0.4038,     0.3355,     0.1690
    // 0.40:     0.4010,     0.3282,     0.1816
    // 0.45:     0.4041,     0.3220,     0.2030
    // 0.50:     0.3961,     0.3098,     0.2180
    // 0.55:     0.4126,     0.3068,     0.2565
    // 0.60:     0.4144,     0.3071,     0.2588
    // 0.65:     0.4157,     0.3013,     0.2752
    // 0.70:     0.4327,     0.3068,     0.2909
    // 0.75:     0.4259,     0.2934,     0.3110
    // 0.80:     0.4247,     0.2944,     0.3067
    // 0.85:     0.4301,     0.3073,     0.2856
    // 0.90:     0.4316,     0.3237,     0.2501
    // 0.95:     0.4314,     0.3788,     0.1220
    // In fact, for varying n and capacity, hmap is consistently faster than dict, except for very
    // high or very low load factors. I get similar results if compiled with gcc.
}

void stress_dict(long n, long capacity, float load_factor, DictKeyHash hash)
{
    char key[1024];
    Dict dict = dict_create(capacity, load_factor, hash, sizeof(long), memcpy, free);
    for (long i = 0; i < n; ++i) dict_insert(&dict, number(key, i), &i);
    assert(dict.size == n);
    for (long i = 0; i < n; ++i) assert(dict_find(&dict, number(key, i)));
    for (long i = 0; i < n; ++i) free(dict_remove(&dict, number(key, i)));
    assert(dict.size == 0);
    dict_clear(&dict);
}

void stress_hmap(long n, long capacity, float load_factor, HmapKeyHash hash)
{
    char key[1024];
    Hmap hmap = hmap_create(capacity, load_factor, hash, sizeof(long), memcpy, free);
    for (long i = 0; i < n; ++i) hmap_insert(&hmap, number(key, i), &i);
    assert(hmap.size == n);
    for (long i = 0; i < n; ++i) assert(hmap_find(&hmap, number(key, i)));
    for (long i = 0; i < n; ++i) free(hmap_remove(&hmap, number(key, i)));
    assert(hmap.size == 0);
    hmap_clear(&hmap);
}

char *number(char *str, long num)
{
    sprintf(str, "%ld-%ld-%ld", num, num, num);
    return str;
}
