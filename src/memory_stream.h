#pragma once
#include "common.h"
#include "bit_stream.h"

typedef struct {
    bit_stream bs;
    u8 *buf;
    u8 *ptr;
} memory_stream;

int ms_open(memory_stream *stream, u8 *buf, u32 bytes);