/*
 * envfx.c — environment tint and flash palette effects. See envfx.h.
 */

#include "envfx.h"
#include "gamma.h"

#include <stdlib.h>
#include <string.h>

#include "libsys/graphics/shading.h"   /* cosine_table (= sine_table, see there) */

static envfx_config_t s_cfg;
static i32 s_cam;
static i32 s_now;

/* Flash state: dword_4B7B28 (level), 4B7B2C (end time), 4B7B30 (start time),
 * 509228 (target), 5099D0 (start level). */
static i32 s_flash_on;
static i32 s_level, s_t_end, s_t_start, s_target, s_from;
static i32 s_frz, s_fgz, s_fbz;         /* 5099C8, 5099CC, 509258 */
static i32 s_freq, s_rand;              /* 5099C4, 509A38 */
static i32 s_tint[3];                   /* added to 6-bit components */

void envfx_configure(const envfx_config_t *cfg) {
    if (cfg) s_cfg = *cfg; else memset(&s_cfg, 0, sizeof(s_cfg));
    envfx_set_camera(0);
}

void envfx_set_camera(i32 camera) {
    if (camera < 0 || camera >= ENVFX_MAX_CAMERAS) camera = 0;
    s_cam = camera;

    for (i32 i = 0; i < 3; i++) s_tint[i] = 2 * s_cfg.env[camera][i];

    /* 0x46BE40 can_do_32k */
    s_level = s_t_end = s_t_start = 0;
    s_frz = s_cfg.flash_rgb[camera][0];
    s_fgz = s_cfg.flash_rgb[camera][1];
    s_fbz = s_cfg.flash_rgb[camera][2];
    s_freq = s_cfg.flash_freq[camera];
    s_rand = s_cfg.flash_rand[camera];
    s_flash_on = s_cfg.flash_present && (s_frz || s_fgz || s_fbz);
}

static i32 rnd(i32 n) { return n > 0 ? rand() % n : 0; }

/* 0x42F565 timing part: pick a new target level when the ramp ends. */
void envfx_tick(i32 now) {
    s_now = now;
    if (!s_flash_on) return;
    i32 v14 = (s_freq << 6) / 255;
    i32 v7 = (abs(s_rand) << 6) / 255;
    if (s_t_end <= now) {
        if (v7 <= 0) s_target = s_target ? 0 : 6;
        else         s_target = rnd(6);
        s_from = s_level;
        i32 v3 = v7 + rnd(v14);
        s_t_end = now + abs(s_target - s_from) * v3;
        s_t_start = now;
        if (s_t_end == now) s_t_end = s_t_start + 100;
    }
    s_level = (now - s_t_start) * (s_target - s_from) / (s_t_end - s_t_start) + s_from;
}

b8 envfx_underwater(void) { return s_cfg.underwater != 0; }

b8 envfx_active(void) {
    return s_cfg.underwater || (s_tint[0] || s_tint[1] || s_tint[2]) || (s_flash_on && s_level != 0);
}


/* 0x42FADF render_fullscreen_effect: every group of 4 columns is shifted
 * vertically by a sum of two travelling sine waves (+-16 rows); rows pushed
 * off the image are black. */
static void warp_frame(u32 *px, i32 now) {
    enum { W = 640, H = 480 };
    static u32 tmp[W * H];
    i32 off[160];
    for (i32 i = 0; i < 160; i++) {
        i32 a = (i32)((u32)(16 * now + (i << 9) - 16376) >> 4) & 0xFFF;
        i32 b = (i32)((u32)(16 * now - (i << 8) - 16376) >> 4) & 0xFFF;
        off[i] = (cosine_table[(a + 1024) & 0xFFF] + cosine_table[(b + 1024) & 0xFFF]) / 2048;
    }
    memcpy(tmp, px, sizeof(tmp));
    for (i32 j = 0; j < H; j++)
        for (i32 i = 0; i < 160; i++) {
            i32 sj = j - off[i];
            for (i32 k = 0; k < 4; k++)
                px[j * W + i * 4 + k] = (sj >= 0 && sj < H) ? tmp[sj * W + i * 4 + k] : 0xFF000000u;
        }
}


static i32 clamp63(i32 v) { return v < 0 ? 0 : v > 63 ? 63 : v; }

void envfx_apply_pixels(u32 *px, i32 n) {
    if (!px || gamma_bypassed() || !envfx_active()) return;
    if (s_cfg.underwater && n == 640 * 480) warp_frame(px, s_now);
    b8 tint = s_tint[0] || s_tint[1] || s_tint[2];
    b8 flash = s_flash_on && s_level != 0;
    if (!tint && !flash) return;
    for (i32 i = 0; i < n; i++) {
        u32 p = px[i];
        i32 c[3] = { (i32)(p & 0xFF) * 63 / 255, (i32)((p >> 8) & 0xFF) * 63 / 255,
                     (i32)((p >> 16) & 0xFF) * 63 / 255 };
        i32 d[3] = { 0, 0, 0 };
        if (tint) {
            for (i32 k = 0; k < 3; k++) {
                i32 t = clamp63(c[k] + s_tint[k]);
                d[k] = t - c[k];
                c[k] = t;
            }
        }
        if (flash) {
            i32 sq = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
            i32 v13 = s_level * sq / 1024;
            i32 add[3] = { (i32)((double)v13 * ((double)s_frz / 255.0)),
                           (i32)((double)v13 * ((double)s_fgz / 255.0)),
                           (i32)((double)v13 * ((double)s_fbz / 255.0)) };
            for (i32 k = 0; k < 3; k++) {
                i32 t = clamp63(c[k] + add[k]);
                d[k] += t - c[k];
            }
        }
        u32 out = p & 0xFF000000u;
        for (i32 k = 0; k < 3; k++) {
            i32 v = (i32)((p >> (8 * k)) & 0xFF) + d[k] * 255 / 63;
            out |= (u32)(v < 0 ? 0 : v > 255 ? 255 : v) << (8 * k);
        }
        px[i] = out;
    }
}
