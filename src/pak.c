#include "pak.h"


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