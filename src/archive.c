#define _POSIX_C_SOURCE 200809L
#include "archive.h"
#include "sun.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define XSUN_MAGIC "XSUN03"

typedef struct {
    char path[256];
    uint32_t size;
    uint32_t original_size;
    uint32_t mode; /* Preserve file permissions */
    uint8_t  compressed;
    uint8_t  checksum[32];
} FileHeader;

static void pack_recursive(FILE *out, const char *base_path, const char *rel_path) {
    char full_path[SUN_MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_path, rel_path);

    struct stat st;
    if (stat(full_path, &st) != 0) return;

    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(full_path);
        if (!d) return;
        struct dirent *de;
        while ((de = readdir(d)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
            char next_rel[SUN_MAX_PATH];
            if (strlen(rel_path) == 0) snprintf(next_rel, sizeof(next_rel), "%s", de->d_name);
            else snprintf(next_rel, sizeof(next_rel), "%s/%s", rel_path, de->d_name);
            pack_recursive(out, base_path, next_rel);
        }
        closedir(d);
    } else if (S_ISREG(st.st_mode)) {
        FILE *in = fopen(full_path, "rb");
        if (!in) return;

        FileHeader fh;
        memset(&fh, 0, sizeof(fh));
        strncpy(fh.path, rel_path, sizeof(fh.path) - 1);
        fh.original_size = (uint32_t)st.st_size;
        fh.size = fh.original_size;
        fh.mode = (uint32_t)st.st_mode;
        fh.compressed = 0;

        fwrite(&fh, sizeof(fh), 1, out);
        char *buf = malloc(fh.size);
        fread(buf, 1, fh.size, in);
        fwrite(buf, 1, fh.size, out);
        free(buf);
        fclose(in);
        printf("  packed   %s (%u bytes)\n", rel_path, fh.size);
    }
}

int sun_pack(const char *dir_path, const char *out_path) {
    FILE *out = fopen(out_path, "wb");
    if (!out) return -1;

    fwrite(XSUN_MAGIC, 6, 1, out);
    pack_recursive(out, dir_path, "");
    fclose(out);
    return 0;
}

int sun_unpack(const char *archive_path, const char *out_dir) {
    FILE *in = fopen(archive_path, "rb");
    if (!in) return -1;

    char magic[6];
    if (fread(magic, 6, 1, in) != 1 || memcmp(magic, XSUN_MAGIC, 6) != 0) {
        fclose(in);
        return -2;
    }

    mkdir(out_dir, 0755);

    FileHeader fh;
    while (fread(&fh, sizeof(fh), 1, in) == 1) {
        char full_out[SUN_MAX_PATH];
        snprintf(full_out, sizeof(full_out), "%s/%s", out_dir, fh.path);

        /* ensure directory exists */
        char *last_slash = strrchr(full_out, '/');
        if (last_slash) {
            *last_slash = '\0';
            sun_mkdir_p(full_out);
            *last_slash = '/';
        }

        FILE *out = fopen(full_out, "wb");
        if (!out) {
            fseek(in, fh.size, SEEK_CUR);
            continue;
        }

        char *buf = malloc(fh.size);
        fread(buf, 1, fh.size, in);
        fwrite(buf, 1, fh.size, out);
        free(buf);
        fclose(out);
        chmod(full_out, (mode_t)fh.mode);
        printf("  unpacked %s (%u bytes)\n", fh.path, fh.size);
    }

    fclose(in);
    return 0;
}
