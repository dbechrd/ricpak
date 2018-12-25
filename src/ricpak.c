// "Implode/Explode" PKZIP method 6
// "Deflate/Inflate" PKZIP method 8 (DEFLATE)
// "Reduce/Unreduce" ???
// "Shrink/Unshrink" Dynamic LZW

// https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
// https://github.com/kornelski/7z/blob/20e38032e62bd6bb3a176d51bce0558b16dd51e2/CPP/7zip/Archive/Zip/ZipIn.cpp#L332

// An Explanation of the Deflate Algorithm (1997)
// http://www.zlib.org/feldspar.html

// A Technique for High-Performance Data Compression (1984)
// https://www2.cs.duke.edu/courses/spring03/cps296.5/papers/welch_1984_technique_for.pdf

// The Data Compression Book 2nd Ed (1995)
// https://pdfs.semanticscholar.org/6a25/d08e1697dcd97e8f86ffdfe7cf0e23b0f7f1.pdf

// A Technique for High Ratio LZW Compression (2003)
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.3.4564&rep=rep1&type=pdf
// https://marknelson.us/posts/2011/11/08/lzw-revisited.html

// Size Adaptive Region Based Huffman Compression Technique (2014)
// https://arxiv.org/ftp/arxiv/papers/1403/1403.0153.pdf

// Fast Computation of Huffman Codes (2016)
// https://software.intel.com/en-us/articles/fast-computation-of-huffman-codes

// minizip
// https://github.com/madler/zlib/tree/master/contrib/minizip

// Coding: Huffman, Arithmetic
// Modeling: Statistical, Dictionary

//------------------------------------------------------------------------------
// DEFLATE Compressed Data Format Specification version 1.3 (1996)
// https://www.ietf.org/rfc/rfc1951.txt

// Series of arbitrary blocks
// Max size of uncompressable block is 64K
// Each block has two Huffman trees and some compressed data
// LZ77 can reference strings up to 32K backward in input stream
// Compressed data is series of elements, either: literal bytes or pointers to
// backreference <length, backward distance> (max len 258, max dist 32K)
// Tree 1: literals and lengths
// Tree 2: distances

//------------------------------------------------------------------------------

#include "common.h"
#include "file_stream.h"

#define CDIR_HDR_SIZE offsetof(pak_cdir_hdr, filename)
#define COMMENT_LEN_MAX USHRT_MAX

#define SIG_LOCAL_HDR 0x04034b50
#define SIG_CDIR_HDR  0x02014b50
#define SIG_CDIR_END  0x06054b50

//const char sig_cdir_hdr[]  = { 0x50, 0x4b, 0x01, 0x02 };
//const char sig_local_hdr[] = { 0x50, 0x4b, 0x03, 0x04 };
//const char sig_cdir_end[]  = { 0x50, 0x4b, 0x05, 0x06 };

typedef struct pak_cdir_hdr pak_cdir_hdr;
#pragma pack(push,1)
struct pak_cdir_hdr {
    u32 signature;
	u16 version;
	u16 version_needed;
	u16 flags;
	u16 compression;
	u16 mod_time;
	u16 mod_date;
    u32 crc32;
    u32 compressed_size;
    u32 uncompressed_size;
    u16 filename_len;
    u16 extra_field_len;
    u16 file_comment_len;
    u16 disk_start;
    u16 internal_attr;
    u32 external_attr;
    u32 local_hdr_offset;

    char *filename;
    char *extra_field;
    char *file_comment;
    pak_cdir_hdr *next;
};
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    u32 signature;
	u16 disk_end;
	u16 disk_start;
	u16 entries_disk_end;
	u16 entries_count;
	u32 size;
	u32 disk_start_offset;
	u16 comment_len;

    // char *comment;
} pak_cdir_end;
#pragma pack(pop)

typedef struct {
    file_stream stream;
    pak_cdir_hdr *cdir_hdrs;
    pak_cdir_end cdir_end;
} pak_file;

const char *compression_str(u16 compression) {
    switch (compression) {
        case 0: return "uncompressed";
        case 8: return "deflate";
        default: return "unknown";
    }
}

void date_str(char *buf, u16 buf_len, u16 mod_date, u16 mod_time) {
    u16 year  = (mod_date >> 9) + 1980;
    u16 month = (mod_date >> 5) & 0xF;
    u16 day   = mod_date & 0x1F;
    u16 hour  = mod_time >> 11;
    u16 min   = (mod_time >> 5) & 0x3F;
    u16 sec   = (mod_time & 0x1F) << 1;
    snprintf(buf, buf_len, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
}

void print_pak_cdir_hdrs(pak_cdir_hdr *hdr) {
    char date_buf[] = "1900-01-01 00:00:00";
    while (hdr) {
        printf("\ncentral directory file header\n");
        printf("----------------------------------------\n");
        printf(" signature         : 0x%-8.8x\n", hdr->signature);
        //printf(" version           : 0x%-8.4x (%u)\n", TWICE(hdr->version));
        //printf(" version_needed    : 0x%-8.4x (%u)\n", TWICE(hdr->version_needed));
        printf(" version           : %u\n", hdr->version);
        printf(" version_needed    : %u\n", hdr->version_needed);
        printf(" flags             : 0x%-8.4x (%u)\n", TWICE(hdr->flags));
        //printf(" compression       : 0x%-8.4x (%u)\n", TWICE(hdr->compression));
        printf(" compression       : %s\n", compression_str(hdr->compression));
        //printf(" mod_time          : 0x%-8.4x (%u)\n", TWICE(hdr->mod_time));
        //printf(" mod_date          : 0x%-8.4x (%u)\n", TWICE(hdr->mod_date));
        date_str(date_buf, sizeof(date_buf), hdr->mod_date, hdr->mod_time);
        printf(" modified          : %s\n", date_buf);
        printf(" crc32             : 0x%-8.8x (%u)\n", TWICE(hdr->crc32));
        //printf(" compressed_size   : 0x%-8.8x (%u)\n", TWICE(hdr->compressed_size));
        //printf(" uncompressed_size : 0x%-8.8x (%u)\n", TWICE(hdr->uncompressed_size));
        printf(" compressed_size   : %u bytes\n", hdr->compressed_size);
        printf(" uncompressed_size : %u bytes\n", hdr->uncompressed_size);
        printf(" compression_perc  : %.2f%%\n", (1.0f - (float)hdr->compressed_size / hdr->uncompressed_size) * 100.0f);
        //printf(" filename_len      : 0x%-8.4x (%u)\n", TWICE(hdr->filename_len));
        //printf(" extra_field_len   : 0x%-8.4x (%u)\n", TWICE(hdr->extra_field_len));
        //printf(" file_comment_len  : 0x%-8.4x (%u)\n", TWICE(hdr->file_comment_len));
        //printf(" disk_start        : 0x%-8.4x (%u)\n", TWICE(hdr->disk_start));
        printf(" disk_start        : %u\n", hdr->disk_start);
        printf(" internal_attr     : 0x%-8.4x (%u)\n", TWICE(hdr->internal_attr));
        printf(" external_attr     : 0x%-8.8x (%u)\n", TWICE(hdr->external_attr));
        //printf(" local_hdr_offset  : 0x%-8.8x (%u)\n", TWICE(hdr->local_hdr_offset));
        //printf(" local_hdr_offset  : %u\n", hdr->local_hdr_offset);
        printf(" local_hdr_offset  : %u bytes\n", hdr->local_hdr_offset);
        printf(" filename          : %.*s\n", hdr->filename_len, hdr->filename);
        printf(" extra field       : %.*s\n", hdr->extra_field_len, hdr->extra_field);
        printf(" file comment      : %.*s\n", hdr->file_comment_len, hdr->file_comment);
        hdr = hdr->next;
    }
}

void print_pak_cdir_end(pak_cdir_end *ptr) {
    printf("\nend of central directory record\n");
    printf("----------------------------------------\n");
    printf(" signature         : 0x%-8.8x\n", ptr->signature);
    printf(" disk_start        : 0x%-8.4x (%u)\n", TWICE(ptr->disk_start));
    printf(" disk_end          : 0x%-8.4x (%u)\n", TWICE(ptr->disk_end));
    printf(" entries_disk_end  : 0x%-8.4x (%u)\n", TWICE(ptr->entries_disk_end));
    printf(" entries_count     : 0x%-8.4x (%u)\n", TWICE(ptr->entries_count));
    printf(" disk_start_offset : 0x%-8.8x (%u)\n", TWICE(ptr->disk_start_offset));
    printf(" size              : 0x%-8.8x (%u)\n", TWICE(ptr->size));
    printf(" comment_length    : 0x%-8.4x (%u)\n", TWICE(ptr->comment_len));
}

void load_stuff(pak_file *pak) {
    READ(pak->cdir_end, pak->stream.hnd);
    fseek(pak->stream.hnd, pak->cdir_end.disk_start_offset, SEEK_SET);

    // Read entire central directory
    char *buf = calloc(1, pak->cdir_end.size);
    fread(buf, pak->cdir_end.size, 1, pak->stream.hnd);

    // Extra data size, per entry
    int extra_bytes = sizeof(pak_cdir_hdr) - CDIR_HDR_SIZE;
    pak->cdir_hdrs = calloc(1, pak->cdir_end.size + pak->cdir_end.entries_count * extra_bytes);

    // TODO: Traverse central directory, create linked list of entries and do pointer fix-up
    pak_cdir_hdr *hdr = pak->cdir_hdrs;
    int entires_left = pak->cdir_end.entries_count;
    while (hdr) {
        memcpy(hdr, buf, CDIR_HDR_SIZE);
        buf += CDIR_HDR_SIZE;
        assert(hdr->signature == SIG_CDIR_HDR);

        hdr->filename = (char *)hdr + sizeof(pak_cdir_hdr);
        memcpy(hdr->filename, buf, hdr->filename_len);
        buf += hdr->filename_len;

        hdr->extra_field = hdr->filename + hdr->filename_len;
        memcpy(hdr->extra_field, buf, hdr->extra_field_len);
        buf += hdr->extra_field_len;

        hdr->file_comment = hdr->extra_field + hdr->extra_field_len;
        memcpy(hdr->file_comment, buf, hdr->file_comment_len);
        buf += hdr->file_comment_len;

        if (--entires_left) {
            hdr->next = (pak_cdir_hdr *)(hdr->file_comment + hdr->file_comment_len);
        }
        hdr = hdr->next;
    }
}

int find_pak_signature(pak_file *pak) {
    const int eocd_size = sizeof(pak->cdir_end);
    assert(eocd_size == 22);  // Ensure struct packing is disabled
    fseek(pak->stream.hnd, -eocd_size, SEEK_END);

    // For large files, we can give up searching for the signature after we've
    // searched just past the maximum comment length.
    int min_hdr_offset = 0;
    if (pak->stream.bytes > (CDIR_HDR_SIZE + COMMENT_LEN_MAX)) {
        min_hdr_offset = pak->stream.bytes - (CDIR_HDR_SIZE + COMMENT_LEN_MAX);
    }

    int found = 0;
    while (ftell(pak->stream.hnd) >= min_hdr_offset) {
        long offset = 0;
        printf("seeking eocd at: 0x%8.8x\n", ftell(pak->stream.hnd));
        READ(pak->cdir_end.signature, pak->stream.hnd);
        offset -= 4;
        if (pak->cdir_end.signature == SIG_CDIR_END) {
            fseek(pak->stream.hnd, -4, SEEK_CUR);
            printf("  found eocd at: 0x%8.8x\n", ftell(pak->stream.hnd));
            found = 1;
            break;
        }
        // Looking for: 0x50, 0x4b, 0x05, 0x06
        char first_byte = pak->cdir_end.signature & 0xFF;
        switch (first_byte) {
        case SIG_CDIR_END & 0x000000FF:
            offset -= 1;
        case SIG_CDIR_END & 0x0000FF00:
            offset -= 1;
            break;
        case SIG_CDIR_END & 0x00FF0000:
            offset -= 2;
            break;
        case SIG_CDIR_END & 0xFF000000:
            offset -= 3;
            break;
        default:
            offset -= 4;
        }
        fseek(pak->stream.hnd, offset, SEEK_CUR);
    }

    return found;
}

void read_pak_file(pak_file *pak) {
    assert(pak->stream.hnd);
    if (find_pak_signature(pak)) {
        load_stuff(pak);
        fs_close(&pak->stream);

        print_pak_cdir_end(&pak->cdir_end);
        print_pak_cdir_hdrs(pak->cdir_hdrs);
    } else {
        fs_close(&pak->stream);
        DIE("Didn't find signature\n");
    }
}

#define TO_BASE_N (sizeof(unsigned)*CHAR_BIT + 1)

//                               v. compound literal .v
#define TO_BASE(x, b, pad) my_to_base((char [TO_BASE_N]){""}, (x), (b), (pad))

// Tailor the details of the conversion function as needed
// This one does not display unneeded leading zeros
// Use return value, not `buf`
char *my_to_base(char *buf, unsigned i, int base, int pad) {
    assert(base >= 2 && base <= 36);
    assert(pad <= TO_BASE_N);
    char *s = &buf[TO_BASE_N - 1];
    *s = '\0';
    int pad_accum = pad;
    do {
        s--;
        *s = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % base];
        i /= base;
        pad_accum--;
    } while (i);

    while (pad_accum > 0) {
        s--;
        *s = '0';
        pad_accum--;
    }

    // Could employ memmove here to move the used buffer to the beginning

    return s;
}

#define DEFLATE_MAX_CODE_LEN 15

typedef struct huff_code {
    u16 len;
    u16 code;
} huff_code;

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