#include "bit_stream.h"
#include "memory_stream.h"
#include "file_stream.h"

u8 fs_next_byte(file_stream *stream);
u8 ms_next_byte(memory_stream *stream);

int bs_read_byte(bit_stream *stream, u8 *dst)
{
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

int bs_read_bit(bit_stream *stream, u8 *dst)
{
    if (!stream->mask) {
        if (!bs_read_byte(stream, dst)) {
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

int bs_read_bits(bit_stream *stream, u32 *dst, u8 count)
{
    assert(count > 0 && count <= 32);
    u32 accum = 0;
    for (int i = 0; i < count; i++) {
        u8 bit = 0;
        if (!bs_read_bit(stream, &bit)) {
            return 0;  // Out of bits
        }
        accum = (accum << 1) | bit;
    }
    if (dst) {
        *dst = accum;
    }
    return 1;
}

void bs_discard_bits(bit_stream *stream)
{
    while (stream->mask) {
        stream->mask >>= 1;
    }
}

void bs_test()
{
    char buf[] = {
        // Test 1
        0x50, 0x4b,
        // Test 2
        0x03, 0x04,
        // Test 3
        0xe5, 0xc9
    };
    memory_stream stream = { 0 };
    ms_open(&stream, buf, ARRAY_COUNT(buf));

    // 50 01010000
    // 4b 01001011
    for (int i = 0; i < 2; i++) {
        u8 byte = 0;
        for (int j = 0; j < 8; j++) {
            u8 bit;
            assert(bs_read_bit(&stream.bs, &bit));
            byte = (byte << 1) | bit;
        }
        assert(byte == buf[i]);
    }

    // 03 00000011
    // 04 00000100
    u32 bits = 0;
    assert(bs_read_bits(&stream.bs, &bits, 4));
    assert(bits == 0x0);
    assert(bs_read_bits(&stream.bs, &bits, 4));
    assert(bits == 0x3);
    assert(bs_read_bits(&stream.bs, &bits, 4));
    assert(bits == 0x0);
    assert(bs_read_bits(&stream.bs, &bits, 4));
    assert(bits == 0x4);

    u8 byte = 0;
    assert(bs_read_byte(&stream.bs, &byte));
    assert(byte == 0xe5);
    assert(bs_read_byte(&stream.bs, &byte));
    assert(byte == 0xc9);
}