#pragma once
#include "common.h"

typedef enum {
    BS_MEMORY,
    BS_FILE
} bit_stream_type;

typedef struct {
    bit_stream_type type;
    u32 bytes;
    u8 byte;  // last read byte
    u8 mask;  // mask for bitwise I/O
} bit_stream;

int bs_read_byte(bit_stream *stream, u8 *dst);
int bs_read_bit(bit_stream *stream, u8 *dst);
int bs_read_bits(bit_stream *stream, u32 *dst, u8 count);
void bs_discard_bits(bit_stream *stream);
void bs_test();