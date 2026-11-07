#pragma once

/*
 * envfx.h — per-camera environment tint and flash (lightning) palette effect
 * driven by the level .DAT file.
 *
 *   env tint   0x40C010 apply_palette_transformation_2: adds 2*(envred,
 *              envgreen, envblue)<camera> to every palette colour (6-bit
 *              units, clamped 0..63). Applied when the camera changes
 *              (sub_46E883).
 *   flash      0x46BE40 can_do_32k (per-camera set-up) and 0x42F565 (per
 *              frame): the palette is brightened by a flickering amount
 *              proportional to each colour's squared magnitude, tinted with
 *              flashred/green/blue. Runs only when a flash colour is nonzero.
 *
 * The port draws through many palettes into an RGB frame, so (as in gamma.h)
 * the per-colour functions are applied to the finished frame at blit time.
 */

#include "libsys/common.h"

#define ENVFX_MAX_CAMERAS 10      /* the original arrays hold 10 cameras */

typedef struct {
    i32 underwater;                    /* "underwater" key (render_mode_flag 1) */
    i32 ncamera;
    i32 env[ENVFX_MAX_CAMERAS][3];     /* envred/envgreen/envblue<N>          */
    i32 flash_present;                 /* "flash" key found                   */
    i32 flash_rgb[ENVFX_MAX_CAMERAS][3];
    i32 flash_freq[ENVFX_MAX_CAMERAS]; /* default 255                         */
    i32 flash_rand[ENVFX_MAX_CAMERAS]; /* default 255                         */
} envfx_config_t;

/* Level load: take the parsed DAT values (a zeroed config clears the effect). */
void envfx_configure(const envfx_config_t *cfg);

/* Camera changed (or first set at level load). Mirrors can_do_32k + tint. */
void envfx_set_camera(i32 camera);

/* Per frame, with the game clock in ms (0x42F565). */
void envfx_tick(i32 now);

/* render_mode_flag == 1: wavy full-screen warp (0x42FADF) and air bubbles
 * (0x430081, see fx_bubbles_tick). */
b8   envfx_underwater(void);

/* Is any of the effects active for the frame? */
b8   envfx_active(void);

/* Apply tint + flash to n pixels in place (byte 0 = R, 1 = G, 2 = B). */
void envfx_apply_pixels(u32 *px, i32 n);
