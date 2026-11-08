#ifndef DKE_INVENTORY_H
#define DKE_INVENTORY_H

/*
 * inventory.h — player inventory, OBJETS.CTX table, item EtatObjet states
 * and the native inventory screen (see inventory.c).
 */

#include "libsys/common.h"

/* dword_4DD9B4: 1 in game; 2 redraw pending; 10 back from a screen. The
 * scene steps it back to 1 (10 -> 2 -> 1). */
extern i32 g_ui_state;
extern i32 g_used_object;     /* dword_507258: item "used" from the screen */
extern i32 g_used_variant;    /* dword_505C34 */

/* sub_4723C0: OBJETS.CTX object table. */
i32  objets_ctx_load(const c8 *path);

/* OBJETS.CTX flags of object id (combat_ability_table[id*20]). */
u8   objets_flags(i32 id);
/* sub_42639A: add qty (pending = picked up, shown by the next screen). */
i32  inv_add(const c8 *obj_name, i32 qty, i32 pending);

typedef struct {
    c8  name[16];       /* action +0x30, 'S' prefix stripped */
    i32 a6;             /* +0x0C state; -3 becomes 3 in place */
    i32 a7;             /* +0x54 mode / quantity */
} etat_item_t;

/* sub_44A58B case 6, item states a6 <= -2 and 3: 1 = done, 0 = call again. */
i32  etat_objet_item(etat_item_t *a, i32 had_s_prefix);

/* sub_4654CE: take an item in hand by name ("OB00" = bare hands); add = 1
 * also gives it (DIVERS 44). */
void inv_use_by_name(const c8 *name, i32 add);
/* Hotkey slot k (0..8) -> stored item name ("" when free). */
const c8 *inv_hotkey(i32 k);
/* DIVERS op 5: weapon charge in percent (b == -1 reads, else sets). */
i32  inv_resource_pct(const c8 *name, i32 b);
/* sub_426A24: take one away (name = object name, letter at [4]). */
void inv_remove_one(c8 *obj_name);
/* sub_427816: held object first, linked objects grouped. */
void inv_sort(void);
/* sub_4727C7: may this item be drawn during a fight (level 35)? */
i32  inv_fight_usable(const c8 *name);
/* DIVERS op 7: sub_4274C2 wipe / sub_427593 confiscate / sub_4276B6 restore. */
void inv_clear_all(void);
/* sub_426D2C: re-hold the equipped object after a body swap. */
i32  inv_rehold(void);
i32  inv_confiscate(void);
void inv_restore(void);

/* sub_42701B: run the inventory screen (blocking), then fold new items in. */
void inventory_screen_open(void);

#endif /* DKE_INVENTORY_H */
