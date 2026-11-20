#include "../vclock.h"
#include "gamma.h"
#include "envfx.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../system.h"
#include "../common.h"
#include "../audio/audio.h"

static Display    *display = NULL;
static Window      xwindow = 0;
static GC          gc;
static XImage     *ximage  = NULL;
static u8         *surface = NULL;

i32 ls_init_instance(system_t *system, i32 width, i32 height, i32 bpp) {
    system->quit   = false;
    system->width  = width;
    system->height = height;
    system->bpp    = bpp;

    display = XOpenDisplay(NULL);
    if (!display) return 0;

    i32 screen = DefaultScreen(display);
    xwindow = XCreateSimpleWindow(display, RootWindow(display, screen),
                                  0, 0, (u32)width, (u32)height, 0,
                                  BlackPixel(display, screen),
                                  BlackPixel(display, screen));
    XStoreName(display, xwindow, system->title ? system->title : "DKE");
    XSelectInput(display, xwindow,
                 ExposureMask | KeyPressMask | KeyReleaseMask |
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(display, xwindow);

    gc = XCreateGC(display, xwindow, 0, NULL);

    surface = (u8 *)malloc((size_t)(width * height * bpp));
    if (!surface) return 0;
    memset(surface, 0, (size_t)(width * height * bpp));

    ximage = XCreateImage(display, DefaultVisual(display, screen),
                          (u32)(bpp * 8), ZPixmap, 0,
                          (char *)surface, (u32)width, (u32)height,
                          32, width * bpp);
    return 1;
}

void ls_update_events(system_t *system) {
    if (!display) return;
    while (XPending(display)) {
        XEvent ev;
        XNextEvent(display, &ev);
        switch (ev.type) {
            case KeyPress:
                if (XLookupKeysym(&ev.xkey, 0) == 'q') system->quit = true;
                break;
            case ClientMessage:
                system->quit = true;
                break;
            case MotionNotify:
                system->motion_x = ev.xmotion.x;
                system->motion_y = ev.xmotion.y;
                break;
            default:
                break;
        }
    }
}

u32 ls_tick(void) {
    if (vclock_active()) return vclock_tick();
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void ls_delay(u32 ms) {
    if (vclock_active()) { vclock_advance(ms); return; }
    struct timespec ts;
    ts.tv_sec  = (time_t)(ms / 1000);
    ts.tv_nsec = (long)((ms % 1000) * 1000000);
    nanosleep(&ts, NULL);
}

void ls_set_title(system_t *system, c8 *title) {
    (void)system;
    if (display && xwindow) XStoreName(display, xwindow, title);
}

void ls_blit(system_t *system, u32 *front_buffer) {
    if (!surface || !front_buffer) return;
    memcpy(surface, (u8 *)front_buffer,
           (size_t)(system->width * system->height * system->bpp));
    envfx_apply_pixels((u32 *)surface, system->width * system->height);   /* envfx.h */
    gamma_apply_pixels((u32 *)surface, system->width * system->height);   /* gamma.h */
}

void ls_flip(system_t *system) {
    if (!display || !ximage) return;
    XPutImage(display, xwindow, gc, ximage,
              0, 0, 0, 0, (u32)system->width, (u32)system->height);
    XFlush(display);
}

void ls_release(system_t *system) {
    (void)system;
    if (ximage) { ximage->data = NULL; XDestroyImage(ximage); ximage = NULL; }
    if (surface) { free(surface); surface = NULL; }
    if (display) { XCloseDisplay(display); display = NULL; }
}

/* Audio platform stubs — ALSA not implemented */
b8   ls_platform_audio_open(i32 sr, i32 ch, i32 bd)   { (void)sr; (void)ch; (void)bd; return true; }
i32  ls_platform_audio_push(const i16 *p, i32 n)      { (void)p; return n; }
i32  ls_platform_audio_queued(void)                   { return -1; }
void ls_platform_audio_close(void)                    { }
