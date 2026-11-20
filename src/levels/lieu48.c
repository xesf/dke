/*
 * lieu48.c — Native hand-port of LIEU48.DLL (level-48 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu48_trig.c (reached through
 * l48_trigger, see lieu48_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000DFC0, the state
 * pointer global is dword_1001E41C and the trigger table is
 * dword_1001E330[42].
 *
 * event4, event5, event6, lieu_action_register, lieu_pre_scene and
 * sub_10001200 (float reset) are the shared lieu_common.c. The tick body
 * (sub_1000805C) is level-48 specific. The DLL's only writable global is
 * the sprintf buffer (0x1001E130), which holds no state.
 */

#include <stdio.h>

#include "lieu48_int.h"

/* Per-frame body — sub_1000805C (thunk 0x100010B9), level-48 specific.
 * `return` == original `jmp` to the epilogue. */
static void tick_body(void) {
    static c8 buf1[256];   /* 0x1001E130 */
    static c8 buf2[256];
    static c8 buf3[256];
    static c8 buf4[256];

    snprintf(buf1, sizeof(buf1), "combat_on : %x combat_on : %d", ST(V_ENGAGED_PERSONA), ST(V_ENGAGED_PERSONA));
    CB->print_msg(buf1, 0, 420);
    i32 t1 = CB->on_joindre(P_KALHI13);
    snprintf(buf2, sizeof(buf2), "test9: %d combat_on: %x onjoindre 13 : %d  E02 : %d ", ST(L48_KALHI_FLEE_STEP), ST(V_ENGAGED_PERSONA), t1, ST(V_EVENT_2));
    CB->print_msg(buf2, 0, 450);
    /* 0x80F2 */
    if ((ST(V_ENGAGED_PERSONA) & 0xFF) != 0x13 && ST(ST_VIE(P_KALHI13)) == 1 && ST(L48_KALHI_STORY_STAGE) == 8) {
        i32 t2 = CB->divers(DV_HP, P_KALHI13, -1);
        snprintf(buf3, sizeof(buf3), "NRJ KALHI:%d         ", t2);
        CB->print_msg(buf3, 0, 400);
        /* 0x8168 */
        if (CB->divers(DV_HP, P_KALHI13, -1) < 100) {
            CB->divers(DV_HP, P_KALHI13, 100);
        }
    }
    /* 0x8189 */
    if (((ST(V_ENGAGED_PERSONA) >> 8) & 0xFF) != 0x63 && ST(ST_VIE(P_MALROT3)) == 1 && ST(L48_KALHI_STORY_STAGE) == 8) {
        i32 t3 = CB->divers(DV_HP, P_MALROT3, -1);
        snprintf(buf4, sizeof(buf4), "NRJ MALFRAT:%d        ", t3);
        CB->print_msg(buf4, 0, 410);
        /* 0x8201 */
        if (CB->divers(DV_HP, P_MALROT3, -1) < 200) {
            CB->divers(DV_HP, P_MALROT3, 200);
        }
    }
    /* 0x8228 */
    if (ST(V_EVENT_1) == 1) {
        CB->select_pile(1, 1);
        ST(V_EVENT_1) = 0;
        /* 0x8259 */
        if (ST(V_GUARD_FIGHT_DONE) >= 2) {
            CB->animation("248eauN");
        } else {
            CB->animation("248eauL");
        }
        CB->select_pile(0, 1);
    }
    /* 0x8293 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_KALHI13) && ST(ST_VIE(P_KALHI13)) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12)) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        lieu_action_register("L04D41", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8379 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_MALROT7) && ST(ST_VIE(P_MALROT7)) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13)) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->combat(P_MALROT7, 0);
        ST(V_ENGAGED_PERSONA) |= 0x6700;
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x845F */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_MALROT3) && ST(ST_VIE(P_MALROT3)) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12)) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        lieu_action_register("L48D35", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
        CB->combat(P_MALROT3, 0);
        ST(V_ENGAGED_PERSONA) |= 0x6300;
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x855E */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        ST(V_END_GAME_REQ) = 0;
        CB->evenement(0x2C, 0x5DC);
    }
    /* 0x85B0 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        ST(V_DEATH_EVENT_SENT) = 1;
        /* 0x85F0 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x8619 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x8645 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x8671 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8695 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        } else {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x86D9 */
    if (CB->divers(DV_WAS_HIT, P_MALROT7, 0) && ST(ST_VIE(P_MALROT7)) < 2) {
        CB->select_pile(0, 2);
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(ST_VIE(P_MALROT7)) = VIE_DEAD;
        CB->divers_empile(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_MALROT7, 0);
        CB->combat(P_MALROT7, -1);
        ST(V_ENGAGED_PERSONA) &= 0xFF;
        CB->stop_joindre(-P_MALROT7);
        CB->animation("SP67IMP52");
        CB->animation("P67_65535");
        CB->animation("P67CAD");
        CB->pos_perso(P_MALROT7, 0, 0, 0, 2, "P67CAD", 0xD, 0);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x87D6 */
    if (CB->divers(DV_WAS_HIT, P_MALROT3, 0) && ST(ST_VIE(P_MALROT3)) < 2) {
        CB->select_pile(0, 2);
        CB->divers(DV_WAS_HIT, P_MALROT3, 1);
        CB->select_pile(1, 1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_MALROT3, 0);
        CB->divers_empile(DV_CUT_SPEECH, 0, 0);
        CB->change_camera(-1, CAM_AUTO);
        /* 0x885A */
        if (ST(L48_SCENE_STAGE) == 2) {
            CB->divers_empile(DV_CINEMATIC, 0, 0);
        }
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(ST_VIE(P_MALROT3)) = VIE_DEAD;
        CB->combat(P_MALROT3, -1);
        ST(V_ENGAGED_PERSONA) &= 0xFF;
        CB->stop_joindre(-P_MALROT3);
        CB->animation("SP63IMP52");
        CB->animation("P63_65535");
        CB->animation("P63CAD");
        CB->pos_perso(P_MALROT3, 0, 0, 0, 2, "P63CAD", 0xB, 0);
        CB->modify_var(&ST(L48_MALROT3_KILLED), 1);
        CB->change_camera(-1, CAM_AUTO);
    }
    /* 0x8931 */
    if (CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) < 2 && ST(L48_KALHI_STORY_STAGE) != 9) {
        CB->select_pile(0, 2);
        CB->divers(DV_WAS_HIT, P_KALHI13, 1);
        CB->select_pile(1, 1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_KALHI13, 0);
        CB->divers_empile(DV_CUT_SPEECH, 0, 0);
        CB->change_camera(-1, CAM_AUTO);
        /* 0x89C7 */
        if (ST(L48_SCENE_STAGE) == 2) {
            CB->divers_empile(DV_CINEMATIC, 0, 0);
        }
        CB->bloque(BLOQUE_ALL);
        ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
        ST(ST_SCRIPT_BUSY) = 1;
        CB->combat(P_KALHI13, -1);
        ST(V_ENGAGED_PERSONA) &= 0xFF00;
        CB->stop_joindre(-P_KALHI13);
        CB->evenement(0, -1);
        ST(V_EVENT_0) = 0;
        CB->animation("SP13IMP52");
        CB->animation("P13_65535");
        CB->animation("P13CAD");
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13CAD", 0xC, 0);
        CB->modify_var(&ST(L48_KALHI_STORY_STAGE), 20);
        CB->modify_var(&ST(L48_SCENE_STAGE), 4);
        CB->animation("Sar_10");
        /* 0x8AC7 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("LCOD94", P_ARKHAN2);
        }
        /* 0x8AE9 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("LCOD95", P_ARKHAN3);
        }
        /* 0x8B0B */
        if (ST(ST_VIE(P_MALROT3)) == 1) {
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_WAS_HIT, P_MALROT3, 1);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->combat(P_MALROT3, -1);
            ST(V_ENGAGED_PERSONA) &= 0xFF;
            CB->animation("SP63GAR02_10");
            /* 0x8B88 */
            if (CB->divers(DV_DESC_POS, P_MALROT3, 1) > 3300) {
                CB->change_camera(-1, CAM_AUTO);
            }
            CB->joindre(-P_MALROT3, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L48D106", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            CB->divers_empile(DV_RUN, P_MALROT3, 1);
            CB->joindre(P_MALROT3, 2032, 4140, 0x1B58, -1);
            /* 0x8C0E */
            if (ST(ST_PLAYER_WEAPON) != 27 && !CB->divers(DV_WAS_HIT, P_MALROT3, 0)) {
                CB->modify_var(&ST(L48_MALROT3_FIGHT_STARTED), 1);
            }
            CB->modify_var(&ST(L48_MALROT3_FLEE_STEP), 1);
            return;
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x8C90 */
    if ((ST(L48_KALHI_STORY_STAGE) == 8 && ST(ST_VIE(P_MALROT3)) == VIE_DEAD) || ST(L48_MALROT3_KILLED) == 1) {
        /* 0x8CC3 */
        if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(L48_KALHI_REACT_STAGE) == 0) {
            ST(L48_KALHI_REACT_STAGE) = 2;
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            CB->change_camera(-1, CAM_AUTO);
        } else if (ST(ST_VIE(P_KALHI13)) == 1 && ST(L48_KALHI_REACT_STAGE) == 0) {
            ST(L48_KALHI_REACT_STAGE) = 1;
            ST(L48_MALROT3_KILLED) = 2;
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            ST(L48_KALHI_FLEE_STEP) = 0;
            CB->divers(DV_COMBAT_STANCE, 0, 0);
            CB->combat(P_MALROT3, -1);
            ST(V_ENGAGED_PERSONA) &= 0xFF;
            CB->divers_empile(DV_CINEMATIC, 0, 0);
            CB->joindre_perm(ST(ST_HERO), P_KALHI13);
            CB->joindre_perm(-P_KALHI13, ST(ST_HERO));
            lieu_action_register("L48D13", P_KALHI13, "SP13DIA56", "SP13ARR01_10");
            lieu_action_register("L48D14", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            lieu_action_register("L48D15", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            lieu_action_register("L48D16", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            CB->divers_empile(DV_RUN, P_KALHI13, 1);
            /* 0x8E62 */
            if (CB->distance(6, 4) < 200) {
                CB->joindre(ST(ST_HERO), 2080, 3680, 0xACA2, -1);
            }
            CB->joindre(P_KALHI13, 2670, 3650, 0x1068, -1);
            CB->modify_var(&ST(L48_KALHI_RUN_ACTIVE), 1);
            CB->modify_var(&ST(L48_KALHI_FLEE_STEP), 1);
            CB->modify_var(&ST(ST_VIE(P_MALROT3)), 2);
            CB->modify_var(&ST(L48_KALHI_STORY_STAGE), 9);
            CB->change_camera(-1, CAM_AUTO);
            return;
        }
    }
    /* 0x8F20 */
    if (ST(L48_KALHI_STORY_STAGE) == 8 && ST(ST_VIE(P_KALHI13)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) & 0xFF) == 0x13) {
        ST(L48_KALHI_STORY_STAGE) = 20;
        ST(L48_SCENE_STAGE) = 4;
    }
    /* 0x8F6E */
    if (ST(L48_MALROT3_FIGHT_STARTED) == 0 && ST(L48_KALHI_STORY_STAGE) == 8 && ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_VIE(P_MALROT3)) == 1 && (ST(V_ENGAGED_PERSONA) & 0xFF) != 0x13) {
        ST(L48_KALHI_STORY_STAGE) = 20;
        ST(L48_SCENE_STAGE) = 4;
        CB->bloque(BLOQUE_ALL);
        ST(L48_MALROT3_FIGHT_STARTED) = 1;
        CB->dialogue("L48D51", P_MALROT3);
        CB->combat(P_MALROT3, 0);
        ST(V_ENGAGED_PERSONA) |= 0x6300;
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x903A */
    if (ST(L48_SCENE_STAGE) == 1 && ST(V_EVENT_0) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT3)) == 1 && ST(ST_VIE(P_KALHI13)) == 1 && ST(L48_KALHI_STORY_STAGE) != 9 && ST(L48_KALHI_STORY_STAGE) != 20) {
        ST(V_EVENT_0) = 0;
        CB->bloque(BLOQUE_ALL);
        ST(V_DLG_RANDOM) = CB->random(3);
        CB->joindre(-P_KALHI13, 2060, 3434, 0xFA0, -1);
        /* 0x9111 */
        if (ST(V_DLG_RANDOM) == 0) {
            lieu_action_register("L48D46", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D47", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            lieu_action_register("L48D48", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            CB->evenement(0, 10000);
        }
        /* 0x917B */
        if (ST(V_DLG_RANDOM) == 1) {
            lieu_action_register("L48D49", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            lieu_action_register("L48D50", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D51", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            CB->evenement(0, 10000);
        }
        /* 0x91E5 */
        if (ST(V_DLG_RANDOM) == 2) {
            lieu_action_register("L48D52", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D53", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            CB->evenement(0, 10000);
        }
        /* 0x9236 */
        if (ST(V_DLG_RANDOM) == 3) {
            lieu_action_register("L48D54", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D55", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            CB->evenement(0, 10000);
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x928F */
    if (ST(L48_SCENE_STAGE) == 1 && ST(ST_VIE(P_MALROT3)) == 1 && ST(ST_VIE(P_KALHI13)) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0x92DD */
        if ((CB->distance(6, 12) < 250 || CB->distance(6, 11) < 250) && ST(L48_PATROL_MEET_DONE) == 0 && ST(V_EVENT15_STEP) == 0) {
            CB->select_pile(0, 2);
            ST(V_EVENT_0) = 0;
            CB->evenement(0, -1);
            ST(V_EVENT15_STEP) = 1;
            ST(L48_PATROL_MEET_DONE) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->joindre(-P_MALROT3, 0, 0, 0, ST(ST_HERO));
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
            lieu_action_register("L48D06", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_MALROT3);
            lieu_action_register("L48D07A", P_MALROT3, "SP63DIA54", "SP63ARR01_10");
            lieu_action_register("L48D07B", P_MALROT3, "SP63DIA54", "SP63ARR01_10");
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
            lieu_action_register("L48D08A", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D08B", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D08C", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            CB->joindre(-P_KALHI13, 0, 0, 0, P_MALROT3);
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_MALROT3);
            CB->animation("Sar_10");
            lieu_action_register("L48D09", P_MALROT3, "SP63DIA54", "SP63ARR01_10");
            CB->joindre(-P_MALROT3, 0, 0, 0, P_KALHI13);
            CB->animation("SP13GAR02_10");
            CB->animation("SP63GAR02_10");
            CB->bloque(BLOQUE_NONE);
            CB->evenement(0, 3000);
            CB->select_pile(0, 1);
        }
        /* 0x954F */
        if (CB->distance(6, 11) > 320 && ST(ST_LOCKED_CAMERA) == 2 && ST(L48_PATROL_MEET_DONE) == 1 && ST(V_EVENT15_STEP) == 1) {
            CB->select_pile(0, 2);
            ST(V_EVENT_0) = 0;
            CB->evenement(0, -1);
            ST(V_EVENT15_STEP) = 2;
            CB->bloque(BLOQUE_ALL);
            CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
            lieu_action_register("L48D43", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            CB->joindre(-P_KALHI13, 0, 0, 0, P_MALROT3);
            lieu_action_register("L48D44", P_MALROT3, "SP63DIA54", "SP63ARR01_10");
            lieu_action_register("L48D45", P_MALROT3, "SP63DIA60", "SP63GAR02_10");
            CB->animation("Sar_10");
            CB->animation("SP13GAR02_10");
            CB->bloque(BLOQUE_NONE);
            CB->evenement(0, 2000);
            CB->select_pile(0, 1);
        }
    }
    /* 0x96AA */
    if (ST(L48_SCENE_STAGE) == 2 && ST(ST_VIE(P_MALROT3)) == 1 && ST(ST_VIE(P_KALHI13)) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0x96F8 */
        if ((CB->distance(6, 12) < 250 || CB->distance(6, 11) < 250) && ST(L48_PATROL_MEET_DONE) == 0 && ST(V_EVENT15_STEP) == 0) {
            ST(L48_PATROL_MEET_DONE) = 1;
            ST(V_EVENT15_STEP) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->divers_empile(DV_CINEMATIC, 0, 0);
            CB->animation("SP13GAR02_10");
            CB->animation("SP63GAR02_10");
            CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
            lieu_action_register("L48D10", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_MALROT3);
            CB->joindre(-P_KALHI13, 0, 0, 0, P_MALROT3);
            lieu_action_register("L48D11A", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            lieu_action_register("L48D11B", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            CB->animation("SP13GAR02_10");
            CB->animation("Sar_10");
            CB->divers_empile(DV_CINEMATIC, 1, 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x9885 */
        if (CB->distance(6, 11) > 320 && ST(ST_LOCKED_CAMERA) > 0 && ST(L48_PATROL_MEET_DONE) == 1 && ST(V_EVENT15_STEP) == 1) {
            ST(V_EVENT15_STEP) = 2;
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->divers_empile(DV_CINEMATIC, 0, 0);
            CB->animation("SP13GAR02_10");
            CB->animation("SP63GAR02_10");
            lieu_action_register("L48D12", P_KALHI13, "SP13DIA54", "SP13GAR02_10");
            lieu_action_register("L48D11A", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            lieu_action_register("L48D11B", P_MALROT3, "SP63DIA54", "SP63GAR02_10");
            CB->animation("SP13GAR02_10");
            CB->divers_empile(DV_CINEMATIC, 1, 0);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0x9999 */
    if (ST(L48_MALROT3_FLEE_STEP) == 1 && !CB->on_joindre(P_MALROT3)) {
        ST(L48_MALROT3_FLEE_STEP) = 2;
        ST(ST_VIE(P_MALROT3)) = 4;
        CB->pos_perso(P_MALROT3, 0, 0, 0, 4, "P63ARR01", 0xB, 0);
        /* 0x99F8 */
        if (ST(ST_VIE(P_MALROT7)) != VIE_DEAD) {
            CB->pos_perso(P_MALROT7, 2281, 1751, 0x12C, 1, "P67ARR01", 0xD, 0);
        }
        CB->animation("Sar_10");
        CB->change_camera(-1, CAM_AUTO);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9A63 */
    if (ST(L48_KALHI_FLEE_STEP) == 1 && !CB->on_joindre(P_KALHI13)) {
        ST(L48_KALHI_RUN_ACTIVE) = 0;
        ST(L48_KALHI_FLEE_STEP) = 2;
        CB->change_camera(0, CAM_AUTO);
        CB->pos_perso(P_KALHI13, 0, 0, 0, 4, "P13ARR01", 0xC, 0);
        /* 0x9ACC */
        if (ST(ST_VIE(P_MALROT7)) != VIE_DEAD) {
            CB->pos_perso(P_MALROT7, 2281, 1751, 0x12C, 1, "P67ARR01", 0xD, 0);
        }
        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9B3B */
    if (ST(L48_KALHI_STORY_STAGE) != 8 && ST(ST_VIE(P_MALROT3)) == 0 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < 0x960 && ST(L48_MALROT7_ALERT) == 0 && ST(ST_VIE(P_MALROT7)) < 2 && ST(V_ENGAGED_PERSONA) != 0x6700) {
        ST(L48_MALROT7_ALERT) = 1;
        /* 0x9BC1 */
        if (ST(V_ARMAL_STAGE) < 2) {
            CB->animation("SP67SIF01");
        }
        /* 0x9BDE */
        if (ST(V_ARMAL_STAGE) >= 2) {
            CB->animation("SP67SIF02");
        }
        CB->animation("P67ARR01_10");
    }
    /* 0x9C17 */
    if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > 0xAF0 && ST(L48_MALROT7_ALERT) == 1 && ST(ST_VIE(P_MALROT7)) < 2 && ST(L48_KALHI_STORY_STAGE) != 8 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L48_MALROT7_ALERT) = 0;
    }
    /* 0x9C74 */
    if (ST(L48_MALROT7_AMBUSH_STEP) == 1 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > 0x92E && ST(L48_KALHI_STORY_STAGE) != 8 && ST(ST_VIE(P_MALROT7)) < 2) {
        CB->bloque(BLOQUE_ALL);
        ST(L48_MALROT7_AMBUSH_STEP) = 2;
        ST(L48_MALROT7_MODE) = 2;
        CB->animation("SP67ACT01");
        CB->animation("P67_65535");
        CB->animation("P67GAR26_10");
        CB->animation("P67GAR02_10");
        CB->vire_col(-2, 0);
        CB->modify_var(&ST(L48_MALROT7_AMBUSH_STEP), 3);
    }
    /* 0x9D3D */
    if (ST(L48_MALROT7_AMBUSH_STEP) == 3 && ST(ST_VIE(P_MALROT7)) < 2 && ST(L48_MALROT7_ALERT) != 10) {
        ST(L48_MALROT7_AMBUSH_STEP) = 4;
        ST(L48_MALROT7_ALERT) = 1;
        lieu_action_register("L48D02", P_MALROT7, "SP67DIA59", "SP67GAR02_10");
        CB->combat(P_MALROT7, 0);
        ST(V_ENGAGED_PERSONA) |= 0x6700;
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9DBF */
    if (ST(L48_MALROT7_ARMED) == 1 && ST(L48_KALHI_STORY_STAGE) != 8 && ST(L48_MALROT7_FROM_L30) == 0 && ST(ST_VIE(P_MALROT7)) < 2) {
        ST(L48_MALROT7_ALERT) = 10;
        ST(L48_MALROT7_AMBUSH_STEP) = 4;
        ST(L48_MALROT7_ARMED) = 2;
        CB->vire_col(16, 1);
        CB->animation("SP67GAR02_10");
        CB->combat(P_MALROT7, 0);
        ST(V_ENGAGED_PERSONA) |= 0x6700;
        CB->dialogue("L48D03", P_MALROT7);
    }
    /* 0x9E6C */
    if (((ST(V_ENGAGED_PERSONA) >> 8) & 0xFF) == 0x67) {
        i32 t4 = CB->divers(DV_DESC_POS, ST(ST_HERO), 1);
        /* 0x9EA0 */
        if (t4 < CB->divers(DV_DESC_POS, P_MALROT7, 1) && ST(L48_MALROT7_BARRIER_ON) == 1 && CB->distance(6, 13) > 200) {
            ST(L48_MALROT7_BARRIER_ON) = 0;
            CB->vire_col(16, 0);
        }
    }
    /* 0x9EF8 */
    if (((ST(V_ENGAGED_PERSONA) >> 8) & 0xFF) == 0x67 && ST(L48_MALROT7_BARRIER_ON) == 0) {
        i32 t5 = CB->divers(DV_DESC_POS, ST(ST_HERO), 1);
        /* 0x9F3E */
        if (t5 > CB->divers(DV_DESC_POS, P_MALROT7, 1) || CB->distance(6, 13) <= 200) {
            ST(L48_MALROT7_BARRIER_ON) = 1;
            CB->vire_col(16, 1);
        }
    }
    /* 0x9F8A */
    if (ST(ST_VIE(P_MALROT7)) == VIE_DEAD && ((ST(V_ENGAGED_PERSONA) >> 8) & 0xFF) == 0x67) {
        ST(V_ENGAGED_PERSONA) &= 0xFF;
    }
    /* 0x9FB7 */
    if (ST(ST_VIE(P_MALROT3)) == VIE_DEAD && ((ST(V_ENGAGED_PERSONA) >> 8) & 0xFF) == 0x63) {
        ST(V_ENGAGED_PERSONA) &= 0xFF;
    }
    /* 0x9FE4 */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) & 0xFF) == 0x13) {
        ST(V_ENGAGED_PERSONA) &= 0xFF00;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000E359).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001E49C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu48_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000E582);
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
        l48_trig00();   /* 0x1000E589: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000E45C — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000E5DC): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000DFC0).
 * Jump table @ 0x1000E659, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu48_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu48: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l48_trigger(arg);             break;  /* dword_1001E330[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
