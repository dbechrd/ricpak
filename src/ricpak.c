#include "common.h"
#include "bit_stream.h"
#include "memory_stream.h"
#include "pak_file.h"

void test_huffman();

int main(int argc, char *argv[])
{
    //bs_test();
    //test_huffman();

    ricpak pak = { 0 };
    //ricpak_open(&pak, "test.zip");
    //ricpak_open(&pak, "pcorner/CRC_V3.ZIP");
    ricpak_open(&pak, "pcorner/ADVENTUR.ZIP");
    //ricpak_open(&pak, "pcorner/HACKSRC.ZIP");

    //// 000 001 010 000 001 110
    //char in[] = { 0b00000101, 0b00000011, 0b10000000 };
    //memory_stream ms = { 0 };
    //ms_open(&ms, in, ARRAY_COUNT(in));
    //
    //u8 out[1024] = { 0 };
    //huf_decompress_stream(out, ARRAY_COUNT(out), &ms.bs);

    puts("Done.");
    (void)getchar();
    return 0;
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

static void test_huffman()
{
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
    u16 next_code[DEFLATE_MAX_CODE_LEN] = { 0 };
    for (int bits = 1; bits < DEFLATE_MAX_CODE_LEN; bits++) {
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
            TO_BASE(tree.nodes[i].code, 2, tree.nodes[i].len),
            tree.nodes[i].code);
    }

    // TODO: Refactor cleanup if useful
    free(tree.nodes);
}