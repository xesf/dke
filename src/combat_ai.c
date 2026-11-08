/*
 * combat_ai_draft.c -- DRAFT port of the NPC combat AI (dkev.exe sub_4701E0
 * and callees). Written against src/entity.h / actor.h / can.h accessors.
 * Not compiled; meant to become src/combat_ai.c (or the tail of actor_move.c).
 *
 *   sub_4701E0 @ 0x4701E0  combat_ai_tick           per-frame brain
 *   sub_46FB40 @ 0x46FB40  ai_measure               distance + two bearings
 *   sub_470A8F @ 0x470A8F  ai_check_player_hit      player's blade on me?
 *   sub_470507 @ 0x470507  ai_take_hit              damage + reaction state
 *   sub_4715D3 @ 0x4715D3  ai_housekeep             death / get-up / reset
 *   sub_4711F9 @ 0x4711F9  ai_counter               strike into player's windup
 *   sub_46FDC9 @ 0x46FDC9  ai_parry                 parry matching attack
 *   sub_470E54 @ 0x470E54  ai_engage                attack in range / approach
 *   sub_470BB6 @ 0x470BB6  ai_strike_facing         close/lunge/ranged strike
 *   sub_429695 @ 0x429695  weapon_hits              clefs sphere test
 *   sub_4295E0 @ 0x4295E0  in_hit_window
 *   calculate_entity_damage @ 0x401040              attack_damage
 *   check_entity_special_abilities @ 0x401130       ai_ranged
 *   is_creature_special_level_type  @ 0x401200      st_attacking
 *   is_master_eligible_creature     @ 0x401270      st_hurt
 *   is_creature_in_special_level_range @ 0x4011A0   st_parrying
 *   check_entity_revival @ 0x402420                 small_zone
 *   sub_46642E head @ 0x46642E..0x4665CC            combat_ai_cinematic
 *
 * Special personas with their own brains (NOT drafted, stubs below):
 *   id 0x5C (P92, flying imp)  sub_45FD40  (clips p92imp01/02, p92mar01, p92ata01)
 *   id 0x28 (P40, boss)        sub_460444  (+0xD70/+0xD74 phase, "p40gar02"): phase 2 = boss_guard_tick
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "libsys/common.h"
#include "state.h"
#include "actor.h"
#include "can.h"
#include "game_init.h"
#include "lieu_cb.h"
#include "world.h"                     /* g_world.world_controller */
#include "fx.h"
#include "libsys/graphics/shading.h"   /* cosine_table */

#define ROOT(e)  EROOT(e)
#define ANIM(e)  ((can_rec_t *)EPTR(e, 0xD50))
#define DHP(e)   (*(i32 *)(EDESC(e) + 0x84))        /* persona HP, 0..1023 scale */

/* CAN header fields used by combat (hdr offsets). CAN_ROOT_YAW (0xFC) doubles
 * as the striking bone slot when flag 0x40 (attack clip) is set. */
#define CAN_HIT_WINDOW(r) (*(i32 *)&(r)->hdr[0x100])  /* lo16 start, hi16 end (anim ticks) */
#define CAN_DAMAGE(r)     ((r)->hdr[0x104])            /* u8 */
#define CAN_BONE(r)       CAN_ROOT_YAW(r)

/* ---- globals the original keeps elsewhere ------------------------------ */
extern i32 g_cinematic_mode;          /* cinematic_mode (actor_move.c) */
/* NEW / to export:
 *   g_combat_flags  = "debug_flags" @ ?: written by COMBAT (sub_46FF9A: =mode)
 *                     and DIVERS op 21 (=b). bit0 -> D14, bit1 -> enemy can't
 *                     be hurt (no HP loss, no C68 growth, no impact fx).
 *   g_cam_lock      = dword_4FF0A0, today `static` in lieu_cb.c:414.
 *   g_ai_freeze     = dword_4B21A0, debug key 'm' (also XORs both D14).
 *   g_hero_autofight= dword_4B219C, set by COMBAT(hero id<4); debug key 'k'. */
extern i32 g_combat_flags;
extern i32 g_cam_lock;
static i32 g_ai_freeze = 0;
i32        g_hero_autofight = 0;

static i32 g_ranged;                  /* dword_4DD778 (set every tick) */
static i32 g_hit_bone;                /* "sound_enabled" : defender bone slot hit */
static i32 g_hit_pt[3];               /* sound_param1 (x), param3 (y), param2 (z) */
i32        g_enemy_hits;              /* dword_506520 (hits landed this fight) */

static const i32 k_body_bones[15] = { 14, 16, 13, 15, 9, 1, 3, 2, 4, 6, 5, 7, 17, 11, 12 }; /* dword_4B44B0 */

static i32 now_ms(void) { return state ? (i32)state->current_game_time : 0; }

/* ls_random(n) & 0xFFFF: 0..n (n itself only when rand()==0x7FFF). Same as
 * lo_random() in lieu_cb.c -- share one helper. */
static i32 ai_random(i32 n) {
    return (i32)(((u32)n * ((u32)rand() & 0x7FFFu)) / 32767u) & 0xFFFF;
}

/* ---- state classes (D04) ----------------------------------------------- */
/* 7..12 melee attacks, 14 ranged, 29 / 60 hero specials */
static i32 st_attacking(const entity_t *e) {
    if (!e) return 0;
    i32 s = E32(e, 0xD04);
    return (s >= 7 && s <= 12) || s == 29 || s == 60 || s == 14;
}
/* 22 hit, 24 knocked down, 26 dying, 27/28 heavy hit (NOT 23, 25). NULL -> 1 */
static i32 st_hurt(const entity_t *e) {
    if (!e) return 1;
    i32 s = E32(e, 0xD04);
    return s >= 22 && s <= 28 && s != 23 && s != 25;
}
/* 16..21 parries (one per attack 7..12), 30..53 */
static i32 st_parrying(const entity_t *e) {
    if (!e) return 0;
    i32 s = E32(e, 0xD04);
    return (s >= 16 && s <= 21) || (s >= 30 && s <= 53);
}

/* check_entity_special_abilities: held object (desc+0x97) has a gun class:
 * 0x10 foudre | 0x40 cracker | 0x80 pistolet | 0x20 lance-feu. */
static i32 ai_ranged(const entity_t *e) {
    if (!e || !EDESC(e)) return 0;
    const creature_entry_t *c = game_init_get_creature(EDESC(e)[0x97]);
    u32 f = c ? c->flags : 0;
    return (f & 0xD0) || (f & 0x20);
}

/* sub_4295E0: attack clip inside its active window, no clash/projectile flag. */
static i32 in_hit_window(const entity_t *e) {
    if (E32(e, 0xD18) || E32(e, 0xD1C)) return 0;
    const can_rec_t *r = ANIM(e);
    if (!r || !(CAN_FLAGS(r) & 0x40)) return 0;
    i32 lo = CAN_HIT_WINDOW(r) & 0xFFFF, hi = CAN_HIT_WINDOW(r) >> 16;
    return E32(e, 0xC40) >= lo && E32(e, 0xC40) <= hi;
}

/* Heading used for combat: gravit yaw, minus the skeleton's baked root yaw
 * while a root-yaw clip (flag 1) is playing (0x46FBA7..0x46FBEF). */
static i32 combat_heading(entity_t *e) {
    obc_node_t *g = ROOT(e);
    const can_rec_t *r = ANIM(e);
    if (r && (CAN_FLAGS(r) & 1) && g->child)
        return (g->rot[2] - (g->child->rot[0] - CAN_ROOT_YAW(r))) & 0xFFFF;
    return g->rot[2] & 0xFFFF;
}

static i32 wrap16(i32 a, i32 b) {      /* (a - b) wrapped to -0x8000..0x8000 */
    i32 r = a - b;
    if (b <= a) { if (r > 0x8000) r -= 0x10000; }
    else        { if (r < -0x8000) r += 0x10000; }
    return r;
}

/* sub_46FB40(e, &dist, &bear_self, &bear_opp). The opponent is ALWAYS the
 * global g_player_entity (callers swap it for NPC-vs-NPC fights).
 *   bear_self: turn needed for e to face the opponent
 *   bear_opp : how far the opponent's facing is off pointing at e
 * Returns 0 when |bear_self| < 0x38E (~5 deg), else its sign (unused). */
static i32 ai_measure(entity_t *e, f64 *dist, i32 *bear_self, i32 *bear_opp) {
    entity_t *p = g_player_entity;
    f64 dx = (f64)(ROOT(p)->pos[0] - ROOT(e)->pos[0]);
    f64 dz = (f64)(ROOT(p)->pos[2] - ROOT(e)->pos[2]);
    *dist = sqrt(dz * dz + dx * dx);
    i32 dir = (-(i32)(atan2(dx, dz) * 65536.0 / 6.283185307179586)) & 0xFFFF;
    *bear_self = wrap16(dir, combat_heading(e));
    *bear_opp  = wrap16((dir + 0x8000) & 0xFFFF, combat_heading(p));
    if (abs(*bear_self) < 0x38E) return 0;
    return *bear_self > 0 ? 1 : -1;
}

/* Common "enter combat state st now" block (0x470F0D.. and copies):
 * stop locomotion, D04 = D08 = st, play its clip, and cache the clip's
 * wind-up (hit window start) in desc+4+st*4 (read back by ai_counter). */
static void ai_play_state(entity_t *e, i32 st) {
    c8 clip[16] = {0};
    E32(e, 0xCF0) = 0; E32(e, 0xCEC) = 0;
    E32(e, 0xCF8) = 0; E32(e, 0xCF4) = 0;
    E32(e, 0xD04) = st; E32(e, 0xD08) = st;
    actor_bake_root_yaw(e);                         /* process_entity_state_change */
    if (actor_prepare_anim(e, st, clip))
        actor_play_anim(e, can_get_anim(clip), 0);
    if (ANIM(e))
        *(i32 *)(EDESC(e) + 4 + st * 4) = CAN_HIT_WINDOW(ANIM(e)) & 0xFFFF;
}

/* Like ai_play_state but only D04/D08 + clip (no locomotion reset, no cache):
 * the get-up (6), victory (15) and death (26) plays. D04 is only written when
 * the clip exists (0x471731), except for 15/26 which set it first. */
static i32 ai_play_plain(entity_t *e, i32 st) {
    c8 clip[16] = {0};
    actor_bake_root_yaw(e);
    if (!actor_prepare_anim(e, st, clip)) return 0;
    actor_play_anim(e, can_get_anim(clip), 0);
    return 1;
}

/* calculate_entity_damage(attacker) @ 0x401040: clip damage byte * base * 8 / 100.
 * base = 100; player with a 'Q' weapon: 89 (is_character_type_a) / 64 (type_b).
 * The 'O' weapon counter (consume_action_resource + execute_counter_attack)
 * is hero-only and left out here. */
static i32 attack_damage(entity_t *a) {
    i32 base = 100;
    if (a == g_player_entity && !g_cinematic_mode && ESTR(a, 0xD2)[0] == 'Q') {
        /* TODO: is_character_type_a(desc[0x97]) -> 0x59, is_character_type_b -> 0x40 */
    }
    return ANIM(a) ? (i32)CAN_DAMAGE(ANIM(a)) * base * 8 / 100 : 0;
}

/* check_entity_revival @ 0x402420: standing on a floor zone smaller than
 * 3.6e9 units^2 -> no knock-down (would fall off), use the light hit. */
static i32 small_zone(entity_t *e) {
    const u8 *z = (const u8 *)EPTR(e, 0xA8);
    if (!z) return 0;
    f64 w = (f64)(*(const i32 *)(z + 0x1E8) - *(const i32 *)(z + 0x1DC));
    f64 h = (f64)(*(const i32 *)(z + 0x1F0) - *(const i32 *)(z + 0x1E4));
    return h * w <= 3.6e9;
}

/* sub_429695(a, d): a's striking bone (and up to 3 body-bone ancestors for
 * limb strikes 0x1F/0x0E/0x10/0x07/0x05; 0x1F adds its child, 0x22 its parent)
 * against d's 15 body bones, clefs sphere vs sphere (AABB pre-test).
 * NEEDS clefs[i].reserved[0..2] = world-space centre (orig +0x10), filled by
 * the per-frame hierarchy transform -- check the port fills it. */
static i32 weapon_hits(entity_t *a, entity_t *d) {
    if (!a || !d || !ANIM(a) || !(CAN_FLAGS(ANIM(a)) & 0x40)) return 0;
    i32 slot = CAN_BONE(ANIM(a));
    obc_node_t *n = (slot >= 0 && slot < ENTITY_BONE_SLOTS) ? EBONE(a, slot) : NULL;
    if (!n) { print("combat: clip %p strikes with empty bone %d\n", (void *)ANIM(a), slot); return 0; }
    obc_node_t *list[4]; u32 cnt = 0;
    list[cnt++] = n;
    if (slot == 0x1F)      list[cnt++] = n->child;
    else if (slot == 0x22) list[cnt++] = n->parent;
    if (slot == 0x1F || slot == 0x0E || slot == 0x10 || slot == 7 || slot == 5) {
        obc_node_t *p = n;
        while (cnt < 4 && p->parent) {
            p = p->parent;
            for (i32 k = 0; k < 15; k++)
                if (EBONE(a, k_body_bones[k]) == p) { list[cnt++] = p; break; }
        }
    }
    for (u32 li = 0; li < cnt; li++) {
        obc_node_t *A = list[li];
        if (!A) break;                                  /* 0x4298F8: stops the scan */
        for (i32 k = 0; k < 15; k++) {
            obc_node_t *B = EBONE(d, k_body_bones[k]);
            if (!B) continue;
            for (u32 i = 0; i < (u32)A->has_clefs; i++) {
                const obc_clefs_t *ca = &A->clefs[i];
                f64 ra = (f64)(u32)ca->radius;
                const i32 *pa = ca->reserved;
                for (u32 j = 0; j < (u32)B->has_clefs; j++) {
                    const obc_clefs_t *cb = &B->clefs[j];
                    f64 rb = (f64)(u32)cb->radius;
                    const i32 *pb = cb->reserved;
                    if (!(pb[0] - rb <= pa[0] + ra) || (pb[0] + rb < pa[0] - ra)) continue;
                    if (!(pb[1] - rb <= pa[1] + ra) || (pb[1] + rb < pa[1] - ra)) continue;
                    if ((pa[2] + ra < pb[2] - rb) || !(pa[2] - ra <= pb[2] + rb)) continue;
                    i32 dx = pa[0] - pb[0], dy = pa[1] - pb[1], dz = pa[2] - pb[2];
                    f64 rr = (rb + ra) * (rb + ra);
                    if ((f64)dz * dz + (f64)dx * dx + (f64)dy * dy < rr) {
                        g_hit_bone  = k_body_bones[k];
                        g_hit_pt[0] = (pb[0] + pa[0]) / 2;
                        g_hit_pt[1] = (pb[1] + pa[1]) / 2;
                        g_hit_pt[2] = (pb[2] + pa[2]) / 2;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* sub_45C700: enemy energy bar (HUD slot 0 = companion, 1 = secondary),
 * HP*59/1024 px, +0xCE4 = now. Not ported -> hook. */
static void hud_enemy_bar(entity_t *e) {
    if (g_combat_flags & 2) return;
    if (e == g_companion_entity || e == g_secondary_companion_entity) E32(e, 0xCE4) = now_ms();
    /* TODO HUD */
}

/* ========================================================================
 * sub_470507 — e was struck by g_player_entity.
 * ======================================================================== */
static void ai_take_hit(entity_t *e) {
    entity_t *p = g_player_entity;
    i32 id = E32(e, 0xB0);
    u8 *d = EDESC(e);
    if (st_hurt(e)) return;                                   /* already reeling */
    g_enemy_hits++;
    if (g_hit_bone && !(g_combat_flags & 2))      /* play_sound_at_position @ 0x470564: blood spray */
        fx_blood_start(g_hit_pt[0], g_hit_pt[1], g_hit_pt[2], ROOT(e)->rot[2] + 0x4000);
    /* play_entity_sound(e, get_sound_id(bone 0x1F/0x22 ? "impactd" : "impacta"))
     * TODO: needs an exported sfx_entity(e, name) in sfx.c. */
    if (!(g_combat_flags & 2) && id != 0x1A && id != 0x1B && id != 0x54 && id != 0x19) {
        DHP(e) -= attack_damage(p);
        hud_enemy_bar(e);
    }

    if (E32(p, 0xD04) == 60) {                                /* hero special: finisher */
        if (id == 0x28) {
            E32(e, 0xD04) = 24; E32(e, 0xD70) = 1; E32(e, 0xC64) = 0x10000;
            return;
        }
        i32 tough = (d[0x98] == 3 || id == 0x1E || id == 0x1F || id == 0x5A);
        if (id == 0x19 || id == 0x1A || (tough && DHP(e) > 0)) {
            E32(e, 0xD04) = 27;
        } else {
            u8 pw = EDESC(p)[0x97];
            const creature_entry_t *c = game_init_get_creature(pw);
            i32 foudre = c && (c->flags & 0x10);                 /* is_character_type_c */
            E32(e, 0xD04) = ((pw == 0 || foudre) && !tough) ? 27 : 28;
        }
        if (!(g_combat_flags & 2) && id != 0x1E && id != 0x1F && id != 0x5A &&
            id != 0x19 && id != 0x1A && d[0x98] != 3)
            DHP(e) = 0;
        return;                                                /* D04 played by loco_scripted */
    }

    if (DHP(e) <= 0) {                                         /* killing blow */
        E32(e, 0xD04) = 26; E32(e, 0xD08) = 26;
        ai_play_plain(e, 26);
        return;
    }
    if (id == 0x28) E32(e, 0xC64)++;                           /* P40 counts hits */
    else            E32(e, 0xC64) -= E32(e, 0xC68);            /* stagger gauge */
    hud_enemy_bar(e);
    if (id == 0x28) {
        if (E32(e, 0xC64) == 0x10004) { E32(e, 0xD04) = 24; E32(e, 0xD70) = 1; E32(e, 0xC64) = 0x10000; }
        else E32(e, 0xD04) = 22;
        return;
    }
    if (E32(e, 0xC64) <= 0) {                                  /* gauge empty: knock-down */
        if (!(g_combat_flags & 2))
            E32(e, 0xC68) = (i32)((f64)E32(e, 0xC68) * 1.125);  /* next one comes sooner */
        E32(e, 0xC64) = 0x10000;
        const u8 *wc = (const u8 *)g_world.world_controller;
        if (wc && *(const i32 *)(wc + 0x1E0) != E32(e, 0x8C) && small_zone(e))
            E32(e, 0xD04) = 22;
        else
            E32(e, 0xD04) = (d[0x117 + 24 * 8] == 'x') ? 22 : 24;   /* desc+0x1D7 */
    } else {
        E32(e, 0xD04) = 22;
    }
}

/* ========================================================================
 * sub_470A8F — has the player's attack connected with e this frame?
 * ======================================================================== */
static i32 ai_check_player_hit(entity_t *e) {
    entity_t *p = g_player_entity;
    if (!st_attacking(p) || !in_hit_window(p) || !weapon_hits(p, e)) return 0;
    i32 ps = E32(p, 0xD04);
    if (ps == 60) {                                           /* specials ignore parries */
        ai_take_hit(e); E32(p, 0xC64) = 0x10000; return 1;
    }
    if (st_parrying(e)) return 0;                             /* parried */
    if (ps >= 7 && ps <= 12) {
        ai_take_hit(e); E32(p, 0xC64) = 0x10000; return 1;
    }
    return 0;                                                 /* 14 / 29: not here */
}

/* ========================================================================
 * sub_4715D3 — death, knock-down recovery, victory, return to guard.
 * Returns 1 when it consumed the frame.
 * ======================================================================== */
static i32 ai_housekeep(entity_t *e) {
    i32 id = E32(e, 0xB0), st = E32(e, 0xD04);
    u8 *d = EDESC(e);
    if (st == 26 || st == 27 || st == 28 || DHP(e) <= 0) {
        if (!actor_anim_done(e)) return 1;
        if (DHP(e) > 0) {                                     /* survived a 27/28 */
            if (id != 0x1E && id != 0x1F && id != 0x5A && (d[0x98] == 3 || id == 0x19)) {
                E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1;
                E32(e, 0xD04) = 0; E32(e, 0xD08) = 0;
                E32(e, 0xCF4) = 0; E32(e, 0xCF8) = 0;
            } else if (ai_play_plain(e, 6)) {                 /* get up */
                E32(e, 0xD04) = 6; E32(e, 0xD08) = 6;
            }
        } else {                                              /* dead, anim over */
            g_cinematic_mode = 0;
            E32(e, 0x70) = -1;
            d[0x96] = 1;                                      /* drops out of every tick */
            /* if (id != 0x40) create_model_render_data(e);   TODO
             * render_entity_with_ui(e, 1);  (bakes the corpse)  TODO */
            lieu_set_persona_vie(id, 2);                      /* dword_505DD0[id] = 2 */
        }
        return 1;
    }
    if (st == 15) return 1;                                   /* victory loop */
    if (g_player_entity && DHP(g_player_entity) <= 0) {      /* hero died: gloat */
        E32(e, 0xCF0) = 0; E32(e, 0xCEC) = 0; E32(e, 0xCF8) = 0; E32(e, 0xCF4) = 0;
        E32(e, 0xD04) = 15; E32(e, 0xD08) = 15;
        ai_play_plain(e, 15);
        if (ANIM(e)) CAN_FLAGS(ANIM(e)) |= 0x200;             /* hold last frame */
        return 1;
    }
    if (st == 24) {                                           /* knocked down */
        if (actor_anim_done(e) && ai_play_plain(e, 6)) { E32(e, 0xD04) = 6; E32(e, 0xD08) = 6; }
        return 1;
    }
    if (st == 6) {                                            /* getting up */
        if (actor_anim_done(e)) {
            if (id == 0x28) {
                E32(e, 0xD04) = 0; E32(e, 0xD08) = 0;
                actor_play_anim(e, can_get_anim("p40gar02"), 0);
                E32(e, 0xD70) = 2; E32(e, 0xD74) = now_ms();
            } else {
                E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1;
                E32(e, 0xD04) = 0; E32(e, 0xD08) = 0;
                E32(e, 0xCF4) = 0; E32(e, 0xCF8) = 0;
            }
        }
        return 1;
    }
    i32 done = actor_anim_done(e);
    if (!done && ((ANIM(e) && (CAN_FLAGS(ANIM(e)) & 0x80)) || st_hurt(e))) return 0;
    if ((st != 0 && done) || E32(e, 0xD18)) {                 /* action over -> guard */
        c8 clip[16] = {0};
        E32(e, 0xD18) = 0;
        E32(e, 0xD04) = 0; E32(e, 0xD08) = 0;
        actor_bake_root_yaw(e);
        E32(e, 0xC30) = now_ms();
        if (actor_prepare_anim(e, 5, clip) &&                 /* 5 = combat guard */
            actor_play_anim(e, can_get_anim(clip), 0))
            return 1;
    }
    return 0;
}

/* ========================================================================
 * sub_4711F9 — the player is swinging at me and facing me: answer with an
 * attack timed to land first. Melee only.
 * ======================================================================== */
static i32 ai_counter(entity_t *e, i32 dist, i32 bear_opp) {
    entity_t *p = g_player_entity;
    if (!E32(e, 0xD48) || g_ranged || g_ai_freeze) return 0;
    if (!((f64)dist < (f64)E32(e, 0xC60) * 0.9)) return 0;
    if (abs(bear_opp) >= 0x1000) return 0;                    /* player faces me (+-22.5 deg) */
    if (!st_attacking(p) || st_hurt(e) || st_parrying(e) || st_attacking(e)) return 0;

    i32 base = ((f64)dist < (f64)E32(e, 0xC5C) * 0.9) ? 0 : 3;   /* close set 7..9 / far 10..12 */
    const can_rec_t *pr = ANIM(p);
    i32 win = pr ? CAN_HIT_WINDOW(pr) : 0;
    if ((win >> 16) < E32(p, 0xC40)) {                        /* player's window already passed */
        ai_play_state(e, ai_random(3) + base + 7);
        return 1;
    }
    /* Pick the attack of the set with the shortest cached wind-up that still
     * beats the time left before the player's blade goes live. NB: the
     * original stores 7/8/9 even for base 3 (keeps the bug). */
    i32 left = (win & 0xFFFF) - E32(p, 0xC40);
    i32 best = 0x1B207, pick = -1;
    const i32 *wind = (const i32 *)(EDESC(e) + 4);
    for (i32 k = 0; k < 3; k++) {
        i32 w = wind[7 + base + k];
        if (w == -1) continue;
        if (k == 0 || best > w) { best = w; pick = 7 + k; }
    }
    if (left > best && best != 0x1B207) { ai_play_state(e, pick); return 1; }
    return 0;
}

/* ========================================================================
 * sub_46FDC9 — parry (only called when dist < 180000.0).
 * ======================================================================== */
static i32 ai_parry(entity_t *e) {
    entity_t *p = g_player_entity;
    if (g_ai_freeze || !E32(e, 0xD44) || E32(e, 0xD04) != 0) return 0;
    const can_rec_t *pr = ANIM(p);
    if (!in_hit_window(p)) {
        /* not live yet: wait until the player's clip is past half its
         * duration (word at hdr 0xAA + nframes*2 = last keyframe time) */
        if (!pr || CAN_DURATION(pr) / 2 >= E32(p, 0xC40)) return 0;
    }
    if (pr && (CAN_FLAGS(pr) & 0x40) && E32(p, 0xC40) < (CAN_HIT_WINDOW(pr) & 0xFFFF) - 100)
        return 0;                                             /* >100 ticks before window */
    i32 ps = E32(p, 0xD04);
    if (ps < 7 || ps > 12) return 0;
    E32(e, 0xD04) = ps + 9;                                   /* 7..12 -> parry 16..21 */
    return 1;                                                 /* clip played by loco_scripted */
}

/* ========================================================================
 * sub_470E54 — attack when inside distmin, else walk in. Ranged holders
 * only take the attack branch (no approach).
 * ======================================================================== */
static i32 ai_engage(entity_t *e, i32 dist) {
    entity_t *p = g_player_entity;
    if (g_ai_freeze) return 0;
    if (E32(e, 0xD04) != 0 || E32(e, 0xD14) != 0) return 0;   /* busy / passive (mode bit0) */
    if (E32(e, 0xC50) > dist && E32(e, 0xD48) && E32(e, 0xD04) == 0 &&
        !st_hurt(p) && E32(p, 0xD04) != 6) {
        ai_play_state(e, ai_random(3) + 7);                   /* close attack 7..9 (10 rare) */
        return 1;
    }
    if (g_ranged) return 1;
    if (E32(e, 0xC54) < dist && E32(e, 0xD48)) { E32(e, 0xCEC) = 1; return 1; }       /* beyond distmax: advance */
    if (g_cinematic_mode && E32(e, 0xD48) && E32(e, 0xC60) < dist) { E32(e, 0xCEC) = 1; return 1; }
    if (E32(e, 0xCEC) == 1 && E32(e, 0xC58) >= dist) { E32(e, 0xCEC) = 0; return 1; } /* reached bestdist */
    return 1;
}

/* ========================================================================
 * sub_470BB6 — lined up (|bearing| < 5 deg): close / lunge / ranged strike.
 * Sets D04 only; actor_control's loco_scripted plays it.
 * ======================================================================== */
static i32 ai_strike_facing(entity_t *e, i32 bear_self, i32 dist) {
    entity_t *p = g_player_entity;
    if (g_ai_freeze || !E32(e, 0xD48)) return 0;
    if (!g_cinematic_mode && EPTR(e, 0xA8) && EPTR(p, 0xA8)) {
        i32 ha = *(const i32 *)((const u8 *)EPTR(e, 0xA8) + 0x1E0);
        i32 hb = *(const i32 *)((const u8 *)EPTR(p, 0xA8) + 0x1E0);
        if (abs(ha - hb) > 0x8CA0) return 0;                  /* different floor level */
    }
    if (st_hurt(e) || E32(e, 0xD04) == 6) return 1;
    if (abs(bear_self) >= 0x38E || E32(e, 0xD04) != 0 || st_hurt(p) || E32(p, 0xD04) == 6)
        return 0;
#define STOP() (E32(e, 0xCF0) = 0, E32(e, 0xCEC) = 0, E32(e, 0xCF8) = 0, E32(e, 0xCF4) = 0)
    if (E32(e, 0xC5C) > dist) { STOP(); E32(e, 0xD04) = ai_random(3) + 7; return 1; }   /* distcoupproche */
    if (g_ranged) { STOP(); E32(e, 0xD04) = 14; return 1; }                            /* shoot */
    if (E32(e, 0xC60) > dist) { STOP(); E32(e, 0xD04) = ai_random(3) + 10; return 1; } /* distcouploin: lunge */
#undef STOP
    return 0;
}

/* ========================================================================
 * sub_46034D @ 0x46034D — the player's live strike lands on e (the imp):
 * attacking, inside its hit window, clefs touching. Resets the player's
 * stagger meter (+0xC64 = 0x10000) like the other hit tests.
 * ======================================================================== */
static i32 imp_struck(entity_t *e) {
    entity_t *p = g_player_entity;
    if (!e) return 0;
    if (!st_attacking(p) || !in_hit_window(p) || !weapon_hits(p, e)) return 0;
    E32(p, 0xC64) = 0x10000;
    return 1;
}

/* ========================================================================
 * sub_45FD40 @ 0x45FD40 — the flying imp P92 (persona 0x5C). It hovers
 * 72000 above the floor on a slow bob, keeps 66000..90000 from the hero,
 * lunges ("p92ata01") now and then, and falls dead after "p92imp02".
 *   D04: 0 hover, 1 lunging, 2 hurt ("p92imp01"), 3 dying ("p92imp02")
 *   +0xD34 last lunge roll, +0xD38 start of the lunge / hurt / death,
 *   +0xC34 last anim tick (the step is scaled by the time since).
 * Every move goes through sub_4603CB (actor_nudge): collision-resolved.
 * ======================================================================== */
static void imp_play(entity_t *e, const c8 *clip) {      /* play_entity_animation(e, id, 1) */
    actor_play_anim(e, can_get_anim(clip), 1);
}

static void imp_tick(entity_t *e) {
    i32 now = now_ms();
    obc_node_t *g = ROOT(e);
    entity_t *p = g_player_entity;
    u8 *d = EDESC(e);
    i32 v13 = cosine_table[((u32)(g->rot[2] + 16392) >> 4) & 0xFFF];   /* the original's sine_table is cos */
    i32 v14 = cosine_table[((u32)(g->rot[2] + 8) >> 4) & 0xFFF];
    f64 step = (f64)(now - E32(e, 0xC34)) / 200.0;
    f64 fall;

    if (E32(e, 0xD04) == 3 && !d[0x96]) {                /* dying: sinks, slides back, then lies */
        g->pos[1] -= (i32)(step * 7200.0);
        fall = 5.0 - (f64)(now - E32(e, 0xD38)) / 50.0;
        if (fall < 0.0) fall = 0.0;
        step *= fall;
        actor_nudge(e, (i32)-((f64)v13 * step), (i32)-((f64)v14 * step));
        if (g->pos[1] <= E32(e, 0x8C)) {
            g->pos[1] = E32(e, 0x8C);
            if (actor_anim_done(e)) {
                E32(e, 0x70) = -1;
                d[0x96] = 1;                             /* drops out of every tick */
                /* create_model_render_data(e); render_entity_with_ui(e, 1) bake the corpse. TODO */
                lieu_set_persona_vie(E32(e, 0xB0), 2);   /* dword_505DD0[id] = 2 */
            }
        }
        return;
    }

    f64 dist; i32 bear_self, bear_opp;
    ai_measure(e, &dist, &bear_self, &bear_opp);         /* sub_46FB40 */
    g->rot[2] += bear_self;
    g->pos[1] = cosine_table[((u32)(32 * now + 8) >> 4) & 0xFFF] / 2 + E32(e, 0x8C) + 72000;

    i32 hurt_wait = 0;
    if (E32(e, 0xD04) != 2) {
        if (imp_struck(e)) {                             /* sub_46034D */
            DHP(e) -= 128;
            hud_enemy_bar(e);                            /* sub_45C700 */
            if (DHP(e) > 0) { imp_play(e, "p92imp01"); E32(e, 0xD38) = now; E32(e, 0xD04) = 2; }
            else            { imp_play(e, "p92imp02"); E32(e, 0xD38) = now; E32(e, 0xD04) = 3; }
        }
    } else if (!actor_anim_done(e)) {
        hurt_wait = 1;                                   /* staggering back, no decisions */
    } else {
        imp_play(e, "p92mar01");
        E32(e, 0xD04) = 0;
    }

    if (hurt_wait) {
        fall = 5.0 - (f64)(now - E32(e, 0xD38)) / 50.0;
        if (fall < 0.0) fall = 0.0;
        step *= fall;
        actor_nudge(e, (i32)-((f64)v13 * step), (i32)-((f64)v14 * step));
        return;
    }

    if (E32(e, 0xD04) == 1) {                            /* lunging */
        if (dist >= 54000.0) {
            actor_nudge(e, (i32)((f64)v13 * step * 2.0), (i32)((f64)v14 * step * 2.0));
        } else {
            if (!st_parrying(p)) {                       /* is_creature_in_special_level_range */
                DHP(p) -= 64;
                E32(p, 0xD04) = DHP(p) > 0 ? 22 : 26;
                E32(p, 0xD08) = 0;
            }
            imp_play(e, "p92mar01");
            E32(e, 0xD04) = 0;
        }
    } else if (now - E32(e, 0xD34) > 250) {
        E32(e, 0xD34) = now;
        if (!st_hurt(p) && ai_random(0x100) <= 32) {
            imp_play(e, "p92ata01");
            actor_nudge(e, (i32)((f64)v13 * step), (i32)((f64)v14 * step));
            E32(e, 0xD38) = now;
            E32(e, 0xD04) = 1;
        }
    }
    if (dist < 66000.0) actor_nudge(e, (i32)-((f64)v13 * step), (i32)-((f64)v14 * step));
    if (dist > 90000.0) actor_nudge(e, (i32)((f64)v13 * step), (i32)((f64)v14 * step));
}

/* ========================================================================
 * sub_4701E0 — per-frame combat brain for an NPC with +0xD60 set.
 * ======================================================================== */
/* ========================================================================
 * sub_460444 @ 0x460444 — P40 (persona 0x28) while +0xD70 == 2: it stands in
 * "p40gar02" and only reacts to a landed player strike. State 1997 is its
 * reaction (waits for the clip to end, then back to guard). Any other phase
 * runs the common brain below, like the original's else branch.
 * ======================================================================== */
static void boss_guard_tick(entity_t *e) {
    entity_t *p = g_player_entity;
    if (E32(e, 0xD04) == 1997) {
        if (actor_anim_done(e)) {
            actor_play_anim(e, can_get_anim("p40gar02"), 1);
            E32(e, 0xD08) = 0; E32(e, 0xD04) = 0;
        }
        return;
    }
    if (!st_attacking(p) || !in_hit_window(p) || !weapon_hits(p, e)) return;
    i32 ps = E32(p, 0xD04);
    if (ps == 60) {
        actor_play_anim(e, can_get_anim("p40imp02"), 1);
        E32(e, 0xD74) += 10000;                          /* the guard phase lasts longer */
        E32(e, 0xD08) = 1997; E32(e, 0xD04) = 1997;
    }
    if (ps == 9 || ps == 12 || ps == 7 || ps == 10 || ps == 8 || ps == 11) {
        actor_play_anim(e, can_get_anim("p40imp02"), 1);
        E32(e, 0xD08) = 1997; E32(e, 0xD04) = 1997;
    }
}

void combat_ai_tick(entity_t *e) {
    if (!e || !ROOT(e) || !EDESC(e) || !g_player_entity || !ROOT(g_player_entity)) return;
    u8 *d = EDESC(e);
    i32 now = now_ms();
    /* debug_enabled: draw "NRJ : %d" (HP) at (0, 0x140). */

    if (E32(e, 0xB0) == 0x5C) { imp_tick(e); return; }          /* sub_45FD40: flying imp P92 */
    if (E32(e, 0xB0) == 0x28 && E32(e, 0xD70) == 2) { boss_guard_tick(e); return; }   /* sub_460444, phase 2 */

    if (now - E32(e, 0xD34) > 250) {                          /* re-roll moods 4x/s */
        E32(e, 0xD34) = now;
        i32 hp = DHP(e);
        /* reflexes (0x9B, full HP) .. reflexesfin (0x9D, HP 0); same for
         * agressivite 0x9C .. agressivitefin 0x9E. Chances out of 256. */
        i32 parry_ch  = d[0x9D] - ((d[0x9D] - d[0x9B]) * hp) / 1024;
        i32 attack_ch = d[0x9E] - ((d[0x9E] - d[0x9C]) * hp) / 1024;
        E32(e, 0xD44) = (ai_random(0x100) <= parry_ch);
        if (g_cam_lock && !g_cinematic_mode) E32(e, 0xD48) = 0;   /* script owns the scene */
        else E32(e, 0xD48) = (ai_random(0x100) <= attack_ch);
    }

    g_ranged = ai_ranged(e);
    f64 dist; i32 bear_self, bear_opp;
    ai_measure(e, &dist, &bear_self, &bear_opp);

    i32 st = E32(e, 0xD04);
    if (st != 6 && st != 24 && st != 27 && st != 28 && st != 26 && st != 15)
        ROOT(e)->rot[2] += bear_self / 2;                     /* turn half the gap each frame */

    i32 idist = (i32)dist;
    if (getenv("DKE_AIDBG")) {
        static i32 last = 0;
        if (now - last > 400) {
            last = now;
            const can_rec_t *ar = ANIM(e);
            if (ar) print("ai: clip %s flags=0x%X win=%d..%d c40=%d T=%d bone=%d dmg=%d\n", ESTR(e, 0xC4),
                          CAN_FLAGS(ar), CAN_HIT_WINDOW(ar) & 0xFFFF, (u32)CAN_HIT_WINDOW(ar) >> 16,
                          E32(e, 0xC40), CAN_DURATION(ar), CAN_BONE(ar), CAN_DAMAGE(ar));
            print("ai: p%02d d04=%d d08=%d d14=%d d44=%d d48=%d dist=%d bear=%d C50=%d C54=%d C5C=%d C60=%d hp=%d cam=%d ag=%d/%d\n",
                  E32(e, 0xB0), E32(e, 0xD04), E32(e, 0xD08), E32(e, 0xD14), E32(e, 0xD44), E32(e, 0xD48),
                  idist, bear_self, E32(e, 0xC50), E32(e, 0xC54), E32(e, 0xC5C), E32(e, 0xC60), DHP(e),
                  g_cam_lock, d[0x9C], d[0x9E]);
        }
    }
    if (ai_check_player_hit(e)) return;
    if (ai_housekeep(e)) return;
    if (!g_ranged && ai_counter(e, idist, bear_opp)) return;
    if (dist < 180000.0 && ai_parry(e)) return;
    ai_engage(e, idist);                                      /* return value ignored */
    if (E32(e, 0xCEC) == -1) return;
    ai_strike_facing(e, bear_self, idist);
}

/* ========================================================================
 * sub_46642E head (0x466446..0x4665CC): NPC-vs-NPC fights while
 * cinematic_mode != 0. Runs every frame regardless of +0xD60, in addition to
 * the D60-gated calls in sub_461468. Opponent = whatever g_player_entity is
 * swapped to; restored afterwards.
 *   cinematic_mode == 1 : the two slots fight each other.
 *   cinematic_mode == id: that persona attacks the other slot; the other
 *                         slot attacks the nearer of the hero / that persona.
 * ======================================================================== */
static i32 ent_dist(entity_t *a, entity_t *b) {             /* get_entity_distance */
    if (!a || !b) return -1;
    f64 dx = ROOT(a)->pos[0] - ROOT(b)->pos[0], dz = ROOT(a)->pos[2] - ROOT(b)->pos[2];
    return (i32)sqrt(dz * dz + dx * dx);
}

void combat_ai_cinematic(void) {
    entity_t *hero = g_player_entity, *c1 = g_companion_entity, *c2 = g_secondary_companion_entity;
    if (!g_cinematic_mode) return;
    if (g_cinematic_mode == 1) {
        if (c1 && c2) {
            g_player_entity = c1; combat_ai_tick(c2);
            g_player_entity = c2; combat_ai_tick(c1);
        }
    } else {
        /* NB the original dereferences c1/c2 without NULL checks here. */
        if (c1 && E32(c1, 0xB0) == g_cinematic_mode) {
            g_player_entity = c2; combat_ai_tick(c1);
        } else if (c1) {
            g_player_entity = (ent_dist(c1, c2) < ent_dist(c1, hero)) ? c2 : hero;
            combat_ai_tick(c1);
        }
        if (c2 && E32(c2, 0xB0) == g_cinematic_mode) {
            g_player_entity = c1; combat_ai_tick(c2);
        } else if (c2) {
            g_player_entity = (ent_dist(c2, c1) < ent_dist(c2, hero)) ? c1 : hero;
            combat_ai_tick(c2);
        }
    }
    g_player_entity = hero;
}

/* sub_46642E 0x466884..0x466911: hero auto-fight (COMBAT on a hero id, or
 * debug 'k'): when not locked, special_render_flags == 0, g_hero_autofight,
 * hero D60 and !desc[0x96]: run combat_ai_tick(hero) once against each slot
 * (g_player_entity swapped to c1, then c2) and SKIP sub_451BF9 input. */
void combat_ai_hero_autofight(void) {
    entity_t *hero = g_player_entity;
    if (!hero || !g_hero_autofight || !E32(hero, 0xD60) || EDESC(hero)[0x96]) return;
    if (g_companion_entity)           { g_player_entity = g_companion_entity;           combat_ai_tick(hero); }
    if (g_secondary_companion_entity) { g_player_entity = g_secondary_companion_entity; combat_ai_tick(hero); }
    g_player_entity = hero;
}

/* ------------------------------------------------------------------------
 * INSERTION POINTS (do not apply blindly; src/ untouched by this draft)
 *
 * 1. src/actor_move.c companion_tick() (0x461675 / 0x461715):
 *        if (!c || !EDESC(c) || EDESC(c)[0x96]) return;
 *        if (E32(c, 0xD60)) combat_ai_tick(c);          // <- replace the comment
 *        actor_update(c); ...
 *
 * 2. src/actor_move.c player_tick() top (sub_46642E 0x466446):
 *        combat_ai_cinematic();          // before the CBC walk test
 *    and in the "not locked" branch, before scene input (0x466884):
 *        if (g_hero_autofight && hero D60 ...) { combat_ai_hero_autofight(); return; }
 *
 * 3. src/lieu_cb.c combat_set(): `g_combat_flags = mode;` where the comment
 *    "debug_flags = mode" is; `g_hero_autofight = 1;` in the id<4 path;
 *    `g_enemy_hits = 0;` for "dword_506520 = 0". DIVERS op 21: g_combat_flags = b.
 *    Make g_cam_lock non-static (or add a getter).
 *
 * 4. src/actor.c actor_desc_load(): BUG -- with the lip-sync flag set the OME
 *    carries SIX ints (coefbouche, gauche, droite, haut gauche, haut droite,
 *    bas -> dword_4ECD40, 4EC828, 4EC82C, 4DF5B8, 4DF5BC, 4DF5C0; reader
 *    sub_462C9E @ 0x463?? / writer sub_46319A 0x4636BF), not five. Today
 *    C50..C58 and desc+0x84 are read one int early for every such persona
 *    (P87/P88/P89/P12...). Fix: g_ai_param[6], loop k < 6.
 *
 * 5. Enemy -> hero damage is NOT here: sub_451BF9 (hero input, unported)
 *    calls process_combat_block(hero, c1/c2) (weapon clash), then
 *    process_entity_damage_attempt(c1, hero, dist1) and (c2, hero, dist2)
 *    @ 0x401DA0, which mirrors ai_check_player_hit with roles swapped
 *    (+ ranged branch: special attacker, dist >= C5C, hero D1C set by the
 *    projectile) and calls process_entity_damage @ 0x401530 (mirror of
 *    ai_take_hit for the hero: 7/10 -> 22, 8/11 -> 27, 9/12 -> 28, 14 -> 22).
 *    Without it enemies swing harmlessly.
 * ---------------------------------------------------------------------- */
