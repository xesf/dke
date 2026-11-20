#ifndef DKE_LIEU_ACTION_H
#define DKE_LIEU_ACTION_H

/*
 * lieu_action.h — LIEU script action queue ("les piles").
 *
 * Every LIEU scripting primitive in dkev.exe does the same two things:
 *   1. fill the 0x30-byte record at dword_4F4CC0 and call the demo recorder
 *      sub_45D560 @ 0x45D560 (replay logging; not ported),
 *   2. allocate an action node via sub_4441A5 @ 0x4441A5, fill it, and set
 *      the "queue non-empty" flag dword_4FFB50 @ 0x4FFB50.
 *
 * The engine then drains the queue one head node per pile per frame from
 * sub_44CDB8 @ 0x44CDB8, dispatching each node through the 40-case switch in
 * sub_44A58B @ 0x44A58B. A node stays at the head across frames until its
 * handler calls sub_4442EE @ 0x4442EE ("action complete") — that is how
 * multi-frame actions (walk to a point, hold an animation for N ms) work.
 *
 * Three independent piles exist (the script picks one via SELECTPILE, engine
 * sub_446A05 @ 0x446A05 → dword_505C30 @ 0x505C30):
 *   pile 0  nodes dword_4FC1A0 @ 0x4FC1A0  count dword_4FC190 @ 0x4FC190
 *   pile 1  nodes dword_502BB0 @ 0x502BB0  count dword_50724C @ 0x50724C
 *   pile 2  nodes dword_4FFCD0 @ 0x4FFCD0  count dword_507250 @ 0x507250
 * Each holds up to 100 nodes of 0x78 (120) bytes; overflow logs
 * "la pile N est saturee" and reuses a scratch node.
 *
 * Action type == the index into the 16-byte-stride name table at 0x4B5438
 * ("ANIMATION", "DIALOGUE", ...) and matches the engine callback slot the
 * script called (see lieu_cb.h).
 */

#include "libsys/common.h"

/* Action types — name table @ 0x4B5438, 16 bytes per entry. */
typedef enum {
    LA_ANIMATION    = 0,
    LA_DIALOGUE     = 1,
    LA_EVENEMENT    = 2,
    LA_CINEMA       = 3,
    LA_KILLPERSO    = 4,
    LA_POSPERSO     = 5,
    LA_ETATOBJET    = 6,
    LA_COLOBJET     = 7,
    LA_JOINDRE      = 8,
    LA_CHANGECAMERA = 9,
    LA_CHANGEPIECE  = 10,
    LA_BRUITAGE     = 11,
    LA_AJOUTEZONE   = 12,
    LA_RETIREZONE   = 13,
    LA_COMBAT       = 14,
    LA_DIRECPERSO   = 15,
    LA_DIRECZONE    = 16,
    LA_DIRECSPEC    = 17,
    LA_DISTANCE     = 18,
    LA_ANIMOBJET    = 19,
    LA_BLOQUE       = 20,
    LA_FOUILLE      = 21,
    LA_STOPJOINDRE  = 22,
    LA_VIRECOL      = 23,
    LA_SIMULT       = 24,
    LA_PRINT        = 25,
    LA_JOINOBJ      = 26,
    LA_JOINDREPERM  = 27,
    LA_MODIFYVAR    = 28,
    LA_RANDOM       = 29,
    LA_SELECTPILE   = 30,
    LA_ONJOINDRE    = 31,
    LA_LINKOBJET    = 32,
    LA_PLACEANIMAL  = 33,
    LA_DIVERS       = 34,
    LA_PLAYANIM     = 35,
    LA_PAUSEANIM    = 36,
    LA_DIVERSEMPILE = 37,
    LA_LOADANIM     = 38,
    LA_FREEANIM     = 39,
    LA_TYPE_COUNT   = 40
} lieu_action_type_e;

/* Name of an action type (table @ 0x4B5438), or "?" out of range. */
const c8 *lieu_action_name(i32 type);

/*
 * Action node. The original is a flat 0x78-byte record; the fields below
 * carry their original byte offsets so the layout stays checkable against
 * the disassembly. Offsets not listed are unused by the ported primitives.
 */
typedef struct {
    i32 type;        /* +0x00  action type; -1 = free/complete            */
    i32 prev_type;   /* +0x04  previous type, kept by sub_4442EE          */
    i32 actor;       /* +0x0C  entity / object id (abs value)            */
    i32 p10;         /* +0x10  JOINDRE a4, JOINDREPERM flag              */
    i32 x;           /* +0x14  world X (script tile x600)                */
    i32 y;           /* +0x18  world Y (script tile x600)                */
    i32 z;           /* +0x1C  world Z (script tile x600)                */
    i32 p20;         /* +0x20                                            */
    i32 p24;         /* +0x24                                            */
    i32 p28;         /* +0x28                                            */
    i32 p2C;         /* +0x2C  POSPERSO zone radius                      */
    c8  name[32];    /* +0x30  16-byte name buffer in the original       */
    i32 duration;    /* +0x40  hold time in ms; -1 = complete at once    */
    i32 id48;        /* +0x48  EVENEMENT event id                        */
    i32 p44;         /* +0x44  ANIMATION: name came from the persona table */
    i32 p4C;         /* +0x4C  POSPERSO vie                              */
    i32 p50;         /* +0x50  POSPERSO zone                             */
    i32 p54;         /* +0x54                                            */
    i32 p58;         /* +0x58  CHANGECAMERA camera                       */
    i32 num5C;       /* +0x5C  numeric suffix parsed from the name       */
    i32 p68;         /* +0x68                                            */
    i32 p6C;         /* +0x6C  PAUSEANIM frame                           */
    i32 flag70;      /* +0x70  ANIMATION: 1 = sound-prefixed name        */
    i32 *slot74;     /* +0x74  MODIFYVAR: script state slot to write     */
    i32 p60, p64;    /* +0x60 / +0x64  BLOQUE camera / movement lock     */
    /* Port-local bookkeeping — no counterpart in the 0x78-byte original,
     * which tracks in-progress actions through the entity struct instead. */
    u32 start_ms;    /* when this node reached the head of its pile       */
    i32 aux0, aux1;  /* JOINDRE: origin X / Z while interpolating         */
    i32 aux_state;   /* 0 = not started, 1 = running                      */
} lieu_action_t;

/*
 * Allocate the next node of the active pile (sub_4441A5 @ 0x4441A5). Never
 * returns NULL: on overflow it returns the pile's scratch node, matching the
 * original's unk_4FF008 / unk_505A18 / unk_502B38 behaviour. The node is
 * zeroed and its type set to `type`; the caller fills the rest.
 */
lieu_action_t *lieu_action_push(i32 type);

/* Mark the head node complete and pop the active pile (sub_4442EE @ 0x4442EE). */
void lieu_action_done(lieu_action_t *node);

/* SELECTPILE: choose the pile new actions land in (dword_505C30 @ 0x505C30). */
void lieu_action_select_pile(i32 pile);

/* Enable/disable draining of a pile (dword_4FC194 / 507274 / 507270). */
void lieu_action_enable_pile(i32 pile, i32 enable);

/* SELECTPILE mode 2: drop every node of one pile and re-enable it
 * (sub_446A05 @ 0x446A6B — memset 0x2EE0 bytes to -1, count = 0). */
void lieu_action_clear_pile(i32 pile);

/* Drop every queued action + timer (level load / teardown). */
void lieu_action_reset(void);

/*
 * Per-frame driver (sub_44CDB8 @ 0x44CDB8 + the object-timer scan inside
 * sub_44968B @ 0x44968B). Runs the head action of each enabled pile and
 * fires any expired object timer as opcode 4. `now_ms` is the game clock.
 */
void lieu_action_run(u32 now_ms);

/* Queue introspection (debug HUD / logging). */
i32 lieu_action_pending(void);
void lieu_action_dump_heads(void);   /* debug: head of each pile */
i32 lieu_action_count(i32 pile);

/*
 * Object timers (engine table object_timers @ 0x4FFB90, 20 slots of 16
 * bytes: +0 start, +4 now, +8 duration, +0xC event id; count at
 * object_count). EVENEMENT arms one with duration >= 0 and cancels it with
 * duration == -1. On expiry the engine calls FonctionEditor opcode 4 with
 * the event id — that is what advances timed script sequences.
 */
void lieu_timer_add(i32 event_id, i32 duration_ms, u32 now_ms);
void lieu_timer_remove(i32 event_id);
i32  lieu_timer_count(void);
void lieu_timers_shift(i32 delta, i32 rebase, u32 now);   /* pause time compensation */

/*
 * Dispatch one head action (the 40-case switch of sub_44A58B @ 0x44A58B).
 * Implemented in lieu_cb.c, which owns the entity / spawn / zone state.
 * Return 1 when the action is finished (the driver then pops it), 0 to keep
 * it at the head for another frame.
 */
i32 lieu_action_dispatch(lieu_action_t *node, u32 now_ms);

/* Pile whose head is being run (dword_507248 @ 0x507248). */
i32 lieu_action_run_pile(void);

/* Pile new actions land in (dword_505C30 @ 0x505C30). */
i32 lieu_selected_pile(void);

#endif /* DKE_LIEU_ACTION_H */
