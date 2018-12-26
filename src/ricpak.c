#include "common.h"
#include "bit_stream.h"
#include "memory_stream.h"
#include "pak.h"

void test_bit_read() {
    //file_stream stream = { 0 };
    //fs_open(&stream, "test.zip");
    char buf[] = { 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0xda, 0x06, 0x89, 0x4d, 0xe5, 0xc9 };
    memory_stream stream = { 0 };
    ms_open(&stream, buf, ARRAY_COUNT(buf));

    // 50 01010000
    // 4b 01001011
    // 03 00000011
    // 04 00000100

    for (int i = 0; i < 2; i++) {
        u8 byte = 0;
        for (int j = 0; j < 8; j++) {
            u8 bit;
            if (!bs_read_bit(&bit, &stream.bs)) {
                DIE("Failed to read bit\n");
            }
            printf("%d", bit);
            byte = (byte << 1) | bit;
        }
        printf("\n%02x\n", byte);
    }

    for (int i = 0; i < 4; i++) {
        u32 bits = 0;
        if (!bs_read_bits(&bits, &stream.bs, 4)) {
            DIE("Failed to read bits\n");
        }
        printf("%s ", TO_BASE(bits, 2, 4));
    }
    printf("\n");

    for (int i = 0; i < 12; i++) {
        u8 byte = 0;
        if (!bs_read_byte(&byte, &stream.bs)) {
            DIE("Failed to read byte\n");
        }
        printf("%02x ", byte);
    }

    printf("\n");
    //fs_close(&stream);
}


typedef enum {
    HUF_NONE     = 0,
    HUF_FIXED    = 1,
    HUF_DYNAMIC  = 2,
    HUF_RESERVED = 3,
} huf_block_type;

typedef struct {
    u8 final;
    huf_block_type type;
} huf_block_hdr;

void huf_read_block_hdr(huf_block_hdr *hdr, bit_stream *stream) {
    if (!bs_read_bit(&hdr->final, stream)) {
        DIE("Failed to read huf block header 'final' bit\n");
    }
    assert(sizeof(hdr->type) == sizeof(u32));  // Double-check enum cast
    if (!bs_read_bits((u32 *)&hdr->type, stream, 2)) {
        DIE("Failed to read huf block header 'type' bits\n");
    }
}

int huf_decompress_block(u8 *dest, u32 size, bit_stream *stream) {
    huf_block_hdr hdr;
    huf_read_block_hdr(&hdr, stream);

    u8 *dest_ptr = dest;
    switch (hdr.type) {
    case HUF_NONE:
        printf("HUF_NONE\n");
        bs_discard_bits(stream);

        // +---+---+---+---+================================+
        // |  LEN  | NLEN  |... LEN bytes of literal data...|
        // +---+---+---+---+================================+
        u8 len_high, len_low;
        u8 nlen_high, nlen_low;
        if (!bs_read_byte(&len_high, stream)) DIE("Failed to read len for uncompressed block\n");
        if (!bs_read_byte(&len_low, stream)) DIE("Failed to read len for uncompressed block\n");
        if (!bs_read_byte(&nlen_high, stream)) DIE("Failed to read len for uncompressed block\n");
        if (!bs_read_byte(&nlen_low, stream)) DIE("Failed to read len for uncompressed block\n");

        // TODO: What is nlen for? Let's assume it's an error check
        u16 len = (len_high << 8) & len_low;
        u16 nlen = (nlen_high << 8) & nlen_low;
        if (len != !nlen) DIE("Block len sanity check failed\n");

        // TODO: bs_read_bytes(dest_ptr, len)
        while (len) {
            u8 byte;
            if (!bs_read_byte(&byte, stream)) {
                DIE("Failed to read uncompressed byte\n");
            }
            *dest_ptr = byte;
            dest_ptr++;
            len--;
        }

        break;
    case HUF_FIXED:
        assert(0);  // Not yet implemented
        break;
    case HUF_DYNAMIC:
        assert(0);  // Not yet implemented
        break;
    case HUF_RESERVED:
        assert(0);  // Error
        break;
    default: assert(0);  // Unrecognized type (not possible from 2 bits)
    }
    if (hdr.final) {
        printf("Final\n");
    }

    assert(dest_ptr <= dest + size);
    return !hdr.final;
}

void huf_decompress_stream(u8 *dest, u32 size, bit_stream *stream) {
    int blocks = 0;
    do {
        blocks++;
    } while (huf_decompress_block(dest, size, stream));
    printf("Decompressed %d blocks\n", blocks);
}

void test_pak_file() {
    pak_file pak = { 0 };
    const char *filename = "test.zip";
    if (!fs_open(&pak.stream, filename)) {
        DIE("Failed to open file: %s\n", filename);
    }
    read_pak_file(&pak);

    pak_cdir_hdr *hdr = pak.cdir_hdrs;
    while (hdr) {
        printf("> cat %.*s\n", hdr->filename_len, hdr->filename);
        if (hdr->compression == PAK_CDIR_COMPRESSED_NONE) {
            assert(hdr->compressed_size == hdr->uncompressed_size);

            // Read local header
            fseek(pak.stream.hnd, hdr->local_hdr_offset, SEEK_SET);
            pak_local_hdr local_hdr = { 0 };
            fread(&local_hdr, LOCAL_HDR_SIZE, 1, pak.stream.hnd);
            fseek(pak.stream.hnd, local_hdr.filename_len, SEEK_CUR);
            fseek(pak.stream.hnd, local_hdr.extra_field_len, SEEK_CUR);

            // Seems like a useful sanity check..
            assert(hdr->uncompressed_size == local_hdr.uncompressed_size);

            // Read file contents
            u8 *out = calloc(1, local_hdr.uncompressed_size);
            u8 *out_ptr = out;
            u32 len = local_hdr.uncompressed_size;
            while (len) {
                if (!bs_read_byte(out_ptr, &pak.stream.bs)) {
                    printf("Failed to read byte\n");
                }
                out_ptr++;
                len--;
            }
            printf("%.*s\n", hdr->uncompressed_size, out);
            free(out);
        //} else if (hdr->compression == PAK_CDIR_COMPRESSED_DEFLATE) {
        //    u8 *out = calloc(1, hdr->uncompressed_size);
        //    huf_decompress_stream(out, hdr->uncompressed_size, &pak.stream.bs);
        //    printf("%.*s\n", hdr->uncompressed_size, out);
        //    free(out);
        } else {
            printf("Compressed with unsupported format: %d\n", hdr->compression);
        }
        hdr = hdr->next;
    }

    fs_close(&pak.stream);
}

typedef struct {
    u16 len;
    u16 code;
} huf_node;

#define HUF_CODES_MAX 514
typedef struct {
    u16 node_count;
    huf_node *nodes;
} huf_tree;

void test_huffman() {
    u16 input[] = { 3, 3, 3, 3, 3, 2, 4, 4 };
    huf_tree tree = { 0 };
    tree.node_count = ARRAY_COUNT(input);
    tree.nodes = calloc(tree.node_count, sizeof(huf_node));
    for (int i = 0; i < ARRAY_COUNT(input); i++) {
        tree.nodes[i].len = input[i];
    }

    // Calculate how many symbols there are for each code length
    int bl_counts[DEFLATE_MAX_CODE_LEN] = { 0 };
    for (int i = 0; i < tree.node_count; i++) {
        bl_counts[tree.nodes[i].len - 1]++;
    }

    // Calculate base code for each code length
    u16 code = 0;
    u16 next_code[ARRAY_COUNT(bl_counts)] = { 0 };
    for (int bits = 1; bits < ARRAY_COUNT(bl_counts); bits++) {
        code = (code + bl_counts[bits-1]) << 1;
        next_code[bits] = code;
    }

    // Calculate code for each symbol
    for (int i = 0; i < tree.node_count; i++) {
        u16 idx = tree.nodes[i].len - 1;
        if (idx != 0) {
            tree.nodes[i].code = next_code[idx];
            next_code[idx]++;
        }
    }

    // DEBUG: Print code table
    for (int i = 0; i < tree.node_count; i++) {
        printf("tree[%d] %c %d % 6s (%d)\n", i, 'A'+i, tree.nodes[i].len,
               TO_BASE(tree.nodes[i].code, 2, tree.nodes[i].len), tree.nodes[i].code);
    }

    // TODO: Refactor cleanup if useful
    free(tree.nodes);
}

int main(int argc, char *argv[]) {
    //test_bit_read();
    test_pak_file();
    //test_huffman();

    //// 000 001 010 000 001 110
    //char in[] = { 0b00000101, 0b00000011, 0b10000000 };
    //memory_stream ms = { 0 };
    //ms_open(&ms, in, ARRAY_COUNT(in));
    //
    //u8 out[1024] = { 0 };
    //huf_decompress_stream(out, ARRAY_COUNT(out), &ms.bs);

    getc(stdin);
    return 0;
}