#pragma once
#include "common.h"
#include "file_stream.h"

#define COMMENT_LEN_MAX USHRT_MAX

#define MAGIC_ZIP_LOCAL_HEADER   0x04034b50
#define MAGIC_ZIP_CENTRAL_HEADER 0x02014b50
#define MAGIC_ZIP_CENTRAL_END    0x06054b50

//const char sig_cdir_hdr[]  = { 0x50, 0x4b, 0x01, 0x02 };
//const char sig_local_hdr[] = { 0x50, 0x4b, 0x03, 0x04 };
//const char sig_cdir_end[]  = { 0x50, 0x4b, 0x05, 0x06 };

typedef enum zip_flags {
    // Bit 00: encrypted file
    // Bit 01: compression option
    // Bit 02: compression option
    // Bit 03: data descriptor
    // Bit 04: enhanced deflation
    // Bit 05: compressed patched data
    // Bit 06: strong encryption
    // Bit 07-10: unused
    // Bit 11: language encoding
    // Bit 12: reserved
    // Bit 13: mask header values
    // Bit 14-15: reserved
} zip_flags;

typedef enum zip_compression_type {
    ZIP_COMPRESSION_NONE    = 0,
    ZIP_COMPRESSION_DEFLATE = 8,
} zip_compression_type;

#pragma pack(push,1)
typedef struct zip_local_header {
    // 0x04034b50
    u32 signature;

    // PKZip version needed
    u16 version;

    // General purpose bit flag:
    // Bit 00: encrypted file
    // Bit 01: compression option
    // Bit 02: compression option
    // Bit 03: data descriptor
    // Bit 04: enhanced deflation
    // Bit 05: compressed patched data
    // Bit 06: strong encryption
    // Bit 07-10: unused
    // Bit 11: language encoding
    // Bit 12: reserved
    // Bit 13: mask header values
    // Bit 14-15: reserved






    // Bit 0: If set, indicates that the file is encrypted.
    //
    // (For Method 6 - Imploding)
    // Bit 1: If the compression method used was type 6,
    // Imploding, then this bit, if set, indicates
    // an 8K sliding dictionary was used.  If clear,
    // then a 4K sliding dictionary was used.
    //
    // Bit 2: If the compression method used was type 6,
    // Imploding, then this bit, if set, indicates
    // 3 Shannon-Fano trees were used to encode the
    // sliding dictionary output.  If clear, then 2
    // Shannon-Fano trees were used.
    //
    // (For Methods 8 and 9 - Deflating)
    // Bit 2  Bit 1
    // 0      0    Normal (-en) compression option was used.
    // 0      1    Maximum (-exx/-ex) compression option was used.
    // 1      0    Fast (-ef) compression option was used.
    // 1      1    Super Fast (-es) compression option was used.
    //
    // (For Method 14 - LZMA)
    // Bit 1: If the compression method used was type 14,
    // LZMA, then this bit, if set, indicates
    // an end-of-stream (EOS) marker is used to
    // mark the end of the compressed data stream.
    // If clear, then an EOS marker is not present
    // and the compressed data size must be known
    // to extract.
    //
    // Note:  Bits 1 and 2 are undefined if the compression
    // method is any other.
    //
    // Bit 3: If this bit is set, the fields crc-32, compressed
    // size and uncompressed size are set to zero in the
    // local header.  The correct values are put in the
    // data descriptor immediately following the compressed
    // data.  (Note: PKZIP version 2.04g for DOS only
    //     recognizes this bit for method 8 compression, newer
    //     versions of PKZIP recognize this bit for any
    //     compression method.)
    //
    // Bit 4: Reserved for use with method 8, for enhanced
    // deflating.
    //
    // Bit 5: If this bit is set, this indicates that the file is
    // compressed patched data.  (Note: Requires PKZIP
    //     version 2.70 or greater)
    //
    // Bit 6: Strong encryption.  If this bit is set, you MUST
    // set the version needed to extract value to at least
    // 50 and you MUST also set bit 0.  If AES encryption
    // is used, the version needed to extract value MUST
    // be at least 51. See the section describing the Strong
    // Encryption Specification for details.  Refer to the
    // section in this document entitled "Incorporating PKWARE
    // Proprietary Technology into Your Product" for more
    // information.
    //
    // Bit 7: Currently unused.
    //
    // Bit 8: Currently unused.
    //
    // Bit 9: Currently unused.
    //
    // Bit 10: Currently unused.
    //
    // Bit 11: Language encoding flag (EFS).  If this bit is set,
    // the filename and comment fields for this file
    // MUST be encoded using UTF-8. (see APPENDIX D)
    //
    // Bit 12: Reserved by PKWARE for enhanced compression.
    //
    // Bit 13: Set when encrypting the Central Directory to indicate
    // selected data values in the Local Header are masked to
    // hide their actual values.  See the section describing
    // the Strong Encryption Specification for details.  Refer
    // to the section in this document entitled "Incorporating
    // PKWARE Proprietary Technology into Your Product" for
    // more information.
    //
    // Bit 14: Reserved by PKWARE.
    //
    // Bit 15: Reserved by PKWARE.
    u16 flags;

    // 00: no compression
    // 01: shrunk
    // 02: reduced with compression factor 1
    // 03: reduced with compression factor 2
    // 04: reduced with compression factor 3
    // 05: reduced with compression factor 4
    // 06: imploded
    // 07: reserved
    // 08: deflated
    // 09: enhanced deflated
    // 10: PKWare DCL imploded
    // 11: reserved
    // 12: compressed using BZIP2
    // 13: reserved
    // 14: LZMA
    // 15-17: reserved
    // 18: compressed using IBM TERSE
    // 19: IBM LZ77 z
    // 98: PPMd version I, Rev 1
    u16 compression;

    // File modification time (MS-DOS format):
    // Bits 00-04: seconds divided by 2
    // Bits 05-10: minute
    // Bits 11-15: hour
    u16 mod_time;

    // File modification date (MS-DOS format):
    // Bits 00-04: day
    // Bits 05-08: month
    // Bits 09-15: years from 1980
    u16 mod_date;

    // value computed over file data by CRC-32 algorithm with
    // 'magic number' 0xdebb20e3 (little endian)
    u32 crc32;

    // Compressed size
    // if archive is in ZIP64 format, this filed is 0xffffffff and the
    // length is stored in the extra field
    u32 compressed_size;

    // Uncompressed size
    // if archive is in ZIP64 format, this filed is 0xffffffff and the
    // length is stored in the extra field
    u32 uncompressed_size;

    // Variable-length of file name
    u16 filename_len;

    // Variable-length of extra field
    u16 extra_field_len;
} zip_local_header;
#pragma pack(pop)

typedef struct pak_local_header {
    zip_local_header zip;

    // Paths should use forward slashes '/'
    char *filename;

    // Used to store additional information. The field consistes of a
    // sequence of header and data pairs, where the header has a 2 byte
    // identifier and a 2 byte data size field.
    char *extra_field;

    struct pak_local_header *next;
} pak_local_header;

#pragma pack(push,1)
typedef struct zip_central_header {
    // 0x02014b50
    u32 signature;
    // Version made by
    // upper byte: OS compatibility
    // lower byte: zip specification (APPNOTE.TXT) version
    //     The value/10 indicates the major version number, and the value
    //     mod 10 is the minor version number.
    u16 version;
    // Minimum version needed to extract
    // upper/lower bytes same as above
    u16 version_needed;
    // General purpose bit flag:
    // Bit 00: encrypted file
    // Bit 01: compression option
    // Bit 02: compression option
    // Bit 03: data descriptor
    // Bit 04: enhanced deflation
    // Bit 05: compressed patched data
    // Bit 06: strong encryption
    // Bit 07-10: unused
    // Bit 11: language encoding
    // Bit 12: reserved
    // Bit 13: mask header values
    // Bit 14-15: reserved
    u16 flags;
    // 00: no compression
    // 01: shrunk
    // 02: reduced with compression factor 1
    // 03: reduced with compression factor 2
    // 04: reduced with compression factor 3
    // 05: reduced with compression factor 4
    // 06: imploded
    // 07: reserved
    // 08: deflated
    // 09: enhanced deflated
    // 10: PKWare DCL imploded
    // 11: reserved
    // 12: compressed using BZIP2
    // 13: reserved
    // 14: LZMA
    // 15-17: reserved
    // 18: compressed using IBM TERSE
    // 19: IBM LZ77 z
    // 98: PPMd version I, Rev 1
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
} zip_central_header;
#pragma pack(pop)

typedef struct pak_central_header {
    zip_central_header zip;
    char *filename;
    char *extra_field;
    char *file_comment;
    struct pak_central_header *next;
} pak_central_header;

#pragma pack(push,1)
typedef struct zip_central_end {
    // 0x05064b50
    u32 signature;
    // number of this disk containing zip_central_end
    u16 disk_end;
    // number of disk where first zip_central_header is
    u16 disk_start;
    // number of zip_central_headers on this disk
    u16 entries_disk_end;
    // total number of zip_central_headers
    u16 entries_count;
    // size of entire central directory in bytes
    u32 size;
    // offset of first zip_central_header on disk_start
    u32 disk_start_offset;
    // length of variable-length comment in bytes
    u16 comment_len;
} zip_central_end;
#pragma pack(pop)

typedef struct pak_central_end {
    zip_central_end zip;
    char *comment;
} pak_central_end;

typedef struct ricpak {
    file_stream stream;
    pak_central_header *pak_central_headers;
    pak_central_end *central_end;
} ricpak;

void ricpak_open(ricpak *pak, const char *filename);