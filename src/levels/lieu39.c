/*
 * lieu39.c - Native hand-port of LIEU39.DLL (level-39 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu39_trig.c (reached through
 * l39_trigger, see lieu39_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x1000CAC0, the state pointer global is dword_1001E7AC and the
 * trigger table is dword_1001E6C0[42].
 *
 * event4, event5, event6, lieu_action_register, lieu_pre_scene and the float
 * reset are the shared lieu_common.c; the entry and INIT follow LIEU38.
 * tick_body (sub_1000163D) is level-39 specific. The level code touches no
 * DLL .data global, so INIT resets nothing.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu39_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_1000163D (thunk 0x100010B9), level-39 specific.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x1648 */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x1686 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x16B2 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x16D8 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x1704 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x1730 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x175C */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x1779 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x17A5 - Leona was hit outside zone 2 */
    if (CB->divers(DV_WAS_HIT, P_LEONA, 0) && ST(ST_VIE(P_LEONA)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_LEONA, 1);
        CB->combat(P_LEONA, -1);
        ST(ST_VIE(P_LEONA)) = VIE_DEAD;
        CB->stop_joindre(-P_LEONA);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_LEONA, 0);
        CB->animation("SP14IMP52");
        CB->animation("P14CAD");
        CB->pos_perso(P_LEONA, 0, 0, 0, 2, "P14CAD", 2, 0);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x18A0 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        ST(L39_FIGHT_EVENT_PENDING) = 0;
        /* 0x18D0 */
        if (ST(L39_LEONA_STATE) == 10) {
            /* 0x18E2 */
            if (ST(ST_LOCKED_CAMERA) == 2) {
                ST(L39_LEONA_STATE) = 210;
            } else {
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                CB->joindre(P_LEONA, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP14GAR26");
                lieu_action_register("L39D107", P_LEONA, "SP14GAR03", "SP14GAR03");
                ST(V_ENGAGED_PERSONA) = P_LEONA;
                CB->combat(P_LEONA, 4);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
            }
        }
        /* 0x1995 */
        if (ST(L39_LEONA_STATE) == 2) {
            /* 0x19A7 */
            if (ST(ST_LOCKED_CAMERA) == 2) {
                ST(L39_LEONA_STATE) = 202;
            } else {
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                CB->joindre(P_LEONA, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L39D03A", P_LEONA, "SP14DIA54", "SP14ARR01");
                lieu_action_register("L39D03B", P_LEONA, "SP14DIA55", "SP14ARR01");
                CB->joindre_perm(P_LEONA, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
            }
        }
        /* 0x1A62 */
        if (ST(L39_LEONA_STATE) == 1) {
            /* 0x1A74 */
            if (ST(ST_LOCKED_CAMERA) == 2) {
                ST(L39_LEONA_STATE) = 201;
            } else {
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                CB->joindre(P_LEONA, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L39D02", P_LEONA, "SP14DIA57", "SP14ARR01");
                lieu_action_register("L39D01B", P_LEONA, "SP14DIA58", "SP14ARR01");
                CB->joindre_perm(P_LEONA, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0x1B2F */
    if (ST(ST_VIE(P_LEONA)) != VIE_DEAD && ST(ST_LOCKED_CAMERA) == 1) {
        /* 0x1B53 */
        if (ST(L39_LEONA_STATE) == 210) {
            ST(L39_LEONA_STATE) = 10;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre(P_LEONA, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP14GAR26");
            lieu_action_register("L39D107", P_LEONA, "SP14GAR03", "SP14GAR03");
            ST(V_ENGAGED_PERSONA) = P_LEONA;
            CB->combat(P_LEONA, 4);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x1C04 */
        if (ST(L39_LEONA_STATE) == 202) {
            ST(L39_LEONA_STATE) = 2;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre(P_LEONA, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L39D03A", P_LEONA, "SP14DIA50", "SP14ARR01");
            lieu_action_register("L39D03B", P_LEONA, "SP14DIA53", "SP14ARR01");
            CB->joindre_perm(P_LEONA, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x1CBD */
        if (ST(L39_LEONA_STATE) == 201) {
            ST(L39_LEONA_STATE) = 1;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre(P_LEONA, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L39D02", P_LEONA, "SP14DIA57", "SP14ARR01");
            lieu_action_register("L39D01B", P_LEONA, "SP14DIA56", "SP14ARR01");
            CB->joindre_perm(P_LEONA, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
    }
    /* 0x1D76 - ambience follows the camera */
    if (ST(L39_AMBIENCE_CAM2) == 0 && ST(ST_LOCKED_CAMERA) == 2) {
        CB->bruitage("L38AMB1", -1, -1);
        ST(L39_AMBIENCE_CAM2) = 1;
    }
    /* 0x1DBB */
    if (ST(L39_AMBIENCE_CAM2) == 1 && (ST(ST_LOCKED_CAMERA) == 0 || ST(ST_LOCKED_CAMERA) == 1)) {
        CB->bruitage("L39AMB1", -1, -1);
        ST(L39_AMBIENCE_CAM2) = 0;
    }
    /* 0x1E12 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x1E33 */
        if (ST(L39_LEONA_CRY_STAGE) == 0) {
            ST(L39_LEONA_CRY_STAGE) = 1;
            CB->select_pile(1, 1);
            CB->stop_joindre(P_LEONA);
            lieu_action_register("L39D04", P_LEONA, "SP14GAR03", "SP14GAR03");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x1EBB */
        if (ST(L39_LEONA_CRY_STAGE) == 1) {
            CB->select_pile(1, 1);
            CB->stop_joindre(P_LEONA);
            ST(V_ENGAGED_PERSONA) = P_LEONA;
            ST(L39_LEONA_STATE) = 10;
            lieu_action_register("L39D108", P_LEONA, "SP14GAR03", "SP14GAR03");
            CB->combat(P_LEONA, 4);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x1F62 */
    if (ST(ST_VIE(P_LEONA)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x1F8F */
    if (ST(ST_LOCKED_CAMERA) == 2 && ST(V_ENGAGED_PERSONA) != 0) {
        CB->select_pile(1, 2);
        CB->combat(P_LEONA, -1);
        ST(V_ENGAGED_PERSONA) = 0;
        ST(L39_LEONA_STATE) = 10;
        CB->select_pile(0, 1);
    }
    /* 0x1FF8 */
    if (ST(ST_LOCKED_CAMERA) != 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(L39_LEONA_STATE) == 10 && ST(L39_FIGHT_EVENT_PENDING) == 0) {
        CB->select_pile(1, 2);
        CB->combat(P_LEONA, 4);
        ST(V_ENGAGED_PERSONA) = P_LEONA;
        CB->select_pile(0, 1);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000CE4A).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001BECC = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu39_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000D082);
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
        l39_trig00();   /* 0x1000D089: call sub_10001073 -> trig00_10001200 */
        return 0;
    }
    /* 0x1000CF6E - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000CAC0).
 * Jump table @ 0x1000D159, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu39_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu39: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l39_trigger(arg);             break;  /* dword_1001E6C0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
