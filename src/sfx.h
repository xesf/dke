#ifndef DKE_SFX_H
#define DKE_SFX_H

/*
 * sfx.h — in-game sound effects (see sfx.c): the 50-entry sample cache,
 * positional one-shots, the room ambience loop, the timed event slots,
 * dialogue voices and animation-message sounds, mixed in software.
 */

#include "libsys/common.h"
#include "entity.h"
#include "libsys/audio/wav.h"

/* sub_46D305 / sub_448F4D: level sound setup (reverb from the .DAT). */
void sfx_level_begin(const c8 *root, i32 level, i32 reverb);
/* sub_449BAD: LIEU exit. */
void sfx_level_end(void);
/* sub_44968B: per-frame event slots, voice release and mixer pump. */
void audio_lock(void);        /* sfx/music state is shared with the mixer thread */
void audio_unlock(void);
void sfx_tick(void);
void sfx_pump(void);
void sfx_pause_all(void);     /* ls_pause_sound 0x4802B0 on every game voice */
void sfx_resume_all(void);    /* ls_resume_sound 0x480260 */          /* keep the mixer fed inside a blocking loop */
/* Menu sounds, mixed (not queued): the click is ls_play_sound(help_screen_context,
 * freq, 1); freq 0 = the WAV's rate, the menus confirm at 8000 Hz. `w` must
 * outlive the playback; ds_vol is in 1/100 dB before the master volume. */
void sfx_ui_click(i32 freq);
void sfx_ui_play(const wav_t *w, i32 freq, i32 ds_vol);
/* sub_46E883 tail: re-place the ambience after a camera switch. */
void sfx_camera_changed(void);
void sfx_set_master(i32 v);
void sfx_set_reverse_stereo(b8 on);
i32  sfx_get_master(void);
b8   sfx_get_reverse_stereo(void);

/* DIVERS op 4: event slot a = slot<<16 | ev, b = delay<<16 | jitter. */
void sfx_divers_event(i32 a, i32 b);
/* Bruitage action (case 0x0B): x, z in script units. */
void sfx_bruitage(const c8 *name, i32 x, i32 z);
/* sub_44E57B: voice / one-shot / ambience; returns a voice's length (ms). */
u32  sfx_voice_play(const entity_t *e, const c8 *name, i32 x, i32 z);
b8   sfx_voice_busy(void);
void sfx_voice_stop(void);
void sfx_free_slots(void);    /* sub_44E46A(0) + sub_44E46A(1) */
void sfx_play_anims(const c8 *name);
/* sub_41AED1 types 1 / 0x13; returns 1 when the caller skips to the next
 * message. */
i32  sfx_anim_message(const entity_t *e, i32 type, i32 p, i32 mtick);

/* Provided by scene.c. */
void sfx_hook_camera_origin(i32 origin[3]);
i32  sfx_hook_project(i32 wx, i32 wy, i32 wz, i32 *sx);

#endif /* DKE_SFX_H */
