#pragma once

/*
 * cjr.h — "big animations" (biganims/ANIM0n.CJR), played by LIEU op 17.
 *
 * A CJR file is an Autodesk FLC (640x480, 8 bpp). The original opens it as a
 * background layer (sub_454810), then every frame decodes the delta frames up
 * to the current one straight into the room backdrop (sub_454690, sub_454E00,
 * sub_454CB0). Frame 0 (palette + full image) is skipped: the deltas apply to
 * the backdrop of the room the animation was made from. Only chunk type 7
 * (DELTA_FLC, sub_4553B0) occurs in the shipped files; the other chunks are
 * skipped as in the original dispatcher.
 *
 * The animation runs at 15 frames per second and ends on its last frame,
 * which stays in the backdrop until the next camera load.
 */

#include "libsys/common.h"

/* sub_454810: start ANIM%02d.CJR under `game_path`. Returns 1 if it opened. */
i32  cjr_start(const c8 *game_path, i32 n, i32 now);

/* sub_454690: decode up to the current frame into `pixels` (w x h, 8 bpp).
 * Returns 1 when the backdrop changed. */
i32  cjr_tick(i32 now, u8 *pixels, i32 w, i32 h);

/* bonus_mode_active. */
b8   cjr_active(void);

/* update_game_timing_state: bonus_timer += time_delta after a modal screen. */
void cjr_shift_time(i32 delta);

void cjr_stop(void);
