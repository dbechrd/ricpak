#pragma once
#include "common.h"

typedef enum {
    BS_MEMORY = 0,
    BS_FILE   = 1,
} bit_stream_type;

typedef struct {
    u32 bytes;
    u8 byte;  // last read byte
    u8 mask;  // mask for bitwise I/O
    bit_stream_type type;
} bit_stream;

int bs_read_byte(u8 *dst, bit_stream *stream);
int bs_read_bit(u8 *dst, bit_stream *stream);
int bs_read_bits(u32 *dst, bit_stream *stream, u8 count);
void bs_discard_bits(bit_stream *stream);