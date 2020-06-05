#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "types.h"
#include "filepath.h"

#include "ConvertUTF.h"

#ifdef _WIN32
#include <wchar.h>
#endif

void os_strcpy(oschar_t *dst, const char *src) {
#ifdef _WIN32
    if (src == NULL) return;

    const UTF8 *sourceStart = (const UTF8 *)src;
    UTF16 *targetStart = (UTF16 *)dst;
    uint32_t src_len, dst_len;
    src_len = strlen(src);
    dst_len = src_len + 1;
    const UTF8 *sourceEnd = (const UTF8 *)(src + src_len);
    UTF16 *targetEnd = (UTF16 *)(dst + dst_len);

    if (ConvertUTF8toUTF16(&sourceStart, sourceEnd, &targetStart, targetEnd, 0) != conversionOK) {
        fprintf(stderr, "Failed to convert %s to UTF-16!\n", src);
        exit(EXIT_FAILURE);
    }
#else
    strcpy(dst, src);
#endif
}

int os_makedir(const oschar_t *dir) {
#ifdef _WIN32
    return _wmkdir(dir);
#else
    return mkdir(dir, 0777);
#endif
}

int os_rmdir(const oschar_t *dir) {
#ifdef _WIN32
    return _wrmdir(dir);
#else
    return remove(dir);
#endif
}

static void filepath_update(filepath_t *fpath) {
    memset(fpath->os_path, 0, MAX_PATH * sizeof(oschar_t));
    os_strcpy(fpath->os_path, fpath->char_path);
}

void filepath_init(filepath_t *fpath) {
    fpath->valid = VALIDITY_INVALID;
}

void filepath_copy(filepath_t *fpath, filepath_t *copy) {
    if (copy != NULL && copy->valid == VALIDITY_VALID)
        memcpy(fpath, copy, sizeof(filepath_t));
    else
        memset(fpath, 0, sizeof(filepath_t));
}

void filepath_append(filepath_t *fpath, const char *format, ...) {
    char tmppath[MAX_PATH];
    va_list args;

    if (fpath->valid == VALIDITY_INVALID)
        return;

    memset(tmppath, 0, MAX_PATH);

    va_start(args, format);
    vsnprintf(tmppath, sizeof(tmppath), format, args);
    va_end(args);

    strcat(fpath->char_path, OS_PATH_SEPARATOR);
    strcat(fpath->char_path, tmppath);
    filepath_update(fpath);
}

void filepath_append_n(filepath_t *fpath, uint32_t n, const char *format, ...) {
    char tmppath[MAX_PATH];
    va_list args;

    if (fpath->valid == VALIDITY_INVALID || n > MAX_PATH)
        return;

    memset(tmppath, 0, MAX_PATH);

    va_start(args, format);
    vsnprintf(tmppath, sizeof(tmppath), format, args);
    va_end(args);

    strcat(fpath->char_path, OS_PATH_SEPARATOR);
    strncat(fpath->char_path, tmppath, n);
    filepath_update(fpath);
}

void filepath_set(filepath_t *fpath, const char *path) {
    if (strlen(path) < MAX_PATH) {
        fpath->valid = VALIDITY_VALID;
        memset(fpath->char_path, 0, MAX_PATH);
        strncpy(fpath->char_path, path, MAX_PATH - 1);
        filepath_update(fpath);
    } else {
        fpath->valid = VALIDITY_INVALID;
    }
}

void filepath_set_format(filepath_t *fpath, const char *format, ...) {
    va_list args;

    if (fpath->valid == VALIDITY_INVALID)
        return;

    memset(fpath->char_path, 0, MAX_PATH);

    va_start(args, format);
    vsnprintf(fpath->char_path, MAX_PATH, format, args);
    va_end(args);

    filepath_update(fpath);
}

oschar_t *filepath_get(filepath_t *fpath) {
    if (fpath->valid == VALIDITY_INVALID)
        return NULL;
    else
        return fpath->os_path;
}
