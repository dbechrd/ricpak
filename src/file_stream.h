#pragma once
#include "common.h"

typedef struct {
    const char *filename;
    FILE *hnd;
    int bytes;
    u8 byte;
    u8 mask; // accumulator for bit reads
} file_stream;

int fs_open(file_stream *stream, const char *filename);
void fs_close(file_stream *stream);
u8 fs_read_byte(file_stream *stream);
u8 fs_read_bit(file_stream *stream);
u32 fs_read_bits(file_stream *stream, u8 count);