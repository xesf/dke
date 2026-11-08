#ifndef DKE_COMBAT_H
#define DKE_COMBAT_H

/*
 * combat.h — fights: the NPC combat brain (combat_ai.c, sub_4701E0), the
 * player's combat input (combat_player.c, sub_451BF9 / sub_4342C7) and
 * held weapons (sub_47398A / sub_473DBC).
 */

#include "libsys/common.h"
#include "entity.h"
#include "object.h"
#include "effect.h"
#include "libsys/system.h"
#include "libsys/graphics/graphic.h"

extern i32 g_combat_flags;      /* debug_flags: COMBAT mode bits */
extern i32 g_cam_lock;          /* dword_4FF0A0 */
extern i32 g_hero_autofight;    /* dword_4B219C */
extern i32 g_enemy_hits;        /* dword_506520 */

void combat_ai_tick(entity_t *e);       /* sub_4701E0: one fighter vs the player */
void combat_ai_cinematic(void);         /* sub_46642E: NPC vs NPC fights */
void combat_ai_hero_autofight(void);    /* dword_4B219C: the hero on autopilot */

/* combat_player.c */
void combat_keys_latch(const system_t *sys);   /* once per frame, before actor_frame */
void player_input_tick(void);                  /* sub_46642E player half */
void combat_aim_pose(entity_t *e);             /* sub_41CCB7 @ 0x41D3D8 */
i32  weapon_take(game_object_t *o);            /* sub_47398A */
i32  weapon_holster(game_object_t *o, i32 mode);   /* sub_473DBC */
void weapon_link(entity_t *e, game_object_t *o, i32 hand);   /* sub_47404E (LinkObjet b != 0) */
void weapon_unlink(entity_t *e);                              /* sub_474462 (LinkObjet b == 0) */
void weapon_detach(entity_t *e, i32 slot);         /* sub_4735A2: object off the hand */

/* combat_damage.c */
void combat_anim_message(entity_t *e, i32 type, i32 p);   /* sub_41AED1 0x11/0x15/0x19/0x24 */
void combat_enemy_bar(entity_t *e);
void combat_enemy_bar_draw(bmap_t *game_context);
void combat_enemy_bar_fb(u32 *fb, i32 fw, i32 fh);
void combat_regen_tick(void);                  /* update_game_progression */
void combat_projectile_reset(void);            /* level load: the flame projectile off */
void combat_projectile_tick(void);             /* process_entity_direction_and_state @ 0x411333 */
i32  combat_projectile_draw(effect_draw_t *out);   /* the flying fireball (OB54), or 0 */

#endif /* DKE_COMBAT_H */
