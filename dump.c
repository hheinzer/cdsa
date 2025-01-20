#include "dump.h"

#define countof(A) ((long)(sizeof(A) / sizeof(*(A))))
#define array_dump(A) dump(A, (A) + countof(A))

int main(void) {
    char string[] = "Hello, World!";
    array_dump(string);
    printf("\n");

    int array[] = {14, -15, 23, 111};
    array_dump(array);
}
