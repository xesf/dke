/*
 * lieu_cb.c — Engine-side bridge for the LIEU level-scripting hand-port.
 *
 * Provides:
 *   - the 40-slot callback vtable g_lieu_cb that lieu01.c calls into,
 *   - the per-level state buffer (engine dword_505C40, 0xFA0 bytes),
 *   - lieu_enter / lieu_tick / lieu_exit for scene.c to drive.
 *
 * Two kinds of callback live here:
 *
 *  - DEFERRED. The original primitives (dkev sub_444xxx..sub_446xxx) do not
 *    act: they allocate an action node, fill it, and let the per-frame
 *    driver sub_44CDB8 @ 0x44CDB8 dispatch it one head node per pile per
 *    frame. Those now go through src/lieu_action.c and land in
 *    lieu_action_dispatch() at the bottom of this file — the port of the
 *    40-case switch in sub_44A58B @ 0x44A58B.
 *
 *  - IMMEDIATE. A handful of callbacks are queued in the original but are
 *    still applied straight away here: zones (AJOUTEZONE/RETIREZONE),
 *    camera (CHANGECAMERA), room change (CHANGEPIECE), persona placement
 *    (POSPERSO), dialogue, cutscenes and the BLOQUE brackets. INIT emits a
 *    burst of those and the scene code consumes them the same frame; moving
 *    them behind the queue would stagger level setup over ~40 frames. They
 *    stay immediate until the scene side can cope with staggered setup.
 *
 * Effects that need engine subsystems dke has not ported (positional audio,
 * 2D anim playback, object meshes) record their state in the small
 * registries below and log once, so scripts still advance.
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "libsys/common.h"
#include "lieu_cb.h"
#include "can.h"
#include "game_init.h"
#include "anim2d.h"
#include "object.h"
#include "sfx.h"
#include "screen_fx.h"
#include "inventory.h"
#include "scene.h"
#include "cjr.h"
#include "effect.h"
#include "world.h"
#include "music.h"
#include "libsys/file_system.h"
#include "lieu_action.h"
#include "levels/levels.h"
#include "player.h"
#include "dialogue.h"
#include "combat.h"
#include "menu/screens.h"
#include "actor.h"
#include "state.h"

/* -------------------------------------------------------------------------
 * Per-level state buffer + owned vtable pointer.
 * ---------------------------------------------------------------------- */
static lieu_state_t g_lieu_state[0xFA0];   /* dword_505C40, 4000 bytes, game-wide */
static i32          g_lieu_level = 0;      /* active level id (0 = none)   */

const lieu_cb_t *g_lieu_cb = NULL;         /* storage (extern in header)   */

/* -------------------------------------------------------------------------
 * Zone table (engine dword_506C80[41], stride 0x24). Populated by
 * AjouteZone (slot 12 = spawn_978) / cleared by RetireZone (slot 13 =
 * cb_93C). Zone id == LIEU trigger index (1..40). Feeds op-3 dispatch and
 * the query_990/944 callbacks. Only static (non-entity) zones for now.
 * ---------------------------------------------------------------------- */
typedef struct {
    i32 center_x;   /* world units; -1 = empty slot */
    i32 center_z;
    i32 size_x;     /* full extent (half = /2) */
    i32 size_z;
    i32 active;     /* 1 = always tested; 0 = only when g_force_zones */
    void *entity;   /* dword_506CA0: actor the zone follows (POSPERSO), or NULL */
} zone_t;

#define LIEU_ZONE_COUNT 41
#define LIEU_ZONE_SCALE 600            /* AjouteZone tile->world (0x258)   */
static zone_t g_zones[LIEU_ZONE_COUNT];
static i32    g_force_zones = 0;       /* dword_4FFB80: inactive zones answer the action key */
static i32    g_action_key  = 0;       /* key 0x20 (Space) held this frame */
static i32    g_action_pressed = 0;    /* dword_505C4C */

/* sub_449F9F feeds the action key in; scene.c calls this every frame. */
static i32 g_updown_key = 0;           /* arrow up/down held (dword_505C44) */
void lieu_set_action_key(i32 down) { g_action_key = down; }
void lieu_set_updown_key(i32 down) { g_updown_key = down; }

/* dword_4B52DC: -1 while the level script's INIT burst is being drained. */
static i32 g_lieu_loading = 0;
i32 lieu_loading(void) { return g_lieu_loading; }

static void zones_clear(void) {
    for (i32 i = 0; i < LIEU_ZONE_COUNT; i++) {
        g_zones[i].center_x = -1;
        g_zones[i].entity   = NULL;
    }
}

/* sub_463FB5 @ 0x464019: POSPERSO hangs a zone on the placed actor. The zone
 * then moves with it (sub_449E03 reads the actor's position as the centre). */
void lieu_zone_attach(i32 zone, void *entity, i32 size, i32 active) {
    if (zone < 0 || zone >= LIEU_ZONE_COUNT) return;
    g_zones[zone].entity = entity;
    g_zones[zone].size_x = size;
    g_zones[zone].size_z = size;
    g_zones[zone].active = active;
}

/* A zone attached to an actor (dword_506CA0) sits on that actor: refresh
 * its centre so the distance queries (query_990, layer_op 9) see it move. */
static void zones_follow(void) {
    for (i32 i = 1; i < LIEU_ZONE_COUNT; i++) {
        entity_t *e = (entity_t *)g_zones[i].entity;
        if (!e || !EROOT(e)) continue;
        g_zones[i].center_x = EROOT(e)->pos[0];
        g_zones[i].center_z = EROOT(e)->pos[2];
    }
}

void lieu_zone_detach(void *entity) {
    for (i32 i = 1; i < LIEU_ZONE_COUNT; i++)
        if (g_zones[i].entity == entity) g_zones[i].entity = NULL;
}

void lieu_zone_debug(void) {
    i32 minx = 0x7FFFFFFF, maxx = -0x7FFFFFFF;
    i32 minz = 0x7FFFFFFF, maxz = -0x7FFFFFFF;
    i32 n = 0;

    for (i32 i = 1; i < LIEU_ZONE_COUNT; i++) {
        if (g_zones[i].center_x == -1) continue;
        print("zone[%d] c=(%d,%d) half=(%d,%d) active=%d\n", i,
              g_zones[i].center_x, g_zones[i].center_z,
              g_zones[i].size_x / 2, g_zones[i].size_z / 2, g_zones[i].active);
        if (g_zones[i].center_x < minx) minx = g_zones[i].center_x;
        if (g_zones[i].center_x > maxx) maxx = g_zones[i].center_x;
        if (g_zones[i].center_z < minz) minz = g_zones[i].center_z;
        if (g_zones[i].center_z > maxz) maxz = g_zones[i].center_z;
        n++;
    }

    /*
     * Coordinate-space check. AjouteZone rectangles, PosPerso placements and
     * the player transform must all live in the same space: LIEU script tile
     * x600 (LIEU_ZONE_SCALE, engine 0x258 — the same divisor the HUD uses).
     * Printing the three side by side makes any future drift obvious instead
     * of showing up as triggers that never fire.
     */
    if (n) {
        i32 px = 0, pz = 0;
        i32 have_player = player_get_pos(&px, NULL, &pz);
        print("lieu: zone space X[%d..%d] Z[%d..%d] over %d zones\n",
              minx, maxx, minz, maxz, n);
        for (i32 i = 0; i < lieu_spawn_count(); i++) {
            i32 tx = 0, tz = 0, view = 0;
            const c8 *nm = lieu_spawn_get(i, &tx, &tz, &view);
            print("lieu: spawn '%s' world=(%d,%d)\n", nm ? nm : "?",
                  tx * LIEU_ZONE_SCALE, tz * LIEU_ZONE_SCALE);
        }
        /* The player transform is seeded from the camera anchor after INIT,
         * so at room-entry time it is still (0,0) — report that rather than
         * a misleading "outside every zone". */
        if (have_player && (px || pz))
            print("lieu: player world=(%d,%d) zone=%d\n", px, pz, lieu_detect_zone());
        else if (have_player)
            print("lieu: player not placed yet (transform seeded after INIT)\n");
    }
}

/* -------------------------------------------------------------------------
 * Facing tests — DirectionPerso / DirectionZone / DirectionSpec.
 *
 * These gate most script triggers: "is the player looking at the thing?".
 * Decoded from sub_44599B @ 0x44599B (DirectionPerso) and sub_445C82 @
 * 0x445C82 (DirectionZone), which share the same maths:
 *
 *   target_ang = -(atan2(dx, dz) * 65536 / 2pi) & 0xFFFF
 *   diff       = signed shortest arc (target_ang - player_ang)
 *   facing     = abs(diff) < 0x2400            (+/- 50.6 degrees)
 *
 * Note the atan2 argument order: the engine passes the X delta FIRST
 * (asm 0x445BBE pushes dz then dx, so dx lands as argument 1).
 *
 * DirectionPerso additionally returns true regardless of angle when the
 * target is within 30000 world units (asm 0x445A8B compares the squared
 * distance against dbl_4B0678 = 9e8). DirectionZone has no such bypass.
 * ---------------------------------------------------------------------- */
#define LIEU_FACE_TOLERANCE  0x2400      /* +/- 50.6 degrees, BAM        */
#define LIEU_FACE_NEAR_SQ    900000000.0 /* dbl_4B0678: (30000 units)^2  */

/* Signed shortest-arc compare against the tolerance. */
static i32 facing_ok(i32 player_ang, i32 target_ang) {
    i32 d = (target_ang - player_ang) & 0xFFFF;
    if (d > 0x8000) d -= 0x10000;
    return (d < 0 ? -d : d) < LIEU_FACE_TOLERANCE;
}

/* BAM angle toward (dx,dz). The engine passes the X delta as atan2's first
 * argument (sub_44599B @ 0x445BBE), so keep that order here too. */
static i32 bam_angle(i32 dx, i32 dz) {
    double a = atan2((double)dx, (double)dz) * 65536.0 / (2.0 * M_PI);
    return (i32)(-(i32)a) & 0xFFFF;
}

/* Player heading in BAM. scene.c keeps the live heading in cosine-table
 * units (4096 per turn) and mirrors it into entity+0x1E0. */
static i32 player_facing_bam(void) {
    return (i32)player_get_facing() & 0xFFFF;
}

/* Log a callback the first time it fires (avoid per-frame spam). */
#define CBLOG(name)                                                          \
    do { static i32 w = 0; if (!w) { w = 1;                                  \
        print("lieu_cb: %s (stub)\n", name); } } while (0)

/* -------------------------------------------------------------------------
 * Player movement blocking (group open/close brackets).
 * group(3) = open bracket (block player), group(0) = close bracket.
 * Original engine queues these as actions, but we apply directly since
 * dke has no action queue processor.
 * ---------------------------------------------------------------------- */
static i32 g_player_blocked = 0;   /* player_movement_enabled @ 0x4FF080 */
static i32 g_pile_actor[3];        /* dword_4B53F0: last ANIMATION actor per pile */
static i32 g_dlg_pile = 0;         /* dword_4FF09C: pile that queued the line  */
i32 lieu_player_blocked(void) { return g_player_blocked; }

/* -------------------------------------------------------------------------
 * State toggle layers (3 independent dialogue/scripting queues).
 * Original: mode 0→dword_4FC194, mode 1→dword_507274, mode 2→dword_507270.
 * Each has a mode byte (0/1/2) and a counter. layer=2 means "reset".
 * ---------------------------------------------------------------------- */
static i32 g_layer_mode[3]    = {0, 0, 0};
static i32 g_layer_counter[3] = {0, 0, 0};

/* -------------------------------------------------------------------------
 * Entity state tracking. The original engine tracks entity state in
 * per-entity structs (0x220-stride objects at dword_4DF5D0 for IDs 0-3,
 * separate NPC array for higher IDs). LIEU queries entity alive, HP,
 * visibility, combat readiness, and anim state via layer_op.
 *
 * We track a lightweight version: alive, HP (0x400 = full), visibility,
 * combat ready, anim state per entity. Spawn/kill callbacks update this.
 * ---------------------------------------------------------------------- */
#define ENT_MAX 256
typedef struct {
    i32 alive;        /* 1 = loaded/present */
    i32 hp;           /* 0..0x3FF (1023); layer_op(6) reads/writes */
    i32 visible;      /* 1 = visible; layer_op(12) toggles */
    i32 combat_ready; /* 1 = in combat mode; layer_op(18) tests */
    i32 anim_state;   /* layer_op(6) r/w with entity anim tag */
    i32 walking;      /* JOINDRE in flight (engine: entity+0xCBC != 0) */
    i32 joined;       /* JOINDREPERM in flight (engine: entity+0xCB4 != 0) */
    /* Walk destination handed over by JOINDRE (engine: entity+0xCC0/0xCC4/
     * 0xCC8, consumed by the entity movement system). */
    i32 dest_x, dest_z, dest_facing;
    u32 walk_ms;      /* last time the walk stepper advanced this actor */
} ent_state_t;
static ent_state_t g_ents[ENT_MAX];

/* Engine tests entity+0xCBC / +0xCB4 before accepting a new join
 * (sub_4453E1 @ 0x445446, sub_446808 @ 0x446871). */
static i32 ent_is_walking(i32 id) {
    return (id >= 0 && id < ENT_MAX) ? g_ents[id].walking : 0;
}
static i32 ent_is_joined(i32 id) {
    return (id >= 0 && id < ENT_MAX) ? g_ents[id].joined : 0;
}

static void ents_clear(void) {
    for (i32 i = 0; i < ENT_MAX; i++) {
        g_ents[i].alive = 0;
        g_ents[i].hp = 0x200;
        g_ents[i].visible = 1;
        g_ents[i].combat_ready = 0;
        g_ents[i].anim_state = 0;
        g_ents[i].walking = 0;
        g_ents[i].joined = 0;
        g_ents[i].dest_x = 0;
        g_ents[i].dest_z = 0;
        g_ents[i].dest_facing = 0;
        g_ents[i].walk_ms = 0;
    }
    for (i32 i = 0; i < 4; i++) { g_ents[i].alive = 1; g_ents[i].hp = 0x3FF; }
    g_ents[12].alive = 1;
    g_ents[12].hp = 0x3FF;
}

static void ent_spawn(i32 id) {
    if (id >= 0 && id < ENT_MAX) {
        g_ents[id].alive = 1;
        if (g_ents[id].hp <= 0) g_ents[id].hp = 0x200;
        g_ents[id].visible = 1;
    }
}

static void ent_kill(i32 id) {
    if (id >= 0 && id < ENT_MAX) {
        g_ents[id].alive = 0;
        g_ents[id].hp = 0;
        g_ents[id].combat_ready = 0;
    }
}

static i32 g_op9c_counter = 0;

/* -------------------------------------------------------------------------
 * Callback implementations. Signatures match lieu_cb_t exactly.
 * ---------------------------------------------------------------------- */

/*
 * slot 0 sub_44453E @ 0x44453E "Animation" — queue an ANIMATION action.
 *
 * asm: strcpy + _strlwr the name, then dispatch on its first character:
 *   '2'  2D sprite: build "<prefix><name>.ANI", register it in the anim2d
 *        list (dword_505A90 / byte_505AA0, cap 20 "too many anim2d"), load it
 *        via sub_41333A @ 0x41333A, node+0x0C = -2 (no owning entity).
 *   's'  sound-prefixed: drop the 's', node+0x70 = 1.
 *   'a'  actor animation; "ar..." holds for 500 ms (node+0x40 = 0x1F4),
 *        anything else completes as soon as the animation is handed over
 *        (node+0x40 = -1).
 * A "_NN" suffix is split off the name and stored as a number at node+0x5C.
 */
static void cb_animation(const c8 *name) {
    if (!name) {
        print("lieu_cb: Scripteur, tu es un bougre d'ane : mets un nom d'animation\n");
        return;
    }
    lieu_action_t *n = lieu_action_push(LA_ANIMATION);

    c8 buf[32];
    snprintf(buf, sizeof(buf), "%s", name);
    for (c8 *q = buf; *q; q++) if (*q >= 'A' && *q <= 'Z') *q += 32;   /* _strlwr */
    /* sub_443720 @ 0x443720 (sound/anim preload for the name) not ported. */
    n->flag70   = 0;
    n->duration = -1;
    memset(n->name, 0, 16);

    if (buf[0] == '2') {
        /* 2D animation: registered in the named list and loaded into slot
         * 30 (sub_41333A); the node waits on that slot (actor -2). */
        c8 up[32];
        snprintf(up, sizeof(up), "%s", name);
        anim2d_register_named(g_lieu_level, up);
        n->actor = -2;
        return;
    }
    if (buf[0] == 's') {                                   /* 444729 */
        memmove(buf, buf + 1, strlen(buf));
        n->flag70 = 1;
    }
    c8 *u = strchr(buf, '_');                              /* 444778 */
    if (u) { *u = '\0'; n->num5C = atoi(u + 1); }
    else   n->num5C = 0;

    if (buf[0] == 'a') {                                   /* 4447D4 */
        n->duration = (buf[1] == 'r') ? 0x1F4 : -1;
        entity_t *e = actor_find(0);
        if (e) {
            if (E32(e, 0xD60))                         actor_prepare_anim(e, 5, n->name);
            else if (e != g_player_entity)             actor_prepare_anim(e, -5, n->name);
            else                                       actor_prepare_anim(e, 0x3B, n->name);
        }
        n->actor = 0;
        n->p44   = 1;
    }
    if (buf[0] == 'p' || buf[0] == 'g') {                  /* 4448BF */
        c8 num[3] = { buf[1], buf[2], 0 };
        if (num[0] == '0') { num[0] = num[1]; num[1] = 0; }
        i32 id = atoi(num);
        if (id > 0 && id < 4) id = 0;                      /* heroes -> player */
        snprintf(n->name, sizeof(n->name), "%s", buf);
        n->actor = id;
    }
}

/* slot 1 sub_4449A5 "Dialogue": enqueue dialogue line. */
static void cb_dialogue(const c8 *name, i32 v) {      /* sub_4449A5 */
    lieu_action_t *n = lieu_action_push(LA_DIALOGUE);
    if (name) snprintf(n->name, sizeof(n->name), "%s", name);
    n->actor = v;
}

/*
 * slot 2 sub_444A3E @ 0x444A3E "Evenement" — queue an EVENEMENT action.
 * id lands at node+0x48, the delay at node+0x40. The dispatcher (case 2 of
 * sub_44A58B @ 0x44AD41) turns it into an object-timer entry: delay >= 0
 * arms a timer that fires FonctionEditor opcode 4 with `id` once it
 * expires, delay == -1 cancels the pending timer for that id.
 */
static void cb_evenement(i32 id, i32 p) {
    lieu_action_t *n = lieu_action_push(LA_EVENEMENT);
    n->id48     = id;
    n->duration = p;
}
/* Cinematique(name): the LIEU script asks to play cutscene <name> (e.g.
 * "SC22_500"). Map to "SCnn.CRH" and QUEUE it — the movie player must run
 * from the render phase (scene_render), not from here (called mid INIT /
 * mid update; playing synchronously re-enters the frame loop). */
static c8  g_lieu_movie[64];
static i32 g_lieu_movie_pending = 0;
/* RetireZone / Fouille clear BOTH the centre (dword_506C80 = -1) and the
 * attached entity (dword_506CA0 = -1). The port's cb_93C clears only the
 * centre, so a zone hung on an actor by POSPERSO survives (zones_follow puts
 * the centre back the next frame). */
static void zone_clear(i32 i) {
    if (i < 0 || i >= LIEU_ZONE_COUNT) return;
    g_zones[i].center_x = -1;
    g_zones[i].entity   = NULL;
}



static i32 g_last_zone = 0;          /* dword_50725C */
i32 g_cam_lock = 0;                  /* dword_4FF0A0: script holds the camera */
i32 g_combat_flags = 0;              /* debug_flags: COMBAT mode (bit 0 hold, bit 1 unhurt) */
extern i32 g_hero_autofight;         /* dword_4B219C (combat_ai.c) */
extern i32 g_enemy_hits;             /* dword_506520 */
static i32 g_game_exit_code = 0;     /* game_exit_code */
u8         g_movie_seen[8];          /* byte_507350: bit n = movie SCnn seen */

/* slot 3 sub_444AC0 "Cinematique": QUEUES type 3, strcpy(node+0x30, name).
 * The port played it straight away; it must wait its turn in the pile
 * (L01 queues e.g. "Sar_50" then "SC21_500"). */
static void cb_cinematique(const c8 *name) {
    lieu_action_t *n = lieu_action_push(LA_CINEMA);
    if (name) snprintf(n->name, sizeof(n->name), "%s", name);
}

/* Drained by scene_render: returns a queued cutscene path (or NULL). */
const c8 *lieu_poll_movie(void) {
    if (!g_lieu_movie_pending) return NULL;
    g_lieu_movie_pending = 0;
    return g_lieu_movie;
}
/* slot 5 sub_44502D "PosPerso": position persona <name> at (x,y), view v.
 * The engine enqueues this as an action (dword_4FFB50) processed by the
 * persona/action system. dke has no persona render yet, so we record each
 * placement into a registry (for a future render pass) and log it. */
/* PosPerso arg mapping (verified vs engine sub_44502D/sub_463C2E):
 *   anim(p1) -> world X (tile, x600),  x(p2) -> world Z (tile, x600),
 *   y(p3) = facing (NOT position),     flag(p4) = vie (0..4 mode). */
typedef struct {
    c8  name[24];
    i32 wx, wz, view, id;   /* wx=anim tile (*600=worldX), wz=x tile (*600=worldZ) */
    i32 ground;             /* PosPerso arg8 z = world Y ground (unscaled)         */
    i32 facing;             /* PosPerso arg4 y = facing angle                      */
} lieu_spawn_t;
#define LIEU_SPAWN_MAX 64
static lieu_spawn_t g_spawns[LIEU_SPAWN_MAX];
static i32          g_spawn_count = 0;

/* PosPerso (sub_44502D): anim*600=worldX, x*600=worldZ, y=facing, z=world Y. */
static void cb_pos_perso(i32 zone, i32 anim, i32 x, i32 y, i32 flag,
                         const c8 *name, i32 kind, i32 z) {
    (void)kind;
    if (name && g_spawn_count < LIEU_SPAWN_MAX) {
        lieu_spawn_t *s = &g_spawns[g_spawn_count++];
        i32 k = 0;
        while (name[k] && k < (i32)sizeof(s->name) - 1) { s->name[k] = name[k]; k++; }
        s->name[k] = '\0';
        s->wx = anim; s->wz = x; s->view = flag; s->id = zone;
        s->ground = z; s->facing = y;
    }
    /* sub_44502D @ 0x44502D queues the placement; case 5 of sub_44A58B runs
     * sub_4640BC with these fields. */
    {
        lieu_action_t *n = lieu_action_push(LA_POSPERSO);
        if (name) {
            snprintf(n->name, sizeof(n->name), "%s", name);
            for (c8 *q = n->name; *q; q++) if (*q >= 'A' && *q <= 'Z') *q += 32;
        }
        n->actor = zone;                    /* persona id   (+0x0C)        */
        n->x     = anim * LIEU_ZONE_SCALE;  /* tile X * 600 (+0x14)        */
        n->z     = x * LIEU_ZONE_SCALE;     /* tile Z * 600 (+0x1C)        */
        n->p20   = y;                       /* facing       (+0x20)        */
        if (flag < 0 || flag > 4)
            print("lieu_cb: Bad parameter vie in POSPERSO function\n");
        n->p4C   = flag;                    /* vie          (+0x4C)        */
        n->p50   = kind;                    /* zone         (+0x50)        */
        n->p2C   = z;                       /* zone radius  (+0x2C)        */
    }
    /* Mark entity as alive/spawned. vie==2 means dead/cadaver placement. */
    if (zone >= 0 && zone < ENT_MAX) {
        if (flag == 2)
            ent_kill(zone);
        else
            ent_spawn(zone);
    }
    print("lieu_cb: PosPerso '%s' id=%d vie=%d tileXZ=(%d,%d) facing=%d zone=%d radius=%d\n",
          name ? name : "(null)", zone, flag, anim, x, y, kind, z);
}

/* Ground Y + facing for spawn i (see lieu_spawn_get for name/tile/view). */
i32 lieu_spawn_ground(i32 i) {
    return (i >= 0 && i < g_spawn_count) ? g_spawns[i].ground : 0;
}
i32 lieu_spawn_facing(i32 i) {
    return (i >= 0 && i < g_spawn_count) ? g_spawns[i].facing : 0;
}

/* Registry accessors: wx and wz = world tile coords (multiply by 600 for
 * world units, then project). */
i32 lieu_spawn_count(void) { return g_spawn_count; }
const c8 *lieu_spawn_get(i32 i, i32 *wx, i32 *wz, i32 *view) {
    if (i < 0 || i >= g_spawn_count) return NULL;
    if (wx)   *wx   = g_spawns[i].wx;
    if (wz)   *wz   = g_spawns[i].wz;
    if (view) *view = g_spawns[i].view;
    return g_spawns[i].name;
}
/*
 * slot 6 sub_445189 @ 0x445189 "EtatObjet" — queue an ETATOBJET action.
 * asm field map (name is _strupr'd first):
 *   node+0x30 = name      node+0x14 = p  * 600   (world X)
 *   node+0x0C = a6        node+0x18 = a1 * 600   (world Y)
 *   node+0x54 = a7        node+0x1C = a2 * 600   (world Z)
 *   node+0x20 = a3        node+0x24 = a4         node+0x28 = a5
 */
static void cb_etat_objet(const c8 *name, i32 p, i32 a1, i32 a2, i32 a3,
                            i32 a4, i32 a5, i32 a6, i32 a7) {
    lieu_action_t *n = lieu_action_push(LA_ETATOBJET);
    if (name) {
        i32 k = 0;
        while (name[k] && k < (i32)sizeof(n->name) - 1) {
            c8 ch = name[k];
            if (ch >= 'a' && ch <= 'z') ch -= 32;        /* _strupr */
            n->name[k] = ch;
            k++;
        }
        n->name[k] = '\0';
    }
    n->x     = p  * LIEU_ZONE_SCALE;
    n->y     = a1 * LIEU_ZONE_SCALE;
    n->z     = a2 * LIEU_ZONE_SCALE;
    n->p20   = a3;
    n->p24   = a4;
    n->p28   = a5;
    n->actor = a6;
    n->p54   = a7;
}

/*
 * slot 8 sub_4453E1 @ 0x4453E1 "Joindre" — walk an actor to a point.
 *
 * Signature is (ecx = actor, edx = tile X, a2 = tile Z, a3, a4): the second
 * argument is a COORDINATE, not a facing angle — asm 0x4454A7/0x4454BC scale
 * both by 600 the same way PosPerso does. A negative actor id forces the
 * join even while the actor is already walking; the engine otherwise drops
 * the request (asm 0x445446: entity+0xCBC != 0 && actor >= 0 -> return).
 *
 * asm: node+0x0C = abs(actor)  node+0x14 = X*600  node+0x1C = Z*600
 *      node+0x10 = a4          node+0x24 = a3
 */
static void cb_joindre(i32 actor, i32 tile_x, i32 tile_z, i32 a3, i32 a4) {
    i32 id = actor < 0 ? -actor : actor;

    /* Engine guard: skip if this actor already has a walk in flight and the
     * caller did not force it with a negative id. */
    if (actor >= 0 && ent_is_walking(id)) return;

    lieu_action_t *n = lieu_action_push(LA_JOINDRE);
    n->actor = id;
    n->x     = tile_x * LIEU_ZONE_SCALE;
    n->z     = tile_z * LIEU_ZONE_SCALE;
    n->p10   = a4;
    n->p24   = a3;
}
/* ChangeCamera (slot 9, dkev sub_4454F2): a = camera index, b = mode
 * (1 = auto-follow, 2 = forced/locked). The LIEU script drives the in-game
 * camera through this (e.g. cb_458(6,1) sets the opening bedroom view).
 * Queue it; the scene loop applies it via lieu_poll_camera_change. */
static i32 g_next_cam = -1;      /* -1 none, -2 mode change only */
static i32 g_next_cam_mode = 1;  /* byte_4B7280 */
static void cb_change_camera(i32 a, i32 b) {                       /* sub_4454F2 */
    if (a < 0) {
        /* Immediate: mode b; mode 2 locks camera -a-1 (color_mode). */
        g_next_cam_mode = b;
        g_next_cam = (b == 2) ? -a - 1 : -2;
        if (b == 2) LIEU_ST(g_lieu_state, 0xF90) = -a - 1;  /* dword_506BD0 */
        return;
    }
    lieu_action_t *n = lieu_action_push(LA_CHANGECAMERA);
    n->p58  = a;
    n->num5C = b;
}

/* Pending camera the script requested (or -1). Clears on read; *mode gets the
 * ChangeCamera mode (2 = forced -> disable auto-switch until next change). */
i32 lieu_poll_camera_change(i32 *mode) {
    i32 c = g_next_cam;
    if (c >= 0) { if (mode) *mode = g_next_cam_mode; g_next_cam = -1; }
    return c;
}
/* CHANGEPIECE (slot 10, sub_4455EE @ 0x4455EE): queue a move to room `a`
 * through entry `b` (node +0x08 / +0x5C). A negative room only sets the
 * entry point (dword_4B52E0). Case 10 of the dispatcher performs it. */
static i32 g_next_level = -1;
static i32 g_next_entry = 0;      /* dword_4B52E0 */
static void cb_change_piece(i32 a, i32 b) {
    if (a < 0) { g_next_entry = b; return; }
    lieu_action_t *n = lieu_action_push(LA_CHANGEPIECE);
    n->p58   = a;                 /* +0x08 in the original node */
    n->num5C = b < 0 ? 0 : b;
}

/* Pending room change target (or -1). Clears on read; fills *entry. */
i32 lieu_poll_level_change(i32 *entry) {
    i32 t = g_next_level;
    if (t >= 0) { if (entry) *entry = g_next_entry; g_next_level = -1; }
    return t;
}
/* slot 11 sub_44569A @ 0x44569A "Bruitage" — queue a sound (action 0x0B):
 * name, and the tile x / z it plays at (ambience loops only). */
static void cb_bruitage(const c8 *name, i32 x, i32 z) {
    if (name && name[0] == 'Z') return;            /* sub_44D23D: dead body */
    lieu_action_t *n = lieu_action_push(LA_BRUITAGE);
    if (name) snprintf(n->name, sizeof(n->name), "%s", name);
    n->x = x;                                      /* +0x14, script units */
    n->z = z;                                      /* +0x1C */
}
/* dword_50725C */
static void cb_ajoute_zone(i32 id, i32 cx, i32 cz, i32 sx, i32 sz, i32 flag) {
    if (id < 0 || id >= LIEU_ZONE_COUNT) return;
    g_last_zone = id;
    g_zones[id].center_x = cx * LIEU_ZONE_SCALE;
    g_zones[id].center_z = cz * LIEU_ZONE_SCALE;
    /* zone Y = actor_floor_height(cx*600, cz*600, NULL) — no field in zone_t */
    g_zones[id].size_x   = sx * LIEU_ZONE_SCALE;
    g_zones[id].size_z   = sz * LIEU_ZONE_SCALE;
    g_zones[id].active   = flag;
}
/* slot 13 sub_4458AD "RetireZone"(id) — IMMEDIATE, no range check in dkev. */
static void cb_retire_zone(i32 id) { zone_clear(id); }

/* slot 14 sub_445919 "Combat"(id, mode): QUEUES type 14,
 * +0x0C = id, +0x5C = mode. Effect happens in the dispatcher (combat_set).
 * NOTE the port had the sense inverted: mode -1 = LEAVE combat,
 * anything else = ENTER combat (bit 0 of mode -> entity+0xD14). */
static void cb_combat(i32 id, i32 mode) {
    lieu_action_t *n = lieu_action_push(LA_COMBAT);
    n->actor = id;
    n->num5C = mode;
}
/* Defined further down with the object registry / spawn helpers. */
typedef struct lieu_obj_s lieu_obj_t;
static const lieu_obj_t *obj_lookup(const c8 *name);
static i32 spawn_get_world(i32 id, i32 *wx, i32 *wz);
static i32 obj_world(const lieu_obj_t *o, i32 *x, i32 *z);

/*
 * slot 15 sub_44599B @ 0x44599B "DirectionPerso" — is the player looking at
 * actor/object `a`?
 *   a >= 0  a persona id; its placement is the target.
 *   a <  0  an object: -a > 1000 selects "IM%02d" (-a - 1000), otherwise
 *           "OB%02d" (-a), looked up by name.
 * Within 30000 world units the angle is not consulted at all.
 * Falls back to permissive when the target or the player transform is
 * unknown, so a missing placement cannot deadlock a script.
 */
static i32 cb_direction_perso(i32 a) {
    i32 px, pz;
    if (!player_get_pos(&px, NULL, &pz)) return 1;

    i32 tx = 0, tz = 0;
    if (a >= 0) {
        if (!spawn_get_world(a, &tx, &tz)) return 1;
    } else {
        i32 v = -a;
        c8 name[16];
        if (v > 1000) snprintf(name, sizeof(name), "IM%02d", v - 1000);
        else          snprintf(name, sizeof(name), "OB%02d", v);
        const lieu_obj_t *o = obj_lookup(name);
        if (!o || !obj_world(o, &tx, &tz)) return 1;
    }

    double dx = (double)(tx - px), dz = (double)(tz - pz);
    if (dx * dx + dz * dz < LIEU_FACE_NEAR_SQ) return 1;    /* close enough */
    i32 pa = player_facing_bam(), ta = bam_angle(tx - px, tz - pz);
    i32 ok = facing_ok(pa, ta);
    if (getenv("DKE_FACEDBG"))
        print("face: DirectionPerso(%d) player=(%d,%d)@0x%X target=(%d,%d)@0x%X dist=%.0f -> %d\n",
              a, px, pz, pa, tx, tz, ta, sqrt(dx*dx+dz*dz), ok);
    return ok;
}

/*
 * slot 16 sub_445C82 @ 0x445C82 "DirectionZone" — is the player looking at
 * zone `n`? The engine prefers the entity attached to the zone record
 * (dword_506CA0[n], zone struct +0x20) and falls back to the zone centre
 * (dword_506C80[n] / +8); the port only tracks centres. No distance bypass.
 */
static i32 cb_direction_zone(i32 n) {
    i32 px, pz;
    if (n < 1 || n >= LIEU_ZONE_COUNT) return 1;
    if (g_zones[n].center_x == -1) return 1;
    if (!player_get_pos(&px, NULL, &pz)) return 1;
    i32 pa = player_facing_bam();
    i32 ta = bam_angle(g_zones[n].center_x - px, g_zones[n].center_z - pz);
    i32 ok = facing_ok(pa, ta);
    if (getenv("DKE_FACEDBG"))
        print("face: DirectionZone(%d) player=(%d,%d)@0x%X zone=(%d,%d)@0x%X -> %d\n",
              n, px, pz, pa, g_zones[n].center_x, g_zones[n].center_z, ta, ok);
    return ok;
}

/* slot 17 sub_445E99 "DirectionSpec" — target resolution not yet decoded,
 * so this stays permissive rather than guessing which thing it aims at. */
/* slot 17 sub_445E99 @ 0x445E99 "DirectionSpec" — is the player facing
 * BAM angle `a` (within 0x2400)? The heading includes a clip's root yaw. */
static i32 cb_direction_spec(i32 a) {
    entity_t *p = g_player_entity;
    if (!p || !EROOT(p)) return 0;
    a &= 0xFFFF;
    obc_node_t *g = EROOT(p);
    can_rec_t *r = (can_rec_t *)EPTR(p, 0xD50);
    i32 h;
    if (r && (CAN_FLAGS(r) & 1) && g->child)
        h = (g->rot[2] - (g->child->rot[0] - CAN_ROOT_YAW(r))) & 0xFFFF;
    else
        h = g->rot[2] & 0xFFFF;
    i32 d = 0;
    if (h <= a) { d = a - h; if (d > 0x8000) d -= 0x10000; }
    if (h > a)  { d = a - h; if (d < -0x8000) d += 0x10000; }
    return abs(d) < 0x2400;
}
/* query_990: distance between zone a and b, world units / 600. */
static i32  cb_distance(i32 a, i32 b) {
    if (a < 1 || a >= LIEU_ZONE_COUNT || b < 1 || b >= LIEU_ZONE_COUNT) return 0;
    double dx = (double)(g_zones[a].center_x - g_zones[b].center_x);
    double dz = (double)(g_zones[a].center_z - g_zones[b].center_z);
    return (i32)(sqrt(dx * dx + dz * dz) / LIEU_ZONE_SCALE);
}
/* slot 19 sub_446159 @ 0x446159 "AnimObjet" — queue an object animation.
 * asm: node+0x30 = name, node+0x20 = p, node+0x24 = a1, node+0x28 = a3,
 *      node+0x40 = a2 (hold time in ms). */
static void cb_anim_objet(const c8 *name, i32 p, i32 a1, i32 a2, i32 a3) {
    lieu_action_t *n = lieu_action_push(LA_ANIMOBJET);
    if (name) snprintf(n->name, sizeof(n->name), "%s", name);
    n->p20      = p;
    n->p24      = a1;
    n->p28      = a3;
    n->duration = a2;
}

/* slot 20 sub_446227 "Bloque": dialogue/cutscene group brackets.
 * group(3) = begin group (block player movement + camera).
 * group(0) = end group (close bracket).
 * Negative values (not used in L01) have immediate effects: bit 1=unlock
 * movement, bit 2=unlock camera, bit 4=reset combat state.
 * Since dke has no action queue, we apply the blocking directly. */
static void cb_bloque(i32 a) {                               /* sub_446227 */
    lieu_action_t *n = lieu_action_push(LA_BLOQUE);
    if (a < 0) {
        /* Negative: immediate. 1 = lock movement, 2 = lock camera
         * (dword_4FF0A0), 4 = stop the player's locomotion. */
        a = -a;
        if (a & 1) g_player_blocked = 1;
        if ((a & 4) && g_player_entity) {
            E32(g_player_entity, 0xCEC) = 0; E32(g_player_entity, 0xCF0) = 1;
            E32(g_player_entity, 0xCF4) = 0; E32(g_player_entity, 0xCF8) = 0;
        }
        return;
    }
    n->num5C = a;
    if (a == 0) { n->p60 = 0; n->p64 = 0; }
    if (a & 1) n->p64 = a;
    if (a & 2) n->p60 = a;
}

/* slot 21 sub_44637B "Fouille"(id): search the body of persona id.
 *   QUEUE type 21, +0x0C = id, +0x40 = 1 (state "start").
 *   IMMEDIATELY: every zone whose attached entity (506CA0) is that persona's
 *   entity gets centre = -1 and entity = -1 (so the corpse zone is gone
 *   before the queue runs). (The inner 0x62 loop in the asm is redundant.) */
static void cb_fouille(i32 id) {
    lieu_action_t *n = lieu_action_push(LA_FOUILLE);
    n->actor    = id;
    n->duration = 1;                          /* +0x40 state */
    entity_t *e = actor_find(id);
    for (i32 i = 0; i < LIEU_ZONE_COUNT; i++)
        if (e && g_zones[i].entity == (void *)e) zone_clear(i);
}

/* slot 22 sub_44648B "StopJoindre"(a).
 *   a > 4 and persona table slot dword_4DF5D0[a] empty -> log
 *     "DirectionPerso : Perso %d non charge !!" and return. Only POSITIVE
 *     ids are checked (cmp a,4 / jle is signed), negative ids skip it.
 *   a < 0: IMMEDIATE on e = sub_44411D(-a) (return if NULL):
 *     if e+0xCB4: CB4=0, CEC=0, CF0=1, CF4=0
 *     if e+0xCBC: D14=0, CDC=1, CBC=0, C44=0, CEC=0, CF0=1, CF4=0
 *   a >= 0: QUEUE type 22, +0x0C = a (case 22 does the same clears).
 * The port's immediate path only touched g_ents[], never the entity. */
static void cb_stop_joindre(i32 a) {
    if (a > 4 && !actor_find(a)) {
        print("lieu_cb: DirectionPerso : Perso %d non charge !!\n", a);
        return;
    }
    if (a < 0) {
        entity_t *e = actor_find(-a);
        if (!e) return;
        if (E32(e, 0xCB4)) {
            E32(e, 0xCB4) = 0; E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1; E32(e, 0xCF4) = 0;
        }
        if (E32(e, 0xCBC)) {
            E32(e, 0xD14) = 0; E32(e, 0xCDC) = 1; E32(e, 0xCBC) = 0; E32(e, 0xC44) = 0;
            E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1; E32(e, 0xCF4) = 0;
        }
        if (-a < ENT_MAX) { g_ents[-a].walking = 0; g_ents[-a].joined = 0; }
        return;
    }
    lieu_action_t *n = lieu_action_push(LA_STOPJOINDRE);
    n->actor = a;
}

/* slot 23 sub_44660D @ 0x44660D "VireCol" — queue a script flag write.
 * asm: node+0x68 = ecx (flag id), node+0x4C = edx (value). */
static void cb_vire_col(i32 flag_id, i32 value) {
    lieu_action_t *n = lieu_action_push(LA_VIRECOL);
    n->p68 = flag_id;
    n->p4C = value;
}

/* slot 25: debug/subtitle text — no-op. */
static void cb_print_msg(const c8 *t, i32 edx, i32 len) { (void)t; (void)edx; (void)len; }

/*
 * slot 27 sub_446808 @ 0x446808 "JoindrePerm" — permanent actor join.
 * Like Joindre but the actor keeps following its target. Engine drops the
 * request when the actor already has a permanent join and the id is not
 * negative (asm 0x446871: entity+0xCB4 != 0 && actor >= 0 -> return), and
 * when the entity does not exist at all.
 * asm: node+0x0C = abs(actor), node+0x10 = target.
 */
static void cb_joindre_perm(i32 actor, i32 target) {
    i32 id = actor < 0 ? -actor : actor;
    if (id >= ENT_MAX || !g_ents[id].alive) return;      /* sub_44411D -> NULL */
    if (actor >= 0 && ent_is_joined(id)) return;

    lieu_action_t *n = lieu_action_push(LA_JOINDREPERM);
    n->actor = id;
    n->p10   = target;
}

/* slot 28: cb_46C sets the state slot the handler passed. */
static void cb_modify_var(i32 *slot, i32 edx) {                 /* sub_4468E1 */
    if (!slot) {
        print("lieu_cb: Bad parameter ptr in MODIFYVAR function\n");
        return;
    }
    lieu_action_t *n = lieu_action_push(LA_MODIFYVAR);
    n->slot74 = slot;
    n->num5C  = edx;
}

/* slot 29: query_940 — random integer in [0, n]. */
static i32  cb_random(i32 n) {
    return (i32)(((u32)n * ((u32)rand() & 0x7FFFu)) / 32767u) & 0xFFFF;
}

/*
 * slot 30 sub_446A05 @ 0x446A05 "SelectPile" — pick the action pile.
 *
 * `pile` becomes the allocation target (dword_505C30 @ 0x505C30) and `mode`
 * is written to that pile's enable flag (dword_4FC194 / dword_507274 /
 * dword_507270). mode == 2 means RESET: the engine memsets the pile's 0x2EE0
 * bytes (100 nodes x 0x78) to -1, forces the enable flag back to 1 and zeroes
 * the count. Anything else is stored as-is, so mode 0 parks the pile.
 */
static void cb_select_pile(i32 pile, i32 mode) {
    if (pile < 0 || pile > 2) return;

    lieu_action_select_pile(pile);
    g_layer_mode[pile] = mode;

    if (mode == 2) {
        lieu_action_clear_pile(pile);
        g_layer_mode[pile]    = 1;
        g_layer_counter[pile] = 0;
    } else {
        lieu_action_enable_pile(pile, mode);
    }
}

/* slot 32 sub_446BE9 @ 0x446BE9 "LinkObjet" — attach an object to a holder.
 * asm: name _strupr'd into node+0x30, node+0x0C = holder id,
 *      node+0x54 = a, node+0x5C = b. */
static void cb_link_objet(i32 holder, const c8 *name, i32 a, i32 b) {
    lieu_action_t *n = lieu_action_push(LA_LINKOBJET);
    if (name) {
        i32 k = 0;
        while (name[k] && k < (i32)sizeof(n->name) - 1) {
            c8 ch = name[k];
            if (ch >= 'a' && ch <= 'z') ch -= 32;        /* _strupr */
            n->name[k] = ch;
            k++;
        }
        n->name[k] = '\0';
    }
    n->actor = holder;
    n->p54   = a;
    n->num5C = b;
}

static u32 lo_now(void) { return state ? state->current_game_time : 0; }  /* current_game_time */

/* ls_random(n): 0..n, & 0xFFFF at every call site. Same formula as
 * cb_random (slot 29). */
static i32 lo_random(i32 n) {
    return (i32)(((u32)n * ((u32)rand() & 0x7FFFu)) / 32767u) & 0xFFFF;
}

/* Persona descriptor as sub_446E19 case op 1 picks it (0x446E8E..0x446EBF):
 * a < 4 -> dword_4ECD50 (hero record 0, NOT indexed by a), else
 * dword_4DF5D0 + a*0x220. No upper bound check in the original. */
static const u8 *lo_desc_by_id(i32 a) {
    if (a < 4) return g_hero_desc[0];
    if (a < PERSONA_COUNT) return g_persona_desc[a].raw;
    return NULL;                                  /* original reads out of bounds */
}

/* sub_449E03 @ 0x449E03 — first zone (1..40) containing (x, z), else -1.
 * Same loop as the tail of lieu_detect_zone(); factor it out of there. */
static i32 lo_zone_at(i32 x, i32 z) {
    for (i32 i = 1; i < LIEU_ZONE_COUNT; i++) {
        zone_t *zn = &g_zones[i];
        if (zn->center_x == -1 && !zn->entity) continue;          /* 0x449E2B */
        if (zn->active != 1 && g_force_zones != 1) continue;      /* 0x449E53 */
        i32 cx = zn->center_x, cz = zn->center_z;
        if (zn->entity) {                                         /* 0x449E74 */
            entity_t *e = (entity_t *)zn->entity;
            if (e == g_player_entity) continue;
            cx = EROOT(e)->pos[0];
            cz = EROOT(e)->pos[2];
        }
        if (abs(cx - x) > zn->size_x / 2) continue;
        if (abs(cz - z) > zn->size_z / 2) continue;
        return i;
    }
    return -1;
}


/* word_509B40: 8 i16 per row, row = a (op 26). Row count unknown — sized
 * generously; check the binary's extent before relying on it. */
#define LO_TAB509B40_ROWS 0x80
static i16 g_tab_509B40[LO_TAB509B40_ROWS * 8];

static i32 g_dword_505D70 = 0;   /* op 18: hit-reaction delay by weapon   */
static i32 g_dword_4B2B98 = 0;   /* op 15                                 */
static c8  g_byte_507260[0x10];  /* op 40: "s" + clip name                */

/* ------------------------------------------------------------------------ */

static i32 cb_divers(i32 op, i32 a, i32 b) {
    entity_t *e;

    /* TODO sub_45D560 replay record (dword_4F4CC0 = {0x22, op, a, b}). */

    switch (op) {

    /* op 1 @ 0x446E8E — persona position/heading from the descriptor.
     * b: 0 = desc+0x88 / 600 (X tile), 1 = desc+0x8C / 600 (Z tile),
     *    2 = desc+0x90 & 0xFFFF (heading), 3 = actor root pos[1] (+0x40) / 600. */
    case 1: {
        const u8 *d = lo_desc_by_id(a);
        if (b == 0) return d ? *(const i32 *)(d + 0x88) / 600 : 0;
        if (b == 1) return d ? *(const i32 *)(d + 0x8C) / 600 : 0;
        if (b == 2) return d ? *(const i32 *)(d + 0x90) & 0xFFFF : 0;
        if (b == 3) {
            e = actor_find(a);                                     /* sub_44411D */
            if (!e) {
                print("DIVERS(1,%d,3) with perso %d not loaded\n", a, a);
                return 0;
            }
            return EROOT(e)->pos[1] / 600;                          /* 0x446F62 */
        }
        return 0;
    }

    /* op 2 @ 0x446F7D — can the player's creature do combat move `a`?
     * flags = combat_ability_table[player.desc[0x97] * 0x14] (creature flags).
     * a==7: flags&0x90, a==8: flags&0x0C, a==9: flags&0xF0,
     * a<=6: flags & (1 << (a+1)).
     * If able: b==2 -> 1; else look at the held object (player+0xD64):
     *   none -> log "DIVERS 2 without object", 0;
     *   name[0]=='Q' (obj+0xA8) -> (b==1); otherwise -> (b==0). */
    case 2: {
        e = g_player_entity;
        if (!e) return 0;
        const creature_entry_t *c = game_init_get_creature(EDESC(e)[0x97]);
        u32 fl = c ? c->flags : 0;
        i32 ok = 0;
        if (a == 7 && (fl & 0x90)) ok = 1;
        else if (a == 8 && (fl & 0x0C)) ok = 1;
        else if (a == 9 && (fl & 0xF0)) ok = 1;
        else if (a <= 6 && (fl & (1u << (a + 1)))) ok = 1;
        if (!ok) return 0;
        if (b == 2) return 1;
        const u8 *obj = (const u8 *)EPTR(e, 0xD64);
        if (!obj) {
            print("DIVERS 2 without object\n");
            return 0;
        }
        if (obj[0xA8] == 'Q') return b == 1;
        return b == 0;
    }

    /* op 3 @ 0x4470A1 — non-hero actor +0xCD4 = b (run flag; actor_move.c
     * uses CD4 to pick walk mode 3). */
    case 3:
        if (a >= 4 && (e = actor_find(a)) != NULL)
            E32(e, 0xCD4) = b;
        return 0;

    /* op 4 @ 0x4470D7 — arm/stop an ambient event sound.
     * a = slot<<16 | wav, file "%sDATAS>SOUND>L%02dEV%01d.WAV"(world_state, a&0xFFFF).
     * b = period_ms<<16 | jitter. Only slot (a>>16) in 0..9 is touched:
     *   free (handle==0 && next_time==0): path, param=b,
     *     next_time = now + (b>>16) - random(b&0xFFFF), handle = ls_load_sound(path, 8)
     *     (bracketed by set_display_mode(0xC)/restore_display_mode(1,0xC));
     *   busy && b==0: ls_stop_sound(handle, 0) (slot fields are left as is). */
    case 4:
        sfx_divers_event(a, b);                    /* event slots live in sfx.c */
        return 0;

    /* op 5 @ 0x44729A — item charge (resource) read/write, in percent.
     * a==0: object = get_entity_character(player+0xD2) (held item).
     * a!=0: temp object byte_4DB3C0; name = companion+0xD8 if
     *   sscanf(companion+0xDA,"%02d") == a, else secondary+0xD8 if
     *   **b** == secondary's number (original compares b, not a — bug kept),
     *   else log "DIVERS 5 : no persos have object %d" and name "OB%02d"(a);
     *   then obj+0xC4 &= ~0x20.
     * idx = get_character_action_index(obj) (1..0x23), v = obj[0xAC]-'A'
     *   (instance letter; 0 if the byte is 0).
     * b == -1: return max ? resource_table[idx*10+v*2]*100 / max : 0,
     *   max = word_4DC020[idx*10+v*2];
     * else resource_table[...] = max * b / 100. */
    case 5:
        /* TODO inventory/resource tables (resource_table @ 0x4DBBE0,
         * word_4DC020) are static in game_init.c; needs an accessor and
         * get_entity_character / get_character_action_index. */
        return 0;

    /* op 6 @ 0x44750A — persona HP (desc+0x84). b<0 -> read; else write and
     * refresh the HUD: player -> update_player_interface(game_context, 1),
     * other -> sub_45C700(e) (enemy HP bar). */
    case 6:
        e = actor_find(a);
        if (!e) {
            print("DIVERS 6 : Perso %d not loaded !\n", a);
            return 0;
        }
        if (b < 0) return *(i32 *)(EDESC(e) + 0x84);
        *(i32 *)(EDESC(e) + 0x84) = b;
        /* TODO e == g_player_entity ? update_player_interface(ctx,1)
         *                            : sub_45C700(e) HP bar */
        return 0;

    /* op 7 @ 0x4475A2 — inventory.
     * a==2: sub_4274C2 clear the inventory (drop held weapon via sub_473DBC,
     *   dword_4B3440 = 0, wipe the 0x86 slots byte_4B3448 stride 0x1C);
     * a==0: sub_4276B6 restore/merge the saved copy byte_50A160 (returns 0);
     * else: return sub_427593() — save the inventory to byte_50A160 and
     *   strip items (confiscation). */
    case 7:
        if (a == 2) { inv_clear_all(); return 0; }   /* sub_4274C2 */
        if (a == 0) { inv_restore(); return 0; }     /* sub_4276B6 */
        return inv_confiscate();                     /* sub_427593 */

    /* op 8 @ 0x4475DF — |angle| between actor a's facing and the direction to
     * actor b, BAM 0..0x8000. -1 if either is missing.
     * heading = root->rot[2] (+0x5C); if the current clip (e+0xD50) has
     * CAN_FLAGS & 1: heading -= root->child->rot[0] (+0x7C->+0x54) - CAN_ROOT_YAW.
     * dir = (-(i32)(atan2(dx, dz) * 65536 / 2pi)) & 0xFFFF, dx/dz = b - a. */
    case 8: {
        e = actor_find(a);
        entity_t *t = actor_find(b);
        if (!e || !t) return -1;
        obc_node_t *r = EROOT(e), *rt = EROOT(t);
        f64 dx = (f64)(rt->pos[0] - r->pos[0]);
        f64 dz = (f64)(rt->pos[2] - r->pos[2]);
        can_rec_t *rec = (can_rec_t *)EPTR(e, 0xD50);
        i32 hd;
        if (rec && (CAN_FLAGS(rec) & 1))
            hd = (r->rot[2] - (r->child->rot[0] - CAN_ROOT_YAW(rec))) & 0xFFFF;
        else
            hd = r->rot[2] & 0xFFFF;
        i32 dir = (-(i32)(atan2(dx, dz) * 65536.0 / 6.283185307179586)) & 0xFFFF;
        i32 d = 0;
        if (dir >= hd) { d = dir - hd; if (d > 0x8000) d -= 0x10000; }
        if (dir <  hd) { d = dir - hd; if (d < -0x8000) d = 0x8000 - (-0x8000 - d); }
        return abs(d);
    }

    /* op 9 @ 0x4477B6 — is actor a inside zone b?
     * a not loaded -> 0. b == -1 -> return sub_449E03(player X, player Z)
     * (zone id under the player, or -1). Else the zone centre is the attached
     * entity's root pos (dword_506CA0 != -1; that entity == a -> 0) or the
     * stored x/z; inside = |cx-x| <= size_x/2 && |cz-z| <= size_z/2.
     * No empty/active test here (unlike sub_449E03). */
    case 9: {
        e = actor_find(a);
        if (!e) return 0;
        if (b == -1)
            return lo_zone_at(EROOT(g_player_entity)->pos[0], EROOT(g_player_entity)->pos[2]);
        if (b < 0 || b >= LIEU_ZONE_COUNT) return 0;          /* original: no bound */
        zone_t *zn = &g_zones[b];
        i32 cx, cz;
        if (zn->entity) {                                     /* != -1 in the original */
            if ((entity_t *)zn->entity == e) return 0;
            cx = EROOT((entity_t *)zn->entity)->pos[0];
            cz = EROOT((entity_t *)zn->entity)->pos[2];
        } else {
            cx = zn->center_x;
            cz = zn->center_z;
        }
        if (abs(cx - EROOT(e)->pos[0]) > zn->size_x / 2) return 0;
        if (abs(cz - EROOT(e)->pos[2]) > zn->size_z / 2) return 0;
        return 1;
    }

    /* op 10 @ 0x447923 — screen transition: copy the 640x480 frame into a
     * temp bmap (ls_init_bmap_ex over ui_special_meter_buffer), run the
     * transition effect sub_42E0E0(a ms, plays "trans"), ls_delay(b ms),
     * then put the frame back (16-bit: draw_image; 8-bit:
     * render_loading_transition(bmap, 1000)). Display mode 0xD. */
    case 10: {                                 /* 0x447923: dissolve, hold, restore */
        if (!state || !state->graphic.front_buffer) return 0;
        size_t np = (size_t)state->graphic.width * state->graphic.height;
        u32 *save = (u32 *)malloc(np * sizeof(u32));
        if (save) memcpy(save, state->graphic.front_buffer, np * sizeof(u32));
        screen_dissolve(a);
        if (!getenv("DKE_NOTRANS")) ls_delay((u32)b);
        if (save) {
            if (!g_screen_keep_black) {
                memcpy(state->graphic.front_buffer, save, np * sizeof(u32));
                fx_present();
            }
            free(save);
        }
        return 0;
    }

    case 11:
        e = g_player_entity;
        if (a == 0) {
            if (E32(e, 0xCEC) || E32(e, 0xCF4) || E32(e, 0xD04)) return 0;
            return (i32)lo_now() - (i32)E32(e, 0xC30);
        }
        E32(e, 0xC30) = lo_now();
        return 0;

    /* op 12 @ 0x447A56 — actor a looks at the player: +0xC4C = (b != 0).
     * sub_41CCB7 then calls sub_450DE8(e, player) (head tracking). */
    case 12:
        e = actor_find(a);
        if (!e) return 0;
        E32(e, 0xC4C) = b ? 1 : 0;
        return 0;

    /* op 13 @ 0x447AA3 — sub_418CC2(actor a): preload every clip named in the
     * descriptor (desc+0x11F, 8-byte names, 0x3F entries for heroes / 0x20
     * otherwise, 'x' = none) via get_animation_id. No NULL check before the
     * call; sub_418CC2 returns on NULL. */
    case 13:
        /* TODO preload: for each name call can_get_anim(name) */
        return 0;

    /* op 14 @ 0x447ABE — byte_4B71F0 = 'N'. */
    case 14:
        g_light_char = 'N';
        return 0;

    /* op 15 @ 0x447ACA — a in {0,1}: dword_4B2B98 = a. */
    case 15:
        if (a == 0 || a == 1) g_dword_4B2B98 = a;
        return 0;

    /* op 16 @ 0x447AF4 — screen/HUD control:
     * 0 clear graphics_context; 1 dword_4B69BC=1 + clear;
     * 2 redraw HUD (update_visual_effects, update_game_hud,
     *   update_player_interface(ctx,1), blit game_context->graphics);
     * 3 dword_4B69BC=0 + same as 2; 4 sub_42E0E0(500);
     * 5 dword_4B69BC=1 + sub_42E0E0(500);
     * 6 sub_46F7B5 (full redraw incl. companions) + HUD + blit. */
    case 16:                                   /* 0x447AF4: screen control */
        switch (a) {
        case 0: if (state && state->graphic.front_buffer)
                    memset(state->graphic.front_buffer, 0,
                           (size_t)state->graphic.width * state->graphic.height * 4);
                break;
        case 1: g_screen_keep_black = 1; break;
        case 3: g_screen_keep_black = 0; break;
        case 4: screen_dissolve(500); break;
        case 5: g_screen_keep_black = 1; screen_dissolve(500); break;
        default: break;                        /* 2 / 6: redraws, done every frame */
        }
        return 0;

    /* op 17 @ 0x447CBD — big animation biganims/ANIM0<a>.CJR (sub_454810). */
    case 17:
        cjr_start(state ? state->game_path : NULL, a, (i32)state->current_game_time);
        return 0;

    /* op 18 @ 0x447D09 — was actor a hit (+0xC74, set by
     * calculate_attack_trajectory)?
     * b != 0 (and actor): clear C74, return 0.
     * C74 set: dword_505D70 = atoi(player+0xD4) (player weapon number),
     *   remapped 23->250, 27->800, 32->300, 35->400, 55->250; return 1.
     * else 0. */
    case 18:
        e = actor_find(a);
        if (e && b) { E32(e, 0xC74) = 0; return 0; }
        if (e && E32(e, 0xC74)) {
            i32 w = atoi(ESTR(g_player_entity, 0xD4));
            switch (w) {
            case 23: w = 250; break;
            case 27: w = 800; break;
            case 32: w = 300; break;
            case 35: w = 400; break;
            case 55: w = 250; break;
            default: break;
            }
            g_dword_505D70 = w;
            return 1;
        }
        return 0;

    /* op 19 @ 0x447E4E — return sub_4272B0(): run "hotel.dll" (inn menu),
     * frees sound slots 0/1, returns dword_4B33F4. */
    case 19:
        /* TODO hotel menu */
        return 0;

    /* op 20 @ 0x447E58 — score_counter (HUD bar, 0..0x3FF). a<0 -> read,
     * else write + update_player_interface(ctx, 1). */
    case 20:
        /* TODO score_counter lives in scene.c (g_score_counter, static) —
         * export scene_score_get/set. */
        if (a < 0) return 0 /* scene_score_get() */;
        /* scene_score_set(a); update_player_interface(ctx, 1); */
        return 0;

    /* op 21 @ 0x447E94 — cinematic_mode = a; debug_flags = b. */
    case 21:
        g_cinematic_mode = a;
        g_combat_flags = b;                    /* debug_flags */
        return 0;

    /* op 22 @ 0x447EAC — menu_gametime_5072C8 = now; sub_42E0E0(500);
     * sub_43E038(0,0) (native screen); update_display_state. Mode 0xE. */
    case 22:                                   /* 0x447EAC */
        screen_dissolve(500);
        /* TODO sub_43E038(0, 0): the save / load file screen. */
        return 0;

    case 23:
        e = g_player_entity;
        E32(e, 0xD60) = a;
        LIEU_ST(g_lieu_state, ST_COMBAT_STANCE) = a;   /* dword_505C50 */
        if (E32(e, 0xCEC) != 3) {
            E32(e, 0xCEC) = 0;
            E32(e, 0xCF0) = 1;
            E32(e, 0xCF8) = 0;
            E32(e, 0xCF4) = 0;
        }
        E32(e, 0xD04) = 0;
        E32(e, 0xD00) = 0;
        /* TODO update_player_interface(ctx, 1) */
        return 0;

    /* op 24 @ 0x447F9D — music. !music_enabled -> 0.
     * file "%sDATAS>MOD>L%02dMUS%02d.MOD"(world_state, a); cmd = b & 0xFF:
     * 0 load only (skip if same file as byte_4F9320; else stop/free old,
     *   sub_48D940 load -> dword_4B729C);
     * 1 (if loaded && !sub_48E070 && volume): start sub_48DA10(mod,0x40,0),
     *   set volume dword_4B60B4;
     * 2 free (sub_48DCA0), then falls into 3;
     * 3 volume = ((b>>8) * dword_4B60B4) / 64;
     * 4 fade out sub_48DE30(1000). */
    case 24: {                                 /* cmd 0 loads only */
        static i32 s_mod = 0;
        static c8  s_mod_name[512];            /* byte_4F9320, kept after a free */
        if (!state || !state->music_enabled) return 0;
        c8 path[512];
        snprintf(path, sizeof(path), "%s/datas/mod/L%02dMUS%02d.MOD",
                 state->game_path ? state->game_path : ".", g_lieu_level, a);
        switch (b & 0xFF) {
        case 0:
            if (s_mod && !strcmp(path, s_mod_name)) break;
            if (s_mod) { music_stop(0); music_free(); }
            s_mod = ls_exists(path) && music_load(path);
            if (s_mod) snprintf(s_mod_name, sizeof(s_mod_name), "%s", path);
            print("lieu_cb: music %s %s\n", path, s_mod ? "loaded" : "missing");
            break;
        case 1:
            if (s_mod && !music_is_playing() && g_music_option) {
                music_play(0x40, 0);
                music_volume(g_music_option);
                print("lieu_cb: music play\n");
            }
            break;
        case 2: if (s_mod) { music_free(); s_mod = 0; } break;
        case 3: if (s_mod) music_volume(((b >> 8) * g_music_option) / 64); break;
        case 4: if (s_mod) music_fade(1000); break;
        }
        return 0;
    }

    /* op 25 @ 0x44828D — give/take item: target_entity = byte_4DB3C0, name
     * "OB%02d"(a), +0xC4 &= ~0x20. b<=0 -> sub_426A24(obj) (remove);
     * else sub_427816() (re-equip held) + sub_42639A(obj, b, 1) (add b). */
    case 25:
        /* TODO inventory */
        return 0;

    /* op 26 @ 0x4482F6 — row a of word_509B40 (8 i16).
     * |b| > 99 -> log "DIVERS(26,%d,%d) : multiple is not impl..." (continues).
     * b>0: if row has b or b+100 -> return 0; else store b in first 0 cell.
     * b<=0: clear every cell equal to -b or 100-b. */
    case 26: {
        if (b > 99 || b < -99)
            print("DIVERS(26,%d,%d) : multiple is not implemented\n", a, b);
        if (a < 0 || a >= LO_TAB509B40_ROWS) return 0;
        i16 *row = &g_tab_509B40[a * 8];
        if (b > 0) {
            for (i32 i = 0; i < 8; i++)
                if (row[i] == b || row[i] == b + 100) return 0;
            for (i32 i = 0; i < 8; i++)
                if (row[i] == 0) { row[i] = (i16)b; break; }
        } else {
            for (i32 i = 0; i < 8; i++)
                if (row[i] == -b || row[i] == 100 - b) row[i] = 0;
        }
        return 0;
    }

    /* op 27 @ 0x44844D — clear graphics_context, game_context,
     * render_buffer1; game_exit_code = 3 (end of game). */
    case 27:
        /* TODO game_exit_code = 3 */
        return 0;

    /* op 28 @ 0x44847D — positional sound at bone (b ? b : 0x11) of actor a:
     * play_sound_at_position(node+0x90, +0x94, +0x98 (world pos from
     * actor_node_positions), root->rot[2] + 0x4000). */
    case 28:
        /* TODO positional audio */
        return 0;

    /* op 29 @ 0x448516 — is persona a in one of the 3 actor slots
     * (companion, secondary, player; +0xB0 == a)? */
    case 29:
        if (g_companion_entity && (i32)E32(g_companion_entity, 0xB0) == a) return 1;
        if (g_secondary_companion_entity && (i32)E32(g_secondary_companion_entity, 0xB0) == a) return 1;
        if (g_player_entity && (i32)E32(g_player_entity, 0xB0) == a) return 1;
        return 0;

    /* op 30 @ 0x4485AC — persona 0x19's actor: play sound "A198"
     * (play_entity_sound(0, get_sound_id("A198")), byte_4FB1F0 swapped to
     * byte_4FC010 around it); +0xB4 = now, +0xB8 = a*600, +0xBC = b*600. */
    case 30:
        e = actor_find(0x19);
        if (e) {
            /* TODO sound "A198" */
            E32(e, 0xB4) = lo_now();
            E32(e, 0xB8) = a * 600;
            E32(e, 0xBC) = b * 600;
        }
        return 0;

    /* op 31 @ 0x448656 — tile distance player -> game_structures[a]
     * (0x543CD0, stride 0xF0; +0x3C x, +0x44 z): (i32)sqrt((dx^2+dz^2)/360000).
     * Slot empty (+0 == 0) -> 100000000. */
    case 31:
        {
            i32 sx, sz;
            if (a < 0 || a >= EFFECT_SLOTS || !effect_slot_xz(a, &sx, &sz) ||
                !g_player_entity || !EROOT(g_player_entity))
                return 100000000;
            f64 dx = (f64)(EROOT(g_player_entity)->pos[0] - sx);
            f64 dz = (f64)(EROOT(g_player_entity)->pos[2] - sz);
            return (i32)sqrt((dx * dx + dz * dz) / 360000.0);   /* dbl_4B0688 */
        }

    /* op 32 @ 0x44870C — player+0xCB0 (index of the collision zone under
     * the player, actor_move.c). */
    case 32:
        return (i32)E32(g_player_entity, 0xCB0);

    /* op 33 @ 0x448721 — credits screen "DATAS>MENUS>CREDITS>CRED_%02d.GIF"(a):
     * wait keys released (Esc/Space/Enter), show, wait press, wait release,
     * restore palette/screen. Blocking. */
    case 33:
        /* TODO */
        return 0;

    /* op 34 @ 0x448906 — player looks at persona a: actor a must be loaded
     * (else 0); player+0xC4C = b ? a+1 : 0 (sub_41CCB7 tracks C4C-1). */
    case 34:
        if (!actor_find(a)) return 0;
        E32(g_player_entity, 0xC4C) = b ? a + 1 : 0;
        return 0;

    /* op 35 @ 0x44895B — stop effects: if renderer_instance (sound slot 0):
     * ls_stop_sound, sub_44E46A(0) free it; for player/companion/secondary
     * with +0xD90 != 0: D90 = 0, sub_42D81F(e, 1) (end timed effect).
     * Then if dword_4FF098 != 0: dword_4FF098 = 1. */
    case 35:
        /* TODO the whole case sits under `if (renderer_instance)` — with no
         * audio in dke that is never true, so the D90 resets do not run
         * either. Keep that gate when audio lands. */
        return 0;

    /* op 36 @ 0x448A45 — actor a loaded and not in combat (+0xD60 == 0):
     * b != 0 -> dword_505C54 = 0 (return 0);
     * else if dword_505C54 == 2: return player+0xD04 == 60 &&
     *   sub_4295E0(player) && sub_429695(player, e). Otherwise 0. */
    case 36:
        /* TODO dword_505C54 is g_dword_505C54 (scene.c, static); sub_4295E0 /
         * sub_429695 (combat proximity tests) not ported. */
        return 0;

    /* op 37 @ 0x448AE9 — sub_433610(player, a/100.0): character_scale_factor
     * *= s; for bones 1..0x21 except 0x1F, 0x20: sub_433490(e, i, s, 0). */
    case 37:                                   /* 0x448AE9: sub_433610 */
        actor_scale_body(g_player_entity, (f64)a / 100.0);
        return 0;

    case 38: {                                 /* 0x448B17: sub_433490 */
        i32 ab = abs(b);
        if (ab & 0x10000) actor_scale_bone(g_player_entity, ab & 0xFFFF, (f64)a / 100.0, 2 + (b <= 0));
        else              actor_scale_bone(g_player_entity, ab,          (f64)a / 100.0,     (b <= 0));
        return 0;
    }

    case 39:
        e = actor_find(0x28);
        if (!e) return 0;
        if (a == 0) return (i32)E32(e, 0xD70);
        if (b) { E32(e, 0xD70) = b; E32(e, 0xD74) = lo_now(); }
        else     E32(e, 0xD70) = 0;
        return 0;

    /* op 40 @ 0x448C34 — clip name for state b of actor a: byte_507260 =
     * "s" + prepare_entity_animation(e, b) if non-empty; returns the
     * POINTER byte_507260. Not loaded -> log, 0. */
    case 40: {
        e = actor_find(a);
        if (!e) {
            print("DIVERS(40,%d,%d) with perso %d not loaded\n", a, b, a);
            return 0;
        }
        c8 clip[16] = {0};
        actor_prepare_anim(e, b, clip);
        if (clip[0]) snprintf(g_byte_507260, sizeof g_byte_507260, "s%s", clip);
        /* The original returns the pointer byte_507260; a 64-bit pointer does
         * not fit the i32 result, so return 1 and let the script read the
         * buffer with lieu_divers_text() (lieu_anim_name in the levels). */
        return 1;
    }

    /* op 41 @ 0x448CC4 — update_player_interface(game_context, 1). */
    case 41:
        /* TODO HUD refresh hook */
        return 0;

    /* op 42 @ 0x448CD9 — sub_42BE0E() (one full world tick/render). */
    case 42:
        /* TODO */
        return 0;

    /* op 43 @ 0x448CE3 — special_mode_active ^= 1, dword_505DD8 = it;
     * player: CEC==1 -> CF0=0; CEC==0 -> CF0=1; update_player_interface. */
    case 43:
        g_special_mode_active ^= 1;
        LIEU_ST(g_lieu_state, ST_SPECIAL_MODE) = g_special_mode_active;  /* dword_505DD8 */
        e = g_player_entity;
        if (E32(e, 0xCEC) == 1) E32(e, 0xCF0) = 0;
        if (E32(e, 0xCEC) == 0) E32(e, 0xCF0) = 1;
        return 0;

    /* op 44 @ 0x448D4B — sub_4654CE("OB%02d"(a), 1): player picks up item. */
    case 44: {
        c8 nm[8];
        snprintf(nm, sizeof(nm), "OB%02d", a);
        inv_use_by_name(nm, 1);
        return 0;
    }

    /* op 45 @ 0x448D78 — ls_wait_key() (blocking). */
    case 45:
        return 0;

    /* op 46 @ 0x448D82 — return process_input_command(0x20) (Space edge). */
    case 46:
        return 0; /* TODO input edge; lieu_cb has g_action_key */

    /* op 47 @ 0x448D96 — is tile (a, b) inside a solid floor obstacle?
     * dword_4F915C = 0; for each poly in the NULL-terminated list
     * dword_4F5FB0 with (*poly & 1): sub_4593F7(poly, a*600, b*600) -> 1. */
    case 47:
        /* TODO expose the actor_move.c obstacle list + sub_4593F7 probe */
        return 0;

    /* op 48 @ 0x448E40 — ls_random(a) & 0xFFFF. */
    case 48:
        return lo_random(a);

    default:                                   /* def_448E78: op 0, > 48 */
        return 0;
    }
}

/* slot 36 sub_443273 @ 0x443273 "FreeAnim" — release a 2D anim slot.
 * asm: type 0x27 (FREEANIM), node+0x48 = slot. */
/* slot 7 sub_4452CD "ColObjet": queue type 7; name upper-cased at +0x30,
 * +0x14 = x, +0x1C = z, +0x20 = w, +0x28 = d (all tiles x 600). */
static void cb_col_objet(const c8 *name, i32 x, i32 z, i32 w, i32 d) {
    lieu_action_t *n = lieu_action_push(LA_COLOBJET);
    if (name) {
        snprintf(n->name, sizeof(n->name), "%s", name);
        for (c8 *q = n->name; *q; q++) if (*q >= 'a' && *q <= 'z') *q -= 32;
    }
    n->x   = x * LIEU_ZONE_SCALE;
    n->z   = z * LIEU_ZONE_SCALE;
    n->p20 = w * LIEU_ZONE_SCALE;
    n->p28 = d * LIEU_ZONE_SCALE;
}

/* slot 24 sub_446685 "Simult": queue type 24, +0x6C = n. */
static void cb_simult(i32 v) {
    lieu_action_t *n = lieu_action_push(LA_SIMULT);
    n->p6C = v;
}

/* slot 26 sub_446763 "Joindre obj": queue type 26, name at +0x30,
 * +0x14 = dist x 600. */
static void cb_joindre_obj(const c8 *name, i32 dist) {
    lieu_action_t *n = lieu_action_push(LA_JOINOBJ);
    if (name) snprintf(n->name, sizeof(n->name), "%s", name);
    n->x = dist * LIEU_ZONE_SCALE;
}

/* slot 33 sub_446CBC "PlaceAnimal": queue type 33, +0x0C = id, +0x14 = x,
 * +0x18 = y, +0x1C = z, +0x28 = a, +0x4C = b (unscaled). */
static void cb_place_animal(i32 id, i32 x, i32 y, i32 z, i32 a, i32 b) {
    lieu_action_t *n = lieu_action_push(LA_PLACEANIMAL);
    n->actor = id; n->x = x; n->y = y; n->z = z; n->p28 = a; n->p4C = b;
}

/* slot 35 sub_4430B0 "LoadAnim": queue type 38, name at +0x30, +0x48 = n. */
static void cb_load_anim(const c8 *name, i32 slot) {
    lieu_action_t *n = lieu_action_push(LA_LOADANIM);
    if (name) snprintf(n->name, sizeof(n->name), "%s", name);
    n->id48 = slot;
}

/* slot 38 sub_443643 "PauseAnim": slots >= 20 are rejected; queue type 36,
 * +0x48 = n, +0x6C = frame, +0x70 = mode. */
static void cb_pause_anim(i32 slot, i32 frame, i32 mode) {
    if (slot >= 20) {
        print("lieu_cb: Plantage! animation invalid (slot %d)\n", slot);
        return;
    }
    lieu_action_t *n = lieu_action_push(LA_PAUSEANIM);
    n->id48   = slot;
    n->p6C    = frame;
    n->flag70 = mode;
}

static void cb_free_anim(i32 slot) {
    lieu_action_t *n = lieu_action_push(LA_FREEANIM);
    n->id48 = slot;
}

/* slot 37 sub_44356F @ 0x44356F "PlayAnim" — start a 2D anim slot.
 * asm: type 0x23 (PLAYANIM), node+0x48 = slot, node+0x70 = mode. Slots
 * >= 20 are rejected with "Plantage! animation invalid". */
static void cb_play_anim(i32 slot, i32 mode) {
    if (slot >= 20) {
        print("lieu_cb: Plantage! animation invalid (slot %d)\n", slot);
        return;
    }
    lieu_action_t *n = lieu_action_push(LA_PLAYANIM);
    n->id48   = slot;
    n->flag70 = mode;
}

/* slot 39 sub_446D84 "DiversEmpile"(op, a, arg): QUEUE type 37,
 * +0x14 = op, +0x18 = a, +0x1C = arg. Nothing else. Case 37 completes the
 * node and calls sub_446E19(op, a, arg) = cb_divers, result discarded.
 * So op_9c is "DIVERS, but in pile order" — replace the old immediate
 * switch entirely. */
static void cb_divers_empile(i32 op, i32 a, i32 arg) {
    lieu_action_t *n = lieu_action_push(LA_DIVERSEMPILE);
    n->x = op;
    n->y = a;
    n->z = arg;
}

/* slot 31 sub_446B54 "OnJoindre"(a) — immediate query: -1 when the actor
 * is not loaded, 5 for a permanent join (+0xCB4), else +0xCBC (walking). */
static i32 cb_on_joindre(i32 a) {
    entity_t *e = actor_find(a < 0 ? -a : a);
    if (!e) return -1;
    if (E32(e, 0xCB4)) return 5;
    return E32(e, 0xCBC);
}

/* Concrete vtable. Unlisted (void*) slots stay NULL — the ported handlers
 * never call them. */
static const lieu_cb_t g_lieu_cb_impl = {
    .animation     = cb_animation,
    .dialogue = cb_dialogue,
    .evenement   = cb_evenement,
    .cinematique      = cb_cinematique,
    .pos_perso        = cb_pos_perso,
    .etat_objet     = cb_etat_objet,
    .joindre        = cb_joindre,
    .change_camera           = cb_change_camera,
    .change_piece           = cb_change_piece,
    .bruitage        = cb_bruitage,
    .ajoute_zone        = cb_ajoute_zone,
    .retire_zone           = cb_retire_zone,
    .combat           = cb_combat,
    .direction_perso        = cb_direction_perso,
    .direction_zone        = cb_direction_zone,
    .direction_spec        = cb_direction_spec,
    .distance        = cb_distance,
    .anim_objet           = cb_anim_objet,
    .on_joindre           = cb_on_joindre,
    .bloque            = cb_bloque,
    .fouille           = cb_fouille,
    .stop_joindre       = cb_stop_joindre,
    .vire_col           = cb_vire_col,
    .print_msg       = cb_print_msg,
    .joindre_perm        = cb_joindre_perm,
    .modify_var           = cb_modify_var,
    .random        = cb_random,
    .select_pile     = cb_select_pile,
    .link_objet       = cb_link_objet,
    .divers         = cb_divers,
    .free_anim           = cb_free_anim,
    .col_objet           = cb_col_objet,
    .simult              = cb_simult,
    .joindre_obj         = cb_joindre_obj,
    .place_animal        = cb_place_animal,
    .load_anim           = cb_load_anim,
    .pause_anim          = cb_pause_anim,
    .play_anim           = cb_play_anim,
    .divers_empile            = cb_divers_empile,
};

/* -------------------------------------------------------------------------
 * Script-visible object state.
 *
 * ETATOBJET / LINKOBJET / VIRECOL / the 2D-anim opcodes all mutate engine
 * state dke has not ported (object meshes at dword_4DF5D0, the anim2d slot
 * table at anim_state_531BE0, the collision flag table). We keep the values
 * in these registries so the state is real and queryable rather than lost,
 * and so a later render/audio pass can consume it without re-reversing the
 * scripts.
 * ---------------------------------------------------------------------- */
#define LIEU_OBJ_MAX 64
struct lieu_obj_s {
    c8  name[32];
    i32 x, y, z;      /* world units (tile x600) */
    i32 p20, p24, p28;
    i32 owner;        /* entity the object belongs to, -1 = none */
    i32 linked_to;    /* LINKOBJET holder id, -1 = unlinked      */
    i32 used;
};
static lieu_obj_t g_objs[LIEU_OBJ_MAX];
static i32        g_obj_count = 0;

static const lieu_obj_t *obj_lookup(const c8 *name) {
    if (!name || !name[0]) return NULL;
    for (i32 i = 0; i < g_obj_count; i++)
        if (strcmp(g_objs[i].name, name) == 0) return &g_objs[i];
    return NULL;
}

/* World position recorded for an object by ETATOBJET. */
static i32 obj_world(const lieu_obj_t *o, i32 *x, i32 *z) {
    if (!o) return 0;
    if (x) *x = o->x;
    if (z) *z = o->z;
    return 1;
}

static lieu_obj_t *obj_find(const c8 *name, i32 create) {
    if (!name || !name[0]) return NULL;
    for (i32 i = 0; i < g_obj_count; i++)
        if (strcmp(g_objs[i].name, name) == 0) return &g_objs[i];
    if (!create || g_obj_count >= LIEU_OBJ_MAX) return NULL;
    lieu_obj_t *o = &g_objs[g_obj_count++];
    memset(o, 0, sizeof(*o));
    snprintf(o->name, sizeof(o->name), "%s", name);
    o->owner = -1;
    o->linked_to = -1;
    o->used = 1;
    return o;
}

/* VIRECOL flag table (engine: per-level collision/state flags). */
#define LIEU_FLAG_MAX 128
static i32 g_flags[LIEU_FLAG_MAX];

/* 2D anim slots driven by PLAYANIM / PAUSEANIM / LOADANIM / FREEANIM.
 * 0 = free, 1 = loaded, 2 = playing, 3 = paused. */
#define LIEU_ANIM_SLOTS 20
static i32 g_anim_slot[LIEU_ANIM_SLOTS];

static void objs_clear(void) {
    memset(g_objs, 0, sizeof(g_objs));
    g_obj_count = 0;
    memset(g_flags, 0, sizeof(g_flags));
    memset(g_anim_slot, 0, sizeof(g_anim_slot));
}

/* Live world X/Z of persona `id` (sub_44411D -> entity +0x50 node). */
static i32 spawn_get_world(i32 id, i32 *wx, i32 *wz) {
    entity_t *e = actor_find(id);
    if (!e || !EROOT(e)) return 0;
    if (wx) *wx = EROOT(e)->pos[0];
    if (wz) *wz = EROOT(e)->pos[2];
    return 1;
}

/* -------------------------------------------------------------------------
 * Requested animation clips.
 *
 * An ANIMATION action's name is the CAN clip id and carries its own persona:
 * "p12arr03" is Kaly's arrival-03 clip, "p00mar25" is Arkhan walking. The
 * engine resolves the entity and calls play_entity_animation; the port has no
 * entity animation player, so the clip is published here and the scene
 * renderer swaps the persona's CAN over to it.
 * ---------------------------------------------------------------------- */

/*
 * lieu_action_dispatch — port of the 40-case switch in sub_44A58B @ 0x44A58B.
 *
 * Returns 1 when the action is complete (the driver then calls
 * lieu_action_done -> sub_4442EE @ 0x4442EE), 0 to hold it at the head of the
 * pile for another frame. The original expresses "hold" through the entity
 * struct (entity+0xC28 start time vs node+0x40) and the per-pile busy flag
 * dword_4B5418[]; we hold on node->start_ms instead.
 */
/* ==========================================================================
 * sub_46FF9A @ 0x46FF9A — COMBAT (case 14)
 * ========================================================================== */
static void combat_set(i32 id, i32 mode) {
    /* world_state == 0x3D && id == 0x1B -> id = dword_5512CC (not L01). */
    if (id < 4) {                                  /* the player */
        entity_t *p = g_player_entity;
        if (!p) return;
        E32(p, 0xD60) = 1;                         /* dword_505C50 = 1 */
        E32(p, 0xCEC) = 0; E32(p, 0xCF0) = 1; E32(p, 0xCF8) = 0; E32(p, 0xCF4) = 0;
        E32(p, 0xD04) = 0; E32(p, 0xD00) = 0;
        g_hero_autofight = 1;                      /* dword_4B219C */
        return;
    }
    entity_t *e = actor_find(id);                  /* dword_4DF5D0[id*0x220] */
    if (!e) { print("COMBAT : perso %d not loaded\n", id); return; }
    E32(e, 0xD30) = 0;
    if (mode == -1) {                              /* leave combat */
        E32(e, 0xD60) = 0; E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1; E32(e, 0xD14) = 0;
        if (id < ENT_MAX) g_ents[id].combat_ready = 0;
        return;
    }
    g_combat_flags = mode;                         /* debug_flags */
    if (E32(e, 0xD60)) return;                     /* already fighting */
    E32(e, 0xD14) = 0; E32(e, 0xCBC) = 0; E32(e, 0xCB4) = 0;   /* drop walk / join */
    E32(e, 0xD60) = 1;
    if (E32(e, 0xB0) != 0x5C) {
        E32(e, 0xD08) = 0; E32(e, 0xD04) = 0; E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1;
    }
    g_enemy_hits = 0;                              /* dword_506520 */
    E32(e, 0xC64) = 0x10000;
    E32(e, 0xC68) = 0x4000;
    E32(e, 0xD14) = (mode & 1) ? 1 : 0;
    if (id < ENT_MAX) g_ents[id].combat_ready = 1;
    /* TODO combat AI: whatever reads entity+0xD60 / +0xD14 (sub_41AED1 ...). */
}

/* ==========================================================================
 * sub_444BF7 @ 0x444BF7 — CINEMA (case 3)
 * ========================================================================== */
static void cinema_play(const c8 *arg) {
    c8 base[32];
    i32 i = 0, start = 0;
    while (arg[i] && arg[i] != '_' && i < 30) { base[i] = arg[i]; i++; }
    base[i] = '\0';
    if (arg[i] == '_') { start = atoi(arg + i + 1); if (start < 0) start = 0; }
    /* music loaded (dword_4B729C) -> sub_48DE30(0) stop. TODO audio. */
    i32 num = atoi(base + 2);                      /* "SC21" -> 21 */
    if (!(num == 2 && g_lieu_level == 7))          /* world_state == 7 */
        g_movie_seen[(num >> 3) & 7] |= (u8)(1u << (num & 7));
    /* subtitles "DATAS>SBT>SC%02d_%s.TXT"(num, language) ; file "SC>%s.CRH".
     * `start` (the _500 suffix) goes to ls_play_movie as its 3rd argument. */
    snprintf(g_lieu_movie, sizeof(g_lieu_movie), "%s.CRH", base);
    g_lieu_movie_pending = 1;
    /* Movies 16, 21, 22, 23 are endings: played with the skip callback
     * sub_444B10, then the three buffers are cleared and game_exit_code = 3. */
    if (num == 16 || num == 21 || num == 22 || num == 23) g_game_exit_code = 3;
    (void)start;
    print("lieu_act: CINEMA %s (start %d)\n", g_lieu_movie, start);
    /* After the movie: ls_flush_keyboard, clear screen, default palette and
     * spin until ESC is released. */
}

i32 lieu_action_dispatch(lieu_action_t *n, u32 now_ms) {
    if (!n) return 1;

    switch (n->type) {

    case LA_ANIMATION: {                       /* case 0 @ 0x44A8E3 */
        static i32 busy[3];                    /* dword_4B5418[pile] */
        if (g_skip_next_animation) { g_skip_next_animation = 0; return 1; }
        i32 pile = lieu_action_run_pile();
        if (pile < 0 || pile > 2) pile = 0;
        entity_t *e = actor_find(n->actor);
        if (n->actor != -2 && !e) {
            print("lieu_act: Animation %s with perso %d not loaded\n", n->name, n->actor);
            return 1;
        }
        if (n->actor != -2 && EDESC(e) && EDESC(e)[0x96]) {
            print("lieu_act: Animation %s with cadavre %d\n", n->name, n->actor);
            return 1;
        }
        /* 0x44A9C4: an animation queued on the pile that is talking waits for
         * the line; a hold-at-end clip that has finished releases the pile
         * early (dword_4FF09C = -(pile + 1)). */
        if (dlg_is_active() && g_dlg_pile == lieu_selected_pile()) {
            if (e && EPTR(e, 0xD50) && (CAN_FLAGS((can_rec_t *)EPTR(e, 0xD50)) & 0x200) &&
                actor_anim_done(e))
                g_dlg_pile = -(g_dlg_pile + 1);
            return 0;
        }
        if (g_dlg_pile < 0) g_dlg_pile = -g_dlg_pile - 1;
        if (n->num5C == 0xFFFF)                /* wait for the current clip */
            return actor_anim_done(e);
        if (n->actor == -2) return 1;          /* 2D anim player not ported */
        g_pile_actor[pile] = n->actor;         /* dword_4B53F0[pile] */
        e = actor_find(n->actor);
        if (!e) return 0;
        if (E32(e, 0xCBC)) return 0;           /* still walking */

        i32 start = 0;
        if (n->flag70) start = 2;
        else if ((actor_anim_done(e) || n->prev_type == LA_JOINDRE) && !busy[pile]) start = 1;
        if (start) {
            E32(e, 0xD58) = actor_anim_state(e, n->name);
            actor_bake_root_yaw(e);
            actor_play_anim(e, can_get_anim(n->name), 0);
            E32(e, 0xC48) = n->num5C;
            E32(e, 0xCF8) = E32(e, 0xCF4) = E32(e, 0xCF0) = E32(e, 0xCEC) = 0;
            print("lieu_act: ANIMATION '%s' actor=%d hold=%d\n", n->name, n->actor, n->duration);
            if (start == 2 || n->duration == -1) return 1;
            busy[pile] = 1;
            return 0;
        }
        if (n->duration != -1 && busy[pile] == 1 &&
            (i32)now_ms - E32(e, 0xC28) > n->duration) {
            busy[pile] = 0;
            return 1;
        }
        return 0;
    }

    case LA_CHANGEPIECE:                       /* case 10 @ 0x44BA2C */
        if (dlg_is_active()) return 0;
        g_next_entry = n->num5C;
        if (n->p58 == g_lieu_level) return 1;
        /* need_change_level_id; the next room starts on camera 0 in auto
         * mode (color_mode = dword_506BD0 = 0, dword_4B7284 = -2). */
        print("lieu_act: CHANGEPIECE target=%d entry=%d (cur=%d)\n", n->p58, n->num5C, g_lieu_level);
        g_next_level = n->p58;
        LIEU_ST(g_lieu_state, 0xF90) = 0;
        g_next_cam_mode = 1;
        g_next_cam = -2;
        return 1;

    case LA_CHANGECAMERA:                      /* case 9 @ 0x44B949 */
        if (dlg_is_active()) return 0;
        /* 0x44B96D: the locked camera (dword_506BD0 = script ST(0xF90))
         * only changes in mode 2; mode 1 hands the camera to sub_45A67C. */
        g_next_cam_mode = n->num5C;
        if (n->num5C == 2) {
            g_next_cam = n->p58;
            LIEU_ST(g_lieu_state, 0xF90) = n->p58;
        } else {
            g_next_cam = -2;
        }
        return 1;

    case LA_DIALOGUE:                          /* case 1 -> sub_46BC74 */
        if (dlg_is_active()) return 0;         /* dword_4FF098: slot busy */
        menu_dial_log(n->name, n->actor);      /* sub_46A358: the menu's dialogue log */
        {
            entity_t *e = actor_find(n->actor);
            if (e && EDESC(e) && EDESC(e)[0x96]) {
                print("lieu_act: Le cadavre %d a dit %s !\n", n->actor, n->name);
                return 1;
            }
        }
        /* sub_46A358 text setup + sub_44E57B voice; dword_4B2AB4 speaker
         * portrait (hero / companion / other) is not drawn. */
        dlg_start(n->name, n->actor, now_ms);
        g_dlg_pile = lieu_selected_pile();      /* dword_4FF09C = dword_505C30 */
        return 1;

    case LA_POSPERSO:                          /* case 5 @ 0x44AF10 */
        actor_pos_perso(n->actor, n->x, n->z, n->p20, n->p4C, n->name, n->p50, n->p2C);
        return 1;

    case LA_EVENEMENT:                         /* case 2 @ 0x44AD41 */
        /* duration -1 cancels the pending timer, otherwise arm one that
         * fires opcode 4 with node+0x48 (asm 0x44AE21..0x44AEF0). */
        if (n->duration == -1) {
            lieu_timer_remove(n->id48);
            print("lieu_act: EVENEMENT cancel id=%d\n", n->id48);
        } else {
            lieu_timer_add(n->id48, n->duration, now_ms);
            print("lieu_act: EVENEMENT arm id=%d in %d ms\n", n->id48, n->duration);
        }
        return 1;

    case LA_CINEMA:                            /* case 3 @ 0x44CAF9 */
        /* sub_444BF7 plays synchronously in dkev, so the pile is stalled for
         * the whole movie. The port plays from scene_render: hold the node
         * until the movie has been taken and finished if exact pile timing
         * matters (TODO: lieu_movie_done()); completing at once is the
         * current behaviour. */
        cinema_play(n->name);
        return 1;

    case LA_KILLPERSO:                         /* case 4: never queued */
    case LA_AJOUTEZONE:                        /* case 12: never queued */
    case LA_RETIREZONE:                        /* case 13: never queued */
        return 1;

    case LA_COLOBJET:                          /* case 7 @ 0x44B432 */
        /* obj = get_entity_character(name); if obj && obj+0xB0 (polygon):
         *   all of x,z,w/2,d/2 zero -> memset(poly, 0, 0x208)
         *   else +0xF8 = x-w/2, +0xFC = z-d/2, +0x100 = x+w/2, +0x104 = z+d/2,
         *        +0x108 = world_controller+0x1E0, +0xE8 = 0,
         *        sub_47701F(obj, minx, minz, maxx, maxz, +0x108). */
        object_colobjet(n->name, n->x, n->z, n->p20, n->p28);
        return 1;

    case LA_COMBAT:                            /* case 14 @ 0x44BB4C */
        combat_set(n->actor, n->num5C);
        /* update_display_state(graphics_context) */
        return 1;

    case LA_FOUILLE: {                         /* case 21 @ 0x44BDAD */
        entity_t *p = g_player_entity;
        entity_t *corpse = actor_find(n->actor);           /* var_8 */
        i16 *inv = (n->actor >= 0 && n->actor < LO_TAB509B40_ROWS) ? &g_tab_509B40[n->actor * 8] : NULL;
        i32 hero = actor_hero_id();                        /* game_state */
        if (!p) return 1;

        if (n->duration == 1) {                /* start: crouch animation */
            i32 anim = 9;                      /* 9 = nothing to take */
            for (i32 i = 0; inv && i < 8; i++) if (inv[i]) anim = 8;
            E32(p, 0xCF0) = 0; E32(p, 0xCEC) = 0; E32(p, 0xCF8) = 0; E32(p, 0xCF4) = 0;
            c8 clip[16];
            if ((n->actor == 0x42 && g_lieu_level == 0x23) || n->actor == 0x5F)
                snprintf(clip, sizeof(clip), "g%02dpre05", hero);
            else
                snprintf(clip, sizeof(clip), "g%02dpre%02d", hero, anim);
            actor_bake_root_yaw(p);            /* process_entity_state_change */
            actor_play_anim(p, can_get_anim(clip), 0);
            E32(p, 0xD58) = actor_anim_state(p, clip);     /* sub_41918C */
            g_cam_lock = 1;                    /* dword_4FF0A0 */
            g_player_blocked = 1;              /* player_movement_enabled */
            /* if (!g_lieu_loading) update_player_interface */
            n->duration = anim;
            return 0;
        }
        /* every later state first waits for the player's clip (0x44BF5B) */
        if (!actor_anim_done(p)) return 0;                 /* is_entity_in_animation */
        if (n->duration == 8 || n->duration == 9) {
            if (n->duration == 9) {            /* "nothing here" line */
                static const c8 *a[4] = { NULL, "LCOD75", "LCOD76", "LCOD77" };
                i32 id = n->actor;
                i32 cls_a = (id == 10 || (id >= 12 && id <= 16) || id == 26 || id == 65);
                const c8 *msg = NULL;
                if (cls_a)              msg = (hero >= 1 && hero <= 3) ? a[hero] : NULL;
                else if (hero == 1)     msg = "LCOD81";
                else if (hero == 2)     msg = g_special_mode_active ? "LCOD82" : "LCOD83";
                else if (hero == 3)     msg = g_special_mode_active ? "LCOD84" : "LCOD85";
                if (msg) {                                 /* sub_46BC74(msg, game_state) */
                    menu_dial_log(msg, hero);
                    dlg_start(msg, hero, now_ms);
                }
                n->duration = 10;
                /* falls through to the "state != 8" wait below */
            } else {                           /* 8: take the items */
                for (i32 i = 0; inv && i < 8; i++) {
                    i32 v = inv[i];
                    if (!v) continue;
                    if (v == 0x3E || v == 0xA2) v = 0x25;  /* item remap */
                    c8 obj[8];
                    if (v / 100) snprintf(obj, sizeof(obj), "OB%02d%c", v % 100, '@' + v / 100);
                    else         snprintf(obj, sizeof(obj), "OB%02d", v % 100);
                    inv_add(obj, 1, 1);                /* sub_42639A(target, 1, 1) */
                    inv[i] = 0;
                }
                if (corpse) {                  /* strip the corpse's weapon */
                    /* TODO objects: sub_474462(corpse, corpse+0xD68, 1). */
                    game_object_t *w = (game_object_t *)EPTR(corpse, 0xD64);
                    weapon_detach(corpse, 0);  /* sub_4735A2: the weapon's nodes leave the hand */
                    if (w) object_world_remove(w->name);   /* sub_472B47 */
                    EPTR(corpse, 0xD64) = NULL;
                    E32(corpse, 0x178) = 0;
                    if (EDESC(corpse)) EDESC(corpse)[0x97] = 0;
                }
                g_ui_state = 2;                /* dword_4DD9B4: redraw without the corpse's items */
                n->duration = 2;
                return 0;
            }
        }
        /* state 10 (after the line) or 2 (items taken): wait until the game
         * loop has settled the redraw (0x44C336), then state 2 shows the new
         * items once: sub_42701B opens the screen in pickup mode. */
        if (g_ui_state != 1) return 0;
        if (n->duration == 2) inventory_screen_open();
        {
            c8 clip[16] = { 0 };
            actor_bake_root_yaw(p);
            actor_prepare_anim(p, g_special_mode_active ? -5 : 0x3B, clip);
            E32(p, 0xD58) = actor_anim_state(p, clip);
            actor_play_anim(p, can_get_anim(clip), 0);
        }
        g_cam_lock = 0;
        g_player_blocked = 0;
        /* if (!g_lieu_loading) update_player_interface */
        return 1;
    }

    case LA_SIMULT:                            /* case 24 @ 0x44C5D9 */
        g_skip_next_animation = n->p6C;        /* dword_506BE0 */
        return 1;

    case LA_JOINOBJ: {                         /* case 26 @ 0x44C747 */
        /* Walk the PLAYER to the point `dist` (+0x14) from object `name`, on
         * the object->player line, facing the object.
         *   obj = get_entity_character(name); o = (obj+0xD8, obj+0xDC)
         *   d = player - o; f = dist / |d|; t = o + d*f
         *   sub_4558A0(&player.pos, &t) == 0 -> CC0/CC4 = t, else player pos
         *   CBC = 1; CC8 = sub_4440A6(&o, &t) (heading); C44 = 1. done.
         * TODO: the object struct at +0xD8/+0xDC is the dkev object slot
         * (game_object_t has no such fields yet); sub_4558A0 is the path /
         * collision check; heading assumed = bam_angle(o - t). */
        entity_t *p = g_player_entity;
        i32 ox = 0, oz = 0;
        if (!p || !EROOT(p) || !object_pos_xz(n->name, &ox, &oz)) return 1;  /* TODO object_pos_xz */
        double dx = EROOT(p)->pos[0] - ox, dz = EROOT(p)->pos[2] - oz;
        double len = sqrt(dx * dx + dz * dz);
        double f = len > 0 ? n->x / len : 0;
        i32 tx = ox + (i32)(dx * f), tz = oz + (i32)(dz * f);
        E32(p, 0xCC0) = tx;
        E32(p, 0xCC4) = tz;
        E32(p, 0xCBC) = 1;
        E32(p, 0xCC8) = bam_angle(ox - tx, oz - tz);
        E32(p, 0xC44) = 1;
        return 1;
    }

    case LA_PLACEANIMAL:                       /* case 33 @ 0x44CA4B */
        /* count used game_structures (16 x 0xF0); > 12 -> log and skip;
         * else process_creature_effects(id, x, y, z, p28, p4C). */
        if (effect_used_count() > 12) {
            print("Ivan faut pas me prendre pour un con, ah ah ah !!!\n");   /* log_message */
            return 1;
        }
        effect_place_creature(n->actor, n->x, n->y, n->z, n->p28, n->p4C);
        return 1;

    case LA_DIVERSEMPILE:                      /* case 37 @ 0x44CC77 */
        /* dkev completes the node BEFORE the call; nothing in sub_446E19
         * queues, so calling first and returning 1 is equivalent. */
        cb_divers(n->x, n->y, n->z);
        return 1;

    /* 15-18, 25, 29-31, 34 (DIRECPERSO..DISTANCE, PRINT, RANDOM, SELECTPILE,
     * ONJOINDRE, DIVERS): the default case, done at once. Already covered by
     * `default:` (drop its "not wired" log for these). */

    case LA_ETATOBJET: {                       /* case 6 @ 0x44B00F */
        lieu_obj_t *o = obj_find(n->name, 1);
        if (o) {
            /* asm writes entity+0xCC/0xD0/0xD4 = node+0x14/0x18/0x1C and
             * entity+0xE0/0xE4/0xE8/0xEC = node+0x20/0x24/0x28/0x54. */
            o->x = n->x; o->y = n->y; o->z = n->z;
            o->p20 = n->p20; o->p24 = n->p24; o->p28 = n->p28;
            o->owner = n->actor;
        }
        {
            /* Item inventory states (a6 <= -2, and phase 3 of a pickup)
             * run in inventory.c and may keep the action waiting. */
            i32 s_pre = n->name[0] == 'S';
            const c8 *nm = s_pre ? n->name + 1 : n->name;
            i32 item = (nm[0] == 'O' || nm[0] == 'Q') && nm[1] == 'B';
            if (item && (n->actor <= -2 || n->actor == 3)) {
                etat_item_t it;
                snprintf(it.name, sizeof(it.name), "%s", nm);
                it.a6 = n->actor;
                it.a7 = n->p54;
                i32 done = etat_objet_item(&it, s_pre);
                n->actor = it.a6;                      /* -3 -> 3 in place */
                return done;
            }
        }
        object_etat(n->name, n->x, n->y, n->z, n->p20, n->p24, n->p28, n->actor, n->p54);
        return 1;
    }

    case LA_JOINDRE: {                         /* case 8 @ 0x44B5E3 */
        /* Arms the actor's walk (entity +0xCC0/+0xCC4/+0xCC8, state +0xCBC = 1)
         * and completes; sub_469222 walks it on later frames. A JOINDRE that
         * repeats the running walk waits for it to end (heroes only). */
        entity_t *e = actor_find(n->actor);
        entity_t *t = actor_find(n->p10);
        if (!e || !EROOT(e)) return 1;
        obc_node_t *eg = EROOT(e);
        E32(e, 0xCB4) = 0;
        if (t && EROOT(t)) {
            obc_node_t *tg = EROOT(t);
            if (n->x == 0 && n->z == 0 && n->p24 == 0) { n->x = eg->pos[0]; n->z = eg->pos[2]; }
            else                                      { n->x = tg->pos[0]; n->z = tg->pos[2]; }
            n->p24 = bam_angle(tg->pos[0] - eg->pos[0], tg->pos[2] - eg->pos[2]);
        } else if (n->x == 0 && n->z == 0) {
            if (n->p10 < 0 && n->p24 == 0) {
                i32 zi = -n->p10 - 1;
                if (zi >= 0 && zi < LIEU_ZONE_COUNT)
                    n->p24 = bam_angle(g_zones[zi].center_x - eg->pos[0],
                                       g_zones[zi].center_z - eg->pos[2]);
            }
            n->x = eg->pos[0];
            n->z = eg->pos[2];
        }
        if ((E32(e, 0xCBC) != 0 || E32(e, 0xC44) != 0) &&
            (n->x == E32(e, 0xCC0) || E32(e, 0xCC4) == n->z || n->p24 == E32(e, 0xCC8))) {
            if (n->actor > 3) { E32(e, 0xC44) = 0; return 1; }
            if (E32(e, 0xCBC) == 0 && E32(e, 0xC44) == 1) { E32(e, 0xC44) = 0; return 1; }
            return 0;
        }
        if (E32(e, 0xD60) != 0 && e != g_player_entity) {
            print("lieu_act: JOINDRE executed during COMBAT by perso %d\n", E32(e, 0xB0));
            return 1;
        }
        E32(e, 0xCC0) = n->x;
        E32(e, 0xCC4) = n->z;
        E32(e, 0xCC8) = n->p24;
        E32(e, 0xCBC) = 1;
        E32(e, 0xC44) = 1;
        print("lieu_act: JOINDRE actor=%d (%d,%d) -> (%d,%d) facing=0x%X\n",
              n->actor, eg->pos[0], eg->pos[2], n->x, n->z, n->p24 & 0xFFFF);
        return 1;
    }

    case LA_JOINDREPERM: {                     /* case 0x1B @ 0x44C695 */
        entity_t *e = actor_find(n->actor);
        entity_t *t = actor_find(n->p10);
        if (e && t) {
            if (E32(e, 0xD60) != 0 && e != g_player_entity) {
                print("lieu_act: JOINDREPERM executed during COMBAT by perso %d\n", E32(e, 0xB0));
                return 1;
            }
            E32(e, 0xCB4) = 1;
            E32(e, 0xCB8) = n->p10;
        }
        return 1;
    }

    case LA_BRUITAGE:                          /* case 0x0B -> sub_44E57B */
        sfx_bruitage(n->name, n->x, n->z);
        return 1;

    case LA_ANIMOBJET: {                       /* case 0x13 */
        if (n->aux_state == 0) {
            n->aux_state = 1;
            lieu_obj_t *o = obj_find(n->name, 1);
            if (o) { o->p20 = n->p20; o->p24 = n->p24; o->p28 = n->p28; }
        }
        /* sub_412B94 runs it; the action completes once it has finished. */
        return object_anim(n->name, n->p20, n->p24, n->p28, n->duration, now_ms);
    }

    case LA_STOPJOINDRE: {                     /* case 0x16 @ 0x44C435 */
        entity_t *e = actor_find(n->actor);
        if (e) {
            if (E32(e, 0xCB4)) {
                E32(e, 0xCB4) = 0; E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1; E32(e, 0xCF4) = 0;
            }
            if (E32(e, 0xCBC)) {
                E32(e, 0xCDC) = 1; E32(e, 0xD14) = 0; E32(e, 0xCBC) = 0; E32(e, 0xC44) = 0;
                E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1; E32(e, 0xCF4) = 0;
            }
        }
        return 1;
    }

    case LA_VIRECOL: {                         /* case 0x17 @ 0x44C505 */
        /* Collision polygon id (dword_4F5FB0[id]) or, negative, stair
         * floor -id (dword_4F915C[-id] = the sol%02d list): bit 0 makes it
         * solid. Then the navigation grid is rebuilt (sub_45619D). */
        sol_limit_t *z = NULL;
        i32 id = n->p68;
        if (id >= 0 && id < g_world.num_collisions) z = g_world.collision_zones[id];
        if (id < 0 && -id - 1 < g_world.num_stairs) z = g_world.stair_zones[-id - 1];
        if (z) { if (n->p4C) z->flags |= 1; else z->flags &= ~1u; }
        if (id >= 0 && id < LIEU_FLAG_MAX) g_flags[id] = n->p4C;
        scene_rebuild_nav();
        print("lieu_act: VIRECOL %d = %d%s\n", id, n->p4C, z ? "" : " (no polygon)");
        return 1;
    }

    case LA_LINKOBJET: {                       /* case 0x20 */
        lieu_obj_t *o = obj_find(n->name, 1);
        if (o) o->linked_to = n->actor;
        print("lieu_act: LINKOBJET '%s' -> holder %d (%d,%d)\n",
              n->name, n->actor, n->p54, n->num5C);
        return 1;
    }

    case LA_PLAYANIM:                          /* case 0x23 */
        if (n->id48 >= 0 && n->id48 < LIEU_ANIM_SLOTS) g_anim_slot[n->id48] = 2;
        anim2d_start(n->id48 + 10, n->flag70);           /* frame 0, loop = +0x70 */
        return 1;

    case LA_PAUSEANIM:                         /* case 0x24 */
        if (n->id48 >= 0 && n->id48 < LIEU_ANIM_SLOTS) g_anim_slot[n->id48] = 3;
        anim2d_set_frame(n->id48 + 10, n->p6C, n->flag70);   /* 0 hold, -1 off */
        return 1;

    case LA_LOADANIM:                          /* case 0x26 -> sub_44313F */
        if (n->id48 >= 0 && n->id48 < LIEU_ANIM_SLOTS) g_anim_slot[n->id48] = 1;
        anim2d_load_named(g_lieu_level, n->id48, n->name);
        return 1;

    case LA_FREEANIM:                          /* case 0x27 */
        if (n->id48 >= 0 && n->id48 < LIEU_ANIM_SLOTS) g_anim_slot[n->id48] = 0;
        print("lieu_act: FREEANIM slot=%d\n", n->id48);
        return 1;

    case LA_BLOQUE:                            /* case 0x14 @ 0x44BC8D */
        if (dlg_is_active()) return 0;
        if ((n->num5C & 4) && g_player_entity) {
            E32(g_player_entity, 0xCEC) = 0; E32(g_player_entity, 0xCF0) = 1;
            E32(g_player_entity, 0xCF4) = 0; E32(g_player_entity, 0xCF8) = 0;
        }
        g_cam_lock = (n->p60 != 0);            /* dword_4FF0A0 */
        if (getenv("DKE_AIDBG")) print("lieu_act: BLOQUE %d cam=%d move=%d\n", n->num5C, g_cam_lock, n->p64 != 0);
        g_player_blocked = (n->p64 != 0);
        if (n->num5C == 0 && g_player_entity &&
            (E32(g_player_entity, 0xD58) == -5 || E32(g_player_entity, 0xD58) == 0x3B))
            E32(g_player_entity, 0xE4) = 1;
        return 1;

    case LA_MODIFYVAR: {                       /* case 0x1C @ 0x44C5F7 */
        i32 pile = lieu_action_run_pile();
        entity_t *e = actor_find(g_pile_actor[(pile >= 0 && pile < 3) ? pile : 0]);
        if (e && n->prev_type != LA_JOINDRE && !actor_anim_done(e)) return 0;
        if (n->slot74) *n->slot74 = n->num5C;
        return 1;
    }

    default:
        print("lieu_act: %s (type %d) not wired — completing\n",
              lieu_action_name(n->type), n->type);
        return 1;
    }
}

/* -------------------------------------------------------------------------
 * Public driver — scene.c calls these.
 * ---------------------------------------------------------------------- */

/* sub_448F4D @ 0x4494B2..0x449530: run the script's INIT, then drain every
 * action it queued before the level starts (with dword_4B52DC = -1, which
 * makes is_entity_in_animation report "done"), then fire trigger 41
 * (sub_44A263, "room entered"). */
/* The current level's script entry (NULL: not ported). */
static i32 lieu_call(i32 op, i32 arg, i32 arg2) {
    lieu_entry_t fe = lieu_level_entry(g_lieu_level);
    return fe ? fe(op, g_lieu_state, arg, arg2) : 0;
}

static void lieu_run_init(void) {
    g_lieu_loading = 1;
    lieu_call(LIEU_OP_INIT, 0, 0);                                /* op 1 */
    u32 t0 = state ? state->current_game_time : 0;
    for (i32 guard = 0; lieu_action_pending() && guard < 100000; guard++)
        lieu_action_run(t0 + (u32)guard);
    lieu_call(LIEU_OP_EVENT, 0x29, 0);                            /* op 3 (41) */
    g_lieu_loading = 0;
}
/* sub_43A63D @ 0x43A79B: a new game clears the script variables. */
void lieu_new_game(void) {
    memset(g_lieu_state, 0, sizeof(g_lieu_state));
    /* L01 trig00's entry phase: 1 = the opening of a new game (L01 INIT
     * clears it, so later visits take the persisted path). */
    LIEU_ST(g_lieu_state, ST_START_POINT(1)) = 1;
}

/* sub_4640BC @ 0x464306: POSPERSO records a persona's life mode in
 * dword_505DD0[id] (script ST(0x190 + id*4)); death sets it to 2. */
/* game_death_state (ST 0x194): the hero, or 4 once the player is dead. */
void lieu_set_death_state(i32 v) { LIEU_ST(g_lieu_state, 0x194) = v; }

void lieu_set_persona_vie(i32 id, i32 vie) {
    if (id >= 4 && id < (i32)((sizeof(g_lieu_state) - 0x190) / 4))
        LIEU_ST(g_lieu_state, 0x190 + id * 4) = vie;
}

void lieu_enter(i32 level_id) {
    g_lieu_level = level_id;
    g_lieu_cb    = &g_lieu_cb_impl;
    /* dword_505C40 is game-wide: only a new game clears it (lieu_new_game). */
    zones_clear();   /* engine memsets dword_506C80 to -1 before INIT arms */
    ents_clear();    /* reset entity alive/HP/visibility state              */
    objs_clear();    /* object/flag/anim-slot registries                    */
    lieu_action_reset();  /* drop stale actions + object timers            */
    g_spawn_count = 0;
    g_op9c_counter = 0;
    /* sub_448F4D @ 0x449021: every room starts with the camera and the
     * player free and the zones answering only the action key. */
    g_cam_lock = 0;          /* dword_4FF0A0 */
    g_player_blocked = 0;    /* player_movement_enabled */
    g_force_zones = 0;       /* dword_4FFB80 */
    dlg_reset();
    /* Trace on via env LIEU_TRACE=1 (observe opcode dispatch at runtime). */
    g_lieu_trace = (getenv("LIEU_TRACE") != NULL);

    if (!lieu_level_entry(level_id)) {
        print("lieu: level %02d has no native scripting yet\n", level_id);
        return;
    }

    /* ST(0x194) is game_death_state (dword_505DD4): the hero being played,
     * 0..3 (DKE.CFG FirstPerso = 0 for a new game). INIT only plays SC22 for
     * the value 4. ST_START_POINT(1) = 1 selects trig00's fresh-entry setup.
     * TODO: take both from the engine's game state once hero swaps exist. */
    LIEU_ST(g_lieu_state, 0x194) = actor_hero_id();
    /* +0x0C is dword_505C4C, the per-frame action-key flag sub_449F9F
     * writes (see lieu_detect_zone); it is not a constant. */

    print("lieu: LIEU%02d native scripting — running INIT\n", level_id);
    lieu_common_reset();   /* the DLL is loaded afresh for every room */
    lieu_run_init();
}

/*
 * lieu_detect_zone — per-frame zone lookup feeding op 3, mirroring the
 * original engine's sub_449F9F: read player position (player_entity[0x50]
 * +0x3C/+0x44 = X/Z) then sub_449E03(X,Z) -> zone id (or -1).
 *
 * dke has no live player transform or level zone geometry yet, so this
 * returns -1 (no zone -> no trigger). Wire it when those exist:
 *   1. export player_entity + its transform,
 *   2. load L01 zone rectangles (LCOD table),
 *   3. point-in-zone test -> zone id.
 */
i32 lieu_detect_zone(void) {                              /* sub_449E03 */
    i32 px, pz;
    if (!player_get_pos(&px, NULL, &pz)) return -1;
    /* 0x44A179: process_input_event(0x20) is a key-down EDGE, so the
     * action flag is 1 for one frame per press. A press while the player is
     * locked keeps the previous value; no press clears it. */
    static i32 s_key_prev = 0;
    i32 edge = g_action_key && !s_key_prev;
    s_key_prev = g_action_key;
    if (g_player_entity && E32(g_player_entity, 0xD60) == 0 && edge) {
        if (!g_player_blocked) { g_action_pressed = 1; g_force_zones = 1; }
    } else {
        g_action_pressed = 0;
        g_force_zones = 0;
    }
    /* The script reads these through its state buffer (dword_505C40). */
    LIEU_ST(g_lieu_state, 0x0C) = g_action_pressed;   /* dword_505C4C */
    LIEU_ST(g_lieu_state, 0x04) = g_updown_key;       /* dword_505C44 */
    LIEU_ST(g_lieu_state, ST_GAME_TIME) = (i32)lo_now();  /* 0x44A22D: dword_5065A0 */
    if (getenv("DKE_LIEUDBG")) {
        static u32 last = 0;
        u32 t = state ? state->current_game_time : 0;
        if (t - last > 500) {
            last = t;
            print("lieudbg: t=%u key=%d blocked=%d st0C=%d st30=%d st708=%d st68=%d facing=%04X q998=%d pend=%d d64=%d d67=%d z6=(%d,%d) z4=(%d,%d)\n",
                  t, g_action_key, g_player_blocked, LIEU_ST(g_lieu_state, 0x0C), LIEU_ST(g_lieu_state, 0x30),
                  LIEU_ST(g_lieu_state, 0x708), LIEU_ST(g_lieu_state, 0x68), player_get_facing(),
                  cb_direction_spec(0x8660), lieu_action_pending(), cb_distance(6, 4), cb_distance(6, 7), g_zones[6].center_x, g_zones[6].center_z, g_zones[4].center_x, g_zones[4].center_z);
        }
    }
    zones_follow();
    for (i32 i = 1; i < LIEU_ZONE_COUNT; i++) {           /* first hit wins  */
        zone_t *zn = &g_zones[i];
        if (zn->center_x == -1 && !zn->entity) continue;  /* empty slot      */
        if (zn->active != 1 && g_force_zones != 1) continue;
        i32 cx = zn->center_x, cz = zn->center_z;
        if (zn->entity) {
            entity_t *e = (entity_t *)zn->entity;
            if (e == g_player_entity || !EROOT(e)) continue;
            cx = EROOT(e)->pos[0];
            cz = EROOT(e)->pos[2];
        }
        i32 dx = cx - px, dz = cz - pz;
        i32 hx = zn->size_x / 2, hz = zn->size_z / 2;
        if ((dx < 0 ? -dx : dx) > hx) continue;
        if ((dz < 0 ? -dz : dz) > hz) continue;
        return i;                                         /* == trigger idx  */
    }
    return -1;
}

void lieu_tick(u32 now_ms) {
    zones_follow();
    /* Per-frame: engine calls op -1 every frame (drain + tick_body). */
    lieu_call(LIEU_OP_TICK, 0, 0);   /* op -1 */
    /* sub_44968B @ 0x449AF6: an item used on the inventory screen reaches
     * the script as event 5 (object id, instance) once play resumes. */
    if (g_used_object && g_ui_state == 1) {
        lieu_pending_event(g_used_object, g_used_variant);
        g_used_object = 0;
    }

    (void)now_ms;   /* the piles are drained by actor_frame (sub_461468 order) */
}

void lieu_exit(void) {
    /* Teardown: engine op 2 (DLL handler is a no-op). */
    lieu_call(LIEU_OP_TEARDOWN, 0, 0); /* op 2 */
    g_lieu_level = 0;
}


/* -------------------------------------------------------------------------
 * Runtime event feeders. The original engine drives these from zone
 * detection (sub_449F9F -> op 3), object timers (op 4) and a pending-event
 * flag (op 5). dke has none of those subsystems yet, so nothing calls the
 * feeders during normal play — call them from a debug keybind (or wire to
 * real engine systems later) to exercise the trigger / event handlers.
 * ---------------------------------------------------------------------- */
void lieu_zone_trigger(i32 zone_id) {   /* op 3: run trigger[zone_id]      */
    lieu_call(LIEU_OP_EVENT, zone_id, 0);
}
void lieu_object_timer(i32 id) {        /* op 4: event4(id)                */
    lieu_call(LIEU_OP_CALL4, id, 0);
}
void lieu_pending_event(i32 v, i32 phase) { /* op 5: event5(v, phase)      */
    lieu_call(LIEU_OP_CALL5, v, phase);
}

/* Read/write the live state buffer (so the engine can populate the fields
 * handler guards read: 0xC, 0x18, 0x194 submode, 0x964 phase, ...). */
const c8 *lieu_divers_text(void) { return g_byte_507260; }

i32 *lieu_state_slot(i32 off) { return &LIEU_ST(g_lieu_state, off); }

/* Debug: place persona `id` next to the player and start a fight (the
 * script's POSPERSO + COMBAT), the hero on autofight (dword_4B219C). */
void lieu_debug_fight(i32 id, const c8 *clip) {
    entity_t *p = g_player_entity;
    if (!p || !EROOT(p)) return;
    i32 x = EROOT(p)->pos[0], z = EROOT(p)->pos[2] + 50000;   /* world units, open floor */
    actor_pos_perso(id, x, z, 0x4000, 1, clip, -1, 0);
    combat_set(id, 0);
    combat_set(0, 0);
    print("debug: fight with persona %d at (%d,%d)\n", id, x, z);

}
