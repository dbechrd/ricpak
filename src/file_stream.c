#include "file_stream.h"

int fs_open(file_stream *stream, const char *filename) {
    assert(!stream->hnd);
    stream->bs.type = BS_FILE;
    stream->filename = filename;
    stream->hnd = fopen(stream->filename, "rb");
    if (!stream->hnd) {
        return 0;
    }
    fseek(stream->hnd, 0, SEEK_END);
    stream->bs.bytes = ftell(stream->hnd);
    fseek(stream->hnd, 0, SEEK_SET);
    return 1;
}

void fs_close(file_stream *stream) {
    if (stream->hnd) {
        return;
    }
    fclose(stream->hnd);

    // DEBUG: Clear memory to force obvious errors
    memset(stream, 0, sizeof(*stream));
}

int fs_next_byte(file_stream *stream) {
    assert(stream->hnd);
    if (!READ(stream->bs.byte, stream->hnd)) {
        assert(feof(stream->hnd));  // If not EOF, something broke
        return 0;
    }
    return 1;
}