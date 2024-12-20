#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

// print hexdump of buffer to file
static void fhexdump(FILE *file, const void *ptr, size_t size)
{
    assert(file);
    assert(ptr);
    assert(size);
    const int nbytes = 16;
    fprintf(file, "%-8s  %-*s %s\n", "Offset", 3 * nbytes, "Data", "ASCII");
    for (size_t i = 0; i < size; i += nbytes) {
        fprintf(file, "%08zx  ", i);
        for (size_t j = 0; j < nbytes; ++j) {
            if (i + j < size)
                fprintf(file, "%02x ", ((uint8_t *)ptr)[i + j]);
            else
                fprintf(file, "   ");
        }
        fprintf(file, " ");
        for (size_t j = 0; j < nbytes; ++j) {
            if (i + j >= size) break;
            const char c = ((uint8_t *)ptr)[i + j];
            fprintf(file, "%c", (isprint(c) ? c : '.'));
        }
        fprintf(file, "\n");
    }
}

// print hexdump of buffer to stdout
static void hexdump(const void *ptr, size_t size)
{
    fhexdump(stdout, ptr, size);
}
