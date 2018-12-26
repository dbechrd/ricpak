#include "bit_stream.h"
#include "memory_stream.h"
#include "file_stream.h"

extern u8 fs_next_byte(file_stream *stream);
extern u8 ms_next_byte(memory_stream *stream);

int bs_read_byte(u8 *dst, bit_stream *stream) {
    assert(stream->mask == 0);  // Enforce byte read only on byte boundaries
    switch (stream->type) {
    case BS_MEMORY:
        if (!ms_next_byte((memory_stream *)stream)) return 0;  // Out of bytes
        break;
    case BS_FILE:
        if (!fs_next_byte((file_stream *)stream)) return 0;  // Out of bytes
        break;
    default: assert(0);  // Unrecognized stream type
    }
    if (dst) {
        *dst = stream->byte;
    }
    return 1;
}

int bs_read_bit(u8 *dst, bit_stream *stream) {
    if (!stream->mask) {
        if (!bs_read_byte(dst, stream)) {
            return 0;  // Out of bytes
        }
        stream->mask = 1 << (sizeof(stream->mask) * CHAR_BIT - 1);
    }
    if (dst) {
        *dst = (stream->byte & stream->mask) ? 1 : 0;
    }
    stream->mask >>= 1;
    return 1;
}

int bs_read_bits(u32 *dst, bit_stream *stream, u8 count) {
    assert(count > 0 && count <= 32);
    u32 accum = 0;
    for (int i = 0; i < count; i++) {
        u8 bit = 0;
        if (!bs_read_bit(&bit, stream)) {
            return 0;  // Out of bits
        }
        accum = (accum << 1) | bit;
    }
    if (dst) {
        *dst = accum;
    }
    return 1;
}

void bs_discard_bits(bit_stream *stream) {
    while (stream->mask) {
        stream->mask >>= 1;
    }
}