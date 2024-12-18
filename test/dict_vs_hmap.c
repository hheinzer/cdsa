#include <stdio.h>
#include <time.h>

#include "../dict.h"
#include "../hash.h"
#include "../hmap.h"

void stress_dict(long n, long capacity, double load_factor, DictKeyHash hash);
void stress_hmap(long n, long capacity, double load_factor, HmapKeyHash hash);
char *number(char *str, long num);

/* Compiled with release flags I get:
 * - for m = 0 (maximum amount of resizing):
 *   0.05:     1.5534,     1.3444,     0.1345
 *   0.10:     2.0774,     1.7301,     0.1672
 *   0.15:     1.1148,     0.9791,     0.1218
 *   0.20:     0.9256,     0.8227,     0.1111
 *   0.25:     1.1039,     0.9411,     0.1474
 *   0.30:     0.9032,     0.7847,     0.1312
 *   0.35:     0.9258,     0.7964,     0.1398
 *   0.40:     0.9479,     0.8085,     0.1470
 *   0.45:     0.9487,     0.7885,     0.1689
 *   0.50:     0.9087,     0.7482,     0.1766
 *   0.55:     0.9908,     0.7832,     0.2096
 *   0.60:     1.0357,     0.7993,     0.2283
 *   0.65:     1.0353,     0.8250,     0.2032
 *   0.70:     1.2060,     0.8332,     0.3091
 *   0.75:     1.1596,     0.8234,     0.2899
 *   0.80:     1.3033,     0.9131,     0.2994
 *   0.85:     1.4334,     1.0385,     0.2755
 *   0.90:     1.7734,     1.4115,     0.2041
 *   0.95:     2.5671,     2.7638,    -0.0766
 * - for m = n (no resizing):
 *   0.05:     1.2129,     1.0731,     0.1153
 *   0.10:     0.9908,     0.8953,     0.0964
 *   0.15:     0.9227,     0.8381,     0.0918
 *   0.20:     0.8889,     0.8096,     0.0892
 *   0.25:     0.8741,     0.7889,     0.0974
 *   0.30:     0.8644,     0.7756,     0.1027
 *   0.35:     0.8598,     0.7659,     0.1093
 *   0.40:     0.8598,     0.7582,     0.1181
 *   0.45:     0.8589,     0.7505,     0.1262
 *   0.50:     0.8601,     0.7475,     0.1310
 *   0.55:     0.8629,     0.7398,     0.1427
 *   0.60:     0.8657,     0.7307,     0.1559
 *   0.65:     0.8720,     0.7274,     0.1658
 *   0.70:     0.8717,     0.7203,     0.1736
 *   0.75:     0.8758,     0.7174,     0.1809
 *   0.80:     0.8796,     0.7120,     0.1905
 *   0.85:     0.8841,     0.7172,     0.1888
 *   0.90:     0.8881,     0.7278,     0.1805
 *   0.95:     0.8933,     0.7742,     0.1333
 *
 * This show that hmap is consistently faster than dict, except for very high low load factors,
 * where both data structures perform badly. I get similar results if compiled with gcc.
 */
int main(void)
{
    const long n = 2'000'000, capacity = n;
    for (long i = 0; i < 19; ++i) {
        double load_factor = 0.05 + i * 0.05;

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
}

void stress_dict(long n, long capacity, double load_factor, DictKeyHash hash)
{
    char key[1024];
    Dict dict = dict_create(capacity, sizeof(long), load_factor, hash, memcpy, free);
    for (long i = 0; i < n; ++i) dict_insert(&dict, number(key, i), &i, 0);
    assert(dict.size == n);
    for (long i = 0; i < n; ++i) assert(dict_find(&dict, number(key, i)));
    for (long i = 0; i < n; ++i) free(dict_remove(&dict, number(key, i)));
    assert(dict.size == 0);
    dict_clear(&dict);
}

void stress_hmap(long n, long capacity, double load_factor, HmapKeyHash hash)
{
    char key[1024];
    Hmap hmap = hmap_create(capacity, sizeof(long), load_factor, hash, memcpy, free);
    for (long i = 0; i < n; ++i) hmap_insert(&hmap, number(key, i), &i, 0);
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
