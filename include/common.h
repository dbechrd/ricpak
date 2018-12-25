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
#define DIE(msg, ...) printf(msg, __VA_ARGS__); getc(stdin); exit(1)
#define TWICE(stmt) (stmt), (stmt)
#define READ(dest, hnd) fread(&(dest), sizeof((dest)), 1, hnd)