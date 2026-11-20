/*
 * lieu25.c - Native hand-port of LIEU25.DLL (level-25 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu25_trig.c (reached through
 * l25_trigger, see lieu25_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x1000EB80, the state pointer global is dword_1001FC8C and the
 * trigger table is dword_1001FBA0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000F24C), lieu_pre_scene (sub_1000F3FC) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are instruction-for-instruction identical to LIEU03. tick_body
 * (sub_1000809D) is level-25 specific. The level code touches no DLL .data
 * globals, so INIT has nothing to reset.
 */

#include <stdlib.h>

#include "lieu25_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_1000809D (thunk 0x100010B9), level-25 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x80AF */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(0x2C, 3000);
    }
    /* 0x80F2 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x8123 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x814C */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x8178 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x81A4 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x81C8 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x81E5 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x8211 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->select_pile(1, 1);
        CB->change_camera(2, CAM_AUTO);
        CB->pos_perso(P_RYLSADHAR20, 0, 0, 0, 4, "", -1, 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x828B */
    if (CB->divers(DV_WAS_HIT, P_RYLSADHAR20, 0) && ST(ST_VIE(P_RYLSADHAR20)) != VIE_DEAD) {
        ST(L25_RYLSADHAR_SCENE_ENDED) = 1;
        CB->pos_perso(P_RYLSADHAR20, 0, 0, 0, 4, "", -1, 0);
        ST(ST_VIE(P_RYLSADHAR20)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_RYLSADHAR20);
        CB->divers_empile(DV_BLOOD_AT_BONE, 20, 0);
        CB->animation("SP20IMP99");
        CB->animation("P20CAD");
        CB->pos_perso(P_RYLSADHAR20, 0, 0, 0, 2, "P20CAD", 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x8355 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x8376 */
        if (ST(L25_DOOR_EVENT0_DONE) == 0) {
            ST(L25_DOOR_EVENT0_DONE) = 1;
            /* 0x8394 */
            if (ST(L25_RYLSADHAR_INTRO_DONE) == 1 && ST(V_RYLSADHAR_ZED_STAGE) != 7) {
                ST(V_RYLSADHAR_ZED_STAGE) = 7;
            }
            CB->select_pile(2, 1);
            CB->evenement(0, 1250);
            CB->play_anim(0, 0);
            CB->select_pile(2, 1);
            CB->bruitage("porte1", 6575, -3670);
            CB->select_pile(0, 1);
            return;
        } else {
            ST(ST_LEVEL) = 25;
            /* 0x8436 */
            if (ST(ST_HERO) == P_ARKHAN1 && ST(V_RYLSADHAR_ZED_STAGE) == 10) {
                CB->change_piece(23, 2);
            }
            /* 0x846A */
            if (ST(V_ROUTE_STAGE) == 3) {
                CB->change_piece(23, 1);
            } else {
                CB->change_piece(23, 0);
            }
        }
    }
    /* 0x849E */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x84BC */
        if (ST(L25_DOOR_EVENT1_STEP) == 0) {
            CB->select_pile(1, 1);
            ST(L25_DOOR_EVENT1_STEP) = 1;
            CB->evenement(1, 500);
            CB->bruitage("L25B01", -1, -1);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x851B */
        if (ST(L25_DOOR_EVENT1_STEP) == 1) {
            CB->select_pile(2, 1);
            ST(L25_DOOR_EVENT1_STEP) = 2;
            CB->play_anim(1, 0);
            CB->evenement(1, 2500);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x8575 */
        if (ST(L25_DOOR_EVENT1_STEP) == 2) {
            ST(L25_RYLSADHAR_GATE_OPENED) = 1;
            ST(V_GUARD_WAVE) = 2;
            ST(L25_DOOR_EVENT1_STEP) = 0;
            ST(ST_LEVEL) = 25;
            CB->change_piece(26, 0);
        }
    }
    /* 0x85CA */
    if (ST(ST_HERO) == P_ARKHAN1 && ST(L25_RYLSADHAR_INTRO_DONE) == 0 && ST(V_RYLSADHAR_ZED_STAGE) < 7 && !CB->on_joindre(P_RYLSADHAR20)) {
        ST(L25_RYLSADHAR_INTRO_DONE) = 1;
        CB->select_pile(1, 1);
        CB->vire_col(8, 0);
        CB->joindre(-P_RYLSADHAR20, 6670, -3030, 0x7530, -1);
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP20LEV01");
        CB->animation("P20ARR02_50");
        CB->modify_var(&ST(V_RYLSADHAR_ZED_STAGE), 7);
        CB->animation("P20ARR02");
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x86AD */
    if ((ST(ST_HERO) == P_ARKHAN2 || ST(ST_HERO) == P_ARKHAN3) && ST(V_RYLSADHAR_ZED_STAGE) != 9 && ST(L25_RYLSADHAR_SCENE_ENDED) == 0) {
        /* 0x86F2 */
        if (ST(L25_RYLSADHAR_TALK_STATE) == 1) {
            ST(L25_RYLSADHAR_KALHI_STATE) = 1;
            ST(L25_RYLSADHAR_TALK_STATE) = 2;
            CB->select_pile(2, 1);
            CB->pos_perso(P_KALHI13, 6593, -3641, 0xD4DB, 1, "P13ARR01", 3, 651);
            CB->bloque(BLOQUE_ALL);
            CB->change_camera(0, CAM_LOCKED);
            CB->play_anim(0, 0);
            CB->pause_anim(0, 6, 0);
            CB->dialogue("L25D15", P_KALHI13);
            CB->joindre(P_KALHI13, 6771, -3418, 0x18C3, -1);
            CB->modify_var(&ST(L25_KALHI_MEETING_STEP), 1);
            CB->select_pile(0, 1);
        }
        /* 0x87D5 */
        if (ST(ST_LOCKED_CAMERA) == 0 && ST(L25_KALHI_MEETING_STEP) != 10 && CB->distance(3, 7) < 50) {
            CB->select_pile(2, 1);
            CB->retire_zone(7);
            CB->change_camera(2, CAM_LOCKED);
            CB->change_camera(1, CAM_LOCKED);
            i32 t1 = CB->distance(1, 19);
            /* 0x8862 */
            if (t1 > CB->distance(1, 18)) {
                CB->pos_perso(ST(ST_HERO), 6777, -3066, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            } else {
                CB->pos_perso(ST(ST_HERO), 6656, -3131, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            }
            CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
            CB->select_pile(0, 1);
        }
        /* 0x8924 */
        if (ST(ST_LOCKED_CAMERA) == 1 && ST(L25_KALHI_MEETING_STEP) == 1 && CB->distance(1, 3) < 100) {
            ST(L25_KALHI_MEETING_STEP) = 99;
            CB->select_pile(0, 1);
            CB->joindre(ST(ST_HERO), 0, 0, 0, 13);
            CB->select_pile(1, 1);
            CB->joindre(P_RYLSADHAR20, 0, 0, 0, 13);
            CB->select_pile(2, 1);
            CB->change_camera(2, CAM_AUTO);
            CB->stop_joindre(P_KALHI13);
            CB->animation("SP13ARR02_50");
            i32 t2 = CB->distance(1, 19);
            /* 0x8A01 */
            if (t2 > CB->distance(1, 18)) {
                CB->pos_perso(P_KALHI13, 6771, -3183, CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P13ARR02", 3, 651);
            } else {
                CB->pos_perso(P_KALHI13, 6677, -3190, CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P13ARR02", 3, 651);
            }
            CB->animation("SP13ARR02_50");
            CB->modify_var(&ST(L25_KALHI_MEETING_STEP), 2);
            CB->select_pile(0, 1);
        }
        /* 0x8AAD */
        if (ST(L25_KALHI_MEETING_STEP) == 1 && ST(L25_RYLSADHAR_TALK_STATE) == 0) {
            ST(L25_KALHI_MEETING_STEP) = 99;
            CB->select_pile(1, 1);
            CB->joindre(-P_RYLSADHAR20, 0, 0, 0, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x8B19 */
        if (ST(L25_KALHI_MEETING_STEP) == 2) {
            ST(L25_KALHI_MEETING_STEP) = 99;
            CB->select_pile(2, 1);
            CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->animation("P13ARR02");
            lieu_action_register("L25D16A", P_KALHI13, "SP13DIA53", "SP13ARR02_10");
            lieu_action_register("L25D16B", P_KALHI13, "SP13DIA50", "SP13ARR02_10");
            lieu_action_register("L25D100A", P_RYLSADHAR20, "SP20DIA51", "SP20ARR01_10");
            lieu_action_register("L25D100B", P_RYLSADHAR20, "SP20DIA54", "SP20ARR01_10");
            CB->animation("Sar_50");
            CB->modify_var(&ST(L25_KALHI_MEETING_STEP), 3);
            CB->select_pile(0, 1);
        }
        /* 0x8BFC */
        if (ST(L25_KALHI_MEETING_STEP) == 3) {
            ST(L25_KALHI_MEETING_STEP) = 99;
            CB->select_pile(1, 1);
            CB->joindre(P_RYLSADHAR20, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L25D17A", P_RYLSADHAR20, "SP20DIA59", "SP20ARR01_10");
            lieu_action_register("L25D17B", P_RYLSADHAR20, "SP20DIA58", "SP20ARR01_10");
            lieu_action_register("L25D17C", P_RYLSADHAR20, "SP20DIA57", "SP20ARR01_10");
            CB->animation("Sar_50");
            CB->modify_var(&ST(L25_KALHI_MEETING_STEP), 4);
            CB->select_pile(0, 1);
        }
        /* 0x8CBB */
        if (ST(L25_KALHI_MEETING_STEP) == 4) {
            ST(L25_KALHI_MEETING_STEP) = 99;
            CB->divers_empile(DV_MUSIC, 1, 4);
            CB->divers_empile(DV_MUSIC, 1, 1);
            CB->select_pile(1, 1);
            /* 0x8D18 */
            if (CB->divers(DV_DESC_POS, P_RYLSADHAR20, 2) < 0xC000 && CB->divers(DV_DESC_POS, P_RYLSADHAR20, 2) > 0x88B8) {
                CB->joindre(-P_RYLSADHAR20, 6667, -3208, 0x89F0, -1);
            }
            /* 0x8D6E */
            if (CB->divers(DV_DESC_POS, P_RYLSADHAR20, 2) < 0x88B8 && CB->divers(DV_DESC_POS, P_RYLSADHAR20, 2) > 0x4000) {
                CB->joindre(-P_RYLSADHAR20, 6827, -3208, 0x89F0, -1);
            }
            CB->divers(DV_LOOK_AT_PLAYER, P_RYLSADHAR20, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(L25_KALHI_MEETING_STEP), 5);
            CB->select_pile(0, 1);
        }
        /* 0x8DF4 */
        if (ST(L25_KALHI_MEETING_STEP) == 5 && !CB->on_joindre(P_RYLSADHAR20)) {
            ST(L25_KALHI_MEETING_STEP) = 99;
            CB->select_pile(1, 1);
            CB->vire_col(8, 1);
            CB->joindre(P_RYLSADHAR20, 6584, -3650, 0x40D5, -1);
            CB->modify_var(&ST(L25_KALHI_MEETING_STEP), 6);
            CB->select_pile(2, 1);
            lieu_action_register("L25D18A", P_KALHI13, "SP13DIA53", "SP13ARR02");
            lieu_action_register("L25D18B", P_KALHI13, "SP13DIA58", "SP13ARR02");
            lieu_action_register("L25D18C", P_KALHI13, "SP13DIA57", "SP13ARR02");
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->select_pile(0, 1);
        }
        /* 0x8EF0 */
        if (ST(L25_KALHI_MEETING_STEP) == 6 && !CB->on_joindre(P_RYLSADHAR20)) {
            ST(L25_KALHI_MEETING_STEP) = 10;
            ST(V_RYLSADHAR_ZED_STAGE) = 9;
            CB->select_pile(1, 1);
            CB->stop_joindre(-P_RYLSADHAR20);
            CB->change_camera(0, CAM_LOCKED);
            CB->play_anim(0, 0);
            CB->pause_anim(0, 6, 0);
            CB->animation("SP20ARR01");
            CB->joindre(-P_RYLSADHAR20, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP20ARR01");
            lieu_action_register("L25D93", P_RYLSADHAR20, "SP20DIA60", "SP20ARR01");
            CB->joindre(-P_RYLSADHAR20, 6584, -3650, 0x40D5, -1);
            CB->evenement(2, 500);
            CB->animation("SP20MAR01");
            CB->select_pile(0, 1);
        }
    }
    /* 0x9001 */
    if (ST(ST_VIE(P_RYLSADHAR20)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(L25_RYLSADHAR_SCENE_ENDED) == 0) {
        /* 0x9031 */
        if (ST(L25_RYLSADHAR_WATCHING) == 1 && CB->distance(1, 2) > 300) {
            CB->divers(DV_LOOK_AT_PLAYER, P_RYLSADHAR20, 0);
            ST(L25_RYLSADHAR_WATCHING) = 0;
        }
        /* 0x907C */
        if (ST(L25_RYLSADHAR_ALERT) == 1 && CB->distance(1, 2) > 300) {
            ST(L25_RYLSADHAR_STANCE_WARNED) = 0;
            ST(L25_RYLSADHAR_ALERT) = 0;
            /* 0x90C1 */
            if (ST(L25_RYLSADHAR_TALK_STATE) == 0) {
                CB->select_pile(1, 1);
                CB->joindre(-P_RYLSADHAR20, 6670, -3030, 0x7530, -1);
                CB->animation("SP20LEV01");
                CB->animation("P20ARR02_50");
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0x9122 */
    if (ST(V_EVENT_38) == 1) {
        ST(V_EVENT_38) = 0;
        CB->select_pile(1, 2);
        ST(ST_VIE(P_RYLSADHAR20)) = VIE_DEAD;
        CB->divers(DV_BLOOD_AT_BONE, 20, 0);
        CB->animation("SP20IMP51");
        CB->bloque(BLOQUE_ALL);
        lieu_action_register("L25D26", P_RYLSADHAR20, "P20ATT03", "SP20CAD");
        CB->animation("SP20CAD");
        CB->animation("Sar_50");
        CB->modify_var(&ST(ST_VIE(P_RYLSADHAR20)), 2);
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_RYLSADHAR20, 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x91F7 */
    if (ST(V_EVENT_39) == 1) {
        ST(V_EVENT_39) = 0;
        CB->select_pile(1, 2);
        ST(ST_VIE(P_RYLSADHAR20)) = VIE_DEAD;
        CB->divers(DV_BLOOD_AT_BONE, 20, 0);
        CB->animation("SP20IMP51");
        lieu_action_register("L25D55", P_RYLSADHAR20, "P20ATT03", "SP20CAD");
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_RYLSADHAR20, 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x9294 */
    if (ST(ST_VIE(P_RYLSADHAR20)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L25_RYLSADHAR_SCENE_ENDED) = 1;
        ST(V_RYLSADHAR_ZED_STAGE) = 10;
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x92DF */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        ST(V_KALHI_KILLED) = 1;
        ST(V_ENGAGED_PERSONA) = 0;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000EF19).
 * Callback ingestion (0x1000EB9A..0x1000ED61), the trigger-table fill and
 * the unused dword_1001FD0C = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu25_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000F142);
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
        l25_trig00();   /* 0x1000F149: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000F02E - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000EB80).
 * Jump table @ 0x1000F219, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu25_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu25: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l25_trigger(arg);             break;  /* dword_1001FBA0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
