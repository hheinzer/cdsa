#include "dump.h"

#define countof(A) (sizeof(A) / sizeof(*(A)))
#define array_dump(A) dump(A, A + countof(A))

int main(void) {
    char s[] = "Hello, World!";
    array_dump(s);
    printf("\n");

    int a[] = {14, -15, 23, 111};
    array_dump(a);
}
