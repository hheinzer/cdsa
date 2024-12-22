#include "dump.h"

#define countof(a) (sizeof(a) / sizeof(*(a)))

int main(void) {
    char s[] = "Hello, World!";
    dump(s, s + countof(s));
    printf("\n");

    int a[] = {14, -15, 23, 111};
    dump(a, a + countof(a));
}
