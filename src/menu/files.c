/*
 * files.c — sub_43E038 — save / load (MENUFICH / MENUCHAR).
 *
 * sub_43E038(ecx = mode, edx = defer):
 *   mode 0  save   (in game, F2)            backdrop MENUFICH.GIF
 *   mode 1  load   (main menu, in game F3)  backdrop MENUCHAR.GIF
 *   mode 2/3 the same into "ENTRY>" (Shift+F2/F3, debug; not ported)
 *   defer   1 from the main menu: the loader is not run here, the file name
 *           goes to byte_4D32E8 and the menu calls
 *           scene_load_savegame_level_43B9ED(byte_4D32E8) at 0x43A4C5.
 * The port has no separate menu step, so menu_files_run loads the game
 * itself (defer is only kept as a comment).
 *
 * Files, all in byte_5072D0 = "<game_path>SAVE>" (bin/game/save here):
 *   COUNT.GPT     i32[10] save number per slot (dword_5072A0, [0] unused)
 *                 + i32 last number handed out (dword_4B4774, 1..99)
 *   SAVEnn.VIG    i32 colour_mode_0_8bit_1_16bit, then 338 x 266 pixels
 *                 (stride * height bytes): 8-bit = indices into the
 *                 MENUFICH palette, 16-bit = one u16 per pixel
 *   SAVEnn.$$$    the game state (sub_43AB20 writes, sub_43B9ED reads)
 * byte_4B4840[slot] ("slot used") is set when SAVEnn.VIG exists.
 *
 * Screen (hit boxes at 0x4B4780, 16 bytes {x, y, w, h}, test sub_43D5D8):
 *   1..9  slot buttons      (0x205, 43 + ~40*i)  47 x 20
 *   10    exit              (0x21A, 0x189)       79 x 79
 *   11    OK (save / load)  (0x1CD, 0x189)       79 x 79
 * The selected slot's thumbnail fills the window at (87,86) 338 x 266
 * (DKELOGO.GIF when it has none) and each used slot shows its save number
 * in two FICHIMA2 digits. Up / Down step the slot (wrapping 9 <-> 1, auto
 * repeat every 200 ms), Enter = OK, Esc or the right button = exit; the
 * left button selects a slot or presses OK / exit while held. OK on an
 * unused slot does nothing when loading. Saving over a used slot asks
 * first (sub_4379C4, SAVE.GIF); "no" reopens the screen.
 *
 * Nothing else is shown: the original has no names or dates on this screen.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "menu/screens.h"
#include "state.h"
#include "game_init.h"
#include "sfx.h"
#include "screen_fx.h"
#include "scene.h"
#include "actor.h"
#include "entity.h"
#include "obc.h"
#include "inventory.h"
#include "lieu_cb.h"
#include "libsys/common.h"
#include "libsys/system.h"
#include "libsys/graphics/graphic.h"
#include "libsys/audio/wav.h"

extern i32 g_combat_flags;          /* debug_flags (combat_ai.c) */

#define THUMB_W   0x152             /* 338 */
#define THUMB_H   0x10A             /* 266 */
#define THUMB_X   0x57              /* window in MENUFICH */
#define THUMB_Y   0x56
#define SAVE_MAGIC 0xEA0            /* first dword = entity size */

/* State that outlives the screen, like the original globals. */
static i32 s_slot = 1;              /* dword_4B484C (initial 1) */
static u8  s_used[10];              /* byte_4B4840 */
static i32 s_number[10];            /* dword_5072A0 */
static i32 s_last_number;           /* dword_4B4774 */
static c8  s_dir[512];              /* byte_5072D0, host form */

typedef struct { i32 x, y, w, h; } fbox_t;

/* 0x4B4780: 12 entries, the last one empty. */
static const fbox_t BOX[11] = {
    { 0x205, 0x02B, 0x2F, 0x14 }, { 0x205, 0x053, 0x2F, 0x14 },
    { 0x205, 0x07B, 0x2F, 0x14 }, { 0x205, 0x0A3, 0x2F, 0x14 },
    { 0x205, 0x0CB, 0x2F, 0x14 }, { 0x205, 0x0F2, 0x2F, 0x14 },
    { 0x205, 0x11A, 0x2F, 0x14 }, { 0x205, 0x141, 0x2F, 0x14 },
    { 0x205, 0x16A, 0x2F, 0x14 },
    { 0x21A, 0x189, 0x4F, 0x4F },   /* 10 exit */
    { 0x1CD, 0x189, 0x4F, 0x4F },   /* 11 OK   */
};

/* Row of each slot's number (var_CC[1..9]) and of its highlight (the
 * switch at 0x43EB9E). */
static const i32 NUM_Y[10] = { 0, 0x2A, 0x52, 0x7A, 0xA1, 0xC9, 0xF1, 0x11A, 0x141, 0x169 };
static const i32 SEL_Y[10] = { 0, 0x2B, 0x53, 0x7C, 0xA3, 0xCB, 0xF3, 0x11B, 0x143, 0x16B };

typedef struct {
    bmap_t *bg;          /* dword_4D6188: MENUFICH / MENUCHAR */
    bmap_t *ima;         /* dword_4D338C: FICHIMA2 */
    bmap_t *logo;        /* dword_4D3388: DKELOGO */
    bmap_t *curs;        /* dword_4D3374: CURS128 */
    u8      pal[768];    /* dword_4D60D0, index 0 forced to black */
    u8      thumb[THUMB_W * THUMB_H];   /* dword_4D32E0, 8-bit */
    u32     thumb_rgb[THUMB_W * THUMB_H];
    i32     thumb_is_rgb;
} fscreen_t;

static fscreen_t *S;

/* ------------------------------------------------------------------ */
/* helpers                                                             */

static void f_free_bmap(bmap_t *b) {
    if (!b) return;
    if (b->owns_buffer && b->pixels) free(b->pixels);
    if (b->owns_palette && b->palette) free(b->palette);
    free(b);
}

static u32 rgb_of(const u8 *pal, u8 idx) {
    return 0xFF000000u | ((u32)pal[idx * 3 + 2] << 16) |
           ((u32)pal[idx * 3 + 1] << 8) | pal[idx * 3];
}

/* ls_bit_move(src, dst, x1, y1, x2, y2, dx, dy, mask): x2 / y2 exclusive. */
static void bit_move(u32 *fb, i32 fw, i32 fh, const bmap_t *src, const u8 *pal,
                     i32 x1, i32 y1, i32 x2, i32 y2, i32 dx, i32 dy, i32 mask) {
    if (!src || !src->pixels || !pal) return;
    for (i32 r = 0; r < y2 - y1; r++) {
        i32 y = dy + r, ys = y1 + r;
        if (y < 0 || y >= fh || ys < 0 || ys >= src->height) continue;
        for (i32 c = 0; c < x2 - x1; c++) {
            i32 x = dx + c, xs = x1 + c;
            if (x < 0 || x >= fw || xs < 0 || xs >= (i32)src->width) continue;
            u8 idx = src->pixels[ys * src->width + xs];
            if (mask && idx == 0) continue;
            fb[y * fw + x] = rgb_of(pal, idx);
        }
    }
}

static void click(void) { sfx_ui_click(0); }   /* help_screen_context */

static void shot(void) {
    const c8 *path = getenv("DKE_MENUSHOT");
    if (!path) return;
    FILE *f = fopen(path, "wb");
    if (!f) return;
    i32 fw = state->graphic.width, fh = state->graphic.height;
    fprintf(f, "P6\n%d %d\n255\n", fw, fh);
    for (i32 i = 0; i < fw * fh; i++) {
        u32 p = state->graphic.front_buffer[i];
        fputc((u8)p, f); fputc((u8)(p >> 8), f); fputc((u8)(p >> 16), f);
    }
    fclose(f);
}

static b8 file_exists(const c8 *p) {        /* select_language_version */
    FILE *f = fopen(p, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* byte_5072D0: sprintf("%sSAVE>", game_path). The port keeps the save
 * directory next to the game data (bin/game/save, created if missing). */
static void files_set_dir(void) {
    const c8 *base = (state && state->game_path) ? state->game_path : ".";
    struct stat st;
    snprintf(s_dir, sizeof(s_dir), "%s/SAVE", base);
    if (stat(s_dir, &st) == 0) return;
    snprintf(s_dir, sizeof(s_dir), "%s/save", base);
    if (stat(s_dir, &st) == 0) return;
#ifdef _WIN32
    _mkdir(s_dir);
#else
    mkdir(s_dir, 0755);
#endif
}

/* "%s>%s" + name with ".$$$" swapped for ext (sub_43DCC1 / sub_43DB85). */
static void slot_path(c8 *out, i32 n, i32 slot, const c8 *ext) {
    snprintf(out, n, "%s/SAVE%02d%s", s_dir, slot, ext);
}

/* ------------------------------------------------------------------ */
/* COUNT.GPT                                                           */

/* sub_43DF94 */
static void count_read(void) {
    c8 p[600];
    snprintf(p, sizeof(p), "%s/COUNT.GPT", s_dir);
    FILE *f = fopen(p, "rb");
    if (!f) {
        memset(s_number, 0, sizeof(s_number));
        s_last_number = 0;
        return;
    }
    u8 b[44] = {0};
    size_t n = fread(b, 1, sizeof(b), f);
    fclose(f);
    if (n < sizeof(b)) { memset(s_number, 0, sizeof(s_number)); s_last_number = 0; return; }
    for (i32 i = 0; i < 10; i++)
        s_number[i] = (i32)(b[i * 4] | b[i * 4 + 1] << 8 | b[i * 4 + 2] << 16 | (u32)b[i * 4 + 3] << 24);
    s_last_number = (i32)(b[40] | b[41] << 8 | b[42] << 16 | (u32)b[43] << 24);
}

static void put32(FILE *f, i32 v) {
    u8 b[4] = { (u8)v, (u8)(v >> 8), (u8)(v >> 16), (u8)((u32)v >> 24) };
    fwrite(b, 1, 4, f);
}

/* sub_43DF23 */
static void count_write(void) {
    c8 p[600];
    snprintf(p, sizeof(p), "%s/COUNT.GPT", s_dir);
    FILE *f = fopen(p, "wb");
    if (!f) return;
    for (i32 i = 0; i < 10; i++) put32(f, s_number[i]);
    put32(f, s_last_number);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* thumbnails                                                          */

/* sub_43DCC1: the slot's SAVEnn.VIG into dword_4D32E0, else DKELOGO. */
static void thumb_load(i32 slot) {
    c8 p[600];
    memset(S->thumb, 0, sizeof(S->thumb));                 /* ls_fill_rect 0 */
    S->thumb_is_rgb = 0;
    slot_path(p, sizeof(p), slot, ".VIG");
    FILE *f = fopen(p, "rb");
    if (f) {
        u8 h[4] = {0};
        if (fread(h, 1, 4, f) == 4) {
            i32 deep = h[0] | h[1] << 8 | h[2] << 16 | h[3] << 24;
            if (deep) {
                /* 16-bit capture: stride 676. The pixel format of the
                 * DirectDraw surface is not pinned; RGB565 assumed. */
                u8 *row = (u8 *)malloc(THUMB_W * 2);
                S->thumb_is_rgb = 1;
                for (i32 y = 0; row && y < THUMB_H; y++) {
                    if (fread(row, 1, THUMB_W * 2, f) != THUMB_W * 2) break;
                    for (i32 x = 0; x < THUMB_W; x++) {
                        u16 v = (u16)(row[x * 2] | row[x * 2 + 1] << 8);
                        u32 r = (v >> 11) & 31, g = (v >> 5) & 63, b = v & 31;
                        S->thumb_rgb[y * THUMB_W + x] = 0xFF000000u |
                            ((b << 3 | b >> 2) << 16) | ((g << 2 | g >> 4) << 8) | (r << 3 | r >> 2);
                    }
                }
                free(row);
            } else {
                size_t n = fread(S->thumb, 1, sizeof(S->thumb), f);
                (void)n;
            }
        }
        fclose(f);
        return;
    }
    if (S->logo && S->logo->pixels)                        /* dword_4D3388 */
        for (i32 y = 0; y < THUMB_H && y < S->logo->height; y++)
            for (i32 x = 0; x < THUMB_W && x < (i32)S->logo->width; x++)
                S->thumb[y * THUMB_W + x] = S->logo->pixels[y * S->logo->width + x];
}

/* sub_43D690 + sub_43D8B3: shrink the game frame (640 x 480) into the
 * thumbnail, remapped to the MENUFICH palette. The original dithers between
 * two remap tables on a checkerboard; one nearest-colour table here. */
static void thumb_capture(const u32 *game, i32 gw, i32 gh) {
    static u8  cache_ok[1 << 15];
    static u8  cache[1 << 15];
    memset(cache_ok, 0, sizeof(cache_ok));
    for (i32 y = 0; y < THUMB_H; y++)
        for (i32 x = 0; x < THUMB_W; x++) {
            i32 sy = y * gh / THUMB_H, sx = x * gw / THUMB_W;
            u32 p = game[sy * gw + sx];
            i32 r = p & 0xFF, g = (p >> 8) & 0xFF, b = (p >> 16) & 0xFF;
            i32 key = (r >> 3) << 10 | (g >> 3) << 5 | (b >> 3);
            if (!cache_ok[key]) {
                i32 best = 0, bd = 1 << 30;
                for (i32 i = 0; i < 256; i++) {
                    i32 dr = r - S->pal[i * 3], dg = g - S->pal[i * 3 + 1], db = b - S->pal[i * 3 + 2];
                    i32 d = dr * dr * 3 + dg * dg * 4 + db * db * 2;
                    if (d < bd) { bd = d; best = i; }
                }
                cache[key] = (u8)best;
                cache_ok[key] = 1;
            }
            S->thumb[y * THUMB_W + x] = cache[key];
        }
    S->thumb_is_rgb = 0;
}

/* sub_43DB85: write SAVEnn.VIG (8-bit, colour_mode 0). */
static void thumb_write(i32 slot) {
    c8 p[600];
    slot_path(p, sizeof(p), slot, ".VIG");
    FILE *f = fopen(p, "wb");
    if (!f) return;
    put32(f, 0);
    fwrite(S->thumb, 1, sizeof(S->thumb), f);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* SAVEnn.$$$                                                           */
/*
 * Layout (sub_43AB20 writes it in this order, sub_43B9ED reads it back).
 * Sizes in bytes; "(port)" marks what the port fills in, everything else
 * is written as zeros because the port has no equivalent global yet.
 *
 *   4      0xEA0 (file check)                                     (port)
 *   4      object_count, then per object 3 x i32
 *          (|menu_gametime - object_timers[i]|, 4FFB98[i], 4FFB9C[i])
 *   3 x 0x2EE0  dword_4FC1A0, dword_502BB0, dword_4FFCD0
 *   7 x 4  4FC190 50724C 507250 4FC194 507274 507270 4FFB50
 *   4      rgb_brightness_slider_control, 4 color_mode
 *   4      world_state (the room)                                 (port)
 *   3 x actor: player_entity, companion_entity, secondary_companion_entity
 *          4 pointer (0 = none, ends the block), 0xEA0 entity bytes,
 *          root pos x/y/z (+0x50 -> +0x3C/40/44), +0x30, +0x38, +0x34,
 *          root heading (+0x5C), +0x64                            (port)
 *   0xFA0  dword_505C40: LIEU script variables                    (port)
 *   0x300  palette_data
 *   4 + 0xEC4  dword_4B3440 / byte_4B3448: inventory (count, 135 x 0x1C)
 *   4 + 0xEC4  dword_509B34 / byte_50A160: confiscated copy
 *   4, 4   dword_4B430C, dword_4B3444 (inventory page)
 *   0x5C4  dword_506C80: script zones (entity pointers saved as indices)
 *   0x80 4DE3C0, 0x9D8 4DCDA0, 0x9D8 4DC2E0, 0xD2 4DCCC0,
 *   0x2BC resource_table, 0x2BC word_4DC020, 0x860 unk_4DAB60
 *   per used entry of unk_4DAB60 (8 x 0x10C, pointer at +0x50): 2 x 0xC
 *   6 x 4  animation_system_active, 4B2938 .. 4B2948
 *   4 player_movement_enabled (port), 4 dword_4FF0A0,
 *   4 special_mode_active (port), 4 score_counter, 4 4FF098, 4 4B2AB4
 *   0xE 531BC0, 0x3000 byte_4F60E0 (nav workspaces)
 *   0xC60  dword_4ECD50: hero descriptors                         (port)
 *   0xD040 dword_4DF5D0: 98 persona descriptors x 0x220           (port)
 *   4 need_change_level_id_4B7278, 4 dword_4B52E0, 0x620 word_509B40
 *   0xC8 byte_505AA0 + 3 x 0x28 (anim2D slots 10..19)
 *   0x100 byte_4FC090, 0x30 dword_4FB340
 *   0x40 x 4 (dword_4F5FB0[i] -> *[0]), 0x6E x 4 (render_buffer_pool)
 *   4 byte_4B7280, 4 dword_507254 (version), 0xC 4B53F0, 1 4B71F0,
 *   4 4B787C, 0x8000 byte_5492B0, 4 current_game_time (port), 4 4B7284
 *   16 x 0xF0  dword_543C60 effect records (times made relative)
 *   4 dword_506BE0 (port), 4 dword_4B2B98, 4 debug_flags (port),
 *   4 cinematic_mode (port), 8 dbl_543C00, 4 543C08, 4 543BEC, 8 543C38,
 *   0xC 543C20, 0xC 543BF0, 8 543C30, 8 543B28, 8 543C10, 4 level_timer
 *   9 x 8  byte_4B33F8 .. byte_4B3438: inventory hot keys 1..9
 */

#define INV_BYTES   0xEC4
#define INV_ENTRY   0x1C

static void wz(FILE *f, const void *p, i32 n) {
    if (p) { fwrite(p, 1, (size_t)n, f); return; }
    static const u8 z[256];
    while (n > 0) { i32 k = n > 256 ? 256 : n; fwrite(z, 1, (size_t)k, f); n -= k; }
}

static void save_actor(FILE *f, entity_t *e) {
    put32(f, e ? 1 : 0);
    if (!e) return;
    obc_node_t *r = EROOT(e);
    wz(f, e->raw.b, ENTITY_SIZE);
    put32(f, r ? r->pos[0] : 0);
    put32(f, r ? r->pos[1] : 0);
    put32(f, r ? r->pos[2] : 0);
    put32(f, E32(e, 0x30));
    put32(f, E32(e, 0x38));
    put32(f, E32(e, 0x34));
    put32(f, r ? r->rot[2] : 0);
    put32(f, E32(e, 0x64));
}

/* sub_43AB20 */
static b8 game_save(i32 slot) {
    c8 p[600];
    slot_path(p, sizeof(p), slot, ".$$$");
    FILE *f = fopen(p, "wb");
    if (!f) return 0;

    put32(f, SAVE_MAGIC);
    put32(f, 0);                                   /* object_count: TODO hook */
    wz(f, NULL, 3 * 0x2EE0);
    wz(f, NULL, 7 * 4);
    put32(f, 0);                                   /* rgb_brightness_slider_control */
    put32(f, 0);                                   /* color_mode */
    put32(f, state ? state->current_level_id : 1); /* world_state */
    save_actor(f, g_player_entity);
    save_actor(f, g_companion_entity);
    save_actor(f, g_secondary_companion_entity);
    wz(f, lieu_state_slot(0), 0xFA0);              /* dword_505C40 */
    wz(f, NULL, 0x300);                            /* palette_data */
    /* Inventory: no accessor yet (TODO hook inventory_save_state). */
    put32(f, 0); wz(f, NULL, INV_BYTES);
    put32(f, 0); wz(f, NULL, INV_BYTES);
    put32(f, 0); put32(f, 0);
    wz(f, NULL, 0x5C4);
    wz(f, NULL, 0x80 + 0x9D8 + 0x9D8 + 0xD2 + 0x2BC + 0x2BC + 0x860);
    /* unk_4DAB60 written as zeros: no 2 x 0xC records follow. */
    wz(f, NULL, 6 * 4);
    put32(f, g_player_movement_locked);
    put32(f, 0);                                   /* dword_4FF0A0 */
    put32(f, g_special_mode_active);
    put32(f, 0); put32(f, 0); put32(f, 0);         /* score, 4FF098, 4B2AB4 */
    wz(f, NULL, 0xE);
    wz(f, NULL, 0x3000);
    wz(f, g_hero_desc, HERO_COUNT * HERO_DESC_SIZE);
    for (i32 i = 0; i < PERSONA_COUNT; i++)
        wz(f, g_persona_desc[i].raw, PERSONA_DESC_SIZE);
    put32(f, 0); put32(f, 0);
    wz(f, NULL, 0x620);
    wz(f, NULL, 0xC8 + 3 * 0x28);
    wz(f, NULL, 0x100 + 0x30);
    wz(f, NULL, (0x40 + 0x6E) * 4);
    put32(f, 1);                                   /* byte_4B7280 */
    put32(f, 0);                                   /* dword_507254 */
    wz(f, NULL, 0xC + 1);
    put32(f, 0);
    wz(f, NULL, 0x8000);
    put32(f, state ? (i32)state->current_game_time : 0);
    put32(f, 0);
    wz(f, NULL, 16 * 0xF0);
    put32(f, g_skip_next_animation);               /* dword_506BE0 */
    put32(f, 0);
    put32(f, g_combat_flags);                      /* debug_flags */
    put32(f, g_cinematic_mode);
    wz(f, NULL, 8 + 4 + 4 + 8 + 0xC + 0xC + 8 + 8 + 8 + 4);
    wz(f, NULL, 9 * 8);                            /* hot keys: TODO hook */
    b8 ok = !ferror(f);
    fclose(f);
    print("files: saved slot %d -> %s\n", slot, p);
    return ok;
}

/* Parsed SAVEnn.$$$: what the port can put back. */
typedef struct {
    i32 world_state;
    i32 has[3];
    u8  ent[3][ENTITY_SIZE];
    i32 pos[3][3], d30[3], d38[3], d34[3], head[3], d64[3];
    u8  lieu[0xFA0];
    i32 inv_count;
    u8  inv[INV_BYTES];
    u8  hero[HERO_COUNT * HERO_DESC_SIZE];
    u8  persona[PERSONA_COUNT][PERSONA_DESC_SIZE];
    i32 movement_locked, special_mode, skip_next, combat_flags, cinematic;
} save_t;

typedef struct { const u8 *p; size_t n, at; b8 bad; } rd_t;

static const u8 *rd(rd_t *r, size_t n) {
    if (r->bad || r->at + n > r->n) { r->bad = 1; return NULL; }
    const u8 *q = r->p + r->at;
    r->at += n;
    return q;
}

static i32 rd32(rd_t *r) {
    const u8 *q = rd(r, 4);
    return q ? (i32)(q[0] | q[1] << 8 | q[2] << 16 | (u32)q[3] << 24) : 0;
}

static void rdcpy(rd_t *r, void *dst, size_t n) {
    const u8 *q = rd(r, n);
    if (q && dst) memcpy(dst, q, n);
}

/* sub_43B9ED's read pass (0x43BA97..0x43CACB). */
static b8 save_parse(const u8 *buf, size_t len, save_t *s) {
    rd_t r = { buf, len, 0, 0 };
    if (rd32(&r) != SAVE_MAGIC) {
        print("files: Sauvegarde non valide\n");
        return 0;
    }
    i32 objs = rd32(&r);
    if (objs < 0 || objs > 0x10000) return 0;
    rd(&r, (size_t)objs * 12);
    rd(&r, 3 * 0x2EE0 + 7 * 4 + 4 + 4);
    s->world_state = rd32(&r);
    for (i32 a = 0; a < 3; a++) {
        s->has[a] = rd32(&r) != 0;
        if (!s->has[a]) continue;
        rdcpy(&r, s->ent[a], ENTITY_SIZE);
        for (i32 k = 0; k < 3; k++) s->pos[a][k] = rd32(&r);
        s->d30[a] = rd32(&r); s->d38[a] = rd32(&r); s->d34[a] = rd32(&r);
        s->head[a] = rd32(&r); s->d64[a] = rd32(&r);
    }
    rdcpy(&r, s->lieu, 0xFA0);
    rd(&r, 0x300);
    s->inv_count = rd32(&r);
    rdcpy(&r, s->inv, INV_BYTES);
    rd(&r, 4 + INV_BYTES + 4 + 4);
    rd(&r, 0x5C4 + 0x80 + 0x9D8 + 0x9D8 + 0xD2 + 0x2BC + 0x2BC);
    const u8 *tab = rd(&r, 0x860);                 /* unk_4DAB60 copy */
    for (i32 i = 0; tab && i < 8; i++) {
        const u8 *q = tab + i * 0x10C + 0x50;
        if (q[0] | q[1] | q[2] | q[3]) rd(&r, 2 * 0xC);
    }
    rd(&r, 6 * 4);
    s->movement_locked = rd32(&r);
    rd32(&r);                                      /* dword_4FF0A0 (var_528) */
    s->special_mode = rd32(&r);
    rd(&r, 3 * 4 + 0xE + 0x3000);
    rdcpy(&r, s->hero, sizeof(s->hero));
    rdcpy(&r, s->persona, sizeof(s->persona));
    rd(&r, 4 + 4 + 0x620 + 0xC8 + 3 * 0x28 + 0x100 + 0x30 + (0x40 + 0x6E) * 4);
    rd(&r, 4 + 4 + 0xC + 1 + 4 + 0x8000 + 4 + 4 + 16 * 0xF0);
    s->skip_next = rd32(&r);
    rd32(&r);                                      /* dword_4B2B98 */
    s->combat_flags = rd32(&r);
    s->cinematic = rd32(&r);
    rd(&r, 8 + 4 + 4 + 8 + 0xC + 0xC + 8 + 8 + 8 + 4 + 9 * 8);
    return !r.bad;
}

/* scene_load_savegame_level_43B9ED, as far as the port's state goes. */
static b8 game_load(i32 slot) {
    c8 p[600];
    slot_path(p, sizeof(p), slot, ".$$$");
    FILE *f = fopen(p, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    u8 *buf = len > 0 ? (u8 *)malloc((size_t)len) : NULL;
    save_t *s = (save_t *)calloc(1, sizeof(save_t));
    b8 ok = buf && s && fread(buf, 1, (size_t)len, f) == (size_t)len &&
            save_parse(buf, (size_t)len, s);
    fclose(f);
    free(buf);
    if (!ok) { free(s); return 0; }

    /* 0x43B9FF: sub_43A63D(0, -1) resets the game (script variables
     * included), then sub_46E4A8 drops the level. A level already running
     * is left to scene_load_level, like the menu's NEW GAME.
     * TODO hook: scene_unload_level() (see report). */
    if (state && state->current_level_id > 0) {
        lieu_exit();
        actor_release_companions();
    }
    actor_new_game();

    /* Globals restored before the level loads (read pass). */
    memcpy(lieu_state_slot(0), s->lieu, 0xFA0);              /* dword_505C40 */
    memcpy(g_hero_desc, s->hero, sizeof(s->hero));           /* dword_4ECD50 */
    for (i32 i = 0; i < PERSONA_COUNT; i++) {                /* dword_4DF5D0 */
        memcpy(g_persona_desc[i].raw, s->persona[i], PERSONA_DESC_SIZE);
        memset(g_persona_desc[i].raw, 0, 4);                 /* 0x43CBF6: [i*0x220] = 0 */
    }
    g_special_mode_active = s->special_mode;
    g_cinematic_mode      = s->cinematic;
    g_combat_flags        = s->combat_flags;
    g_skip_next_animation = s->skip_next;
    (void)s->movement_locked;                                /* read into var_230 only */

    /* 0x43D20A: scene_load_level_46DFEC(world_state). */
    i32 level = s->world_state;
    if (level <= 0 || level > 99 || scene_load_level(level) != 0) {
        print("files: scene_load_level(%d) failed\n", level);
        free(s);
        return 0;
    }

    /* The hero being played: entity +0xB0 = abs(persona id). */
    i32 hero = s->has[0] ? *(i32 *)&s->ent[0][0xB0] : -1;
    if (hero >= 0 && hero < HERO_COUNT && hero != actor_hero_id())
        actor_hero_swap(hero);

    /* 0x43CCC7 / 0x43D20F: player fields and root transform. */
    entity_t *pe = g_player_entity;
    if (pe && s->has[0]) {
        const u8 *e = s->ent[0];
        E32(pe, 0xCB4) = *(const i32 *)&e[0xCB4];
        E32(pe, 0xCB8) = *(const i32 *)&e[0xCB8];
        E32(pe, 0xC78) = *(const i32 *)&e[0xC78];
        E32(pe, 0x30)  = s->d30[0];
        E32(pe, 0x38)  = s->d38[0];
        E32(pe, 0x34)  = s->d34[0];
        E32(pe, 0x64)  = s->d64[0];
        E32(pe, 0xC0)  = 0x7FFFFFFF;
        if (EROOT(pe)) {
            EROOT(pe)->pos[0] = s->pos[0][0];
            EROOT(pe)->pos[1] = s->pos[0][1];
            EROOT(pe)->pos[2] = s->pos[0][2];
            EROOT(pe)->rot[2] = s->head[0];
        }
        E32(pe, 0xCEC) = 0; E32(pe, 0xCF0) = 1;              /* 0x43D25E */
        E32(pe, 0xCF4) = 0; E32(pe, 0xCF8) = 1;
    }

    /* 0x43CDB6 / 0x43CFCF: companions back with POSPERSO (sub_4640BC). */
    for (i32 a = 1; a < 3; a++) {
        if (!s->has[a]) continue;
        const u8 *e = s->ent[a];
        i32 id  = *(const i32 *)&e[0xB0];
        i32 vie = *(const i32 *)&e[0x70];
        if (vie == 0 || vie == -1) vie = 2;
        c8 anim[16];
        memcpy(anim, &e[0xC4], sizeof(anim) - 1);
        anim[sizeof(anim) - 1] = 0;
        entity_t *ce = actor_pos_perso(id, s->pos[a][0], s->pos[a][2], s->head[a],
                                       vie, anim, 0, -1);
        if (ce && EROOT(ce)) EROOT(ce)->pos[1] = s->pos[a][1];
    }

    /* Inventory: re-added item by item (no clear exists yet, see report). */
    for (i32 i = 0; i < INV_BYTES / INV_ENTRY; i++) {
        const u8 *q = s->inv + i * INV_ENTRY;
        if (!q[0]) continue;
        c8 name[9];
        memcpy(name, q, 8);
        name[8] = 0;
        i32 qty = *(const i32 *)&q[0x14];
        if (qty > 0) inv_add(name, qty, 0);
    }

    free(s);
    print("files: loaded slot %d (level %d)\n", slot, level);
    return 1;
}

/* ------------------------------------------------------------------ */
/* screen                                                              */

/* sub_43D5D8: strict inside test, index + 1. */
static i32 hit_test(i32 mx, i32 my) {
    for (i32 i = 0; i < 11; i++)
        if (BOX[i].x < mx && BOX[i].x + BOX[i].w > mx &&
            BOX[i].y < my && BOX[i].y + BOX[i].h > my) return i + 1;
    return 0;
}

static void draw_screen(u32 *fb, i32 fw, i32 fh, i32 save_mode, i32 hover, i32 mx, i32 my) {
    const u8 *pal = S->pal;
    bit_move(fb, fw, fh, S->bg, pal, 0, 0, 640, 480, 0, 0, 0);
    /* sub_43DCC1: thumbnail, then the backdrop's frame masked over it. */
    for (i32 y = 0; y < THUMB_H; y++)
        for (i32 x = 0; x < THUMB_W; x++) {
            i32 X = THUMB_X + x, Y = THUMB_Y + y;
            if (X >= fw || Y >= fh) continue;
            fb[Y * fw + X] = S->thumb_is_rgb ? S->thumb_rgb[y * THUMB_W + x]
                                             : rgb_of(pal, S->thumb[y * THUMB_W + x]);
        }
    bit_move(fb, fw, fh, S->bg, pal, THUMB_X, THUMB_Y, 0x1A9, 0x160, THUMB_X, THUMB_Y, 1);
    /* 0x43EB9E: the selected slot. */
    if (s_slot >= 1 && s_slot <= 9)
        bit_move(fb, fw, fh, S->ima, pal, 1, 3, 0x2B, 0x13, 0x205, SEL_Y[s_slot], 1);
    /* 0x43EBE2: the pressed button under the pointer. */
    if (hover == 10)
        bit_move(fb, fw, fh, S->ima, pal, 0x63, 0, 0x9A, 0x36, 0x226, 0x196, 1);
    else if (hover == 11 && save_mode)
        bit_move(fb, fw, fh, S->ima, pal, 0x2C, 0, 0x63, 0x36, 0x1D8, 0x193, 1);
    else if (hover == 11)
        bit_move(fb, fw, fh, S->ima, pal, 0x9B, 0, 0xD2, 0x36, 0x1D7, 0x196, 1);
    /* 0x43ECA8: save numbers, digits 12 x 13 from FICHIMA2 (d*12, 0x37). */
    for (i32 i = 1; i < 10; i++) {
        i32 hi = s_number[i] / 10, lo = s_number[i] - hi * 10;
        i32 y = NUM_Y[i] + 4;
        if (hi) {
            bit_move(fb, fw, fh, S->ima, pal, hi * 12, 0x37, hi * 12 + 12, 0x44, 0x20C, y, 1);
            bit_move(fb, fw, fh, S->ima, pal, lo * 12, 0x37, lo * 12 + 12, 0x44, 0x21B, y, 1);
        } else if (lo) {
            bit_move(fb, fw, fh, S->ima, pal, lo * 12, 0x37, lo * 12 + 12, 0x44, 0x213, y, 1);
        }
    }
    /* sub_43F57C: pointer 29 x 33 from CURS128, the hand (x 0) over a box. */
    if (S->curs) {
        i32 sx = hover ? 0 : 0x20;
        bit_move(fb, fw, fh, S->curs, pal, sx, 0, sx + 0x1D, 0x21, mx, my, 1);
    }
}

/* sub_4379C4: "overwrite?" box over the current picture. Returns nonzero
 * to go on with the save. Right button leaves with -1, which the caller
 * takes as yes (as in the original). */
static i32 confirm_overwrite(void) {
    u32 *fb = state->graphic.front_buffer;
    i32 fw = state->graphic.width, fh = state->graphic.height;
    system_t *sys = &state->system;
    bmap_t *box = game_init_load_cursor("DATAS>MENUS>SAVE.GIF");
    if (!box) return 1;
    u32 *under = (u32 *)malloc((size_t)fw * fh * 4);
    if (!under) { f_free_bmap(box); return 1; }
    memcpy(under, fb, (size_t)fw * fh * 4);
    /* 40-byte entries: src x/y, w/h, dst x/y, value. */
    static const struct { i32 sx, sy, w, h, dx, dy, val; } B[2] = {
        { 0x13B, 0x00, 0x20, 0x20, 0x7D,  0xE1, 1 },     /* no  */
        { 0x13B, 0x20, 0x22, 0x25, 0x15A, 0xDD, 2 },     /* yes */
    };
    i32 sel = 1, result = 0;                 /* var_4, var_78 */
    i32 prev[256];
    memcpy(prev, sys->actions, sizeof(prev));
    #define EDGE(k) (sys->actions[k] && !prev[k])
    u32 opened = ls_tick();
    i32 armed = !sys->mouse_left;
    while (!sys->quit) {
        ls_update_events(sys);
        sfx_pump();
        i32 kl = EDGE(37), kr = EDGE(39), ke = EDGE(13), kesc = EDGE(27);
        memcpy(prev, sys->actions, sizeof(prev));
        if (getenv("DKE_FRAMES") && ls_tick() - opened > 1500) ke = 1;   /* headless */
        if (!sys->mouse_left) armed = 1;
        if (sys->mouse_right) { result = 0; break; }
        i32 mx = sys->motion_x, my = sys->motion_y, hov = 0;
        for (i32 i = 0; i < 2; i++)
            if (B[i].dx < mx && B[i].dx + B[i].w > mx && B[i].dy < my && B[i].dy + B[i].h > my) {
                hov = i + 1;
                if (armed && sys->mouse_left) result = B[i].val;
            }
        if (result) break;
        if (kl || kr) sel = abs(sel - 1);
        if (ke) { memcpy(fb, under, (size_t)fw * fh * 4); free(under); f_free_bmap(box); return sel; }
        if (kesc) { memcpy(fb, under, (size_t)fw * fh * 4); free(under); f_free_bmap(box); return 0; }
        memcpy(fb, under, (size_t)fw * fh * 4);
        bit_move(fb, fw, fh, box, S->pal, 0, 0, 0x13A, 0x6F, 0x62, 0xB8, 1);
        bit_move(fb, fw, fh, box, S->pal, B[sel].sx, B[sel].sy, B[sel].sx + B[sel].w,
                 B[sel].sy + B[sel].h, B[sel].dx, B[sel].dy, 1);
        if (S->curs) {
            i32 sx = hov ? 0 : 0x20;
            bit_move(fb, fw, fh, S->curs, S->pal, sx, 0, sx + 0x1D, 0x20, mx, my, 1);
        }
        shot();
        fx_present();
        ls_delay(10);
    }
    #undef EDGE
    while (!sys->quit && (sys->mouse_left || sys->mouse_right)) { ls_update_events(sys); ls_delay(10); }
    memcpy(fb, under, (size_t)fw * fh * 4);
    free(under);
    f_free_bmap(box);
    return result - 1;
}

static void screen_free(void) {
    if (!S) return;
    f_free_bmap(S->bg); f_free_bmap(S->ima); f_free_bmap(S->logo); f_free_bmap(S->curs);
    free(S);
    S = NULL;
}

/* sub_43E038. save_mode = !(mode & 1). Returns 1 after a load. */
static i32 files_run(i32 save_mode) {
    if (!state || !state->graphic.front_buffer) return 0;
    u32 *fb = state->graphic.front_buffer;
    i32 fw = state->graphic.width, fh = state->graphic.height;
    system_t *sys = &state->system;

    /* 0x43E1B8: no saving while the script forbids it (dword_505C50 =
     * ST 0x10) or the player is busy (+0xD60). */
    if (save_mode && (*lieu_state_slot(0x10) != 0 ||
                      (g_player_entity && E32(g_player_entity, 0xD60) != 0))) {
        print("Dark Earth message :>> Sauvegarde impossible\n");
        return 0;
    }

    /* The game frame is still in the front buffer: keep it for the
     * thumbnail (sub_43D8B3 reads game_context). */
    u32 *game = NULL;
    if (save_mode) {
        game = (u32 *)malloc((size_t)fw * fh * 4);
        if (game) memcpy(game, fb, (size_t)fw * fh * 4);
    }

reopen:
    files_set_dir();
    S = (fscreen_t *)calloc(1, sizeof(fscreen_t));
    if (!S) { free(game); return 0; }
    S->bg   = game_init_load_cursor(save_mode ? "DATAS>MENUS>MENUFICH.GIF" : "DATAS>MENUS>MENUCHAR.GIF");
    S->ima  = game_init_load_cursor("DATAS>MENUS>FICHIMA2.GIF");
    S->logo = game_init_load_cursor("DATAS>MENUS>DKELOGO.GIF");
    S->curs = game_init_load_cursor("DATAS>MENUS>CURS128.GIF");
    if (!S->bg || !S->ima || !S->bg->palette) { screen_free(); free(game); return 0; }
    memcpy(S->pal, S->bg->palette, 768);
    S->pal[0] = S->pal[1] = S->pal[2] = 0;          /* 0x43E224 */

    /* 0x43E2C6: slots with a thumbnail are used. */
    for (i32 i = 1; i < 10; i++) {
        c8 p[600];
        slot_path(p, sizeof(p), i, ".VIG");
        if (file_exists(p)) s_used[i] = 1;
    }
    count_read();                                   /* sub_43DF94 */
    thumb_load(s_slot);

    sys->motion_x = 0; sys->motion_y = 0;           /* 0x43E36C */
    i32 prev[256];
    memcpy(prev, sys->actions, sizeof(prev));
    i32 armed = !sys->mouse_left;                   /* ignore the click that opened us */
    u32 opened = ls_tick(), last_up = 0, last_dn = 0;
    i32 hover = 0, result = 0;                      /* var_4 */

    while (!sys->quit) {
        ls_update_events(sys);
        sfx_pump();
        u32 now = ls_tick();
        i32 kesc = sys->actions[27] && !prev[27];
        i32 kent = sys->actions[13] && !prev[13];
        i32 dn = sys->actions[40], up = sys->actions[38];
        i32 dn_edge = dn && !prev[40], up_edge = up && !prev[38];
        memcpy(prev, sys->actions, sizeof(prev));
        if (getenv("DKE_FRAMES") && now - opened > 1500) kesc = 1;   /* headless */
        if (!sys->mouse_left) armed = 1;
        i32 held = armed && sys->mouse_left;        /* dword_4D3368 == 1 */

        i32 mx = sys->motion_x, my = sys->motion_y;
        hover = hit_test(mx, my);

        /* 0x43E740: a held click on a slot selects it. */
        if (hover > 0 && hover < 10 && held) {
            if (s_slot != hover) click();
            s_slot = hover;
            thumb_load(s_slot);
        }
        /* 0x43E7C4: Down / Up, repeating every 200 ms. */
        if (dn && (dn_edge || now - last_dn >= 200)) {
            click();
            last_dn = now;
            if (++s_slot >= 10) s_slot = 1;
            thumb_load(s_slot);
        }
        if (up && (up_edge || now - last_up >= 200)) {
            click();
            last_up = now;
            if (--s_slot <= 0) s_slot = 9;
            thumb_load(s_slot);
        }

        /* 0x43E940: Enter = OK when the slot can be used. */
        if (kent && (save_mode || s_used[s_slot])) { click(); result = 11; }
        /* 0x43EE86: Esc or the right button leaves. */
        else if (kesc || sys->mouse_right) result = 10;
        else if (hover >= 10 && held) {
            if (hover == 11 && (save_mode || s_used[s_slot])) { click(); result = 11; }
            else if (hover == 10) { click(); result = 10; }
        }

        draw_screen(fb, fw, fh, save_mode, result ? result : hover, mx, my);
        shot();
        fx_present();
        ls_delay(10);
        if (result) break;
    }
    /* 0x43EF8B: wait for the buttons to come up. */
    while (!sys->quit && (sys->mouse_left || sys->mouse_right)) { ls_update_events(sys); ls_delay(10); }
    f_free_bmap(S->logo); S->logo = NULL;           /* 0x43EF76 */

    if (result != 11) { screen_free(); free(game); return 0; }

    if (save_mode) {
        /* 0x43F131: overwriting a used slot asks first; "no" reopens. */
        if (s_number[s_slot] != 0 && !confirm_overwrite()) {
            screen_free();
            goto reopen;
        }
        if (game) thumb_capture(game, fw, fh);
        else memset(S->thumb, 0, sizeof(S->thumb));
        thumb_write(s_slot);                        /* sub_43DB85 */
        if (game_save(s_slot)) {                    /* sub_43AB20 */
            s_used[s_slot] = 1;
            s_last_number = (s_last_number == 99) ? 1 : s_last_number + 1;
            s_number[s_slot] = s_last_number;
            count_write();                          /* sub_43DF23 */
        }
        screen_free();
        free(game);
        return 0;
    }

    /* 0x43F359: load. The original's defer path copies "SAVEnn.$$$" to
     * byte_4D32E8 for the menu; here the load runs at once. */
    i32 slot = s_slot;
    screen_free();
    free(game);
    return game_load(slot) ? 1 : 0;
}

/* Main menu FILES (sub_43E038(1, 1)) and in-game F3 (sub_43E038(1, 0)). */
i32 menu_files_run(void) {
    return files_run(0);
}

/* In-game F2: sub_43E038(0, 0). */
i32 menu_files_save_run(void) {
    return files_run(1);
}
