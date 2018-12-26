#pragma once
#include "common.h"
#include "bit_stream.h"

typedef struct {
    bit_stream bs;
    const char *filename;
    FILE *hnd;
} file_stream;

int fs_open(file_stream *stream, const char *filename);
void fs_close(file_stream *stream);