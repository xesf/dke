#ifndef DKE_FX_H
#define DKE_FX_H

/*
 * fx.h -- screen-space combat effects: the 12 smoke puffs, the blood spray
 * and the 154-particle flame burst. All three are update_game_hud overlay
 * records in the original; here they are plain state drawn once a frame
 * straight onto the 32-bit front buffer, tested against the Z image.
 */

#include "libsys/common.h"
#include "libsys/graphics/graphic.h"

/* Level / room load: puffs freed, blood and burst stopped. */
void fx_reset(void);

/* play_sound_at_position @ 0x42CDA6 (really the blood spray): 32 particles
 * at world (x, y, z), thrown along heading `a4` (BAM). One at a time. */
void fx_blood_start(i32 x, i32 y, i32 z, i32 a4);

/* process_entity_random_transformations @ 0x411D09: 154-particle burst. */
void fx_flame_burst(i32 x, i32 y, i32 z);

/* initialize_entity_positions @ 0x411C27: a smoke puff near `pos` (+0..0x2EE0
 * jitter) living `life` ms. */
void fx_puff_flame(const i32 *pos, i32 life);

/* sub_42F476 @ 0x42F476: a 384 ms puff around `pos` (wall hit). */
void fx_puff_spark(const i32 *pos);

/* sub_430081 @ 0x430081 (underwater levels): once per frame, spawn and rise the 32
 * air bubbles around `player` (world x, y, z). Drawn by fx_draw. */
void fx_bubbles_tick(i32 now, const i32 *player);

/* sub_42F31B @ 0x42F31B: once per frame, animate and age the puffs. */
void fx_tick(i32 now);

/* update_game_hud cases 7, 10 and 11 (0x467C6F, 0x467C79): draw everything.
 * zimg is the Z image (640x480, bigger = nearer), pal the 768-byte palette,
 * grd the 64-step shading ramps (grd[colour * 64 + shade]). */
void fx_draw(u32 *fb, i32 fw, i32 fh, const bmap_t *zimg, const u8 *pal, const u8 *grd, i32 now);

/* Hooks into the renderer (scene.c). */
i32 scene_fx_project(i32 wx, i32 wy, i32 wz, i32 *sx, i32 *sy, i32 *cz);   /* sub_45ACC5 */
i32 scene_fx_zval(i32 wx, i32 wy, i32 wz);                                  /* calculate_model_scale @ 0x4207FA */
i32 scene_fx_floor(i32 x, i32 z);                                           /* generate_effect @ 0x457345 */

#endif /* DKE_FX_H */
