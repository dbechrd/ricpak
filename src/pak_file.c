#include "pak_file.h"

#define TWICE(stmt) (stmt), (stmt)

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

static void version_str(char *buf, u16 buf_len, u16 version)
{
    static const char *os_table[] = {
        [0] = "MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)",
        [1] = "Amiga",
        [2] = "OpenVMS",
        [3] = "UNIX",
        [4] = "VM/CMS",
        [5] = "Atari ST",
        [6] = "OS/2 H.P.F.S.",
        [7] = "Macintosh",
        [8] = "Z-System",
        [9] = "CP/M",
        [10] = "Windows NTFS",
        [11] = "MVS (OS/390 - Z/OS)",
        [12] = "VSE",
        [13] = "Acorn Risc",
        [14] = "VFAT",
        [15] = "alternate MVS",
        [16] = "BeOS",
        [17] = "Tandem",
        [18] = "OS/400",
        [19] = "OS/X (Darwin)"
    };

    u8 os_compat = version & 0xFF00;
    u8 spec_version = version & 0x00FF;
    const char *os_compat_str = "unknown";
    if (os_compat < ARRAY_COUNT(os_table)) {
        os_compat_str = os_table[os_compat];
    }
    snprintf(buf, buf_len, "%1d.%1d (%s)", spec_version / 10, spec_version % 10,
        os_compat_str);
}

static const char *compression_str(u16 compression)
{
    static const char *compression_table[] = {
        [0]  = "UNCOMPRESSED", //"The file is stored (no compression)",
        [1]  = "[LEGACY] The file is Shrunk",
        [2]  = "[LEGACY] The file is Reduced with compression factor 1",
        [3]  = "[LEGACY] The file is Reduced with compression factor 2",
        [4]  = "[LEGACY] The file is Reduced with compression factor 3",
        [5]  = "[LEGACY] The file is Reduced with compression factor 4",
        [6]  = "[LEGACY] The file is Imploded",
        [7]  = "Reserved for Tokenizing compression algorithm",
        [8]  = "DEFLATE", //"The file is Deflated",
        [9]  = "Enhanced Deflating using Deflate64(tm)",
        [10] = "PKWARE Data Compression Library Imploding (old IBM TERSE)",
        [11] = "Reserved by PKWARE",
        [12] = "File is compressed using BZIP2 algorithm",
        [13] = "Reserved by PKWARE",
        [14] = "LZMA",
        [15] = "Reserved by PKWARE",
        [16] = "IBM z/OS CMPSC Compression",
        [17] = "Reserved by PKWARE",
        [18] = "File is compressed using IBM TERSE (new)",
        [19] = "IBM LZ77 z Architecture (PFS)",
        [96] = "JPEG variant",
        [97] = "WavPack compressed data",
        [98] = "PPMd version I, Rev 1",
        [99] = "AE-x encryption marker (see APPENDIX E)"
    };

    const char *compression_str = "unknown";
    if (compression < ARRAY_COUNT(compression_table)) {
        compression_str = compression_table[compression];
    }
    return compression_str;
}

static const char *flags_str(u16 flags)
{
    return "TODO: convert flag bits to comma-delimeted list?";
}

static void date_str(char *buf, u16 buf_len, u16 mod_date, u16 mod_time)
{
    u16 year  = (mod_date >> 9) + 1980;
    u16 month = (mod_date >> 5) & 0xF;
    u16 day   = mod_date & 0x1F;
    u16 hour  = mod_time >> 11;
    u16 min   = (mod_time >> 5) & 0x3F;
    u16 sec   = (mod_time & 0x1F) << 1;
    snprintf(buf, buf_len, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
}

static void pak_central_header_print(pak_central_header *hdr)
{
#if 0
    printf("central directory file header (raw)\n");
    printf("----------------------------------------\n");
    printf(" signature         : 0x%-8.8x\n", hdr->zip.signature);
    printf(" version           : 0x%-8.4x (%u)\n", TWICE(hdr->zip.version));
    printf(" version_needed    : 0x%-8.4x (%u)\n", TWICE(hdr->zip.version_needed));
    printf(" flags             : 0x%-8.4x (%u)\n", TWICE(hdr->zip.flags));
    printf(" compression       : 0x%-8.4x (%u)\n", TWICE(hdr->zip.compression));
    printf(" mod_time          : 0x%-8.4x (%u)\n", TWICE(hdr->zip.mod_time));
    printf(" mod_date          : 0x%-8.4x (%u)\n", TWICE(hdr->zip.mod_date));
    printf(" crc32             : 0x%-8.8x (%u)\n", TWICE(hdr->zip.crc32));
    printf(" compressed_size   : 0x%-8.8x (%u)\n", TWICE(hdr->zip.compressed_size));
    printf(" uncompressed_size : 0x%-8.8x (%u)\n", TWICE(hdr->zip.uncompressed_size));
    printf(" filename_len      : 0x%-8.4x (%u)\n", TWICE(hdr->zip.filename_len));
    printf(" extra_field_len   : 0x%-8.4x (%u)\n", TWICE(hdr->zip.extra_field_len));
    printf(" file_comment_len  : 0x%-8.4x (%u)\n", TWICE(hdr->zip.file_comment_len));
    printf(" disk_start        : 0x%-8.4x (%u)\n", TWICE(hdr->zip.disk_start));
    printf(" internal_attr     : 0x%-8.4x (%u)\n", hdr->zip.internal_attr, TO_BASE(hdr->internal_attr, 2, 16));
    printf(" external_attr     : 0x%-8.8x (%u)\n", hdr->zip.external_attr, TO_BASE(hdr->internal_attr, 2, 32));
    printf(" local_hdr_offset  : 0x%-8.8x (%u)\n", TWICE(hdr->zip.local_hdr_offset));
    printf(" filename          : %.*s\n", hdr->zip.filename_len, hdr->filename);
    printf(" extra field       : %.*s\n", hdr->zip.extra_field_len, hdr->extra_field);
    printf(" file comment      : %.*s\n", hdr->zip.file_comment_len, hdr->file_comment);
    printf("\n");
#else
    char version[64] = { 0 };
    version_str(version, sizeof(version), hdr->zip.version);
    char version_needed[64] = { 0 };
    version_str(version_needed, sizeof(version_needed), hdr->zip.version_needed);
    char date_modified[] = "1900-01-01 00:00:00";
    date_str(date_modified, sizeof(date_modified), hdr->zip.mod_date, hdr->zip.mod_time);
    float compression_perc = (1.0f - (float)hdr->zip.compressed_size / hdr->zip.uncompressed_size) * 100.0f;

    printf("central directory file header\n");
    printf("----------------------------------------\n");
    printf(" signature         : 0x%-8.8x\n", hdr->zip.signature);
    printf(" version           : %s\n", version);
    printf(" version_needed    : %s\n", version_needed);
    printf(" flags             : %s\n", TO_BASE(hdr->zip.flags, 2, 16));
    printf(" compression       : %s\n", compression_str(hdr->zip.compression));
    printf(" date modified     : %s\n", date_modified);
    printf(" crc32             : 0x%08x\n", hdr->zip.crc32);
    printf(" compressed_size   : %u bytes\n", hdr->zip.compressed_size);
    printf(" uncompressed_size : %u bytes\n", hdr->zip.uncompressed_size);
    printf(" compression_perc  : %.2f%%\n", compression_perc);
    printf(" disk_start        : %u\n", hdr->zip.disk_start);
    printf(" file type         : %s\n", hdr->zip.internal_attr & 0x0001 ? "ASCII text file" : "binary data");
    printf(" file attributes   : %c%c%c%c%c%c\n",
        hdr->zip.external_attr & 0x20 ? 'A' : '-',  // Archive
        hdr->zip.external_attr & 0x10 ? 'D' : '-',  // Directory
        hdr->zip.external_attr & 0x08 ? '?' : '-',  // -- unknown --
        hdr->zip.external_attr & 0x04 ? 'S' : '-',  // System
        hdr->zip.external_attr & 0x02 ? 'H' : '-',  // Hidden
        hdr->zip.external_attr & 0x01 ? 'R' : '-'   // Read-Only
    );
    printf(" local_hdr_offset  : %u bytes\n", hdr->zip.local_hdr_offset);
    printf(" filename_len      : 0x%04x (%u)\n", TWICE(hdr->zip.filename_len));
    printf(" extra_field_len   : 0x%04x (%u)\n", TWICE(hdr->zip.extra_field_len));
    printf(" file_comment_len  : 0x%04x (%u)\n", TWICE(hdr->zip.file_comment_len));
    printf(" filename          : %.*s\n", hdr->zip.filename_len, hdr->filename);
    printf(" extra field       : ");
    for (int i = 0; i < hdr->zip.extra_field_len; ++i) {
        if (i && i % 16 == 0) {
            printf("\n                     ");
        }
        printf("%02x ", (u8)hdr->extra_field[i]);
    }
    printf("\n");
    printf(" file comment      : %.*s\n", hdr->zip.file_comment_len, hdr->file_comment);
    printf("\n");
#endif
}

static void pak_central_end_print(pak_central_end *end)
{
#if 0
    printf("end of central directory record (raw)\n");
    printf("----------------------------------------\n");
    printf(" signature         : 0x%-8.8x\n", end->zip.signature);
    printf(" disk_start        : 0x%-8.4x (%u)\n", TWICE(end->zip.disk_start));
    printf(" disk_end          : 0x%-8.4x (%u)\n", TWICE(end->zip.disk_end));
    printf(" entries_disk_end  : 0x%-8.4x (%u)\n", TWICE(end->zip.entries_disk_end));
    printf(" entries_count     : 0x%-8.4x (%u)\n", TWICE(end->zip.entries_count));
    printf(" disk_start_offset : 0x%-8.8x (%u)\n", TWICE(end->zip.disk_start_offset));
    printf(" size              : 0x%-8.8x (%u)\n", TWICE(end->zip.size));
    printf(" comment_length    : 0x%-8.4x (%u)\n", TWICE(end->zip.comment_len));
    printf("\n");
#else
    printf("end of central directory record\n");
    printf("----------------------------------------\n");
    printf(" signature         : 0x%-8.8x\n", end->zip.signature);
    printf(" disk_start        : %u\n", end->zip.disk_start);
    printf(" disk_end          : %u\n", end->zip.disk_end);
    printf(" entries_disk_end  : %u\n", end->zip.entries_disk_end);
    printf(" entries_count     : %u\n", end->zip.entries_count);
    printf(" disk_start_offset : %u\n", end->zip.disk_start_offset);
    printf(" size              : %u\n", end->zip.size);
    printf(" comment_length    : %u\n", end->zip.comment_len);
    printf(" comment           : %.*s\n", end->zip.comment_len, end->comment);
    printf("\n");
#endif
}

static int zip_central_end_locate(ricpak *pak)
{
    const int eocd_size = sizeof(zip_central_end);
    assert(eocd_size == 22);  // Ensure struct packing is disabled
    fseek(pak->stream.hnd, -eocd_size, SEEK_END);

    // For large files, we can give up searching for the signature after we've
    // searched just past the maximum comment length.
    int min_hdr_offset = 0;
    if (pak->stream.bs.bytes > (sizeof(zip_central_header) + COMMENT_LEN_MAX)) {
        min_hdr_offset = pak->stream.bs.bytes - (sizeof(zip_central_header) + COMMENT_LEN_MAX);
    }

    int found = 0;
    long file_cursor = ftell(pak->stream.hnd);
    long offset = 0;
    u32 signature = 0;

    while (file_cursor >= min_hdr_offset) {
        offset = 0;
        fread(&signature, sizeof(signature), 1, pak->stream.hnd);
        offset -= sizeof(signature);
        if (signature == MAGIC_ZIP_CENTRAL_END) {
            fseek(pak->stream.hnd, -4, SEEK_CUR);
            //printf("  found eocd at: 0x%8.8x\n", ftell(pak->stream.hnd));
            found = 1;
            break;
        }
        // Looking for: 0x06054b50
        switch (signature & 0xFF) {
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
        if (file_cursor + offset < 0) {
            break;
        }
        fseek(pak->stream.hnd, offset, SEEK_CUR);
        file_cursor = ftell(pak->stream.hnd);
    }

    return found;
}

static void zip_central_load(ricpak *pak)
{
    if (!zip_central_end_locate(pak)) {
        DIE("Didn't find signature\n");
    }

    // Read central end of directory record
    zip_central_end zip_central_end = { 0 };
    fread(&zip_central_end, sizeof(zip_central_end), 1, pak->stream.hnd);

    // Calculate size, including variable-length fields
    u32 total_size = sizeof(pak_central_end) +
        zip_central_end.comment_len;
    pak->central_end = calloc(1, total_size);
    assert(pak->central_end);

    // Read variable-length comment
    pak->central_end->zip = zip_central_end;
    pak->central_end->comment = (char *)pak->central_end + sizeof(pak_central_end);
    fread(pak->central_end->comment, pak->central_end->zip.comment_len, 1, pak->stream.hnd);

    fseek(pak->stream.hnd, pak->central_end->zip.disk_start_offset, SEEK_SET);
    assert(pak->central_end->zip.entries_count);

    // We wrap zip_central_headers inside of pak_central_headers with a few
    // extra quality-of-life fields (i.e. variable-length field pointers and an
    // intrustive linked list).
    const u32 extra_bytes = sizeof(pak_central_header) - sizeof(zip_central_header);
    u32 bytes = pak->central_end->zip.size + (extra_bytes * pak->central_end->zip.entries_count);
    pak->pak_central_headers = calloc(1, bytes);
    assert(pak->pak_central_headers);

    // Traverse central directory, create linked list of entries and do pointer fix-up
    pak_central_header *pak_hdr = pak->pak_central_headers;
    char *ptr = (char *)pak_hdr;

    for (int i = 0; i < pak->central_end->zip.entries_count; ++i) {
        fread(ptr, sizeof(pak_hdr->zip), 1, pak->stream.hnd);
        ptr += sizeof(*pak_hdr);
        assert(pak_hdr->zip.signature == MAGIC_ZIP_CENTRAL_HEADER);

        // Read variable-length filename
        if (pak_hdr->zip.filename_len) {
            fread(ptr, pak_hdr->zip.filename_len, 1, pak->stream.hnd);
            pak_hdr->filename = ptr;
            ptr += pak_hdr->zip.filename_len;
        }

        // Read variable-length extra field
        if (pak_hdr->zip.extra_field_len) {
            fread(ptr, pak_hdr->zip.extra_field_len, 1, pak->stream.hnd);
            pak_hdr->extra_field = ptr;
            ptr += pak_hdr->zip.extra_field_len;
        }

        // Read variable-length file comment
        if (pak_hdr->zip.file_comment_len) {
            fread(ptr, pak_hdr->zip.file_comment_len, 1, pak->stream.hnd);
            pak_hdr->file_comment = ptr;
            ptr += pak_hdr->zip.file_comment_len;
        }

        if (i == pak->central_end->zip.entries_count - 1)
            break;

        pak_hdr->next = (void *)ptr;
        pak_hdr = pak_hdr->next;
    }
}

static void huf_read_block_hdr(huf_block_hdr *hdr, bit_stream *stream)
{
    if (!bs_read_bit(stream, &hdr->final)) {
        DIE("Failed to read huf block header 'final' bit\n");
    }
    assert(sizeof(hdr->type) == sizeof(u32));  // Double-check enum cast
    if (!bs_read_bits(stream, (u32 *)&hdr->type, 2)) {
        DIE("Failed to read huf block header 'type' bits\n");
    }
}

static int huf_decompress_block(u8 *dest, u32 size, bit_stream *stream)
{
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
            if (!bs_read_byte(stream, &len_high))
                DIE("Failed to read len for uncompressed block\n");
            if (!bs_read_byte(stream, &len_low))
                DIE("Failed to read len for uncompressed block\n");
            if (!bs_read_byte(stream, &nlen_high))
                DIE("Failed to read len for uncompressed block\n");
            if (!bs_read_byte(stream, &nlen_low))
                DIE("Failed to read len for uncompressed block\n");

            // TODO: What is nlen for? Let's assume it's an error check
            u16 len = (len_high << 8) & len_low;
            u16 nlen = (nlen_high << 8) & nlen_low;
            if (len != !nlen) DIE("Block len sanity check failed\n");

            // TODO: bs_read_bytes(dest_ptr, len)
            while (len) {
                u8 byte;
                if (!bs_read_byte(stream, &byte)) {
                    DIE("Failed to read uncompressed byte\n");
                }
                *dest_ptr = byte;
                dest_ptr++;
                len--;
            }

            break;
        case HUF_FIXED:
            DIE("Not Yet Implemented - Fixed Huffman block\n");
            break;
        case HUF_DYNAMIC:
            DIE("Not Yet Implemented - Dynamic Huffman block\n");
            break;
        case HUF_RESERVED:
            DIE("ERROR - Reserved Huffman block\n");
            break;
        default: assert(0);  // Unrecognized type (not possible from 2 bits)
    }
    if (hdr.final) {
        printf("Final\n");
    }

    assert(dest_ptr <= dest + size);
    return !hdr.final;
}

static void huf_decompress_stream(u8 *dest, u32 size, bit_stream *stream)
{
    int blocks = 0;
    while (huf_decompress_block(dest, size, stream)) {
        blocks++;
    }
    printf("Decompressed %d blocks\n", blocks);
}

static void pak_decompress_files(ricpak *pak)
{
    pak_central_header *central_hdr = pak->pak_central_headers;
    while (central_hdr) {
        printf("%.*s:\n", central_hdr->zip.filename_len, central_hdr->filename);

        // Read local header
        fseek(pak->stream.hnd, central_hdr->zip.local_hdr_offset, SEEK_SET);
        zip_local_header zip_local_hdr = { 0 };
        fread(&zip_local_hdr, sizeof(zip_local_header), 1, pak->stream.hnd);

        // Calculate size, including variable-length fields
        u32 total_size = sizeof(pak_local_header) +
            zip_local_hdr.filename_len + zip_local_hdr.extra_field_len;
        pak_local_header *pak_hdr = calloc(1, total_size);
        assert(pak_hdr);

        pak_hdr->zip = zip_local_hdr;
        pak_hdr->filename = (char *)pak_hdr + sizeof(pak_local_header);
        pak_hdr->extra_field = (char *)pak_hdr + sizeof(pak_local_header) + pak_hdr->zip.filename_len;
        fread(pak_hdr->filename, pak_hdr->zip.filename_len, 1, pak->stream.hnd);
        fread(pak_hdr->extra_field, pak_hdr->zip.extra_field_len, 1, pak->stream.hnd);

        // TODO: Check flags bit 3 to determine if CRC is in a post-buffer data descriptor
        // TODO: Actually calculate the checksum of the file data
        assert(pak_hdr->zip.crc32);

        // DEBUG: Random local vs. cdir header sanity check
        // NOTE: This will fail if flag 3 is set (data descriptors)
        assert(central_hdr->zip.crc32 == pak_hdr->zip.crc32);

        if (central_hdr->zip.compression == ZIP_COMPRESSION_NONE) {
            assert(central_hdr->zip.compressed_size == central_hdr->zip.uncompressed_size);

            // Read file contents
            u8 *out = calloc(1, pak_hdr->zip.uncompressed_size);
            u8 *out_ptr = out;
            u32 len = pak_hdr->zip.uncompressed_size;
            while (len) {
                if (!bs_read_byte(&pak->stream.bs, out_ptr)) {
                    printf("Failed to read byte\n");
                }
                out_ptr++;
                len--;
            }
            printf("%.*s\n", central_hdr->zip.uncompressed_size, out);
            free(out);
        } else if (central_hdr->zip.compression == ZIP_COMPRESSION_DEFLATE) {
            assert(central_hdr->zip.compressed_size);
            assert(central_hdr->zip.uncompressed_size);
#if 1
            u8 *out = calloc(1, central_hdr->zip.uncompressed_size);
            huf_decompress_stream(out, central_hdr->zip.uncompressed_size, &pak->stream.bs);
            printf("%.*s\n", central_hdr->zip.uncompressed_size, out);
            free(out);
#else
            // Read file contents
            u8 byte = 0;
            for (u32 i = 0; i < pak_hdr->zip.compressed_size; ++i) {
                if (i && (i % 8 == 0)) {
                    printf("\n");
                }
                if (!bs_read_byte(&pak->stream.bs, &byte)) {
                    printf("Failed to read byte\n");
                }
                printf("%s ", TO_BASE(byte, 2, 8));
                //printf("%c ", byte);
            }
            printf("\n");
#endif
        } else {
            printf("Unsupported compression format: %s\n", compression_str(central_hdr->zip.compression));
        }
        central_hdr = central_hdr->next;
    }
}

void ricpak_open(ricpak *pak, const char *filename)
{
    if (!fs_open(&pak->stream, filename)) {
        DIE("Failed to open file: %s\n", filename);
    }
    assert(pak->stream.hnd);

    zip_central_load(pak);
    pak_central_header *hdr = pak->pak_central_headers;
    while (hdr) {
        // General purpose flag 3 (0x4) has a lot of side effects. I'm curious
        // if it's set in any of the .zip files we test.
        assert((hdr->zip.flags & 0x4) == 0);
        pak_central_header_print(hdr);
        hdr = hdr->next;
    }
    pak_central_end_print(pak->central_end);
    pak_decompress_files(pak);

    fs_close(&pak->stream);
}