#include "common.h"

// Copyright - Some random person on StackOverflow.. this is mostly debug code
// https://stackoverflow.com/a/34641674/770230
char *to_base(char *buf, unsigned i, int base, int pad)
{
    assert(base >= 2 && base <= 36);
    assert(pad <= TO_BASE_N);
    char *s = &buf[TO_BASE_N - 1];
    *s = '\0';
    int pad_accum = pad;
    do {
        s--;
        *s = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % base];
        i /= base;
        pad_accum--;
    } while (i);

    while (pad_accum > 0) {
        s--;
        *s = '0';
        pad_accum--;
    }

    // Could employ memmove here to move the used buffer to the beginning

    return s;
}