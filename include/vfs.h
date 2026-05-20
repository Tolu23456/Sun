#ifndef SUN_VFS_H
#define SUN_VFS_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char name[256];
    uint8_t *data;
    size_t size;
} VFSFile;

typedef struct {
    VFSFile *files;
    int count;
    int capacity;
} VFS;

void vfs_init(VFS *vfs);
void vfs_add(VFS *vfs, const char *name, uint8_t *data, size_t size);
uint8_t *vfs_read(VFS *vfs, const char *name, size_t *out_size);
void vfs_free(VFS *vfs);

extern VFS global_vfs;

#endif
