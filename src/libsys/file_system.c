#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "file_system.h"
#include "common.h"

inline const c8* ls_get_base_path() {
#ifdef PLATFORM_VITA
    return "ux0:data/dke";             /* the game data is copied here */
#else
    return getcwd(NULL, 0);
#endif
}

inline bool ls_exists(const c8* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

inline i32 ls_get_filesize(const c8* path) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) return -1;
    return (i32)buffer.st_size;
}

inline FILE* ls_open(const c8* path, const c8* mode) {
    return fopen(path, mode);
}

inline void ls_close(FILE* file) {
    fclose(file);
}

inline void ls_seek(FILE* file, i32 offset, i32 origin) {
    fseek(file, offset, origin);
}

// read number of bytes
inline void ls_read_buffer(FILE* file, void* buffer, i32 size) {
    fread(buffer, size, 1, file);
}

c8* ls_resolve_path(const c8 *base_path, const c8 *vpath, c8 *dst, i32 dst_size) {
    if (!vpath || !dst || dst_size <= 0) return NULL;

    /* Absolute host path — pass through. */
    if (vpath[0] == '/' || vpath[0] == '\\') {
        if ((i32)strlen(vpath) >= dst_size) return NULL;
        strcpy(dst, vpath);
        return dst;
    }

    /* LIBSYS virtual path form: "<VOL>:><ROOT>><rest>".
     * In the original, game_path is "HDISK:>EARTH>" and file loads concatenate
     * it with a relative path. In the port, base_path is already the game root
     * directory on the host, so strip both the volume AND the root component. */
    const c8 *p = vpath;
    const c8 *colon = strchr(vpath, ':');
    if (colon && colon[1] == '>') {
        p = colon + 2;
        /* Skip one more '>' separated component ("EARTH>"). */
        const c8 *next = strchr(p, '>');
        if (next) p = next + 1;
    }

    /* Emit base_path + '/' + translated path (with '>' -> '/'). */
    const c8 *base = base_path ? base_path : ".";
    i32 n = snprintf(dst, dst_size, "%s/%s", base, p);
    if (n < 0 || n >= dst_size) return NULL;

    /* Translate '>' -> '/' in the appended portion only. */
    i32 base_len = (i32)strlen(base) + 1; /* + '/' */
    for (i32 i = base_len; i < n; i++) {
        if (dst[i] == '>') dst[i] = '/';
    }
    return dst;
}

inline u8 ls_read_i8(FILE* file) {
    return fgetc(file);
}

inline u16 ls_read_i16(FILE* file) {
    u16 value = 0;
    value |= fgetc(file);
    value |= fgetc(file) << 8;
    return value;
}

inline u32 ls_read_i32(FILE* file) {
    u32 value = 0;
    value |= fgetc(file);
    value |= fgetc(file) << 8;
    value |= fgetc(file) << 16;
    value |= fgetc(file) << 24;
    return value;
}

inline i8 ls_read_u8(FILE* file) {
    return (i8)ls_read_i8(file);
}

inline i16 ls_read_u16(FILE* file) {
    return (i16)ls_read_i16(file);
}

inline i32 ls_read_u32(FILE* file) {
    return (i32)ls_read_i32(file);
}
