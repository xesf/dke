/*
 * sfx.c — in-game sound effects for the dkev.exe port.
 *
 * Mirrors (dkev.exe addresses):
 *   sub_46D305 +0x109..0x1DC  WAV dir choice: "%swav>normal>" / "%swav>reverb>"
 *                             (DAT key "reverb : ", dword_4F9420) + "%swav>anims>"
 *                             -> byte_4FB1F0 (sfx dir) / byte_4FC010 (anims dir)
 *   get_sound_id   @0x44D8EE  50-entry cache, key = upper("%s%s.wav", sfx dir, name)
 *   sub_44D7C8     @0x44D7C8  cache loader (first free slot)
 *   play_entity_sound @0x44DC63  one DS buffer per cache slot (dword_4D62C8[i*64]),
 *                             position-attenuated, pitch = base rate +/-2000 Hz, play once
 *   sub_44D488     @0x44D488  volume / pan from a world point
 *   sub_44CFF6     @0x44CFF6  master volume (dword_4B60B8, 0..64)
 *   sub_44D06F     @0x44D06F  pan, negated when byte_4B60A8 (reverse stereo)
 *   sub_44E57B     @0x44E57B  "Bruitage" / voice player: AMB loop, dialogue, one-shot
 *   sub_44D5BA     @0x44D5BA  re-place the AMB loop (camera change, volume change)
 *   sub_446E19 case 3 (DIVERS op 4)  timed event sound slots (unk_4FF0B0, 10 x 0x110)
 *   sub_44968B     @0x44968B  per-frame event-slot driver + voice slot release
 *   sub_449BAD     @0x449BAD  LIEU exit: free slots, voice, AMB
 *   sub_41AED1     @0x41AED1  animation messages type 1 / 0x13 (sound)
 *   sub_414A0A / sub_414AC2   floor material under the contact foot
 *   sub_449589     @0x449589  "mmmm%d_L|o" breathing/grunt
 *
 * Backend: the port's ls_audio_push() is a FIFO of whole buffers (no mixing),
 * so this module owns a small software mixer and streams ~16-90 ms of mixed
 * 22050 Hz stereo i16 each frame. Dialogue (dlg_start) must move to
 * sfx_voice_play() so it shares the mixer — and, as in the original, voice
 * slot 0 with Bruitage one-shots.
 *
 * Hooks the integrator must provide (not in the port yet):
 *   sfx_hook_camera_origin(o)   transformation_origin = -camera eye (scene.c g_proj_eye negated)
 *   sfx_hook_project(x,y,z,&sx) sub_45ACC5: screen x of a world point; returns
 *                               1 if in front of the camera (scene_project_world)
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>

#include "libsys/common.h"
#include "libsys/audio/audio.h"
#include "libsys/audio/wav.h"
#include "state.h"
#include "entity.h"
#include "can.h"
#include "actor.h"
#include "world.h"
#include "libsys/file_system.h"

extern state_t *state;

#include "sfx.h"
#include "gamecfg.h"
#include "music.h"

/* ========================================================================
 * Constants
 * ======================================================================== */
#define SFX_RATE          22050
#define SFX_CACHE         50      /* dword_4FB380[50], byte_4FB450[50][60] */
#define SFX_EV_SLOTS      10      /* unk_4FF0B0, stride 0x110 */
#define SFX_INST_VOICE    0       /* renderer_instance[0]  (dialogue / one-shot) */
#define SFX_INST_AMB      1       /* renderer_instance[1] == dword_4FB334 (AMB loop) */
#define SFX_UI_SLOTS      4       /* menu clicks and the sound-screen test sample */
#define SFX_VOICES        (SFX_CACHE + 2 + SFX_EV_SLOTS + SFX_UI_SLOTS)
#define SFX_LEAD_FRAMES   2048    /* ~93 ms queued ahead of the device */
#define SFX_MAX_CHUNK     4096
#define SFX_MIN_CHUNK     256     /* ~12 ms minimum refill */
#define SFX_PUSH_CHUNK    512     /* ~23 ms per device buffer: the lead spans
                                   * several, so one finishing never empties it */

#define DS_VOL_MIN        (-10000)  /* 0xFFFFD8F0 */
#define POS_NONE          (-600)    /* 0xFFFFFDA8: x or z == -1 tile -> not placed */

/* ========================================================================
 * Mixer
 * ======================================================================== */
typedef struct {
    const wav_t *w;
    u64  pos;        /* 16.16 source frame; 32-bit wraps on long voice WAVs */
    u32  step;       /* 16.16 source frames per output frame */
    i32  gl, gr;     /* Q12 gains */
    i32  rate;       /* playback rate (Hz) */
    i32  vol, pan;   /* DirectSound units (1/100 dB) */
    b8   loop;
    b8   active;
    b8   held;       /* ls_pause_sound 0x4802B0: stopped but still "playing" */
} sfx_voice_t;

static sfx_voice_t g_v[SFX_VOICES];
static u32 g_clock_t0;       /* ms at which g_pushed frames started */
static u32 g_pushed;         /* frames pushed since t0 */
static b8  g_streaming;

static i32 g_master = 0x40;  /* dword_4B60B8 (options slider 0..64) */
static b8  g_reverse;        /* byte_4B60A8 */

static i32 ds_to_q12(i32 ds) {
    if (ds <= DS_VOL_MIN) return 0;
    if (ds >= 0) return 4096;
    return (i32)(4096.0 * pow(10.0, (f64)ds / 2000.0));
}

static void voice_apply(sfx_voice_t *v) {
    i32 g = ds_to_q12(v->vol);
    i32 l = g, r = g;
    if (v->pan < 0) r = (i32)((i64)g * ds_to_q12(v->pan) >> 12);   /* DS: pan<0 cuts right */
    if (v->pan > 0) l = (i32)((i64)g * ds_to_q12(-v->pan) >> 12);
    v->gl = l; v->gr = r;
}

/* ls_play_sound(h, freq, count): stops first, count==0 -> DSBPLAY_LOOPING. */
static void voice_play(i32 vi, const wav_t *w, i32 freq, i32 count) {
    sfx_voice_t *v = &g_v[vi];
    if (!w || !w->samples || !w->num_samples) { v->active = 0; return; }
    if (getenv("DKE_SFXDBG")) print("sfx: voice %d play %u samples @%d loop=%d t=%u\n", vi, w->num_samples, (i32)w->sample_rate, count == 0, ls_tick());
    v->w = w;
    v->pos = 0;
    v->rate = freq ? freq : (i32)w->sample_rate;
    if (v->rate < 100) v->rate = 100;
    v->step = (u32)(((u64)v->rate << 16) / SFX_RATE);
    v->loop = (count == 0);
    v->active = 1;
    voice_apply(v);
}
static void voice_stop(i32 vi)                 { g_v[vi].active = 0; g_v[vi].held = 0; }
static b8   voice_playing(i32 vi)              { return g_v[vi].active; }  /* ls_get_sound_status==1 */
static void voice_volume(i32 vi, i32 ds)       { g_v[vi].vol = ds; voice_apply(&g_v[vi]); }
static void voice_pan(i32 vi, i32 pan)         { g_v[vi].pan = g_reverse ? -pan : pan; voice_apply(&g_v[vi]); }

static void mix(i16 *out, u32 frames) {
    static i32 acc[SFX_MAX_CHUNK * 2];
    memset(acc, 0, frames * 2 * sizeof(i32));
    for (i32 i = 0; i < SFX_VOICES; i++) {
        sfx_voice_t *v = &g_v[i];
        if (!v->active || v->held) continue;
        const wav_t *w = v->w;
        for (u32 f = 0; f < frames; f++) {
            u32 sf = (u32)(v->pos >> 16);
            if (sf >= w->num_samples) {
                if (!v->loop) { v->active = 0; break; }
                v->pos -= (u64)w->num_samples << 16;
                sf = (u32)(v->pos >> 16);
            }
            i32 l, r;
            if (w->channels == 2) { l = w->samples[sf * 2]; r = w->samples[sf * 2 + 1]; }
            else                  { l = r = w->samples[sf]; }
            acc[f * 2]     += (l * v->gl) >> 12;
            acc[f * 2 + 1] += (r * v->gr) >> 12;
            v->pos += v->step;
        }
    }
    music_mix(acc, frames);            /* its own DS buffer at 0 dB in the original */
    for (u32 i = 0; i < frames * 2; i++) {
        i32 s = acc[i];
        out[i] = (i16)(s > 32767 ? 32767 : s < -32768 ? -32768 : s);
    }
}

/* Called once per frame. Pushes only while something plays, so menu
 * sfx_play() and the movie decoder keep the queue to themselves otherwise. */
/* All sfx/music state is shared with the mixer thread: every public entry
 * point (sfx_* / music_*) takes this lock; the thread takes it per chunk. */
static pthread_mutex_t g_audio_mx;
static pthread_once_t  g_audio_once = PTHREAD_ONCE_INIT;
static void audio_mx_init(void) {
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_audio_mx, &a);
}
void audio_lock(void)   { pthread_once(&g_audio_once, audio_mx_init); pthread_mutex_lock(&g_audio_mx); }
void audio_unlock(void) { pthread_mutex_unlock(&g_audio_mx); }

/* One mixing step (mixer thread, lock held). Keeps SFX_LEAD_FRAMES queued
 * ahead of the device. When the backend reports its queue, that is the
 * measure: the sound card's clock drifts from ls_tick, and pacing by the
 * wall clock alone slowly drained the queue (crackle) or overfilled it.
 * Otherwise the wall clock stands in (the game clock stops while the
 * inventory / a pause is up). */
static void mixer_pump(u32 now) {
    if (!state || !state->audio_enabled) return;
    now = ls_tick();
    b8 any = 0;
    for (i32 i = 0; i < SFX_VOICES && !any; i++) any = g_v[i].active;
    any = any || music_is_playing();
    if (!any) { g_streaming = 0; return; }
    if (!g_streaming) { g_streaming = 1; g_clock_t0 = now; g_pushed = 0; }
    /* Frames missing from the lead. The device queue may also hold audio
     * that is not ours (a movie's tail, a menu click): it counts too. */
    i32 need;
    i32 queued = ls_audio_queued(&state->audio);
    if (queued >= 0) {
        need = SFX_LEAD_FRAMES - queued / 4;
    } else {
        u32 played = (u32)((u64)(now - g_clock_t0) * SFX_RATE / 1000);
        if (g_pushed < played) g_pushed = played;          /* underrun: resync */
        need = (i32)(played + SFX_LEAD_FRAMES - g_pushed);
    }
    while (need >= SFX_MIN_CHUNK) {
        i32 n = need > SFX_PUSH_CHUNK ? SFX_PUSH_CHUNK : need;
        static i16 buf[SFX_MAX_CHUNK * 2];
        mix(buf, (u32)n);
        if (ls_audio_push(&state->audio, buf, n * 4) <= 0) break;
        g_pushed += (u32)n;
        need -= n;
    }
}

/* The mixer runs on its own thread so a slow frame, a room load or a
 * blocking screen cannot starve the output queue. */
static void *mixer_thread(void *arg) {
    (void)arg;
    for (;;) {
        audio_lock();
        mixer_pump(0);
        audio_unlock();
        usleep(6000);
    }
    return NULL;
}
static void mixer_thread_start(void) {
    static b8 started = 0;
    if (started) return;
    started = 1;
    pthread_t t;
    if (pthread_create(&t, NULL, mixer_thread, NULL) == 0) pthread_detach(t);
}

/* ========================================================================
 * Helpers
 * ======================================================================== */
static u32 g_seed = 0x1234567;
static i32 rnd(i32 n) {                 /* ls_random(n): [0, n) */
    g_seed = g_seed * 1103515245u + 12345u;
    return n > 0 ? (i32)((g_seed >> 16) % (u32)n) : 0;
}
static u32 now_ms(void) { return state ? state->current_game_time : 0; }

static void upper(c8 *s) { for (; *s; s++) *s = (c8)toupper((u8)*s); }

/* "DATAS>SOUND>X.WAV" style -> host path under the game root. */
static c8 g_root[256] = ".";
static c8 g_dir_sfx[300];     /* byte_4FB1F0: <root>/wav/NORMAL/ or /wav/REVERB/ */
static c8 g_dir_anims[300];   /* byte_4FC010: <root>/wav/ANIMS/ */
static i32 g_level = 1;       /* world_state */

/* sub_44CFF6 */
static i32 master_vol(i32 v) {
    if (g_master < 0 || g_master > 0x40) return DS_VOL_MIN;
    if (g_master == 0) return DS_VOL_MIN;
    return v - (0x40 - g_master) * 32;
}

/* sub_44D488: world point -> (DS volume, DS pan). Volume falls 1/100 dB per
 * 600 world units (= one script unit), clamped to [-1000, -80]; pan maps the
 * projected screen x 0..640 to -1000..+1000 (mirrored when behind the camera). */
static void place(i32 x, i32 y, i32 z, i32 *vol, i32 *pan) {
    i32 o[3];
    sfx_hook_camera_origin(o);
    f64 dx = (f64)(o[0] + x), dy = (f64)(o[1] + y), dz = (f64)(o[2] + z);
    i32 d = (i32)-sqrt(dx * dx + dy * dy + dz * dz);
    i32 v = -abs(d / 600);
    if (v > -80)    v = -80;
    if (v < -10000) v = -10000;
    i32 sx = 320;
    i32 front = sfx_hook_project(x, y, z, &sx);
    if (!front) sx = 640 - sx;
    *pan = sx * 2000 / 640 - 1000;
    /* Port: an off-screen point (or one culled at the near plane) projects
     * far outside 0..640; hold it at the screen edge. */
    if (*pan < -1000) *pan = -1000;
    if (*pan >  1000) *pan =  1000;
    if (v < -1000) v = -1000;
    *vol = v;
}

/* ========================================================================
 * Sample cache — get_sound_id / sub_44D7C8
 * ======================================================================== */
typedef struct { c8 key[400]; wav_t w; b8 used; } sfx_entry_t;
static sfx_entry_t g_cache[SFX_CACHE];

static void cache_path(c8 *out, i32 n, const c8 *dir, const c8 *name) {
    snprintf(out, (size_t)n, "%s%s.wav", dir, name);
    upper(out + strlen(dir));                       /* host fs: keep dir case */
}

/* Returns cache index or -1 (missing file). Like the original, the key is the
 * full upper-cased path, so "fend1a" from NORMAL and REVERB never collide. */
static i32 sfx_get_id(const c8 *name, const c8 *dir) {
    if (!state || !state->audio_enabled || !name || !name[0]) return -1;
    c8 path[400];
    cache_path(path, sizeof(path), dir, name);
    const c8 *key = path;
    for (i32 i = 0; i < SFX_CACHE; i++)
        if (g_cache[i].used && strcmp(g_cache[i].key, key) == 0) return i;
    if (!ls_exists(path)) return -1;
    for (i32 tries = 0; tries < 20; tries++) {
        for (i32 i = 0; i < SFX_CACHE; i++) {
            if (g_cache[i].used) continue;
            if (!wav_load(&g_cache[i].w, path)) return -1;
            snprintf(g_cache[i].key, sizeof(g_cache[i].key), "%s", key);
            g_cache[i].used = 1;
            return i;
        }
        /* Full: drop entry 0 and shift down (original's memory-pressure path). */
        voice_stop(0);
        wav_free(&g_cache[0].w);
        memmove(&g_cache[0], &g_cache[1], sizeof(g_cache[0]) * (SFX_CACHE - 1));
        memmove(&g_v[0], &g_v[1], sizeof(g_v[0]) * (SFX_CACHE - 1));
        memset(&g_cache[SFX_CACHE - 1], 0, sizeof(g_cache[0]));
        memset(&g_v[SFX_CACHE - 1], 0, sizeof(g_v[0]));
    }
    return -1;
}

/* play_entity_sound @0x44DC63: one voice per cached sample (a retrigger
 * restarts it), placed at the entity root (+0x50 -> +0x3C) or centred. */
static void sfx_play_entity(const entity_t *e, i32 id) {
    if (id < 0 || id >= SFX_CACHE || !g_cache[id].used) return;
    if (e && EROOT(e)) {
        const obc_node_t *r = EROOT(e);
        i32 vol, pan;
        place(r->pos[0], r->pos[1], r->pos[2], &vol, &pan);
        voice_volume(id, master_vol(vol));
        voice_pan(id, pan);
    } else {
        voice_volume(id, master_vol(0));
    }
    i32 freq = (i32)g_cache[id].w.sample_rate + rnd(4000) - 2000;
    voice_play(id, &g_cache[id].w, freq, 1);
}

/* ========================================================================
 * Voice slots — renderer_instance[0] (dialogue / one-shot), [1] (AMB loop)
 * ======================================================================== */
#define VI_INST(i)  (SFX_CACHE + (i))
static wav_t g_inst[2];
static b8    g_inst_used[2];
static b8    g_inst_dialog;     /* slot 0 holds a dialogue voice (not a one-shot) */
static i32   g_amb_pos[3] = { POS_NONE, 0, POS_NONE };   /* dword_4FB34C/50/54 */
static c8    g_amb_name[32];                             /* byte_4FC090 */
static i32   g_vox_pos[3];      /* dialogue voice's speaker point (port) */
static b8    g_vox_placed;

static void inst_free(i32 i) {                           /* sub_44E46A */
    if (i == SFX_INST_VOICE) g_inst_dialog = 0;
    voice_stop(VI_INST(i));
    wav_free(&g_inst[i]);
    g_inst_used[i] = 0;
}
static b8 inst_load(i32 i, const c8 *path) {
    if (g_inst_used[i]) inst_free(i);
    if (!wav_load(&g_inst[i], path)) return 0;
    g_inst_used[i] = 1;
    return 1;
}

/* sub_44D5BA(1) — re-place the AMB loop. */
static void amb_place(void) {
    if (!g_inst_used[SFX_INST_AMB]) return;
    if (g_amb_pos[0] == POS_NONE || g_amb_pos[2] == POS_NONE) {
        voice_volume(VI_INST(SFX_INST_AMB), master_vol(0));
        return;
    }
    i32 vol, pan;
    place(g_amb_pos[0], g_amb_pos[1], g_amb_pos[2], &vol, &pan);
    voice_volume(VI_INST(SFX_INST_AMB), master_vol(vol));
    voice_pan(VI_INST(SFX_INST_AMB), pan);
}

/* Place the dialogue voice at its speaker. The original places it once at
 * the line's start; the port also re-places it after a camera cut so the
 * pan follows the new view. */
static void vox_place(void) {
    if (!g_inst_dialog || !g_inst_used[SFX_INST_VOICE]) return;
    i32 vol = POS_NONE, pan = 0;                          /* -600: unplaced voice */
    if (g_vox_placed) place(g_vox_pos[0], g_vox_pos[1], g_vox_pos[2], &vol, &pan);
    voice_volume(VI_INST(SFX_INST_VOICE), master_vol(vol));
    voice_pan(VI_INST(SFX_INST_VOICE), pan);
    if (getenv("DKE_SFXDBG")) print("sfx: voice at (%d,%d,%d) vol=%d pan=%d\n", g_vox_pos[0], g_vox_pos[1], g_vox_pos[2], vol, pan);
}

static const c8 *const k_lcod[13] = {          /* aLcod06, stride 13 */
    "LCOD06","LCOD07","LCOD09","LCOD10","LCOD12","LCOD13","LCOD18",
    "LCOD20","LCOD26","LCOD75","LCOD76","LCOD78","LCOD79",
};

/* sub_44E57B(entity, name, x, z) — x, z in script units (world / 600).
 * Returns the voice length in ms for dialogue (0 otherwise), for dlg timers. */
static u32 sfx_voice_play_impl(const entity_t *e, const c8 *name_in, i32 x, i32 z) {
    if (!state || !state->audio_enabled || !name_in) return 0;
    c8 name[64], path[512];
    snprintf(name, sizeof(name), "%s", name_in);
    upper(name);

    /* 1. "xxxAMB*": room ambience, looped, positional (renderer_instance[1]). */
    if (strlen(name) >= 6 && !strncmp(name + 3, "AMB", 3)) {
        snprintf(g_amb_name, sizeof(g_amb_name), "%s", name);
        g_amb_pos[0] = x * 600;
        g_amb_pos[2] = z * 600;
        if (x != -1 && z != -1)
            g_amb_pos[1] = actor_floor_height(g_amb_pos[0], g_amb_pos[2], NULL);  /* generate_effect */
        snprintf(path, sizeof(path), "%s/datas/sound/%s.WAV", g_root, name);
        if (!ls_exists(path)) return 0;
        if (!inst_load(SFX_INST_AMB, path)) return 0;
        voice_volume(VI_INST(SFX_INST_AMB), master_vol(0));
        voice_play(VI_INST(SFX_INST_AMB), &g_inst[SFX_INST_AMB], 0, 0);   /* count 0 = loop */
        amb_place();
        return 0;
    }

    /* 2. "LxxD*": dialogue voice, positional, slot 0 (always replaces). */
    if (name[0] == 'L' && name[3] == 'D') {
        inst_free(SFX_INST_VOICE);
        for (i32 i = 0; i < 13; i++)
            if (!strcasecmp(name, k_lcod[i])) {
                if (rnd(2)) { strcat(name, "A"); }     /* alternate take */
                break;
            }
        /* HDD layout: DIALOG>LCO>%s.WAV (+ "R"-prefixed reverb take when the
         * level has reverb) for "?C..." names, else DIALOG>%.3s>%s.WAV.
         * The port's dialog tree is <root>/dialog/Lnn/<code>.WAV. */
        snprintf(path, sizeof(path), "%s/dialog/%.3s/%s.WAV", g_root, name, name);
        if (!g_gamecfg.voices) return 0;              /* byte_4B60B0 off: text only */
        if (!ls_exists(path) || !inst_load(SFX_INST_VOICE, path)) return 0;
        g_inst_dialog = 1;
        voice_play(VI_INST(SFX_INST_VOICE), &g_inst[SFX_INST_VOICE], 0, 1);
        /* Original: y = world_controller+0x1E0 (default floor). The port's
         * renderer draws an actor at its root node (entity+0x50) position,
         * so the voice takes that node's y; the default floor is in another
         * space and put every voice behind the camera. */
        i32 fy = e && EROOT(e) ? EROOT(e)->pos[1]
               : g_world.world_controller ? *(const i32 *)((const u8 *)g_world.world_controller + 0x1E0) : 0;
        g_vox_pos[0] = x * 600; g_vox_pos[1] = fy; g_vox_pos[2] = z * 600;
        g_vox_placed = (x != 0 || z != 0);
        vox_place();
        (void)e;  /* original: entity+0xD90 = start time, +0xD8C/+0xD94 lip-sync */
        const wav_t *w = &g_inst[SFX_INST_VOICE];
        return w->sample_rate ? (u32)((u64)w->num_samples * 1000 / w->sample_rate) : 0;
    }

    /* 3. Anything else (L01B07, PORTE3, ...): one-shot in slot 0, NOT positional
     *    (x, z ignored), dropped while a voice / one-shot is still playing. */
    if (g_inst_used[SFX_INST_VOICE]) return 0;
    snprintf(path, sizeof(path), "%s/datas/sound/%s.WAV", g_root, name);
    if (!ls_exists(path) || !inst_load(SFX_INST_VOICE, path)) return 0;
    voice_play(VI_INST(SFX_INST_VOICE), &g_inst[SFX_INST_VOICE], 0, 1);
    voice_volume(VI_INST(SFX_INST_VOICE), master_vol(0));
    (void)e;
    return 0;
}

u32 sfx_voice_play(const entity_t *e, const c8 *name_in, i32 x, i32 z) {
    audio_lock();
    u32 r = sfx_voice_play_impl(e, name_in, x, z);
    audio_unlock();
    return r;
}

/* Bruitage action (LIEU slot 11 sub_44569A -> dispatcher case 0x0B).
 * cb_place_95C(name, x, z): 'Z'-prefixed names go to sub_44D23D, whose body
 * is dead code (jmp to epilogue) — a no-op, and no action is queued.
 * Otherwise the action plays at once with entity = NULL and completes. */
static void sfx_bruitage_impl(const c8 *name, i32 x, i32 z) {
    if (!name || name[0] == 'Z') return;
    sfx_voice_play(NULL, name, x, z);
}

void sfx_bruitage(const c8 *name, i32 x, i32 z) {
    audio_lock();
    sfx_bruitage_impl(name, x, z);
    audio_unlock();
}

/* ========================================================================
 * Event slots — DIVERS(4, slot<<16 | ev, (delay<<16) | jitter)
 * ======================================================================== */
typedef struct {
    c8    path[256];    /* +0x000 */
    i32   next;         /* +0x100 next start time (0 = slot unused) */
    i32   timing;       /* +0x104 hi16 delay ms, lo16 jitter ms */
    wav_t w;            /* +0x108 handle */
    b8    loaded;
    b8    playing;      /* +0x10C */
} sfx_ev_t;
static sfx_ev_t g_ev[SFX_EV_SLOTS];
#define VI_EV(i)  (SFX_CACHE + 2 + (i))

static i32 ev_next(i32 timing) {
    return (i32)now_ms() + (timing >> 16) - rnd(timing & 0xFFFF);
}

/* sub_446E19 case 3 (port layer_op(4, a, b)). */
static void sfx_divers_event_impl(i32 a, i32 timing) {
    if (!state || !state->audio_enabled) return;
    i32 slot = a >> 16, ev = a & 0xFFFF;
    if (slot < 0 || slot >= SFX_EV_SLOTS) return;
    sfx_ev_t *s = &g_ev[slot];
    if (!s->loaded && s->next == 0) {
        snprintf(s->path, sizeof(s->path), "%s/datas/sound/L%02dEV%01d.WAV", g_root, g_level, ev);
        s->timing = timing;
        s->next   = ev_next(timing);
        s->loaded = ls_exists(s->path) && wav_load(&s->w, s->path);
    } else if (timing == 0 && s->loaded) {
        voice_stop(VI_EV(slot));   /* only cuts the current playback; slot re-arms */
    }
}

void sfx_divers_event(i32 a, i32 timing) {
    audio_lock();
    sfx_divers_event_impl(a, timing);
    audio_unlock();
}

/* ========================================================================
 * Animation messages — sub_41AED1 type 1 (always) / 0x13 (50 %)
 * ======================================================================== */
static const c8 k_names[64][9] = {   /* 9-byte table at aFend1a (0x4B42C0 file 0xB42C0) */
    "fend1a","fend2a","fend1b","fend2b","fend1c","fend2c","fend1d","fend2d","fend1e","fend2e",
    "pas1a","pas2a","pas1b","pas2b","pas1c","pas2c","chute1","chute2","chute3","bp",
    "vide","objet1","objet2","objet3","impacta","impactb","impactc","impactd","impacte",
    "atak1a","atak2a","atak1b","atak2b","atak1c","atak2c","atak1d","atak2d",
    "aie1a","aie2a","aie1b","aie2b","aie1c","aie2c","aie1d","aie2d",
    "deada","deadb","deadc","deadd","block1","block2","rengaine","degaine",
    "crac1","mitr","lfeu1","357m","powfoire","pas1d","pas2d","powshkr","cri1","cri2","cri3",
};

static u32 g_cut_time;        /* last_event_check_time: set on camera change */

/* sub_414AC2: bone of the foot-contact (type 0) message nearest `tick`
 * (cyclic over the clip duration). */
static i32 contact_bone(const can_rec_t *r, i32 tick) {
    i32 dur = CAN_DURATION(r), best = 9999999, bone = -1;
    for (i32 i = 0; i < CAN_NMES(r); i++) {
        const u8 *m = r->mes[i];
        if (!m || m[1] != 0) continue;
        i32 d = abs(*(const i32 *)(m + 4) - tick);
        if (d > dur / 2) d = dur - d;
        if (d < best) { best = d; bone = *(const i32 *)(m + 8); }
    }
    return bone;
}

/* sub_414A0A: floor zone under that foot; material = flags >> 24 when
 * flags & 8, else 1. (L01: every zone -> 1.) */
static i32 foot_material(const entity_t *e, i32 tick) {
    const can_rec_t *r = (const can_rec_t *)EPTR(e, 0xD50);
    i32 b = r ? contact_bone(r, tick) : -1;
    void *zone = g_world.world_controller;
    if (b >= 0 && b < ENTITY_BONE_SLOTS && EBONE(e, b)) {
        const obc_node_t *n = EBONE(e, b);
        actor_floor_zone(n->raw_88[2], n->raw_88[4], e, &zone);   /* node +0x90 / +0x98 */
    }
    u32 fl = zone ? *(const u32 *)zone : 0;
    return (fl & 8) ? (i32)(fl >> 24) : 1;
}

/* sub_449589: breathing / grunt on the player, at most every 500 ms.
 * health_state = game_death_state (0..3), alt = special_mode_active. */
void sfx_breath(const entity_t *player, i32 health_state, b8 alt) {
    static u32 last;
    u32 now = now_ms();
    if ((i32)(now - last) < 500) return;
    last = now;
    i32 k = (health_state <= 1) ? 1 : health_state;
    c8 nm[16];
    snprintf(nm, sizeof(nm), alt ? "mmmm%01d_L" : "mmmm%01d_o", k + rnd(2));
    sfx_play_entity(player, sfx_get_id(nm, g_dir_sfx));
}

/* Hook for actor_move.c anim_messages(): call for type 1 and 0x13.
 * Returns 1 when the caller must `continue` (the original jumps straight to
 * the next message after an "A%03d" sound or a suppressed footstep). */
static i32 sfx_anim_message_impl(const entity_t *e, i32 type, i32 p, i32 mtick) {
    if (type != 1 && type != 0x13) return 0;
    if (type == 0x13 && rnd(2) != 1) return 0;
    if (p == -1) return 0;
    if ((i32)(now_ms() - g_cut_time) <= 0x32) return 0;

    if (p < 0) {                                   /* "A%03d" from wav/ANIMS */
        c8 nm[16];
        snprintf(nm, sizeof(nm), "A%03d", -(p + 2));
        sfx_play_entity(e, sfx_get_id(nm, g_dir_anims));
        return 1;
    }
    if (p < 10) p = (p / 2) * 2 + rnd(2);           /* fend1x / fend2x pair */
    if (p > 0x66) { sfx_breath(e, 0, 0); return 1; } /* caller should pass real state */
    if (p >= 64) return 0;                          /* table overrun: garbage in exe */

    c8 nm[24];
    snprintf(nm, sizeof(nm), "%s", k_names[p]);
    if (nm[0] == 'p' && nm[1] == 'a') {            /* footstep: pasNx_<material> */
        if ((i32)(now_ms() - (u32)E32(e, 0xD2C)) < 15) return 1;   /* walk just started */
        c8 suf[8];
        snprintf(suf, sizeof(suf), "_%d", foot_material(e, mtick));
        strcat(nm, suf);
    }
    if (nm[0] == 'b' && nm[1] == 'p') {            /* bp<level>, fallback bp00 */
        c8 t[24];
        snprintf(t, sizeof(t), "%s%02d", nm, g_level);
        snprintf(nm, sizeof(nm), "%s", t);
        if (sfx_get_id(nm, g_dir_sfx) < 0) snprintf(nm, sizeof(nm), "bp00");
    }
    sfx_play_entity(e, sfx_get_id(nm, g_dir_sfx));
    return 0;
}

i32 sfx_anim_message(const entity_t *e, i32 type, i32 p, i32 mtick) {
    audio_lock();
    i32 r = sfx_anim_message_impl(e, type, p, mtick);
    audio_unlock();
    return r;
}

/* ========================================================================
 * Lifecycle
 * ======================================================================== */

/* sub_46D305 (dirs) + sub_448F4D (memset unk_4FF0B0) + sub_44D706 (cache). */
static void sfx_level_begin_impl(const c8 *root, i32 level, i32 reverb) {
    snprintf(g_root, sizeof(g_root), "%s", root ? root : ".");
    g_level = level;
    snprintf(g_dir_sfx,   sizeof(g_dir_sfx),   "%s/wav/%s/", g_root, reverb ? "REVERB" : "NORMAL");
    snprintf(g_dir_anims, sizeof(g_dir_anims), "%s/wav/ANIMS/", g_root);
    for (i32 i = 0; i < SFX_EV_SLOTS; i++) {
        voice_stop(VI_EV(i));
        if (g_ev[i].loaded) wav_free(&g_ev[i].w);
    }
    memset(g_ev, 0, sizeof(g_ev));
    for (i32 i = 0; i < SFX_CACHE; i++) {
        voice_stop(i);
        if (g_cache[i].used) wav_free(&g_cache[i].w);
    }
    memset(g_cache, 0, sizeof(g_cache));
    g_cut_time = now_ms();
}

void sfx_level_begin(const c8 *root, i32 level, i32 reverb) {
    audio_lock();
    sfx_level_begin_impl(root, level, reverb);
    audio_unlock();
}

/* sub_449BAD: LIEU exit. */
static void sfx_level_end_impl(void) {
    for (i32 i = 0; i < SFX_EV_SLOTS; i++) {
        voice_stop(VI_EV(i));
        if (g_ev[i].loaded) wav_free(&g_ev[i].w);
        g_ev[i].loaded = 0;
    }
    inst_free(SFX_INST_VOICE);
    inst_free(SFX_INST_AMB);
}

void sfx_level_end(void) {
    audio_lock();
    sfx_level_end_impl();
    audio_unlock();
}

/* sub_46E883 tail: camera switched. */
static void sfx_camera_changed_impl(void) {
    g_cut_time = now_ms();
    amb_place();
    vox_place();
}

void sfx_camera_changed(void) {
    audio_lock();
    sfx_camera_changed_impl();
    audio_unlock();
}

/* Options slider (sub_440B92): 0..64, then re-place AMB. */
static void sfx_set_master_impl(i32 v) {
    g_master = v < 0 ? 0 : v > 0x40 ? 0x40 : v;
    amb_place();
}

void sfx_set_master(i32 v) {
    audio_lock();
    sfx_set_master_impl(v);
    audio_unlock();
}
static void sfx_set_reverse_stereo_impl(b8 on) { g_reverse = on; }
i32 sfx_get_master(void)         { return g_master; }
b8  sfx_get_reverse_stereo(void) { return g_reverse; }

void sfx_set_reverse_stereo(b8 on) {
    audio_lock();
    sfx_set_reverse_stereo_impl(on);
    audio_unlock();
}

/* sub_44968B: per-frame. */
void sfx_tick(void) {
    audio_lock();
    u32 now = now_ms();
    for (i32 i = 0; i < SFX_EV_SLOTS; i++) {
        sfx_ev_t *s = &g_ev[i];
        if (s->next == 0) continue;
        if (!s->playing) {
            if ((i32)now > s->next) {
                voice_play(VI_EV(i), s->loaded ? &s->w : NULL, 0, 1);
                voice_volume(VI_EV(i), master_vol(0));
                s->playing = 1;
            }
        } else if (!voice_playing(VI_EV(i))) {
            s->next = ev_next(s->timing);
            s->playing = 0;
        }
    }
    /* renderer_instance[0] finished -> release (unblocks Bruitage one-shots). */
    if (g_inst_used[SFX_INST_VOICE] && !voice_playing(VI_INST(SFX_INST_VOICE)))
        inst_free(SFX_INST_VOICE);
    audio_unlock();
    mixer_thread_start();
}

/* ls_pause_sound 0x4802B0 / ls_resume_sound 0x480260 over every game voice
 * (sub_44D68C / sub_44D6C9 walk the 50 cached sounds; the P key also pauses
 * renderer_instance[0], [1] == dword_4FB334 and the event slots). The DS
 * buffer is Stopped (position kept) and later Played again; a held voice
 * stays "playing" so nothing waiting on it is released. The menu click voices
 * (the last SFX_UI_SLOTS) and the music player are left alone. */
void sfx_pause_all(void) {
    audio_lock();
    for (i32 i = 0; i < SFX_VOICES - SFX_UI_SLOTS; i++)
        if (g_v[i].active) g_v[i].held = 1;
    audio_unlock();
}
void sfx_resume_all(void) {
    audio_lock();
    for (i32 i = 0; i < SFX_VOICES; i++) g_v[i].held = 0;
    audio_unlock();
}

/* Kept for loops that block the frame: the mixer thread feeds the device. */
void sfx_pump(void) { }

/* Menu sounds: mixed on their own voices, so a click never queues ahead of
 * the music (ls_audio_push is a FIFO; a whole clip pushed there delays
 * everything behind it). */
#define VI_UI(i)  (SFX_CACHE + 2 + SFX_EV_SLOTS + (i))
static wav_t g_ui_click;      /* help_screen_context: WAV>ANIMS>INVENT2.WAV (sub_44CF90) */
static b8    g_ui_click_tried;

static void sfx_ui_play_impl(const wav_t *w, i32 freq, i32 ds_vol) {
    static i32 next;
    i32 slot = -1;
    for (i32 i = 0; i < SFX_UI_SLOTS && slot < 0; i++)
        if (!voice_playing(VI_UI(i))) slot = i;
    if (slot < 0) slot = next++ % SFX_UI_SLOTS;          /* all busy: cut the oldest */
    voice_play(VI_UI(slot), w, freq, 1);
    voice_volume(VI_UI(slot), ds_vol);
    voice_pan(VI_UI(slot), 0);
}

void sfx_ui_play(const wav_t *w, i32 freq, i32 ds_vol) {
    audio_lock();
    sfx_ui_play_impl(w, freq, master_vol(ds_vol));
    audio_unlock();
    mixer_thread_start();
}

void sfx_ui_click(i32 freq) {
    audio_lock();
    if (!g_ui_click_tried && state && state->game_path) {
        g_ui_click_tried = 1;
        c8 p[512];
        snprintf(p, sizeof(p), "%s/wav/ANIMS/INVENT2.WAV", state->game_path);
        if (!wav_load(&g_ui_click, p)) print("sfx: no menu click %s\n", p);
    }
    if (g_ui_click.samples) sfx_ui_play_impl(&g_ui_click, freq, master_vol(0));   /* sub_44CFF6(0) */
    audio_unlock();
    mixer_thread_start();
}

static b8 sfx_voice_busy_impl(void) { return g_inst_used[SFX_INST_VOICE]; }

b8 sfx_voice_busy(void) {
    audio_lock();
    b8 r = sfx_voice_busy_impl();
    audio_unlock();
    return r;
}

/* Cut the dialogue line short; leaves a one-shot in slot 0 alone. */
/* sub_44E46A(0) and sub_44E46A(1): free the voice / one-shot slot and the AMB
 * loop slot (the level script starts its ambience again). */
void sfx_free_slots(void) {
    audio_lock();
    inst_free(SFX_INST_VOICE);
    inst_free(SFX_INST_AMB);
    audio_unlock();
}

void sfx_voice_stop(void) {
    audio_lock();
    if (g_inst_dialog) inst_free(SFX_INST_VOICE);
    audio_unlock();
}

/* A sound from wav>anims> (e.g. "trans" for sub_42E0E0). */
/* play_entity_sound with a name from the sound-effect dir. */
static void sfx_entity_named_impl(const entity_t *e, const c8 *name) { sfx_play_entity(e, sfx_get_id(name, g_dir_sfx)); }

void sfx_entity_named(const entity_t *e, const c8 *name) {
    audio_lock();
    sfx_entity_named_impl(e, name);
    audio_unlock();
}

static void sfx_play_anims_impl(const c8 *name) { sfx_play_entity(NULL, sfx_get_id(name, g_dir_anims)); }

void sfx_play_anims(const c8 *name) {
    audio_lock();
    sfx_play_anims_impl(name);
    audio_unlock();
}
