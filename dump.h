#pragma once

#include <ctype.h>
#include <stdio.h>

static void dump(const void *begin, const void *end) {
    int offset = 2 * 8;
    printf("%-8s  %-*s %s\n", "Offset", 3 * offset, "Data", "ASCII");
    for (const char *byte = begin; byte < (char *)end; byte += offset) {
        printf("%08tx  ", byte - (char *)begin);
        for (int i = 0; i < offset; i++) {
            if (byte + i < (char *)end) {
                printf("%02x ", (unsigned char)byte[i]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (int i = 0; i < offset; i++) {
            if (byte + i < (char *)end) {
                printf("%c", isprint(byte[i]) ? byte[i] : '.');
            }
        }
        printf("\n");
    }
}
