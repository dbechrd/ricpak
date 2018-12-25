#include "common.h"
#include "file_stream.h"
#include "pak.h"

void test_bit_read() {
    file_stream stream = { 0 };
    fs_open(&stream, "test.zip");

    // 50 01010000
    // 4b 01001011
    // 03 00000011
    // 04 00000100

    for (int i = 0; i < 2; i++) {
        u8 byte = 0;
        for (int j = 0; j < 8; j++) {
            u8 bit = fs_read_bit(&stream);
            printf("%d", bit);
            byte = (byte << 1) | bit;
        }
        printf("\n%02x\n", byte);
    }

    printf("%s ", TO_BASE(fs_read_bits(&stream, 4), 2, 4));
    printf("%s ", TO_BASE(fs_read_bits(&stream, 4), 2, 4));
    printf("%s ", TO_BASE(fs_read_bits(&stream, 4), 2, 4));
    printf("%s ", TO_BASE(fs_read_bits(&stream, 4), 2, 4));
    printf("\n");

    for (int i = 0; i < 16; i++) {
        u8 byte = fs_read_byte(&stream);
        printf("%02x ", byte);
    }

    printf("\n");
    fs_close(&stream);
}

void test_pak_file() {
    pak_file pak = { 0 };
    const char *filename = "test.zip";
    if (!fs_open(&pak.stream, filename)) {
        DIE("Failed to open file: %s\n", filename);
    }
    read_pak_file(&pak);
    fs_close(&pak.stream);
}

typedef struct huff_code {
    u16 len;
    u16 code;
} huff_code;

void test_huffman() {
    u16 input[] = { 3, 3, 3, 3, 3, 2, 4, 4 };
    huff_code tree[ARRAY_COUNT(input)] = { 0 };
    for (int i = 0; i < ARRAY_COUNT(input); i++) {
        tree[i].len = input[i];
    }

    int bl_counts[DEFLATE_MAX_CODE_LEN] = { 0 };
    for (int i = 0; i < ARRAY_COUNT(tree); i++) {
        assert(tree[i].len > 0);
        bl_counts[tree[i].len - 1]++;
    }

    // Calculate base codes for each code length
    u16 code = 0;
    u16 next_code[ARRAY_COUNT(bl_counts)] = { 0 };
    for (int bits = 1; bits < ARRAY_COUNT(bl_counts); bits++) {
        code = (code + bl_counts[bits-1]) << 1;
        next_code[bits] = code;
    }

    for (int i = 0; i < ARRAY_COUNT(tree); i++) {
        u16 idx = tree[i].len - 1;
        if (idx != 0) {
            tree[i].code = next_code[idx];
            next_code[idx]++;
        }
    }

    // DEBUG: Print code table
    for (int i = 0; i < ARRAY_COUNT(tree); i++) {
        printf("tree[%d] %c %d % 6s (%d)\n", i, 'A'+i, tree[i].len,
               TO_BASE(tree[i].code, 2, tree[i].len), tree[i].code);
    }
}

int main(int argc, char *argv[]) {
    //test_bit_read();
    //test_pak_file();
    test_huffman();

    getc(stdin);
    return 0;
}