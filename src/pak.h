#pragma once
#include "common.h"
#include "file_stream.h"

#define COMMENT_LEN_MAX USHRT_MAX

#define SIG_LOCAL_HDR 0x04034b50
#define SIG_CDIR_HDR  0x02014b50
#define SIG_CDIR_END  0x06054b50

//const char sig_cdir_hdr[]  = { 0x50, 0x4b, 0x01, 0x02 };
//const char sig_local_hdr[] = { 0x50, 0x4b, 0x03, 0x04 };
//const char sig_cdir_end[]  = { 0x50, 0x4b, 0x05, 0x06 };

typedef enum {
    PAK_CDIR_COMPRESSED_NONE    = 0,
    PAK_CDIR_COMPRESSED_DEFLATE = 8,
} pak_cdir_hdr_compress_type;

#define LOCAL_HDR_SIZE offsetof(pak_local_hdr, filename)

typedef struct pak_local_hdr pak_local_hdr;
#pragma pack(push,1)
struct pak_local_hdr {
    u32 signature;
    u16 version;
    u16 flags;
    u16 compression;
    u16 mod_time;
    u16 mod_date;
    u32 crc32;
    u32 compressed_size;
    u32 uncompressed_size;
    u16 filename_len;
    u16 extra_field_len;

    char *filename;
    char *extra_field;
    pak_local_hdr *next;
};
#pragma pack(pop)

#define CDIR_HDR_SIZE offsetof(pak_cdir_hdr, filename)

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

void read_pak_file(pak_file *pak);