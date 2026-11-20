/*
 * lieu29.c - Native hand-port of LIEU29.DLL (level-29 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu29_trig.c (reached through
 * l29_trigger, see lieu29_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000BF40, the state
 * pointer global is dword_1001C4DC and the trigger table is
 * dword_1001C3F0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000C60C), lieu_pre_scene and sub_10001200
 * (float reset) are the shared lieu_common.c; the entry and INIT are
 * instruction-for-instruction identical to LIEU25. tick_body (sub_10007A84)
 * is level-29 specific. The level code touches no DLL .data globals, so INIT
 * has nothing to reset.
 */

#include <stdlib.h>

#include "lieu29_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10007A84 (thunk 0x100010B9), level-29 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue (0x100088F3).
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x7A93 */
    if (ST(L29_PHEDORIA_SENT_AWAY) == 0 && ST(V_LORY_STAGE) != 1 && ST(V_GUARD_FIGHT_DONE) == 2 && CB->distance(1, 3) > 350) {
        ST(L29_PHEDORIA_SENT_AWAY) = 1;
        CB->select_pile(2, 1);
        CB->joindre(-P_PHEDORIA, 7672, -4468, 0xD042, -1);
        CB->select_pile(0, 1);
    }
    /* 0x7B29 */
    if (ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD && ST(V_EVENT15_STEP) == 1 && CB->distance(1, 3) > 300) {
        CB->joindre(P_PHEDORIA, 7738, -4538, 0xF798, -1);
        ST(V_EVENT15_STEP) = 0;
    }
    /* 0x7B90 */
    if (ST(L29_LORY_NEAR) == 0 && ST(ST_VIE(P_LORY)) != VIE_DEAD && CB->distance(1, 2) < 100) {
        ST(L29_LORY_NEAR) = 1;
        CB->joindre(P_LORY, 0, 0, 0, ST(ST_HERO));
        return;
    }
    /* 0x7BF8 */
    if (ST(L29_LORY_NEAR) == 1 && CB->distance(1, 2) > 350) {
        ST(L29_LORY_NEAR) = 0;
        CB->joindre(P_LORY, 0, 0, 0x4CFA, -1);
    }
    /* 0x7C44 */
    if (ST(L29_PHEDORIA_WALKING) == 1) {
        CB->select_pile(2, 1);
        /* 0x7C67 */
        if (!CB->on_joindre(P_PHEDORIA)) {
            ST(L29_PHEDORIA_WALKING) = 0;
            CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->joindre(ST(ST_HERO), 0, 0, 0, P_PHEDORIA);
            CB->animation("G01ARR13");
            CB->animation("SG26FRA01");
            /* 0x7CD1 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
                CB->animation("SG02ATT07");
            }
            /* 0x7D06 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
                CB->animation("SG03ATT07");
            }
            /* 0x7D3B */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SG01ATT07");
            }
            CB->animation("P26GAR03");
        }
        CB->select_pile(0, 1);
    }
    /* 0x7D70 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x7D91 */
        if (ST(ST_LOCKED_CAMERA) == 0) {
            CB->change_camera(2, CAM_LOCKED);
        }
        CB->joindre(P_LORY, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP15GAR26");
        CB->animation("P15ATA22");
        CB->evenement(0x22, 200);
    }
    /* 0x7DF6 */
    if (ST(V_EVENT_1) == 1 && ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD) {
        CB->select_pile(1, 1);
        ST(V_EVENT_1) = 0;
        CB->animation("SP26GAR03");
        CB->evenement(2, 600);
        CB->animation("SG26FRA01");
        CB->select_pile(0, 1);
    }
    /* 0x7E6C */
    if (ST(V_EVENT_2) == 1 && ST(ST_HERO) != 4) {
        CB->select_pile(0, 2);
        ST(V_EVENT_2) = 0;
        /* 0x7EAC */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            CB->animation("SG02ATT07");
        }
        /* 0x7EE1 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            CB->animation("SG03ATT07");
        }
        /* 0x7F16 */
        if (ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD) {
            CB->animation("P26GAR03");
            lieu_action_register("L29D201", P_PHEDORIA, "SP26DIA50", "SP26ARR01_10");
        }
        CB->modify_var(&ST(ST_HERO), 4);
    }
    /* 0x7F63 */
    if (ST(V_EVENT_33) == 1) {
        CB->select_pile(1, 1);
        ST(V_EVENT_33) = 0;
        CB->joindre_perm(P_PHEDORIA, ST(ST_HERO));
        CB->animation("SG26FRA01");
        CB->evenement(2, 600);
        CB->select_pile(0, 1);
    }
    /* 0x7FD2 */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0x7FF3 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SP02IMP53");
        }
        /* 0x8010 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SP03IMP53");
        }
        CB->animation("P15GAR04");
        lieu_action_register("L29D202", P_LORY, "SP15DIA50", "SP15ARR01_10");
        CB->modify_var(&ST(ST_HERO), 4);
    }
    /* 0x8065 */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && ST(L29_PHEDORIA_STANCE_WARNED) == 0 && ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD && ST(V_LORY_STAGE) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(L29_PHEDORIA_STANCE_WARNED) = 1;
        CB->animation("SP26GAR03");
        CB->divers(DV_LOOK_AT_PLAYER, P_PHEDORIA, 1);
        CB->joindre_perm(-P_PHEDORIA, ST(ST_HERO));
        CB->dialogue("L29D203", P_PHEDORIA);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8129 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L29_PHEDORIA_STANCE_WARNED) == 1 && ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(L29_PHEDORIA_STANCE_WARNED) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_PHEDORIA, 0);
        CB->dialogue("L29D204", P_PHEDORIA);
        CB->stop_joindre(-P_PHEDORIA);
        CB->joindre(P_PHEDORIA, 0, 0, 0, P_LORY);
        CB->animation("SP26ARR01");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x81D4 */
    if (CB->divers(DV_WAS_HIT, P_PHEDORIA, 0) && ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->retire_zone(7);
        CB->bloque(BLOQUE_ALL);
        CB->divers_empile(DV_BLOOD_AT_BONE, 26, 1);
        CB->combat(P_PHEDORIA, -1);
        ST(V_ENGAGED_PERSONA) = P_PHEDORIA;
        CB->stop_joindre(-P_PHEDORIA);
        CB->animation("SP26IMP53");
        CB->animation("P26CAD");
        CB->pos_perso(P_PHEDORIA, 0, 0, 0, 2, "P26CAD", 3, 0);
        ST(ST_VIE(P_PHEDORIA)) = VIE_DEAD;
        CB->evenement(0, 300);
        CB->select_pile(0, 1);
    }
    /* 0x82BA */
    if (CB->divers(DV_WAS_HIT, P_LORY, 0) && ST(ST_VIE(P_LORY)) != VIE_DEAD) {
        ST(ST_VIE(P_LORY)) = VIE_DEAD;
        CB->select_pile(2, 2);
        CB->retire_zone(7);
        CB->bloque(BLOQUE_ALL);
        CB->divers_empile(DV_BLOOD_AT_BONE, 15, 1);
        CB->combat(P_LORY, -1);
        CB->stop_joindre(-P_LORY);
        CB->animation("SP15IMP54");
        CB->animation("P15CAD");
        CB->pos_perso(P_LORY, 0, 0, 0, 2, "P15CAD", 3, 0);
        CB->evenement(0x21, 300);
        CB->select_pile(0, 1);
    }
    /* 0x839D */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_PHEDORIA)) != VIE_DEAD && CB->direction_perso(P_PHEDORIA)) {
        ST(V_CRI_USED) = 1;
        ST(L29_PHEDORIA_CRY_TARGET) = 99;
        /* 0x8434 */
        if (ST(L29_PHEDORIA_CRY_TARGET) != 0) {
            ST(L29_PHEDORIA_CRY_TARGET) = P_PHEDORIA;
        }
        CB->bloque(BLOQUE_ALL);
        /* 0x8460 */
        if (ST(L29_PHEDORIA_CRY_STAGE) == 0) {
            CB->evenement(0x23, 800);
        }
        /* 0x8482 */
        if (ST(L29_PHEDORIA_CRY_STAGE) == 1) {
            CB->evenement(0x23, 600);
        }
        return;
    }
    /* 0x849D */
    if (ST(V_EVENT_35) == 1) {
        ST(V_EVENT_35) = 0;
        /* 0x84CA */
        if (ST(L29_PHEDORIA_CRY_STAGE) == 0 && ST(L29_PHEDORIA_CRY_TARGET) == P_PHEDORIA) {
            ST(L29_PHEDORIA_CRY_STAGE) = 1;
            CB->dialogue("L29D52", P_PHEDORIA);
            CB->animation("SP26ARR01_100");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->animation("SP26ARR01");
            CB->bloque(BLOQUE_NONE);
            return;
        }
        if (ST(L29_PHEDORIA_CRY_STAGE) == 1 && ST(L29_PHEDORIA_CRY_TARGET) == P_PHEDORIA) {
            CB->bloque(BLOQUE_ALL);
            CB->dialogue("L29D53", P_PHEDORIA);
            CB->animation("SP26ARR01_100");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->animation("SP26ARR01");
            CB->joindre(P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->evenement(1, 200);
            return;
        }
    }
    /* 0x8601 */
    if (ST(L29_PHEDORIA_CRY_TARGET) == 99) {
        ST(L29_PHEDORIA_CRY_TARGET) = P_PHEDORIA;
    }
    /* 0x8630 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_LORY)) != VIE_DEAD && CB->direction_perso(P_LORY)) {
        ST(V_CRI_USED) = 1;
        ST(L29_LORY_CRY_TARGET) = 99;
        /* 0x86C7 */
        if (ST(L29_LORY_CRY_TARGET) != 0) {
            ST(L29_LORY_CRY_TARGET) = P_LORY;
        }
        CB->bloque(BLOQUE_ALL);
        ST(V_PLAYER_STRIKE) = -1;
        ST(V_ENGAGED_PERSONA) = -1;
        CB->animation("SP15DIA51");
        CB->animation("SP26GAR03");
        CB->dialogue("L29D47", P_PHEDORIA);
        CB->evenement(1, 100);
        CB->animation("Sar_20");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        return;
    }
    /* 0x8762 */
    if (ST(ST_HERO) == 4 && ST(L29_DEATH_HANDLED) == 0) {
        ST(L29_DEATH_HANDLED) = 1;
        CB->evenement(0x2C, 3000);
    }
    /* 0x87A5 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x87CB */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x87F4 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x8820 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x884C */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8870 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x888D */
        if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_PASSAGE_OPEN) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x88BC */
        if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_PASSAGE_OPEN) >= 1) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000C2D9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001C55C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu29_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000C502);
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
        l29_trig00();   /* 0x1000C509: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000C3F5 - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000BF40).
 * Jump table @ 0x1000C5D9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu29_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu29: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l29_trigger(arg);             break;  /* dword_1001C3F0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
