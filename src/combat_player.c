/*
 * combat_player_draft.c -- DRAFT, not wired into the build.
 *
 * Player side of combat and held weapons for the dke C port.
 * Written against src/entity.h (E32/EPTR/EROOT/EBONE/EDESC), src/actor.h
 * (actor_prepare_anim / actor_play_anim / actor_anim_done /
 * actor_bake_root_yaw / can_get_anim) and src/object.h (game_object_t).
 *
 * Proposed home: a new src/combat.c (+ combat.h), with the small pieces
 * marked "INSERT INTO <file>" moved into existing files. Every function
 * cites its original address. Unported dependencies are declared as
 * combat_hook_* externs with a TODO; stub them to 0 first.
 *
 * Persona state indices used here (desc+0x117+st*8 for st >= 0):
 *    1/2 combat walk fwd/back   3/4 combat turn R/L      5 combat idle
 *    6   get up (after 0x18)    7/8/9 close attack fwd/right/left
 *   10/11/12 long attack fwd/right/left                  0x0E fire (shot)
 *   0x0F victory (all enemies down, flag 0x200 = hold last frame)
 *   0x10..0x15 parry vs bare-handed attack 7..12
 *   0x16 hit (front)   0x17 special move   0x18 knock-down -> 6
 *   0x19 aim / dry fire (no ammo)          0x1A death
 *   0x1B / 0x1C hit from right / left attack (and victim of 0x3C)
 *   0x1D special move part 1 -> 0x3C (+100 score when it ends)
 *   0x1E..0x35 parry vs weapon classes (see combat_choose_parry)
 *   0x3E Space in stance (heroes only)
 *   54..61 player-only "special_mode_active == 0" locomotion set
 */

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "libsys/common.h"
#include "libsys/system.h"
#include "state.h"
#include "entity.h"
#include "actor.h"
#include "can.h"
#include "obc.h"
#include "object.h"
#include "lieu_cb.h"

#include "libsys/graphics/shading.h"   /* i16 cosine_table[] = the asm sine_table (same indexing as actor_move.c) */

/* ------------------------------------------------------------------------
 * Globals the original keeps next to the player (see notes for owners)
 * ------------------------------------------------------------------------ */
i32 g_control_scheme      = 0;  /* dword_4B7494: 0 = sub_451BF9, 1 = sub_4342C7;
                                   toggled on the MENUPERS options page
                                   (sub_43F8C7 @ 0x440308), saved in gamedke.cfg
                                   (config_load_gamedke_cfg @ 0x45B06F). */
/* Copies the scripts read: dword_505C50 / 505D68 / 505DD8 / 505DDC are
 * slots of the script state block dword_505C40. */
#define g_held_weapon_id      (*lieu_state_slot(ST_PLAYER_WEAPON))  /* desc[0x97] */
#define g_held_variant        (*lieu_state_slot(ST_USED_VARIANT))   /* +0xD6 - 'A' (sub_471B80) */
#define g_stance_copy         (*lieu_state_slot(ST_COMBAT_STANCE))  /* +0xD60 */
#define g_special_mode_copy   (*lieu_state_slot(ST_SPECIAL_MODE))
i32 g_bare_desc_cached    = 0;  /* dword_4B74A0: hero id + 1 whose bare-hand
                                   descriptor is cached (sub_46386D/sub_463906) */

#define DKE_IS_USING_HDD 1      /* is_using_hdd. The port always runs the HDD
                                   install path; on that path the original
                                   ignores the 0xE2 / Shift attack-range
                                   overrides (0x4521FB / 0x452224). Set to 0 to
                                   get the CD-install behaviour. */

/* ------------------------------------------------------------------------
 * Hooks into code that is not ported yet (TODO: implement or stub to 0)
 * ------------------------------------------------------------------------ */
extern i32  combat_hook_score_get(void);                     /* score_counter (scene.c static) */
extern void combat_hook_score_add(i32 v);                    /* + update_player_interface */
extern void combat_hook_hud_refresh(void);                   /* update_player_interface(ctx,1) */
extern i32  combat_hook_block(entity_t *p, entity_t *enemy); /* process_combat_block @ 0x401CD8? (4923) */
extern i32  combat_hook_entity_damage(entity_t *atk, entity_t *vic); /* calculate_entity_damage */
extern i32  combat_hook_check_revival(entity_t *vic);        /* check_entity_revival */
extern i32  combat_hook_contact(entity_t *atk, entity_t *vic); /* sub_429695: weapon contact test */
extern void combat_hook_blood(entity_t *vic);                   /* play_sound_at_position @ 0x42CDA6 */
extern i32  combat_hook_has_ammo(const c8 *weapon_name);     /* sub_475990 @ 0x475990 */
extern i32  combat_hook_ray_dist(entity_t *atk, f64 *out);   /* get_distance_between_points */
extern i32  combat_hook_in_cone(const i32 *tgt, const i32 *from, const i32 *dir, f64 dist); /* is_point_in_view_cone */
extern i32  combat_hook_head_track(entity_t *self, entity_t *tgt);  /* = actor_head_track (sub_450DE8), export it */
extern void combat_hook_objects_flagbits(game_object_t *o);  /* sub_4728DF + byte_4DCCC0 update in sub_473756 */
extern u8   objets_flags(i32 id);                            /* combat_ability_table[id*20], export from inventory.c */
extern i32  inv_set_equipped(const c8 *obj_name, i32 on);    /* sub_426BB5 (inventory.c) */
extern game_object_t *object_get(const c8 *name);            /* export obj_get (sub_475BB6 + sub_472C5C) */
extern i32  actor_desc_reload(entity_t *e, i32 weapon);      /* see INSERT INTO actor.c below */

#define DESC_HP(e)     (*(i32 *)(EDESC(e) + 0x84))
#define DESC_WEAPON(e) (EDESC(e)[0x97])
#define DESC_HAND(e)   (EDESC(e)[0x99])
#define ANIMREC(e)     ((can_rec_t *)EPTR(e, 0xD50))

static i32 now_ms(void) { return state ? (i32)state->current_game_time : 0; }

/* ========================================================================
 * 1. Keys. process_input_command(vk) = held, process_input_event(vk, 0) =
 *    key-down edge. VK codes as in the original (they index sys->actions).
 *    process_input_command @ 0x405820 also maps the joystick: Up/Down/Left/
 *    Right from the stick, button 1 = Shift (0x10), 2 = Space, 4 = Ctrl,
 *    8 = Enter, 0x10 = Esc, 0x20 = 'C'.
 * ======================================================================== */
enum {
    VK_TAB_ = 0x09, VK_SHIFT_ = 0x10, VK_CTRL_ = 0x11, VK_ALT_ = 0x12,
    VK_SPACE_ = 0x20, VK_LEFT_ = 0x25, VK_UP_ = 0x26, VK_RIGHT_ = 0x27,
    VK_DOWN_ = 0x28, VK_C_ = 0x43, VK_OEM102_ = 0xE2
};

static i32 s_key[256], s_key_prev[256];

/* INSERT INTO scene.c: call once per frame where the "pin.velocity" stand-in
 * reads player_input_update (scene.c ~3166), before actor_frame runs. */
void combat_keys_latch(const system_t *sys) {
    memcpy(s_key_prev, s_key, sizeof(s_key));
    for (i32 i = 0; i < 256; i++) s_key[i] = sys ? (sys->actions[i] != 0) : 0;
    /* Headless: DKE_AUTOATTACK holds Ctrl+Up for 120 ms every 900 ms. */
    if (getenv("DKE_AUTOATTACK")) {
        i32 t = now_ms() % 900;
        if (t < 120) { s_key[VK_CTRL_] = 1; s_key[VK_UP_] = 1; }
    }
    /* WASD mirrors the arrows, like player_input_update does today. */
    if (sys) {
        s_key[VK_UP_]    |= sys->actions['w'] != 0;
        s_key[VK_DOWN_]  |= sys->actions['s'] != 0;
        s_key[VK_LEFT_]  |= sys->actions['a'] != 0;
        s_key[VK_RIGHT_] |= sys->actions['d'] != 0;
    }
}
static i32 key(i32 vk)      { return s_key[vk & 0xFF]; }                         /* process_input_command */
static i32 key_edge(i32 vk) { return s_key[vk & 0xFF] && !s_key_prev[vk & 0xFF]; } /* process_input_event */

/* ========================================================================
 * 2. State classes and small helpers
 * ======================================================================== */

/* is_creature_special_level_type @ 0x401201: attacking. */
static i32 st_attacking(const entity_t *e) {
    if (!e) return 0;
    i32 s = E32(e, 0xD04);
    return (s >= 7 && s <= 12) || s == 0x1D || s == 0x3C || s == 0x0E;
}
/* is_creature_in_special_level_range @ 0x40119A: parrying. */
static i32 st_parrying(const entity_t *e) {
    if (!e) return 0;
    i32 s = E32(e, 0xD04);
    return (s >= 0x10 && s <= 0x15) || (s >= 0x1E && s <= 0x35);
}
/* is_master_eligible_creature @ 0x401275: hurt (NULL counts as hurt). */
static i32 st_hurt(const entity_t *e) {
    if (!e) return 1;
    i32 s = E32(e, 0xD04);
    return s >= 0x16 && s <= 0x1C && s != 0x17 && s != 0x19;
}
/* sub_4295E0 @ 0x4295E0: the clip is in its strike window
 * (anim flag 0x40, keyframe +0xC40 in [lo16, hi16] of anim+0x100). */
static i32 st_strike_window(const entity_t *e) {
    if (E32(e, 0xD18) || E32(e, 0xD1C)) return 0;
    const can_rec_t *r = ANIMREC(e);
    if (!r || !(CAN_FLAGS(r) & 0x40)) return 0;
    i32 w = *(const i32 *)&r->hdr[0x100];
    i32 lo = w & 0xFFFF, hi = w >> 16;
    return E32(e, 0xC40) >= lo && E32(e, 0xC40) <= hi;
}

/* combat_ability_table classes (OBJETS.TXT keys, game_init.c:893). */
#define ABIL_SABRE    0x04   /* is_character_type_a @ 0x4726E6 */
#define ABIL_HACHE    0x08   /* is_character_type_b            */
#define ABIL_FOUDRE   0x10   /* is_character_type_c @ 0x4726F2 */
#define ABIL_LANCEFEU 0x20   /* sub_47271C                     */
#define ABIL_CRACKER  0x40   /* sub_472746                     */
#define ABIL_PISTOLET 0x80   /* sub_472770                     */
static i32 abil(i32 id, u8 bit) { return (id > 0 && id < 100) ? (objets_flags(id) & bit) : 0; }

/* check_entity_special_abilities @ 0x401134: holding a ranged weapon. */
static i32 has_ranged(const entity_t *e) {
    if (!e || !EDESC(e)) return 0;
    u8 f = objets_flags(DESC_WEAPON(e));
    return (f & 0xD0) || (f & 0x20);
}

/* sub_450C70 / sub_450CDD: the two companion slots as opponents. */
static entity_t *opponent(entity_t *c) {
    if (!c || !EDESC(c) || EDESC(c)[0x96]) return NULL;
    if (E32(c, 0xB0) == g_cinematic_mode) return NULL;
    return c;
}

/* get_entity_distance @ 0x450D50: XZ distance, -1 when either is NULL. */
static i32 ent_dist(const entity_t *a, const entity_t *b) {
    if (!a || !b) return -1;
    f64 dx = EROOT(a)->pos[0] - EROOT(b)->pos[0];
    f64 dz = EROOT(a)->pos[2] - EROOT(b)->pos[2];
    return (i32)sqrt(dz * dz + dx * dx);
}

/* Body heading: gravit yaw, minus the skeleton root's X angle when the clip
 * has flag 1 (0x451042). */
static i32 body_heading(const entity_t *e) {
    const obc_node_t *g = EROOT(e);
    const can_rec_t *r = ANIMREC(e);
    if (r && (CAN_FLAGS(r) & 1) && g->child)
        return (g->rot[2] - (g->child->rot[0] - CAN_ROOT_YAW(r))) & 0xFFFF;
    return g->rot[2] & 0xFFFF;
}

/* sub_450FD3 @ 0x450FD3: signed BAM turn from the player's heading to the
 * target; INT_MAX when there is no target. (sub_434160 is the same test.) */
static i32 yaw_to(const entity_t *t) {
    entity_t *p = g_player_entity;
    if (!t) return INT_MAX;
    f64 dx = EROOT(p)->pos[0] - EROOT(t)->pos[0];
    f64 dz = EROOT(p)->pos[2] - EROOT(t)->pos[2];
    i32 h = body_heading(p);
    i32 a = -(i32)(atan2(-dx, -dz) * 65536.0 / 6.283185307179586) & 0xFFFF;
    i32 d = 0;
    if (a >= h) { d = a - h; if (d > 0x8000) d -= 0x10000; }
    if (a <  h) { d = a - h; if (d < -0x8000) d = 0x8000 - (-0x8000 - d); }
    return d;
}

/* sub_45113A @ 0x45113A: auto-face. Turn half-way toward the target when it
 * is inside +0xC60 and 0x38E <= |turn| <= 0x31C4. Always returns 0 (the
 * caller stores it in +0xCF4). */
static i32 face_half(entity_t *t, i32 dist) {
    if (!t) return 0;
    i32 d = yaw_to(t);
    if (abs(d) < 0x38E || abs(d) > 0x31C4) return 0;
    if (E32(g_player_entity, 0xC60) > dist) EROOT(g_player_entity)->rot[2] += d / 2;
    return 0;
}

/* sub_43503E @ 0x43503E (scheme 1): always turn half-way. */
static void face_half_always(entity_t *t) {
    i32 d = yaw_to(t);
    if (d != INT_MAX) EROOT(g_player_entity)->rot[2] += d / 2;
}

/* prepare + play one persona state; the asm pattern
 * memset / process_entity_state_change / prepare / get_animation_id / play. */
static i32 play_state(entity_t *e, i32 st) {
    c8 clip[16] = {0};
    actor_bake_root_yaw(e);
    if (!actor_prepare_anim(e, st, clip)) return 0;
    return actor_play_anim(e, can_get_anim(clip), 0);
}

/* Marks the current clip as already finished (0x473CB2 / 0x401D5C). */
static void clip_force_end(entity_t *e) {
    const can_rec_t *r = ANIMREC(e);
    if (!r) return;
    E32(e, 0xC28) = now_ms() - (i32)((f64)CAN_DURATION(r) / g_anim_divisor);
    E32(e, 0xE4) = 1;
}

/* sub_419117 @ 0x419117: the current clip is an idle (fidget restartable). */
static i32 idle_clip(const entity_t *e) {
    i32 s = E32(e, 0xD58);
    return s == -5 || s == 59 || s == 55 || s == 54 || s >= 1000;
}

/* ========================================================================
 * 3. End of a combat action and the input gate
 * ======================================================================== */

/* end_entity_invulnerability @ 0x401BA5 (misnamed): when the action clip in
 * +0xD04 has played out, clear it and go back to the stance idle. Runs every
 * frame from the input handler, so attacks / parries / hits lock the player
 * until their clip ends (unless message 0x20 opened +0xCE8, see below). */
i32 combat_end_action(entity_t *e) {
    if (!actor_anim_done(e)) {
        const can_rec_t *r = ANIMREC(e);
        if ((r && (CAN_FLAGS(r) & 0x80)) || st_hurt(e)) return 0;
    }
    i32 atk = st_attacking(e);
    if (!(E32(e, 0xD04) != 0 && actor_anim_done(e))) {
        if (!atk || E32(e, 0xD18) == 0) return 0;
    }
    if (e == g_player_entity && E32(e, 0xD04) == 0x3C) combat_hook_score_add(100);
    E32(e, 0xD18) = 0;
    E32(e, 0xD08) = 0;
    E32(e, 0xD04) = 0;
    i32 st = E32(e, 0xD60) ? 5 : (g_special_mode_active || e != g_player_entity) ? -5 : 59;
    E32(e, 0xC30) = now_ms();
    if (!play_state(e, st)) return 0;
    if (g_player_movement_locked && e == g_player_entity) clip_force_end(e);
    return 1;
}

/* sub_4511D5 @ 0x4511D5: every active opponent (+0xD60 or +0xCD4) is out
 * (+0x70 == -1). 0 when nobody is active. */
static i32 fight_over(entity_t *a, entity_t *b) {
    i32 aa = a && (E32(a, 0xD60) || E32(a, 0xCD4));
    i32 bb = b && (E32(b, 0xD60) || E32(b, 0xCD4));
    if (aa && bb) return E32(a, 0x70) == -1 && E32(b, 0x70) == -1;
    if (aa) return E32(a, 0x70) == -1;
    if (bb) return E32(b, 0x70) == -1;
    return 0;
}

/* handle_entity_state_transition @ 0x401406: knock-down 0x18 -> get-up 6
 * -> back to locomotion. Returns 1 while busy. */
static i32 knockdown_tick(entity_t *e) {
    if (E32(e, 0xD04) == 0x18) {
        if (actor_anim_done(e) && play_state(e, 6)) E32(e, 0xD04) = E32(e, 0xD08) = 6;
        return 1;
    }
    if (E32(e, 0xD04) == 6) {
        if (actor_anim_done(e)) {
            E32(e, 0xCEC) = 0; E32(e, 0xCF0) = 1;
            E32(e, 0xD04) = E32(e, 0xD08) = 0;
            E32(e, 0xCF4) = E32(e, 0xCF8) = 0;
        }
        return 1;
    }
    return 0;
}

/* sub_4512E4 @ 0x4512E4: victory pose / special-move chain / knock-down.
 * Returns 1 to skip the rest of the input handler. */
static i32 combat_gate(void) {
    entity_t *p = g_player_entity;
    if (E32(p, 0xD04) == 0x0F) {                         /* victory, hold */
        if (actor_anim_done(p)) {
            E32(p, 0xD04) = E32(p, 0xD08) = 0;
            E32(p, 0xCEC) = 0; E32(p, 0xCF0) = 1;
            E32(p, 0xCF4) = E32(p, 0xCF8) = 0;
        }
        return 1;
    }
    if (fight_over(g_companion_entity, g_secondary_companion_entity)) {
        entity_t *c[2] = { g_companion_entity, g_secondary_companion_entity };
        for (i32 i = 0; i < 2; i++)
            if (c[i] && EDESC(c[i]) && DESC_HP(c[i]) <= 0) E32(c[i], 0x70) = 2;
        E32(p, 0xCEC) = E32(p, 0xCF0) = 0;
        E32(p, 0xCF4) = E32(p, 0xCF8) = 0;
        /* debug_flags & 4 skips the pose (0x45145F). */
        if (play_state(p, 0x0F)) {
            E32(p, 0xD04) = E32(p, 0xD08) = 0x0F;
            CAN_FLAGS(ANIMREC(p)) |= 0x200;              /* hold the last frame */
        }
        return 1;
    }
    if (E32(p, 0xD04) == 0x1D) {                         /* special move part 2 */
        if (actor_anim_done(p) && play_state(p, 0x3C)) E32(p, 0xD04) = E32(p, 0xD08) = 0x3C;
        return 1;
    }
    return knockdown_tick(p);
}

/* ========================================================================
 * 4. Being hit -- process_entity_damage_attempt / process_entity_damage
 * ======================================================================== */

/* process_entity_damage @ 0x401531 (ecx = attacker, edx = victim). */
static void combat_damage(entity_t *atk, entity_t *vic, i32 dist) {
    (void)dist;
    if (st_hurt(vic)) return;
    combat_hook_blood(vic);                              /* 0x401590: blood spray at the contact point */
    /* damage_counter++, impact sounds "impactd" (anim+0xFC 0x1F/0x22) or
     * "impacta" — sfx.c. */
    i32 id = E32(vic, 0xB0);
    if (id != 0x1A && id != 0x1B && id != 0x54 && id != 0x19 && id != 0x28)
        DESC_HP(vic) -= combat_hook_entity_damage(atk, vic);
    if (vic == g_player_entity) combat_hook_hud_refresh();
    if (getenv("DKE_AIDBG"))
        print("combat: p%02d hits p%02d (state %d) -> hp %d\n", E32(atk, 0xB0), id, E32(atk, 0xD04), DESC_HP(vic));

    if (E32(atk, 0xD04) == 0x3C) {                       /* special move: instant */
        i32 w = DESC_WEAPON(atk);
        E32(vic, 0xD04) = (w == 0 || abil(w, ABIL_FOUDRE)) ? 0x1B : 0x1C;
        if (id != 0x1E && id != 0x1F && id != 0x5A && id != 0x19 && id != 0x1A &&
            EDESC(vic)[0x98] != 3)
            DESC_HP(vic) = 0;
        return;
    }
    if (DESC_HP(vic) <= 0) {                             /* death, played now */
        E32(vic, 0xD04) = E32(vic, 0xD08) = 0x1A;
        play_state(vic, 0x1A);
        if (vic == g_player_entity) lieu_set_death_state(4);   /* 0x401804 */
        return;
    }
    E32(vic, 0xC64) -= E32(vic, 0xC68);                  /* stamina */
    if (E32(vic, 0xC64) <= 0) {
        /* C68 *= damage_multiplier (debug_flags & 2 skips). */
        E32(vic, 0xC64) = 0x10000;
        if (/* world+0x1E0 == floor || */ !combat_hook_check_revival(vic)) {
            E32(vic, 0xD04) = (EDESC(vic)[0x1D7] == 'x') ? 0x16 : 0x18;
            return;
        }
    }
    if (vic != g_player_entity) { E32(vic, 0xD04) = 0x16; return; }
    switch (E32(atk, 0xD04)) {                           /* jpt_40195A */
    case 7: case 10: case 0x0E: E32(vic, 0xD04) = 0x16; break;
    case 8: case 11:            E32(vic, 0xD04) = 0x1B; break;
    case 9: case 12:            E32(vic, 0xD04) = 0x1C; break;
    default: break;
    }
}

/* process_entity_damage_attempt @ 0x401DAE (ecx = attacker, edx = victim). */
static i32 combat_damage_attempt(entity_t *atk, entity_t *vic, i32 dist) {
    if (!atk || !vic || DESC_HP(vic) <= 0) return 0;
    if (!st_attacking(atk)) return 0;
    if (has_ranged(atk) && E32(vic, 0xC5C) <= dist) {    /* shot from range */
        if (!E32(vic, 0xD1C)) return 0;
        combat_damage(atk, vic, dist);
        E32(vic, 0xD1C) = 0;
        E32(atk, 0xC64) = 0x10000;
        return 1;
    }
    if (getenv("DKE_AIDBG") && st_strike_window(atk))
        print("hit?: p%02d st=%d dist=%d contact=%d\n", E32(atk, 0xB0), E32(atk, 0xD04), dist,
              combat_hook_contact(atk, vic));
    if (!st_strike_window(atk) || !combat_hook_contact(atk, vic)) return 0;
    if (E32(atk, 0xD04) != 0x3C && st_parrying(vic)) return 0;   /* parried */
    if (E32(atk, 0xD04) == 0x3C ||
        (E32(atk, 0xD04) >= 7 && E32(atk, 0xD04) <= 12)) {
        combat_damage(atk, vic, dist);
        E32(atk, 0xC64) = 0x10000;
        return 1;
    }
    return 0;
}

/* calculate_attack_trajectory @ 0x402110 (ecx = t1, edx = t2, arg0 =
 * attacker, arg1 = collide): hitscan shot along the attacker's heading.
 * Needs Alt held, an NPC attacker, or ammo (sub_475990). Sets +0xD04 = 0xE.
 * A hit sets target +0xD1C (pending, read by combat_damage_attempt) and
 * +0xC74; a miss stores the end point in attacker +0xD20..+0xD28. */
static i32 combat_shot(entity_t *t1, entity_t *t2, entity_t *atk, i32 collide) {
    if (!key(VK_ALT_) && atk == g_player_entity && !combat_hook_has_ammo(ESTR(atk, 0xD2)))
        return 0;
    E32(atk, 0xD04) = 0x0E;
    i32 a = (body_heading(atk) + 0x4000) & 0xFFFF;
    f64 s = cosine_table[((a + 8) >> 4) & 0xFFF];      /* asm sine_table == port cosine_table */
    f64 c = cosine_table[((a - 0x3FF8) >> 4) & 0xFFF];
    i32 dir[3] = { (i32)s, 0, (i32)c };   /* asm: dir_x, dir_z written as two ints */
    f64 dist = 0;
    if (!combat_hook_ray_dist(atk, &dist)) return 0;
    entity_t *t[2] = { t1, t2 };
    for (i32 i = 0; i < 2; i++) {
        if (!t[i] || !collide || E32(t[i], 0xD1C)) continue;
        if (!combat_hook_in_cone(EROOT(t[i])->pos, EROOT(atk)->pos, dir, dist)) continue;
        if (t[i] == g_player_entity) E32(t[i], 0xD1C) = 1;   /* + sound_enabled = 9 */
        E32(t[i], 0xC74) = 1;
        return 1;
    }
    E32(atk, 0xD20) = (i32)(s * dist + EROOT(atk)->pos[0]);
    E32(atk, 0xD24) = EROOT(atk)->pos[1];
    E32(atk, 0xD28) = (i32)(c * dist + EROOT(atk)->pos[2]);
    return 0;
}

/* Firing with a ranged weapon (0x4523C0 / 0x4349A3): aim state 0x19, then
 * the shot for cracker/foudre/pistolet, or 0xE for the lance-feu when Alt or
 * ammo. */
static void combat_fire(entity_t *e1, entity_t *e2) {
    entity_t *p = g_player_entity;
    i32 w = DESC_WEAPON(p);
    E32(p, 0xD04) = 0x19;
    if (abil(w, ABIL_CRACKER) || abil(w, ABIL_FOUDRE) || abil(w, ABIL_PISTOLET))
        combat_shot(e1, e2, p, 1);
    if (abil(w, ABIL_LANCEFEU) && (key(VK_ALT_) || combat_hook_has_ammo(ESTR(p, 0xD2))))
        E32(p, 0xD04) = 0x0E;
}

/* sub_4515FD @ 0x4515FD: parry chosen from the enemy's attack (8/11 right,
 * 9/12 left, else front), its weapon class and the range (+0xC5C). */
static i32 combat_choose_parry(entity_t *en, i32 dist) {
    entity_t *p = g_player_entity;
    if (!en) { E32(p, 0xD04) = 0x13; return 1; }
    static const struct { u8 cls; i32 near_[3], far_[3]; } tab[] = {
        /* order inside a triple: front, right(8/11), left(9/12) */
        { 0,             {0x10, 0x11, 0x12}, {0x13, 0x14, 0x15} },
        { ABIL_SABRE,    {0x1E, 0x1F, 0x20}, {0x2A, 0x2B, 0x2C} },
        { ABIL_HACHE,    {0x21, 0x22, 0x23}, {0x2D, 0x2E, 0x2F} },
        { ABIL_FOUDRE,   {0x24, 0x25, 0x26}, {0x30, 0x31, 0x32} },
        { ABIL_LANCEFEU, {0x27, 0x28, 0x29}, {0x33, 0x34, 0x35} },
    };
    i32 s = E32(en, 0xD04), w = DESC_WEAPON(en);
    i32 side = (s == 8 || s == 11) ? 1 : (s == 9 || s == 12) ? 2 : 0;
    i32 near_ = E32(p, 0xC5C) > dist;
    for (u32 i = 0; i < sizeof(tab) / sizeof(tab[0]); i++) {
        if (i == 0 ? w != 0 : !abil(w, tab[i].cls)) continue;
        E32(p, 0xD04) = near_ ? tab[i].near_[side] : tab[i].far_[side];
        return 1;
    }
    return 0;
}

/* ========================================================================
 * 5. Input handlers
 * ======================================================================== */

/* Tab / 'C' toggles, identical in both schemes (0x451D19 / 0x434520).
 * Returns 1 when 'C' toggled the stance (the handler then returns). */
static i32 input_toggles(void) {
    entity_t *p = g_player_entity;
    if (!E32(p, 0xD60) && !key(VK_ALT_) && key_edge(VK_TAB_)) {
        g_special_mode_active ^= 1;                      /* walk-set toggle */
        g_special_mode_copy = g_special_mode_active;
        if (E32(p, 0xCEC) == 1) E32(p, 0xCF0) = 0;       /* restart the walk clip */
        if (E32(p, 0xCEC) == 0) E32(p, 0xCF0) = 1;       /* restart the idle */
        combat_hook_hud_refresh();
    }
    if (E32(p, 0xD04) == 0 && key_edge(VK_C_)) {         /* draw / sheathe */
        E32(p, 0xD60) ^= 1;
        g_stance_copy = E32(p, 0xD60);
        if (E32(p, 0xCEC) != 3) {
            E32(p, 0xCEC) = 0; E32(p, 0xCF0) = 1;
            E32(p, 0xCF8) = 0; E32(p, 0xCF4) = 0;
        }
        E32(p, 0xD04) = 0;
        E32(p, 0xD00) = 0;
        combat_hook_hud_refresh();
        return 1;
    }
    return 0;
}

/* Aim-turn with a ranged weapon in stance (0x45269E / 0x434DA2): turn the
 * gravit directly by 30 BAM/ms and play the stance turn clip 3/4. */
static void aim_turn(i32 dir) {
    entity_t *p = g_player_entity;
    i32 dt = now_ms() - E32(p, 0xE8);
    EROOT(p)->rot[2] += dir * dt * 30;
    i32 st = dir > 0 ? 3 : 4;
    if (E32(p, 0xD58) != st && E32(p, 0xD04) == 0) play_state(p, st);
}

/* Common prologue (both schemes): opponents, distances, look-at. */
typedef struct {
    entity_t *e1, *e2;   /* var_18 / var_24 */
    i32 d1, d2;          /* var_C / var_14  */
    i32 dn;              /* var_10: nearer of the two (-1 ignored) */
    i32 ranged;          /* var_4 */
} combat_ctx_t;

static void ctx_init(combat_ctx_t *c) {
    entity_t *p = g_player_entity;
    c->e1 = opponent(g_companion_entity);
    c->e2 = opponent(g_secondary_companion_entity);
    c->d1 = ent_dist(p, c->e1);
    c->d2 = ent_dist(p, c->e2);
    c->ranged = has_ranged(p);
    if (c->d2 > c->d1) c->dn = (c->d1 != -1) ? c->d1 : c->d2;   /* 0x451F36 */
    else               c->dn = (c->d2 != -1) ? c->d2 : c->d1;
}

/* Shared middle part: gate, blocks, end-of-action, being hit, busy test.
 * Returns 0 = stop, 1 = go on to the combat keys, 2 = skip to movement. */
static i32 combat_common(combat_ctx_t *c) {
    entity_t *p = g_player_entity;
    if (combat_gate()) return 0;                         /* sub_4512E4 */
    combat_hook_block(p, g_companion_entity);            /* process_combat_block */
    combat_hook_block(p, g_secondary_companion_entity);
    combat_end_action(p);                                /* end_entity_invulnerability(p, 0) */
    /* dword_4B70D8 debug: "dist %03ld" overlay (0x451EFF), skipped. */
    if (combat_damage_attempt(c->e1, p, c->d1)) return 0;
    if (combat_damage_attempt(c->e2, p, c->d2)) return 0;
    return 1;
}

/* ---------------------------------------------------------------------
 * sub_451BF9 @ 0x451BF9 -- default control scheme (dword_4B7494 == 0)
 *
 *   Out of stance:  Up walk (+Shift run), Down back, Left/Right turn,
 *                   Shift+Left/Right sidestep, Tab walk-set toggle,
 *                   'C' draw (enter stance).
 *   In stance (+0xD60): same movement (combat clips 1..5, sidestep 'c'
 *                   variant), plus with Ctrl held:
 *     Ctrl+Up    attack front  7 (close) / 10 (long) / fire (ranged)
 *     Ctrl+Right attack right  8 / 11
 *     Ctrl+Left  attack left   9 / 12
 *     Ctrl+Down  parry (sub_4515FD; 0x13 when nothing matches)
 *     Ctrl+Shift+Up special move (heroes 2/3: 0x1D->0x3C below 873 score,
 *                   else 0x17)
 *     Space      state 0x3E (not moving, heroes only)
 *     Shift / 0xE2 force long / close attacks (CD install only)
 *   Close vs long: the target is the opponent needing the smaller turn;
 *   close when its distance < +0xC5C. Ranged weapon: "long" means fire.
 * --------------------------------------------------------------------- */
void player_input_default(void) {
    entity_t *p = g_player_entity;
    if (!p || !EDESC(p) || DESC_HP(p) <= 0) return;
    combat_ctx_t c;
    ctx_init(&c);

    /* Look at the nearest opponent (0x451C7C). Note it passes the slot
     * globals, not the filtered opponents. */
    if (E32(p, 0xD60) && E32(p, 0xD04) == 0 && !g_player_movement_locked && !c.ranged) {
        if (c.d2 == -1)       combat_hook_head_track(p, g_companion_entity);
        else if (c.d1 == -1)  combat_hook_head_track(p, g_secondary_companion_entity);
        else if (c.d2 > c.d1) combat_hook_head_track(p, g_companion_entity);
        else                  combat_hook_head_track(p, g_secondary_companion_entity);
    }
    if (input_toggles()) return;
    if (!combat_common(&c)) return;

    if (st_attacking(p) && c.ranged) goto move;          /* 0x451FBB */
    if (E32(p, 0xCE8) == 0 && (st_attacking(p) || st_hurt(p) || st_parrying(p)))
        return;                                          /* busy until the clip ends
                                                            or message 0x20 opens +0xCE8 */
    if (!E32(p, 0xD60)) goto move;
    if ((c.e1 || c.e2) && st_hurt(c.e1) && st_hurt(c.e2)) goto move;

    if (key(VK_SPACE_) && E32(p, 0xD04) == 0 && E32(p, 0xCEC) == 0 &&
        E32(p, 0xCF4) == 0 && E32(p, 0xB0) >= 0 && E32(p, 0xB0) <= 3)
        E32(p, 0xD04) = 0x3E;
    if (!key(VK_CTRL_)) goto move;

    if (key(VK_SHIFT_) && key(VK_UP_)) {                 /* special move */
        i32 hero = actor_hero_id();
        if (E32(p, 0xD04) == 0 && (hero == 2 || hero == 3))
            E32(p, 0xD04) = (combat_hook_score_get() < 0x369) ? 0x1D : 0x17;
        goto post;
    }
    if (key(VK_DOWN_)) {                                 /* parry */
        if (combat_choose_parry(c.e1, c.d1)) {
            if (E32(p, 0xD04) == 0x13) combat_choose_parry(c.e2, c.d2);
        } else {
            E32(p, 0xD04) = 0x13;
        }
        goto post;
    }

    i32 kind = -1;                                       /* var_1C: 0 close, 3 long, 1 fire */
    if (!DKE_IS_USING_HDD && key(VK_OEM102_))     kind = 0;
    else if (!DKE_IS_USING_HDD && key(VK_SHIFT_)) kind = 3;
    else {
        i32 a1 = abs(yaw_to(c.e1)), a2 = abs(yaw_to(c.e2)), tgt;
        if (a1 == INT_MAX || a2 == INT_MAX) {
            if (a1 == INT_MAX && a2 == INT_MAX) tgt = c.dn;
            else tgt = (a1 == INT_MAX) ? c.d2 : c.d1;
        } else {
            tgt = (a2 > a1) ? c.d1 : c.d2;
        }
        if (tgt == -1) tgt = E32(p, 0xC5C);
        if (E32(p, 0xC5C) > tgt) kind = 0;
        else { kind = 3; if (c.ranged) kind = 1; }
    }
    if (key(VK_UP_) && kind != -1) {
        if (kind == 1) combat_fire(c.e1, c.e2);
        else E32(p, 0xD04) = kind + 7;
        goto post;
    }
    if (key(VK_LEFT_) && kind != -1 && kind != 1)  { E32(p, 0xD04) = kind + 9; goto post; }
    if (key(VK_RIGHT_) && kind != -1 && kind != 1) { E32(p, 0xD04) = kind + 8; goto post; }

move:                                                    /* 0x452509 */
    if (key(VK_RIGHT_) && key(VK_SHIFT_) && !key(VK_UP_) && !key(VK_DOWN_))      E32(p, 0xCEC) = 2;
    else if (key(VK_LEFT_) && key(VK_SHIFT_) && !key(VK_UP_) && !key(VK_DOWN_))  E32(p, 0xCEC) = -2;
    else if (key(VK_DOWN_)) { if (key(VK_CTRL_)) goto post; E32(p, 0xCEC) = -1; }
    else if (key(VK_UP_)) {
        E32(p, 0xD14) = 0;
        if (key(VK_CTRL_)) goto post;
        E32(p, 0xCEC) = key(VK_SHIFT_) ? 3 : 1;
    } else E32(p, 0xCEC) = 0;

    if (key(VK_RIGHT_)) {
        if (c.ranged && E32(p, 0xD60) && E32(p, 0xCEC) == 0) aim_turn(1);
        else { if (key(VK_CTRL_)) goto post; E32(p, 0xCF4) = 1; }
    } else if (key(VK_LEFT_)) {
        if (c.ranged && E32(p, 0xD60) && E32(p, 0xCEC) == 0) aim_turn(-1);
        else { if (key(VK_CTRL_)) goto post; E32(p, 0xCF4) = -1; }
    } else {
        E32(p, 0xCF4) = 0;
        if (E32(p, 0xD58) == 3 || E32(p, 0xD58) == 4) { E32(p, 0xCEC) = 0; E32(p, 0xCF0) = 1; }
    }

post:                                                    /* 0x4528FD: auto-face */
    if (E32(p, 0xD60) && E32(p, 0xCF4) == 0 && E32(p, 0xCEC) == 0 &&
        E32(p, 0xD58) != 0x1A && E32(p, 0xD58) != 3 && E32(p, 0xD58) != 4) {
        entity_t *t[2] = { c.e1, c.e2 };
        i32 d[2] = { c.d1, c.d2 };
        for (i32 i = 0; i < 2; i++) {
            if (d[i] != c.dn || !t[i]) continue;
            if (E32(t[i], 0xB0) == g_cinematic_mode || !E32(t[i], 0xD60)) continue;
            if (E32(t[i], 0x70) <= 0 || DESC_HP(t[i]) <= 0) continue;
            E32(p, 0xCF4) = face_half(t[i], d[i]);
        }
    }
}

/* ---------------------------------------------------------------------
 * sub_4342C7 @ 0x4342C7 -- alternate scheme (dword_4B7494 == 1)
 *   Ctrl is "fight": it turns toward the target, Up/Down still walk, and
 *   with no direction the game picks the move: parry when the target is in
 *   its strike window (or past half its clip) with attack 7..12 ->
 *   0x10..0x15; else random 7..9 inside +0xC50, random 10..12 inside
 *   [+0xC5C, +0xC60); ranged: 7 inside +0xC5C, else fire.
 *   Shift+Left/Right sidestep directions are swapped vs. the default.
 * --------------------------------------------------------------------- */
void player_input_alt(void) {
    entity_t *p = g_player_entity;
    if (!p || !EDESC(p) || DESC_HP(p) <= 0) return;
    combat_ctx_t c;
    ctx_init(&c);

    entity_t *tgt = NULL;                                /* var_24 / var_18 */
    i32 td = 0;
    if (c.ranged) {                                      /* smaller turn wins */
        if (c.e1 && !c.e2)      { tgt = c.e1; td = c.d1; }
        else if (!c.e1 && c.e2) { tgt = c.e2; td = c.d2; }
        else if (c.e1 && c.e2) {
            if (abs(yaw_to(c.e1)) < abs(yaw_to(c.e2))) { tgt = c.e1; td = c.d1; }
            else                                       { tgt = c.e2; td = c.d2; }
        }
    } else {                                             /* nearer wins, tie -> e2 */
        if (c.e1 && !c.e2)      { tgt = c.e1; td = c.d1; }
        else if (!c.e1 && c.e2) { tgt = c.e2; td = c.d2; }
        else if (c.e1 && c.e2) {
            if (c.d2 > c.d1) { tgt = c.e1; td = c.d1; } else { tgt = c.e2; td = c.d2; }
        }
    }
    if (E32(p, 0xD60) && E32(p, 0xD04) == 0 && !g_player_movement_locked) {
        if (c.d2 > c.d1) {
            if (!combat_hook_head_track(p, g_companion_entity))
                combat_hook_head_track(p, g_secondary_companion_entity);
        } else if (!combat_hook_head_track(p, g_secondary_companion_entity)) {
            combat_hook_head_track(p, g_companion_entity);
        }
    }
    if (input_toggles()) return;
    if (!combat_common(&c)) return;

    if (E32(p, 0xCE8) == 0 && (st_attacking(p) || st_hurt(p) || st_parrying(p))) return;
    if (!E32(p, 0xD60)) goto move;
    if ((c.e1 || c.e2) && st_hurt(c.e1) && st_hurt(c.e2)) goto move;
    if (key(VK_SPACE_) && E32(p, 0xD04) == 0 && E32(p, 0xCEC) == 0 &&
        E32(p, 0xCF4) == 0 && E32(p, 0xB0) >= 0 && E32(p, 0xB0) <= 3)
        E32(p, 0xD04) = 0x3E;
    if (!key(VK_CTRL_)) goto move;

    if (tgt) face_half_always(tgt);
    if (key(VK_UP_))   { E32(p, 0xD14) = 0; E32(p, 0xCEC) = 1; return; }
    if (key(VK_DOWN_)) { E32(p, 0xCEC) = -1; return; }
    if (E32(p, 0xCEC) != 0) { E32(p, 0xCEC) = 0; return; }
    if (!tgt || st_hurt(tgt) || E32(tgt, 0xD04) == 6) return;

    if (c.ranged && !(E32(tgt, 0xB0) == 0x28 && E32(tgt, 0xD70) != 0)) {
        if (E32(p, 0xC5C) > td) E32(p, 0xD04) = 7;
        else combat_fire(c.e1, c.e2);
        return;
    }
    {
        const can_rec_t *r = ANIMREC(tgt);
        i32 half = r ? CAN_DURATION(r) / 2 : 0;
        i32 ts = E32(tgt, 0xD04);
        if ((st_strike_window(tgt) || half < E32(tgt, 0xC40)) && ts >= 7 && ts <= 12) {
            static const i32 parry[6] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };
            E32(p, 0xD04) = parry[ts - 7];
            return;
        }
        if (E32(p, 0xC50) > td) E32(p, 0xD04) = 7 + (rand() % 3);          /* ls_random(3) */
        if (E32(p, 0xC5C) <= td && E32(p, 0xC60) > td) E32(p, 0xD04) = 10 + (rand() % 3);
        goto post;
    }

move:                                                    /* 0x434BD7 */
    if (key(VK_RIGHT_) && key(VK_SHIFT_) && !key(VK_UP_) && !key(VK_DOWN_))      E32(p, 0xCEC) = -2;
    else if (key(VK_LEFT_) && key(VK_SHIFT_) && !key(VK_UP_) && !key(VK_DOWN_))  E32(p, 0xCEC) = 2;
    else if (key(VK_DOWN_)) { if (key(VK_CTRL_)) goto post; E32(p, 0xCEC) = -1; }
    else if (key(VK_UP_)) {
        E32(p, 0xD14) = 0;
        if (key(VK_CTRL_)) goto post;
        E32(p, 0xCEC) = key(VK_SHIFT_) ? 3 : 1;
    } else E32(p, 0xCEC) = 0;

    if (key(VK_RIGHT_)) {
        if (key(VK_CTRL_)) goto post;
        E32(p, 0xCF4) = 1;
        if (c.ranged && E32(p, 0xD60) && E32(p, 0xCEC) == 0) {
            EROOT(p)->rot[2] += (now_ms() - E32(p, 0xE8)) * 30;
            if (E32(p, 0xD58) != 3 && E32(p, 0xD04) == 0) { play_state(p, 3); E32(p, 0xCF8) = 1; }
        }
    } else if (key(VK_LEFT_)) {
        if (key(VK_CTRL_)) goto post;
        E32(p, 0xCF4) = -1;
        if (c.ranged && E32(p, 0xD60) && E32(p, 0xCEC) == 0) {
            EROOT(p)->rot[2] -= (now_ms() - E32(p, 0xE8)) * 30;
            if (E32(p, 0xD58) != 4 && E32(p, 0xD04) == 0) { play_state(p, 4); E32(p, 0xCF8) = -1; }
        }
    } else {
        E32(p, 0xCF4) = 0;
    }

post:                                                    /* 0x434F98 */
    if (E32(p, 0xD60) && E32(p, 0xCF4) == 0 && E32(p, 0xCEC) == 0 && E32(p, 0xD58) != 0x1A &&
        tgt && E32(tgt, 0xB0) != g_cinematic_mode && E32(tgt, 0x70) > 0 && DESC_HP(tgt) > 0)
        E32(p, 0xCF4) = face_half(tgt, td);
}

/* sub_46642E @ 0x46642E, player half (0x466884..0x46693A).
 * INSERT INTO actor_move.c player_tick(): replace the body after the
 * +0xCBC early-out with:
 *
 *     if (g_player_movement_locked) { ...existing locked branch... }
 *     else player_input_tick();
 *
 * and drop the stand-in in scene.c (keep its DKE_AUTOWALK / DKE_AUTOTURN /
 * DKE_AUTOPATH overrides, applied after player_input_tick, or gate them on
 * getenv so headless tests keep working). */
void player_input_tick(void) {
    /* special_render_flags != 0 -> no input (0x466884).
     * dword_4B219C && D60 && !desc[0x96]: the companions' combat AI runs
     * with player_entity swapped (sub_4701E0), not ported. */
    if (g_control_scheme) player_input_alt();
    else                  player_input_default();
}

/* ========================================================================
 * 6. Held weapon -- hand bone attach / detach, take / holster
 *
 * Bone slots (CAN channel order, model_ome.c CAN_SLOT_NAMES):
 *   0x1A acces4 (2nd object slot, NPCs)   0x1B pacdro (right palm)
 *   0x1C pacgau (left palm)                0x1F arme   (= entity+0x178)
 * A weapon object's tree is  armea -> armem -> arme -> ...(mesh, armeb).
 * Holding it puts the object's "arme" node into bone slot 0x1F and makes
 * it the child of the palm bone, so:
 *   - the actor's renderer draws it (it is in the actor's child chain),
 *   - CAN channel 31 ("arme") drives its rotation per clip,
 *   - message 'e' (0x65) can re-parent it mid-clip like any bone.
 * entity+0x178 is not a separate field: 0xFC + 0x1F*4 == 0x178.
 * ======================================================================== */

/* sub_4373EF @ 0x4373EF: bone[child_slot] becomes the (only) child of
 * bone[parent_slot]. The parent's previous child link is overwritten. */
static void bone_link(entity_t *e, i32 parent_slot, i32 child_slot) {
    obc_node_t *par = EBONE(e, parent_slot), *ch = EBONE(e, child_slot);
    if (!par || !ch) return;
    par->child = ch;
    ch->parent = par;
}

/* sub_41F6BA @ 0x41F6BA: rebuild the 34 bone slots by node name, walking
 * the child/next chains from entity+0x50. Same walk as actor_create's
 * inline copy; export one of them. */
static void bones_rebind(entity_t *e) {
    for (i32 i = 0; i < ENTITY_BONE_SLOTS; i++) e->ptr[(0xFC >> 2) + i] = NULL;
    obc_node_t *stack[64];
    i32 sp = 0;
    obc_node_t *n = EROOT(e);
    for (;;) {
        while (n) {
            if (n->next && sp < 64) stack[sp++] = n->next;
            i32 slot = obc_node_type(n->name);
            if (slot >= 0 && slot < ENTITY_BONE_SLOTS) e->ptr[(0xFC >> 2) + slot] = n;
            n = n->child;
        }
        if (!sp) break;
        n = stack[--sp];
    }
}

void weapon_detach(entity_t *e, i32 slot);
static i32 s_hand_override = -1;          /* sub_47404E: the script's palm bone */

/* sub_473756 @ 0x473756 (ecx = actor, edx = object, arg0 = full). */
void weapon_attach(entity_t *e, game_object_t *o, i32 full) {
    obc_node_t *root = o ? o->root : NULL;
    if (full) {
        if (root) {
            EPTR(e, 0x178) = root->child ? root->child->child : root;   /* object "arme" */
            for (obc_node_t *n = root; n; n = n->child)                  /* grip at the palm */
                if (strcmp(n->name, "armeb") != 0) n->pos[0] = n->pos[1] = n->pos[2] = 0;
        }
        obc_node_t *w = (obc_node_t *)EPTR(e, 0x178);
        if (w) w->rot[0] = w->rot[1] = w->rot[2] = 0;
    } else {
        EPTR(e, 0x178) = root;
    }
    combat_hook_objects_flagbits(o);   /* name[1]=='B': byte_4DCCC0 bit 2 -> bit 4, else
                                          object +0xC4 &= ~2 (0x473863..0x473919) */
    /* sub_474731 (weapon_resolve_conflicts in scene.c): take it from any
     * other actor holding it, then: */
    EPTR(e, 0xD64) = o;
    if (s_hand_override >= 0) bone_link(e, s_hand_override, 0x1F);
    else if (full || e == g_player_entity) bone_link(e, 0x1B, 0x1F);     /* right palm */
    else                              bone_link(e, DESC_HAND(e), 0x1F);  /* persona hand */
}

/* LinkObjet with b != 0 (sub_47404E): `e` takes object `o` in the palm bone `hand`. */
void weapon_link(entity_t *e, game_object_t *o, i32 hand) {
    if (!e || !o || !o->root || !EDESC(e)) return;
    game_object_t *old = (game_object_t *)EPTR(e, 0xD64);
    if (old && old != o) {                                   /* sub_472B47(old) */
        weapon_detach(e, 0);
        EPTR(e, 0xD64) = NULL;
    }
    s_hand_override = hand;
    weapon_attach(e, o, 1);
    s_hand_override = -1;
    snprintf(ESTR(e, 0xD8), 8, "%s", o->name);               /* +0xD8 = (v6+168) copied to +216 */
}

/* LinkObjet with b == 0 (sub_474462): `e` lets go of what it holds. */
void weapon_unlink(entity_t *e) {
    if (!e) return;
    game_object_t *o = (game_object_t *)EPTR(e, 0xD64);
    if (!o) return;
    weapon_detach(e, 0);
    EPTR(e, 0xD64) = NULL;
    ESTR(e, 0xD8)[0] = 0;
}

/* sub_4735A2 @ 0x4735A2 (ecx = actor, edx = slot 0/1): give the object its
 * own "arme" node back. */
void weapon_detach(entity_t *e, i32 slot) {
    game_object_t *o = e ? (game_object_t *)EPTR(e, 0xD64 + slot * 4) : NULL;
    if (!o) return;
    i32 b = (slot == 0 || e == g_player_entity) ? 0x1F : 0x1A;
    obc_node_t *root = o->root;
    if (root && root->parent && strncmp(root->parent->name, "arme", 4) != 0) {
        root->parent->child = NULL;
        root->parent = NULL;
    }
    obc_node_t *bone = EBONE(e, b);
    if (!bone) return;
    if (E32(e, 0xE5C)) {                     /* shell casing hung under the muzzle */
        E32(e, 0xE5C) = 0;
        if (bone->child) bone->child->child = NULL;
    }
    if (bone->parent && strncmp(bone->parent->name, "arme", 4) != 0) {   /* off the palm */
        bone->parent->child = NULL;
        bone->parent = NULL;
    }
    if (root) {                              /* back under the object's "armem" */
        bone->parent = root->child;
        if (root->child) root->child->child = bone;
    }
}

/* sub_47398A @ 0x47398A: take object `o` in hand (inventory "use",
 * hotkeys, DIVERS 44, message 0x16/0x1E). Returns 1 on success. */
i32 weapon_take(game_object_t *o) {
    entity_t *p = g_player_entity;
    if (!o || !p || !EDESC(p)) goto fail;
    inv_set_equipped(o->name, 1);                        /* sub_426BB5(o, 1) */
    if (EPTR(p, 0xD64) != o) {
        if (EPTR(p, 0xD64)) {                            /* sub_472AF1: drop the old one */
            weapon_detach(p, 0);
            EPTR(p, 0xD64) = NULL;
            /* sub_472B47(old): world copy off */
        }
        /* sub_472FB5(o): load its tree (sub_472C5C) + 0x208 collision buffer;
         * object_get() already loads o->root. */
    }
    c8 c0, c1;
    i32 id = 0;
    sscanf(o->name, "%c%c%02d", &c0, &c1, &id);
    i32 old = DESC_WEAPON(p);
    DESC_WEAPON(p) = (u8)id;
    snprintf(ESTR(p, 0xD2), 8, "%s", o->name);
    i32 block = id;
    /* sub_472A48(o) (is a weapon): process_scene_root_node builds the
     * weapon's control points (collision spheres); else block = 0. */
    weapon_attach(p, o, 1);
    if (o->name[0] == 'Q') {
        /* 'Q' objects: the deepest "armeb" node is the projectile; it is cut
         * off and handed to current_entity (+0x50, +0x70 = 0, +0x74 = 2).
         * update_weapon_state() puts it back. TODO with projectiles. */
    }
    /* dword_4DD9B4 = 2 (inventory UI refresh) */
    if (old != id && (objets_flags(old) | 1) != (objets_flags(id) | 1))
        actor_desc_reload(p, block);                     /* sub_462BCC: new move set */
    if (E32(p, 0xD58) == 0) E32(p, 0xD58) = g_special_mode_active ? -5 : 59;
    if (E32(p, 0xD58) != 0) {                            /* replay the state from the new set */
        c8 clip[16] = {0};
        actor_bake_root_yaw(p);
        if (actor_prepare_anim(p, E32(p, 0xD58), clip) && strcmp(clip, ESTR(p, 0xC4)) != 0)
            actor_play_anim(p, can_get_anim(clip), 0);
    }
    if (ANIMREC(p) && idle_clip(p)) clip_force_end(p);
    g_held_weapon_id = DESC_WEAPON(p);
    g_held_variant = (DESC_WEAPON(p) && ESTR(p, 0xD6)[0]) ? ESTR(p, 0xD6)[0] - 'A' : 0;  /* sub_471B80 */
    {
        void *keep = EPTR(p, 0x178);
        bones_rebind(p);
        EPTR(p, 0x178) = keep;
    }
    return 1;
fail:
    if (p && EDESC(p)) {
        DESC_WEAPON(p) = 0;
        g_held_variant = g_held_weapon_id = 0;
        ESTR(p, 0xD2)[0] = 0;
        bones_rebind(p);
    }
    return 0;
}

/* sub_473DBC @ 0x473DBC (ecx = object, edx = mode).
 *   mode bit 1 (2): keep the world copy.
 *   mode != 0: reload the bare-hand move set when the old weapon had one.
 *   mode == 0: model removed but desc[0x97] keeps the old id. */
i32 weapon_holster(game_object_t *o, i32 mode) {
    entity_t *p = g_player_entity;
    if (!o || !p || !EDESC(p)) return 0;
    /* update_weapon_state @ 0x411BA9: projectile back under "arme". */
    inv_set_equipped(o->name, 0);
    weapon_detach(p, 0);
    EPTR(p, 0xD64) = NULL;
    i32 old = DESC_WEAPON(p);
    DESC_WEAPON(p) = 0;
    ESTR(p, 0xD2)[0] = 0;
    if (!(mode & 2)) object_world_remove(o->name);       /* sub_472B47 */
    i32 hb = DESC_HAND(p);
    if (hb && EBONE(p, hb)) EBONE(p, hb)->child = NULL;
    DESC_HAND(p) = 0;
    if (mode && old && (objets_flags(old) | 1) != 1)
        actor_desc_reload(p, 0);
    if (E32(p, 0xD58) != 0) {
        c8 clip[16] = {0};
        actor_bake_root_yaw(p);
        if (actor_prepare_anim(p, E32(p, 0xD58), clip) && strcmp(clip, ESTR(p, 0xC4)) != 0)
            actor_play_anim(p, can_get_anim(clip), 0);
    }
    if (idle_clip(p)) clip_force_end(p);
    g_held_variant = g_held_weapon_id = 0;
    if (mode == 0) DESC_WEAPON(p) = (u8)old;
    EPTR(p, 0x178) = NULL;
    return 1;
}

/* INSERT INTO inventory.c (replace the two TODO bodies at ~830):
 *
 * void inv_equip(const c8 *obj_name) {               // sub_47398A(sub_475BB6(e))
 *     if (!weapon_take(object_get(obj_name))) inv_set_equipped(obj_name, 0);
 *     g_ui_state = 2;
 * }
 * void inv_holster(const c8 *obj_name, i32 mode) {   // sub_473DBC
 *     game_object_t *o = object_get(obj_name);
 *     if (!o || !weapon_holster(o, mode)) {
 *         inv_set_equipped(obj_name, 0);
 *         if (!(mode & 2)) object_world_remove(obj_name);
 *     }
 * }
 */


/* sub_41CCB7 @ 0x41D3D8..0x41D778: aim the shoulder at the nearer foe
 * with a ranged weapon; keep the right palm still unless needed. */
void combat_aim_pose(entity_t *e) {
    entity_t *p = g_player_entity;
    if (e != p) return;
    if ((g_companion_entity || g_secondary_companion_entity) && E32(p, 0xD60) &&
        (g_held_weapon_id == 0x17 || g_held_weapon_id == 0x20 || g_held_weapon_id == 0x23) &&
        E32(p, 0xCEC) == 0) {
        /* Aim the right shoulder (slot 12 epadro, entity+0x12C) up/down at
         * the nearer live opponent. */
        f64 dist2[2] = { 1.0e100, 1.0e100 };
        entity_t *c[2] = { g_companion_entity, g_secondary_companion_entity };
        for (i32 i = 0; i < 2; i++) {
            if (!c[i] || !EDESC(c[i]) || EDESC(c[i])[0x96]) continue;
            f64 dx = EROOT(c[i])->pos[0] - EROOT(p)->pos[0];
            f64 dz = EROOT(c[i])->pos[2] - EROOT(p)->pos[2];
            dist2[i] = dz * dz + dx * dx;
        }
        /* 0x41D5AF: the secondary wins ties (sec <= comp). */
        i32 k = (dist2[1] <= dist2[0]) ? 1 : 0;
        entity_t *t = c[k];
        f64 best = dist2[k];
        if (t && best != 1.0e100 && EBONE(p, 12)) {
            f64 r = (f64)(EROOT(p)->pos[1] - EROOT(t)->pos[1]) / sqrt(best);
            f64 a = (1.0 - r * 1.5) * -16384.0;
            if (a > -8192.0) a = -12288.0;      /* sic: 0x41D69B */
            if (a < -16384.0) a = -16384.0;
            EBONE(p, 12)->rot[2] = (i32)a;
        }
    }
    /* Right palm (slot 27, +0x168) keeps a zero rotation unless the clip
     * has flag 0x8000 or the held object is a weapon (sub_47279D: flag 2). */
    obc_node_t *palm = EBONE(p, 27);
    if (palm && ANIMREC(p) && !(CAN_FLAGS(ANIMREC(p)) & 0x8000) &&
        !(objets_flags(DESC_WEAPON(p)) & 0x02))
        palm->rot[0] = palm->rot[1] = palm->rot[2] = 0;
}

