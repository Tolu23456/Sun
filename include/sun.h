#ifndef SUN_H
#define SUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUN_VERSION "0.1.0"
#define SUN_MAX_ERRORS 64
#define SUN_MAX_PATH   512

typedef struct {
    char messages[SUN_MAX_ERRORS][256];
    int count;
} SunErrors;

void sun_error(SunErrors *errs, const char *fmt, ...);
char *sun_read_file(const char *path);
int   sun_write_file(const char *path, const char *content);
void  sun_mkdir_p(const char *path);

#endif
