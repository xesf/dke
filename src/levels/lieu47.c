/*
 * lieu47.c - Native hand-port of LIEU47.DLL (level-47 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick, the tick body and the
 * event4 / event5 / event6 dispatch
 * (the handlers are the shared older-version ones, lieu_common_v1.c). The 42 zone trigger handlers live in
 * lieu47_trig.c (reached through l47_trigger, see lieu47_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. LIEU47 is the room-11 script, built from
 * the same older script version as LIEU33: the entry is at 0x10001200, the
 * state pointer global is dword_10019B0C and the trigger table is
 * dword_10019A20[42].
 *
 * Its event4 (sub_10001D60), event5 (sub_10002AFF) and event6
 * (sub_10005DD3) are not the lieu_common.c ones. They match the LIEU33 ones
 * (instruction by instruction, checked by a normalised listing diff) except:
 *   - event4 has one extra case, 44 (EV_END_GAME): V_END_GAME_REQ = 1;
 *   - event5 OB_PARCHMENT sets ST(L47_LEVER_ANIM_PAUSED) = 1 when ST_LEVEL == 28, right
 *     after the V_LCOD23_SEEN update.
 * lieu_action_register is the shared one. sub_10001856 (the LIEU33
 * pre-scene variant, 706 bytes) is never called in this DLL and is not
 * ported. The DLL .data the level code touches: the four event4 float
 * accumulators (dword_10015B80..8C, initially 1.0f), the two event5 sprintf
 * buffers and the tick print_msg buffer; all are reset at INIT.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu47_int.h"

/* tick body print_msg buffer (dword_10019820). */
static c8 g_txt[32];

/* -------------------------------------------------------------------------
 * Per-frame body - sub_100076BA (thunk 0x100010B9), level-47 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x76CC */
    if (ST(ST_HERO) == P_ARKHAN3 && ST(L47_ARKHAN3_SPOTTED) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 500 && ST(L47_GUARD_HOSTILE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L47_ARKHAN3_SPOTTED) = 1;
        CB->animation("SP32GAR05");
        CB->dialogue("L11D01", P_GUARD);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7797 */
    if (ST(ST_HERO) == P_ARKHAN3 && ST(L47_ARKHAN3_CHALLENGED) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 200 && ST(L47_GUARD_HOSTILE) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L47_ARKHAN3_CHALLENGED) = 1;
        CB->vire_col(7, 0);
        CB->dialogue("L11D02", P_GUARD);
        ST(V_ENGAGED_PERSONA) = P_GUARD;
        CB->combat(P_GUARD, 0);
        ST(L47_GUARD_HOSTILE) = 1;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x787D */
    if (ST(ST_HERO) == P_ARKHAN2 && ST(L47_ARKHAN2_SPOTTED) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 400 && ST(L47_GUARD_HOSTILE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L47_ARKHAN2_SPOTTED) = 1;
        CB->vire_col(7, 0);
        CB->animation("SP32GAR05");
        CB->dialogue("L11D03", P_GUARD);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7955 */
    if (ST(ST_HERO) == P_ARKHAN2 && ST(L47_ARKHAN2_WARNED) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 250 && ST(L47_GUARD_HOSTILE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L47_ARKHAN2_WARNED) = 1;
        CB->vire_col(7, 0);
        CB->animation("SP32ARR04");
        CB->dialogue("L11D04", P_GUARD);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7A2D */
    if (ST(ST_VIE(P_GUARD)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_GUARD) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x7A62 */
    if (CB->divers(DV_WAS_HIT, P_GUARD, 0) && ST(ST_VIE(P_GUARD)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_GUARD, 1);
        CB->combat(P_GUARD, -1);
        /* 0x7AB5 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_GUARD;
        }
        ST(L47_GUARD_HOSTILE) = 1;
        /* 0x7AF6 */
        if ((CB->divers(DV_HP, P_GUARD, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_GUARD, (CB->divers(DV_HP, P_GUARD, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_GUARD, 1);
        }
        /* 0x7B57 */
        if (CB->divers(DV_HP, P_GUARD, -1) <= 1) {
            ST(ST_VIE(P_GUARD)) = VIE_DEAD;
            CB->select_pile(2, 2);
            CB->divers_empile(DV_BLOOD_AT_BONE, 32, 0);
            CB->select_pile(1, 1);
            CB->animation("SP32IMP55");
            CB->animation("P32CAD");
            CB->pos_perso(P_GUARD, 0, 0, 0, 2, "P32CAD", 0xC, 0);
        } else {
            CB->divers_empile(DV_BLOOD_AT_BONE, 32, 0);
            CB->animation("SP32IMP15");
            CB->animation("P32IMP50");
            CB->animation("P32GAR05");
            CB->combat(P_GUARD, 0);
        }
        CB->select_pile(0, 1);
    }
    snprintf(g_txt, sizeof(g_txt), "Garde:%d", ST(ST_VIE(P_GUARD)));
    CB->print_msg(g_txt, 0, 240);
    snprintf(g_txt, sizeof(g_txt), "F37:%d", ST(L47_GUARD_FOOD_STAGE));
    CB->print_msg(g_txt, 0, 260);
    /* 0x7C8E */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x7CAF */
        if (ST(L47_SHOUT_STATE) == 32) {
            ST(L47_SHOUT_STATE) = 99;
            ST(V_ENGAGED_PERSONA) = P_GUARD;
            CB->combat(P_GUARD, 0);
            ST(L47_GUARD_HOSTILE) = 1;
            lieu_action_register("L11D05", P_GUARD, "SP32ARR04", "SP32GAR05");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x7D3D */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        CB->play_anim(0, 0);
        CB->animation("Sar_20");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        /* 0x7D84 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->cinematique("SC28_500");
        }
        /* 0x7DA1 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->cinematique("SC29_500");
        }
        CB->bloque(BLOQUE_NONE);
        ST(ST_LEVEL) = 11;
        CB->change_piece(12, -1);
    }
    /* 0x7DE2 */
    if (ST(L47_GUARD_HOSTILE) == 1 && ST(ST_LEVEL) == 10) {
        CB->pos_perso(P_GUARD, 6087, -1362, 0x449F, 1, "P32GAR05", 0xC, 0);
    }
    /* 0x7E28 */
    if (ST(ST_HERO) == 4 && ST(L47_END_EVENT_SENT) == 0) {
        ST(L47_END_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x7E65 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x7E8B */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x7EB4 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x7EE0 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x7F0C */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x7F30 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x7F4D */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
    /* 0x7F7E */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_LOCKED_CAMERA) == 3 && ST(L47_LEVER_ANIM_PAUSED) == 0) {
        CB->pause_anim(0, 1, 0);
        ST(L47_LEVER_ANIM_PAUSED) = 1;
    }
    /* 0x7FD2 */
    if (ST(ST_LOCKED_CAMERA) != 3 && ST(L47_LEVER_ANIM_PAUSED) == 1) {
        ST(L47_LEVER_ANIM_PAUSED) = 0;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000158A).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_100194DC = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu47_trig.c. There is no death
 * branch in this version.
 *   arg = reload flag (non-zero => skip arming).
 * ---------------------------------------------------------------------- */
static void init(i32 reload_flag) {
    if (reload_flag == 0)
        l47_trig00();   /* 0x100015A3: call sub_10001073 -> sub_10007183 */
}

/* -------------------------------------------------------------------------
 * TICK (opcode -1, 0x100015F6): drain pending sound, run body.
 * ---------------------------------------------------------------------- */
static void tick(void) {
    if (ST(V_PENDING_EVENT) != 0) {
        CB->select_pile(2, 1);
        CB->evenement(ST(V_PENDING_EVENT) + EV_PENDING_BASE, 1000);
        ST(V_PENDING_EVENT) = 0;
        CB->select_pile(0, 1);
    }
    tick_body();
}

/* The DLL is loaded afresh for every room: its globals restart. Called by
 * INIT, and by tools/lieu_difftest_harness.c before every case. */
void l47_reset_dll_globals(void) {
    lieu_v1_reset();
    g_txt[0] = '\0';
}

/* -------------------------------------------------------------------------
 * Entry point - @FonctionEditor@16 (thunk 0x1000104B -> 0x10001200).
 * Jump table @ 0x10001673, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu47_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu47: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        l47_reset_dll_globals();
        init(arg);
        break;
    case LIEU_OP_EVENT:  l47_trigger(arg);             break;  /* dword_10019A20[arg]() */
    case LIEU_OP_CALL4:  lieu_v1_event4(arg, 1);            break;
    case LIEU_OP_CALL5:  lieu_v1_event5(arg, arg2, 1);      break;
    case LIEU_OP_CALL6:  lieu_v1_event6(arg);                   break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
