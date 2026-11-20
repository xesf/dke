/*
 * lieu09.c — Native hand-port of LIEU09.DLL (level-9 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu09_trig.c (reached through
 * l09_trigger, see lieu09_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..03; the
 * entry is at 0x1000A920, the state pointer global is dword_1001A78C and the
 * trigger table is dword_1001A6A0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000AFEC), lieu_pre_scene (sub_1000B19C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are instruction-for-instruction identical to LIEU03. tick_body
 * (sub_1000798C) is level-9 specific.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu09_int.h"

/* Global sprintf buffer (dword_1001A4A0). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * Per-frame body — sub_1000798C (thunk 0x100010B9), level-9 specific.
 * Labels in comments = original addresses. `return` == original `jmp` to
 * the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x799E — hero dead: queue the game-over event once */
    if (ST(ST_HERO) == 4 && ST(V_WEAPON_WARNED) == 0) {
        ST(V_WEAPON_WARNED) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x79E1 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x7A12 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x7A3B */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x7A67 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x7A93 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x7AB7 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x7AD4 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x7B00 */
    if (ST(V_EVENT_25) == 1) {
        ST(V_EVENT_25) = 0;
        CB->evenement(25, 3000);
    }
    /* 0x7B31 */
    if (ST(V_EVENT_25) == 2) {
        ST(V_EVENT_25) = 0;
        CB->bruitage("L09B02", 320, -132);
        CB->joindre(ST(ST_HERO), 0, 0, 0xFA0, -1);
        /* 0x7B83 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->animation("SG01PRE07");
        }
        /* 0x7BA0 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SG02PRE07");
        }
        /* 0x7BBD */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SG03PRE07");
        }
        snprintf(g_txt, sizeof(g_txt), "OB07%c", (ST(V_EVENT15_STEP) + 65));
        CB->etat_objet(g_txt, -1, -1, -1, -1, -1, -1, -3, 1);
        CB->animation("Sar_100");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("ar");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x7C4D */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        CB->select_pile(1, 2);
        CB->bruitage("porte2", 464, -618);
        CB->select_pile(2, 2);
        /* 0x7CA3 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            CB->anim_objet("IM71", 3, 0x3C, 0x5DC, 0);
        }
        /* 0x7CCE */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->anim_objet("IM71", 3, 0x3C, 0x1F4, 0x400);
        }
        ST(ST_LEVEL) = 9;
        CB->bloque(BLOQUE_NONE);
        CB->change_piece(2, 0);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7D2F */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        /* 0x7D50 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            CB->anim_objet("IM23", 1, -54, 0x3E8, 0);
        }
        /* 0x7D7B */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->anim_objet("IM23", 1, -54, 0x12C, 0x200);
        }
        CB->evenement(33, 1200);
        return;
    }
    /* 0x7DBE */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        CB->animation("P02_65535");
        CB->animation("ar_50");
        CB->pos_perso(ST(ST_HERO), 738, -205, 0xFFFF, 1, "ar", 4, 0);
        CB->animation("Sar_50");
        /* 0x7E2B */
        if (ST(ST_SPECIAL_MODE) == 1) {
            CB->evenement(34, 150);
            CB->animation("G02OUV05");
            CB->animation("ar_50");
        }
        /* 0x7E63 */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->evenement(34, 250);
            CB->animation("G02OUV45");
            CB->animation("ar_50");
        }
        return;
    }
    /* 0x7EA0 */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        CB->select_pile(1, 1);
        CB->anim_objet("IM23", 1, 0x36, 0x12C, 0x200);
        /* 0x7EED */
        if (ST(L09_GATE_STATE) == 1) {
            ST(L09_GATE_STATE) = 2;
            CB->etat_objet("OB33D", -1, -1, -1, -1, -1, -1, -3, 3);
        }
        /* 0x7F2C */
        if (ST(L09_GATE_STATE) == 0) {
            ST(L09_GATE_STATE) = 1;
            CB->etat_objet("OB33C", -1, -1, -1, -1, -1, -1, -3, 3);
        }
        CB->animation("Sar_50");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7FA1 */
    if (ST(V_EVENT_35) == 1) {
        ST(V_EVENT_35) = 0;
        /* 0x7FC2 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            CB->evenement(34, 950);
            CB->animation("G02OUV07");
            CB->animation("ar_50");
        }
        /* 0x7FFA */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->evenement(34, 200);
            CB->animation("G02OUV47");
            CB->animation("ar_50");
        }
        return;
    }
    /* 0x8037 */
    if (ST(V_GUARDIAN8_STAGE) == 2 && CB->divers(DV_WAS_HIT, P_GUARDIAN8, 0) && ST(ST_VIE(P_GUARDIAN8)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN8, 1);
        CB->combat(P_GUARDIAN8, -1);
        /* 0x80BB */
        if ((CB->divers(DV_HP, P_GUARDIAN8, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_GUARDIAN8, (CB->divers(DV_HP, P_GUARDIAN8, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_GUARDIAN8, 1);
        }
        /* 0x811C */
        if (CB->divers(DV_HP, P_GUARDIAN8, -1) <= 1) {
            ST(ST_VIE(P_GUARDIAN8)) = VIE_DEAD;
            CB->stop_joindre(-P_GUARDIAN8);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN8, 0);
            CB->animation("SP48IMP55");
            CB->animation("P48CAD");
            CB->pos_perso(P_GUARDIAN8, 0, 0, 0, 2, "P48CAD", 4, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_GUARDIAN8);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN8, 0);
            CB->animation("SP48IMP15");
            CB->animation("P48IMP50");
            CB->animation("P48GAR05");
            /* 0x81DE */
            if (ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN8;
            }
            CB->combat(P_GUARDIAN8, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8216 */
    if (ST(V_GUARDIAN8_STAGE) == 2 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_GUARDIAN8)) == VIE_DEAD) {
        ST(V_GUARDIAN8_STAGE) = 3;
        ST(V_ENGAGED_PERSONA) = 0;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1.
 * Callback ingestion, the trigger-table fill and the unused state-pointer
 * copy are skipped: g_lieu_cb is provided by the bridge, the trigger table
 * is static in lieu09_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending,
 * 0x1000AEE2); every other path returns state[0].
 * ---------------------------------------------------------------------- */
static i32 init(i32 reload_flag) {
    if (reload_flag)
        return 0;

    CB->select_pile(2, 2);
    CB->select_pile(1, 2);
    CB->select_pile(0, 2);
    CB->select_pile(0, 1);
    CB->evenement(0, -1);
    CB->evenement(1, -1);
    CB->evenement(2, -1);
    CB->evenement(33, -1);
    CB->evenement(34, -1);
    CB->evenement(35, -1);
    CB->evenement(36, -1);
    CB->evenement(37, -1);
    CB->evenement(38, -1);
    CB->evenement(39, -1);
    CB->evenement(41, -1);

    if (ST(ST_HERO) != 4 || ST(V_CHASE_STATE) == 1) {
        l09_trig00();   /* 0x1000AEE9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* the hero is dead: play the death ending */
    CB->divers_empile(DV_SCREEN, 1, 0);
    ST(V_DEATH_EVENT_SENT) = 1;
    for (i32 layer = 0; layer < 4; layer++) {
        if (CB->divers(DV_PERSONA_PRESENT, layer, 0))
            CB->divers(DV_HP, layer, 0);
    }
    if (ST(V_GUARD_FIGHT_DONE) == 0)
        CB->cinematique("SC22_500");
    if (ST(V_GUARD_FIGHT_DONE) != 0)
        CB->cinematique("SC23_500");
    CB->divers_empile(DV_END_GAME, 0, 0);
    return 1;
}

/* -------------------------------------------------------------------------
 * TICK (opcode -1, 0x1000AF3C): drain pending sound, run body.
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

/* -------------------------------------------------------------------------
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000A920).
 * Jump table @ 0x1000AFB9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu09_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu09: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l09_trigger(arg);             break;  /* dword_1001A6A0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
