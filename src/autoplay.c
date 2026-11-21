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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autoplay.h"
#include "actor.h"
#include "entity.h"
#include "state.h"
#include "lieu_cb.h"
#include "lieu_action.h"
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

enum { PH_IDLE, PH_WALK, PH_MOVE, PH_PRESS, PH_WAIT };

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
static u8  s_unreach[ROOMS][ZONES];      /* walk gave up this epoch */
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
            if (s_unreach[r][z]) continue;
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
        if (!s_exit_known[r][z] || !s_exit[r][z] || s_unreach[r][z]) continue;
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

/* First zone containing (x,z), as lieu_detect_zone tests them. */
static i32 zone_at(i32 x, i32 z) {
    for (i32 i = 1; i < ZONES; i++) {
        i32 cx, cz, sx, sz, act;
        if (!lieu_zone_get(i, &cx, &cz, &sx, &sz, &act)) continue;
        if (abs(x - cx) <= sx / 2 && abs(z - cz) <= sz / 2) return i;
    }
    return -1;
}
static i32 s_cur_zone = -1;

/* ---- walking: BFS over the nav grid, steering with the arrow keys ---- */
#define NG NAV_GRID_SIZE
static i16 s_path[NG * NG][2];
static i32 s_plen, s_pi;
static i32 s_lastx, s_lastz;
static u32 s_t_moved, s_t_walk;

static i32 cell_of(i32 x, i32 z, i32 *cx, i32 *cz) {
    if (g_nav.cell_width <= 0 || g_nav.cell_height <= 0) return 0;
    *cx = (x - g_nav.world_x_origin) / g_nav.cell_width;
    *cz = (z - g_nav.world_y_origin) / g_nav.cell_height;
    return *cx >= 0 && *cx < NG && *cz >= 0 && *cz < NG;
}
static i32 s_avoid_target = -1;
static i32 s_clear;                            /* 1: keep one free cell away from walls (door posts snag the hero) */
static i32 raw_free(i32 cx, i32 cz) {
    return cx >= 0 && cx < NG && cz >= 0 && cz < NG && g_nav.grid[cz * NG + cx] == NAV_FREE;
}
static i32 cell_free(i32 cx, i32 cz) {
    if (!raw_free(cx, cz)) return 0;
    if (s_clear)
        for (i32 j = -s_clear; j <= s_clear; j++)
            for (i32 i = -s_clear; i <= s_clear; i++)
                if (!raw_free(cx + i, cz + j)) return 0;
    /* Do not cross a known exit on the way to something else. */
    i32 wx = g_nav.world_x_origin + cx * g_nav.cell_width + g_nav.cell_width / 2;
    i32 wz = g_nav.world_y_origin + cz * g_nav.cell_height + g_nav.cell_height / 2;
    i32 z = zone_at(wx, wz);
    if (z > 0 && z != s_avoid_target && room_ok(s_level) && s_exit_known[s_level][z] && s_exit[s_level][z]) return 0;
    return 1;
}
/* Path of cell centres from (px,pz) to the free cell nearest (tx,tz). */
static i32 plan1(i32 px, i32 pz, i32 tx, i32 tz, i32 *miss) {
    static i16 prev[NG * NG];
    static i32 q[NG * NG];
    i32 sx, sz, gx, gz;
    s_plen = s_pi = 0;
    if (!cell_of(px, pz, &sx, &sz) || !cell_of(tx, tz, &gx, &gz)) return 0;
    for (i32 i = 0; i < NG * NG; i++) prev[i] = -2;
    i32 head = 0, tail = 0, best = sz * NG + sx, bestd = 1 << 30;
    prev[best] = -1;
    q[tail++] = best;
    while (head < tail) {
        i32 c = q[head++], cx = c % NG, cz = c / NG;
        i32 d = (cx - gx) * (cx - gx) + (cz - gz) * (cz - gz);
        if (d < bestd) { bestd = d; best = c; }
        if (!d) break;
        static const i32 dx[8] = { 1, -1, 0, 0, 1, 1, -1, -1 }, dz[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
        for (i32 k = 0; k < 8; k++) {
            i32 nx = cx + dx[k], nz = cz + dz[k];
            if (!cell_free(nx, nz) || prev[nz * NG + nx] != -2) continue;
            if (k >= 4 && (!cell_free(cx + dx[k], cz) || !cell_free(cx, cz + dz[k]))) continue;
            prev[nz * NG + nx] = (i16)c;
            q[tail++] = nz * NG + nx;
        }
    }
    i32 n = 0;
    for (i32 c = best; c >= 0 && n < NG * NG; c = prev[c]) n++;
    s_plen = n;
    i32 i = n - 1;
    for (i32 c = best; c >= 0 && i >= 0; c = prev[c], i--) {
        s_path[i][0] = (i16)(c % NG);
        s_path[i][1] = (i16)(c / NG);
    }
    s_pi = 1;                                    /* [0] is the start cell */
    *miss = bestd;
    return 1;
}
static i32 plan(i32 px, i32 pz, i32 tx, i32 tz) {
    i32 miss = 1 << 30;
    i32 ok = 0;
    for (s_clear = 5; s_clear >= 0; s_clear -= (s_clear > 2 ? 2 : 1)) {   /* 5, 3, 2, 1, 0 cells of clearance */
        ok = plan1(px, pz, tx, tz, &miss);
        if (ok && miss <= 2) break;
    }
    s_clear = 0;
    return ok;
}

/* One frame of walking toward (tx,tz): 1 arrived, -1 gave up, 0 walking. */
static i32 walk(system_t *sys, entity_t *p, i32 tx, i32 tz, i32 hx, i32 hz, u32 t) {
    i32 x = EROOT(p)->pos[0], z = EROOT(p)->pos[2];
    if (abs(x - tx) <= hx && abs(z - tz) <= hz) return 1;
    if (getenv("DKE_AUTOPLAY_DBG")) {
        static u32 dt;
        if (t - dt > 500) {
            dt = t;
            print("auto: walk pos (%d,%d) rot %04X -> (%d,%d) cEC=%d cF4=%d d04=%d d60=%d lock=%d\n", x, z, EROOT(p)->rot[2] & 0xFFFF, tx, tz,
                  E32(p, 0xCEC), E32(p, 0xCF4), E32(p, 0xD04), E32(p, 0xD60), g_player_movement_locked);
        }
    }
    if (abs(x - s_lastx) + abs(z - s_lastz) > 150) { s_lastx = x; s_lastz = z; s_t_moved = t; }
    static u32 esc_until, esc_n;
    if (t - s_t_moved > 1200 && t >= esc_until && esc_n < 6) { esc_until = t + 1300; esc_n++; s_t_moved += 1300; }  /* sidestep a snag */
    if (t > esc_until + 3000) esc_n = 0;
    if (t - s_t_moved > 2500 || t - s_t_walk > 40000) return -1;
    i32 wx = tx, wz = tz;
    while (s_pi < s_plen) {
        wx = g_nav.world_x_origin + s_path[s_pi][0] * g_nav.cell_width + g_nav.cell_width / 2;
        wz = g_nav.world_y_origin + s_path[s_pi][1] * g_nav.cell_height + g_nav.cell_height / 2;
        if (abs(x - wx) < g_nav.cell_width * 2 / 3 && abs(z - wz) < g_nav.cell_height * 2 / 3) { s_pi++; continue; }
        break;
    }
    if (s_pi >= s_plen) { wx = tx; wz = tz; }
    f64 dx = (f64)(wx - x), dz = (f64)(wz - z);
    i32 want = (i32)(-atan2(dx, dz) * 65536.0 / (2.0 * 3.14159265358979)) & 0xFFFF;
    if (t < esc_until) want = (want + (esc_n & 1 ? 0x3000 : -0x3000)) & 0xFFFF;
    i32 d = (want - (EROOT(p)->rot[2] & 0xFFFF)) & 0xFFFF;
    if (d > 0x8000) d -= 0x10000;
    if (abs(d) > 0x1800) EROOT(p)->rot[2] = want;  /* big turns: face at once, keys are slow */
    else if (abs(d) > 0x400) sys->actions[d > 0 ? 39 : 37] = 1;
    sys->actions[38] = 1;                        /* Up: walk */
    return 0;
}

static void reopen_all(void) {
    memset(s_unreach, 0, sizeof(s_unreach));
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
    sys->actions[37] = sys->actions[38] = sys->actions[39] = sys->actions[40] = 0;

    if (state->current_level_id != s_level) {
        i32 to = state->current_level_id;
        /* A room change within 3 s of standing in a zone: that zone is an exit. */
        if (room_ok(s_last_room) && s_cur_zone > 0) {
            s_exit_known[s_last_room][s_cur_zone] = 1;
            s_exit[s_last_room][s_cur_zone] = (i8)to;
        }
        print("auto: entered level %d from %d (progress so far %d)\n", to, s_level, s_progress_total);
        s_level = to;
        if (room_ok(to)) s_visits[to]++;
        memset(s_volatile, 0, sizeof(s_volatile));
        snap(s_idle);
        s_phase = PH_IDLE;
        s_t_phase = s_t_enter = t;
        s_last_zone = -1;
        s_cur_zone = -1;
    }
    if (!room_ok(s_level)) return;
    s_last_room = s_level;
    {
        i32 z = zone_at(EROOT(p)->pos[0], EROOT(p)->pos[2]);
        if (z > 0) s_cur_zone = z;
        else if (!lieu_player_blocked()) s_cur_zone = -1;      /* keep it through the exit script */
    }

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
        static u32 s_t_report;
        if (t - s_t_busy > 30000 && t - s_t_report > 30000) {
            s_t_report = t;
            print("auto: WAITING %us in level %d: dlg=%d cjr=%d locked=%d blocked=%d\n", (t - s_t_busy) / 1000, s_level,
                  dlg_is_active(), cjr_active(), g_player_movement_locked, lieu_player_blocked());
            lieu_action_dump_heads();
        }
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
        if (getenv("DKE_AUTOPLAY_TELEPORT")) {
            i32 cx, cz, sx, sz, act;
            lieu_zone_get(s_zone, &cx, &cz, &sx, &sz, &act);
            teleport(p, cx, cz, s_head * (0x10000 / HEADINGS));
            s_last_room = s_level; s_last_zone = s_zone; s_t_last_try = t;
            snap(s_snap);
            s_phase = PH_MOVE;
        } else {
            i32 cx, cz, sx, sz, act;
            lieu_zone_get(s_zone, &cx, &cz, &sx, &sz, &act);
            s_avoid_target = s_zone;
            plan(EROOT(p)->pos[0], EROOT(p)->pos[2], cx, cz);
            s_lastx = EROOT(p)->pos[0]; s_lastz = EROOT(p)->pos[2];
            s_t_moved = s_t_walk = t;
            s_last_room = s_level; s_last_zone = s_zone; s_t_last_try = t;
            s_phase = PH_WALK;
        }
        s_t_phase = t;
        return;
    case PH_WALK: {
        i32 cx, cz, sx, sz, act;
        if (!lieu_zone_get(s_zone, &cx, &cz, &sx, &sz, &act)) { s_phase = PH_IDLE; return; }
        s_t_last_try = t;
        i32 hx = sx / 2 > 300 ? sx / 2 - 200 : sx / 2, hz = sz / 2 > 300 ? sz / 2 - 200 : sz / 2;
        i32 r = walk(sys, p, cx, cz, hx, hz, t);
        if (r < 0) {
            s_unreach[s_level][s_zone] = 1;
            i32 cx, cz, sx, sz, act, pcx = -1, pcz = -1;
            lieu_zone_get(s_zone, &cx, &cz, &sx, &sz, &act);
            cell_of(EROOT(p)->pos[0], EROOT(p)->pos[2], &pcx, &pcz);
            print("auto: L%d zone %d unreachable from (%d,%d) cell (%d,%d)=%d, zone at (%d,%d) path %d/%d, grid origin (%d,%d) cell %dx%d\n",
                  s_level, s_zone, EROOT(p)->pos[0], EROOT(p)->pos[2], pcx, pcz,
                  (pcx >= 0 && pcx < NG && pcz >= 0 && pcz < NG) ? g_nav.grid[pcz * NG + pcx] : -1, cx, cz, s_pi, s_plen,
                  g_nav.world_x_origin, g_nav.world_y_origin, g_nav.cell_width, g_nav.cell_height);
            if (getenv("DKE_AUTOPLAY_DBG") && pcx >= 0)
                for (i32 dz = -5; dz <= 5; dz++) {
                    c8 row[32]; i32 k = 0;
                    for (i32 dx = -8; dx <= 8; dx++) {
                        i32 x = pcx + dx, z = pcz + dz;
                        row[k++] = (x < 0 || x >= NG || z < 0 || z >= NG) ? ' ' :
                            (dx == 0 && dz == 0) ? '@' : " .#B"[g_nav.grid[z * NG + x] == NAV_FREE ? 1 : g_nav.grid[z * NG + x] == NAV_WALL ? 2 : 3];
                    }
                    row[k] = 0;
                    print("auto:   %s\n", row);
                }
            s_phase = PH_IDLE;
        } else if (r > 0) {
            EROOT(p)->rot[2] = s_head * (0x10000 / HEADINGS);
            snap(s_snap);
            s_phase = PH_MOVE;
            s_t_phase = t;
        }
        return;
    }
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
