/*
 * autoplay.c — headless playthrough bot (DKE_AUTOPLAY=1).
 *
 * Not part of the original game. It drives the player through the level
 * scripts to find blockers. In each room it stands in every script zone,
 * faces each of 8 headings and presses the action key, then uses every held
 * item there. A try after which a script variable takes a value it never
 * had counts as progress and re-opens every try in every room. Zones that
 * send the player to another room are remembered as exits and only taken
 * when the room has nothing left to try, toward the room with the most
 * untried work. Dialogue is advanced, fights are fought with the
 * alternate control scheme, and the hero cannot die (DKE_AUTOPLAY_MORTAL=1
 * to allow it).
 *
 * Log lines start with "auto:". "auto: STUCK" means no reachable room has
 * a try left.
 *
 * Pair with DKE_FASTCLOCK=<ms> (virtual clock), DKE_NOAUDIO=1, DKE_NOMOVIE=1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autoplay.h"
#include "actor.h"
#include "entity.h"
#include "state.h"
#include "lieu_cb.h"
#include "dialogue.h"
#include "cjr.h"
#include "inventory.h"
#include "nav_grid.h"
#include "libsys/common.h"

extern i32 g_control_scheme;                 /* combat_player.c */

#define ST_BYTES   0xFA0
#define ST_SLOTS   (ST_BYTES / 4)
#define HEADINGS   8
#define MAX_ITEMS  64
#define TRIES      (HEADINGS + MAX_ITEMS)
#define ZONES      41
#define ROOMS      100
#define SEEN       12

enum { PH_IDLE, PH_MOVE, PH_PRESS, PH_WAIT };

static i32 s_on = -1;
static i32 s_level = -1;
static i32 s_phase = PH_IDLE;
static u32 s_t_phase, s_t_enter;
static i32 s_zone, s_head, s_item, s_is_exit; /* current try */
static u8  s_tried[ROOMS][ZONES][TRIES];
static i8  s_exit[ROOMS][ZONES];             /* target room, 0 = not an exit (known) */
static u8  s_exit_known[ROOMS][ZONES];
static u8  s_room_zone[ROOMS][ZONES];        /* zone seen in that room */
static i32 s_visits[ROOMS];
static i32 s_last_room = -1, s_last_zone = -1;
static u32 s_t_last_try;
static u8  s_volatile[ST_SLOTS];
static i32 s_seen[ST_SLOTS][SEEN];
static u8  s_nseen[ST_SLOTS];
static i32 s_snap[ST_SLOTS], s_idle[ST_SLOTS];
static i32 s_stuck_logged;
static i32 s_progress_total;
static u32 s_t_busy;
static c8  s_items[MAX_ITEMS][8];
static i32 s_nitems;

i32 autoplay_active(void) {
    if (s_on < 0) {
        s_on = getenv("DKE_AUTOPLAY") != NULL;
        if (s_on) {
#ifdef _WIN32
            _putenv("DKE_AUTOADV=1");
#else
            setenv("DKE_AUTOADV", "1", 0);
#endif
            g_control_scheme = 1;            /* Ctrl alone fights */
        }
    }
    return s_on;
}

static u32 now(void) { return state ? state->current_game_time : 0; }
static void snap(i32 *dst) { memcpy(dst, lieu_state_slot(0), ST_BYTES); }

/* Only a value a slot never held counts as progress. */
static i32 seen_add(i32 i, i32 v) {
    for (i32 k = 0; k < s_nseen[i]; k++) if (s_seen[i][k] == v) return 0;
    if (s_nseen[i] >= SEEN) { s_volatile[i] = 1; return 0; }
    s_seen[i][s_nseen[i]++] = v;
    return 1;
}

/* Slots that change while the bot does nothing are timers / counters. */
static void learn_volatile(void) {
    const i32 *cur = lieu_state_slot(0);
    for (i32 i = 0; i < ST_SLOTS; i++)
        if (cur[i] != s_idle[i]) { s_volatile[i] = 1; seen_add(i, cur[i]); }
    snap(s_idle);
}

static i32 changed_since(const i32 *base, i32 *first) {
    const i32 *cur = lieu_state_slot(0);
    i32 n = 0;
    for (i32 i = 0; i < ST_SLOTS; i++) {
        if (s_volatile[i] || cur[i] == base[i]) continue;
        if (i == 0x0C / 4 || i == 0x04 / 4) continue;          /* action / up-down keys */
        if (!seen_add(i, cur[i])) continue;
        if (!n && first) *first = i * 4;
        n++;
    }
    return n;
}

static entity_t *enemy(void) {
    entity_t *c[2] = { g_companion_entity, g_secondary_companion_entity };
    for (i32 i = 0; i < 2; i++)
        if (c[i] && EDESC(c[i]) && E32(c[i], 0xD60) && *(i32 *)(EDESC(c[i]) + 0x84) > 0 && E32(c[i], 0x70) > 0)
            return c[i];
    return NULL;
}

static void teleport(entity_t *p, i32 x, i32 z, i32 head) {
    nav_snap_to_walkable(&x, &z);
    EROOT(p)->pos[0] = x;
    EROOT(p)->pos[2] = z;
    EROOT(p)->pos[1] = actor_floor_height(x, z, p);
    EROOT(p)->rot[2] = head;
    E32(p, 0xCEC) = 0;
    E32(p, 0xCF4) = 0;
}

static i32 room_ok(i32 r) { return r > 0 && r < ROOMS; }

/* Untried work left in room r (zones seen there so far). */
static i32 room_left(i32 r) {
    if (!room_ok(r)) return 0;
    if (!s_visits[r]) return 1000;                       /* never entered */
    i32 n = 0;
    for (i32 z = 1; z < ZONES; z++) {
        if (!s_room_zone[r][z] || (s_exit_known[r][z] && s_exit[r][z])) continue;
        for (i32 k = 0; k < HEADINGS + s_nitems; k++) n += !s_tried[r][z][k];
    }
    return n;
}

/* Next untried (zone, heading-or-item) in this room, exits excluded. */
static i32 next_try(void) {
    i32 r = s_level;
    for (i32 pass = 0; pass < 2; pass++) {
        for (i32 z = 1; z < ZONES; z++) {
            i32 cx, cz, sx, sz, act;
            if (!lieu_zone_get(z, &cx, &cz, &sx, &sz, &act)) continue;
            s_room_zone[r][z] = 1;
            if (s_exit_known[r][z] && s_exit[r][z]) continue;
            i32 lo = pass == 0 ? 0 : HEADINGS, hi = pass == 0 ? HEADINGS : HEADINGS + s_nitems;
            for (i32 k = lo; k < hi; k++) {
                if (s_tried[r][z][k]) continue;
                s_zone = z;
                s_head = k < HEADINGS ? k : 0;
                s_item = k < HEADINGS ? -1 : k - HEADINGS;
                s_is_exit = 0;
                s_tried[r][z][k] = 1;
                return 1;
            }
        }
    }
    return 0;
}

/* Room exhausted: take the exit toward the room with the most work left
 * (one hop; ties go to the least visited). */
static i32 pick_exit(void) {
    i32 r = s_level, best = -1, best_left = 0, best_vis = 1 << 30;
    for (i32 z = 1; z < ZONES; z++) {
        i32 cx, cz, sx, sz, act;
        if (!s_exit_known[r][z] || !s_exit[r][z]) continue;
        if (!lieu_zone_get(z, &cx, &cz, &sx, &sz, &act)) continue;
        i32 t = s_exit[r][z], left = room_left(t);
        if (left > best_left || (left == best_left && s_visits[t] < best_vis)) {
            best = z; best_left = left; best_vis = s_visits[t];
        }
    }
    if (best < 0) return 0;
    /* Nothing left anywhere next door: wander to the least visited room. */
    s_zone = best; s_head = 0; s_item = -1; s_is_exit = 1;
    return 1;
}

static void reopen_all(void) {
    memset(s_tried, 0, sizeof(s_tried));
    s_stuck_logged = 0;
}

void autoplay_tick(system_t *sys) {
    if (!autoplay_active() || !sys) return;
    entity_t *p = g_player_entity;
    if (!p || !EROOT(p) || !EDESC(p) || !state) return;
    u32 t = now();

    sys->actions[32] = 0;
    sys->actions[0x11] = 0;
    sys->actions[0x43] = 0;

    if (state->current_level_id != s_level) {
        i32 to = state->current_level_id;
        /* A room change within 3 s of standing in a zone: that zone is an exit. */
        if (room_ok(s_last_room) && s_last_zone > 0 && t - s_t_last_try < 3000) {
            s_exit_known[s_last_room][s_last_zone] = 1;
            s_exit[s_last_room][s_last_zone] = (i8)to;
        }
        print("auto: entered level %d from %d (progress so far %d)\n", to, s_level, s_progress_total);
        s_level = to;
        if (room_ok(to)) s_visits[to]++;
        memset(s_volatile, 0, sizeof(s_volatile));
        snap(s_idle);
        s_phase = PH_IDLE;
        s_t_phase = s_t_enter = t;
        s_last_zone = -1;
    }
    if (!room_ok(s_level)) return;

    if (!getenv("DKE_AUTOPLAY_MORTAL") && *(i32 *)(EDESC(p) + 0x84) < 400)
        *(i32 *)(EDESC(p) + 0x84) = 1000;

    entity_t *en = enemy();
    if (en) {                                    /* fight: draw, hold attack */
        if (!E32(p, 0xD60)) { if ((t / 100) & 1) sys->actions[0x43] = 1; }
        else sys->actions[0x11] = 1;
        s_t_phase = t;
        return;
    }
    if (dlg_is_active() || cjr_active() || g_player_movement_locked || lieu_player_blocked()) {
        if (!s_t_busy) s_t_busy = t;
        if (t - s_t_busy > 20000 && ((t / 500) & 1)) sys->actions[32] = 1;
        s_t_phase = t;
        return;
    }
    s_t_busy = 0;
    if (E32(p, 0xD60)) { if ((t / 100) & 1) sys->actions[0x43] = 1; return; }  /* sheathe */

    switch (s_phase) {
    case PH_IDLE:
        if (t - s_t_phase < 300) return;
        learn_volatile();
        s_nitems = inv_list(s_items, MAX_ITEMS);
        if (!next_try() && !pick_exit()) {
            if (!s_stuck_logged) {
                s_stuck_logged = 1;
                print("auto: STUCK in level %d (no try or exit left, %d items)\n", s_level, s_nitems);
            }
            return;
        }
        {
            i32 cx, cz, sx, sz, act;
            lieu_zone_get(s_zone, &cx, &cz, &sx, &sz, &act);
            teleport(p, cx, cz, s_head * (0x10000 / HEADINGS));
        }
        s_last_room = s_level; s_last_zone = s_zone; s_t_last_try = t;
        snap(s_snap);
        s_phase = PH_MOVE;
        s_t_phase = t;
        return;
    case PH_MOVE:
        if (t - s_t_phase < 400) return;
        if (s_is_exit) {                         /* stand in the exit and wait */
            if (t - s_t_phase > 4000) {
                s_exit_known[s_level][s_zone] = 1;
                s_exit[s_level][s_zone] = 0;     /* did not leave: not an exit (now) */
                s_phase = PH_IDLE;
            }
            return;
        }
        if (s_item >= 0) {
            c8 a, b; i32 id = 0; c8 letter = 0;
            if (sscanf(s_items[s_item], "%c%c%02d%c", &a, &b, &id, &letter) >= 3) {
                g_used_object = id;
                g_used_variant = letter ? letter - 'A' : 0;
            }
        } else {
            sys->actions[32] = 1;
        }
        s_phase = PH_PRESS;
        s_t_phase = t;
        return;
    case PH_PRESS:
        s_phase = PH_WAIT;
        s_t_phase = t;
        return;
    case PH_WAIT: {
        if (t - s_t_phase < 1500) return;
        i32 first = -1, n = changed_since(s_snap, &first);
        if (n) {
            s_progress_total++;
            print("auto: progress L%d zone %d %s%s -> %d vars (first 0x%X)\n", s_level, s_zone,
                  s_item >= 0 ? "item " : "head ", s_item >= 0 ? s_items[s_item] : "", n, first);
            reopen_all();
        }
        snap(s_idle);
        s_phase = PH_IDLE;
        s_t_phase = t;
        return;
    }
    }
}
