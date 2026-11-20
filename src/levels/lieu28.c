/*
 * lieu28.c - Native hand-port of LIEU28.DLL (level-28 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu28_trig.c (reached through
 * l28_trigger, see lieu28_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000D4E0, the state pointer
 * global is dword_1001FDAC and the trigger table is dword_1001FCC0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000DBAC), lieu_pre_scene (sub_1000DD5C) and
 * sub_10001200 (float reset) are the shared lieu_common.c. tick_body
 * (sub_10007BBB) is level-28 specific. The level code touches no DLL .data
 * globals, so INIT has nothing to reset.
 */

#include <stdlib.h>

#include "lieu28_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10007BBB (thunk 0x100010B9), level-28 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x7BCD */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x7BEB */
        if (ST(L28_DOOR_EVENT1_STEP) == 0) {
            CB->select_pile(1, 1);
            CB->play_anim(0, 0);
            CB->bruitage("L28B01", -1, -1);
            /* 0x7C29 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L28D84", P_ARKHAN2);
            }
            /* 0x7C4B */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L28D120", P_ARKHAN3);
            }
            CB->animation("Sar_10");
            CB->etat_objet("OB03A", -1, -1, -1, -1, -1, -1, -3, 3);
            CB->free_anim(0);
            CB->animation("Sar_10");
            CB->load_anim("228OUV02", 1);
            CB->play_anim(1, 0);
            CB->modify_var(&ST(L28_DOOR_EVENT1_STEP), 1);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x7CEC */
    if (ST(ST_LOCKED_CAMERA) == 1 && ST(L28_DOOR_EVENT1_STEP) == 1) {
        CB->free_anim(1);
        ST(L28_DOOR_EVENT1_STEP) = 0;
        return;
    }
    /* 0x7D26 */
    if (ST(L28_ARCHIVIST_LEFT_POST) == 0 && ST(V_LORY_STAGE) != 1 && ST(V_LORY_STAGE) != 2 && ST(V_LORY_STAGE) != 3 && ST(ST_VIE(P_ARCHIVIST)) != VIE_DEAD && CB->distance(1, 2) > 250 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        ST(L28_ARCHIVIST_LEFT_POST) = 1;
        CB->joindre(P_ARCHIVIST, 7890, -4827, 0xEA60, -1);
        CB->animation("SP05LIR01");
        CB->select_pile(0, 1);
    }
    /* 0x7DFA */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        CB->play_anim(1, 0);
        CB->evenement(2, 1000);
    }
    /* 0x7E38 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        /* 0x7E59 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->divers(DV_MUSIC, 1, 4);
        }
        /* 0x7E7D */
        if (ST(V_GUARD_FIGHT_DONE) == 2 || ST(V_LORY_STAGE) >= 1) {
            CB->divers(DV_MUSIC, 2, 4);
        }
        /* 0x7EB3 */
        if (ST(L28_STORY_MARK_C48) != 0) {
            /* 0x7EC5 */
            if (ST(L28_STORY_MARK_C48) == 99 || ST(L28_STORY_MARK_C48) == 100) {
                /* 0x7EE9 */
                if (ST(L28_STORY_MARK_C48) == 99) {
                    ST(L28_STORY_MARK_C48) = 100;
                }
                ST(ST_LEVEL) = 28;
                CB->change_piece(27, 0);
            }
            /* 0x7F23 */
            if (ST(L28_STORY_MARK_C48) == 111) {
                ST(ST_LEVEL) = 28;
                CB->change_piece(27, 1);
            }
        } else {
            ST(ST_LEVEL) = 28;
            CB->change_piece(27, 1);
        }
        CB->select_pile(0, 1);
    }
    /* 0x7F7F */
    if (ST(ST_VIE(P_ARCHIVIST)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_ARCHIVIST) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x7FB4 */
    if (CB->divers(DV_WAS_HIT, P_ARCHIVIST, 0) && ST(ST_VIE(P_ARCHIVIST)) != VIE_DEAD) {
        ST(ST_VIE(P_ARCHIVIST)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_ARCHIVIST, -1);
        CB->stop_joindre(-P_ARCHIVIST);
        CB->divers_empile(DV_BLOOD_AT_BONE, 5, 0);
        CB->animation("SP05IMP51");
        CB->animation("P05CAD");
        CB->pos_perso(P_ARCHIVIST, 0, 0, 0, 2, "P05CAD", 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x8063 */
    if (ST(ST_HERO) == 4 && ST(L28_DEATH_HANDLED) == 0) {
        ST(L28_DEATH_HANDLED) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x80A0 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x80C6 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x80EF */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x811B */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x8147 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x816B */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x8188 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
    /* 0x81C7 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && CB->direction_perso(P_ARCHIVIST)) {
        ST(V_CRI_USED) = 1;
        ST(L28_ARCHIVIST_BATTLE_CRY) = 99;
        /* 0x825E */
        if (ST(L28_ARCHIVIST_BATTLE_CRY) != 0) {
            ST(L28_ARCHIVIST_BATTLE_CRY) = 5;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(34, 1000);
        return;
    }
    /* 0x829F */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0x82C0 */
        if (ST(L28_ARCHIVIST_BATTLE_CRY) == 5) {
            ST(L28_ARCHIVIST_BATTLE_CRY) = 99;
            CB->dialogue("L28D22A", P_ARCHIVIST);
            CB->dialogue("L28D22B", P_ARCHIVIST);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x832D */
    if (ST(L28_ARCHIVIST_BATTLE_CRY) == 99) {
        ST(L28_ARCHIVIST_BATTLE_CRY) = 5;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000D4FA).
 * Callback ingestion (0x1000D4FA..0x1000D86F), the trigger-table fill and
 * the unused dword_1001FD0C = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu28_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000DAA2);
 * every other path returns state[0].
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
        l28_trig00();   /* 0x1000DAA9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000D98E - the hero is dead: play the death ending */
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
 * TICK (opcode -1): drain pending sound, run body.
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000D4E0).
 * Jump table @ 0x1000DB79, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu28_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu28: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l28_trigger(arg);             break;  /* dword_1001FCC0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
