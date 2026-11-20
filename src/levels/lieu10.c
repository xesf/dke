/*
 * lieu10.c - Native hand-port of LIEU10.DLL (level-10 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu10_trig.c (reached through
 * l10_trigger, see lieu10_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. LIEU10 was linked with a different layout
 * from levels 1..9: the entry is at 0x10018240 (not behind the thunk block's
 * usual 0x10001046 slot), the state pointer global is dword_1002BE8C and the
 * trigger table is dword_1002BDA0[42].
 *
 * event4, event5 (sub_100022B4), event6, lieu_action_register
 * (sub_1001890C), pre90_scene, lieu_pre_scene (sub_10018ABC) and the float
 * reset are the shared lieu_common.c; the entry
 * and INIT are instruction-for-instruction the same as LIEU03. tick_body
 * (sub_10009CE7) and the pre-scene variant sub_10007400 are level-10
 * specific.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu10_int.h"

/* Global sprintf buffer of the debug overlay (dword_1002BBA0). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * sub_10007400 (thunk 0x1000103C) - Rylsadhar/Lory pre-scene: __fastcall
 * (ecx = a, edx = b); the caller also pushes 3 stack args (callee `ret 0xC`)
 * that it never reads. a = the hero side (SG%02dPRE80/82), b = the speaker.
 * ---------------------------------------------------------------------- */
static void pre_scene_10(i32 a, i32 b) {
    c8 buf[256];

    snprintf(buf, sizeof(buf), "SG%02dPRE80", a);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "G%02dPRE81", a);    /* sic: no leading 'S' */
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "SP%02dPRE61", b);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "P%02d_65535", b);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "SG%02dPRE82", a);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "P%02darr01_10", a);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "SP%02darr01_10", b);
    CB->animation(buf);
}

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10009CE7 (thunk 0x100010BE), level-10 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    snprintf(g_txt, sizeof(g_txt), "test12=%d", ST(L10_DIALOG_TIMER_ARMED));
    CB->print_msg(g_txt, 0, 0xF5);
    /* 0x9D26 */
    if (ST(L10_AMBIENT_ZONE_NEAR) == 0 && (ST(ST_LOCKED_CAMERA) == 3 || ST(ST_LOCKED_CAMERA) == 7)) {
        ST(L10_AMBIENT_ZONE_NEAR) = 1;
        /* 0x9D68 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->bruitage("L10AMB3", 4380, -1243);
        }
        /* 0x9D8F */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->bruitage("L10AMB4", 4380, -1243);
        }
    }
    /* 0x9DB3 */
    if (ST(L10_AMBIENT_ZONE_NEAR) == 1 && (ST(ST_LOCKED_CAMERA) == 2 || ST(ST_LOCKED_CAMERA) == 5 || ST(ST_LOCKED_CAMERA) == 0)) {
        ST(L10_AMBIENT_ZONE_NEAR) = 0;
        /* 0x9E07 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->bruitage("L10AMB1", 4380, -1243);
        }
        /* 0x9E2E */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->bruitage("L10AMB2", 4380, -1243);
        }
    }
    /* 0x9E55 */
    if (ST(ST_LOCKED_CAMERA) == 9 && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 3400) {
        CB->change_camera(3, CAM_AUTO);
    }
    /* 0x9E9A */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(44, 3000);
    }
    /* 0x9EDD */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x9F0E */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x9F37 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x9F63 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x9F8F */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x9FB3 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x9FD0 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x9FF9 */
    if (ST(L10_COMPANION_MODE) == 10 && CB->distance(30, 10) < 200 && ST(L10_COMPANION_MODE_SAVED) == 10) {
        ST(L10_COMPANION_MODE) = 1;
    }
    /* 0xA04C */
    if (CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) != VIE_DEAD && ST(L10_GUARDIAN15_CHASE_STEP) == 0) {
        ST(V_KALHI_KILLED) = 10;
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_KALHI13, -1);
        CB->stop_joindre(-P_KALHI13);
        CB->animation("SP13IMP52");
        CB->animation("P13CAD");
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13CAD", 0x1F, 0);
        CB->select_pile(0, 1);
    }
    /* 0xA11E */
    if (CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) != VIE_DEAD && ST(ST_LOCKED_CAMERA) == 7 && ST(L10_GUARDIAN15_CHASE_STEP) != 0 && ST(L10_GUARDIAN15_CHASE_STEP) < 5) {
        CB->bloque(BLOQUE_ALL);
        ST(V_KALHI_PHASE) = 10;
        ST(L10_GUARDIAN15_CHASE_STEP) = 5;
        CB->stop_joindre(-P_KALHI13);
        CB->divers_empile(DV_WAS_HIT, P_KALHI13, 1);
        lieu_action_register("L10D127", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
        CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D128", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        CB->joindre(-P_KALHI13, 0, 0, 0x4650, -1);
        /* 0xA251 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < 460) {
            CB->joindre(-ST(ST_HERO), 4400, 550, 0xC350, -1);
        }
        /* 0xA294 */
        if (CB->divers(DV_DESC_POS, P_KALHI13, 0) > 4460 || CB->divers(DV_DESC_POS, P_KALHI13, 2) < 750) {
            CB->joindre(-P_KALHI13, 4450, 800, -1, -1);
        }
        CB->animation("SP13ACT20");
        CB->animation("P13_65535");
        CB->divers_empile(DV_BIG_ANIM, 2, 0);
        CB->divers_empile(DV_MUSIC, 1, 4);
        ST(ST_LEVEL) = 10;
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
        CB->change_piece(14, -1);
    }
    /* 0xA34E */
    if (CB->divers(DV_WAS_HIT, P_KALHI33, 0) && ST(ST_VIE(P_KALHI33)) != VIE_DEAD && ST(L10_GUARDIAN15_CHASE_STEP) == 0) {
        ST(V_KALHI_KILLED) = 10;
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_KALHI33, -1);
        CB->stop_joindre(-P_KALHI33);
        CB->animation("SP33IMP52");
        CB->animation("P33CAD");
        CB->pos_perso(P_KALHI33, 0, 0, 0, 2, "P33CAD", 0x20, 0);
        CB->select_pile(0, 1);
    }
    /* 0xA420 */
    if (CB->divers(DV_WAS_HIT, P_KALHI33, 0) && ST(ST_VIE(P_KALHI33)) != VIE_DEAD && ST(ST_LOCKED_CAMERA) == 7 && ST(L10_GUARDIAN15_CHASE_STEP) != 0 && ST(L10_GUARDIAN15_CHASE_STEP) < 5) {
        CB->bloque(BLOQUE_ALL);
        ST(V_KALHI_PHASE) = 10;
        ST(L10_GUARDIAN15_CHASE_STEP) = 5;
        CB->stop_joindre(-P_KALHI33);
        CB->divers_empile(DV_WAS_HIT, P_KALHI33, 1);
        lieu_action_register("L10D127", P_KALHI33, "SP33DIA60", "SP33ARR01_10");
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI33);
        CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D128", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
        lieu_action_register("L10D57", P_KALHI33, "SP33DIA55", "SP33ARR01_10");
        /* 0xA556 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < 460) {
            CB->joindre(-ST(ST_HERO), 4400, 550, 0xC350, -1);
        }
        CB->joindre(-P_KALHI33, 0, 0, 0x4650, -1);
        /* 0xA5AF */
        if (CB->divers(DV_DESC_POS, P_KALHI33, 0) > 4460 || CB->divers(DV_DESC_POS, P_KALHI33, 2) < 750) {
            CB->joindre(-P_KALHI33, 4450, 800, -1, -1);
        }
        CB->animation("SP33ACT20");
        CB->animation("P33_65535");
        CB->divers_empile(DV_BIG_ANIM, 2, 0);
        CB->divers_empile(DV_MUSIC, 1, 4);
        ST(ST_LEVEL) = 10;
        CB->change_piece(14, -1);
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
    }
    /* 0xA669 */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(V_DELIA_KILLED) = 10;
        ST(V_DELIA_PHASE) = 20;
        CB->select_pile(1, 2);
        CB->combat(P_DELIA, -1);
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0x1E, 0);
        CB->select_pile(0, 1);
    }
    /* 0xA730 */
    if (ST(ST_VIE(P_GUARDIAN15)) == VIE_DEAD && ST(L10_GUARDIAN15_FIGHT) == 1) {
        ST(L10_GUARDIAN15_FIGHT) = 2;
    }
    /* 0xA75A */
    if (ST(V_ENGAGED_PERSONA) == P_GUARDIAN15 && ST(ST_VIE(P_GUARDIAN15)) == VIE_DEAD) {
        ST(L10_GUARDIAN15_RESULT) = 2;
        ST(V_ENGAGED_PERSONA) = 0;
        /* 0xA7A4 */
        if (CB->divers(DV_DESC_POS, P_GUARDIAN15, 1) > 315) {
            ST(L10_GUARDIAN15_TALK_DONE) = 1;
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0xA7DC */
            if (ST(ST_LOCKED_CAMERA) == 3) {
                CB->change_camera(7, CAM_LOCKED);
            }
            /* 0xA7FE */
            if (ST(ST_LOCKED_CAMERA) == 7) {
                CB->change_camera(3, CAM_LOCKED);
            }
            /* 0xA820 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                /* 0xA86C */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D51", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xA897 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D52", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D53", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                /* 0xA8DB */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D54", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xA906 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D55", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D56", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                CB->joindre(-P_KALHI13, 0, 0, 0x4650, -1);
            }
            /* 0xA960 */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI33);
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                /* 0xA9AC */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D51", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xA9D7 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D52", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D53", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                /* 0xAA1B */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D54", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xAA46 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D55", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D56", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
                CB->joindre(-P_KALHI33, 0, 0, 0x4650, -1);
            }
            /* 0xAAA0 */
            if (ST(ST_LOCKED_CAMERA) == 7) {
                CB->change_camera(3, CAM_AUTO);
            }
            /* 0xAAC2 */
            if (ST(ST_LOCKED_CAMERA) == 3) {
                CB->change_camera(7, CAM_AUTO);
            }
            CB->animation("sar_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0xAB08 */
    if ((ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_KALHI13) || (ST(ST_VIE(P_KALHI33)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_KALHI33)) {
        ST(V_KALHI_KILLED) = 10;
        ST(ST_SCRIPT_BUSY) = 0;
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0xAB71 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 2;
        CB->pause_anim(0, 3, -1);
        CB->play_anim(1, 0);
        CB->pause_anim(2, 3, -1);
        CB->play_anim(3, 0);
        CB->pause_anim(4, 3, -1);
        CB->play_anim(5, 0);
        CB->pause_anim(6, 3, -1);
        CB->play_anim(7, 0);
        CB->pause_anim(8, 0, -1);
        CB->play_anim(9, 0);
        CB->joindre(-P_SUNSEER1, 4647, -3500, 0x7530, -1);
        CB->joindre(-P_GUARDIAN15, 5070, -1700, 0x7530, -1);
        CB->animation("SG00DIA07");
        /* 0xAC6A */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L10D80A", P_ARKHAN2);
            CB->dialogue("L10D80B", P_ARKHAN2);
        }
        /* 0xAC9C */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L10D81A", P_ARKHAN3);
            CB->dialogue("L10D81B", P_ARKHAN3);
        }
        CB->animation("Sar_10");
        CB->modify_var(&ST(L10_SUNSEER_STEP), 1);
    }
    /* 0xACF0 */
    if (ST(V_ZED84_TALK_STAGE) == 1 && (ST(V_ARKHAN1_PROGRESS) >= 3 || (ST(V_ARKHAN1_PROGRESS) < 3 && (CB->distance(12, 1) < 100 || CB->distance(12, 2) < 100 || ST(L10_ZED_FORCE_MEET) == 1)))) {
        ST(V_ZED84_TALK_STAGE) = 2;
        CB->select_pile(2, 1);
        CB->stop_joindre(-P_ZED84);
        CB->joindre(-P_ZED84, 0, 0, 0, ST(ST_HERO));
        CB->dialogue("L10D01A", P_ZED84);
        CB->evenement(33, 1000);
        CB->select_pile(0, 1);
    }
    /* 0xADDB */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        ST(V_ZED84_TALK_STAGE) = 3;
        CB->bloque(BLOQUE_ALL);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_ZED84);
        /* 0xAE33 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            lieu_action_register("L10D96", P_ARKHAN1, "SG01DIA08", NULL);
        }
        /* 0xAE5B */
        if (ST(ST_SPECIAL_MODE) == 0) {
            lieu_action_register("L10D97", P_ARKHAN1, "SG01DIA08", NULL);
        }
        /* 0xAE83 */
        if (ST(ST_LOCKED_CAMERA) != 6) {
            CB->change_camera(6, CAM_LOCKED);
        }
        CB->dialogue("L10D01B", P_ZED84);
        CB->select_pile(2, 1);
        CB->joindre(-P_ZED84, 0, 0, 0, ST(ST_HERO));
        CB->select_pile(0, 1);
        /* 0xAEF6 */
        if (CB->distance(12, 10) > 125) {
            CB->animation("SP84ACT01");
            CB->evenement(2, 1600);
        } else {
            CB->animation("SP84ACT03");
            CB->evenement(1, 200);
        }
        CB->select_pile(0, 1);
    }
    /* 0xAF50 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->change_camera(6, CAM_AUTO);
        CB->divers_empile(DV_BLOOD_AT_BONE, 1, 0);
        CB->animation("SG01ATT07");
        CB->animation("P84ARR02_100");
        lieu_action_register("L10D30", P_ZED84, "SP84DIA59", "SP84ARR01_1000");
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
    }
    /* 0xAFF1 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        CB->change_camera(6, CAM_AUTO);
        CB->divers_empile(DV_BLOOD_AT_BONE, 1, 0);
        CB->animation("SP01IMP51");
        CB->animation("P84ACT02");
        CB->animation("P84ARR01_100");
        lieu_action_register("L10D30", P_ZED84, "SP84DIA59", "SP84ARR01_1000");
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
    }
    /* 0xB09A */
    if ((ST(L10_LORY_FOLLOW_REQ) == 1 || ST(L10_LORY_FOLLOW_STATE) == 1) && !CB->on_joindre(P_RYLSADHAR20)) {
        ST(V_ARKHAN1_PROGRESS) = 1;
        ST(L10_LORY_FOLLOW_REQ) = 0;
        CB->joindre(-P_RYLSADHAR20, 0, 0, 0, P_LORY);
        CB->modify_var(&ST(L10_LORY_FOLLOW_STATE), 2);
    }
    /* 0xB11F */
    if (CB->distance(10, 19) < 800 && ST(V_ARKHAN1_PROGRESS) == 1 && ST(ST_HERO) == P_ARKHAN1) {
        ST(V_ARKHAN1_PROGRESS) = 2;
        CB->bloque(BLOQUE_ALL);
        ST(V_RYLSADHAR_ZED_STAGE) = 1;
        CB->joindre_perm(P_RYLSADHAR20, P_LORY);
        CB->change_camera(8, CAM_LOCKED);
        lieu_action_register("L10D05A", P_RYLSADHAR20, "SP20DIA54", "SP20DIA53");
        lieu_action_register("L10D05B", P_RYLSADHAR20, "SP20DIA53", "SP20DIA56");
        lieu_action_register("L10D05C", P_RYLSADHAR20, "SP20DIA54", "SP20DIA56");
        lieu_action_register("L10D05D", P_RYLSADHAR20, "SP20DIA56", "SP20ARR01");
        lieu_action_register("L10D05E", P_RYLSADHAR20, "SP20DIA56", "SP20ARR01");
        lieu_action_register("L10D06", P_LORY, "SP15DIA53", "SP15ARR01");
        lieu_action_register("L10D07A", P_RYLSADHAR20, "SP20DIA60", "SP20DIA55");
        lieu_action_register("L10D07B", P_RYLSADHAR20, "SP20DIA55", "SP20ARR01");
        lieu_action_register("L10D08", P_LORY, "SP15DIA54", "SP15ARR01_10");
        lieu_action_register("L10D09", P_RYLSADHAR20, "SP20DIA50", "SP20ARR01_10");
        pre_scene_10(P_RYLSADHAR20, P_LORY);   /* the 3 stacked args are never read */
        CB->animation("P20ARR01_10");
        CB->animation("SP15ATT70");
        CB->dialogue("L10D10", P_LORY);
        CB->animation("P15ARR01_100");
        CB->animation("SP20ATT70");
        CB->dialogue("L10D11", P_RYLSADHAR20);
        CB->animation("P20ARR01_10");
        CB->animation("P15mar17");
        CB->animation("P15_65535");
        CB->joindre(-P_LORY, 4647, -3520, 0x7530, -1);
        CB->joindre(-P_RYLSADHAR20, 5320, -2640, 0x7530, -1);
        CB->change_camera(4, CAM_AUTO);
        CB->animation("ar_10");
        CB->dialogue("L10D12A", P_ARKHAN1);
        lieu_action_register("L10D12B", P_ARKHAN1, "SG00DIA05", "Sar_10");
        CB->evenement(35, 1500);
    }
    /* 0xB3A6 */
    if (ST(V_EVENT_35) == 1) {
        ST(V_EVENT_35) = 0;
        CB->bloque(BLOQUE_NONE);
        ST(L10_LORY_SCENE_DONE) = 1;
    }
    /* 0xB3DB */
    if (ST(L10_LORY_PLACED) == 0 && CB->distance(26, 20) <= 80) {
        ST(L10_LORY_PLACED) = 1;
        CB->pos_perso(P_LORY, 4647, -3520, 0x7530, 4, "", 0x1A, 0);
        ST(L10_LORY_SPAWN_TIME) = ST(ST_GAME_TIME);
        CB->retire_zone(20);
        CB->retire_zone(26);
    }
    /* 0xB467 */
    if (ST(V_RYLSADHAR_ZED_STAGE) == 1 && CB->distance(27, 21) <= 80 && ST(L10_RYLSADHAR_PLACED) == 0) {
        ST(L10_RYLSADHAR_PLACED) = 1;
        CB->pos_perso(P_RYLSADHAR20, 5320, -2640, 0xAFC8, 4, "", 0x1B, 0);
        ST(L10_RYLSADHAR_SPAWN_TIME) = ST(ST_GAME_TIME);
        CB->retire_zone(21);
        CB->retire_zone(27);
    }
    /* 0xB505 */
    if (ST(L10_SUNSEER_STEP) == 1 && CB->divers(DV_DESC_POS, P_SUNSEER1, 1) < -3350) {
        ST(L10_SUNSEER_STEP) = 2;
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB54B */
    if (ST(L10_SUNSEER_STEP) == 2 && !CB->on_joindre(P_SUNSEER1)) {
        CB->free_anim(-10);
        ST(L10_SUNSEER_STEP) = 3;
        CB->pos_perso(P_SUNSEER1, 4647, -3520, 0x7530, 4, "", 0x22, 0);
        CB->stop_joindre(-P_GUARDIAN15);
        CB->pos_perso(P_GUARDIAN15, 4480, 600, 0x88B8, 1, "P55ARR04", 0x21, 0);
    }
    /* 0xB5DF */
    if (ST(L10_LORY_PLACED) == 1 && ST(V_ARKHAN1_PROGRESS) == 2 && ST(ST_HERO) == P_ARKHAN1 && ST(L10_LORY_SCENE_DONE) == 1) {
        ST(V_ARKHAN1_PROGRESS) = 3;
        ST(L10_ZED_APPROACH_STEP) = 1;
        CB->ajoute_zone(8, 4540, 304, 80, 80, 0);
        CB->pos_perso(P_ZED84, 2300, -1550, 0xC350, 1, "P84ARR01", 0xC, 0);
        CB->link_objet(P_ZED84, "OB68", 0x1D, 1);
        CB->joindre(P_ZED84, 4540, 304, 0, -1);
    }
    /* 0xB6AC */
    if (ST(V_ARKHAN1_PROGRESS) >= 3 && ST(V_ZED84_TALK_STAGE) == 0 && ST(ST_HERO) == P_ARKHAN1) {
        /* 0xB6DF */
        if (ST(L10_ZED_APPROACH_STEP) == 1 && CB->distance(12, 10) < 150 && CB->divers(DV_ANGLE_TO, P_ZED84, ST(ST_HERO)) < 4000) {
            ST(L10_ZED_APPROACH_STEP) = 2;
            CB->stop_joindre(-P_ZED84);
            return;
        }
        /* 0xB74C */
        if (ST(L10_ZED_APPROACH_STEP) == 2 && (CB->distance(12, 10) >= 200 || CB->divers(DV_ANGLE_TO, P_ZED84, ST(ST_HERO)) >= 6000)) {
            ST(L10_ZED_APPROACH_STEP) = 1;
            CB->stop_joindre(-P_ZED84);
            CB->joindre(-P_ZED84, 4540, 304, 0, -1);
            return;
        }
        /* 0xB7D2 */
        if (ST(L10_ZED_APPROACH_STEP) != 0 && !CB->on_joindre(P_ZED84) && CB->distance(12, 8) <= 100) {
            ST(L10_ZED_APPROACH_STEP) = 0;
            CB->retire_zone(8);
            CB->stop_joindre(-P_ZED84);
            CB->animation("SP84ARR01_10");
        }
    }
    /* 0xB83D */
    if (ST(ST_HERO) == P_ARKHAN1 && ST(V_EVENT_34) == 0) {
        /* 0xB861 */
        if (ST(L10_DIALOG_TIMER_ARMED) == 0 && (ST(ST_LOCKED_CAMERA) == 3 || ST(ST_LOCKED_CAMERA) == 7)) {
            ST(L10_DIALOG_TIMER_ARMED) = 1;
            ST(V_EVENT_34) = 0;
            ST(V_DLG_RANDOM) = CB->random(3);
            /* 0xB8CC */
            if (ST(V_DLG_RANDOM) == 0) {
                CB->evenement(34, 2000);
            }
            /* 0xB8EE */
            if (ST(V_DLG_RANDOM) == 1) {
                CB->evenement(34, 4000);
            }
            /* 0xB910 */
            if (ST(V_DLG_RANDOM) == 2) {
                CB->evenement(34, 6000);
            }
        }
        /* 0xB932 */
        if (ST(L10_DIALOG_TIMER_ARMED) == 1 && ST(ST_LOCKED_CAMERA) != 3 && ST(ST_LOCKED_CAMERA) != 7) {
            ST(L10_DIALOG_TIMER_ARMED) = 0;
            ST(V_EVENT_34) = 0;
            CB->evenement(34, -1);
        }
    }
    /* 0xB996 */
    if (ST(ST_HERO) == P_ARKHAN1 && ST(V_EVENT_34) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_ZED_TALK_STAGE_7F4) < 2) {
        ST(V_EVENT_34) = 0;
        CB->select_pile(1, 1);
        ST(V_DLG_RANDOM) = CB->random(9);
        /* 0xBA11 */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->dialogue("L10D29A", -1);
        }
        /* 0xBA33 */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->dialogue("L10D29B", -1);
        }
        /* 0xBA55 */
        if (ST(V_DLG_RANDOM) == 2) {
            CB->dialogue("L10D29C", -1);
        }
        /* 0xBA77 */
        if (ST(V_DLG_RANDOM) == 3) {
            CB->dialogue("L10D29D", -1);
        }
        /* 0xBA99 */
        if (ST(V_DLG_RANDOM) == 4) {
            CB->dialogue("L10D29E", -1);
        }
        /* 0xBABB */
        if (ST(V_DLG_RANDOM) == 5) {
            CB->dialogue("L10D29F", -1);
        }
        /* 0xBADD */
        if (ST(V_DLG_RANDOM) == 6) {
            CB->dialogue("L10D29G", -1);
        }
        /* 0xBAFF */
        if (ST(V_DLG_RANDOM) == 7) {
            CB->dialogue("L10D29H", -1);
        }
        /* 0xBB21 */
        if (ST(V_DLG_RANDOM) == 8) {
            CB->dialogue("L10D29I", -1);
        }
        CB->modify_var(&ST(L10_DIALOG_TIMER_ARMED), 0);
        CB->select_pile(0, 1);
    }
    /* 0xBB64 */
    if (ST(ST_HERO) >= 2 && ST(ST_VIE(P_GUARDIAN15)) == 1 && ST(L10_GUARDIAN15_ALERT) == 0 &&
        (ST(V_KALHI_KILLED) == 1 ||
         ((ST(ST_LOCKED_CAMERA) == 3 || ST(ST_LOCKED_CAMERA) == 7) && CB->distance(10, 33) < 500 && ST(V_KALHI_KILLED) == 10) ||
         (ST(V_KALHI_PHASE) >= 7 && ST(L10_GUARDIAN15_RESULT) != 2 && ST(L10_GUARDIAN15_RESULT) != 3 && ST(L10_GUARDIAN15_FIGHT) == 0 && ST(ST_VIE(P_GUARDIAN15)) == 1 &&
          (ST(ST_VIE(P_KALHI13)) == 1 || ST(ST_VIE(P_KALHI33)) == 1) &&
          (ST(V_ENGAGED_PERSONA) == P_KALHI13 || ST(V_ENGAGED_PERSONA) == P_KALHI33)))) {
        CB->bloque(BLOQUE_ALL);
        /* 0xBC9E */
        if (ST(V_KALHI_KILLED) == 1 || ST(V_KALHI_KILLED) == 10) {
            ST(V_KALHI_KILLED) = 2;
        }
        /* 0xBCD1 */
        if (ST(V_KALHI_RECRUIT) == 0 || ST(L10_PLOT_BD0) == 0) {
            ST(V_KALHI_RECRUIT) = 99;
            ST(L10_PLOT_BD0) = 99;
        }
        ST(L10_GUARDIAN15_ALERT) = 1;
        CB->joindre_perm(-P_GUARDIAN15, ST(ST_HERO));
        CB->stop_joindre(P_GUARDIAN15);
        CB->animation("SP55GAR05_10");
        /* 0xBD4B */
        if (ST(L10_GUARDIAN15_WARNED) == 1) {
            CB->dialogue("L10D35", P_GUARDIAN15);
            CB->print_msg("Alerte 3", 0, 0x104);
        }
        /* 0xBD7F */
        if (ST(L10_GUARDIAN15_WARNED) == 0) {
            ST(L10_GUARDIAN15_WARNED) = 1;
            CB->dialogue("L10D34", P_GUARDIAN15);
            CB->print_msg("Alerte 2", 0, 0x104);
        }
        CB->animation("SP55GAR05_10");
        CB->animation("P55_65535");
        CB->evenement(38, 250);
        CB->animation("P55ATA25");
        CB->animation("P55GAR05");
        /* 0xBDFB */
        if (ST(V_ENGAGED_PERSONA) != P_KALHI13 && ST(V_ENGAGED_PERSONA) != P_KALHI33) {
            CB->joindre(P_GUARDIAN15, -1, -1, -1, ST(ST_HERO));
        }
        /* 0xBE39 */
        if (ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
        return;
    }
    /* 0xBE59 */
    if (((ST(L10_GUARDIAN15_ALERT) == 1 && ST(ST_HERO) < 4) ||
         (ST(V_RETURN_PART) == 1 && (ST(V_KALHI_KILLED) == 2 || (ST(V_KALHI_RECRUIT) == 99 && ST(L10_PLOT_BD0) == 99)) && ST(ST_HERO) < 4)) &&
        ST(V_EVENT_38) == 1) {
        CB->print_msg("je suis mort", 0, 0xF0);
        ST(L10_GUARDIAN15_ALERT) = 0;
        /* 0xBF04 */
        if (ST(V_ENGAGED_PERSONA) != P_KALHI13 && ST(V_ENGAGED_PERSONA) != P_KALHI33) {
            CB->stop_joindre(-P_GUARDIAN15);
        }
        CB->select_pile(2, 2);
        ST(V_EVENT_38) = 0;
        CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
        /* 0xBF67 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SP02IMP55");
        }
        /* 0xBF84 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SP03IMP55");
        }
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
        CB->select_pile(0, 1);
        return;
    }
    /* 0xBFE2 */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(V_KALHI_PHASE) == 7 && ST(L10_GUARDIAN15_AMBUSH_STEP) < 5) {
        ST(L10_KALHI_ESCORT_STARTED) = 1;
        /* 0xC02E */
        if (CB->distance(31, 10) < 120 && ST(L10_COMPANION_CATCHUP_STEP) == 0 && ST(L10_COMPANION_CLOSE) == 0) {
            /* 0xC06A */
            if (CB->divers(DV_RUN, P_KALHI13, 1) == 1) {
                CB->divers(DV_RUN, P_KALHI13, 0);
            }
            ST(L10_COMPANION_CLOSE) = 1;
            ST(L10_COMPANION_CHATTER_ARMED) = 1;
            CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
        }
        /* 0xC0CC */
        if (CB->distance(31, 10) < 300 && ST(L10_COMPANION_CATCHUP_STEP) == 1) {
            ST(L10_COMPANION_CATCHUP_STEP) = 2;
            CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 1);
        }
        /* 0xC130 */
        if (ST(L10_COMPANION_MODE) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0xC163 */
            if (CB->distance(31, 10) >= 180) {
                ST(L10_COMPANION_CLOSE) = 0;
                CB->divers(DV_RUN, P_KALHI13, 0);
                CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xC1BC */
            if (CB->distance(31, 10) >= 300) {
                ST(L10_COMPANION_CLOSE) = 0;
                CB->divers(DV_RUN, P_KALHI13, 1);
                CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
            }
        }
    }
    /* 0xC20C */
    if (ST(L10_KALHI_SWAP_REQ) == 1) {
        ST(L10_KALHI_SWAP_REQ) = 2;
        CB->divers_empile(DV_SCREEN, 5, 0);
        CB->pos_perso(P_KALHI13, -1, -1, -1, 4, "", 0x1F, 0);
        ST(ST_VIE(P_KALHI13)) = 4;
        CB->retire_zone(31);
        CB->pos_perso(P_KALHI33, CB->divers(DV_DESC_POS, P_KALHI13, 0), CB->divers(DV_DESC_POS, P_KALHI13, 1), CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P33ARR01", 0x20, 0);
        CB->divers_empile(DV_SCREEN, 3, 0);
        CB->animation("SP33ATT01");
        lieu_action_register("L10D129A", P_KALHI33, "P33DIA54", "SP33ARR01");
        lieu_action_register("L10D129B", P_KALHI33, "SP33DIA59", "SP33ARR01_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
        CB->divers(DV_IDLE_TIME, 1, 0);
        CB->divers(DV_LOOK_AT_PLAYER, P_KALHI33, 1);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xC368 */
    if (ST(ST_HERO) != 4 && ST(L10_COMPANION_CHATTER_ARMED) == 1 && CB->divers(DV_IDLE_TIME, 0, 0) >= 10000 && ST(ST_VIE(P_KALHI13)) == 1 && ST(L10_GUARDIAN15_AMBUSH_STEP) == 0) {
        ST(L10_COMPANION_CHATTER_ARMED) = 0;
        /* 0xC3D3 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            /* 0xC3E5 */
            if (ST(L10_COMPANION_CHATTER_LINE) == 2) {
                lieu_action_register("L10D98", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
            }
            /* 0xC410 */
            if (ST(L10_COMPANION_CHATTER_LINE) == 1) {
                ST(L10_COMPANION_CHATTER_LINE) = 2;
                lieu_action_register("L10D99", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            }
            /* 0xC44A */
            if (ST(L10_COMPANION_CHATTER_LINE) == 0) {
                ST(L10_COMPANION_CHATTER_LINE) = 1;
                lieu_action_register("L10D999", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
        }
        /* 0xC484 */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            /* 0xC496 */
            if (ST(L10_COMPANION_CHATTER_LINE) == 2) {
                lieu_action_register("L10D98", P_KALHI33, "SP33DIA60", "SP33ARR01_10");
            }
            /* 0xC4C1 */
            if (ST(L10_COMPANION_CHATTER_LINE) == 1) {
                ST(L10_COMPANION_CHATTER_LINE) = 2;
                lieu_action_register("L10D99", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            }
            /* 0xC4FB */
            if (ST(L10_COMPANION_CHATTER_LINE) == 0) {
                ST(L10_COMPANION_CHATTER_LINE) = 1;
                lieu_action_register("L10D999", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
        }
        CB->divers(DV_IDLE_TIME, 1, 0);
        CB->modify_var(&ST(L10_COMPANION_CHATTER_ARMED), 1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xC568 */
    if (ST(ST_VIE(P_KALHI33)) == 1 && ST(V_KALHI_PHASE) == 7 && ST(L10_GUARDIAN15_AMBUSH_STEP) < 5) {
        ST(L10_KALHI_ESCORT_STARTED) = 1;
        /* 0xC5B4 */
        if (CB->distance(32, 10) < 120 && ST(L10_COMPANION_CATCHUP_STEP) == 0 && ST(L10_COMPANION_CLOSE) == 0) {
            /* 0xC5F0 */
            if (CB->divers(DV_RUN, P_KALHI33, 1) == 1) {
                CB->divers(DV_RUN, P_KALHI33, 0);
            }
            ST(L10_COMPANION_CLOSE) = 1;
            ST(L10_COMPANION_CHATTER_ARMED) = 1;
            CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
        }
        /* 0xC652 */
        if (CB->distance(32, 10) < 300 && ST(L10_COMPANION_CATCHUP_STEP) == 1) {
            ST(L10_COMPANION_CATCHUP_STEP) = 2;
            CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
            CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 1);
        }
        /* 0xC6B6 */
        if (ST(L10_COMPANION_MODE) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0xC6E9 */
            if (CB->distance(32, 10) >= 180) {
                ST(L10_COMPANION_CLOSE) = 0;
                CB->divers(DV_RUN, P_KALHI33, 0);
                CB->joindre(-P_KALHI33, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xC742 */
            if (CB->distance(32, 10) >= 300) {
                ST(L10_COMPANION_CLOSE) = 0;
                CB->divers(DV_RUN, P_KALHI33, 1);
                CB->joindre(-P_KALHI33, -1, -1, -1, ST(ST_HERO));
            }
        }
    }
    /* 0xC792 */
    if (ST(ST_HERO) != 4 && ST(ST_HERO) >= 2 && ST(V_KALHI_PHASE) >= 7 && ST(ST_VIE(P_GUARDIAN15)) == 1 && ST(L10_GUARDIAN15_RESULT) != 2 && ST(L10_GUARDIAN15_RESULT) != 3 && (ST(ST_VIE(P_KALHI13)) == 1 || ST(ST_VIE(P_KALHI33)) == 1) && ST(L10_GUARDIAN15_FIGHT) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0xC840 */
        if (ST(L10_GUARDIAN15_AMBUSH_STEP) == 1 || (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) >= -700 && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 4770 && ST(L10_GUARDIAN15_AMBUSH_STEP) == 0) || (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) >= -600 && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) >= 4770 && ST(L10_GUARDIAN15_AMBUSH_STEP) == 0)) {
            ST(L10_GUARDIAN15_AMBUSH_STEP) = 2;
            CB->select_pile(0, 2);
            ST(L10_SAVED_CAMERA) = 0;
            /* 0xC92D */
            if (ST(ST_LOCKED_CAMERA) != 3) {
                ST(L10_SAVED_CAMERA) = ST(ST_LOCKED_CAMERA);
                CB->change_camera(3, CAM_LOCKED);
            }
            /* 0xC966 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->print_msg("test3 passe \340 100 ln 1533", 0, 0xF0);
                ST(L10_COMPANION_MODE) = 100;
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_KALHI13);
                /* 0xC9AC */
                if (ST(L10_COMPANION_CATCHUP_STEP) == 0) {
                    lieu_action_register("L10D42", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                }
                /* 0xC9D7 */
                if (ST(L10_COMPANION_CATCHUP_STEP) == 2) {
                    lieu_action_register("L10D32", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                }
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
                CB->animation("SP13ARR01_10");
            }
            /* 0xCA2A */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                ST(L10_COMPANION_MODE) = 100;
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_KALHI33);
                /* 0xCA5E */
                if (ST(L10_COMPANION_CATCHUP_STEP) == 0) {
                    lieu_action_register("L10D42", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
                }
                /* 0xCA89 */
                if (ST(L10_COMPANION_CATCHUP_STEP) == 2) {
                    lieu_action_register("L10D32", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
                }
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI33);
            }
            CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 3);
            CB->select_pile(0, 1);
        }
        /* 0xCAF5 */
        if (ST(L10_GUARDIAN15_AMBUSH_STEP) == 3) {
            ST(L10_GUARDIAN15_AMBUSH_STEP) = 4;
            CB->vire_col(10, 0);
            ST(L10_GUARDIAN15_RESULT) = 3;
            /* 0xCB32 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->stop_joindre(-P_KALHI13);
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L10D37A", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                lieu_action_register("L10D37B", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                /* 0xCBAF */
                if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 4000) {
                    CB->joindre(-P_KALHI13, 4770, -770, 0x6A4, -1);
                }
                /* 0xCBEE */
                if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 4000) {
                    CB->joindre(-P_KALHI13, 3800, -660, 0x3E8, -1);
                }
                CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 5);
            }
            /* 0xCC33 */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->stop_joindre(-P_KALHI33);
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L10D50", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
                /* 0xCC97 */
                if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 4000) {
                    CB->joindre(-P_KALHI33, 4770, -770, 0x6A4, -1);
                }
                /* 0xCCD6 */
                if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 4000) {
                    CB->joindre(-P_KALHI33, 3800, -660, 0x3E8, -1);
                }
                CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 5);
            }
        }
    }
    /* 0xCD1B */
    if (ST(L10_GUARDIAN15_AMBUSH_STEP) == 5 && (!CB->on_joindre(P_KALHI13) || !CB->on_joindre(P_KALHI33))) {
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 6;
        CB->vire_col(10, 1);
        ST(V_KALHI_KILLED) = 0;
        /* 0xCD81 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->stop_joindre(-P_KALHI13);
            CB->animation("SP13ARR01_10");
            CB->divers(DV_RUN, P_KALHI13, 1);
            CB->joindre(-P_KALHI13, 4630, 270, 0, -1);
            CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 7);
        }
        /* 0xCDEB */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            CB->stop_joindre(-P_KALHI33);
            CB->animation("SP33ARR01_10");
            CB->divers(DV_RUN, P_KALHI33, 1);
            CB->joindre(-P_KALHI33, 4630, 270, 0, -1);
            CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 7);
        }
    }
    /* 0xCE55 */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(L10_GUARDIAN15_AMBUSH_STEP) == 7 && !CB->on_joindre(P_KALHI13) && ST(V_KALHI_KILLED) != 2 && ST(V_KALHI_RECRUIT) != 99 && CB->distance(31, 33) < 250) {
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 8;
        CB->stop_joindre(-P_KALHI13);
        CB->animation("SP13ARR01_10");
        lieu_action_register("L10D73", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        lieu_action_register("L10D74", P_GUARDIAN15, "SP55DIA54", "SP55ARR04_10");
        lieu_action_register("L10D75", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        lieu_action_register("L10D76", P_GUARDIAN15, "SP55DIA54", "SP55ARR04_10");
        CB->ajoute_zone(28, 4420, -140, 70, 70, 0);
        CB->modify_var(&ST(L10_GUARDIAN15_CHASE_STEP), 1);
        CB->divers(DV_RUN, P_GUARDIAN15, 1);
        CB->joindre(-P_GUARDIAN15, 4420, -250, 0x7530, -1);
        CB->joindre(-P_KALHI13, 4500, 754, 0x4650, -1);
    }
    /* 0xCFD0 */
    if (ST(ST_VIE(P_KALHI33)) == 1 && ST(L10_GUARDIAN15_AMBUSH_STEP) == 7 && !CB->on_joindre(P_KALHI33) && CB->distance(32, 33) < 250) {
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 8;
        CB->stop_joindre(-P_KALHI33);
        CB->animation("SP33ARR01_10");
        lieu_action_register("L10D38A", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
        lieu_action_register("L10D38B", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
        lieu_action_register("L10D39", P_GUARDIAN15, "SP55DIA54", "SP55ARR04_10");
        lieu_action_register("L10D40", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
        lieu_action_register("L10D41", P_GUARDIAN15, "SP55DIA57", "SP55ARR04_10");
        CB->ajoute_zone(20, 4647, -3520, 100, 100, 1);
        CB->ajoute_zone(23, 5180, -1200, 100, 100, 1);
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L10_GUARDIAN15_AMBUSH_STEP), 8);
        CB->modify_var(&ST(L10_GUARDIAN15_CHASE_STEP), 1);
        ST(L10_GUARDIAN15_FIGHT) = 0;
        /* 0xD13C */
        if (ST(L10_SAVED_CAMERA) != 0) {
            CB->change_camera(ST(L10_SAVED_CAMERA), CAM_AUTO);
            ST(L10_SAVED_CAMERA) = 0;
        } else {
            CB->change_camera(3, CAM_AUTO);
        }
        CB->divers(DV_RUN, P_GUARDIAN15, 1);
        CB->joindre(-P_GUARDIAN15, 5180, -1200, 0x7530, -1);
        CB->joindre(-P_KALHI33, 4500, 754, 0x4650, -1);
    }
    /* 0xD1D2 */
    if (ST(L10_GUARDIAN15_AMBUSH_STEP) == 8 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI13)) == 1 && ST(ST_VIE(P_GUARDIAN15)) == 1 && CB->distance(33, 28) < 100 && CB->distance(33, 10) > 500) {
        CB->select_pile(1, 1);
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 9;
        CB->stop_joindre(-P_GUARDIAN15);
        CB->animation("SP55GAR05_10");
        CB->dialogue("L10D77", P_GUARDIAN15);
        /* 0xD290 */
        if (ST(L10_SAVED_CAMERA) != 0) {
            CB->change_camera(ST(L10_SAVED_CAMERA), CAM_AUTO);
            ST(L10_SAVED_CAMERA) = 0;
        } else {
            CB->change_camera(3, CAM_AUTO);
        }
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xD2F1 */
    if (ST(ST_HERO) >= 2 && ST(V_KALHI_PHASE) >= 7 && ST(L10_GUARDIAN15_RESULT) != 2 && ST(L10_GUARDIAN15_FIGHT) == 0 && ST(ST_VIE(P_GUARDIAN15)) == 1 && (ST(ST_VIE(P_KALHI13)) == 1 || ST(ST_VIE(P_KALHI33)) == 1) && (ST(L10_GUARDIAN15_AMBUSH_STEP) == 8 || ST(L10_GUARDIAN15_AMBUSH_STEP) == 9) && ((CB->distance(33, 10) < 500 && CB->divers(DV_ANGLE_TO, P_GUARDIAN15, ST(ST_HERO)) < 70000) || CB->distance(33, 10) < 150)) {
        /* 0xD3EA */
        if (ST(V_ENGAGED_PERSONA) == P_KALHI33) {
            CB->combat(P_KALHI33, -1);
        }
        /* 0xD409 */
        if (ST(V_ENGAGED_PERSONA) == P_KALHI13) {
            CB->combat(P_KALHI13, -1);
        }
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN15;
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 10;
        CB->stop_joindre(-P_GUARDIAN15);
        CB->joindre(-P_GUARDIAN15, 0, 0, 0, ST(ST_HERO));
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN15;
        CB->combat(P_GUARDIAN15, 0);
        ST(L10_GUARDIAN15_FIGHT) = 1;
        CB->animation("SP55GAR05_10");
        CB->dialogue("L10D34", P_GUARDIAN15);
        CB->select_pile(2, 1);
        /* 0xD4BE */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->stop_joindre(-P_KALHI13);
            lieu_action_register("L10D43", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
        }
        /* 0xD4F4 */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            CB->stop_joindre(-P_KALHI33);
            lieu_action_register("L10D43", P_KALHI33, "SP33DIA60", "SP33ARR01_10");
        }
        /* 0xD52A */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->divers(DV_RUN, P_KALHI13, 1);
            CB->joindre(-P_KALHI13, 4500, 754, 0x4650, -1);
        }
        /* 0xD56A */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            CB->divers(DV_RUN, P_KALHI33, 1);
            CB->joindre(-P_KALHI33, 4500, 754, 0x4650, -1);
        }
        ST(L10_GUARDIAN15_ALERT) = 1;
    }
    /* 0xD5B6 */
    if (ST(V_EVENT_39) == 1) {
        CB->select_pile(0, 1);
        /* 0xD5E3 */
        if (CB->divers(DV_WAS_HIT, ST(ST_HERO), 0)) {
            CB->divers(DV_WAS_HIT, ST(ST_HERO), 1);
            /* 0xD620 */
            if ((CB->divers(DV_HP, ST(ST_HERO), -1) - 200) > 0) {
                CB->divers(DV_HP, ST(ST_HERO), (CB->divers(DV_HP, ST(ST_HERO), -1) - 200));
            } else {
                CB->bloque(BLOQUE_ALL);
                CB->combat(P_GUARDIAN15, -1);
                CB->divers(DV_HP, ST(ST_HERO), 0);
            }
        }
        ST(V_EVENT_39) = 0;
    }
    /* 0xD6AD */
    if (ST(L10_GUARDIAN15_AMBUSH_STEP) == 8 && CB->distance(33, 23) < 100 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 9;
        CB->select_pile(2, 1);
        CB->retire_zone(23);
        CB->stop_joindre(-P_GUARDIAN15);
        CB->animation("SP55ARR04_10");
        CB->joindre(-P_GUARDIAN15, 4647, -3520, 0x8CA0, -1);
        CB->select_pile(0, 1);
    }
    /* 0xD750 */
    if (ST(L10_GUARDIAN15_AMBUSH_STEP) == 9 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(33, 20) < 100) {
        ST(ST_VIE(P_GUARDIAN15)) = 4;
        CB->select_pile(2, 1);
        ST(L10_GUARDIAN15_AMBUSH_STEP) = 10;
        CB->stop_joindre(-P_GUARDIAN15);
        CB->pos_perso(P_GUARDIAN15, 4480, 600, 0x88B8, 4, "P55ARR04", 0x21, 0);
        CB->select_pile(0, 1);
    }
    /* 0xD7F5 */
    if (ST(L10_GUARDIAN15_CHASE_STEP) == 1) {
        /* 0xD807 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && !CB->on_joindre(P_KALHI13)) {
            ST(L10_GUARDIAN15_CHASE_STEP) = 2;
            CB->stop_joindre(-P_KALHI13);
            CB->animation("SP13ARR01_10");
            CB->joindre(-P_KALHI13, 4500, 807, 0x4650, -1);
            CB->modify_var(&ST(L10_GUARDIAN15_CHASE_STEP), 3);
        }
        /* 0xD884 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && !CB->on_joindre(P_KALHI33)) {
            ST(L10_GUARDIAN15_CHASE_STEP) = 2;
            CB->stop_joindre(-P_KALHI33);
            CB->animation("SP33ARR01_10");
            CB->joindre(-P_KALHI33, 4500, 807, 0x4650, -1);
            CB->modify_var(&ST(L10_GUARDIAN15_CHASE_STEP), 3);
        }
    }
    /* 0xD901 */
    if (ST(L10_GUARDIAN15_CHASE_STEP) == 3) {
        /* 0xD913 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && CB->distance(31, 7) < 100 && !CB->on_joindre(P_KALHI13)) {
            ST(L10_GUARDIAN15_CHASE_STEP) = 4;
            CB->stop_joindre(-P_KALHI13);
        }
        /* 0xD96B */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->distance(32, 7) < 100 && !CB->on_joindre(P_KALHI33)) {
            ST(L10_GUARDIAN15_CHASE_STEP) = 4;
            CB->stop_joindre(-P_KALHI33);
        }
    }
    /* 0xD9CC */
    if (CB->distance(10, 7) < 250 && ST(L10_GUARDIAN15_CHASE_STEP) != 0 && ST(L10_GUARDIAN15_CHASE_STEP) < 5 && (ST(ST_VIE(P_GUARDIAN15)) > 1 || (ST(L10_GUARDIAN15_RESULT) == 3 && ST(L10_GUARDIAN15_FIGHT) != 1))) {
        ST(L10_GUARDIAN15_CHASE_STEP) = 5;
        CB->bloque(BLOQUE_ALL);
        /* 0xDA4F */
        if (ST(ST_LOCKED_CAMERA) != 7) {
            CB->change_camera(7, CAM_AUTO);
        }
        ST(V_KALHI_PHASE) = 10;
        /* 0xDA80 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            /* 0xDA92 */
            if (ST(L10_GUARDIAN15_TALK_DONE) == 0) {
                ST(L10_GUARDIAN15_TALK_DONE) = 1;
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                /* 0xDAED */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D51", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xDB18 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D52", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D53", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                /* 0xDB5C */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D54", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xDB87 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D55", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D56", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
            CB->joindre(-P_KALHI13, 0, 0, 0x4650, -1);
            /* 0xDBEC */
            if (CB->divers(DV_DESC_POS, P_KALHI13, 0) > 4460 || CB->divers(DV_DESC_POS, P_KALHI13, 2) < 750) {
                CB->joindre(-P_KALHI13, 4450, 800, -1, -1);
            }
            CB->animation("SP13ACT20");
            CB->animation("P13_65535");
        }
        /* 0xDC4A */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI33);
            CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
            /* 0xDC96 */
            if (ST(ST_VIE(P_GUARDIAN15)) == 1 && ST(L10_GUARDIAN15_AMBUSH_STEP) >= 7) {
                ST(ST_VIE(P_GUARDIAN15)) = 4;
                CB->pos_perso(P_GUARDIAN15, 4480, 600, 0x88B8, 4, "P55ARR04", 0x21, 0);
            }
            /* 0xDCEE */
            if (ST(L10_GUARDIAN15_RESULT) == 3) {
                /* 0xDD00 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D47", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xDD2B */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D48", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D49", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            }
            /* 0xDD6F */
            if (ST(L10_GUARDIAN15_RESULT) == 2 && ST(L10_GUARDIAN15_TALK_DONE) == 0) {
                ST(L10_GUARDIAN15_TALK_DONE) = 1;
                /* 0xDDA2 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D51", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xDDCD */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D52", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D53", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                /* 0xDE11 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D54", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0xDE3C */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D55", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D56", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
            lieu_action_register("L10D57", P_KALHI33, "SP33DIA55", "SP33ARR01_10");
            CB->joindre(-P_KALHI33, 0, 0, 0x4650, -1);
            /* 0xDEBA */
            if (CB->divers(DV_DESC_POS, P_KALHI33, 0) > 4460 || CB->divers(DV_DESC_POS, P_KALHI33, 2) < 750) {
                CB->joindre(-P_KALHI33, 4450, 800, -1, -1);
            }
            CB->animation("SP33ACT20");
            CB->animation("P33_65535");
        }
        CB->divers_empile(DV_BIG_ANIM, 2, 0);
        CB->divers_empile(DV_MUSIC, 1, 4);
        ST(ST_LEVEL) = 10;
        CB->change_piece(14, -1);
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
    }
    /* 0xDF6C */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(V_KALHI_PHASE) == 8 && ST(L10_KALHI_RETURN_STEP) == 1 && CB->distance(31, 10) < 120) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L10_KALHI_RETURN_STEP) = 2;
        ST(V_KALHI_PHASE) = 7;
        ST(V_KALHI13_TALK_C) = 2;
        CB->animation("SP13LEV01");
        lieu_action_register("L10D103", P_KALHI13, "P13DIA60", "SP13ARR01_10");
        lieu_action_register("L10D104", P_KALHI13, "P13DIA60", "SP13ARR01_10");
        lieu_action_register("L10D105", P_KALHI13, "P13DIA60", "SP13ARR01_10");
        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xE082 */
    if (ST(ST_VIE(P_KALHI33)) == 1 && ST(V_KALHI_PHASE) == 8 && ST(L10_KALHI_RETURN_STEP) == 1 && CB->distance(32, 10) < 120) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L10_KALHI_RETURN_STEP) = 2;
        ST(V_KALHI_PHASE) = 7;
        ST(V_KALHI13_TALK_C) = 2;
        CB->animation("SP33LEV01");
        lieu_action_register("L10D103", P_KALHI33, "P33DIA60", "SP33ARR01_10");
        lieu_action_register("L10D104", P_KALHI33, "P33DIA60", "SP33ARR01_10");
        lieu_action_register("L10D105", P_KALHI33, "P33DIA60", "SP33ARR01_10");
        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xE198 */
    if (ST(V_DELIA_PHASE) == 6 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -750) {
        /* 0xE1F7 */
        if (CB->distance(30, 10) < 140 && ST(L10_COMPANION_CLOSE) == 0) {
            /* 0xE221 */
            if (CB->divers(DV_RUN, P_DELIA, 1) == 1) {
                CB->divers(DV_RUN, P_DELIA, 0);
            }
            ST(L10_COMPANION_CLOSE) = 1;
            CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
        }
        /* 0xE26B */
        if (ST(L10_COMPANION_MODE) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0xE29E */
            if (CB->distance(30, 10) >= 180) {
                ST(L10_COMPANION_CLOSE) = 0;
                CB->divers(DV_RUN, P_DELIA, 0);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xE2F7 */
            if (CB->distance(30, 10) >= 300) {
                ST(L10_COMPANION_CLOSE) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
    }
    /* 0xE347 */
    if (ST(L10_LEVER_ANIM_STEP) == 1 && ST(L10_SAVED_CAMERA) != ST(ST_LOCKED_CAMERA)) {
        ST(L10_LEVER_ANIM_STEP) = 2;
        ST(L10_SAVED_CAMERA) = ST(ST_LOCKED_CAMERA);
        /* 0xE39C */
        if (ST(ST_LOCKED_CAMERA) == 0) {
            CB->pause_anim(1, 0, 0);
        }
        /* 0xE3BD */
        if (ST(ST_LOCKED_CAMERA) == 1) {
            CB->pause_anim(2, 0, 0);
        }
        /* 0xE3DE */
        if (ST(ST_LOCKED_CAMERA) == 5) {
            CB->pause_anim(3, 0, 0);
        }
    }
    /* 0xE40A */
    if (ST(L10_SAVED_CAMERA) != ST(ST_LOCKED_CAMERA) && ST(L10_LEVER_ANIM_STEP) == 2) {
        ST(L10_LEVER_ANIM_STEP) = 1;
    }
    /* 0xE43D */
    if (ST(V_EVENT_11) == 2) {
        ST(V_EVENT_11) = 3;
        CB->bloque(BLOQUE_ALL);
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_EVENT_11), 4);
    }
    /* 0xE48B */
    if (ST(V_EVENT_11) == 4) {
        ST(V_EVENT_11) = 5;
        ST(V_DLG_RANDOM) = CB->distance(10, 14);
        snprintf(g_txt, sizeof(g_txt), "distance 10-14=%d", ST(V_DLG_RANDOM));
        CB->print_msg(g_txt, 0, 0xF0);
        /* 0xE501 */
        if (CB->distance(10, 14) <= 500 || CB->distance(10, 15) <= 500 || CB->distance(10, 16) <= 500 || CB->distance(10, 17) <= 500) {
            CB->divers_empile(DV_SCREEN, 1, 0);
            CB->cinematique("SC16_500");
            CB->animation("sar_50");
            /* 0xE594 */
            if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
                CB->divers(DV_HP, P_ARKHAN2, 0);
            }
            /* 0xE5C0 */
            if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
                CB->divers(DV_HP, P_ARKHAN3, 0);
            }
            ST(V_DEATH_EVENT_SENT) = 1;
            CB->divers_empile(DV_END_GAME, 0, 0);
        } else {
            /* 0xE607 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02PUI01");
                CB->animation("P02_65535");
            }
            /* 0xE62F */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03PUI01");
                CB->animation("P03_65535");
            }
            CB->animation("sar_50");
            CB->divers_empile(DV_SCREEN, 1, 0);
            CB->cinematique("SC15_500");
            ST(V_ROUTE_STAGE) = 3;
            CB->divers_empile(DV_MUSIC, 1, 4);
            CB->change_piece(51, -1);
            ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
        }
    }
    /* 0xE6C4 */
    if (ST(V_EVENT_12) == 2) {
        ST(V_EVENT_12) = 3;
        CB->bloque(BLOQUE_ALL);
        /* 0xE6F0 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->divers_empile(DV_HP, P_ARKHAN2, 0);
        }
        /* 0xE714 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->divers_empile(DV_HP, P_ARKHAN3, 0);
        }
        CB->divers_empile(DV_SCREEN, 1, 0);
        CB->cinematique("SC16_500");
        CB->animation("Sar_50");
        /* 0xE768 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0xE794 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0xE7D6 */
    if (ST(V_CRI_KEY) == 1 && ST(ST_COMBAT_STANCE) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && (ST(V_KALHI_PHASE) == 7 || ST(V_KALHI_PHASE) == 8) && (ST(ST_VIE(P_KALHI13)) == 1 || ST(ST_VIE(P_KALHI33)) == 1) && (CB->direction_perso(P_KALHI13) || CB->direction_perso(P_KALHI33)) && (CB->distance(10, 31) < 1000 || CB->distance(10, 32) < 1000)) {
        ST(V_CRI_USED) = 1;
        /* 0xE8CB */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L03D500A", P_KALHI13, "P13DIA50", "SP13ARR01_10");
        }
        /* 0xE913 */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            CB->joindre(P_KALHI33, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L03D500A", P_KALHI33, "P33DIA50", "SP33ARR01_10");
        }
        CB->modify_var(&ST(V_CRI_USED), 0);
    }
    /* 0xE96F */
    if (ST(V_CRI_KEY) == 1 && ST(ST_COMBAT_STANCE) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && (ST(V_DELIA_PHASE) == 5 || ST(V_DELIA_PHASE) == 6) && ST(ST_VIE(P_DELIA)) == 1 && CB->direction_perso(P_DELIA) && CB->distance(10, 30) < 1000) {
        ST(V_CRI_USED) = 1;
        CB->joindre(P_DELIA, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L03D09", P_DELIA, "P10DIA50", "SP10ARR01_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x100185D9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword = state+0xC8 are skipped: g_lieu_cb is provided by the bridge, the
 * trigger table is static in lieu10_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending,
 * 0x10018802); every other path returns state[0].
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
        l10_trig00();   /* 0x10018809: call sub_10001078 -> sub_10007530 */
        return 0;
    }
    /* 0x100186EE - the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1001885C): drain pending sound, run body.
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
 * Entry point - @FonctionEditor@16 (0x10018240, found through the export
 * table). Jump table @ 0x100188D9, index = op+1 (0..7); op 0 / op 2 / out
 * of range fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu10_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu10: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l10_trigger(arg);             break;  /* dword_1002BDA0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
