#ifndef LS_VCLOCK_H
#define LS_VCLOCK_H
#include "common.h"
/* Virtual clock for fast headless runs (DKE_FASTCLOCK=<ms per frame>). */
i32  vclock_active(void);
u32  vclock_tick(void);
void vclock_advance(u32 ms);
u32  vclock_frame_ms(void);
#endif
