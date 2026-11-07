/*
 * fx.c -- smoke puffs, blood spray and flame burst.
 *
 * Mirrors (dkev.exe addresses):
 *   sub_42F31B / 42F476   puffs: age + spawn on a wall hit        (12 records, type 7)
 *   initialize_entity_positions @0x411C27   puff spawn for the flame trail
 *   sub_4305EF            scaled 16x16 sprite blit, Z tested
 *   update_game_hud case 7 @0x467B?? the puff's projection and size
 *   play_sound_at_position @0x42CDA6   blood spray start (misnamed)
 *   sub_42D03F            blood spray, 32 particles for 0.6 s
 *   process_entity_random_transformations @0x411D09   flame burst start
 *   sub_411FE8            flame burst, 154 particles for 1.3 s
 *
 * The original pre-transforms the burst and blood positions into a camera
 * frame (transform_vector(&parameter_table[12])) before sub_45ACC5 projects
 * them again. The port keeps them in world coordinates and projects once
 * (scene_fx_project), which is the same point on screen.
 *
 * Only the 8-bit (palette) draw paths are ported; the port's frame is always
 * 32-bit, so palette indices go through the scene palette.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "libsys/common.h"
#include "libsys/graphics/shading.h"   /* cosine_table */
#include "state.h"
#include "game_init.h"
#include "fx.h"
#include "sfx.h"

#define FX_PUFFS      12
#define FX_BLOOD_N    32
#define FX_BURST_N    154

/* particle record, 14 dwords like the original (stride 56) */
typedef struct {
    i32 vel[3];     /* [0..2]  */
    i32 pos[3];     /* [3..5]  */
    i32 acc[3];     /* [6..8]  */
    i32 land[3];    /* [9..11] where it hit the floor */
    i32 landed;     /* [12] */
    i32 col;        /* [13] palette index */
} particle_t;

typedef struct {
    i32 state;      /* +0x70: 0 free, 1 live, 2 dying */
    i32 frame;      /* +0x30: SMOKE image 0..4 */
    i32 size;       /* +0x38 */
    i32 pos[3];     /* +0xA8..+0xB0 */
    i32 t0;         /* +0xB4 */
    i32 life;       /* +0xB8 */
} puff_t;

static puff_t     g_puff[FX_PUFFS];
#define FX_BUBBLES 32
typedef struct {
    i32 on;          /* +0x70 */
    i32 speed;       /* +0x34: 512..1535 */
    i32 size;        /* +0x38: 128..255 */
    i32 frame;       /* +0x30: BULLE image 0..3 */
    i32 pos[3];      /* +0xA8..+0xB0 */
    i32 t0;          /* +0xB4 */
    i32 dead;
} bubble_t;
static bubble_t g_bubble[FX_BUBBLES];
static i32 g_bub_last, g_bub_snd, g_bub_t0;   /* dword_4B4540 / dword_4B4544 / period origin */

static particle_t g_blood[FX_BLOOD_N];
static particle_t g_burst[FX_BURST_N];
static i32        g_blood_on = 0, g_blood_t0 = 0;                 /* dword_4B4508 / dword_509ADC */
static i32        g_burst_on = 0, g_burst_t0 = 0, g_burst_phase = 0;   /* dword_4B28F0 / special_mode_timer / dword_4CFE20 */

static const u8 k_blood_col[8] = { 0xFA, 0xF5, 0xE1, 0xDC, 0xFA, 0xF5, 0xE1, 0xDC };   /* byte_4B4510 */
static const u8 k_flame_col[8] = { 0xCD, 0xCE, 0xCF, 0xD0, 0xEE, 0xFC, 0xFD, 0xFE };   /* byte_4B28F8 */

static i32 now_ms(void) { return state ? (i32)state->current_game_time : 0; }

/* ls_random(n): same formula as effect.c, & 0xFFFF. */
static i32 rnd(i32 n) {
    return (i32)(((u32)n * ((u32)rand() & 0x7FFFu)) / 32767u) & 0xFFFF;
}
static i32 cos_at(i32 a) { return cosine_table[((u32)a >> 4) & 0xFFF]; }

void fx_reset(void) {
    memset(g_puff, 0, sizeof(g_puff));
    memset(g_bubble, 0, sizeof(g_bubble));
    g_bub_last = g_bub_snd = g_bub_t0 = 0;
    g_blood_on = 0;
    g_burst_on = 0;
}

/* ---- puffs ------------------------------------------------------------ */

// 00411C27
void fx_puff_flame(const i32 *pos, i32 life) {
    for (i32 i = 0; i < FX_PUFFS; i++) {
        puff_t *p = &g_puff[i];
        if (p->state) continue;
        p->pos[0] = pos[0] + rnd(0x2EE0);
        p->pos[1] = pos[1] + rnd(0x2EE0);
        p->pos[2] = pos[2] + rnd(0x2EE0);
        p->frame = 0;
        p->size = 16;
        p->t0 = now_ms();
        p->life = life;
        p->state = 1;
        return;
    }
}

// 0042F476
void fx_puff_spark(const i32 *pos) {
    for (i32 i = 0; i < FX_PUFFS; i++) {
        puff_t *p = &g_puff[i];
        if (p->state) continue;
        p->pos[0] = pos[0] + rnd(0x1770) - 3000;
        p->pos[1] = pos[1] + rnd(0x2EE0) - 6000;
        p->pos[2] = pos[2] + rnd(0x1770) - 3000;
        p->frame = 0;
        p->size = 16;
        p->t0 = now_ms();
        p->life = 384;
        p->state = 1;
        return;
    }
}

// 0042F31B
void fx_tick(i32 now) {
    for (i32 i = 0; i < FX_PUFFS; i++) {
        puff_t *p = &g_puff[i];
        if (p->state == 2) { p->state = 0; continue; }     /* update_overlay_effects frees the dying */
        if (p->state != 1) continue;
        i32 el = now - p->t0;
        if (el <= p->life) {
            p->frame = ((p->frame + 1) & 3) + 1;           /* 1..4 */
            if (el >= p->life / 2)
                p->size = p->life / 2 / 16 + 16 - (el - p->life / 2) / 16;
            else
                p->size = el / 16 + 16;
            p->pos[1] += 2 * el;                           /* rises, per frame */
        } else {
            p->state = 2;
        }
        if (now - p->t0 < 5) p->frame = 0;
    }
}

/* ---- drawing helpers -------------------------------------------------- */

static inline u32 rgb(const u8 *pal, i32 idx) {
    return 0xFF000000u | ((u32)pal[idx * 3 + 2] << 16) | ((u32)pal[idx * 3 + 1] << 8) | pal[idx * 3];
}

static inline i32 zat(const bmap_t *z, i32 x, i32 y) {
    if (!z || !z->pixels) return 0;
    i32 st = z->stride > 0 ? z->stride : (i32)z->width;
    if (x >= (i32)z->width || y >= z->height) return 0;
    return z->pixels[y * st + x];
}

/* One particle: the 3-pixel L the original writes (odd x: x,y / x,y+1 / x+1,y;
 * even x: x+1,y+1 / x,y+1 / x+1,y), each tested against the Z image. */
static void plot3(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, u32 c, i32 x, i32 y, i32 zv) {
    static const i32 odd[3][2]  = { {0, 0}, {0, 1}, {1, 0} };
    static const i32 even[3][2] = { {1, 1}, {0, 1}, {1, 0} };
    const i32 (*pt)[2] = (x & 1) ? odd : even;
    for (i32 k = 0; k < 3; k++) {
        i32 px = x + pt[k][0], py = y + pt[k][1];
        if (px < 0 || px > 639 || py < 0 || py > 479 || px >= fw || py >= fh) continue;
        if (zat(zimg, px, py) < zv) fb[py * fw + px] = c;
    }
}

// 004305EF
static void blit_sprite(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, const u8 *pal,
                        const bmap_t *spr, i32 a2, i32 cx, i32 cy, i32 zv) {
    if (!spr || !spr->pixels) return;
    i32 half = a2 >> 1;
    i32 n = a2 | 1;
    if (n >= 320) return;
    if (cx - half > 639 || cx + half < 0 || cy - half > 479 || cy + half < 0) return;
    i32 tab[320];
    i32 acc = 0;
    for (i32 i = 0; i < n; i++) { tab[i] = acc >> 16; acc += 1015808 / n; }
    i32 st = spr->stride > 0 ? spr->stride : 16;
    for (i32 r = 0; r < n; r++) {
        i32 y = cy - half + r;
        if (y < 0 || y > 479 || y >= fh) continue;
        const u8 *row = spr->pixels + st * tab[r];
        for (i32 c = 0; c < n; c++) {
            i32 x = cx - half + c;
            if (x < 0 || x > 639 || x >= fw) continue;
            u8 v = row[tab[c]];
            if (v && zat(zimg, x, y) < zv) fb[y * fw + x] = rgb(pal, v);
        }
    }
}

/* position at time t: acc * t^2 + vel * t + pos, or the landing spot */
static void particle_pos(const particle_t *p, f64 t, f64 t2, i32 out[3]) {
    if (p->landed) { memcpy(out, p->land, sizeof(p->land)); return; }
    for (i32 k = 0; k < 3; k++)
        out[k] = (i32)((f64)p->acc[k] * t2 + (f64)p->vel[k] * t + (f64)p->pos[k]);
}

/* ---- blood spray ------------------------------------------------------ */

// 0042CDA6
void fx_blood_start(i32 x, i32 y, i32 z, i32 a4) {
    if (g_blood_on) return;
    g_blood_on = 1;
    g_blood_t0 = now_ms();
    for (i32 i = 0; i < FX_BLOOD_N; i++) {
        particle_t *p = &g_blood[i];
        p->pos[0] = x + rnd(0x2710) - 5000;
        p->pos[1] = y + rnd(0x2710) - 5000;
        p->pos[2] = z + rnd(0x2710) - 5000;
        p->vel[0] = rnd(0x7530) + 4 * cos_at(a4 + 0x8008);
        p->vel[1] = rnd(0x36B0) + 20000;
        p->vel[2] = rnd(0x7530) + 4 * cos_at(a4 + 0x4008);
        p->acc[0] = 0; p->acc[1] = -200000; p->acc[2] = 0;
        p->col = k_blood_col[rnd(8)];
        p->landed = 0;
    }
}

// 0042D03F
static void blood_draw(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, const u8 *pal, i32 now) {
    if (!g_blood_on) return;
    f64 t = (f64)(now - g_blood_t0) / 1000.0;
    if (t > 0.6) { g_blood_on = 0; return; }
    f64 t2 = t * t;
    i32 floor_y = 0, zv = 1;
    for (i32 i = 0; i < FX_BLOOD_N; i++) {
        particle_t *p = &g_blood[i];
        i32 q[3];
        particle_pos(p, t, t2, q);
        if (i == 0) {
            floor_y = scene_fx_floor(q[0], q[2]);
            zv = scene_fx_zval(q[0], q[1], q[2]);
            if (!zv) zv = 1;
        }
        if (q[1] < floor_y) {                       /* lands and stays */
            q[1] = floor_y;
            memcpy(p->land, q, sizeof(q));
            p->landed = 1;
        }
        i32 sx, sy, cz;
        if (scene_fx_project(q[0], q[1], q[2], &sx, &sy, &cz))
            plot3(fb, fw, fh, zimg, rgb(pal, p->col), sx, sy, zv);
    }
}

/* ---- flame burst ------------------------------------------------------ */

// 00411D09
void fx_flame_burst(i32 x, i32 y, i32 z) {
    g_burst_on = 1;
    g_burst_t0 = now_ms();
    g_burst_phase = 1;
    for (i32 i = 0; i < FX_BURST_N; i++) {
        particle_t *p = &g_burst[i];
        p->pos[0] = x + rnd(0x1C20) - 3600;
        p->pos[1] = y + rnd(0x1C20) - 3600;
        p->pos[2] = z + rnd(0x1C20) - 3600;
        f64 a = (f64)(rnd(0x400) - 512), b = (f64)(rnd(0x400) - 512), c = (f64)(rnd(0x400) - 512);
        f64 len = sqrt(c * c + a * a + b * b);
        f64 k = len != 0.0 ? ((f64)rnd(0xFA0) + 4000.0) / len : 0.0;
        a *= k; b *= k; c *= k;
        p->vel[0] = (i32)(a * 16.0); p->vel[1] = (i32)(b * 16.0); p->vel[2] = (i32)(c * 16.0);
        p->acc[0] = (i32)(a * -12.0); p->acc[1] = (i32)(b * -12.0); p->acc[2] = (i32)(c * -12.0);
        p->col = k_flame_col[rnd(8)];
        p->landed = 0;
    }
}

// 00411FE8
static void burst_draw(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, const u8 *pal, const u8 *grd, i32 now) {
    if (!g_burst_on) return;
    f64 t = (f64)(now - g_burst_t0) / 1000.0;
    if (t >= 0.5 && g_burst_phase) {                /* half a second in: slow down, light gravity */
        g_burst_phase--;
        for (i32 i = 0; i < FX_BURST_N; i++) {
            particle_t *p = &g_burst[i];
            f64 t2 = t * t;
            p->pos[0] = (i32)((f64)p->vel[0] * t + (f64)p->acc[0] * t2 + (f64)p->pos[0]);
            p->pos[1] = (i32)((f64)p->vel[1] * t + (f64)p->acc[1] * t2 + (f64)p->pos[1]);
            p->pos[2] = (i32)((f64)p->vel[2] * t + (f64)p->acc[2] * t2 + (f64)p->pos[2]);
            p->vel[0] /= 8; p->vel[1] /= 8; p->vel[2] /= 8;
            p->acc[0] = 0; p->acc[1] = -10000; p->acc[2] = 0;
        }
        g_burst_t0 += 500;
        t = (f64)(now - g_burst_t0) / 1000.0;
    }
    if (t > 0.8) { g_burst_on = 0; return; }
    f64 t2 = t * t;
    i32 floor_y = 0, zv = 1;
    for (i32 i = 0; i < FX_BURST_N; i++) {
        particle_t *p = &g_burst[i];
        i32 c = k_flame_col[rnd(8)];
        if (t <= 0.5)      p->col = c;
        else if (grd)      p->col = grd[(p->col * 64 + 28 - 4 * (i32)t) & 0x3FFF];
        i32 q[3];
        particle_pos(p, t, t2, q);
        if (i == 0) {
            floor_y = scene_fx_floor(q[0], q[2]);
            zv = scene_fx_zval(q[0], q[1], q[2]);
            if (!zv) zv = 1;
        }
        if (q[1] < floor_y) {
            q[1] = floor_y;
            memcpy(p->land, q, sizeof(q));
            p->landed = 1;
        }
        i32 sx, sy, cz;
        if (scene_fx_project(q[0], q[1], q[2], &sx, &sy, &cz))
            plot3(fb, fw, fh, zimg, rgb(pal, p->col), sx, sy, zv);
    }
}


/* ---- air bubbles (underwater levels) ----------------------------------- */

static const u8 k_bubble_frame[16] = { 0, 1, 2, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3 };   /* byte_4B4530 */

// 00430081
void fx_bubbles_tick(i32 now, const i32 *player) {
    if (!g_bub_t0) g_bub_t0 = now;
    /* The original keys the 6 s period on a player timer (+0xD5C); the port uses
     * the time since the level started. */
    i32 v9 = (i32)((double)(now - g_bub_t0) * 0.001) % 6;
    i32 v18 = (now - g_bub_last) / 16;
    for (i32 i = 0; i < FX_BUBBLES; i++) {
        bubble_t *b = &g_bubble[i];
        if (b->on) {
            b->pos[1] += b->speed * (now - b->t0) / 1024;
        } else if (v18 && !v9) {
            if (now - g_bub_snd > 1000) {
                sfx_bruitage("bubulles", player[0], player[2]);
                g_bub_snd = now;
            }
            b->on = 1;
            b->speed = rnd(0x400) + 512;
            b->size = rnd(0x80) + 128;
            b->frame = k_bubble_frame[rnd(16)];
            b->t0 = now;
            b->pos[0] = player[0] + rnd(0x1770);
            b->pos[1] = player[1] + rnd(0x1770) + 18000;
            b->pos[2] = player[2] + rnd(0x1770);
            v18--;
        }
    }
    if (!v9 || v18) g_bub_last = now;
}

static void bubbles_draw(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, const u8 *pal, i32 now) {
    for (i32 i = 0; i < FX_BUBBLES; i++) {
        bubble_t *b = &g_bubble[i];
        if (!b->on) continue;
        i32 age = now - b->t0;
        i32 s = cosine_table[((u32)(((age << 9) + 8) >> 4) + 1024) & 0xFFF];   /* sine_table */
        i32 q[3] = { b->pos[0] + (s >> 5), b->pos[1], b->pos[2] - (s >> 6) };
        i32 sx, sy, cz;
        if (!scene_fx_project(q[0], q[1], q[2], &sx, &sy, &cz)) { b->on = 0; continue; }   /* state 2 -> freed */
        if (!cz) continue;
        i32 zv = scene_fx_zval(b->pos[0], b->pos[1], b->pos[2]);
        if (!zv) zv = 1;
        i32 sz = -(720000 * b->size / 128 / cz) | 1;
        blit_sprite(fb, fw, fh, zimg, pal, game_init_get_bubble(b->frame), sz, sx, sy, zv);
    }
}

/* ---- update_game_hud ---------------------------------------------------- */

void fx_draw(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, const u8 *pal, const u8 *grd, i32 now) {
    if (!fb || !pal) return;
    for (i32 i = 0; i < FX_PUFFS; i++) {            /* case 7 @ 0x467B..: projection and size */
        puff_t *p = &g_puff[i];
        if (p->state != 1) continue;
        i32 sx, sy, cz;
        if (!scene_fx_project(p->pos[0], p->pos[1], p->pos[2], &sx, &sy, &cz) || !cz) continue;
        i32 zv = scene_fx_zval(p->pos[0], p->pos[1], p->pos[2]);
        if (!zv) zv = 1;
        i32 sz = -(5758976 / cz);                   /* dword_57E400 is an address used as a constant */
        sz = (sz * p->size / 32) | 1;
        if (sz < 320) blit_sprite(fb, fw, fh, zimg, pal, game_init_get_smoke(p->frame), sz, sx, sy, zv);
    }
    bubbles_draw(fb, fw, fh, zimg, pal, now);
    blood_draw(fb, fw, fh, zimg, pal, now);
    burst_draw(fb, fw, fh, zimg, pal, grd, now);
}
