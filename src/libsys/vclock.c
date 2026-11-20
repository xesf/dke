/*
 * vclock.c — virtual clock for fast headless runs.
 *
 * DKE_FASTCLOCK=<ms>: ls_tick no longer reads the wall clock. Every main-loop
 * frame advances it by <ms>, ls_delay advances it instead of sleeping, and
 * every read adds 1 ms so busy-wait loops still finish. Time then runs as
 * fast as the machine can render.
 */
#include <stdlib.h>
#include "common.h"
#include "vclock.h"

static i32 s_on = -1;
static u32 s_t = 1000;

i32 vclock_active(void) {
    if (s_on < 0) s_on = getenv("DKE_FASTCLOCK") != NULL;
    return s_on;
}
u32  vclock_tick(void)          { return s_t++; }
void vclock_advance(u32 ms)     { s_t += ms; }
u32  vclock_frame_ms(void) {
    const char *v = getenv("DKE_FASTCLOCK");
    i32 ms = v ? atoi(v) : 0;
    return ms > 0 ? (u32)ms : 33;
}
