#define _POSIX_C_SOURCE 200809L
#include "sun.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

void sun_error(SunErrors *errs, const char *fmt, ...) {
    if (errs->count >= SUN_MAX_ERRORS) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(errs->messages[errs->count++], 256, fmt, ap);
    va_end(ap);
}

char *sun_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

int sun_write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fputs(content, f);
    fclose(f);
    return 0;
}

void sun_mkdir_p(const char *path) {
    char tmp[SUN_MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}
