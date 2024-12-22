#pragma once

#include <ctype.h>
#include <stdio.h>

static void dump(const void *head, const void *tail) {
    int offset = 16;
    printf("%-8s  %-*s %s\n", "Offset", 3 * offset, "Data", "ASCII");
    for (const char *byte = head; byte < (char *)tail; byte += offset) {
        printf("%08tx  ", byte - (char *)head);
        for (int i = 0; i < offset; i++) {
            if (byte + i < (char *)tail) {
                printf("%02x ", (unsigned char)byte[i]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (int i = 0; i < offset; i++) {
            if (byte + i < (char *)tail) {
                printf("%c", isprint(byte[i]) ? byte[i] : '.');
            }
        }
        printf("\n");
    }
}
