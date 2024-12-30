#include "dump.h"

#define countof(a) (sizeof(a) / sizeof(*(a)))
#define dump_array(a) dump(a, a + countof(a))

int main(void) {
    char s[] = "Hello, World!";
    dump_array(s);
    printf("\n");

    int a[] = {14, -15, 23, 111};
    dump_array(a);
}
