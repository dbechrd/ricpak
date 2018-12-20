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
// ----------
// PAGE 28
// ----------

// DEFLATE Compressed Data Format Specification version 1.3 (1996)
// https://www.ietf.org/rfc/rfc1951.txt

// A Technique for High Ratio LZW Compression (2003)
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.3.4564&rep=rep1&type=pdf
// https://marknelson.us/posts/2011/11/08/lzw-revisited.html

// Size Adaptive Region Based Huffman Compression Technique (2014)
// https://arxiv.org/ftp/arxiv/papers/1403/1403.0153.pdf

// minizip
// https://github.com/madler/zlib/tree/master/contrib/minizip

// Coding: Huffman, Arithmetic
// Modeling: Statistical, Dictionary

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "stddef.h"

typedef unsigned int u32;
typedef unsigned short u16;

#define DIE(msg, ...) printf(msg, __VA_ARGS__); getc(stdin); exit(1)

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
    long bytes;
    pak_cdir_hdr *cdir_hdrs;
    pak_cdir_end cdir_end;
} pak_file;

#define TWICE(stmt) (stmt), (stmt)

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

#define READ(dest, hnd) fread(&(dest), sizeof((dest)), 1, hnd)

pak_file *read_pak_file(FILE *hnd) {
    pak_file *pak = calloc(1, sizeof(*pak));

    fseek(hnd, 0, SEEK_END);
    pak->bytes = ftell(hnd);

    int size = sizeof(pak->cdir_end);
    assert(size == 22);  // Ensure struct packing is disabled
    fseek(hnd, -size, SEEK_CUR);

    int found = 0;
    while (ftell(hnd) >= 0) {
        long offset = 0;
        printf("seeking eocd at: 0x%8.8x\n", ftell(hnd));
        READ(pak->cdir_end.signature, hnd);
        offset -= 4;
        if (pak->cdir_end.signature == SIG_CDIR_END) {
            fseek(hnd, -4, SEEK_CUR);
            printf("  found eocd at: 0x%8.8x\n", ftell(hnd));
            READ(pak->cdir_end, hnd);
            fseek(hnd, pak->cdir_end.disk_start_offset, SEEK_SET);

            // Read entire central directory
            char *buf = calloc(1, pak->cdir_end.size);
            fread(buf, pak->cdir_end.size, 1, hnd);

            // Extra data size, per entry
            int raw_hdr_size = offsetof(pak_cdir_hdr, filename);
            int extra_bytes = sizeof(pak_cdir_hdr) - raw_hdr_size;
            pak->cdir_hdrs = calloc(1, pak->cdir_end.size + pak->cdir_end.entries_count * extra_bytes);

            // TODO: Traverse central directory, create linked list of entries and do pointer fix-up
            pak_cdir_hdr *hdr = pak->cdir_hdrs;
            int entires_left = pak->cdir_end.entries_count;
            while (hdr) {
                memcpy(hdr, buf, raw_hdr_size);
                buf += raw_hdr_size;
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

            found = 1;
            break;
        }
        // Looking for: 0x50, 0x4b, 0x05, 0x06
        char first_byte = pak->cdir_end.signature & 0xFF;
        switch (first_byte) {
            case 0x50:
                offset -= 1;
            case 0x4b:
                offset -= 1;
                break;
            case 0x05:
                offset -= 2;
                break;
            case 0x06:
                offset -= 3;
                break;
            default:
                offset -= 4;
        }
        fseek(hnd, offset, SEEK_CUR);
    }

    if (!found) {
        DIE("Didn't find signature\n");
    }

    print_pak_cdir_end(&pak->cdir_end);
    print_pak_cdir_hdrs(pak->cdir_hdrs);
    return pak;
}

int main(int argc, char *argv[]) {
    const char *filename = "test.zip";
    FILE *hnd = fopen(filename, "rb");
    if (!hnd) {
        DIE("File not found: %s\n", filename);
    }
    pak_file *pak = read_pak_file(hnd);
    fclose(hnd);

    free(pak);
    getc(stdin);
    return 0;
}