/*
 * lieu19.c -- Native hand-port of LIEU19.DLL (level-19 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu19_trig.c (reached through
 * l19_trigger, see lieu19_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000D850, the state
 * pointer global is dword_1001E80C and the trigger table is
 * dword_1001E720[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register, lieu_pre_scene and sub_10001200 (float reset) are
 * the shared lieu_common.c; the entry and INIT are instruction-for-
 * instruction identical to LIEU16's. tick_body (sub_100080F7) is level-19
 * specific.
 */

#include <stdlib.h>

#include "lieu19_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body -- sub_100080F7 (thunk 0x100010B9), level-19 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x8109 */
    if (ST(ST_VIE(P_MALROT5)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_MALROT5) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(L19_MALROT_HOSTILE) = 0;
    }
    /* 0x8145 */
    if (ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_DELIA) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(V_DELIA_PHASE) = 20;
        ST(V_DELIA_KILLED) = 19;
    }
    /* 0x819E */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 12) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_DELIA)) != VIE_DEAD && CB->direction_perso(P_DELIA)) {
        ST(V_CRI_USED) = 1;
        /* 0x8226 */
        if (ST(L19_DELIA_CRY_STATE) != 99) {
            ST(L19_DELIA_CRY_STATE) = 10;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(34, 1000);
        return;
    }
    /* 0x8267 */
    if (ST(L19_DELIA_CRY_STATE) == 99) {
        ST(L19_DELIA_CRY_STATE) = 10;
    }
    /* 0x8296 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && CB->direction_perso(P_MALROT5)) {
        ST(V_CRI_USED) = 1;
        /* 0x831E */
        if (ST(L19_MALROT_CRY_STATE) != 99) {
            ST(L19_MALROT_CRY_STATE) = 65;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(2, 1000);
        return;
    }
    /* 0x835F */
    if (ST(L19_MALROT_CRY_STATE) == 99) {
        ST(L19_MALROT_CRY_STATE) = 65;
    }
    /* 0x8380 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        /* 0x83A1 */
        if (ST(L19_MALROT_CRY_STATE) == 65) {
            ST(L19_MALROT_CRY_STATE) = 99;
            CB->dialogue("L19D88", P_MALROT5);
            CB->combat(P_MALROT5, 0);
            ST(V_ENGAGED_PERSONA) = P_MALROT5;
            ST(L19_MALROT_HOSTILE) = 1;
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x8426 */
    if (ST(L19_KALHI_MALROT_SCENE) == 5 && ST(ST_VIE(P_KALHI13)) != VIE_DEAD && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(ST_LOCKED_CAMERA) == 1 && ST(L19_KALHI_SCENE_STARTED) == 0) {
        CB->bloque(BLOQUE_ALL);
        lieu_action_register("L19D01", P_KALHI13, "SP13DIA53", "SP13ARR01");
        lieu_action_register("L19D02", P_MALROT5, "SP65DIA54", "SP65ARR01");
        /* 0x84B7 */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->divers(DV_COMBAT_STANCE, 0, 0);
        }
        CB->joindre(P_ARKHAN2, 3894, 2952, 0xFA22, -1);
        /* 0x84F5 */
        if (!CB->on_joindre(P_ARKHAN2)) {
            ST(L19_KALHI_SCENE_STARTED) = 1;
            CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->evenement(0, 100);
        }
    }
    /* 0x853D */
    if (ST(V_EVENT_0) == 1 && ST(L19_KALHI_MALROT_SCENE) == 5) {
        ST(V_EVENT_0) = 0;
        lieu_action_register("L19D03A", P_KALHI13, "SP13DIA54", "SP13ARR01");
        lieu_action_register("L19D03B", P_KALHI13, "SP13DIA53", "SP13ARR01");
        lieu_action_register("L19D03C", P_KALHI13, "SP13DIA56", "SP13ARR01");
        /* 0x85BB */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L19D04", P_ARKHAN2, "SG02DIA02", NULL);
        }
        /* 0x85E3 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD65", P_ARKHAN3, "SG02DIA02", NULL);
        }
        /* 0x860B */
        if (ST(ST_PLAYER_WEAPON) != 0) {
            /* 0x861D */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02PRE73");
                CB->animation("ar");
            }
            /* 0x8645 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03PRE73");
                CB->animation("ar");
            }
        }
        CB->link_objet(13, "OB64B", 27, 1);
        lieu_pre_scene(ST(ST_HERO), P_KALHI13, 64, 1, 2);
        lieu_action_register("L19D06A", P_KALHI13, "SP13DIA53", "SP13ARR01");
        lieu_action_register("L19D06B", P_KALHI13, "SP13DIA54", "SP13ARR01");
        CB->evenement(1, 200);
    }
    /* 0x86DE */
    if (ST(V_EVENT_1) == 1 && ST(L19_KALHI_MALROT_SCENE) == 5) {
        ST(V_EVENT_1) = 0;
        CB->joindre(P_MALROT5, 4052, 3033, 0x7439, -1);
        CB->divers(DV_RUN, P_KALHI13, 1);
        CB->joindre(P_ARKHAN2, 3773, 2998, 0xB640, -1);
        CB->joindre(P_KALHI13, 3887, 2404, 0x75D5, -1);
        CB->modify_var(&ST(L19_KALHI_LEAVING), 1);
        /* 0x878B */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L19D07", P_ARKHAN2, "SG02DIA02", NULL);
        }
        /* 0x87B3 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L19D08", P_ARKHAN3, "SG02DIA02", NULL);
        }
        /* 0x87DB */
        if (ST(L19_MALROT_HOSTILE) == 1) {
            /* 0x87ED */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L19D94", P_MALROT5);
            }
            /* 0x880F */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L19D89", P_MALROT5);
            }
            CB->combat(P_MALROT5, 0);
            ST(V_ENGAGED_PERSONA) = P_MALROT5;
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8853 */
    if (!CB->on_joindre(P_KALHI13) && ST(L19_KALHI_LEAVING) == 1) {
        ST(L19_KALHI_MALROT_SCENE) = 6;
        ST(L19_KALHI_LEAVING) = 0;
        CB->pos_perso(P_KALHI13, 0, 0, 0, 4, "", -1, 0);
    }
    /* 0x88AB */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0x88CC */
        if (ST(L19_DELIA_CRY_STATE) == 10) {
            ST(L19_DELIA_CRY_STATE) = 99;
            lieu_action_register("L16D122", P_DELIA, "SP10DIA50", "SP10ARR01_10");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x892F */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(L19_MALROT_WATCHING_A2) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(ST_HERO) == P_ARKHAN2 && CB->distance(1, 3) < 200) {
        CB->bloque(BLOQUE_ALL);
        ST(L19_MALROT_WATCHING_A2) = 1;
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
        CB->joindre(-P_MALROT5, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP65GAR26");
        CB->animation("P65GAR02");
        CB->dialogue("L19D94", P_MALROT5);
        CB->joindre_perm(P_MALROT5, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8A25 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L19_MALROT_WATCHING_A2) == 1 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_HERO) == P_ARKHAN2 && CB->distance(1, 3) < 5000) {
        CB->bloque(BLOQUE_ALL);
        ST(L19_MALROT_WATCHING_A2) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT5, 0);
        CB->stop_joindre(-P_MALROT5);
        CB->animation("SP65GAR27");
        CB->animation("P65ARR01");
        /* 0x8AE7 */
        if (CB->distance(1, 3) < 300) {
            CB->dialogue("L19D95", P_MALROT5);
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8B0E */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(V_EVENT15_STEP) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(ST_HERO) == P_ARKHAN3 && ST(L19_MALROT_A3_STEP) != 1 && ST(L19_KALHI_MALROT_SCENE) != 5 && CB->distance(1, 3) < 200) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 1;
        ST(L19_MALROT_A3_STEP) = 1;
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
        CB->joindre(-P_MALROT5, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP65GAR26");
        CB->animation("P65GAR02");
        CB->dialogue("L19D94", P_MALROT5);
        CB->joindre_perm(P_MALROT5, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8C3D */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(L19_MALROT_A3_STEP) == 1 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(L19_MALROT_FIGHT_STARTED_A3) == 0 && CB->distance(1, 3) < 200) {
        ST(L19_MALROT_FIGHT_STARTED_A3) = 1;
        /* 0x8CAF */
        if (ST(V_EVENT15_STEP) == 0) {
            ST(V_EVENT15_STEP) = 1;
            CB->dialogue("L19D94", P_MALROT5);
        }
        ST(L19_MALROT_HOSTILE) = 1;
        ST(V_ENGAGED_PERSONA) = P_MALROT5;
        CB->combat(P_MALROT5, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8D12 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(V_EVENT15_STEP) == 1 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_HERO) == P_ARKHAN3 && CB->distance(1, 3) < 5000) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT5, 0);
        CB->stop_joindre(-P_MALROT5);
        CB->animation("SP65GAR27");
        CB->animation("P65ARR01");
        /* 0x8DDA */
        if (CB->distance(1, 3) < 300) {
            CB->dialogue("L19D95", P_MALROT5);
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8E0C */
    if (CB->divers(DV_WAS_HIT, P_MALROT5, 0) && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(L19_KALHI_MALROT_SCENE) != 5) {
        ST(ST_VIE(P_MALROT5)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_MALROT5, -1);
        ST(V_ENGAGED_PERSONA) = P_MALROT5;
        CB->stop_joindre(-P_MALROT5);
        CB->divers_empile(DV_BLOOD_AT_BONE, 65, 0);
        CB->animation("SP65IMP52");
        CB->animation("P65CAD");
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
        CB->pos_perso(P_MALROT5, 0, 0, 0, 2, "P65CAD", 3, 0);
        CB->select_pile(0, 1);
    }
    /* 0x8EEB */
    if (ST(V_DELIA_PHASE) == 3 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != P_DELIA) {
        CB->select_pile(2, 1);
        /* 0x8F44 */
        if (CB->distance(12, 1) < 100 && ST(L19_DELIA_CLOSE) == 0) {
            ST(L19_DELIA_CLOSE) = 1;
            CB->joindre_perm(-P_DELIA, ST(ST_HERO));
            CB->stop_joindre(P_DELIA);
            CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 0xC, 0);
        }
        /* 0x8FDF */
        if (ST(L19_DELIA_FOLLOWING) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0x9012 */
            if (CB->distance(12, 1) >= 180) {
                CB->divers(DV_RUN, P_DELIA, 0);
                ST(L19_DELIA_CLOSE) = 0;
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0x906B */
            if (CB->distance(12, 1) >= 300) {
                ST(L19_DELIA_CLOSE) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0x90D0 */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 12)) {
        ST(V_DELIA_PHASE) = 20;
        ST(V_DELIA_KILLED) = 19;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_DELIA, -1);
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0xC, 0);
        CB->select_pile(0, 1);
    }
    /* 0x91B4 */
    if (CB->distance(1, 3) < 200 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(L19_MALROT_A3_STEP) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(L19_KALHI_MALROT_SCENE) != 5 && ST(ST_HERO) == P_ARKHAN3) {
        CB->select_pile(1, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L19_MALROT_A3_STEP) = 1;
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
        CB->joindre(-P_MALROT5, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP65GAR26");
        CB->animation("P65GAR02");
        CB->dialogue("L19D89", P_MALROT5);
        CB->joindre_perm(P_MALROT5, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x92D6 */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(L19_KALHI_MALROT_SCENE) == 5) {
        ST(L19_KALHI_MALROT_SCENE) = 10;
    }
    /* 0x9309 */
    if (ST(ST_HERO) == 4 && ST(L19_DEATH_EVENT_SENT) == 0) {
        ST(L19_DEATH_EVENT_SENT) = 1;
        CB->evenement(44, 3000);
    }
    /* 0x934C */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x9372 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x939B */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x93C7 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x93F3 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x9417 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x9434 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000DBE9).
 * Callback ingestion (0x1000D859..0x1000DBDA), the trigger-table fill and
 * the unused dword_1001E88C = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu19_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000DE12);
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
        l19_trig00();   /* 0x1000DE19: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000DCFE — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000DE6C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000D850).
 * Jump table @ 0x1000DEE9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu19_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu19: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l19_trigger(arg);             break;  /* dword_1001E720[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
