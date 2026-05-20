#include "vfs.h"
#include <stdlib.h>
#include <string.h>

VFS global_vfs;

void vfs_init(VFS *vfs) {
    vfs->count = 0;
    vfs->capacity = 16;
    vfs->files = malloc(vfs->capacity * sizeof(VFSFile));
}

void vfs_add(VFS *vfs, const char *name, uint8_t *data, size_t size) {
    if (vfs->count >= vfs->capacity) {
        vfs->capacity *= 2;
        vfs->files = realloc(vfs->files, vfs->capacity * sizeof(VFSFile));
    }
    strncpy(vfs->files[vfs->count].name, name, 255);
    vfs->files[vfs->count].data = malloc(size);
    memcpy(vfs->files[vfs->count].data, data, size);
    vfs->files[vfs->count].size = size;
    vfs->count++;
}

uint8_t *vfs_read(VFS *vfs, const char *name, size_t *out_size) {
    for (int i = 0; i < vfs->count; i++) {
        if (strcmp(vfs->files[i].name, name) == 0) {
            *out_size = vfs->files[i].size;
            return vfs->files[i].data;
        }
    }
    return NULL;
}

void vfs_free(VFS *vfs) {
    for (int i = 0; i < vfs->count; i++) {
        free(vfs->files[i].data);
    }
    free(vfs->files);
}
