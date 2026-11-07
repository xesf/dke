/*
 * pause.c — the P key pause screen and the clock compensation around modal
 * screens.
 *
 *   display_pause_screen   0x40A2B0   PAUSEDn picture blended with the frozen frame, cheat codes
 *   check_keyboard_letter  0x40A280   first A..Z held, else Enter
 *   sub_44D68C / sub_44D6C9           ls_pause_sound / ls_resume_sound over the 50 cached sounds
 *   set_display_mode       0x4063FE   open a modal screen: remember the clock
 *   restore_display_mode   0x4064C3   close it: shift every timer by the time it was open
 *   update_game_timing_state 0x4067EA same, after a loaded game (rebase instead of shift)
 *   sub_41FAA2             0x41FAA2   stamp an entity with current_game_time
 *   update_entity_animation 0x41FAD1  shift the entity animation clocks by the time since the stamp
 *
 * Port notes: the DirectDraw display mode switch and apply_screen_gamma (0x40ACC0, gamma
 * package) are not ported, the timer history ring (dword_548B70..) is profiler data, and the
 * PrintScreen -> SCnnnnn.PCX capture (ls_save_pcx) is left out.
 *
 * Test hooks: DKE_PAUSE=<frame> presses P at that scene frame (the screen closes by itself
 * after DKE_PAUSE_MS, default 1500); DKE_PAUSE_TYPE=<word> types a cheat word then Enter;
 * DKE_PAUSE_SHOT=<file.ppm> dumps the pause screen.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "libsys/common.h"
#include "libsys/system.h"
#include "libsys/graphics/graphic.h"
#include "libsys/graphics/font.h"
#include "state.h"
#include "actor.h"
#include "effect.h"
#include "dialogue.h"
#include "lieu_action.h"
#include "game_init.h"
#include "screen_fx.h"
#include "sfx.h"
#include "pause.h"
#include "cjr.h"

extern state_t *state;

i32 g_easy_mode_enabled = 0;           /* 0x4B2170 */
i32 g_limit_override_flag = 0;         /* 0x4B70CC */
i32 g_debug_display_flag = 0;          /* 0x546340 */
i32 g_character_model_override = 0;    /* 0x505D74 */

/* ========================================================================
 * Clock compensation
 * ======================================================================== */

#define TIMER_STACK 16
static i32 s_depth;                    /* timer_stack_depth @ 0x4B218C */
static i32 s_ctx[TIMER_STACK];         /* timer_context_stack @ 0x548F00 */
static i32 s_time_delta;               /* time_delta @ 0x4B2188: current_game_time at the freeze, 0 = none */

static i32 clock_now(void) { return (i32)ls_tick(); }   /* system_get_gametime */

/* sub_41FAA2: this[782] = current_game_time. */
static void entity_stamp(entity_t *e) {
    if (e) E32(e, 0xC38) = (i32)state->current_game_time;
}

/* update_entity_animation 0x41FAD1: the clip clocks move on by the time since
 * the stamp. */
static void update_entity_animation(entity_t *e) {
    if (!e) return;
    i32 d = clock_now() - E32(e, 0xC38);
    E32(e, 0xC28) += d;                /* v3[778] clip start */
    E32(e, 0xC34) += d;                /* v3[781] */
    E32(e, 0xE8)  += d;                /* v3[58] */
    E32(e, 0xC30) += d;                /* v3[780] */
}

/* 0x4063FE */
void game_time_freeze(i32 ctx) {
    if (!state) return;
    if (s_depth < TIMER_STACK) s_ctx[s_depth] = ctx;
    s_depth++;
    if (!s_time_delta) {
        s_time_delta = (i32)state->current_game_time;
        entity_stamp(g_player_entity);
        entity_stamp(g_companion_entity);
        entity_stamp(g_secondary_companion_entity);
    }
}

/* Shared body of 0x4064C3 (reload = 0) and 0x4067EA (reload = 1). */
static void thaw(i32 refresh_clock, i32 ctx, i32 reload) {
    if (!state) return;
    if (!s_depth) { s_time_delta = 0; return; }
    --s_depth;
    if (s_depth < TIMER_STACK && s_ctx[s_depth] != ctx && state->debug_enabled)
        print("pause: restore_display_mode: mismatched context %d (opened as %d)\n", ctx, s_ctx[s_depth]);
    if (!s_time_delta || s_depth) return;

    if (refresh_clock) {
        state->time_backup = state->current_game_time;
        state->current_game_time = (u32)clock_now();
    }
    i32 td = clock_now() - s_time_delta;
    if (getenv("DKE_PAUSE")) print("pause: ctx %d closed after %d ms, timers shifted\n", ctx, td);
    entity_t *pl = g_player_entity, *c1 = g_companion_entity, *c2 = g_secondary_companion_entity;
    if (pl) E32(pl, 0xD5C) -= td;
    /* dword_4EDFAC += td: not read by the port. */
    entity_t *three[3] = { pl, c1, c2 };
    for (i32 i = 0; i < 3; i++) {
        if (!three[i]) continue;
        E32(three[i], 0xC0) += td;
        update_entity_animation(three[i]);
    }
    /* The other personas keep their own clip clocks in the port, so they move
     * on as well. */
    for (i32 i = 0; i < PERSONA_COUNT; i++) {
        entity_t *e = g_persona_desc[i].entity;
        if (!e || e == pl || e == c1 || e == c2) continue;
        E32(e, 0xC28) += td; E32(e, 0xC34) += td; E32(e, 0xE8) += td; E32(e, 0xC30) += td;
        E32(e, 0xC0) += td;
    }
    if (reload) {
        effect_rebase_time((i32)state->current_game_time);                /* game_structures_time_values */
        lieu_timers_shift(td, 1, state->current_game_time);               /* object_timers */
    } else {
        effect_shift_time(td);
        lieu_timers_shift(td, 0, 0);
    }
    /* last_global_animation_update, level_timer, special/bonus timers and
     * the energy / score stamps have no counterpart in the port. */
    dlg_time_shift(td);                                                   /* world_timer */
    cjr_shift_time(td);                                                   /* bonus_timer */
    if (!reload) {
        if (c1 && E32(c1, 0xCE4)) E32(c1, 0xCE4) += td;
        if (c2 && E32(c2, 0xCE4)) E32(c2, 0xCE4) += td;
        /* dword_5065A0 (ST_GAME_TIME) is rewritten every frame. */
    }
    s_time_delta = 0;
}

/* 0x4064C3 */
void game_time_thaw(i32 refresh_clock, i32 ctx) { thaw(refresh_clock, ctx, 0); }
/* 0x4067EA */
void game_time_thaw_reload(i32 refresh_clock, i32 ctx) { thaw(refresh_clock, ctx, 1); }

/* ========================================================================
 * Pause screen
 * ======================================================================== */

/* DKE_PAUSE_TYPE: the headless run "types" the word, 40 ms per key, then Enter. */
static i32 injected_key(void) {
    static const c8 *word = NULL;
    static u32 t0 = 0;
    if (!word) { word = getenv("DKE_PAUSE_TYPE"); t0 = ls_tick(); }
    if (!word) return 0;
    u32 step = (ls_tick() - t0) / 40;
    u32 n = (u32)strlen(word);
    if (step & 1) return 0;                        /* key up */
    step >>= 1;
    if (step < n) {
        c8 c = word[step];
        return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
    }
    return step == n ? 1 : 0;                      /* Enter once */
}

/* 0x40A280 check_keyboard_letter: the first letter key held (VK 'A'..'Z'),
 * else 1 when Enter is held, else 0. */
static i32 check_keyboard_letter(const system_t *sys) {
    i32 inj = injected_key();
    if (inj) return inj;
    for (i32 v = 'A'; v <= 'Z'; v++)
        if (sys->actions[v]) return v;
    return sys->actions[13] ? 1 : 0;
}

/* The cheat words (0x40A4A6..0x40A921). */
static void pause_cheat(const c8 *w) {
    if (!strcasecmp(w, "fortytwo")) {
        set_position_coordinates(300, 240);
        draw_text("EASY MODE ON");
        fx_present();
        ls_delay(1000);
        g_easy_mode_enabled = 1;
    }
    if (!strcasecmp(w, "oklimit") && g_easy_mode_enabled && state->debug_enabled) g_limit_override_flag = 0;
    if (!strcasecmp(w, "oksave") && g_easy_mode_enabled) g_easy_mode_enabled |= 2;
    if (!strcasecmp(w, "debug") && state->debug_enabled) g_debug_display_flag = 1;
    if (!strcasecmp(w, "bighead") && g_easy_mode_enabled) g_character_model_override = 1;
    if (!strcasecmp(w, "bigfoot") && g_easy_mode_enabled) g_character_model_override = 2;
    if (!strcasecmp(w, "baffe") && g_easy_mode_enabled) g_character_model_override = 3;
    if (!strcasecmp(w, "dwarf") && g_easy_mode_enabled) g_character_model_override = 4;
    if (!strcasecmp(w, "muchbetter") && g_easy_mode_enabled) g_character_model_override = 5;
    if (!strcasecmp(w, "normal") && g_easy_mode_enabled) {
        /* sub_46CE97 reloads the default shading table (not ported). */
        g_character_scale_factor = 1.0;
        g_character_model_override = 9;
    }
}

static void write_ppm(const c8 *path, const u32 *fb, i32 w, i32 h) {
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (i32 i = 0; i < w * h; i++) {
        u8 rgb[3] = { (u8)fb[i], (u8)(fb[i] >> 8), (u8)(fb[i] >> 16) };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

/* The blend of 0x40A2B0: the frozen frame's red channel (0..63, v52) and the
 * picture's grey (0..63, v55) go through a 64x64 table,
 *     lut[g + 64 * r] = (r * (64^0.4 - g^0.4) + g * g^0.4) / 64^0.4,
 * and the result picks the picture palette entry whose red equals it (v49). */
static void pause_blend(u32 *dst, const u32 *game, const bmap_t *img, i32 w, i32 h, i32 invert) {
    u8 pal[768];                                   /* v8: entry 0 is forced to black (v7[0..2] = 0) */
    memcpy(pal, img->palette, sizeof(pal));
    pal[0] = pal[1] = pal[2] = 0;
    i32 shift = 0;                                 /* 6-bit VGA palette or 8-bit */
    for (i32 i = 0; i < 768; i++) if (pal[i] > 63) { shift = 2; break; }
    i32 inv[64];                                   /* v49: red -> palette index */
    for (i32 i = 0; i < 64; i++) inv[i] = 0;
    for (i32 i = 0; i < 256; i++) inv[(pal[i * 3] >> shift) & 63] = i;
    inv[0] = 0;
    f64 k = pow(64.0, 0.4);
    static i32 lut[64 * 64];
    for (i32 g = 0; g < 64; g++) {
        f64 pg = pow((f64)g, 0.4);
        for (i32 r = 0; r < 64; r++) lut[g + 64 * r] = (i32)((r * (k - pg) + g * pg) / k);
    }
    u32 col[64];
    for (i32 v = 0; v < 64; v++) {
        const u8 *c = pal + inv[v] * 3;
        u32 r = c[0], g = c[1], b = c[2];
        if (shift == 0) { r = r * 255 / 63; g = g * 255 / 63; b = b * 255 / 63; }
        if (invert) { r = 255 - r; g = 255 - g; b = 255 - b; }
        col[v] = 0xFF000000u | r | (g << 8) | (b << 16);
    }
    for (i32 y = 0; y < h; y++) {
        for (i32 x = 0; x < w; x++) {
            i32 r6 = (game[y * w + x] & 0xFF) >> 2;
            i32 g6 = 0;
            if (x < img->width && y < img->height)
                g6 = (pal[img->pixels[y * img->stride + x] * 3] >> shift) & 63;
            dst[y * w + x] = col[lut[g6 + 64 * r6]];
        }
    }
}

/* 0x40A2B0 display_pause_screen */
static void display_pause_screen(void) {
    system_t *sys = &state->system;
    u32 *fb = state->graphic.front_buffer;
    i32 w = state->graphic.width, h = state->graphic.height;
    if (!fb) return;
    u32 *game = (u32 *)malloc((size_t)w * h * 4);
    u32 *shot = (u32 *)malloc((size_t)w * h * 4);
    if (!game || !shot) { free(game); free(shot); return; }
    memcpy(game, fb, (size_t)w * h * 4);

    /* PAUSED%d.GIF, ls_random(3). */
    bmap_t *img = NULL;
    i32 pick = (i32)(3u * (u32)(rand() & 0x7FFF) / 0x7FFFu);
    for (i32 t = 0; t < 3 && !img; t++) {
        c8 vp[64];
        snprintf(vp, sizeof(vp), "DATAS>GLOBAL>IMAGES>PAUSED%d.CIF", (pick + t) % 3);
        img = game_init_load_cursor(vp);
    }
    if (img) {
        /* The negative flashes for 50 ms, then the picture settles. */
        pause_blend(shot, game, img, w, h, 1);
        memcpy(fb, shot, (size_t)w * h * 4);
        fx_present();
        ls_delay(50);
        pause_blend(shot, game, img, w, h, 0);
        memcpy(fb, shot, (size_t)w * h * 4);
        fx_present();
    }

    i32 prev = -1, len = 0;
    c8 word[256];
    word[0] = 0;
    u32 opened = ls_tick(), redraw = opened;
    i32 shot_done = 0;
    i32 p_prev = sys->actions[80] != 0;            /* the key that opened us may still be down */
    while (!sys->quit) {
        ls_update_events(sys);
        u32 now = ls_tick();
        if (img && now - redraw > 3000) {          /* draw_image again every 3 s */
            redraw = now;
            memcpy(fb, shot, (size_t)w * h * 4);
        }
        i32 key = check_keyboard_letter(sys);
        if (key != prev) {
            if (key) {
                if (len <= 128) {
                    if (key == 1) {                /* Enter: judge the word */
                        word[len] = 0;
                        pause_cheat(word);
                        len = 0;
                        word[0] = 0;
                    } else if (len < 255) {
                        word[len++] = (c8)key;
                    }
                } else len = 0;
            }
            prev = key;
        }
        if (getenv("DKE_PAUSE_SHOT") && !shot_done && now - opened > 900) {
            write_ppm(getenv("DKE_PAUSE_SHOT"), fb, w, h);
            shot_done = 1;
        }
        fx_present();
        ls_delay(10);
        i32 p_now = sys->actions[80] != 0;
        i32 headless = getenv("DKE_PAUSE") &&
                       now - opened > (u32)(getenv("DKE_PAUSE_MS") ? atoi(getenv("DKE_PAUSE_MS")) : 1500);
        if ((p_now && !p_prev) || headless) break;
        p_prev = p_now;
    }
    if (img) ls_free_bmap(img);
    /* The engine redraws the world and HUD; the port puts the frozen frame back. */
    memcpy(fb, game, (size_t)w * h * 4);
    free(game);
    free(shot);
}

/* The P branch of process_input_command (0x407159..0x4075A8). */
void pause_game(void) {
    if (!state || !state->graphic.front_buffer) return;
    system_t *sys = &state->system;
    sfx_pause_all();                               /* ls_pause_sound x3 + sub_44D68C */
    game_time_freeze(50);                          /* set_display_mode(50) */
    /* apply_screen_gamma(0): gamma package. */
    while (!sys->quit && sys->actions[80] && !getenv("DKE_PAUSE")) ls_update_events(sys);
    display_pause_screen();
    /* apply_screen_gamma(1) */
    while (!sys->quit && sys->actions[80] && !getenv("DKE_PAUSE")) ls_update_events(sys);
    game_time_thaw(1, 50);                         /* restore_display_mode(1, 50) */
    sfx_resume_all();                              /* sub_44D6C9 + ls_resume_sound x3 */
}
