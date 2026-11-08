#include "gamma.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "movie.h"
#include "movie_decoder.h"
#include "system.h"
#include "file_system.h"
#include "audio/audio.h"
#include "../state.h"

/* seb_* sub-allocator state (originals: _allocation_size, _number_allocations,
 * dword_508E30[]) */
#define SEB_MAX_ALLOCS  256
static void *seb_slots[SEB_MAX_ALLOCS];
static i32   seb_num_allocations = 0;
static u32   seb_allocation_size = 0;

/* movie palette / gradient tables built by ls_init_movie.
 *   byte_508B00[32 * 3]  — 32 RGB triples: (0, 2*i, 0), green ramp
 *   byte_508B60[14 * 14 * 3] — 14x14 grid of triples: (i*63/13, 0, j*63/13),
 *                              red/blue gradient LUT for movie colour mapping.
 */
u8 movie_green_ramp[32 * 3];
u8 movie_rb_grid[14 * 14 * 3];

/* movie-wide state (small subset of the original globals used by init/shut) */
b8 movie_is_ci8       = false;   /* _is_ci8            */
b8 movie_extern_sbt   = false;   /* _ls_extern_sbt     */
i32 movie_compression = 0;       /* dword_4B4628 (GetProfileString LIBSYS.COMPR) */

/* Heap slot returned by ls_get_heap(0x9EB10) — released by ls_shut_movie. */
static void *movie_heap_slot = NULL;

/* Callback slots — original stored two function pointers here. The port keeps
 * them as opaque placeholders until audio flip / decoder is wired. */
static void *movie_audio_flip_cb = NULL;

/* Big work buffers — sizes taken verbatim from asm. */
static void *movie_scratch_D6D8 = NULL;      /* dword_5089BC */
static void *movie_slice_9C40[20];           /* dword_508A50[0..19] */
static void *movie_zik_load = NULL;          /* _zik_load, size 0x15CC */

/* 0x00430A50
 * seb_get_memory(aligned, size)
 *   aligned != 0 -> reserve extra 0x10000 bytes and align result down to 64k boundary.
 *   aligned == 0 -> plain allocation.
 * Zero-fills result. Records raw base pointer in seb_slots for later release. */
static void *seb_get_memory(i32 aligned, i32 size) {
    i32 padding = aligned ? 0x10000 : 0;
    u8 *raw     = (u8 *)malloc((size_t)(size + padding));
    if (!raw) return NULL;
    /* aligned == 0 -> return raw. aligned != 0 -> round raw down to 64K
     * boundary, add 64K padding. Full-width mask on 64-bit hosts. */
    uintptr_t mask = aligned ? ~(uintptr_t)0xFFFFu : ~(uintptr_t)0u;
    u8 *aligned_ptr = (u8 *)(((uintptr_t)raw & mask) + (uintptr_t)padding);
    memset(aligned_ptr, 0, (size_t)size);
    if (seb_num_allocations < SEB_MAX_ALLOCS) {
        seb_slots[seb_num_allocations++] = raw;
    }
    seb_allocation_size += (u32)(size + padding);
    return aligned_ptr;
}

/* 00430A08 */
static void init_my_memory(void) {
    seb_allocation_size = 0;
    seb_num_allocations = 0;
}

/* 00430B00 */
static void seb_release_all(void) {
    for (i32 i = 0; i < seb_num_allocations; i++) {
        free(seb_slots[i]);
        seb_slots[i] = NULL;
    }
    seb_num_allocations = 0;
    seb_allocation_size = 0;
}

/* cp_init / cp_uninit @ 0x4328E7 / 0x4329EE — colour-pack codec buffers.
 * 6 buffers × 0xFC00 bytes, zeroed (0xFA00 active region each).
 * Original uses ls_get_memory_heap; we use malloc since heap not ported. */
#define CP_BUF_SIZE  0xFC00
#define CP_ZERO_SIZE 0xFA00
#define CP_BUF_COUNT 6
static void *cp_buffers[CP_BUF_COUNT] = {0};

static void cp_init(void) {
    /* cp_init @ 0x4328E7 */
    for (i32 i = 0; i < CP_BUF_COUNT; i++) {
        cp_buffers[i] = malloc(CP_BUF_SIZE);
        if (cp_buffers[i]) memset(cp_buffers[i], 0, CP_ZERO_SIZE);
    }
}

static void cp_uninit(void) {
    /* cp_uninit @ 0x4329EE */
    for (i32 i = 0; i < CP_BUF_COUNT; i++) {
        free(cp_buffers[i]);
        cp_buffers[i] = NULL;
    }
}

/* 00431EA0 */
void ls_init_movie(i32 a, i32 b) {
    (void)a;

    init_my_memory();
    movie_is_ci8     = false;
    movie_extern_sbt = false;

    /* Green ramp: 32 triples (0, 2*i, 0) */
    for (i32 i = 0; i < 32; i++) {
        movie_green_ramp[i * 3 + 0] = 0;
        movie_green_ramp[i * 3 + 1] = (u8)(i * 2);
        movie_green_ramp[i * 3 + 2] = 0;
    }

    /* 14x14 red/blue gradient LUT */
    for (i32 i = 0; i < 14; i++) {
        for (i32 j = 0; j < 14; j++) {
            i32 idx = (i * 14 + j) * 3;
            movie_rb_grid[idx + 0] = (u8)((i * 63) / 13);
            movie_rb_grid[idx + 1] = 0;
            movie_rb_grid[idx + 2] = (u8)((j * 63) / 13);
        }
    }

    /* Audio flip callback selection (asm: dword_508E10 = seb_traite_zik or
     * seb_traite_zik_flip). Not yet ported — record intent. */
    movie_audio_flip_cb = (void *)(uintptr_t)(b == 0 ? 1 : 2);

    movie_scratch_D6D8 = seb_get_memory(0, 0xD6D8);

    for (i32 i = 0; i < 20; i++) {
        movie_slice_9C40[i] = seb_get_memory(0, 0x9C40);
    }

    movie_zik_load = seb_get_memory(1, 0x15CC);

    /* ls_get_heap slot (0x9EB10 bytes) — decoder scratch. */
    movie_heap_slot = malloc(0x9EB10);
    if (movie_heap_slot) memset(movie_heap_slot, 0, 0x9EB10);

    /* cp_init @ 0x4328E7 — colour-pack codec init.
     * Allocates 6 × 0xFC00 byte buffers for palette compression tables.
     * Original uses ls_get_memory_heap (not ported); we use malloc. */
    cp_init();

    /* Original reads win.ini [LIBSYS] COMPR=<TRUE|FALSE>. Default: FALSE -> 0. */
    movie_compression = 0;
}

/* 00432120 */
void ls_shut_movie(void) {
    cp_uninit();  /* cp_uninit @ 0x4329EE */
    if (movie_heap_slot) {
        free(movie_heap_slot);
        movie_heap_slot = NULL;
    }
    seb_release_all();
    movie_scratch_D6D8 = NULL;
    for (i32 i = 0; i < 20; i++) movie_slice_9C40[i] = NULL;
    movie_zik_load = NULL;
}

/* 00432550
 * ls_play_movie — CRH decoder port (via CRH_to_MP4.py reference).
 *
 * Path resolution:
 *   - If path contains '/' or '\', use as-is (already absolute/rooted).
 *   - Otherwise prefix <state->game_path>/sc/  (Dark Earth stores cutscenes
 *     there; original used virtual "HDISK:>EARTH>sc>" prefix).
 *
 * Video: decoded frame → u32 XRGB (0xFF000000|R<<16|G<<8|B) → front_buffer.
 *   Frames smaller than display: copy top-left w×h region, rest unchanged.
 *   Frames larger than display: log and skip rendering (no crash).
 *
 * Audio: each frame's PCM block is pushed to the output queue as it is
 *   decoded, MOVIE_LEAD frames ahead of the picture.
 *
 * Frame timing: absolute schedule from the first audio push.
 * Quit: ls_update_events; break on system->quit.
 *
 * Ported functions:
 *   init_decompress / read_entete / decompress_anim → crh_open + crh_next_frame
 *   cine_clear_movie_background → front_buffer zeroed after playback
 *   cp_restore_screen thread → not needed; blit+flip per frame is synchronous
 */
/* ---- per-movie options (see movie.h) ----------------------------------- */

static ls_movie_sbt_fn s_sbt_cb;
static c8   s_sbt_lang[8];
static i32  s_fade_ms;                 /* the "_500" suffix: fade-out at the end */
static void (*s_end_card)(void);

void ls_set_movie_subtitles(ls_movie_sbt_fn cb, const c8 *lang) {
    s_sbt_cb = cb;
    snprintf(s_sbt_lang, sizeof(s_sbt_lang), "%s", lang ? lang : "US");
}
void ls_set_movie_fade(i32 ms) { s_fade_ms = ms > 0 ? ms : 0; }
void ls_set_movie_end_card(void (*cb)(void)) { s_end_card = cb; }

typedef struct { i32 start, end; c8 text[256]; } sbt_line_t;
#define SBT_MAX 256

/* 0x430E15 read_and_init_sbt: time = 900*mm + 15*ss + ff/2 (15 fps frames). */
static i32 sbt_load(const c8 *path_hint, const c8 *lang, sbt_line_t *out) {
    /* "…/SC21.CRH" -> 21 */
    const c8 *b = path_hint;
    for (const c8 *p = path_hint; *p; p++) if (*p == '/' || *p == '\\' || *p == '>') b = p + 1;
    if ((b[0] | 0x20) != 's' || (b[1] | 0x20) != 'c') return 0;
    i32 num = atoi(b + 2);
    c8 vpath[64], host[512];
    snprintf(vpath, sizeof(vpath), "DATAS>SBT>SC%02d_%s.TXT", num, lang);
    const c8 *gp = (state && state->game_path) ? state->game_path : ".";
    if (!ls_resolve_path(gp, vpath, host, (i32)sizeof(host))) return 0;
    FILE *f = ls_open(host, "rb");
    if (!f) return 0;
    i32 n = 0;
    c8 line[512];
    while (n < SBT_MAX && fgets(line, sizeof(line), f)) {
        i32 a1, a2, a3, b1, b2, b3;
        if (sscanf(line, "%d:%d:%d %d:%d:%d", &a1, &a2, &a3, &b1, &b2, &b3) != 6) continue;
        const c8 *o = strchr(line, '{'), *e = o ? strchr(o, '}') : NULL;
        if (!o || !e) continue;
        sbt_line_t *s = &out[n++];
        s->start = 900 * a1 + 15 * a2 + a3 / 2;
        s->end   = 900 * b1 + 15 * b2 + b3 / 2;
        i32 len = (i32)(e - o - 1);
        if (len > 255) len = 255;
        memcpy(s->text, o + 1, (size_t)len);
        s->text[len] = '\0';
    }
    ls_close(f);
    return n;
}

static void play_movie_impl(const c8 *path) {
    if (!path || !path[0]) {
        printf("ls_play_movie: null/empty path\n");
        return;
    }

    /* Build resolved path. Accepts:
     *   "HDISK:>EARTH>sc>SC08.CRH" (LIBSYS virtual — original engine style)
     *   "sc/SC08.CRH"              (relative from game_path)
     *   "SC08.CRH"                 (bare name — assume in sc/)
     *   absolute host path         (pass through)
     */
    c8 resolved[512];
    const c8 *gp = (state && state->game_path) ? state->game_path : ".";
    b8 is_virtual = (strchr(path, ':') && strchr(path, '>')) != 0;
    b8 has_sep    = false;
    for (const c8 *p = path; *p; p++) {
        if (*p == '/' || *p == '\\') { has_sep = true; break; }
    }

    if (is_virtual || path[0] == '/' || has_sep) {
        if (!ls_resolve_path(gp, path, resolved, (i32)sizeof(resolved))) {
            printf("ls_play_movie: path overflow: %s\n", path);
            return;
        }
    } else {
        /* Bare filename — assume cutscene folder. */
        snprintf(resolved, sizeof(resolved), "%s/sc/%s", gp, path);
    }

    printf("ls_play_movie: opening %s\n", resolved);

    crh_reader_t r;
    if (!crh_open(&r, resolved)) {
        printf("ls_play_movie: crh_open failed for %s\n", resolved);
        return;
    }

    /* Allocate decode buffers */
    u8  *rgb_buf  = (u8 *)malloc((size_t)(r.width * r.height * 3));

    /* Audio: raw CRH_A_BYTES block + decoded i16 PCM */
    b8   do_audio      = (state && state->audio_enabled);
    i32  ach           = (r.channels > 0 && r.channels <= 2) ? (i32)r.channels : 2;
    /* samples_per_ch = (CRH_A_BYTES - 2*ach) / ach  (seed bytes excluded) */
    i32  samples_per_ch = (CRH_A_BYTES - 2 * ach) / ach;
    /* pcm_frames = 1 (seed) + samples_per_ch; output = frames * channels i16 */
    i32  pcm_buf_samples = (1 + samples_per_ch) * ach;
    u8  *audio_raw  = do_audio ? (u8 *)malloc(CRH_A_BYTES) : NULL;
    i16 *audio_pcm  = do_audio ? (i16 *)malloc((size_t)pcm_buf_samples * sizeof(i16)) : NULL;

    if (do_audio && (!audio_raw || !audio_pcm)) {
        printf("ls_play_movie: audio buffer alloc failed — disabling audio for this movie\n");
        free(audio_raw); free(audio_pcm);
        audio_raw = NULL; audio_pcm = NULL;
        do_audio = false;
    }

    if (!rgb_buf) {
        printf("ls_play_movie: out of memory\n");
        free(audio_raw); free(audio_pcm);
        crh_close(&r);
        return;
    }

    system_t *sys      = state ? &state->system : NULL;
    graphic_t *graphic = state ? &state->graphic : NULL;
    audio_t   *audio   = state ? &state->audio   : NULL;

    /* Clear letterbox bars once at start of playback. Scale loop overwrites
     * the fitted region every frame; the black bars stay untouched. */
    if (graphic && graphic->front_buffer) {
        memset(graphic->front_buffer, 0,
               (size_t)graphic->width * graphic->height * sizeof(u32));
    }

    /* Each CRH frame carries 1/fps s of PCM. Pushing it just as the video
     * frame is shown leaves the output queue empty between pushes (a gap
     * every frame). Instead decode MOVIE_LEAD frames ahead: their audio is
     * queued right away, and each picture is shown on an absolute schedule
     * from the moment the first audio block went out, so the queue always
     * holds ~MOVIE_LEAD frames and video stays locked to the sound. */
    enum { MOVIE_LEAD = 4 };
    i32 n_pixels = r.width * r.height;
    u32 *ring[MOVIE_LEAD + 1] = { 0 };
    for (i32 k = 0; k <= MOVIE_LEAD; k++) {
        ring[k] = (u32 *)malloc((size_t)n_pixels * sizeof(u32));
        if (!ring[k]) {
            for (i32 j = 0; j < k; j++) free(ring[j]);
            free(rgb_buf); free(audio_raw); free(audio_pcm);
            crh_close(&r);
            return;
        }
    }
    u32 decoded = 0, shown = 0, t0 = 0;
    b8  eof = false;

    static sbt_line_t sbt[SBT_MAX];
    i32 sbt_n = s_sbt_cb ? sbt_load(path, s_sbt_lang, sbt) : 0;
    const i32 first_frame = 0;
    bool skipped = false;

    for (;;) {
        /* Keep the decoder MOVIE_LEAD frames ahead of the display. */
        while (!eof && decoded < shown + MOVIE_LEAD + 1) {
            if (!crh_next_frame_ex(&r, rgb_buf, do_audio ? audio_raw : NULL)) { eof = true; break; }
            if (do_audio && audio && audio_raw && audio_pcm) {
                i32 pcm_bytes = crh_decode_audio(audio_raw, CRH_A_BYTES, ach, audio_pcm);
                if (pcm_bytes > 0) ls_audio_push(audio, audio_pcm, pcm_bytes);
            }
            if (decoded == 0) t0 = ls_tick();
            /* RGB triples -> u32 XRGB (NSBitmapImageRep wants byte[0]=R) */
            u32 *dst = ring[decoded % (MOVIE_LEAD + 1)];
            for (i32 i = 0; i < n_pixels; i++)
                dst[i] = 0xFF000000u | ((u32)rgb_buf[i * 3 + 2] << 16)
                       | ((u32)rgb_buf[i * 3 + 1] << 8) | (u32)rgb_buf[i * 3 + 0];
            decoded++;
        }
        if (shown >= decoded) break;

        /* Present frame `shown` when its audio starts playing. */
        u32 due = t0 + (u32)((u64)shown * 1000u / (r.fps ? r.fps : 15));
        i32 wait = (i32)(due - ls_tick());
        if (wait > 0) ls_delay((u32)wait);
        u32 *pix = ring[shown % (MOVIE_LEAD + 1)];
        shown++;

        /* Scale movie into front_buffer preserving aspect ratio.
         * Nearest-neighbour up/downscale, centred. For 320x200 in 640x480
         * this picks the largest integer factor that fits both dims (2×),
         * yielding a 640x400 image centred with 40px vertical margin. */
        if (graphic && graphic->front_buffer && r.width > 0 && r.height > 0) {
            i32 fb_w = graphic->width;
            i32 fb_h = graphic->height;

            /* Fractional fit — pick min of fb_w/w and fb_h/h in fixed 16.16. */
            i32 sx_q16 = (fb_w << 16) / r.width;
            i32 sy_q16 = (fb_h << 16) / r.height;
            i32 s_q16  = sx_q16 < sy_q16 ? sx_q16 : sy_q16;
            i32 out_w  = (r.width  * s_q16) >> 16;
            i32 out_h  = (r.height * s_q16) >> 16;
            i32 off_x  = (fb_w - out_w) / 2;
            i32 off_y  = (fb_h - out_h) / 2;
            i32 istep  = (1 << 16) * r.width  / (out_w > 0 ? out_w : 1);
            i32 jstep  = (1 << 16) * r.height / (out_h > 0 ? out_h : 1);

            for (i32 dy = 0; dy < out_h; dy++) {
                i32 sy = (dy * jstep) >> 16;
                if (sy >= r.height) sy = r.height - 1;
                u32 *src_row = pix + sy * r.width;
                u32 *dst_row = graphic->front_buffer + (off_y + dy) * fb_w + off_x;
                i32 sx_frac = 0;
                for (i32 dx = 0; dx < out_w; dx++) {
                    i32 sx = sx_frac >> 16;
                    if (sx >= r.width) sx = r.width - 1;
                    dst_row[dx] = src_row[sx];
                    sx_frac += istep;
                }
            }
        }

        /* 0x431216 show_sbt: the line whose [start, end) holds this frame. */
        if (sbt_n && graphic && graphic->front_buffer) {
            i32 fno = first_frame + (i32)shown - 1;
            for (i32 k = 0; k < sbt_n; k++)
                if (fno >= sbt[k].start && fno < sbt[k].end) {
                    s_sbt_cb(sbt[k].text, graphic->front_buffer, graphic->width, graphic->height);
                    break;
                }
        }

        /* Blit + flip */
        if (sys) {
            ls_blit(sys, graphic ? graphic->front_buffer : NULL);
            ls_flip(sys);
            ls_update_events(sys);
            if (sys->quit) break;
            /* ESC skips the movie — consume the edge so the menu doesn't
             * also see it. */
            if (sys->actions[27] == 1) {
                sys->actions[27] = 0;
                printf("ls_play_movie: skipped by ESC at frame %u/%u\n",
                       shown, (u32)r.frame_count);
                skipped = true;
                break;
            }
        }
    }
    for (i32 k = 0; k <= MOVIE_LEAD; k++) free(ring[k]);

    /* 0x4330DD cp_play_fade: the last frame fades to black over the "_nnn" ms. */
    if (s_fade_ms > 0 && !skipped && sys && !sys->quit && graphic && graphic->front_buffer) {
        i32 n = graphic->width * graphic->height;
        u32 *orig = (u32 *)malloc((size_t)n * sizeof(u32));
        if (orig) {
            memcpy(orig, graphic->front_buffer, (size_t)n * sizeof(u32));
            u32 t_start = ls_tick();
            for (;;) {
                u32 el = ls_tick() - t_start;
                u32 k = el >= (u32)s_fade_ms ? 0 : 256u - (el * 256u) / (u32)s_fade_ms;   /* 256 -> 0 */
                u32 *d = graphic->front_buffer;
                for (i32 i = 0; i < n; i++) {
                    u32 p = orig[i];
                    d[i] = 0xFF000000u | ((((p >> 16) & 0xFF) * k >> 8) << 16)
                         | ((((p >> 8) & 0xFF) * k >> 8) << 8) | (((p & 0xFF) * k) >> 8);
                }
                ls_blit(sys, d);
                ls_flip(sys);
                ls_update_events(sys);
                if (k == 0 || sys->quit) break;
            }
            free(orig);
        }
    }

    /* cine_clear_movie_background: zero front_buffer after playback */
    if (graphic && graphic->front_buffer) {
        memset(graphic->front_buffer, 0,
               (size_t)(graphic->width * graphic->height) * sizeof(u32));
    }

    free(rgb_buf);
    free(audio_raw);
    free(audio_pcm);
    crh_close(&r);
    printf("ls_play_movie: done (%u frames)\n", (u32)r.frame_count);
}

/* The colour correction (gamma.h) does not apply to movies. */
void ls_play_movie(const c8 *path) {
    gamma_bypass(1);
    play_movie_impl(path);
    if (s_end_card && !(state && state->system.quit)) s_end_card();   /* ls_play_movie's callback */
    gamma_bypass(0);
    s_sbt_cb = NULL;                       /* the options last for one movie */
    s_fade_ms = 0;
    s_end_card = NULL;
}
