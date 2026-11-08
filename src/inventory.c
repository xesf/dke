/*
 * inventory.c — the player inventory (byte_4B3448, 135 x 0x1C), the
 * OBJETS.CTX object table, the EtatObjet item states, and a native rebuild
 * of INVENT.DLL's inventory screen (main @0x100041C0).
 *
 * Sources (dkev.exe unless noted):
 *   inventory table      byte_4B3448  135 x 0x1C            (sub_42639A..)
 *   item count           dword_4B3440
 *   screen category      dword_4B3444  0 objects / 1 weapons
 *   hotkey slots 1..9    byte_4B33F8 + 8*k (9 x char[8])     (sub_4268C9)
 *   confiscation backup  byte_50A160 (copy of table), dword_509B34 (count)
 *   OBJETS.CTX records   dword_4DB850 100 x 20               (sub_4723C0)
 *   resource_table       0x4DB590 35 x 10 i16 (+ copy word_4DC020)
 *   inventory screen     INVENT.DLL __LIB@8 @0x100016F0, main @0x100041C0
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "libsys/common.h"
#include "libsys/file_system.h"
#include "libsys/system.h"
#include "libsys/graphics/graphic.h"
#include "libsys/graphics/font.h"
#include "state.h"
#include "actor.h"
#include "object.h"
#include "model.h"
#include "model_ome.h"
#include "obc.h"
#include "game_init.h"
#include "sfx.h"
#include "screen_fx.h"
#include "inventory.h"
#include "combat.h"
#include "dialogue.h"
#include "lieu_cb.h"
#include "scene.h"
#include "pause.h"

i32 g_ui_state = 1;               /* dword_4DD9B4: 1 in game, 2 redraw, 10 back from a screen */
i32 g_used_object = 0;            /* dword_507258 */
i32 g_used_variant = 0;           /* dword_505C34 */

/* ======================================================================
 * 1. Data model
 * ====================================================================== */

#define INV_SLOTS        135     /* 0x87 */
#define INV_HOTKEYS      9       /* keys 1..9 */
#define OBJ_RECORDS      100
#define OBJ_RES_ROWS     35
#define OBJ_RES_COLS     10

/* Inventory entry, 0x1C bytes (byte_4B3448 + i*0x1C). */
typedef struct {
    c8  name[8];     /* +0x00 "OBnn" + variant letter, e.g. "OB44D". [0]==0 -> free.
                        The first 4 chars are the identity key (strncmp 4).
                        name[0] may be 'Q' instead of 'O' (sub_4267F8 marks the
                        held/quest copy). OB08 is stored as "OB08F".          */
    void *mat;       /* +0x08 INVENT.DLL cache: preview material (0x100075B0),
                        freed by 0x10007DC0; exe only clears it              */
    void *root;      /* +0x0C INVENT.DLL cache: preview hierarchy root
                        (obc_node_t); also the "already in a window" key     */
    u8  variants;    /* +0x10 bit (letter-'A') per held instance; OB08 sets 0x20 */
    u8  equipped;    /* +0x11 1 = in hand (sub_426BB5(obj,1))                 */
    u8  pad[2];
    i32 qty;         /* +0x14 count                                           */
    i32 pending;     /* +0x18 picked up, not yet acknowledged on the screen;
                        sub_42701B folds it into qty after the DLL returns.   */
} inv_entry_t;

/* OBJETS.CTX record, 20 bytes (dword_4DB850 + id*20). Field names are the
 * OBJETS.TXT keys parsed by sub_471C18. */
typedef struct {
    i32 nbobj;       /* +0x00 quantity given on pickup (EtatObjet -3/-4)     */
    i32 nbvie;       /* +0x04 */
    i32 ptvie;       /* +0x08 */
    i32 ptnrj;       /* +0x0C */
    u8  flags;       /* +0x10 combat_ability_table:
                        0x01 world-placeable, +0x11 is its instance slot
                        0x02 weapon: listed on the weapons page, confiscated
                             by sub_427593, sub_47279D(id) tests it
                        0x04/0x08/0x10/0x40/0x80 combat/use classes (unused here) */
    u8  slot;        /* +0x11 byte_4DB861 world instance slot (<35) when flags&1 */
    u8  arme;        /* +0x12 byte_4DB862 weapon row in resource_table        */
    u8  linke;       /* +0x13 byte_4DB863 linked object id (grouped next to it
                        in the list by sub_426E41; e.g. 23->72, 32->39, 35->73) */
} obj_record_t;

static inv_entry_t  g_inv[INV_SLOTS];          /* byte_4B3448 */
static i32          g_inv_count;               /* dword_4B3440 */
static i32          g_inv_category;            /* dword_4B3444 */
static c8           g_inv_hotkey[INV_HOTKEYS][8]; /* byte_4B33F8.. */
static inv_entry_t  g_inv_saved[INV_SLOTS];    /* byte_50A160 */
static i32          g_inv_saved_count;         /* dword_509B34 */

static obj_record_t g_obj_rec[OBJ_RECORDS];    /* dword_4DB850 */
static i16          g_obj_res[OBJ_RES_ROWS][OBJ_RES_COLS];      /* 0x4DB590 */
static i16          g_obj_res_init[OBJ_RES_ROWS][OBJ_RES_COLS]; /* word_4DC020 */

/* "%c%c%02d" -> object id (sub_47286D), 0 if not an OB/QB name. */
static i32 inv_name_id(const c8 *name) {
    i32 id = 0;
    c8 a, b;
    if (!name || (name[0] != 'O' && name[0] != 'Q')) return 0;
    if (sscanf(name, "%c%c%02d", &a, &b, &id) != 3) return 0;
    return id;
}

/* Variant bit index from the 5th char ('A'.. ), 0 when absent. */
static i32 inv_variant(c8 letter) {
    i32 v = (i32)letter - 'A';
    return (v == -'A') ? 0 : v;               /* asm: cmp v, 0FFFFFFBFh */
}

/* ----------------------------------------------------------------------
 * OBJETS.CTX (sub_4723C0). File: objets/OBJETS.CTX (2716 bytes)
 *   i32 version (must be 1), i32 filedate (checked against OBJETS.TXT
 *   only when the TXT is present), i32 700, i32 2000,
 *   i16 res[35][10]    (700 bytes)  -> resource_table and word_4DC020
 *   obj_record_t[100]  (2000 bytes) -> dword_4DB850
 * Returns 0 when the cache is missing/invalid (the exe then parses
 * OBJETS.TXT with sub_471C18; the shipped game has only the CTX).
 * ---------------------------------------------------------------------- */
i32 objets_ctx_load(const c8 *path) {
    FILE *f = fopen(path, "rb");
    i32 hdr[4];
    if (!f) return 0;
    if (fread(hdr, 4, 4, f) != 4 || hdr[0] != 1 || hdr[2] != 0x2BC || hdr[3] != 0x7D0) {
        fclose(f);
        return 0;
    }
    if (fread(g_obj_res, 1, 0x2BC, f) != 0x2BC ||
        fread(g_obj_rec, 1, 0x7D0, f) != 0x7D0) {   /* little-endian host */
        fclose(f);
        return 0;
    }
    memcpy(g_obj_res_init, g_obj_res, sizeof(g_obj_res));
    fclose(f);
    return 1;
}

static const obj_record_t *obj_rec(i32 id) {
    return (id >= 0 && id < OBJ_RECORDS) ? &g_obj_rec[id] : NULL;
}

/* ======================================================================
 * 2. Table primitives
 * ====================================================================== */

/* sub_426066: mark the instance letter of `name` as held. */
static void inv_set_variant(inv_entry_t *e, const c8 *name) {
    e->variants |= (u8)(1u << inv_variant(name[4]));
}

/* sub_4260CB: take one instance back out. The exe writes entry name[4]
 * into obj+0xAC, i.e. the object that leaves the inventory gets the
 * letter stored in the entry (the latest added instance). */
static void inv_clear_variant(inv_entry_t *e, c8 *obj_name_out) {
    c8 letter = e->name[4];
    if (obj_name_out) obj_name_out[4] = letter;
    e->variants &= (u8)~(1u << inv_variant(letter));
}

const c8 *inv_hotkey(i32 k) { return (k >= 0 && k < INV_HOTKEYS) ? g_inv_hotkey[k] : ""; }

/* sub_4727C7: anything but a plain weapon (OBJETS flags 2 without 4 / 8). */
i32 inv_fight_usable(const c8 *name) {
    if (!name || !name[0]) return 0;
    u8 f = objets_flags(inv_name_id(name));
    return !(f & 2) || (f & 4) || (f & 8);
}

/* sub_4268C9: drop `name` from every hotkey slot (4-char, case-insensitive). */
void inv_hotkey_forget(const c8 *name) {
    if (!name || !name[0]) return;
    for (i32 k = 0; k < INV_HOTKEYS; k++)
        if (strncasecmp(name, g_inv_hotkey[k], 4) == 0) g_inv_hotkey[k][0] = 0;
}

/* sub_426B24: find by 4-char key. */
inv_entry_t *inv_find(const c8 *key4) {
    for (i32 i = 0; i < INV_SLOTS; i++)
        if (g_inv[i].name[0] && strncmp(g_inv[i].name, key4, 4) == 0) return &g_inv[i];
    return NULL;
}

void inv_holster(const c8 *obj_name, i32 mode);   /* sub_473DBC, below */

/* The object in the player's hand (player+0xD64 -> +0xA8), or NULL. */
static const c8 *inv_held_name(void) {
    entity_t *p = g_player_entity;
    game_object_t *o = p ? (game_object_t *)EPTR(p, 0xD64) : NULL;
    return o ? o->name : NULL;
}

/* player desc[0x97]: id of the held object (kept by a mode-0 holster). */
static i32 inv_held_id(void) {
    entity_t *p = g_player_entity;
    return (p && EDESC(p)) ? EDESC(p)[0x97] : 0;
}

/*
 * sub_42614C: the instance (0..5) that leaves the entry when it is taken
 * in hand. An equipped entry keeps its letter; OB24 picks the instance with
 * the lowest positive script counter (dword_505D80[v], below 9); weapons
 * the one with the most charges (resource_table[arme][v]); anything else
 * the first instance held.
 */
static i32 inv_best_variant(const inv_entry_t *e) {
    if (!e) return 0;
    i32 id = inv_name_id(e->name);
    if (e->equipped) return e->name[4] ? (e->name[4] & 7) - 1 : 0;
    const obj_record_t *r = obj_rec(id);
    i32 best = 0;
    if (id == 24) {
        i32 lo = 9;
        for (i32 v = 0; v < 6; v++) {
            if (!(e->variants & (1 << v))) continue;
            i32 c = *lieu_state_slot(0x140 + v * 4);
            if (c > 0 && c < lo) { best = v; lo = c; }
        }
        return best;
    }
    if (r && (r->flags & 2)) {
        i32 hi = 0;
        for (i32 v = 0; v < 6; v++) {
            if (!(e->variants & (1 << v)) || r->arme >= OBJ_RES_ROWS) continue;
            if (g_obj_res[r->arme][v] > hi) { best = v; hi = g_obj_res[r->arme][v]; }
        }
        return best;
    }
    for (i32 v = 0; v < 6; v++) if (e->variants & (1 << v)) return v;
    return 0;
}

/*
 * sub_42639A(obj, qty, pending) -> 1 ok, 0 full / NULL.
 * obj_name = object name at obj+0xA8.
 *
 * Existing entry: lift it out, compact the table towards the END (entries
 * live packed at the high indices), and re-insert it at the highest free
 * index, i.e. just in front of the others: "move to front". This is also
 * how sub_427816 / sub_426E41 reorder the list (add with qty 0).
 * New entry: highest free index, count++.
 * qty != 0 also records the instance letter (OB08 instead sets bit 0x20).
 * pending != 0 adds to +0x18 and rewrites the name (keeps the new letter);
 * pending == 0 adds to +0x14 and keeps the stored name.
 */
i32 inv_add(const c8 *obj_name, i32 qty, i32 pending) {
    i32 i, j, dst = -1;
    inv_entry_t tmp;
    i32 is08;
    if (!obj_name) return 0;
    is08 = strncmp(obj_name, "OB08", 4) == 0;

    for (i = 0; i < INV_SLOTS; i++)
        if (g_inv[i].name[0] && strncmp(g_inv[i].name, obj_name, 4) == 0) break;

    if (i < INV_SLOTS) {
        tmp = g_inv[i];
        memset(&g_inv[i], 0, sizeof(g_inv[i]));
        /* compact: for each empty slot from the top, pull the nearest
         * non-empty slot below it up (asm loops 0x86..1 / j..0). */
        for (i = INV_SLOTS - 1; i >= 1; i--) {
            if (g_inv[i].name[0]) continue;
            for (j = i - 1; j >= 0; j--) {
                if (!g_inv[j].name[0]) continue;
                g_inv[i] = g_inv[j];
                memset(&g_inv[j], 0, sizeof(g_inv[j]));
                break;
            }
        }
        for (dst = INV_SLOTS - 1; dst >= 0 && g_inv[dst].name[0]; dst--) {}
        /* asm does not check dst<0 here: the entry it just freed guarantees one */
        g_inv[dst] = tmp;
        if (qty) {
            if (is08) g_inv[dst].variants |= 0x20;
            else      inv_set_variant(&g_inv[dst], obj_name);
        }
        if (pending) {
            g_inv[dst].pending += qty;
            snprintf(g_inv[dst].name, sizeof(g_inv[dst].name), "%s", is08 ? "OB08F" : obj_name);
        } else {
            g_inv[dst].qty += qty;
        }
        return 1;
    }

    for (i = INV_SLOTS - 1; i >= 0; i--)
        if (!g_inv[i].name[0]) { dst = i; break; }
    if (dst < 0) return 0;
    snprintf(g_inv[dst].name, sizeof(g_inv[dst].name), "%s", is08 ? "OB08F" : obj_name);
    if (qty) {
        if (is08) g_inv[dst].variants |= 0x20;
        else      inv_set_variant(&g_inv[dst], obj_name);
    }
    if (pending) g_inv[dst].pending += qty;
    else         g_inv[dst].qty += qty;
    g_inv_count++;
    return 1;
}

/*
 * sub_426A24(obj): remove ONE. Restores the instance letter into the
 * object's name (obj+0xAC), then qty-- or, at qty <= 1, forget hotkeys,
 * count--, wipe the entry (no compaction).
 */
void inv_remove_one(c8 *obj_name /* obj+0xA8, letter rewritten */) {
    if (!obj_name) return;
    for (i32 i = 0; i < INV_SLOTS; i++) {
        inv_entry_t *e = &g_inv[i];
        if (!e->name[0] || strncmp(e->name, obj_name, 4) != 0) continue;
        inv_clear_variant(e, obj_name);
        if (e->qty > 1) {
            e->qty--;
        } else {
            inv_hotkey_forget(e->name);
            g_inv_count--;
            memset(e, 0, sizeof(*e));
        }
        return;
    }
}

/*
 * DIVERS op 5 (0x446E19): charge of an object's weapon row in resource_table,
 * as a percentage of the full charge. b == -1 reads it (0 when the row has
 * no maximum), otherwise sets it to b percent. name = the object name with
 * its instance letter at [4]. Returns the percentage, 0 when it has no row.
 */
i32 inv_resource_pct(const c8 *name, i32 b) {
    i32 id = inv_name_id(name);
    const obj_record_t *r = id ? obj_rec(id) : NULL;
    if (!r || !(r->flags & 2)) return 0;
    i32 k = r->arme, v = name[4] ? name[4] - 'A' : 0;
    if (k <= 0 || k >= OBJ_RES_ROWS || v < 0 || v >= OBJ_RES_COLS) return 0;
    i32 mx = g_obj_res_init[k][v];
    if (b != -1) { g_obj_res[k][v] = (i16)(b * mx / 100); return 0; }
    return mx ? g_obj_res[k][v] * 100 / mx : 0;
}

/*
 * sub_426BB5(obj, on): set/clear the "equipped" byte of the matching
 * entry. When equipping it also clears world-instance bit 0x02 of the
 * entity's slot/variant (byte_4DCCC0[slot*6+v] &= ~2) or, without a slot,
 * entity+0xC4 &= ~2 (the in-world copy stops being "placed").
 * Returns 1 if an entry was equipped.
 */
i32 inv_set_equipped(const c8 *obj_name, i32 on) {
    i32 hit = 0;
    for (i32 i = 0; i < INV_SLOTS; i++) {
        inv_entry_t *e = &g_inv[i];
        if (!e->name[0] || strncmp(e->name, obj_name, 4) != 0) continue;
        e->equipped = on ? 1 : 0;
        if (on) {
            hit = 1;
            /* TODO engine: ent = get_entity_character(e->name);
             *   slot = sub_4728DF(ent) (= rec.slot when rec.flags&1);
             *   v = ent->name[4]-'A';
             *   if (slot>0 && slot<35 && v>=0) byte_4DCCC0[slot*6+v] &= ~2;
             *   else ent->flags_C4 &= ~2; */
        }
    }
    return hit;
}

/* sub_427816: put the held object (player+0xD64) first, regroup linked
 * objects (sub_426E41), then put the held object first again. */
static void inv_regroup_linked(void);
void inv_sort(void) {
    const c8 *held_name = inv_held_name();
    if (held_name) inv_add(held_name, 0, 0);
    inv_regroup_linked();
    if (held_name) inv_add(held_name, 0, 0);
}

/* sub_426E41: walk a snapshot from index 134 down; for each "OBnn" whose
 * OBJETS.linke != 0 and != nn, move "OB<linke>" to the front first (once
 * per linked id; linke==7 also pulls "OB08"), then move the entry itself
 * to the front. Net effect: newest-first order with linked pairs adjacent. */
static void inv_regroup_linked(void) {
    inv_entry_t snap[INV_SLOTS];
    u8 done[INV_SLOTS] = { 0 };
    memcpy(snap, g_inv, sizeof(snap));
    for (i32 i = INV_SLOTS - 1; i >= 0; i--) {
        if (!snap[i].name[0]) continue;
        i32 id = inv_name_id(snap[i].name);
        if (id < 0 || id >= INV_SLOTS || done[id]) continue;
        const obj_record_t *r = obj_rec(id);
        u8 l = r ? r->linke : 0;
        if (l && l != id) {
            c8 nm[8];
            snprintf(nm, sizeof(nm), "OB%02d", l);
            inv_add(nm, 0, 0);
            done[l] = 1;
            if (l == 7) inv_add("OB08", 0, 0);   /* asm sets a flag at +8 of done[] */
        }
        inv_add(snap[i].name, 0, 0);
    }
}

/* sub_4274C2: wipe the inventory (new game). Holsters the held object
 * first (sub_473DBC(held,1)). NOTE: index 0 is never cleared (loop from 1),
 * kept for fidelity. */
void inv_clear_all(void) {
    if (inv_held_name()) inv_holster(inv_held_name(), 1);   /* sub_473DBC(held, 1) */
    g_inv_count = 0;
    for (i32 i = 1; i < INV_SLOTS; i++) {
        inv_entry_t *e = &g_inv[i];
        if (e->name[0]) inv_hotkey_forget(e->name);
        e->name[0] = 0;                        /* strcpy(byte_4B43D8 = "") */
        e->mat = NULL; e->root = NULL; e->variants = 0; e->equipped = 0; e->qty = 0;
        /* +0x18 pending is NOT cleared by the exe */
    }
}

/* sub_427593 (DIVERS/layer_op): confiscate weapons (prison). Saves a full
 * copy, removes every entry whose OBJETS flags&2, returns how many. */
i32 inv_confiscate(void) {
    i32 n = 0;
    if (inv_held_name()) inv_holster(inv_held_name(), 1);
    memcpy(g_inv_saved, g_inv, sizeof(g_inv));
    g_inv_saved_count = g_inv_count;
    for (i32 i = 1; i < INV_SLOTS; i++) {
        inv_entry_t *e = &g_inv[i];
        if (!e->name[0]) continue;
        i32 id = inv_name_id(e->name);
        const obj_record_t *r = obj_rec(id);
        if (!r || !(r->flags & 0x02)) continue;
        inv_hotkey_forget(e->name);
        n++;
        g_inv_count--;
        e->name[0] = 0;                        /* strcpy(byte_4B43E8 = "") */
        e->mat = NULL; e->root = NULL; e->variants = 0; e->equipped = 0; e->qty = 0;
    }
    return n;
}

/* sub_4276B6: give back everything from the saved copy that is not in the
 * table now (by 4-char key) into the highest free slots, count +=
 * saved_count (sic: adds the whole saved count), category = weapons, then
 * opens the inventory screen and requests a full redraw. */
void inventory_screen_open(void);
extern i32 g_ui_state;                         /* dword_4DD9B4 */
void inv_restore(void) {
    for (i32 s = INV_SLOTS - 1; s >= 0; s--) {
        if (!g_inv_saved[s].name[0]) continue;
        if (inv_find(g_inv_saved[s].name)) continue;
        for (i32 d = INV_SLOTS - 1; d >= 0; d--)
            if (!g_inv[d].name[0]) { g_inv[d] = g_inv_saved[s]; break; }
    }
    g_inv_count += g_inv_saved_count;
    g_inv_category = 1;
    inventory_screen_open();
    g_ui_state = 2;
}

/* ======================================================================
 * 3. Equip / holster (engine side, called by the screen and scripts)
 * ====================================================================== */

/*
 * sub_47398A(obj) "take in hand":
 *   inv_set_equipped(obj,1); if player holds another object -> drop it
 *   (sub_472AF1: holster + remove world copy); load obj (sub_472FB5);
 *   player->desc[0x97] = obj id; strcpy(player+0xD2, obj name);
 *   sub_473756(player, obj, 1) attaches it to the hand bone (+0xD64 = obj);
 *   'Q' names re-parent the "armeb" node; g_ui_state = 2;
 *   if the weapon class (OBJETS.flags|1) changed -> sub_462BCC (stance);
 *   restart the idle anim (+0xD58 = 0x3B, or -5 in special mode).
 * sub_473DBC(obj, mode) "holster":
 *   update_weapon_state; inv_set_equipped(obj,0); sub_4735A2(player,0)
 *   (detach); player+0xD64 = 0; desc[0x97] = 0; player+0xD2 = "";
 *   if !(mode & 2) remove the world copy (sub_472B47);
 *   if mode && old id's class != unarmed -> sub_462BCC; reset anim;
 *   if mode == 0 restore desc[0x97] = old id.
 * sub_4654CE(name, add) "use by name" (hotkeys, DIVERS op 44):
 *   "OB00" -> holster; else find/create the object entity (sub_475BB6),
 *   add it first when `add` and missing (sub_42639A(obj, nbobj, 0)), then
 *   sub_47398A. Specials: OB24 / OB77 / OB86 / OB71 (0x47).
 */
void inv_equip(const c8 *obj_name);            /* sub_47398A, below */

/* ======================================================================
 * 4. EtatObjet item states (sub_44A58B case 6 @ 0x44B00F, inner switch
 *    jpt_44B3F3 on a6+6). Returns 1 when the script action completes,
 *    0 to be called again next frame.
 * ====================================================================== */



i32 player_in_anim(void);                      /* is_entity_in_animation(player) */
void obj_world_remove(const c8 *obj_name);     /* sub_472B47 */
void obj_world_drop(const c8 *obj_name);       /* sub_472AF1: holster if held, then sub_472B47 */
void obj_world_place(const c8 *obj_name);      /* sub_472FB5 + sub_4731C6 */
const c8 *player_held_id2(void);               /* player+0xD4: 2 digits of held name */

i32 etat_objet_item(etat_item_t *a, i32 had_s_prefix) {
    /* Target: a6 <= -3 -> existing entity (get_entity_character), else
     * get/create object slot (sub_475BB6); fallback dummy byte_4DB3C0
     * with the name and +0xC4 &= ~0x20. a6 >= -1 also copies pos/rot/mode. */
    i32 nbobj = 1;
    const obj_record_t *r = obj_rec(inv_name_id(a->name));
    if (r) nbobj = r->nbobj;

    /* Inventory states wait for the player's current clip (e.g. SG00PRE07). */
    if (!had_s_prefix && a->a6 <= -2 && player_in_anim()) return 0;

    switch (a->a6) {
    case -6:                                   /* remove from the world */
        if (strncmp(a->name + 2, player_held_id2(), 2) == 0) inv_set_equipped(a->name, 0);
        obj_world_drop(a->name);
        return 1;
    case -5:                                   /* give a7 pieces */
        inv_sort();
        inv_add(a->name, a->a7, 1);
        return 1;
    case -4:                                   /* give nbobj pieces, take it out of the world */
        inv_sort();
        inv_add(a->name, nbobj, 1);
        obj_world_drop(a->name);
        return 1;
    case -3:                                   /* pickup, phase 1 */
        a->a6 = 3;
        inv_sort();
        inv_add(a->name, nbobj, 1);            /* pending -> screen opens in "new items" mode */
        obj_world_remove(a->name);
        g_ui_state = 2;                        /* full redraw without the object */
        return 0;
    case 3:                                    /* pickup, phase 2 (table index 9): once the redraw settled */
        if (g_ui_state != 1) return 0;
        inventory_screen_open();               /* sub_42701B */
        return 1;
    case -2:                                   /* take one out: back in hand or dropped */
        inv_remove_one(a->name);
        if (strncmp(a->name + 2, player_held_id2(), 2) == 0) inv_holster(a->name, 1);
        else obj_world_drop(a->name);
        g_ui_state = 2;
        return 1;
    case -1:                                   /* place in world */
        obj_world_place(a->name);
        return 1;
    case 0:
        obj_world_place(a->name);
        g_ui_state = 2;
        return 1;
    default:                                   /* 1, 2: nothing */
        return 1;
    }
}

/* ======================================================================
 * 5. Opening the screen: sub_42701B (exe side wrapper)
 * ====================================================================== */

/* sub_42784E: automatic combinations before the screen opens, unless the
 * player holds one of the parts (desc[0x97] = held id):
 *   OB84 + OB85 + OB86 present -> remove OB84, OB85 (one each), add OB71
 *     (qty 1, pending), dword_506854 = 3.
 *   OB02 + OB12 + OB18 + OB19 present -> remove all four, add OB75
 *     (pending), dword_506750 = 2.
 * Skipped entirely while any entry still has pending > 0. */
void inv_auto_combine(i32 held_id);

void inventory_screen_run(void);               /* the DLL, rebuilt below */

void inventory_screen_open(void) {             /* sub_42701B */
    /* Test: DKE_GIVE="OB01A,OB11A,..." puts those items in the inventory. */
    if (getenv("DKE_GIVE")) {
        c8 list[256]; snprintf(list, sizeof(list), "%s", getenv("DKE_GIVE"));
        for (c8 *t = strtok(list, ","); t; t = strtok(NULL, ",")) inv_add(t, 1, getenv("DKE_GIVE_NEW") ? 1 : 0);
#ifdef _WIN32
        _putenv("DKE_GIVE=");
#else
        unsetenv("DKE_GIVE");
#endif
    }
    /* dword_4B33F0 = 1; sub_416C10(2); */
    game_time_freeze(0x19);                    /* set_display_mode(0x19) */
    inv_sort();
    inv_auto_combine(inv_held_id());
    /* sub_44DEC9(player): stop the player's sounds */
    inventory_screen_run();                    /* __LIB@8(lib, &unk_4ED9C0) */
    if (g_used_object) *lieu_state_slot(0x128) = g_used_variant;   /* dword_505D68 */
    for (i32 i = 0; i < INV_SLOTS; i++) {      /* acknowledge new items */
        g_inv[i].qty += g_inv[i].pending;
        g_inv[i].pending = 0;
    }
    inv_sort();
    /* restore palette/theme; if player +0xD58 is 0 or >= 1000: set idle anim
     * (0x3B, -5 in special mode) and restart it */
    game_time_thaw(1, 0x19);                   /* restore_display_mode(1, 0x19) */
    g_ui_state = 10;                           /* -> 2 next frame: full redraw */
}

/* ======================================================================
 * 6. Native INVENT.DLL screen (main @0x100041C0)
 *
 * 640x480, composed into an off-screen "ds" (debug_surface) and flushed to
 * the screen; "rb2" (render_buffer2) is a 640x480 sprite atlas + save area.
 * Coordinates are inclusive (draw_image src rect x1,y1..x2,y2).
 *
 * Layout
 *   background   mode 0 INVENT11, mode 1 INVENT12 (DATAS>GLOBAL>IMAGES,
 *                .CIF preferred), then INVENT10 overlaid colour-keyed.
 *                mode 2 (new pickups): the frozen game view darkened with
 *                INVMASK.CTX (shade 0x2E + 8 px inner shadow 0x10).
 *   atlas        OBJETS1.GIF -> rb2 (0,0), then sub-copies (see init).
 *   item windows 4 x 160x160 at (154,43) (348,43) (154,227) (348,227),
 *                slots k=1..4 showing 4 consecutive listable items as
 *                spinning 3D models (OBJETS>OBnn.OME/COB/OBC + MAT).
 *   name list    (44,119)-(123,375): up to 20 OBJNAME lines, 9 px, x=49,
 *                first y=160; scroll thumb at x=14, y 122..301.
 *   description  (194,430) 240x41: OBJDESCR[num] at (202,438);
 *                'Q' items add OBJDESCR[100] at (202,450).
 *   buttons      equip  (527,77)-(603,189)   [sprite rb2 (79,0)-(155,112)]
 *                use    (529,244)-(606,375)  [rb2 (0,0)-(77,131)]
 *                exit   (519,411)-(620,465)  [rb2 (0,133)-(101,187)]
 *                tabs   objects (35,386)-(99,450), weapons (60,406)-(124,470)
 *                arrows up (324,362)-(338,388), down (324,394)-(340,420)
 *   badges       qty box at (X+113..129, Y+127), text font -9;
 *                hotkey digit rb2 (480+12m,252) at (X+20,Y+20);
 *                ammo icon (X+120,Y+20) + "%+3s" at (X+122,Y+40).
 *   spinners     anim A 9 frames at (588,57), anim B 8 frames at (596,244).
 *
 * Input (VK codes; "press" = edge)
 *   Esc / Backspace / 'I' / exit button / right mouse  -> close
 *   F1 or tab click                -> toggle objects/weapons page
 *   Left/Right                     -> select previous/next window (scrolls
 *                                     the 4-window strip at the ends)
 *   Up/Down                        -> move by 2 (row), PgUp/PgDn -> page of 4
 *   Space or click equip button    -> equip/unequip (0x10008510) unless the
 *                                     item is OB71 or (world 0x23 and
 *                                     usable) -> use (0x10008730)
 *   Enter or click use button      -> use (0x10008730)
 *   1..9                           -> assign selected item to hotkey k
 *   0                              -> remove it from all hotkeys
 *   click a window                 -> select it; list row click -> jump
 *   debug only: Q/A ratio, numpad 1-3 angles, X axis, T screenshot,
 *               L/M layout reload; dirty tuning saved to INVENT2.CTX.
 *
 * Results written back: inventory table (equipped flags, name[4] variant,
 * hotkeys), dword_4B3444 (page), dword_507258 = used object id,
 * dword_505C34 = used variant; the player's hand via sub_47398A/sub_473DBC.
 * ====================================================================== */


enum { INV_MODE_OBJECTS = 0, INV_MODE_WEAPONS = 1, INV_MODE_PICKUP = 2, INV_MODE_ALL = 3 };

typedef struct {
    i32 inv_index;                             /* model root +0x48 */
    i32 framed;                                /* +0x4C equipped frame */
    i32 qty;                                   /* +0x50 shown quantity */
    i32 ang[3];                                /* displayed angles (0x100151C8) */
    i32 rest[3];                               /* rest angles (0x10013A98) */
    void *root;                                /* inv[].root */
} inv_window_t;

typedef struct {
    /* resources */
    bmap_t *ds, *rb2;
    u16    *invmask;                           /* INVMASK.CTX: 480 rows of (skip,run) u16 pairs, each row sums to 640 */
    u8      grd[0x4000];                       /* INVENT.GRD: 256 colours x 64 shade levels */
    c8    **objname, **objdescr;               /* OBJNAME.TXT / OBJDESCR.TXT ("@n" headers) */
    /* INVENT.CTX (2100 B) or INVENT2.CTX (2900 B, preferred if present) */
    f64     ratio[200];                        /* 100 (CTX) or 200 (CTX2) doubles; index variant*id, variant = 'Q' ? 2 : 1 */
    i32     rest_ang[100][3];                  /* per id rest rotation (BAM, 0x10000 = 360 deg) */
    i8      axis[100];                         /* spin axes: bit0 X, bit1 Y, bit2 Z; sign = direction */
    /* state */
    i32     mode;                              /* 0x10012044 */
    i32     count;                             /* 0x10013B64 listable entries */
    i32     scroll;                            /* 0x10013998 */
    i32     sel;                               /* 0x100139F4, window 1..4 */
    i32     dirty;                             /* 0x10014A5C bits 1..4 */
    i32     exit;                              /* 0x10012040 */
    i32     open_ctr;                          /* 0x1001204C, 0x46 at open, wipe-in at <=10 */
    i32     thumb_y;                           /* 0x10015244, 122..301 */
    i32     button_hl;                         /* 0x100139C0: 1 equip, 2 use, 3 exit */
    i32     menu_ctr;                          /* 0x10013B6C button press animation 0..16 */
    inv_window_t win[5];                       /* 1..4 */
    u32     prev_ms;
} inv_screen_t;

static inv_screen_t S;

/* 0x10009580: is inventory entry idx hidden on the current page? */
static i32 inv_item_hidden(i32 idx) {
    const inv_entry_t *e = &g_inv[idx];
    i32 id = inv_name_id(e->name), hidden = 0;
    const obj_record_t *r = obj_rec(id);
    i32 weapon = r ? (r->flags & 2) != 0 : 0;
    i32 special = (id == 6 || id == 7 || id == 8 || id == 0x27 || id == 0x48 || id == 0x49);
    if (special) weapon = 1;                   /* ammo lives on the weapons page */
    switch (S.mode) {
    case INV_MODE_WEAPONS: hidden = !weapon; break;
    case INV_MODE_OBJECTS: hidden = weapon; break;
    case INV_MODE_PICKUP:  hidden = (e->pending == 0); break;
    default:               hidden = 0; break;
    }
    if (id == 0x56 && ((state && state->current_level_id == 0x23) ||
                       (g_player_entity && E32(g_player_entity, 0xC78))))
        hidden = 1;                            /* OB86 not in level 35 / while +0xC78 is set */
    if (e->qty == 0 && e->pending == 0) hidden = 1;
    if (!hidden && !e->equipped && special) {
        /* ammo is shown on its weapon's window instead when that weapon is held */
        const c8 *w = id == 6 ? "OB27" : (id == 7 || id == 8) ? "OB55" :
                      id == 0x27 ? "OB32" : id == 0x48 ? "OB23" : "OB35";
        inv_entry_t *we = inv_find(w);
        hidden = (S.mode == INV_MODE_PICKUP) ? (we && we->pending) : (we != NULL);
    }
    return hidden;
}

/* 0x10009430: count listable entries; pick the page. */
static void inv_count_visible(i32 first_open) {
    S.count = 0;
    for (i32 i = 0; i < INV_SLOTS; i++)
        if (g_inv[i].name[0] && g_inv[i].pending) { S.mode = INV_MODE_PICKUP; S.count++; }
    if (S.mode == INV_MODE_PICKUP) return;
    if (first_open) {
        for (i32 i = 0; i < INV_SLOTS; i++) {
            if (!g_inv[i].name[0] || !g_inv[i].equipped) continue;
            const obj_record_t *r = obj_rec(inv_name_id(g_inv[i].name));
            g_inv_category = (r && (r->flags & 2)) ? 1 : 0;
            break;
        }
    }
    for (i32 pass = 0; pass <= first_open; pass++) {
        S.mode = g_inv_category;
        for (i32 i = 0; i < INV_SLOTS; i++)
            if (g_inv[i].name[0] && !inv_item_hidden(i)) S.count++;
        if (S.count) return;
        g_inv_category ^= 1;                   /* empty page: flip once on open */
    }
}

/* 0x10003CA0 */
static i32 inv_nth_listable(i32 n) {
    for (i32 i = 0; i < INV_SLOTS; i++)
        if (g_inv[i].name[0] && !inv_item_hidden(i) && n-- == 0) return i;
    return -1;
}

/* Fill windows 1..4 with the 4 listable entries starting at list rank
 * `first` (the DLL does this incrementally with slide animations
 * 0x10014A88 = 1..6 and an LRU of 4 model heaps; a native port can load
 * on demand). */
static void inv_fill_windows(i32 first) {
    for (i32 k = 1; k <= 4; k++) {
        inv_window_t *w = &S.win[k];
        i32 idx = inv_nth_listable(first + k - 1);
        memset(w, 0, sizeof(*w));
        w->inv_index = idx;
        if (idx < 0) continue;
        inv_entry_t *e = &g_inv[idx];
        i32 id = inv_name_id(e->name);
        w->framed = e->equipped;
        w->qty = (S.mode == INV_MODE_PICKUP) ? e->pending : e->qty;
        if (id >= 0 && id < 100) memcpy(w->rest, S.rest_ang[id], sizeof(w->rest));
        memcpy(w->ang, w->rest, sizeof(w->ang));
        /* TODO engine: 0x100075B0 load OBJETS>%.4s.OME (or .COB / .OBC) and
         * #OBJETS>%.4s>%.4s.MAT; centre on bbox (0x10007B30); camera eye
         * distance = (bbox depth << 14) / 0xA00 * ratio[variant*id], doubled
         * when the child node is "armeb" (0x10007430). */
        S.dirty |= 1 << k;
    }
}

/* 0x10008510: equip / unequip the selected window's item. */
static void inv_toggle_equip(void) {
    inv_window_t *w = &S.win[S.sel];
    if (w->inv_index < 0) return;
    inv_entry_t *e = &g_inv[w->inv_index];
    for (i32 i = 0; i < INV_SLOTS; i++) {      /* only one item in hand */
        if (i == w->inv_index && w->framed) continue;
        if (g_inv[i].equipped) { inv_holster(g_inv[i].name, 0); g_inv[i].equipped = 0; }
    }
    for (i32 k = 1; k <= 4; k++) if (S.win[k].framed) { S.win[k].framed = 0; S.dirty |= 1 << k; }
    if (!e->equipped) {
        i32 v;
        if (e->pending) {
            v = e->name[4] ? e->name[4] - 'A' : 0;
            const obj_record_t *r = obj_rec(inv_name_id(e->name));
            g_inv_category = (r && (r->flags & 2)) ? 1 : 0;
        } else {
            v = inv_best_variant(e);           /* sub_42614C */
        }
        e->name[4] = (c8)('A' + v);
        e->equipped = 1;
        inv_equip(e->name);                    /* sub_47398A(sub_475BB6(e)) */
    } else {
        e->equipped = 0;
        inv_holster(e->name, 1);               /* sub_473DBC(get_entity_character(e), 1) */
    }
    w->framed = e->equipped;
    w->qty = (S.mode == INV_MODE_PICKUP) ? e->pending : e->qty;
    S.exit = 1;                                /* equipping closes the screen */
    S.dirty |= 1 << S.sel;
}

extern i32 g_used_object;                      /* dword_507258 */
extern i32 g_used_variant;                     /* dword_505C34 */

/* 0x100099F0: OBJETS flags & 0xF0 */
static i32 inv_usable(i32 id) { const obj_record_t *r = obj_rec(id); return r && (r->flags & 0xF0); }

/* 0x10008730, OB52: RYLIVRE<language>.GIF full screen until a key or a
 * click (there is no RYLIVRE1, GB reads the US page). */
static void inv_show_book(void) {
    if (!state || !state->graphic.front_buffer) return;
    i32 lang = state->language_id;
    bmap_t *img = NULL;
    for (i32 t = 0; t < 3 && !img; t++) {
        c8 vp[64];
        snprintf(vp, sizeof(vp), "DATAS>GLOBAL>IMAGES>RYLIVRE%01d.CIF", t == 0 ? lang : t == 1 ? 2 : 0);
        img = game_init_load_cursor(vp);
    }
    if (!img) return;
    system_t *sys = &state->system;
    i32 fw = state->graphic.width, fh = state->graphic.height;
    i32 prev[256];
    memcpy(prev, sys->actions, sizeof(prev));
    u32 t0 = ls_tick();
    while (!sys->quit) {
        ls_update_events(sys);
        sfx_pump();
        i32 hit = sys->mouse_left == 2;
        for (i32 k = 0; k < 256 && !hit; k++) if (sys->actions[k] && !prev[k]) hit = 1;
        memcpy(prev, sys->actions, sizeof(prev));
        if ((getenv("DKE_INVAUTO") || getenv("DKE_FRAMES")) && ls_tick() - t0 > 1000) hit = 1;
        blit_bmap_to_framebuffer(state->graphic.front_buffer, fw, fh, img, img->palette, 0, 0);
        fx_present();
        if (hit) break;
        ls_delay(10);
    }
    ls_free_bmap(img);
}

/* 0x10008730: use the selected item on the world (script reads it). */
static void inv_use(void) {
    inv_window_t *w = &S.win[S.sel];
    if (w->inv_index < 0) return;
    inv_entry_t *e = &g_inv[w->inv_index];
    i32 id = inv_name_id(e->name);
    if (id == 52) { inv_show_book(); return; } /* the book */
    g_used_object = id;
    g_used_variant = e->pending ? (e->name[4] ? e->name[4] - 'A' : 0) : inv_best_variant(e);
    /* player+0xD6 = e->name[4] */
    i32 lvl35 = state && state->current_level_id == 0x23;    /* world_state */
    if (!e->equipped && id != 0x47 && !(lvl35 && inv_usable(id)))
        inv_toggle_equip();
    S.exit = 1;
}


/* Input bits a host would supply each frame. */
typedef struct {
    i32 esc, back, key_i, f1, left, right, up, down, pgup, pgdn, space, enter;
    i32 digit;                                 /* -1 none, 0..9 */
    i32 mx, my, lclick, rclick;
} inv_input_t;

static i32 in_rect(const inv_input_t *in, i32 x1, i32 y1, i32 x2, i32 y2) {   /* 0x10007280 */
    return in->mx >= x1 && in->mx <= x2 && in->my >= y1 && in->my <= y2;
}

static const i32 WIN_X[5] = { 0, 154, 348, 154, 348 };
static const i32 WIN_Y[5] = { 0, 43, 43, 227, 227 };

static void inv_select_step(i32 delta) {
    i32 rank = S.scroll + S.sel - 1 + delta;
    if (rank < 0 || rank >= S.count) return;
    if (rank < S.scroll)          S.scroll = rank;          /* slide anim 3/5 */
    else if (rank > S.scroll + 3) S.scroll = rank - 3;      /* slide anim 4/6 */
    S.sel = rank - S.scroll + 1;
    inv_fill_windows(S.scroll);
}

/* One frame of the screen. Returns 0 when it should close. */
i32 inventory_screen_frame(const inv_input_t *in, u32 now_ms) {
    if (in->esc || in->back || in->key_i || in->rclick ||
        (in->lclick && in_rect(in, 519, 411, 620, 465))) { S.button_hl = 3; S.exit = 1; }
    if (S.mode != INV_MODE_PICKUP &&
        (in->f1 || (in->lclick && (in_rect(in, 35, 386, 99, 450) || in_rect(in, 60, 406, 124, 470))))) {
        g_inv_category ^= 1;                   /* 0x10004D56: page switch */
        inv_count_visible(0);
        S.scroll = 0; S.sel = 1;
        inv_fill_windows(0);
        /* redraw background INVENT11/12 + INVENT10, tabs (0x10009760) */
    }
    if (in->left)  inv_select_step(-1);
    if (in->right) inv_select_step(+1);
    if (in->up)    inv_select_step(-2);
    if (in->down)  inv_select_step(+2);
    if (in->pgup)  { S.scroll = S.scroll >= 4 ? S.scroll - 4 : 0; inv_fill_windows(S.scroll); }
    if (in->pgdn)  { if (S.scroll + 4 < S.count) S.scroll += 4; inv_fill_windows(S.scroll); }
    if (in->lclick) {
        for (i32 k = 1; k <= 4; k++)
            if (in_rect(in, WIN_X[k], WIN_Y[k], WIN_X[k] + 159, WIN_Y[k] + 159) && S.win[k].inv_index >= 0) S.sel = k;
        if (in_rect(in, 44, 119, 120, 360)) {  /* list row -> jump (0x10008320) */
            i32 row = (in->my - 160) / 9;
            if (row >= 0 && S.scroll + row < S.count) { S.scroll = S.scroll + row; S.sel = 1; inv_fill_windows(S.scroll); }
        }
    }
    if (S.win[S.sel].inv_index >= 0) {
        i32 id = inv_name_id(g_inv[S.win[S.sel].inv_index].name);
        i32 use_first = (id == 0x47) || (state && state->current_level_id == 0x23 && inv_usable(id));
        if (in->space || (in->lclick && in_rect(in, 527, 77, 603, 189))) {
            /* sound INVENT2 (help_screen_context), button 1 highlight anim */
            if (use_first) inv_use(); else inv_toggle_equip();
        } else if (in->enter || (in->lclick && in_rect(in, 529, 244, 606, 375))) {
            inv_use();
        }
        if (in->digit == 0) inv_hotkey_forget(g_inv[S.win[S.sel].inv_index].name);
        if (in->digit >= 1 && in->digit <= 9) {
            const c8 *nm = g_inv[S.win[S.sel].inv_index].name;
            inv_hotkey_forget(nm);
            snprintf(g_inv_hotkey[in->digit - 1], 8, "%s", nm);
            S.dirty |= 1 << S.sel;
        }
    }
    /* spin the selected window: 0x10008250, angle += axis * (dt << 3) */
    {
        u32 dt = now_ms - S.prev_ms;
        inv_window_t *w = &S.win[S.sel];
        if (w->inv_index >= 0) {
            i32 id = inv_name_id(g_inv[w->inv_index].name);
            i8 a = (id >= 0 && id < 100) ? S.axis[id] : 0;
            i32 sgn = a > 0 ? 1 : -1, m = a < 0 ? -a : a;
            for (i32 c = 0; c < 3; c++) if (m & (1 << c)) w->ang[c] += sgn * (i32)(dt << 3);
        }
        S.prev_ms = now_ms;
    }
    /* draw: background, windows (bg save rb2 (160(k-1),320), model via
     * create_entity_texture into the 160x160 viewport, spot light + zoom
     * when selected, frame rb2 (160,0)-(319,158) when equipped, qty/hotkey/
     * ammo badges), list (OBJNAME, font 7, highlight 0x80, rows of windows
     * darkened), thumb, description, spinners, cursor. */
    S.thumb_y = S.count ? 122 + (S.scroll + S.sel - 1) * 180 / S.count : 122;
    return !S.exit;
}


/* ======================================================================
 * Engine hooks (sub_47398A / sub_473DBC / world copies)
 * ====================================================================== */

void inv_equip(const c8 *obj_name) {               /* sub_47398A(sub_475BB6(e)) */
    inv_set_equipped(obj_name, 1);
    if (!weapon_take(object_get(obj_name))) inv_set_equipped(obj_name, 0);
    g_ui_state = 2;
}

void inv_holster(const c8 *obj_name, i32 mode) {   /* sub_473DBC */
    game_object_t *o = object_get(obj_name);
    if (!o || !weapon_holster(o, mode)) {
        inv_set_equipped(obj_name, 0);
        if (!(mode & 2)) object_world_remove(obj_name);
    }
}

/* sub_426D2C @ 0x426D2C: after the player's body changed, put the first
 * equipped object back in the new body's hand; a creature body (id >= 4
 * with desc[0x97] set) cannot hold it and the flag is dropped. The move-set
 * reload (sub_462BCC) is done by weapon_take. Returns 1 if re-held. */
i32 inv_rehold(void) {
    entity_t *p = g_player_entity;
    for (i32 i = 0; i < INV_SLOTS; i++) {
        inv_entry_t *e = &g_inv[i];
        if (!e->name[0] || !e->equipped) continue;
        if (p && E32(p, 0xB0) >= 4 && EDESC(p) && EDESC(p)[0x97]) { e->equipped = 0; return 0; }
        return weapon_take(object_get(e->name)) ? 1 : 0;
    }
    return 0;
}

i32 player_in_anim(void) {                         /* is_entity_in_animation */
    return g_player_entity && !actor_anim_done(g_player_entity);
}

void obj_world_remove(const c8 *obj_name) { object_world_remove(obj_name); }   /* sub_472B47 */

void obj_world_drop(const c8 *obj_name) {          /* sub_472AF1 */
    inv_set_equipped(obj_name, 0);
    object_world_remove(obj_name);
}

void obj_world_place(const c8 *obj_name) { (void)obj_name; }   /* done by object_etat */

const c8 *player_held_id2(void) {                  /* player+0xD4 */
    for (i32 i = 0; i < INV_SLOTS; i++)
        if (g_inv[i].name[0] && g_inv[i].equipped) return g_inv[i].name + 2;
    return "--";
}

/* sub_42784E (see the note at inventory_screen_open). */
void inv_auto_combine(i32 held_id) {
    c8 nm[8];
    for (i32 i = 0; i < INV_SLOTS; i++)
        if (g_inv[i].name[0] && g_inv[i].pending) return;
    if (held_id != 84 && inv_find("OB84") && held_id != 85 && inv_find("OB85") &&
        held_id != 86 && inv_find("OB86")) {
        snprintf(nm, sizeof(nm), "OB84"); inv_remove_one(nm);
        snprintf(nm, sizeof(nm), "OB85"); inv_remove_one(nm);
        inv_sort();
        inv_add("OB71", 1, 1);
        *lieu_state_slot(0xC14) = 3;           /* dword_506854 */
    }
    if (held_id != 2 && inv_find("OB02") && held_id != 12 && inv_find("OB12") &&
        held_id != 18 && inv_find("OB18") && held_id != 19 && inv_find("OB19")) {
        static const c8 *const parts[4] = { "OB02", "OB12", "OB18", "OB19" };
        for (i32 k = 0; k < 4; k++) { snprintf(nm, sizeof(nm), "%s", parts[k]); inv_remove_one(nm); }
        inv_sort();
        inv_add("OB75", 1, 1);
        *lieu_state_slot(0xB10) = 2;           /* dword_506750 */
    }
}

/*
 * sub_4654CE(name, add): take an item in hand by name (hotkeys, DIVERS 44).
 * "OB00" or the held item's id puts nothing new in hand; any other id
 * holsters the held one first (mode 1 only for "OB00"). add = 1 (scripts)
 * also gives the item: refills its charges, resets the OB24 / OB77
 * counters and puts nbobj pieces in the inventory.
 */
void inv_use_by_name(const c8 *name, i32 add) {
    entity_t *p = g_player_entity;
    c8 a, b, letter = 0, cur[8];
    i32 id = -1;
    if (!p || !EDESC(p) || !name || !name[0]) return;
    if (sscanf(name, "%c%c%02d%c", &a, &b, &id, &letter) < 3 || id < 0 || id > 99) return;
    i32 v = letter ? (letter & 7) - 1 : 0;
    if (add) {
        const obj_record_t *r = obj_rec(id);
        if (r && (r->flags & 2) && r->arme < OBJ_RES_ROWS && v >= 0 && v < OBJ_RES_COLS)
            g_obj_res[r->arme][v] = (i16)r->nbvie;    /* full charges */
        if (id == 24 && v >= 0) *lieu_state_slot(0x140 + v * 4) = 3;
        if (id == 77 && v >= 0) *lieu_state_slot(0x140 + v * 4) = 0;
        if (id == 86) scene_set_combat_meter(0x124F80);   /* dword_506B9C */
    }
    snprintf(cur, sizeof(cur), "OB%02d", inv_held_id());
    if (strncmp(name, cur, 4) != 0) {
        game_object_t *held = (game_object_t *)EPTR(p, 0xD64);
        if (held && strcmp(cur, "OB00") != 0) weapon_holster(held, id == 0);   /* sub_473DBC */
        if (id != 0) {
            game_object_t *o = object_get(name);       /* sub_475BB6 */
            inv_entry_t *e = inv_find(name);
            if (!e && id == 0x47) scene_set_combat_meter(0x124F80);
            if (o && (!e || add)) {
                const obj_record_t *r = obj_rec(id);
                inv_add(o->name, r ? r->nbobj : 1, 0);
            }
            if (o && id != 0x47) weapon_take(o);         /* sub_47398A */
        }
    }
    if (!add) g_ui_state = 1;
}

/* ======================================================================
 * Native screen: resources, drawing, loop
 * ====================================================================== */

typedef struct {
    obc_node_t *root;
    ome_matr_t  matr;
    i32         id;
} inv_model_t;

static bmap_t     *g_bg[2], *g_over;               /* INVENT11 / 12, INVENT10 */
static u8          g_grd[0x4000];                  /* PAL>INVENT.GRD */
static i32         g_grd_ok;
static c8         *g_names_txt, *g_descr_txt;      /* OBJNAME / OBJDESCR */
static inv_model_t g_win_model[5];
static bmap_t     *g_atlas;                       /* OBJETS1 */
static i32     g_atlas_key = -1;                    /* the red key colour index */
static u16    *g_mask;                              /* INVMASK.CTX */
static i32     g_mask_n;

static c8 *inv_read_text(const c8 *vpath) {
    c8 host[512];
    const c8 *base = (state && state->game_path) ? state->game_path : ".";
    if (!ls_resolve_path(base, vpath, host, (i32)sizeof(host))) return NULL;
    FILE *f = ls_open(host, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    c8 *t = (c8 *)malloc((size_t)n + 1);
    if (t && fread(t, 1, (size_t)n, f) != (size_t)n) { free(t); t = NULL; }
    if (t) t[n] = 0;
    ls_close(f);
    return t;
}

/* "@n" records: the line after the header, up to the next '@'. */
static void inv_text_rec(const c8 *txt, i32 n, c8 *out, i32 cap) {
    out[0] = 0;
    if (!txt) return;
    c8 tag[16];
    snprintf(tag, sizeof(tag), "@%d", n);
    for (const c8 *p = txt; (p = strstr(p, tag)) != NULL; p++) {
        const c8 *q = p + strlen(tag);
        if (*q >= '0' && *q <= '9') continue;          /* @1 vs @10 */
        while (*q == '\r' || *q == '\n') q++;
        i32 k = 0;
        while (*q && *q != '@' && k < cap - 1) {
            out[k++] = (*q == '\r' || *q == '\n') ? ' ' : *q;
            q++;
        }
        while (k > 0 && out[k - 1] == ' ') k--;
        out[k] = 0;
        return;
    }
}

/* The dialogue language code (FR GB US DE IT SP JP); GB ships as US. */
static const c8 *inv_lang_dir(void) {
    const c8 *c = dlg_lang_code(state ? state->language_id : 2);
    return strcmp(c, "GB") == 0 ? "US" : c;
}

static void inv_load_resources(void) {
    if (!g_bg[0]) g_bg[0] = game_init_load_cursor("DATAS>GLOBAL>IMAGES>INVENT11.CIF");
    if (!g_bg[1]) g_bg[1] = game_init_load_cursor("DATAS>GLOBAL>IMAGES>INVENT12.CIF");
    if (!g_over)  g_over  = game_init_load_cursor("DATAS>GLOBAL>IMAGES>INVENT10.CIF");
    if (!g_grd_ok) {
        c8 host[512];
        const c8 *base = (state && state->game_path) ? state->game_path : ".";
        if (ls_resolve_path(base, "DATAS>GLOBAL>PAL>INVENT.GRD", host, (i32)sizeof(host))) {
            FILE *f = ls_open(host, "rb");
            if (f) { g_grd_ok = fread(g_grd, 1, sizeof(g_grd), f) == sizeof(g_grd); ls_close(f); }
        }
    }
    if (!g_names_txt) {
        c8 vp[96];
        snprintf(vp, sizeof(vp), "DATAS>DIALOGUE>%s>OBJNAME.TXT", inv_lang_dir());
        g_names_txt = inv_read_text(vp);
        snprintf(vp, sizeof(vp), "DATAS>DIALOGUE>%s>OBJDESCR.TXT", inv_lang_dir());
        g_descr_txt = inv_read_text(vp);
    }
    if (!g_atlas) {
        g_atlas = game_init_load_cursor("DATAS>GLOBAL>IMAGES>OBJETS1.CIF");
        if (g_atlas) {                                  /* the key: the commonest index */
            static i32 hist[256];
            memset(hist, 0, sizeof(hist));
            i32 st = g_atlas->stride > 0 ? g_atlas->stride : g_atlas->width;
            for (i32 y = 0; y < g_atlas->height; y++)
                for (i32 x = 0; x < g_atlas->width; x++) hist[g_atlas->pixels[y * st + x]]++;
            g_atlas_key = 0;
            for (i32 c = 1; c < 256; c++) if (hist[c] > hist[g_atlas_key]) g_atlas_key = c;
        }
    }
    if (!g_mask) {
        c8 h[512];
        const c8 *b = (state && state->game_path) ? state->game_path : ".";
        if (ls_resolve_path(b, "DATAS>GLOBAL>IMAGES>INVMASK.CTX", h, (i32)sizeof(h))) {
            FILE *f = ls_open(h, "rb");
            if (f) {
                fseek(f, 0, SEEK_END);
                long n = ftell(f);
                fseek(f, 0, SEEK_SET);
                g_mask = (u16 *)malloc((size_t)n);
                if (g_mask && fread(g_mask, 1, (size_t)n, f) == (size_t)n) g_mask_n = (i32)(n / 2);
                else { free(g_mask); g_mask = NULL; }
                ls_close(f);
            }
        }
    }
    /* INVENT2.CTX (else INVENT.CTX): 200 / 100 doubles ratio, 100 x 3 i32
     * rest angles, 100 i8 spin axes. */
    c8 host[512];
    const c8 *base = (state && state->game_path) ? state->game_path : ".";
    i32 two = ls_resolve_path(base, "DATAS>GLOBAL>INVENT2.CTX", host, (i32)sizeof(host)) != NULL;
    if (!two && !ls_resolve_path(base, "DATAS>GLOBAL>INVENT.CTX", host, (i32)sizeof(host))) return;
    FILE *f = ls_open(host, "rb");
    if (!f) return;
    i32 nr = two ? 200 : 100;
    if (fread(S.ratio, sizeof(f64), (size_t)nr, f) == (size_t)nr &&
        fread(S.rest_ang, sizeof(S.rest_ang), 1, f) == 1)
        if (fread(S.axis, sizeof(S.axis), 1, f) != 1) memset(S.axis, 0, sizeof(S.axis));
    ls_close(f);
}

static void inv_model_free(inv_model_t *m) {
    if (m->root) obc_free(m->root);
    ome_matr_free(&m->matr);
    memset(m, 0, sizeof(*m));
}

/* 0x100075B0: the preview copy of OBJETS>OBnn.OME. */
static void inv_model_load(inv_model_t *m, i32 id) {
    if (m->root && m->id == id) return;
    inv_model_free(m);
    if (id <= 0) return;
    c8 vp[64], host[512];
    snprintf(vp, sizeof(vp), "OBJETS>OB%02d.OME", id);
    const c8 *base = (state && state->game_path) ? state->game_path : ".";
    m->root = obc_load_ome(vp, base);
    if (!m->root) return;
    ome_file_t ome = {0};
    if (ls_resolve_path(base, vp, host, (i32)sizeof(host)) && model_ome_open(host, &ome)) {
        ome_load_matr(ome.raw, ome.raw_size, &m->matr);
        model_ome_free(&ome);
    }
    m->id = id;
}

/* Draw one window's item: centred on its bounds, turned by the window's
 * angles, lit from the viewer, scaled to fit the 160x160 window. */
static void inv_draw_model(const inv_model_t *m, const inv_window_t *w, i32 k, i32 sel) {
    if (!m->root || !state) return;
    const u8 *pal = g_bg[0] ? g_bg[0]->palette : NULL;
    if (!pal || !g_grd_ok) return;
    obc_node_t *r = m->root;
    i32 save_pos[3] = { r->pos[0], r->pos[1], r->pos[2] };
    i32 save_rot[3] = { r->rot[0], r->rot[1], r->rot[2] };
    r->pos[0] = r->pos[1] = r->pos[2] = 0;
    r->rot[0] = r->rot[1] = r->rot[2] = 0;
    i32 mn[3], mx[3];
    obc_node_bounds(r, mn, mx);
    f64 ex = 0;
    for (i32 c = 0; c < 3; c++) { f64 h = (mx[c] - mn[c]) / 2.0; ex += h * h; }
    ex = sqrt(ex);
    if (ex < 1) ex = 1;
    r->pos[0] = -(mn[0] + mx[0]) / 2; r->pos[1] = -(mn[1] + mx[1]) / 2; r->pos[2] = -(mn[2] + mx[2]) / 2;

    const i32 gscale = 0x8000;
    f64 fit = sel ? 70.0 : 60.0;                     /* the selected window zooms */
    i32 D = (i32)(ex * (gscale / 16384.0) * 512.0 / fit);

    static model_draw_t items[64];
    i32 n = 0;
    fmat_t cur, stack_m[16];
    obc_node_t *stack_n[16];
    i32 sp = 0;
    fmat_rotation(&cur, w->ang[0], w->ang[1], w->ang[2]);
    cur.t[0] = 0; cur.t[1] = 0; cur.t[2] = -D;
    obc_node_t *nd = r;
    for (;;) {
        while (nd) {
            if (nd->next && nd != r && sp < 16) { stack_n[sp] = nd->next; stack_m[sp] = cur; sp++; }
            fmat_t rot, pos;
            fmat_rotation(&rot, nd->rot[0], nd->rot[1], nd->rot[2]);
            fmat_position(&pos, nd->pos[0], nd->pos[1], nd->pos[2]);
            fmat_rotate_translation(&cur, &pos);
            fmat_premul_rotation(&rot, &cur);
            cur.t[0] += pos.t[0]; cur.t[1] += pos.t[1]; cur.t[2] += pos.t[2];
            const model_t *md = nd->atom[0] ? model_from_atm_atom(nd->atom[0], &m->matr) : NULL;
            if (md && n < 64) {
                model_draw_t *it = &items[n++];
                memset(it, 0, sizeof(*it));
                it->m = md;
                for (i32 c = 0; c < 9; c++) it->m3[c] = cur.m[c];
                for (i32 c = 0; c < 3; c++) it->t[c] = cur.t[c];
                it->zval = -1;
                it->light[0] = 0; it->light[1] = -11585; it->light[2] = -11585;
                it->light_s = sel ? 220 : 160;       /* spotlight on the selection */
            }
            nd = nd->child;
        }
        if (sp == 0) break;
        sp--; nd = stack_n[sp]; cur = stack_m[sp];
    }
    model_set_center(WIN_X[k] + 80, WIN_Y[k] + 80);
    model_render_batch(items, n, gscale, NULL, NULL, pal, g_grd, 0, 0, 0);
    model_set_center(320, 240);
    for (i32 c = 0; c < 3; c++) { r->pos[c] = save_pos[c]; r->rot[c] = save_rot[c]; }
}

/* ----------------------------------------------------------------------
 * Screen art: OBJETS1 atlas (320x272, red = transparent, copied into rb2
 * at (0,0) with sub-copies: tabs (173,167) -> rb2 (480,188), hotkey digits
 * (63,237) -> (492,252), ammo icons (176,234) -> (480,264), quantity boxes
 * (63,252) -> (480,293)). Coordinates below are atlas coordinates.
 * ---------------------------------------------------------------------- */

static u32 inv_pal_px(const u8 *pal, u8 idx) {
    return 0xFF000000u | ((u32)pal[idx * 3 + 2] << 16) | ((u32)pal[idx * 3 + 1] << 8) | pal[idx * 3];
}

/* draw_image from the atlas: inclusive source rect, `keyed` skips red. */
static void inv_atlas_blit(u32 *fb, i32 fw, i32 fh, i32 sx1, i32 sy1, i32 sx2, i32 sy2,
                           i32 dx, i32 dy, i32 keyed) {
    if (!g_atlas || !g_atlas->palette) return;
    i32 st = g_atlas->stride > 0 ? g_atlas->stride : g_atlas->width;
    for (i32 y = sy1; y <= sy2 && y < g_atlas->height; y++) {
        i32 ty = dy + y - sy1;
        if (ty < 0 || ty >= fh) continue;
        for (i32 x = sx1; x <= sx2 && x < g_atlas->width; x++) {
            i32 tx = dx + x - sx1;
            if (tx < 0 || tx >= fw) continue;
            u8 idx = g_atlas->pixels[y * st + x];
            if (keyed && idx == g_atlas_key) continue;
            fb[ty * fw + tx] = inv_pal_px(g_atlas->palette, idx);
        }
    }
}

/* INVENT.GRD level `lvl` (32 = unchanged) as a true-colour operation:
 * out = in * a + b, fitted over the 256 palette entries. */
typedef struct { f64 a, b[3]; i32 lvl; } inv_shade_t;

static void inv_shade_fit(inv_shade_t *sh, i32 lvl, const u8 *pal) {
    f64 sx = 0, sxx = 0, sy[3] = {0}, sxy[3] = {0};
    sh->lvl = lvl; sh->a = lvl / 32.0; sh->b[0] = sh->b[1] = sh->b[2] = 0;
    if (!pal || !g_grd_ok) return;
    for (i32 c = 0; c < 256; c++) {
        u8 d = g_grd[c * 64 + lvl];
        for (i32 k = 0; k < 3; k++) {
            f64 x = pal[c * 3 + k], y = pal[d * 3 + k];
            sx += x; sxx += x * x; sy[k] += y; sxy[k] += x * y;
        }
    }
    f64 n = 768, den = n * sxx - sx * sx;
    f64 syt = sy[0] + sy[1] + sy[2], sxyt = sxy[0] + sxy[1] + sxy[2];
    sh->a = den != 0 ? (n * sxyt - sx * syt) / den : lvl / 32.0;
    for (i32 k = 0; k < 3; k++) sh->b[k] = (sy[k] - sh->a * sx / 3) / 256.0;
}

static u32 inv_shade_px(const inv_shade_t *sh, u32 p) {
    i32 c[3] = { (i32)(p & 0xFF), (i32)((p >> 8) & 0xFF), (i32)((p >> 16) & 0xFF) };
    for (i32 k = 0; k < 3; k++) {
        i32 v = (i32)(c[k] * sh->a + sh->b[k]);
        c[k] = v < 0 ? 0 : v > 255 ? 255 : v;
    }
    return 0xFF000000u | ((u32)c[2] << 16) | ((u32)c[1] << 8) | (u32)c[0];
}

/* Walk INVMASK: per row (skip, run) u16 pairs that sum to 640. Fills
 * `in` (640x480 bytes, 1 inside the panel). */
static void inv_mask_cover(u8 *in) {
    memset(in, 0, 640 * 480);
    if (!g_mask) return;
    i32 i = 0;
    for (i32 y = 0; y < 480 && i + 1 < g_mask_n; y++) {
        i32 x = 0;
        while (x < 640 && i + 1 < g_mask_n) {
            x += g_mask[i];
            i32 run = g_mask[i + 1];
            i += 2;
            for (i32 k = 0; k < run && x < 640; k++, x++) in[y * 640 + x] = 1;
        }
    }
}

/* 0x10008F70 mode 2: the frozen game view, shaded at GRD level 0x2E inside
 * the panel shape (0x10001EF0), with an 8 px drop shadow at level 0x10
 * (0x10002040). */
static void inv_draw_pickup_bg(u32 *fb, i32 fw, i32 fh, const u32 *game_view) {
    static u8 in[640 * 480];
    const u8 *pal = g_bg[0] ? g_bg[0]->palette : NULL;
    inv_shade_t fog, shadow;
    inv_shade_fit(&fog, 0x2E, pal);
    inv_shade_fit(&shadow, 0x10, pal);
    inv_mask_cover(in);
    for (i32 y = 0; y < fh; y++)
        for (i32 x = 0; x < fw; x++) {
            u32 p = game_view[y * fw + x];
            if (x < 640 && y < 480) {
                if (in[y * 640 + x]) p = inv_shade_px(&fog, p);
                else if (x >= 8 && y >= 8 && in[(y - 8) * 640 + x - 8]) p = inv_shade_px(&shadow, p);
            }
            fb[y * fw + x] = p;
        }
}

/* 0x10001210: shade a rectangle in place (tabs, level 15). */
static void inv_shade_rect(u32 *fb, i32 fw, i32 x1, i32 y1, i32 x2, i32 y2, i32 lvl) {
    inv_shade_t sh;
    inv_shade_fit(&sh, lvl, g_bg[0] ? g_bg[0]->palette : NULL);
    for (i32 y = y1; y <= y2; y++)
        for (i32 x = x1; x <= x2; x++) fb[y * fw + x] = inv_shade_px(&sh, fb[y * fw + x]);
}

/* 0x10009760: the page tabs (objects icon atlas (173,167), weapons (237,167)). */
static void inv_draw_tabs(u32 *fb, i32 fw, i32 fh) {
    if (S.mode == 0) {
        inv_atlas_blit(fb, fw, fh, 173, 167, 236, 228, 35, 386, 0);
        inv_atlas_blit(fb, fw, fh, 237, 167, 300, 228, 60, 406, 0);
        inv_shade_rect(fb, fw, 35, 386, 59, 450, 15);
        inv_shade_rect(fb, fw, 60, 386, 99, 405, 15);
    } else if (S.mode == 1) {
        inv_atlas_blit(fb, fw, fh, 237, 167, 300, 228, 60, 406, 0);
        inv_atlas_blit(fb, fw, fh, 173, 167, 236, 229, 35, 386, 0);
        inv_shade_rect(fb, fw, 99, 406, 124, 470, 15);
        inv_shade_rect(fb, fw, 60, 449, 98, 470, 15);
    }
}

/* Font colour index 0x80 of the screen palette. */
static u32 inv_text_col(void) {
    const u8 *pal = g_bg[0] ? g_bg[0]->palette : NULL;
    if (!pal) return 0xFFFFFFFFu;
    return 0xFF000000u | ((u32)pal[0x80 * 3] << 16) | ((u32)pal[0x80 * 3 + 1] << 8) | pal[0x80 * 3 + 2];
}

/* 0x10003000 (part): equipped frame, quantity box, hotkey digit. */
static void inv_draw_badges(u32 *fb, i32 fw, i32 fh, i32 k) {
    inv_window_t *w = &S.win[k];
    const inv_entry_t *e = &g_inv[w->inv_index];
    i32 X = WIN_X[k], Y = WIN_Y[k], id = inv_name_id(e->name);
    if (w->framed) inv_atlas_blit(fb, fw, fh, 160, 0, 319, 158, X, Y, 1);
    if (e->name[0] != 'Q') {
        i32 n = w->qty;
        i32 ammo = (id == 6 || id == 7 || id == 8 || id == 0x27 || id == 0x48 || id == 0x49);
        if (n > 1 || ammo) {
            const obj_record_t *r = obj_rec(id);
            if (r && r->linke == id) n--;
            c8 t[16];
            snprintf(t, sizeof(t), "%d", n);
            if (n < 10)       { inv_atlas_blit(fb, fw, fh, 121, 252, 135, 266, X + 0x81, Y + 0x7F, 1); draw_text_tinted(font_handle, X + 0x84, Y + 0x7C, t, inv_text_col()); }
            else if (n < 100) { inv_atlas_blit(fb, fw, fh,  96, 252, 118, 266, X + 0x79, Y + 0x7F, 1); draw_text_tinted(font_handle, X + 0x7C, Y + 0x7C, t, inv_text_col()); }
            else              { inv_atlas_blit(fb, fw, fh,  63, 252,  93, 266, X + 0x71, Y + 0x7F, 1); draw_text_tinted(font_handle, X + 0x74, Y + 0x7C, t, inv_text_col()); }
        }
    }
    for (i32 m = 1; m <= INV_HOTKEYS; m++)
        if (g_inv_hotkey[m - 1][0] && !strncmp(g_inv_hotkey[m - 1], e->name, 4)) {
            inv_atlas_blit(fb, fw, fh, 51 + 12 * m, 237, 62 + 12 * m, 248, X + 20, Y + 20, 1);
            break;
        }
}

/* 0x10003490: a weapon's ammunition in its window corner (when the ammo
 * entry is not held itself). OB27 <- OB06, OB55 <- OB07 + OB08,
 * OB23 <- OB72, OB32 <- OB39, OB35 <- OB73. */
static void inv_draw_ammo(u32 *fb, i32 fw, i32 fh, i32 k) {
    static const i32 icon_x[7] = { 0, 176, 281, 218, 239, 260, 197 };   /* atlas x, y 234..262 */
    static const i32 ammo_id[7] = { 0, 6, 7, 0x48, 0x27, 0x49, 8 };
    inv_window_t *w = &S.win[k];
    i32 id = inv_name_id(g_inv[w->inv_index].name), t;
    switch (id) {
    case 27: t = 1; break;
    case 55: t = 2; break;
    case 23: t = 3; break;
    case 32: t = 4; break;
    case 35: t = 5; break;
    default: return;
    }
    for (i32 row = 0; row < 2; row++) {
        c8 nm[8], q[16], txt[16];
        snprintf(nm, sizeof(nm), "OB%02d", ammo_id[t]);
        const inv_entry_t *a = inv_find(nm);
        if (a && !a->equipped) {
            snprintf(q, sizeof(q), "%d", S.mode == INV_MODE_PICKUP ? a->pending : a->qty);
            snprintf(txt, sizeof(txt), "%3s", q);
            i32 X = WIN_X[k] + 0x78, Y = WIN_Y[k] + 0x14 + row * 0x28;
            inv_atlas_blit(fb, fw, fh, icon_x[t], 234, icon_x[t] + 19, 262, X, Y, 1);
            draw_text_tinted(font_handle, X + 2, Y + 0x14, txt, inv_text_col());
        }
        if (t != 2) break;
        t = 6;                                         /* OB55 also shows OB08 */
    }
}

static void inv_draw(const u32 *game_view) {
    u32 *fb = state->graphic.front_buffer;
    i32 fw = state->graphic.width, fh = state->graphic.height;
    if (S.mode == INV_MODE_PICKUP || !g_bg[0]) {
        inv_draw_pickup_bg(fb, fw, fh, game_view);
        if (g_over) blit_bmap_masked_to_framebuffer(fb, fw, fh, g_over, g_over->palette, 0, 0, 0);
    } else {
        bmap_t *bg = g_bg[g_inv_category ? 1 : 0] ? g_bg[g_inv_category ? 1 : 0] : g_bg[0];
        blit_bmap_to_framebuffer(fb, fw, fh, bg, bg->palette, 0, 0);
        if (g_over) blit_bmap_masked_to_framebuffer(fb, fw, fh, g_over, g_over->palette, 0, 0, 0);
        inv_draw_tabs(fb, fw, fh);
    }
    for (i32 k = 1; k <= 4; k++) {
        inv_window_t *w = &S.win[k];
        if (w->inv_index < 0) continue;
        inv_draw_model(&g_win_model[k], w, k, k == S.sel);
        inv_draw_badges(fb, fw, fh, k);
        inv_draw_ammo(fb, fw, fh, k);
    }
    /* 0x100039E0 Name list (every mode): every listable item, the 4 shown
     * in windows darker, the selection darkest. */
    for (i32 r = 0; r < S.count && r < 20; r++) {   /* 12 px rows: the system font is ~11 px tall */
        i32 idx = inv_nth_listable(r);
        if (idx < 0) break;
        c8 nm[64];
        inv_text_rec(g_names_txt, inv_name_id(g_inv[idx].name), nm, (i32)sizeof(nm));
        if (!nm[0]) snprintf(nm, sizeof(nm), "%.4s", g_inv[idx].name);
        u32 col = (r == S.scroll + S.sel - 1) ? 0xFF000000u
                : (r >= S.scroll && r < S.scroll + 4) ? 0xFF3A3028u : 0xFF7A6A58u;   /* pale panel */
        draw_text_tinted(font_handle, 48, 122 + r * 12, nm, col);
    }
    /* Description of the selection. */
    if (S.win[S.sel].inv_index >= 0) {
        c8 ds[160];
        inv_text_rec(g_descr_txt, inv_name_id(g_inv[S.win[S.sel].inv_index].name), ds, (i32)sizeof(ds));
        draw_text_at(font_handle, 202, 438, ds, 0xFFFFFFFFu);
    }
}

void inventory_screen_run(void) {
    if (!state || !state->graphic.front_buffer) return;
    memset(&S, 0, sizeof(S));
    S.sel = 1; S.open_ctr = 0x46; S.thumb_y = 122;
    for (i32 k = 1; k <= 4; k++) S.win[k].inv_index = -1;
    inv_load_resources();
    inv_count_visible(1);
    inv_fill_windows(0);
    if (S.count == 0) return;
    sfx_play_anims("INVENT1");

    i32 fw = state->graphic.width, fh = state->graphic.height;
    u32 *game_view = (u32 *)malloc((size_t)fw * fh * sizeof(u32));
    if (!game_view) return;
    memcpy(game_view, state->graphic.front_buffer, (size_t)fw * fh * sizeof(u32));

    system_t *sys = &state->system;
    i32 prev[256];
    memcpy(prev, sys->actions, sizeof(prev));        /* keys held at open are not presses */
    S.prev_ms = ls_tick();
    u32 opened = S.prev_ms;
    while (!sys->quit) {
        ls_update_events(sys);
        sfx_pump();                                  /* music / voices keep playing */
        inv_input_t in = {0};
#define EDGE(k) (sys->actions[k] && !prev[k])
        in.esc = EDGE(27); in.back = EDGE(8); in.key_i = EDGE(105); in.f1 = EDGE(112);
        in.left = EDGE(37); in.right = EDGE(39); in.up = EDGE(38); in.down = EDGE(40);
        in.pgup = EDGE(33); in.pgdn = EDGE(34); in.space = EDGE(32); in.enter = EDGE(13);
        in.digit = -1;
        for (i32 dgt = 0; dgt <= 9; dgt++) if (EDGE('0' + dgt)) in.digit = dgt;
#undef EDGE
        in.mx = sys->motion_x; in.my = sys->motion_y;
        in.lclick = sys->mouse_left == 2;
        memcpy(prev, sys->actions, sizeof(prev));
        if ((getenv("DKE_INVAUTO") || getenv("DKE_FRAMES")) && ls_tick() - opened > 1500) in.esc = 1;   /* headless */
        /* Load the preview models of the windows on show. */
        for (i32 k = 1; k <= 4; k++)
            inv_model_load(&g_win_model[k], S.win[k].inv_index >= 0
                           ? inv_name_id(g_inv[S.win[k].inv_index].name) : 0);
        for (i32 k = 1; k <= 4; k++)
            if (S.win[k].inv_index >= 0 && S.win[k].ang[0] == 0 && S.win[k].ang[1] == 0 && S.win[k].ang[2] == 0)
                S.win[k].ang[1] = 0x2000;            /* no tuning entry: a three-quarter view */
        if (!inventory_screen_frame(&in, ls_tick())) break;
        inv_draw(game_view);
        if (getenv("DKE_INVSHOT")) {
            FILE *f = fopen(getenv("DKE_INVSHOT"), "wb");
            if (f) {
                fprintf(f, "P6\n%d %d\n255\n", fw, fh);
                for (i32 i = 0; i < fw * fh; i++) {
                    u32 p = state->graphic.front_buffer[i];
                    fputc((u8)p, f); fputc((u8)(p >> 8), f); fputc((u8)(p >> 16), f);
                }
                fclose(f);
            }
        }
        fx_present();
        ls_delay(10);
    }
    for (i32 k = 1; k <= 4; k++) inv_model_free(&g_win_model[k]);
    memcpy(state->graphic.front_buffer, game_view, (size_t)fw * fh * sizeof(u32));
    free(game_view);
}

u8 objets_flags(i32 id) { const obj_record_t *r = obj_rec(id); return r ? r->flags : 0; }
