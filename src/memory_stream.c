#include "memory_stream.h"

int ms_open(memory_stream *stream, u8 *buf, u32 bytes) {
    assert(buf);
    assert(bytes);
    assert(!stream->buf);
    stream->bs.type = BS_MEMORY;
    stream->buf = buf;
    stream->ptr = stream->buf;
    stream->bs.bytes = bytes;
    return 1;
}

int ms_next_byte(memory_stream *stream) {
    assert(stream->ptr);
    assert(stream->bs.bytes);
    if (stream->ptr < stream->buf + stream->bs.bytes) {
        stream->bs.byte = *stream->ptr;
        stream->ptr++;
        return 1;
    } else {
        return 0;
    }
}