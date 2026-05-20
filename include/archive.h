#ifndef SUN_ARCHIVE_H
#define SUN_ARCHIVE_H

int sun_pack(const char *dir_path, const char *out_path);
int sun_unpack(const char *archive_path, const char *out_dir);

#endif
