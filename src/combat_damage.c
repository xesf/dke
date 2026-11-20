/*
 * combat_damage_draft.c -- DRAFT, not compiled. Combat resolution for dke
 * (melee hit test, parry/clash, damage, hit reactions, death, enemy HP bar).
 * Proposed home: src/combat.c + src/combat.h. Every function cites its
 * dkev.exe original. Offsets are the original entity/desc/anim offsets used
 * through the port accessors (E32/EPTR/EDESC/EBONE, can_rec_t hdr).
 *
 * INSERTION POINTS (do these in src/, not done here):
 *  1. actor_move.c anim_messages(): replace the "0x11 / 0x15 ... not ported"
 *     comment with
 *         if (type == 0x11 || type == 0x15 || type == 0x19 || type == 0x24)
 *             combat_anim_message(e, type, p);
 *     (sub_41AED1 @ 0x41BA96 / 0x41BC8F / 0x41C717 / 0x41CB5F).
 *  2. actor_move.c companion_tick(): where "+0xD60 combat AI (sub_4701E0)
 *     not ported" sits, before actor_update(c):
 *         if (E32(c, 0xD60)) combat_npc_tick(c);
 *     (sub_461468 @ 0x46168C calls sub_4701E0 only when +0xD60 != 0; the
 *     damage part is sub_4701E0 -> sub_470A8F, the death part sub_4715D3.)
 *  3. actor_move.c player_tick() (the sub_46642E stand-in), first line:
 *         combat_player_tick();
 *     (sub_46642E @ 0x466935 -> sub_451BF9 / sub_4342C7 combat prologue.)
 *  4. actor_move.c actor_frame(), before the player's actor_update:
 *         combat_regen_tick();          (update_game_progression @ 0x405003)
 *  5. scene.c: export
 *         void scene_hud_refresh(void) { update_player_interface(game_init_get_game_context(), 1); }
 *         i32  scene_score_get(void);  void scene_score_set(i32 v);  (score_counter)
 *     and draw the two enemy bars (update_game_hud case 13 @ 0x467C23) from
 *     combat_enemy_bar_draw().
 *  6. sfx.c: export  void sfx_entity_named(const entity_t *e, const c8 *name)
 *         { sfx_play_entity(e, sfx_get_id(name, g_dir_sfx)); }   (get_sound_id + play_entity_sound)
 *  7. lieu_cb.c: export void lieu_set_death_state(i32 v)
 *         { LIEU_ST(g_lieu_state, 0x194) = v; }   (game_death_state = dword_505DD4)
 *     op 27 -> g_game_exit_code = 3; op 6 -> scene_hud_refresh() / combat_enemy_bar(e);
 *     op 20 -> scene_score_get/set.
 *  8. game_init.c: export the durability tables
 *         i16 *game_init_resource_table(void);   (resource_table @ 0x4DBBE0, 35 x 10 i16)
 */

#include <math.h>
#include <string.h>
#include "entity.h"
#include "actor.h"
#include "obc.h"
#include "can.h"
#include "world.h"
#include "game_init.h"
#include "state.h"
#include "libsys/graphics/graphic.h"
#include "libsys/graphics/shading.h"   /* cosine_table */
#include "object.h"
#include "effect.h"
#include "fx.h"
#include "combat.h"
#include "inventory.h"

void combat_enemy_bar(entity_t *e);

/* ---- hooks still to export (see header) -------------------------------- */
extern void sfx_entity_named(const entity_t *e, const c8 *name);
extern void scene_hud_refresh(void);
extern i32  scene_score_get(void);
extern void scene_score_set(i32 v);
extern void lieu_set_death_state(i32 v);
extern void lieu_set_persona_vie(i32 id, i32 vie);   /* exists (lieu_cb.c) */

static i32 now_ms(void) { return state ? (i32)state->current_game_time : 0; }

/* debug_flags bit 2 = god mode (no damage), game_state >= 2 = weapons wear. */
static i32 g_debug_flags  = 0;   /* debug_flags        */
static i32 g_game_state   = 2;   /* game_state         */

/* ---- anim record fields used by combat (CAN header, flag 0x40 clips) ----
 * +0xFA flags: 0x40 = strike clip (has a striker bone and an active window),
 *              0x80 = clip may not be interrupted (end_entity_invulnerability),
 *              0x200 = hold last frame.
 * +0xFC = striker bone slot when flags & 0x40 ("Frappeur", sub_429695 @ 0x42970B).
 *         The same dword is the root yaw for flag-1 clips (CAN_ROOT_YAW).
 * +0x100 = active window: lo16 first tick, hi16 last tick (sub_4295E0 @ 0x429613).
 * +0x104 = strike power byte (calculate_entity_damage @ 0x4010F5). */
#define CAN_STRIKER(r)   (*(const i32 *)&(r)->hdr[0xFC])
#define CAN_WINDOW(r)    (*(const i32 *)&(r)->hdr[0x100])
#define CAN_POWER(r)     ((r)->hdr[0x104])
#define ANIM(e)          ((can_rec_t *)EPTR(e, 0xD50))
#define HP(e)            (*(i32 *)(EDESC(e) + 0x84))
#define PERSONA(e)       E32(e, 0xB0)

/* =========================================================================
 * State classes (animation state = entity+0xD04)
 *   1..6 locomotion, 7..12 melee attacks (7/10 = A, 8/11 = B, 9/12 = C),
 *   14 shooting, 15 victory/taunt, 0x10..0x15 + 0x1E..0x35 guard/parry,
 *   0x16 light hit, 0x18 knock-down, 0x1A death, 0x1B/0x1C heavy hits,
 *   0x1D special charge -> 0x3C special strike, 6 = get up.
 * ========================================================================= */
static i32 st_attacking(const entity_t *e) {          /* is_creature_special_level_type @ 0x4011FB */
    if (!e) return 0;
    i32 s = E32(e, 0xD04);
    return (s >= 7 && s <= 12) || s == 0x1D || s == 0x3C || s == 0x0E;
}
static i32 st_guarding(const entity_t *e) {           /* is_creature_in_special_level_range @ 0x401194 */
    if (!e) return 0;
    i32 s = E32(e, 0xD04);
    return (s >= 0x10 && s <= 0x15) || (s >= 0x1E && s <= 0x35);
}
static i32 st_reacting(const entity_t *e) {           /* is_master_eligible_creature @ 0x40126C */
    if (!e) return 1;                                  /* NULL counts as "busy" */
    i32 s = E32(e, 0xD04);
    return s >= 0x16 && s <= 0x1C && s != 0x17 && s != 0x19;
}

/* combat_ability_table (OBJETS.TXT, creature_entry_t.flags) by desc+0x97
 * (held weapon class): 4 sabre, 8 hache, 0x10 foudre, 0x20 lance-feu,
 * 0x40 cracker, 0x80 pistolet. */
static u8 weapon_flags(const entity_t *e) {
    const creature_entry_t *c = e && EDESC(e) ? game_init_get_creature(EDESC(e)[0x97]) : NULL;
    return c ? c->flags : 0;
}
/* combat_ability_table[id*20] (0x4DB860) = the OBJETS.CTX record flags. */
static u8 class_flags(i32 cls) { return (cls > 0 && cls < 100) ? objets_flags(cls) : 0; }
static i32 has_ranged_weapon(const entity_t *e) {     /* check_entity_special_abilities @ 0x401129 */
    return e && (weapon_flags(e) & 0xF0) != 0;
}

/* =========================================================================
 * Blood burst / impact position. The original keeps it in four globals
 * misnamed sound_enabled / sound_param1..3: the hit test writes the victim
 * bone (or 9 for a bullet) and the contact midpoint, the damage routine
 * then calls play_sound_at_position @ 0x42CDA6 (really a 32-particle blood
 * spray, not a sound) at that point, heading = victim yaw + 0x4000.
 * ========================================================================= */
static i32 g_blood = 0;            /* sound_enabled  */
static i32 g_blood_pos[3];         /* sound_param1 (x), sound_param3 (y), sound_param2 (z) */
static i32 g_spark_pos[3];         /* effect_position_reference (parry spark)            */
static i32 g_last_block_time = 0;  /* last_block_time */
static i32 g_damage_counter  = 0;  /* damage_counter / dword_506520 (unused stats) */

static void blood_burst(const entity_t *victim) {
    /* process_entity_damage @ 0x401590 / sub_470507 @ 0x470564: x = param1,
     * y = param3, z = param2, heading = victim root rot[2] + 0x4000. */
    const obc_node_t *r = EROOT(victim);
    fx_blood_start(g_blood_pos[0], g_blood_pos[1], g_blood_pos[2], (r ? r->rot[2] : 0) + 0x4000);
}

/* =========================================================================
 * Collision spheres ("Clefs"). obc_build_control_points gives limb nodes
 * 1..4 spheres (obc_clefs_t: radius, centre = node-local). The original
 * refreshes the world centre (+0x10..+0x18, obc_clefs_t.reserved[]) once per
 * frame in the actor renderer: sub_420D80 @ 0x4211F8 -> sub_493850 @ 0x493850
 * = transform_vector(node world matrix, centre). Here we compute it on demand
 * with the same matrix chain as node_world (actor_move.c sub_41FDC2).
 * ========================================================================= */
static void node_matrix(obc_node_t *root, obc_node_t *n, fmat_t *acc) {
    obc_node_t *chain[64];
    i32 i = 0;
    chain[0] = n;
    while (chain[i] != root && chain[i]->parent && i < 63) { chain[i + 1] = chain[i]->parent; i++; }
    memset(acc, 0, sizeof(*acc));
    acc->m[0] = acc->m[4] = acc->m[8] = 0x4000;
    for (; i >= 0; i--) {
        obc_node_t *k = chain[i];
        fmat_t r, p;
        fmat_rotation(&r, k->rot[0], k->rot[1], k->rot[2]);
        fmat_position(&p, k->pos[0], k->pos[1], k->pos[2]);
        fmat_rotate_translation(acc, &p);
        fmat_premul_rotation(&r, acc);
        acc->t[0] += p.t[0]; acc->t[1] += p.t[1]; acc->t[2] += p.t[2];
    }
}
/* Refresh every clef world centre of `n` (sub_493850). */
static void clefs_world(entity_t *e, obc_node_t *n) {
    if (!n || n->has_clefs <= 0 || !n->clefs) return;
    fmat_t m;
    node_matrix(EROOT(e), n, &m);
    for (i32 i = 0; i < n->has_clefs; i++) {
        i32 v[3] = { n->clefs[i].centre[0], n->clefs[i].centre[1], n->clefs[i].centre[2] };
        fmat_transform_point(&m, v);
        n->clefs[i].reserved[0] = v[0];
        n->clefs[i].reserved[1] = v[1];
        n->clefs[i].reserved[2] = v[2];
    }
}

/* AABB pre-test then (r1+r2)^2 > d^2, exactly as sub_429695 @ 0x429A79..0x429CE0.
 * On contact writes the midpoint to *mid (when non-NULL). */
static i32 spheres_touch(const obc_clefs_t *a, const obc_clefs_t *b, i32 *mid) {
    f64 ra = (f64)(u32)a->radius, rb = (f64)(u32)b->radius;   /* fild of a zero-extended qword */
    const i32 *pa = a->reserved, *pb = b->reserved;
    for (i32 k = 0; k < 3; k++) {
        if (!((f64)pb[k] - rb <= (f64)pa[k] + ra)) return 0;
        if (!((f64)pb[k] + rb >= (f64)pa[k] - ra)) return 0;
    }
    f64 dx = pa[0] - pb[0], dy = pa[1] - pb[1], dz = pa[2] - pb[2];
    f64 rr = (ra + rb) * (ra + rb);
    if (!(dz * dz + dx * dx + dy * dy < rr)) return 0;
    if (mid) {
        mid[0] = (pa[0] + pb[0]) / 2;
        mid[1] = (pa[1] + pb[1]) / 2;
        mid[2] = (pa[2] + pb[2]) / 2;
    }
    return 1;
}

/* dword_4B44B0: the 15 victim bone slots that can be hit, in test order. */
static const i32 k_body[15] = { 0x0E, 0x10, 0x0D, 0x0F, 0x09, 0x01, 0x03, 0x02,
                                0x04, 0x06, 0x05, 0x07, 0x11, 0x0B, 0x0C };

/* sub_4295E0 @ 0x4295E0 — attacker's strike is live this tick. */
static i32 strike_live(const entity_t *a) {
    if (E32(a, 0xD18) || E32(a, 0xD1C)) return 0;  /* D18 is never set in dkev */
    const can_rec_t *r = ANIM(a);
    if (!r || !(CAN_FLAGS(r) & 0x40)) return 0;
    i32 w = CAN_WINDOW(r);
    i32 t = E32(a, 0xC40);                         /* current clip tick */
    return t >= (w & 0xFFFF) && t <= (w >> 16);
}

/* sub_429695 @ 0x429695 — striker bone(s) of `a` against the body of `v`.
 * Striker list: bone[anim+0xFC]; +its first child when it is "arme" (0x1F),
 * +its parent when it is "armeb" (0x22); for arme/poidro/poigau/piegau/piedro
 * (0x1F, 0x0E, 0x10, 0x07, 0x05) also walk up the parents while the list has
 * < 4 entries, keeping ancestors that are one of the 15 body slots. */
static i32 strike_hits(entity_t *a, entity_t *v) {
    if (!a || !v || !ANIM(a) || !(CAN_FLAGS(ANIM(a)) & 0x40)) return 0;
    i32 sb = CAN_STRIKER(ANIM(a));
    if (sb < 0 || sb >= ENTITY_BONE_SLOTS) return 0;
    obc_node_t *list[4]; u32 n = 0;
    obc_node_t *s = EBONE(a, sb);
    if (!s) return 0;                              /* log "Frappeur message in %s %d" */
    list[n++] = s;
    if (sb == 0x1F)      list[n++] = s->child;     /* node+0x7C */
    else if (sb == 0x22) list[n++] = s->parent;    /* node+0x78 */
    if (sb == 0x1F || sb == 0x0E || sb == 0x10 || sb == 7 || sb == 5) {
        obc_node_t *p = s;
        while (n < 4 && p->parent) {
            p = p->parent;
            for (i32 k = 0; k < 15; k++)
                if (EBONE(a, k_body[k]) == p) { list[n++] = p; break; }
        }
    }
    for (u32 i = 0; i < n; i++) {
        obc_node_t *an = list[i];
        if (!an) return 0;                         /* original stops at the first NULL */
        clefs_world(a, an);
        for (i32 k = 0; k < 15; k++) {
            obc_node_t *vn = EBONE(v, k_body[k]);
            if (!vn) continue;
            clefs_world(v, vn);
            for (i32 x = 0; x < an->has_clefs; x++)
                for (i32 y = 0; y < vn->has_clefs; y++)
                    if (spheres_touch(&an->clefs[x], &vn->clefs[y], g_blood_pos)) {
                        g_blood = k_body[k];       /* sound_enabled = hit bone */
                        return 1;
                    }
        }
    }
    return 0;
}

/* check_entities_collision @ 0x429D4E — weapon vs weapon. Both clips must be
 * strike clips (flag 0x40). Each side tests two nodes: its striker bone when
 * attacking, else the weapon bone 0x1F; then that node's child (0x1F) or
 * parent (0x22). Contact point (attacker-side sphere) -> g_spark_pos. */
static obc_node_t *clash_node(entity_t *e, i32 pass) {
    i32 b = st_attacking(e) ? CAN_STRIKER(ANIM(e)) : 0x1F;
    if (b < 0 || b >= ENTITY_BONE_SLOTS) return NULL;
    obc_node_t *n = EBONE(e, b);
    if (!n || pass == 0) return n;
    if (b == 0x1F) return n->child;
    if (b == 0x22) return n->parent;
    return NULL;
}
static i32 weapons_clash(entity_t *a, entity_t *b) {
    if (!a || !b || !ANIM(a) || !ANIM(b)) return 0;
    if (!(CAN_FLAGS(ANIM(a)) & 0x40) || !(CAN_FLAGS(ANIM(b)) & 0x40)) return 0;
    for (i32 i = 0; i < 2; i++) {
        obc_node_t *na = clash_node(a, i);
        if (!na) return 0;
        for (i32 j = 0; j < 2; j++) {
            obc_node_t *nb = clash_node(b, j);
            if (!nb) break;
            clefs_world(a, na); clefs_world(b, nb);
            for (i32 x = 0; x < na->has_clefs; x++)
                for (i32 y = 0; y < nb->has_clefs; y++)
                    if (spheres_touch(&na->clefs[x], &nb->clefs[y], NULL)) {
                        memcpy(g_spark_pos, na->clefs[x].reserved, sizeof(g_spark_pos));
                        return 1;
                    }
        }
    }
    return 0;
}

/* =========================================================================
 * Parry sparks — process_parry_effects @ 0x402EFC, render_parry_effects
 * @ 0x403057. 16x16 sprite of 8 random rays from (8,8), colour
 * 0x9B + 50*rand(3), shown at g_spark_pos (camera-projected, +-600 jitter
 * per axis) for 100 ms. TODO: port the overlay; state only here.
 * ========================================================================= */
static i32 g_spark_active = 0, g_spark_t0 = 0;
static void parry_sparks(void) { g_spark_active = 1; g_spark_t0 = now_ms(); }

/* =========================================================================
 * Weapon wear — consume_action_resource @ 0x4012D6 + execute_counter_attack
 * @ 0x411A00 (misnamed: it is "weapon breaks"). Only weapons whose name
 * starts with 'O' wear. resource_table[action_index*10 + (obj[0xAC]-'A')]
 * -= cost; at <= 0 the object is renamed 'Q...' (broken), player+0xD2[0] =
 * 'Q', the "break" sample plays and a shard is thrown (process_counter_attack).
 * Callers: cost 1 per landed player hit (calculate_entity_damage, needs the
 * striker bone >= 0x1F = weapon) and per anim message 0x24; cost 2 per clash.
 * ========================================================================= */
static void weapon_wear(i32 cost) {
    entity_t *p = g_player_entity;
    if (!p || ESTR(p, 0xD2)[0] != 'O' || (g_debug_flags & 2) || g_cinematic_mode || g_game_state < 2)
        return;
    game_object_t *o = (game_object_t *)EPTR(p, 0xD64);    /* get_entity_character(p+0xD2) */
    if (!o || !inv_wear(o->name, cost)) return;
    /* execute_counter_attack @ 0x411A00: sabres and axes break, except 5, 61, 62. */
    i32 id = 0;
    if (sscanf(o->name + 2, "%2d", &id) != 1 || id == 5 || id == 61 || id == 62) return;
    if (!(class_flags(id) & 0x0C)) return;
    /* process_counter_attack(.., 0) @ 0x410F76: the blade ("armeb", the
     * last node of the weapon chain) comes off. The flying piece is not drawn. */
    obc_node_t *n = o->root;
    while (n && n->child) n = n->child;
    if (n && n->parent && strcmp(n->name, "armeb") == 0) n->parent->child = NULL;
    c8 old[8];
    memcpy(old, o->name, sizeof(old));
    inv_weapon_break(o->name);                             /* also renames o->name to 'Q' */
    for (i32 i = 0; i < OBJ_SLOTS; i++) {                  /* other slots with this item */
        game_object_t *s2 = object_slot(i);
        if (s2 && s2 != o && s2->used && strncmp(s2->name, old, 4) == 0) s2->name[0] = 'Q';
    }
    ESTR(p, 0xD2)[0] = 'Q';
    sfx_entity_named(p, "break");
}

/* Headless test hook (DKE_WEARTEST). */
void combat_debug_wear(i32 cost);
void combat_debug_wear(i32 cost) { weapon_wear(cost); }

/* =========================================================================
 * Damage — calculate_entity_damage @ 0x401000.
 * dmg = anim[0x104] * base * 8 / 100, base 100. With a broken weapon
 * (player+0xD2 == 'Q') the player's base drops to 89 (sabre) or 64 (axe,
 * wins if both). No randomness, no armour: the victim never enters it.
 * ========================================================================= */
static i32 calc_damage(entity_t *a) {
    i32 base = 100;
    entity_t *p = g_player_entity;
    if (a == p && !g_cinematic_mode) {
        if (ESTR(p, 0xD2)[0] == 'O' && ANIM(a) && CAN_STRIKER(ANIM(a)) >= 0x1F)
            weapon_wear(1);
        if (ESTR(p, 0xD2)[0] == 'Q') {
            if (class_flags(EDESC(p)[0x97]) & 0x04) base = 0x59;
            if (class_flags(EDESC(p)[0x97]) & 0x08) base = 0x40;
        }
    }
    i32 power = ANIM(a) ? CAN_POWER(ANIM(a)) : 0;
    return (power * base * 8) / 100;
}

/* check_entity_revival @ 0x402414 — the floor zone the actor stands in is
 * small (area <= 3.6e9): no room to fall, so no knock-down. */
static i32 cramped(const entity_t *e) {
    const u8 *z = (const u8 *)EPTR(e, 0xA8);
    if (!z) return 0;
    f64 w = (f64)(*(const i32 *)(z + 0x1E8) - *(const i32 *)(z + 0x1DC));
    f64 h = (f64)(*(const i32 *)(z + 0x1F0) - *(const i32 *)(z + 0x1E4));
    return h * w <= 3.6e9;
}
/* -1: cramped zone, no fall (caller picks a light reaction);
 * else 0x18, or 0x16 when the persona has no state-0x18 clip ('x'). */
static i32 knockdown_state(entity_t *v) {
    i32 wc_floor = g_world.world_controller ? *(const i32 *)((const u8 *)g_world.world_controller + 0x1E0) : 0;
    if (wc_floor != E32(v, 0x8C) && cramped(v)) return -1;
    return EDESC(v)[0x1D7] == 'x' ? 0x16 : 0x18;   /* desc+0x117+0x18*8 */
}

/* Death clip, played at once: D04 = D08 = 0x1A. */
static void play_death(entity_t *v) {
    c8 clip[16] = {0};
    E32(v, 0xD04) = 0x1A;
    E32(v, 0xD08) = 0x1A;
    actor_bake_root_yaw(v);                        /* process_entity_state_change */
    if (actor_prepare_anim(v, 0x1A, clip))         /* desc+0x117+0x1A*8 = desc+0x1E7 */
        actor_play_anim(v, can_get_anim(clip), 0);
}

/* process_entity_damage @ 0x40152B — `a` (enemy) has hit the player `v`. */
static void damage_player(entity_t *a, entity_t *v) {
    if (st_reacting(v)) return;                    /* already reeling: invulnerable */
    g_damage_counter++;
    if (g_blood && !(g_debug_flags & 2)) blood_burst(v);
    i32 sb = ANIM(a) ? CAN_STRIKER(ANIM(a)) : 0;
    sfx_entity_named(v, (sb == 0x1F || sb == 0x22) ? "impactd" : "impacta");
    i32 pv = PERSONA(v);
    if (!(g_debug_flags & 2)) {
        if (pv != 0x1A && pv != 0x1B && pv != 0x54 && pv != 0x19 && pv != 0x28)
            HP(v) -= calc_damage(a);
        if (v == g_player_entity) scene_hud_refresh();
    }
    if (E32(a, 0xD04) == 0x3C) {                   /* special strike: forced heavy hit + kill */
        u8 w = EDESC(a)[0x97];
        E32(v, 0xD04) = (w == 0 || (class_flags(w) & 0x10)) ? 0x1B : 0x1C;
        if (!(g_debug_flags & 2) && pv != 0x1E && pv != 0x1F && pv != 0x5A && pv != 0x19 &&
            pv != 0x1A && EDESC(v)[0x98] != 3)
            HP(v) = 0;
        return;
    }
    if (HP(v) <= 0) {
        play_death(v);
        if (v == g_player_entity) lieu_set_death_state(4);   /* game_death_state = 4 */
        return;
    }
    E32(v, 0xC64) -= E32(v, 0xC68);                /* stagger meter (0x10000, step 0x4000) */
    if (E32(v, 0xC64) <= 0) {
        if (!(g_debug_flags & 2)) E32(v, 0xC68) = (i32)((f64)E32(v, 0xC68) * 1.125);  /* damage_multiplier */
        E32(v, 0xC64) = 0x10000;
        i32 k = knockdown_state(v);
        if (k >= 0) { E32(v, 0xD04) = k; return; }
        /* cramped: falls through to the reaction below (0x4018DD) */
    }
    if (v != g_player_entity) { E32(v, 0xD04) = 0x16; return; }
    switch (E32(a, 0xD04)) {                       /* jpt_40195A, by attacker's move */
    case 7: case 10: case 14: E32(v, 0xD04) = 0x16; break;
    case 8: case 11:          E32(v, 0xD04) = 0x1B; break;
    case 9: case 12:          E32(v, 0xD04) = 0x1C; break;
    default: break;
    }
}

/* sub_470507 @ 0x470507 — the player has hit enemy `v`. */
static void damage_npc(entity_t *v) {
    entity_t *p = g_player_entity;
    if (st_reacting(v)) return;
    g_damage_counter++;
    if (g_blood && !(g_debug_flags & 2)) blood_burst(v);
    i32 sb = ANIM(p) ? CAN_STRIKER(ANIM(p)) : 0;
    sfx_entity_named(v, (sb == 0x1F || sb == 0x22) ? "impactd" : "impacta");
    i32 pv = PERSONA(v);
    if (!(g_debug_flags & 2) && pv != 0x1A && pv != 0x1B && pv != 0x54 && pv != 0x19) {
        HP(v) -= calc_damage(p);
        combat_enemy_bar(v);
    }
    if (E32(p, 0xD04) == 0x3C) {                   /* special strike */
        if (pv == 0x28) {                          /* boss: stagger only */
            E32(v, 0xD04) = 0x18; E32(v, 0xD70) = 1; E32(v, 0xC64) = 0x10000;
            return;
        }
        i32 alive = HP(v) > 0;
        i32 tough = EDESC(v)[0x98] == 3 || pv == 0x1E || pv == 0x1F || pv == 0x5A;
        u8 w = EDESC(p)[0x97];
        if (pv == 0x19 || pv == 0x1A || (tough && alive) ||
            ((w == 0 || (class_flags(w) & 0x10)) && !tough))
            E32(v, 0xD04) = 0x1B;
        else
            E32(v, 0xD04) = 0x1C;
        if (!(g_debug_flags & 2) && !tough && pv != 0x19 && pv != 0x1A)
            HP(v) = 0;
        return;
    }
    if (HP(v) <= 0) { play_death(v); return; }
    if (pv == 0x28) {                              /* boss counts hits instead */
        E32(v, 0xC64)++;
        combat_enemy_bar(v);
        if (E32(v, 0xC64) == 0x10004) {
            E32(v, 0xD04) = 0x18; E32(v, 0xD70) = 1; E32(v, 0xC64) = 0x10000;
        } else E32(v, 0xD04) = 0x16;
        return;
    }
    E32(v, 0xC64) -= E32(v, 0xC68);
    combat_enemy_bar(v);
    if (E32(v, 0xC64) > 0) { E32(v, 0xD04) = 0x16; return; }
    if (!(g_debug_flags & 2)) E32(v, 0xC68) = (i32)((f64)E32(v, 0xC68) * 1.125);   /* dbl_4B08B8 */
    E32(v, 0xC64) = 0x10000;
    i32 k = knockdown_state(v);
    E32(v, 0xD04) = k < 0 ? 0x16 : k;
}

/* process_entity_damage_attempt @ 0x401DAD — enemy `a` vs player `v`. */
static i32 attempt_on_player(entity_t *a, entity_t *v, i32 dist) {
    if (!a || !v || HP(v) <= 0 || !st_attacking(a)) return 0;
    if (has_ranged_weapon(a) && E32(v, 0xC5C) <= dist) {   /* bullet, flagged by the shot */
        if (!E32(v, 0xD1C)) return 0;
        damage_player(a, v);
        E32(v, 0xD1C) = 0;
        E32(a, 0xC64) = 0x10000;
        return 1;
    }                                              /* closer than C5C: melee test below */
    if (!strike_live(a) || !strike_hits(a, v)) return 0;
    if (E32(a, 0xD04) == 0x3C) {
        g_blood = 0;
        damage_player(a, v);
        E32(a, 0xC64) = 0x10000;
        return 1;
    }
    if (st_guarding(v)) return 0;                  /* parried: no damage */
    i32 s = E32(a, 0xD04);
    if (s < 7 || s > 12) return 0;
    damage_player(a, v);
    E32(a, 0xC64) = 0x10000;
    return 1;
}

/* sub_470A8F @ 0x470A8F — player vs enemy `v` (from the enemy's AI tick). */
static i32 attempt_on_npc(entity_t *v) {
    entity_t *p = g_player_entity;
    if (!p || !st_attacking(p) || !strike_live(p) || !strike_hits(p, v)) return 0;
    if (E32(p, 0xD04) != 0x3C) {
        if (st_guarding(v)) return 0;
        i32 s = E32(p, 0xD04);
        if (s < 7 || s > 12) return 0;
    }
    damage_npc(v);
    E32(p, 0xC64) = 0x10000;
    return 1;
}

/* process_combat_block @ 0x40199A — player `d` vs enemy `a` weapons meet. */
static void combat_block(entity_t *d, entity_t *a) {
    if (!d || !a || E32(d, 0xD18) || E32(a, 0xD18)) return;
    if (has_ranged_weapon(d) || has_ranged_weapon(a)) return;
    if (now_ms() - g_last_block_time <= 2000) return;
    if (!weapons_clash(d, a)) return;
    g_last_block_time = now_ms();                  /* armed even when no case below matches */
    i32 db = CAN_STRIKER(ANIM(d)), ab = CAN_STRIKER(ANIM(a));
    i32 clash = (db >= 0x1F && ab >= 0x1F && st_attacking(d) && st_attacking(a)) ||
                (ab >= 0x1F && st_guarding(d) && st_attacking(a)) ||
                (db >= 0x1F && st_guarding(a) && st_attacking(d));
    if (!clash) return;
    sfx_entity_named(d, "block1");
    parry_sparks();
    weapon_wear(2);
}

/* end_entity_invulnerability @ 0x401B8D — an action/reaction clip is over:
 * back to idle (5 in combat stance, 0x3B player out of stance, else -5).
 * Finishing the special strike 0x3C adds 100 to score_counter. */
static i32 finish_action(entity_t *e) {
    if (!actor_anim_done(e)) {
        if ((ANIM(e) && (CAN_FLAGS(ANIM(e)) & 0x80)) || st_reacting(e)) return 0;
    }
    i32 att = st_attacking(e);
    if (!(E32(e, 0xD04) != 0 && actor_anim_done(e)) && !(att && E32(e, 0xD18))) return 0;
    if (e == g_player_entity && E32(e, 0xD04) == 0x3C) {
        scene_score_set(scene_score_get() + 100);
        scene_hud_refresh();
    }
    E32(e, 0xD18) = 0; E32(e, 0xD08) = 0; E32(e, 0xD04) = 0;
    actor_bake_root_yaw(e);
    i32 st = E32(e, 0xD60) ? 5 : (!g_special_mode_active && e == g_player_entity) ? 0x3B : -5;
    E32(e, 0xC30) = now_ms();
    c8 clip[16] = {0};
    if (!actor_prepare_anim(e, st, clip)) return 0;
    if (!actor_play_anim(e, can_get_anim(clip), 0)) return 0;
    /* 0x401D1F: when a script holds the player, jump the clip clock to its
     * end (C28 = now - last key time / divisor) and set +0xE4 = 1. TODO. */
    return 1;
}

/* handle_entity_state_transition @ 0x401401 — knock-down 0x18 -> get up 6 ->
 * idle (CEC=0, CF0=1, CF4=CF8=0). */
static i32 player_getup(entity_t *p) {
    if (E32(p, 0xD04) == 0x18) {
        if (actor_anim_done(p)) {
            c8 clip[16] = {0};
            actor_bake_root_yaw(p);
            if (actor_prepare_anim(p, 6, clip)) {
                E32(p, 0xD08) = 6; E32(p, 0xD04) = 6;
                actor_play_anim(p, can_get_anim(clip), 0);
            }
        }
        return 1;
    }
    if (E32(p, 0xD04) == 6) {
        if (actor_anim_done(p)) {
            E32(p, 0xCEC) = 0; E32(p, 0xCF0) = 1;
            E32(p, 0xD08) = 0; E32(p, 0xD04) = 0;
            E32(p, 0xCF8) = 0; E32(p, 0xCF4) = 0;
        }
        return 1;
    }
    return 0;
}

/* sub_4511D5: every engaged enemy (D60 or CD4) is gone (+0x70 == -1). */
static i32 enemies_dead(entity_t *c, entity_t *s) {
    i32 ec = c && (E32(c, 0xD60) || E32(c, 0xCD4));
    i32 es = s && (E32(s, 0xD60) || E32(s, 0xCD4));
    if (!ec && !es) return 0;
    if (ec && E32(c, 0x70) != -1) return 0;
    if (es && E32(s, 0x70) != -1) return 0;
    return 1;
}

/* sub_4512E4 @ 0x4512E4 — player-side state transitions; 1 = skip the rest. */
static i32 player_transitions(entity_t *p) {
    entity_t *c = g_companion_entity, *s = g_secondary_companion_entity;
    if (E32(p, 0xD04) == 0x0F) {                   /* victory pose: wait, then idle */
        if (actor_anim_done(p)) {
            E32(p, 0xD08) = 0; E32(p, 0xD04) = 0;
            E32(p, 0xCEC) = 0; E32(p, 0xCF0) = 1; E32(p, 0xCF8) = 0; E32(p, 0xCF4) = 0;
        }
        return 1;
    }
    if (enemies_dead(c, s)) {
        if (c && HP(c) <= 0) E32(c, 0x70) = 2;
        if (s && HP(s) <= 0) E32(s, 0x70) = 2;
        E32(p, 0xCF0) = 0; E32(p, 0xCEC) = 0; E32(p, 0xCF8) = 0; E32(p, 0xCF4) = 0;
        if (g_debug_flags & 4) { E32(p, 0xD08) = 0; E32(p, 0xD04) = 0; return 1; }
        c8 clip[16] = {0};
        actor_bake_root_yaw(p);
        if (actor_prepare_anim(p, 0x0F, clip)) {
            actor_play_anim(p, can_get_anim(clip), 0);
            E32(p, 0xD04) = 0x0F; E32(p, 0xD08) = 0x0F;
            if (ANIM(p)) CAN_FLAGS(ANIM(p)) |= 0x200;   /* hold last frame */
        }
        return 1;
    }
    if (E32(p, 0xD04) == 0x1D) {                   /* special charge -> strike 0x3C */
        if (actor_anim_done(p)) {
            c8 clip[16] = {0};
            actor_bake_root_yaw(p);
            if (actor_prepare_anim(p, 0x3C, clip)) {
                actor_play_anim(p, can_get_anim(clip), 0);
                E32(p, 0xD08) = 0x3C; E32(p, 0xD04) = 0x3C;
            }
        }
        return 1;
    }
    return player_getup(p);
}

static entity_t *live_enemy(entity_t *e) {         /* sub_450C70 / sub_450CDD */
    if (!e || !EDESC(e) || EDESC(e)[0x96] || PERSONA(e) == g_cinematic_mode) return NULL;
    return e;
}
static i32 dist_xz(entity_t *a, entity_t *b) {     /* get_entity_distance @ 0x450D4A */
    if (!a || !b) return -1;
    f64 dx = EROOT(a)->pos[0] - EROOT(b)->pos[0], dz = EROOT(a)->pos[2] - EROOT(b)->pos[2];
    return (i32)sqrt(dz * dz + dx * dx);
}

/* sub_451BF9 @ 0x451C1F..0x451FBB — combat prologue of the player
 * controller (the input-to-attack mapping after it is not in this draft). */
void combat_player_tick(void) {
    entity_t *p = g_player_entity;
    if (!p || !EDESC(p) || HP(p) <= 0) return;
    entity_t *c = live_enemy(g_companion_entity), *s = live_enemy(g_secondary_companion_entity);
    i32 dc = dist_xz(p, c), ds = dist_xz(p, s);
    /* TODO head tracking toward the nearer enemy (sub_450DE8) when in stance,
     * key 9 toggles special_mode_active, key 'C' (0x43) toggles +0xD60. */
    if (player_transitions(p)) return;
    combat_block(p, g_companion_entity);
    combat_block(p, g_secondary_companion_entity);
    finish_action(p);
    if (attempt_on_player(c, p, dc)) return;
    if (attempt_on_player(s, p, ds)) return;
    /* ... attack input (sub_451BF9 @ 0x451FBB onward) */
}

/* sub_4715D3 @ 0x4715D3 (death part) — enemy finished its death clip:
 * becomes a corpse painted into the backdrop. */
static i32 npc_reactions(entity_t *v) {
    i32 s = E32(v, 0xD04);
    if (s == 0x1A || s == 0x1B || s == 0x1C || HP(v) <= 0) {
        if (!actor_anim_done(v)) return 1;
        if (HP(v) > 0) {                           /* heavy hit survived */
            i32 pv = PERSONA(v);
            if (pv != 0x1E && pv != 0x1F && pv != 0x5A && (EDESC(v)[0x98] == 3 || pv == 0x19)) {
                E32(v, 0xCEC) = 0; E32(v, 0xCF0) = 1; E32(v, 0xD08) = 0; E32(v, 0xD04) = 0;
                E32(v, 0xCF8) = 0; E32(v, 0xCF4) = 0;
            } else {
                c8 clip[16] = {0};
                actor_bake_root_yaw(v);
                if (actor_prepare_anim(v, 6, clip)) {   /* get up */
                    E32(v, 0xD08) = 6; E32(v, 0xD04) = 6;
                    actor_play_anim(v, can_get_anim(clip), 0);
                }
            }
            return 1;
        }
        g_cinematic_mode = 0;
        E32(v, 0x70) = -1;                         /* stop drawing the live model */
        EDESC(v)[0x96] = 1;                        /* dead: actor_update/control skip it */
        /* TODO if (PERSONA(v) != 0x40) create_model_render_data(v) @ 0x402877:
         *      add a "sol" floor zone from the corpse's transformed bounds to
         *      render_buffer_pool (the sol%02d list).
         * TODO render_entity_with_ui(v, 1) @ 0x4024AF: render the corpse once
         *      into render_buffer1 / render_buffer2 (baked into the backdrop). */
        lieu_set_persona_vie(PERSONA(v), 2);       /* dword_505DD0[persona] = 2 */
        return 1;
    }
    if (s == 0x0F) return 1;
    if (HP(g_player_entity) <= 0) {                /* player dead: enemy taunts, frozen */
        c8 clip[16] = {0};
        E32(v, 0xCF0) = 0; E32(v, 0xCEC) = 0; E32(v, 0xCF8) = 0; E32(v, 0xCF4) = 0;
        E32(v, 0xD04) = 0x0F; E32(v, 0xD08) = 0x0F;
        actor_bake_root_yaw(v);
        if (actor_prepare_anim(v, 0x0F, clip)) actor_play_anim(v, can_get_anim(clip), 0);
        if (ANIM(v)) CAN_FLAGS(ANIM(v)) |= 0x200;
        return 1;
    }
    /* 0x4718DD..: 0x18 -> 6 -> 0 like player_getup (TODO, same shape). */
    return 0;
}

/* sub_4701E0 @ 0x4701E0, combat part only. The AI that chooses moves
 * (D44 block / D48 attack rolls from desc+0x9B..0x9E scaled by HP every
 * 250 ms, sub_46FB40 / sub_4711F9 / sub_470E54 / sub_470BB6) is not here. */
void combat_npc_tick(entity_t *c) {
    if (!c || !EDESC(c) || EDESC(c)[0x96]) return;
    if (attempt_on_npc(c)) return;                 /* sub_470A8F */
    if (npc_reactions(c)) return;                  /* sub_4715D3 */
    /* TODO AI */
}

/* =========================================================================
 * Shots — calculate_attack_trajectory @ 0x4020ED + is_point_in_view_cone
 * @ 0x401F75. Called from message 0x11 (enemy fires at the player) and 0x15
 * (player fires at the two enemy slots).
 * ========================================================================= */
static i32 in_cone(const i32 *from, const i32 *to, const i32 *dir, f64 range) {
    f64 dx = to[0] - from[0], dz = to[2] - from[2];
    f64 dot = ((f64)dir[2] * dz + (f64)dir[0] * dx) / 268435456.0;
    f64 off = (state && state->current_level_id == 0x18) ? 6.0 : 15.0;   /* world_state == 0x18 */
    if (dot <= 0.0) return 0;
    if (range + off < dot * 1.706666666666667) return 0;
    f64 nd = sqrt(dz * dz + dx * dx) / 600.0;
    f64 ratio = 37.8 / nd + 0.3;
    f64 cross = fabs((f64)dir[2] * dx - (f64)dir[0] * dz) / 268435456.0;
    return cross / dot < ratio;
}
/* get_distance_between_points @ 0x410490: length of the shot ray from the
 * gravit along the body heading until the first wall: an edge of the
 * walkable boundary (world_controller) or of a solid "col" obstacle
 * (dword_4F5FB0, flag 1 set and 2 clear). 0 when no wall is ahead. */
static void ray_poly(const sol_limit_t *s, const i32 *o, f64 sv, f64 cv, f64 *best, i32 *found) {
    const u8 *b = (const u8 *)s;
    i32 n = *(const i32 *)(b + 0x1D8);
    for (i32 i = 0; i < n; i++) {
        const i32 *p = (const i32 *)(b + 0x14 + i * 12);
        const i32 *q = (const i32 *)(b + 0x14 + (i == n - 1 ? 0 : i + 1) * 12);
        f64 c1 = cv * (f64)(p[0] - o[0]) - sv * (f64)(p[2] - o[2]);
        f64 c2 = cv * (f64)(q[0] - o[0]) - sv * (f64)(q[2] - o[2]);
        if (!((c1 <= 0.0 && c2 >= 0.0) || (c1 >= 0.0 && c2 <= 0.0))) continue;
        f64 ex = (f64)(q[0] - p[0]), ez = (f64)(p[2] - q[2]);
        f64 t = ((f64)(p[2] - o[2]) * ex + (f64)(p[0] - o[0]) * ez) / (cv * ex + sv * ez);
        if (!(t > 0.0)) continue;                      /* 0x410707 */
        if (!*found || t < *best) { *best = t; *found = 1; }
    }
}
static i32 shot_ray(entity_t *a, const i32 *dir, f64 *len) {
    (void)dir;
    obc_node_t *g = EROOT(a);
    if (!g) return 0;
    i32 ang;
    if (ANIM(a) && (CAN_FLAGS(ANIM(a)) & 1) && g->child)
        ang = (g->rot[2] - (g->child->rot[0] - CAN_ROOT_YAW(ANIM(a)))) & 0xFFFF;
    else
        ang = g->rot[2] & 0xFFFF;
    ang += 0x4000;
    f64 sv = cosine_table[(((u32)ang + 8u) >> 4) & 0xFFF];
    f64 cv = cosine_table[(((u32)ang - 0x3FF8u) >> 4) & 0xFFF];
    f64 best = 0.0;
    i32 found = 0;
    if (g_world.world_controller) ray_poly(g_world.world_controller, g->pos, sv, cv, &best, &found);
    for (i32 i = 0; i < g_world.num_collisions; i++) {
        const sol_limit_t *c = g_world.collision_zones[i];
        if (!c) continue;
        u8 f = *(const u8 *)c;
        if ((f & 1) && !(f & 2)) ray_poly(c, g->pos, sv, cv, &best, &found);
    }
    if (!found) return 0;
    *len = best;
    return 1;
}
/* sub_475990 @ 0x475990: ammo "OB%02d" (byte_4DB863[weapon]) qty > 0 ->
 * consume one; weapon 55 also needs OB08 (inventory.c). */
static i32 shot_ammo(entity_t *a) { return inv_take_ammo(ESTR(a, 0xD2)); }

static i32 shoot(entity_t *a, entity_t *t1, entity_t *t2) {
    if (a == g_player_entity && !shot_ammo(a)) return 0;   /* (key 0x12 bypasses the check) */
    E32(a, 0xD04) = 0x0E;
    obc_node_t *g = EROOT(a);
    i32 ang;
    if (ANIM(a) && (CAN_FLAGS(ANIM(a)) & 1) && g->child)
        ang = (g->rot[2] - (g->child->rot[0] - CAN_ROOT_YAW(ANIM(a)))) & 0xFFFF;
    else
        ang = g->rot[2] & 0xFFFF;
    ang += 0x4000;
    /* asm "sine_table" holds cos (obc.c trig_cos/trig_sin). */
    f64 sv = cosine_table[(((u32)ang + 8u) >> 4) & 0xFFF];
    f64 cv = cosine_table[(((u32)ang - 0x3FF8u) >> 4) & 0xFFF];
    i32 dir[3] = { (i32)sv, 0, (i32)cv };
    f64 len;
    if (!shot_ray(a, dir, &len)) return 0;
    i32 tx = (i32)(sv * len + g->pos[0]), ty = g->pos[1], tz = (i32)(cv * len + g->pos[2]);
    entity_t *t[2] = { t1, t2 };
    for (i32 i = 0; i < 2; i++) {
        entity_t *v = t[i];
        if (!v || E32(v, 0xD1C)) continue;
        if (!in_cone(g->pos, EROOT(v)->pos, dir, len)) continue;
        if (v == g_player_entity) {
            E32(v, 0xD1C) = 1;                     /* damage applied by attempt_on_player */
            g_blood = 9;
            g_blood_pos[0] = EROOT(v)->raw_88[2];  /* gravit +0x90/+0x94/+0x98 */
            g_blood_pos[1] = EROOT(v)->raw_88[3];
            g_blood_pos[2] = EROOT(v)->raw_88[4];
        }
        E32(v, 0xC74) = 1;                         /* enemies: scripts read it (DIVERS 18) */
        return 1;
    }
    E32(a, 0xD20) = tx; E32(a, 0xD24) = ty; E32(a, 0xD28) = tz;   /* miss point */
    return 0;
}

/* Muzzle flash (sub_41AED1 @ 0x41BB2D) and miss spark (@ 0x41BBD6). */
static void muzzle_flash(entity_t *e) {
    obc_node_t *w = (obc_node_t *)EPTR(e, 0x178);
    if (w && w->child) {
        E32(e, 0xE5C) = 1; E32(e, 0xE58) = now_ms();
        /* TODO w->child->child = flash mesh (dword_5099C0). */
        effect_shell_casing(&w->raw_88[2], &w->child->raw_88[2],
                            EROOT(e) ? EROOT(e)->rot[2] : 0, EDESC(e)[0x97]);   /* @ 0x41BB2D */
    }
}
static void miss_spark(entity_t *e) {
    if (E32(e, 0xD20) || E32(e, 0xD24) || E32(e, 0xD28)) {
        i32 p[3] = { E32(e, 0xD20), E32(e, 0xD24) + 0x8CA0, E32(e, 0xD28) };
        fx_puff_spark(p);                          /* sub_42F476 @ 0x41BC2E / 0x41BE4E */
        E32(e, 0xD20) = 0; E32(e, 0xD24) = 0; E32(e, 0xD28) = 0;
    }
}

/* =========================================================================
 * Enemy flame projectile ("lance-feu", weapon class flag 0x20).
 *   process_counter_attack(.., 1) @ 0x410F76   launch from the weapon tip
 *   process_entity_direction_and_state @ 0x411333   flight, hit, impact
 *   get_distance_between_points @ 0x410490   range = distance to the wall
 * The original keeps the shot in a global pseudo-entity (current_entity) with
 * the weapon's OB54 tree in +0x50; +0xA0 = in flight, +0x70 = 2 once spent.
 * Here it is the file-static g_fl, drawn by the scene like an actor.
 * ========================================================================= */
static struct {
    i32           active;          /* current_entity +0xA0 */
    obc_node_t   *root;            /* +0x50 */
    const ome_matr_t *matr;        /* +0x88 */
    i32           start[3];        /* global_dword_543C20.. */
    i32           trail[3];        /* dword_543BF0: where the last puff went */
    f64           speed, grav;     /* global_dbl_543C30 / 543C38 */
    f64           hcos, vsin;      /* dbl_543B28 / dbl_543C10 */
    f64           range;           /* dbl_543C00 */
    i32           mode;            /* global_dword_543C08: 3072 flame */
    i32           heading;         /* global_dword_543BEC */
    i32           t0;              /* level_timer */
} g_fl;

void combat_projectile_reset(void) { memset(&g_fl, 0, sizeof(g_fl)); }

/* get_distance_between_points @ 0x410490: from `from`, along the player's
 * facing (+0x4000), the nearest forward crossing of any wall edge: the world
 * boundary and every live "col" polygon. Result in units of 16384 world. */
static i32 ray_to_wall(const entity_t *p, const i32 *from, f64 *out) {
    const obc_node_t *r = EROOT(p);
    i32 yaw;
    if (ANIM(p) && (CAN_FLAGS(ANIM(p)) & 1) && r->child)
        yaw = (r->rot[2] - (r->child->rot[0] - CAN_ROOT_YAW(ANIM(p)))) & 0xFFFF;
    else
        yaw = r->rot[2] & 0xFFFF;
    i32 a = yaw + 0x4000;
    f64 c = cosine_table[(((u32)a + 8u) >> 4) & 0xFFF];
    f64 s = cosine_table[(((u32)a - 16376u) >> 4) & 0xFFF];
    f64 best = 0.0;
    i32 have = 0;
    const sol_limit_t *polys[1 + WORLD_MAX_COLLISIONS];
    i32 np = 0;
    if (g_world.world_controller) polys[np++] = g_world.world_controller;
    for (i32 i = 0; i < g_world.num_collisions && i < WORLD_MAX_COLLISIONS; i++) {
        const sol_limit_t *z = g_world.collision_zones[i];
        if (z && (z->flags & 1) && !(z->flags & 2)) polys[np++] = z;
    }
    for (i32 k = 0; k < np; k++) {
        const sol_limit_t *z = polys[k];
        for (i32 i = 0; i < z->vertex_count; i++) {
            const i32 *v0 = z->vertices[i];
            const i32 *v1 = z->vertices[i == z->vertex_count - 1 ? 0 : i + 1];
            f64 e0 = s * (f64)(v0[0] - from[0]) - c * (f64)(v0[2] - from[2]);
            f64 e1 = s * (f64)(v1[0] - from[0]) - c * (f64)(v1[2] - from[2]);
            if (!((e0 <= 0.0 && e1 >= 0.0) || (e0 >= 0.0 && e1 <= 0.0))) continue;
            f64 dx = (f64)(v1[0] - v0[0]), dz = (f64)(v0[2] - v1[2]);
            f64 t = ((f64)(v0[2] - from[2]) * dx + (f64)(v0[0] - from[0]) * dz) / (s * dx + c * dz);
            if (t <= 0.0) continue;
            if (!have || t < best) { best = t; have = 1; }
        }
    }
    if (!have) return 0;
    *out = best;
    return 1;
}

/* process_counter_attack @ 0x410F76, a3 = 1. */
static void flame_launch(entity_t *e) {
    game_object_t *w = (game_object_t *)EPTR(e, 0xD64);
    entity_t *pl = g_player_entity;
    if (!w || !w->proj || !pl || !EROOT(e) || !EROOT(pl)) return;
    obc_node_t *tip = EBONE(e, 31);                    /* run_entity_callback(e, 31, ..) */
    if (!tip) return;
    while (tip->child) tip = tip->child;
    obc_node_t *t = w->proj;
    const obc_node_t *sr = EROOT(e);
    g_fl.speed = 12.0 * 3.662109375;
    g_fl.grav  = 5.0;
    g_fl.mode  = 3072;
    g_fl.root  = t;
    g_fl.matr  = &w->proj_matr;
    t->rot[0] = sr->rot[0]; t->rot[2] = sr->rot[2]; t->rot[1] = sr->rot[1];
    fmat_t tm;                                          /* sub_492310: the tip's world position */
    node_matrix(EROOT(e), tip, &tm);
    t->pos[0] = tm.t[0]; t->pos[1] = tm.t[1]; t->pos[2] = tm.t[2];
    t->raw_88[2] = t->pos[0]; t->raw_88[3] = t->pos[1]; t->raw_88[4] = t->pos[2];
    g_fl.hcos = (f64)cosine_table[(((u32)g_fl.mode + 8u) >> 4) & 0xFFF] * 0.00006103515625;
    g_fl.vsin = (f64)cosine_table[(((u32)g_fl.mode - 16376u) >> 4) & 0xFFF] * 0.00006103515625;
    memcpy(g_fl.start, t->pos, sizeof(g_fl.start));
    memcpy(g_fl.trail, t->pos, sizeof(g_fl.trail));
    if (ANIM(e) && (CAN_FLAGS(ANIM(e)) & 1) && sr->child)
        g_fl.heading = (sr->rot[2] - (sr->child->rot[0] - CAN_ROOT_YAW(ANIM(e)))) & 0xFFFF;
    else
        g_fl.heading = (sr->rot[2] & 0xFFFF) + 0x4000;
    if (!ray_to_wall(pl, g_fl.start, &g_fl.range)) g_fl.range = 0.0;
    g_fl.t0 = now_ms();
    g_fl.active = 1;
}

static f64 dist2_xz(const obc_node_t *a, const obc_node_t *b) {
    f64 dx = (f64)(a->raw_88[2] - b->raw_88[2]), dz = (f64)(a->raw_88[4] - b->raw_88[4]);
    return dx * dx + dz * dz;
}

/* The flame is spent at world (x, y, z): burst, sound, damage within 120000 units. */
static void flame_impact(const i32 *at) {
    entity_t *pl = g_player_entity;
    fx_flame_burst(at[0], at[1], at[2]);                 /* process_entity_random_transformations */
    if (pl) sfx_entity_named(pl, "impactf");
    obc_node_t pt;
    memset(&pt, 0, sizeof(pt));                          /* only raw_88 is read by dist2_xz */
    pt.raw_88[2] = at[0]; pt.raw_88[4] = at[2];
    if (pl && EROOT(pl) && dist2_xz(EROOT(pl), &pt) < 1.44e10) {
        HP(pl) -= 100;
        if (HP(pl) <= 0) {
            play_death(pl);                              /* D04 = 0x1A */
            lieu_set_death_state(4);                     /* game_death_state = 4 */
        } else {
            E32(pl, 0xD04) = 0x18;
        }
        scene_hud_refresh();                             /* update_player_interface(.., 1) */
    }
    entity_t *c[2] = { g_companion_entity, g_secondary_companion_entity };
    for (i32 i = 0; i < 2; i++)
        if (c[i] && EROOT(c[i]) && !EDESC(c[i])[0x96] && dist2_xz(EROOT(c[i]), &pt) < 1.44e10)
            E32(c[i], 0xC74) = 1;                        /* hit flag the scripts read */
}

// 00411333
void combat_projectile_tick(void) {
    if (!g_fl.active || !g_fl.root) return;
    entity_t *pl = g_player_entity;
    obc_node_t *r = g_fl.root;
    f64 t = (f64)(now_ms() - g_fl.t0) * 0.001;
    f64 s = t * g_fl.speed;
    f64 vc = (f64)cosine_table[(((u32)g_fl.heading + 8u) >> 4) & 0xFFF];
    f64 vs = (f64)cosine_table[(((u32)g_fl.heading - 16376u) >> 4) & 0xFFF];
    i32 dir[3] = { (i32)vc, 0, (i32)vs };
    f64 a = g_fl.hcos * s;                               /* angle_threshold: ground travelled */
    f64 dy = g_fl.vsin * s - g_fl.grav * 4.905 * t * t;
    i32 flying = !((f64)r->pos[1] - 12000.0 <= (f64)scene_fx_floor(r->pos[0], r->pos[2])) &&
                 a < g_fl.range;
    i32 hit = !flying;
    if (flying) {
        r->pos[0] = (i32)((f64)g_fl.start[0] + a * vc);
        r->pos[1] = (i32)(dy * 16384.0 + (f64)g_fl.start[1]);
        r->pos[2] = (i32)((f64)g_fl.start[2] + vs * a);
        r->raw_88[2] = r->pos[0]; r->raw_88[3] = r->pos[1]; r->raw_88[4] = r->pos[2];
        if (g_fl.mode != -16384) {
            r->rot[0] += 0x2000;
            r->rot[2] += 0x2000;
            fx_puff_flame(g_fl.trail, 180);              /* initialize_entity_positions x2 */
            fx_puff_flame(g_fl.trail, 180);
            memcpy(g_fl.trail, r->pos, sizeof(g_fl.trail));
            entity_t *c[2] = { g_companion_entity, g_secondary_companion_entity };
            for (i32 i = 0; i < 2 && !hit; i++) {
                if (!c[i] || !EROOT(c[i]) || EDESC(c[i])[0x96]) continue;
                if (pl && in_cone(EROOT(pl)->pos, EROOT(c[i])->pos, dir, a) &&
                    dist2_xz(EROOT(c[i]), r) < 900000000.0)
                    hit = 1;
            }
        }
    }
    if (!hit) return;
    if (g_fl.mode != -16384) {
        i32 at[3] = { r->raw_88[2], r->raw_88[3], r->raw_88[4] };
        flame_impact(at);
    }
    g_fl.active = 0;                                     /* current_entity +0xA0 = 0 */
}

i32 combat_projectile_draw(effect_draw_t *out) {
    if (!g_fl.active || !g_fl.root || !out) return 0;
    out->root = g_fl.root;
    out->matr = g_fl.matr;
    out->ground_y = scene_fx_floor(g_fl.root->pos[0], g_fl.root->pos[2]);
    return 1;
}

/* =========================================================================
 * Animation messages — sub_41AED1 types 0x11 / 0x15 / 0x19 / 0x24.
 * ========================================================================= */
void combat_anim_message(entity_t *e, i32 type, i32 p) {
    if (type == 0x11) {                            /* 0x41BA96: enemy shot / thrown */
        if (e != g_player_entity) shoot(e, g_player_entity, NULL);
        u8 wf = weapon_flags(e);
        if (wf & (0x10 | 0x40 | 0x80)) {
            if (!(wf & 0x40)) muzzle_flash(e);     /* no flash for the cracker */
            miss_spark(e);
        }
        if (wf & 0x20)
            flame_launch(e);       /* process_entity_action @ 0x411C06 -> process_counter_attack(.., 1) */
    }
    if (type == 0x15 && e == g_player_entity) {    /* 0x41BC8F: player fires */
        muzzle_flash(e);
        entity_t *c = g_companion_entity, *s = g_secondary_companion_entity;
        if (c && (EDESC(c)[0x96] || PERSONA(c) == g_cinematic_mode)) c = NULL;
        if (s && (EDESC(s)[0x96] || PERSONA(s) == g_cinematic_mode)) s = NULL;
        shoot(e, c, s);
        miss_spark(e);
    }
    if (type == 0x19 && PERSONA(e) != 0x40) {      /* 0x41C717: sparks at bone p */
        obc_node_t *b = (p >= 0 && p < ENTITY_BONE_SLOTS) ? EBONE(e, p) : NULL;
        if (b) {
            g_spark_pos[0] = b->raw_88[2]; g_spark_pos[1] = b->raw_88[3]; g_spark_pos[2] = b->raw_88[4];
            parry_sparks();
        }
    }
    if (type == 0x24 && e == g_player_entity)      /* 0x41CB5F: weapon wear 1 */
        weapon_wear(1);
}

/* =========================================================================
 * Enemy HP bar — sub_45C700 @ 0x45C700 (+ update_companion_status @ 0x45C9C5
 * without the timer). Slot 0 = companion_entity, 1 = secondary. Copies
 * GINETTE.GIF (dword_4DDE1C, 20x75 fallback) into dword_4DE648[slot]
 * (game_init.c g_health_bmap_a/_b), blanks the empty part with colour 0x7F:
 * fill (7, 9)-(12, 68 - v), v = hp*59 >> 10 clamped 0..59, 1 if alive.
 * Overlay record type 13 at (10 + 30*slot, 10), shown 2000 ms after the last
 * hit (entity+0xCE4), drawn by update_game_hud case 13 @ 0x467C23.
 * ========================================================================= */
extern bmap_t *game_init_get_ginette(void);        /* TODO export */
extern bmap_t *game_init_get_health_bmap(i32 slot);/* TODO export */

void combat_enemy_bar(entity_t *e) {
    if (g_debug_flags & 2) return;
    i32 slot = (e == g_companion_entity) ? 0 : (e == g_secondary_companion_entity) ? 1 : -1;
    if (slot < 0) return;
    E32(e, 0xCE4) = now_ms();
    i32 hp = HP(e);
    i32 v = (hp * 59) / 1024;                      /* cdq/and 3FF/sar 10 = truncation */
    if (v < 0) v = 0;
    if (v > 59) v = 59;
    if (hp > 0 && v < 1) v = 1;
    bmap_t *src = game_init_get_ginette(), *dst = game_init_get_health_bmap(slot);
    if (!src || !dst) return;
    ls_bit_move(dst, src, 0, 0, src->width, src->height, 0, 0, 0);
    ls_fill_rect(dst, 7, 9, 0x0C, 59 - v + 9, 0x7F);
}
/* Call from the HUD pass (update_game_hud @ 0x4674F9). */
void combat_enemy_bar_draw(bmap_t *game_context) {
    entity_t *e[2] = { g_companion_entity, g_secondary_companion_entity };
    for (i32 i = 0; i < 2; i++) {
        if (!e[i] || !E32(e[i], 0xCE4) || now_ms() - E32(e[i], 0xCE4) > 2000) continue;
        bmap_t *b = game_init_get_health_bmap(i);
        if (b) ls_bit_move(game_context, b, 0, 0, b->width, b->height, 10 + 30 * i, 10, 0);
    }
}

/* =========================================================================
 * Regeneration — update_game_progression @ 0x405003 (HP part).
 * Persona 2: +1 HP / 2 s; persona 3: +1 HP / 1 s (and boss 0x28 refilled to
 * 0x3FF; in world_state 0x2B the companion regains 50 / s). Score_counter
 * (the darkness meter, 0..0x3FF) rises 1 per 20 s, or per 40 s at full HP;
 * event_callback(6, 0..4) at thresholds 0x1C2 / 0x3B6 / 0x1F4 / 0x3FF.
 * ========================================================================= */
void combat_regen_tick(void) {
    static i32 last_hp = 0;
    entity_t *p = g_player_entity;
    if (!p || !EDESC(p) || HP(p) <= 0) return;
    i32 pv = PERSONA(p), t = now_ms();
    if (pv == 2 && t - last_hp >= 2000 && HP(p) < 0x3FF) { last_hp = t; HP(p)++; scene_hud_refresh(); }
    if (pv == 3 && t - last_hp >= 1000) { last_hp = t; if (HP(p) < 0x3FF) { HP(p)++; scene_hud_refresh(); } }
    /* TODO score_counter timers + event_callback (update_game_progression @ 0x4050CF..0x40563C). */
}

/* =========================================================================
 * Hooks used by combat_player.c (sub_451BF9 path)
 * ========================================================================= */
extern i32 actor_head_track(entity_t *self, entity_t *tgt);

i32  combat_hook_score_get(void)           { return scene_score_get(); }
void combat_hook_score_add(i32 v)          { scene_score_set(scene_score_get() + v); scene_hud_refresh(); }
void combat_hook_hud_refresh(void)         { scene_hud_refresh(); }
i32  combat_hook_block(entity_t *p, entity_t *en) { combat_block(p, en); return 0; }   /* process_combat_block */
i32  combat_hook_entity_damage(entity_t *atk, entity_t *vic) { (void)vic; return calc_damage(atk); }
void combat_hook_blood(entity_t *vic) { if (g_blood && !(g_debug_flags & 2)) blood_burst(vic); }
i32  combat_hook_check_revival(entity_t *vic) { return cramped(vic); }               /* check_entity_revival */
i32  combat_hook_contact(entity_t *atk, entity_t *vic) { return strike_hits(atk, vic); } /* sub_429695 */
i32  combat_hook_has_ammo(const c8 *w)     { return inv_take_ammo(w); }               /* sub_475990 */
i32  combat_hook_ray_dist(entity_t *a, f64 *out) { return shot_ray(a, NULL, out); }  /* get_distance_between_points */
i32  combat_hook_in_cone(const i32 *t, const i32 *f, const i32 *d, f64 r) { return in_cone(f, t, d, r); }
i32  combat_hook_head_track(entity_t *s, entity_t *t) { return actor_head_track(s, t); }
void combat_hook_objects_flagbits(game_object_t *o) { (void)o; }                      /* sub_4728DF */

/* update_game_hud case 13 (0x467C23): the enemy bars, straight onto the
 * port's 32-bit frame (palette of GINETTE). */
void combat_enemy_bar_fb(u32 *fb, i32 fw, i32 fh) {
    bmap_t *gin = game_init_get_ginette();
    if (!fb || !gin || !gin->palette) return;
    entity_t *e[2] = { g_companion_entity, g_secondary_companion_entity };
    for (i32 i = 0; i < 2; i++) {
        if (!e[i] || !E32(e[i], 0xCE4) || now_ms() - E32(e[i], 0xCE4) > 2000) continue;
        bmap_t *b = game_init_get_health_bmap(i);
        if (b && b->pixels) blit_bmap_to_framebuffer(fb, fw, fh, b, gin->palette, 10 + 30 * i, 10);
    }
}
