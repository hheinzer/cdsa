#pragma once

#include <ctype.h>
#include <sanitizer/asan_interface.h>
#include <stdio.h>

static constexpr int x__dump_offset = 16;

static void dump(const void *begin, const void *end) {
    ASAN_UNPOISON_MEMORY_REGION(begin, (char *)end - (char *)begin);
    printf("%-8s  %-*s %s\n", "offset", 3 * x__dump_offset, "data", "ascii");
    for (const char *byte = begin; byte < (char *)end; byte += x__dump_offset) {
        printf("%08tx  ", byte - (char *)begin);
        for (int i = 0; i < x__dump_offset; i++) {
            if (byte + i < (char *)end) {
                printf("%02x ", (unsigned char)byte[i]);
            }
            else {
                printf("   ");
            }
        }
        printf(" ");
        for (int i = 0; i < x__dump_offset; i++) {
            if (byte + i < (char *)end) {
                printf("%c", isprint(byte[i]) ? byte[i] : '.');
            }
        }
        printf("\n");
    }
}
