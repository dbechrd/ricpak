#include "file_stream.h"

int fs_open(file_stream *stream, const char *filename) {
    if (stream->hnd) {
        return 1;
    }
    stream->filename = filename;
    stream->hnd = fopen(stream->filename, "rb");
    if (!stream->hnd) {
        return 0;
    }
    fseek(stream->hnd, 0, SEEK_END);
    stream->bytes = ftell(stream->hnd);
    fseek(stream->hnd, 0, SEEK_SET);
    return 1;
}

void fs_close(file_stream *stream) {
    if (stream->hnd) {
        return;
    }
    fclose(stream->hnd);
    memset(stream, 0, sizeof(*stream));
}

u8 fs_read_byte(file_stream *stream) {
    assert(stream->hnd);
    assert(stream->mask == 0);  // Enforce byte read only on byte boundaries
    if (!READ(stream->byte, stream->hnd)) {
        assert(0); // TODO: Handle eof/error
    }
    return stream->byte;
}

u8 fs_read_bit(file_stream *stream) {
    assert(stream->hnd);
    if (!stream->mask) {
        fs_read_byte(stream);
        stream->mask = 1 << (sizeof(stream->mask) * CHAR_BIT - 1);
    }
    u8 bit = (stream->byte & stream->mask) ? 1 : 0;
    stream->mask >>= 1;
    return bit;
}

u32 fs_read_bits(file_stream *stream, u8 count) {
   assert(count > 0 && count <= 32);
   u32 accum = 0;
   for (int i = 0; i < count; i++) {
       u8 bit = fs_read_bit(stream);
       accum = (accum << 1) | bit;
   }
   return accum;
}