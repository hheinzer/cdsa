#include "trie.h"

#include <stdio.h>

#include "dump.h"

int main(void) {
    Arena arena = arena_create(1 << 20);

    Trie a = trie_create(&arena, sizeof(int));

    const char *key[] = {
        "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    };
    for (long i = 0; i < 10; ++i) {
        trie_insert(&a, key[i], 0, &i, 0);
    }

    Trie b = trie_clone(&a, &arena);

    trie_remove(&b, "six", 0);
    trie_insert(&b, "ten", 0, (int[]){10}, 0);

    dump(arena.data, arena.begin);
    printf("\n");

    printf("a = {");
    trie_for_each(item, &a) printf("%s: %d, ", item->key.data, *(int *)item->data);
    printf("}\n");

    printf("b = {");
    trie_for_each(item, &b) printf("%s: %d, ", item->key.data, *(int *)item->data);
    printf("}\n");

    printf("a.find(six) = %p\n", trie_find(&a, "six", 0));
    printf("b.find(six) = %p\n", trie_find(&b, "six", 0));

    printf("a.find(ten) = %p\n", trie_find(&a, "ten", 0));
    printf("b.find(ten) = %p\n", trie_find(&b, "ten", 0));

    trie_destroy(&a);
    trie_destroy(&b);

    arena_destroy(&arena);
}
