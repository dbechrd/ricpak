#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "stddef.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define ARRAY_COUNT(a) (sizeof(a)/sizeof(a[0]))
#define DIE(msg, ...) printf(msg, __VA_ARGS__); (void)getchar(); exit(1)

#define TO_BASE_N (sizeof(unsigned)*CHAR_BIT + 1)
#define TO_BASE(x, b, pad) to_base((char [TO_BASE_N]){""}, (x), (b), (pad))
#define DEFLATE_MAX_CODE_LEN 15

char *to_base(char *buf, unsigned i, int base, int pad);