/*
 * lieu_action.c — LIEU script action queue ("les piles") + object timers.
 *
 * Port of the engine's deferred-action machinery:
 *   sub_4441A5 @ 0x4441A5  allocate a node in the active pile
 *   sub_4442EE @ 0x4442EE  action complete -> pop the head
 *   sub_44CDB8 @ 0x44CDB8  per-frame driver: run each pile's head node
 *   sub_44A58B @ 0x44A58B  40-case dispatch (effects live in lieu_cb.c)
 *   sub_44968B @ 0x44968B  per-frame object-timer scan -> opcode 4
 *
 * See lieu_action.h for the pile/global address map.
 */

#include <string.h>
#include <stdlib.h>

#include "libsys/common.h"
#include "lieu_action.h"
#include "lieu_cb.h"

#define LA_PILE_COUNT 3
#define LA_PILE_MAX   100   /* engine cap; overflow logs "la pile N est saturee" */

/* Action type names — engine table @ 0x4B5438 (16 bytes per entry). */
static const c8 *const LA_NAMES[LA_TYPE_COUNT] = {
    "ANIMATION",   "DIALOGUE",  "EVENEMENT",  "CINEMA",     "KILLPERSO",
    "POSPERSO",    "ETATOBJET", "COLOBJET",   "JOINDRE",    "CHANGECAMERA",
    "CHANGEPIECE", "BRUITAGE",  "AJOUTEZONE", "RETIREZONE", "COMBAT",
    "DIRECPERSO",  "DIRECZONE", "DIRECSPEC",  "DISTANCE",   "ANIMOBJET",
    "BLOQUE",      "FOUILLE",   "STOPJOINDRE","VIRECOL",    "SIMULT",
    "PRINT",       "JOINOBJ",   "JOINDREPERM","MODIFYVAR",  "RANDOM",
    "SELECTPILE",  "ONJOINDRE", "LINKOBJET",  "PLACEANIMAL","DIVERS",
    "PLAYANIM",    "PAUSEANIM", "DIVERSEMPILE","LOADANIM",  "FREEANIM",
};

const c8 *lieu_action_name(i32 type) {
    if (type < 0 || type >= LA_TYPE_COUNT) return "?";
    return LA_NAMES[type];
}

/* -------------------------------------------------------------------------
 * Pile storage.
 *   nodes    dword_4FC1A0 / dword_502BB0 / dword_4FFCD0
 *   counts   dword_4FC190 / dword_50724C / dword_507250
 *   enables  dword_4FC194 / dword_507274 / dword_507270
 *   busy     dword_4B5418[pile]
 *   drained  dword_4B5428 / dword_4B542C / dword_4B5430
 * ---------------------------------------------------------------------- */
static lieu_action_t g_pile[LA_PILE_COUNT][LA_PILE_MAX];
static lieu_action_t g_scratch[LA_PILE_COUNT];    /* unk_4FF008 / 505A18 / 502B38 */
static i32 g_count[LA_PILE_COUNT];
static i32 g_enable[LA_PILE_COUNT] = {1, 1, 1};
static u32 g_drained_ms[LA_PILE_COUNT];
static i32 g_sel_pile = 0;    /* dword_505C30 @ 0x505C30 — allocation target */
static i32 g_run_pile = 0;    /* dword_507248 @ 0x507248 — pile being drained */
static i32 g_pending  = 0;    /* dword_4FFB50 @ 0x4FFB50 — any action queued  */
static u32 g_now_ms   = 0;

i32 lieu_action_pending(void) { return g_pending; }

i32 lieu_action_count(i32 pile) {
    if (pile < 0 || pile >= LA_PILE_COUNT) return 0;
    return g_count[pile];
}

void lieu_action_select_pile(i32 pile) {
    if (pile < 0 || pile >= LA_PILE_COUNT) return;
    g_sel_pile = pile;
}

void lieu_action_enable_pile(i32 pile, i32 enable) {
    if (pile < 0 || pile >= LA_PILE_COUNT) return;
    g_enable[pile] = enable ? 1 : 0;
}

void lieu_action_clear_pile(i32 pile) {
    if (pile < 0 || pile >= LA_PILE_COUNT) return;
    for (i32 i = 0; i < LA_PILE_MAX; i++) g_pile[pile][i].type = -1;
    g_count[pile]  = 0;
    g_enable[pile] = 1;
    if (g_count[0] == 0 && g_count[1] == 0 && g_count[2] == 0) g_pending = 0;
}

void lieu_action_reset(void) {
    for (i32 p = 0; p < LA_PILE_COUNT; p++) {
        for (i32 i = 0; i < LA_PILE_MAX; i++) g_pile[p][i].type = -1;
        g_scratch[p].type = -1;
        g_count[p]      = 0;
        g_enable[p]     = 1;
        g_drained_ms[p] = 0;
    }
    g_sel_pile = 0;
    g_run_pile = 0;
    g_pending  = 0;
    for (i32 i = 0; i < 20; i++) lieu_timer_remove(-1);   /* clears the table */
}

/*
 * sub_4441A5 @ 0x4441A5 — allocate the next node of the active pile.
 *
 * Divergence from the original: we zero the node and default +0x40 (duration)
 * to -1. The original leaves stale bytes from the previous occupant and each
 * primitive overwrites only the fields it uses; -1 is the "complete
 * immediately" value every primitive that does not set a hold time relies on.
 */
i32 lieu_action_run_pile(void) { return g_run_pile; }
i32 lieu_selected_pile(void)   { return g_sel_pile; }

/* Debug: the head action of each pile, for stall reports. */
void lieu_action_dump_heads(void) {
    for (i32 p = 0; p < LA_PILE_COUNT; p++) {
        if (g_count[p] <= 0) continue;
        const lieu_action_t *n = &g_pile[p][0];
        print("  pile %d (%s, %d queued): %s actor=%d name='%s' x=%d y=%d z=%d started=%u\n", p,
              g_enable[p] ? "on" : "off", g_count[p], lieu_action_name(n->type), n->actor, n->name,
              n->x, n->y, n->z, n->start_ms);
    }
}

lieu_action_t *lieu_action_push(i32 type) {
    i32 p = g_sel_pile;
    if (p < 0 || p >= LA_PILE_COUNT) p = 0;

    lieu_action_t *n;
    if (g_count[p] >= LA_PILE_MAX) {
        static i32 warned[LA_PILE_COUNT];
        if (!warned[p]) {
            warned[p] = 1;
            print("lieu_action: la pile %d est saturee\n", p);
        }
        n = &g_scratch[p];             /* engine: unk_4FF008 / 505A18 / 502B38 */
    } else {
        n = &g_pile[p][g_count[p]++];
    }

    memset(n, 0, sizeof(*n));
    n->type      = type;
    n->prev_type = -1;
    n->duration  = -1;
    g_pending    = 1;                  /* dword_4FFB50 = 1 */
    return n;
}

/*
 * sub_4442EE @ 0x4442EE — the head action reported completion: record its
 * type for the next node to inspect (dword_4FC1A4 = head+4), drop it, and
 * shift the pile down. MODIFYVAR (0x1C) does not overwrite the recorded
 * previous type — the engine keeps the type before it so an ANIMATION can
 * still see that a JOINDRE preceded the variable write.
 */
void lieu_action_done(lieu_action_t *node) {
    if (!node) return;

    i32 finished = node->type;
    i32 carried  = node->prev_type;
    if (finished != LA_MODIFYVAR) carried = finished;
    node->type = -1;

    i32 p = g_run_pile;
    if (p < 0 || p >= LA_PILE_COUNT) p = 0;

    if (g_count[p] > 0) {
        for (i32 i = 0; i < g_count[p] - 1; i++)
            g_pile[p][i] = g_pile[p][i + 1];
        g_count[p]--;
        g_pile[p][g_count[p]].type = -1;      /* clear the vacated tail slot */
        if (g_count[p] == 0) g_drained_ms[p] = g_now_ms ? g_now_ms : 1;
    }
    g_pile[p][0].prev_type = carried;          /* dword_4FC1A4 */
    g_pile[p][0].start_ms  = 0;                /* new head starts its own clock */

    if (g_count[0] == 0 && g_count[1] == 0 && g_count[2] == 0)
        g_pending = 0;                         /* dword_4FFB50 = 0 */
}

/* -------------------------------------------------------------------------
 * Object timers — engine table object_timers @ 0x4FFB90, 20 records of 16
 * bytes (+0 start, +4 now, +8 duration, +0xC event id), count at
 * object_count. Armed by EVENEMENT with duration >= 0, cancelled with -1.
 * Scanned every frame by sub_44968B @ 0x44968B (0x4499A5..0x449ADD); on
 * expiry the engine calls FonctionEditor opcode 4 with the event id.
 * ---------------------------------------------------------------------- */
#define LA_TIMER_MAX 20

typedef struct {
    u32 start;      /* +0x00 gametime when armed */
    u32 now;        /* +0x04 gametime of the last scan */
    i32 duration;   /* +0x08 ms; -1 = free slot */
    i32 id;         /* +0x0C event id passed to opcode 4 */
} lieu_timer_t;

static lieu_timer_t g_timers[LA_TIMER_MAX];
static i32          g_timer_count = 0;
static i32          g_timers_init = 0;

static void timers_init_once(void) {
    if (g_timers_init) return;
    g_timers_init = 1;
    for (i32 i = 0; i < LA_TIMER_MAX; i++) { g_timers[i].duration = -1; g_timers[i].id = -1; }
}

i32 lieu_timer_count(void) { return g_timer_count; }

/* 0x4064C3: object_timers[4k] (the start stamp) += time_delta, so a timer
 * armed before a pause keeps its remaining time. 0x4067EA instead rebases it
 * to (current_game_time - start): `rebase` = 1. */
void lieu_timers_shift(i32 delta, i32 rebase, u32 now) {
    timers_init_once();
    for (i32 k = 0; k < g_timer_count; k++) {
        if (rebase) g_timers[k].start = now - g_timers[k].start;
        else        g_timers[k].start += (u32)delta;
    }
}

/* Drop slot `i` and compact the table (asm 0x449A6D..0x449ACF). */
static void timer_drop(i32 i) {
    for (i32 j = i; j < LA_TIMER_MAX - 1; j++) g_timers[j] = g_timers[j + 1];
    g_timers[LA_TIMER_MAX - 1].duration = -1;
    g_timers[LA_TIMER_MAX - 1].id       = -1;
    if (g_timer_count > 0) g_timer_count--;
}

void lieu_timer_add(i32 event_id, i32 duration_ms, u32 now_ms) {
    timers_init_once();
    i32 slot = -1;
    if (g_timer_count < LA_TIMER_MAX - 1) {        /* asm: object_count < 0x13 */
        slot = g_timer_count++;
    } else {
        for (i32 i = 0; i < LA_TIMER_MAX; i++)      /* else reuse a free slot */
            if (g_timers[i].duration == -1) { slot = i; break; }
    }
    if (slot < 0) {
        print("lieu_action: object timer table full — dropping event %d\n", event_id);
        return;
    }
    g_timers[slot].id       = event_id;
    g_timers[slot].duration = duration_ms;
    g_timers[slot].start    = now_ms;
    g_timers[slot].now      = now_ms;
}

void lieu_timer_remove(i32 event_id) {
    timers_init_once();
    if (event_id == -1) {                            /* internal: clear all */
        for (i32 i = 0; i < LA_TIMER_MAX; i++) { g_timers[i].duration = -1; g_timers[i].id = -1; }
        g_timer_count = 0;
        return;
    }
    for (i32 i = 0; i < LA_TIMER_MAX; i++) {
        if (g_timers[i].duration != -1 && g_timers[i].id == event_id) { timer_drop(i); return; }
    }
}

/* Per-frame expiry scan (sub_44968B @ 0x4499A5). Fires opcode 4. */
static void lieu_timer_tick(u32 now_ms) {
    timers_init_once();
    for (i32 i = 0; i < LA_TIMER_MAX; i++) {
        if (g_timers[i].duration == -1) continue;
        g_timers[i].now = now_ms;
        if ((i32)(now_ms - g_timers[i].start) <= g_timers[i].duration) continue;

        i32 id = g_timers[i].id;
        timer_drop(i);
        print("lieu_action: timer fired event=%d -> opcode 4\n", id);
        lieu_object_timer(id);                       /* FonctionEditor op 4 */
        i--;                                         /* slot i now holds i+1 */
    }
}

/*
 * sub_44CDB8 @ 0x44CDB8 — per-frame driver. Runs the head node of each
 * enabled pile; a node that reports "not finished" stays at the head so
 * multi-frame actions (walks, timed animations) keep running.
 *
 * Not ported: the 500 ms post-drain re-run of sub_44A2B5 @ 0x44A2B5, which
 * only redraws the debug pile overlay.
 */
void lieu_action_run(u32 now_ms) {
    /* DKE_NOQUEUE=1 parks the drain (actions still queue) — A/B switch for
     * telling queue-driven effects apart from the rest of the frame. */
    static i32 disabled = -1;
    if (disabled < 0) disabled = getenv("DKE_NOQUEUE") ? 1 : 0;
    if (disabled) return;

    g_now_ms = now_ms;

    lieu_timer_tick(now_ms);

    if (!g_pending) return;

    for (i32 p = 0; p < LA_PILE_COUNT; p++) {
        if (!g_enable[p] || g_count[p] <= 0) continue;

        lieu_action_t *n = &g_pile[p][0];
        if (n->type == -1) continue;

        g_run_pile = p;                              /* dword_507248 */
        if (n->start_ms == 0) n->start_ms = now_ms ? now_ms : 1;

        if (lieu_action_dispatch(n, now_ms))
            lieu_action_done(n);
    }
}
