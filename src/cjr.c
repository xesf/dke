/*
 * cjr.c — big-animation (FLC) player. See cjr.h.
 */

#include "cjr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libsys/file_system.h"

#define CJR_MAX_FRAMES 1024

static u8  *s_data;
static i32  s_size;
static i32  s_off[CJR_MAX_FRAMES + 1];   /* frame record offsets (dword_4FB100 equivalent) */
static i32  s_n;                         /* frame count from the header (+8 / [v3+8]) */
static i32  s_records;                   /* frame records found */
static i32  s_t0;                        /* bonus_timer */
static i32  s_last;                      /* dword_4FB1CC */
static b8   s_active;

static i32 rd16(const u8 *p) { return (i16)(p[0] | (p[1] << 8)); }
static u32 rd32(const u8 *p) { return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24); }

void cjr_stop(void) {
    free(s_data);
    s_data = NULL;
    s_active = 0;
}

b8 cjr_active(void) { return s_active; }

void cjr_shift_time(i32 delta) { if (s_active) s_t0 += delta; }

i32 cjr_start(const c8 *game_path, i32 n, i32 now) {
    cjr_stop();
    c8 vpath[48], host[512];
    snprintf(vpath, sizeof(vpath), "BIGANIMS>ANIM%02d.CJR", n);
    if (!ls_resolve_path(game_path ? game_path : ".", vpath, host, (i32)sizeof(host))) return 0;
    FILE *f = ls_open(host, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 128) { ls_close(f); return 0; }
    s_data = (u8 *)malloc((size_t)sz);
    if (!s_data || fread(s_data, 1, (size_t)sz, f) != (size_t)sz) { free(s_data); s_data = NULL; ls_close(f); return 0; }
    ls_close(f);
    s_size = (i32)sz;
    /* sub_454810: magic 0xAF12 at +4 (bytes 0x12, 0xAF), frame count at +6. */
    if (s_data[4] != 0x12 || s_data[5] != 0xAF) { cjr_stop(); return 0; }
    s_n = s_data[6] | (s_data[7] << 8);
    /* Frame records follow the header from the offset at +80; each starts
     * with its size (the .OFF file only caches this walk). */
    i32 p = (i32)rd32(s_data + 80);
    s_records = 0;
    while (p + 16 <= s_size && s_records < CJR_MAX_FRAMES) {
        s_off[s_records++] = p;
        u32 fs = rd32(s_data + p);
        if (fs < 16) break;
        p += (i32)fs;
    }
    s_t0 = now;
    s_last = 0;
    s_active = 1;
    return 1;
}

/* sub_4553B0: DELTA_FLC chunk, 8-bit path. `c` points past the 6-byte chunk header. */
static void delta7(const u8 *c, const u8 *end, u8 *px, i32 w, i32 h) {
    if (c + 2 > end) return;
    i32 lines = rd16(c);
    c += 2;
    i32 y = 0;
    while (lines > 0 && c + 2 <= end) {
        i32 v = rd16(c);
        c += 2;
        i32 packets;
        if (v >= 0) packets = v;
        else {
            y += -v;
            if (c + 2 > end) return;
            packets = rd16(c);
            c += 2;
        }
        i32 x = 0;
        for (i32 k = 0; k < packets && c + 2 <= end; k++) {
            x += c[0];
            i32 cnt = (i8)c[1];
            c += 2;
            if (cnt >= 0) {
                i32 nb = 2 * cnt;
                for (i32 i = 0; i < nb && c < end; i++, x++, c++)
                    if (y >= 0 && y < h && x >= 0 && x < w) px[y * w + x] = *c;
            } else {
                if (c + 2 > end) return;
                u8 b1 = c[0], b2 = c[1];
                c += 2;
                for (i32 i = 0; i < -cnt; i++) {
                    if (y >= 0 && y < h && x >= 0 && x + 1 < w) { px[y * w + x] = b1; px[y * w + x + 1] = b2; }
                    x += 2;
                }
            }
        }
        y++;
        lines--;
    }
}

/* sub_454CB0: one frame record. Frame 0 is never applied. */
static void apply_frame(i32 idx, u8 *px, i32 w, i32 h) {
    if (idx <= 0 || idx >= s_records) return;
    const u8 *fr = s_data + s_off[idx];
    const u8 *fend = s_data + s_size;
    u32 fs = rd32(fr);
    if (fr + fs < fend) fend = fr + fs;
    i32 chunks = rd16(fr + 6);
    const u8 *c = fr + 16;
    for (i32 i = 0; i < chunks && c + 6 <= fend; i++) {
        u32 cs = rd32(c);
        u8 type = c[4];
        if (cs < 6) break;
        const u8 *cend = (c + cs < fend) ? c + cs : fend;
        if (type == 7) delta7(c + 6, cend, px, w, h);
        else if (type != 4 && type != 11 && type != 12 && type != 13 && type != 15 && type != 16 && type != 18)
            break;                       /* dispatcher default: stop */
        c += cs;
    }
}

i32 cjr_tick(i32 now, u8 *px, i32 w, i32 h) {
    if (!s_active || !px || w != 640 || h != 480) return 0;
    i32 f = (i32)((double)(now - s_t0) * 0.015);
    i32 fresh = !(f && s_last == f);
    if (f >= s_n) {
        f = s_n - 1;
        s_active = 0;                    /* bonus_mode_active = 0 after this last frame */
        fresh = 1;
    }
    if (!fresh && s_active) return 0;
    for (i32 i = s_last + 1; i <= f; i++) apply_frame(i, px, w, h);
    s_last = f;
    if (!s_active) {
        free(s_data);
        s_data = NULL;
    }
    return 1;
}
