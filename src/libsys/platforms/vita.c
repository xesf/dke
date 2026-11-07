/*
 * vita.c — PlayStation Vita backend (VitaSDK).
 *
 *   video  the 640x480 frame stretched 4:3 into a 960x544 framebuffer in
 *          CDRAM, by the CPU (725 x 544 centred)
 *   input  buttons and the left stick become the keys the engine reads
 *   sound  a ring buffer drained by an audio thread into one BGM port
 *
 * The game reads its data from ux0:data/dke (see ls_get_base_path).
 */

#include <psp2/audioout.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/power.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "gamma.h"
#include "envfx.h"
#include "../system.h"
#include "../common.h"
#include "../audio/audio.h"

/* Newlib heap: the engine keeps the level data, 32-bit buffers and sounds. */
int _newlib_heap_size_user = 200 * 1024 * 1024;

#define SCREEN_W 960
#define SCREEN_H 544
#define FB_SIZE (((SCREEN_W * SCREEN_H * 4) + 0x3ffff) & ~0x3ffff)

static void   *s_fb[2];
static SceUID  s_fb_block[2];
static int     s_back;
static u32    *s_surface;            /* corrected copy of the frame */

static void workers_start(void);

static void *alloc_cdram(SceUID *block) {
    void *base = NULL;
    *block = sceKernelAllocMemBlock("dke fb", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, FB_SIZE, NULL);
    if (*block < 0 || sceKernelGetMemBlockBase(*block, &base) < 0) return NULL;
    memset(base, 0, FB_SIZE);
    return base;
}

static void show(void *fb) {
    SceDisplayFrameBuf f;
    memset(&f, 0, sizeof(f));
    f.size = sizeof(f);
    f.base = fb;
    f.pitch = SCREEN_W;
    f.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
    f.width = SCREEN_W;
    f.height = SCREEN_H;
    sceDisplaySetFrameBuf(&f, SCE_DISPLAY_SETBUF_NEXTFRAME);
}

i32 ls_init_instance(system_t *system, i32 width, i32 height, i32 bpp) {
    system->quit = false;
    system->width = width;
    system->height = height;
    system->bpp = (i16)bpp;
    scePowerSetArmClockFrequency(444);
    scePowerSetGpuClockFrequency(222);
    for (int i = 0; i < 2; i++) {
        s_fb[i] = alloc_cdram(&s_fb_block[i]);
        if (!s_fb[i]) return 0;
    }
    s_surface = (u32 *)calloc((size_t)width * height, sizeof(u32));
    if (!s_surface) return 0;
    show(s_fb[0]);
    s_back = 1;
    workers_start();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    return 1;
}

/* ---- input --------------------------------------------------------------- */

/* Provisional layout (the keys are the PC ones, as process_input_command and
 * the player code read them):
 *   d-pad, left stick  arrows            Cross     Space + Enter (action, OK)
 *   Square             Q                 Triangle  E
 *   Circle             C (draw / sheathe)
 *   L                  Tab (special)     R         I (inventory)
 *   Start              Esc (menu)        Select    P (pause) */
static void set_key(system_t *s, int vk, int down) { s->actions[vk] = down; }

void ls_update_events(system_t *system) {
    SceCtrlData d;
    memset(&d, 0, sizeof(d));
    sceCtrlPeekBufferPositive(0, &d, 1);
    unsigned b = d.buttons;
    int lx = (int)d.lx - 128, ly = (int)d.ly - 128;
    const int dead = 48;
    set_key(system, 37, (b & SCE_CTRL_LEFT)  || lx < -dead);
    set_key(system, 39, (b & SCE_CTRL_RIGHT) || lx >  dead);
    set_key(system, 38, (b & SCE_CTRL_UP)    || ly < -dead);
    set_key(system, 40, (b & SCE_CTRL_DOWN)  || ly >  dead);
    set_key(system, 32, (b & SCE_CTRL_CROSS) != 0);
    set_key(system, 13, (b & SCE_CTRL_CROSS) != 0);
    set_key(system, 113, (b & SCE_CTRL_SQUARE) != 0);       /* q */
    set_key(system, 101, (b & SCE_CTRL_TRIANGLE) != 0);     /* e */
    set_key(system, 0x43, (b & SCE_CTRL_CIRCLE) != 0);      /* C */
    set_key(system, 9, (b & SCE_CTRL_LTRIGGER) != 0);       /* Tab */
    set_key(system, 105, (b & SCE_CTRL_RTRIGGER) != 0);     /* i */
    set_key(system, 27, (b & SCE_CTRL_START) != 0);         /* Esc */
    set_key(system, 80, (b & SCE_CTRL_SELECT) != 0);        /* P */
}

u32 ls_tick(void) {
    static SceUInt64 start;
    SceUInt64 now = sceKernelGetProcessTimeWide();
    if (!start) start = now;
    return (u32)((now - start) / 1000);
}

void ls_delay(u32 ms) { sceKernelDelayThread(ms * 1000); }

void ls_set_title(system_t *system, c8 *title) { (void)system; (void)title; }

/* ---- video --------------------------------------------------------------- */

/* Scaling and presenting run on two worker threads (the Vita has three cores
 * for games), so the engine can already simulate the next frame meanwhile.
 * Worker 0 scales the top half and shows the frame; worker 1 the bottom half. */
#define NWORK 2
static SceUID s_go[NWORK], s_done[NWORK], s_wthread[NWORK];
static volatile int s_wrun, s_w, s_h, s_pending;

static int s_cols[SCREEN_W];

static void scale_rows(int y0, int y1) {
    const int out_w = SCREEN_H * s_w / s_h;
    const int out_x = (SCREEN_W - out_w) / 2;
    u32 *fb = (u32 *)s_fb[s_back];
    for (int y = y0; y < y1; y++) {
        const u32 *row = s_surface + (size_t)(y * s_h / SCREEN_H) * s_w;
        u32 *out = fb + y * SCREEN_W + out_x;
        for (int x = 0; x < out_w; x++) out[x] = row[s_cols[x]];
    }
}

static int scale_thread(SceSize args, void *argp) {
    int id = *(int *)argp;
    (void)args;
    for (;;) {
        sceKernelWaitSema(s_go[id], 1, NULL);
        if (!s_wrun) break;
        scale_rows(id * SCREEN_H / NWORK, (id + 1) * SCREEN_H / NWORK);
        if (id == 0) {
            sceKernelWaitSema(s_done[1], 1, NULL);
            show(s_fb[s_back]);
            sceDisplayWaitVblankStart();
            s_back ^= 1;
            s_pending = 0;
            sceKernelSignalSema(s_done[0], 1);
        } else {
            sceKernelSignalSema(s_done[1], 1);
        }
    }
    return 0;
}

static void workers_start(void) {
    static int ids[NWORK] = {0, 1};
    s_wrun = 1;
    for (int i = 0; i < NWORK; i++) {
        s_go[i] = sceKernelCreateSema("dke go", 0, 0, 1, NULL);
        s_done[i] = sceKernelCreateSema("dke done", 0, 0, 1, NULL);
    }
    for (int i = 0; i < NWORK; i++) {
        s_wthread[i] = sceKernelCreateThread("dke scale", scale_thread, 0x10000100 - 5,
                                             0x4000, 0, i == 0 ? 0x20000 : 0x40000, NULL);
        if (s_wthread[i] >= 0) sceKernelStartThread(s_wthread[i], sizeof(int), &ids[i]);
    }
}

static void wait_presented(void) {
    if (!s_pending) return;
    sceKernelWaitSema(s_done[0], 1, NULL);
}

void ls_blit(system_t *system, u32 *front_buffer) {
    if (!s_surface || !front_buffer) return;
    wait_presented();                       /* previous frame is on screen */
    int n = system->width * system->height;
    memcpy(s_surface, front_buffer, (size_t)n * sizeof(u32));
    envfx_apply_pixels(s_surface, n);
    gamma_apply_pixels(s_surface, n);

    /* 4:3 in the middle of the panel: 725 x 544, nearest neighbour. */
    const int out_w = SCREEN_H * system->width / system->height;
    static int cols_for;
    if (cols_for != system->width) {
        for (int x = 0; x < out_w; x++) s_cols[x] = x * system->width / out_w;
        cols_for = system->width;
    }
    s_w = system->width;
    s_h = system->height;
    s_pending = 1;
    for (int i = 0; i < NWORK; i++) sceKernelSignalSema(s_go[i], 1);
}

void ls_flip(system_t *system) {
    (void)system;                           /* the worker shows the frame */
}

void ls_release(system_t *system) {
    (void)system;
    wait_presented();
    s_wrun = 0;
    for (int i = 0; i < NWORK; i++) sceKernelSignalSema(s_go[i], 1);
    for (int i = 0; i < 2; i++)
        if (s_fb[i]) { sceKernelFreeMemBlock(s_fb_block[i]); s_fb[i] = NULL; }
    free(s_surface);
    s_surface = NULL;
}

/* ---- audio --------------------------------------------------------------- */

#define AUDIO_FRAMES 1024
#define RING_BYTES   (1 << 20)

static int            s_port = -1;
static volatile int   s_running;
static SceUID         s_thread = -1;
static int            s_channels = 2;
static u8             s_ring[RING_BYTES];
static volatile int   s_r, s_w;                 /* read / write offsets */
static pthread_mutex_t s_mu = PTHREAD_MUTEX_INITIALIZER;

static int ring_used(void) { return (s_w - s_r + RING_BYTES) % RING_BYTES; }

static int audio_thread(SceSize args, void *argp) {
    static short buf[2][AUDIO_FRAMES * 2];
    int which = 0;
    (void)args; (void)argp;
    while (s_running) {
        int need = AUDIO_FRAMES * s_channels * 2;
        u8 *dst = (u8 *)buf[which];
        pthread_mutex_lock(&s_mu);
        int have = ring_used();
        int take = have < need ? have : need;
        for (int i = 0; i < take; i++) { dst[i] = s_ring[s_r]; s_r = (s_r + 1) % RING_BYTES; }
        pthread_mutex_unlock(&s_mu);
        if (take < need) memset(dst + take, 0, (size_t)(need - take));   /* underrun: silence */
        sceAudioOutOutput(s_port, buf[which]);
        which ^= 1;
    }
    return 0;
}

b8 ls_platform_audio_open(i32 sample_rate, i32 channels, i32 bit_depth) {
    (void)bit_depth;
    if (s_port >= 0) return true;
    s_channels = channels == 1 ? 1 : 2;
    s_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, AUDIO_FRAMES, sample_rate,
                                 s_channels == 1 ? SCE_AUDIO_OUT_MODE_MONO : SCE_AUDIO_OUT_MODE_STEREO);
    if (s_port < 0) return false;
    s_r = s_w = 0;
    s_running = 1;
    s_thread = sceKernelCreateThread("dke audio", audio_thread, 0x10000100 - 10, 0x10000, 0, 0, NULL);
    if (s_thread < 0) { sceAudioOutReleasePort(s_port); s_port = -1; return false; }
    sceKernelStartThread(s_thread, 0, NULL);
    return true;
}

i32 ls_platform_audio_push(const i16 *pcm, i32 bytes) {
    if (s_port < 0 || !pcm || bytes <= 0) return 0;
    pthread_mutex_lock(&s_mu);
    int room = RING_BYTES - 1 - ring_used();
    if (bytes > room) bytes = room;                 /* backpressure: drop the rest */
    const u8 *src = (const u8 *)pcm;
    for (int i = 0; i < bytes; i++) { s_ring[s_w] = src[i]; s_w = (s_w + 1) % RING_BYTES; }
    pthread_mutex_unlock(&s_mu);
    return bytes;
}

i32 ls_platform_audio_queued(void) {
    if (s_port < 0) return -1;
    pthread_mutex_lock(&s_mu);
    int q = ring_used();
    pthread_mutex_unlock(&s_mu);
    return q;
}

void ls_platform_audio_close(void) {
    if (s_port < 0) return;
    s_running = 0;
    sceKernelWaitThreadEnd(s_thread, NULL, NULL);
    sceKernelDeleteThread(s_thread);
    sceAudioOutReleasePort(s_port);
    s_port = -1;
}
