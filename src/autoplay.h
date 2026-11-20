#ifndef DKE_AUTOPLAY_H
#define DKE_AUTOPLAY_H

#include "libsys/common.h"
#include "libsys/system.h"

/* Headless playthrough bot (DKE_AUTOPLAY=1), see autoplay.c. */
i32  autoplay_active(void);
void autoplay_tick(system_t *sys);

#endif
