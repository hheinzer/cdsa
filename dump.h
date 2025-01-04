#pragma once

#include <ctype.h>
#include <stdio.h>

#define OFFSET (2 * 8)

static void dump(const void *begin, const void *end) {
    printf("%-8s  %-*s %s\n", "offset", 3 * OFFSET, "data", "ascii");
    for (const char *byte = begin; byte < (char *)end; byte += OFFSET) {
        printf("%08tx  ", byte - (char *)begin);
        for (int i = 0; i < OFFSET; i++) {
            if (byte + i < (char *)end) {
                printf("%02x ", (unsigned char)byte[i]);
            }
            else {
                printf("   ");
            }
        }
        printf(" ");
        for (int i = 0; i < OFFSET; i++) {
            if (byte + i < (char *)end) {
                printf("%c", isprint(byte[i]) ? byte[i] : '.');
            }
        }
        printf("\n");
    }
}
