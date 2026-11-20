/*
 * lieu10_trig.c - Native hand-port of the LIEU10.DLL zone trigger handlers
 * (opcode 3). The entry point, INIT, tick and events live in lieu10.c; this
 * file holds the 42-slot trigger table and its handlers.
 *
 * Ported from disassembly (ImageBase 0x10000000). Each function carries its
 * original address. Handlers call the engine only through g_lieu_cb.
 */

#include <stdlib.h>

#include "libsys/common.h"
#include "lieu_cb.h"
#include "levels/lieu10_int.h"

/* -------------------------------------------------------------------------
 * Forward decls for the trigger table.
 * ---------------------------------------------------------------------- */
static void trig01(void); static void trig02(void); static void trig03(void);
static void trig04(void); static void trig05(void); static void trig06(void);
static void trig07(void); static void trig09(void); static void trig11(void);
static void trig12(void); static void trig14(void); static void trig15(void);
static void trig16(void); static void trig17(void); static void trig22(void);
static void trig29(void); static void trig30(void); static void trig31(void);
static void trig32(void); static void trig33(void); static void trig34(void);
static void trig36(void); static void trig37(void); static void trig38(void);
static void trig39(void); static void trig40(void); static void trig41(void);
static void trig_noop(void);   /* the 14 stub slots */

/* dword_1002BDA0[42] - filled at INIT with the 5-byte jmp thunks at
 * 0x10001000+5*i; static here.
 *   idx handler        idx handler        idx handler
 *    0  10007530       14  100107E4       28  10011E8E stub
 *    1  1000EA67       15  10011804       29  10011E99
 *    2  1000EDF5       16  10011B39       30  10011F2E
 *    3  1000F183       17  10011C02       31  1001252F
 *    4  1000F472       18  10011CCB stub  32  10013277
 *    5  1000F73E       19  10011CD6 stub  33  10013F92
 *    6  1000F791       20  10011CE1 stub  34  100141A2
 *    7  1000FA78       21  10011CEC stub  35  1001439D stub
 *    8  1000FE3B stub  22  10011CF7       36  100143A8
 *    9  1000FE46       23  10011E57 stub  37  1001468F
 *   10  1000FE6C stub  24  10011E62 stub  38  1001485D
 *   11  1000FE77       25  10011E6D stub  39  1001495D
 *   12  1000FF75       26  10011E78 stub  40  10014B2B
 *   13  100107D9 stub  27  10011E83 stub  41  10009356
 */
static void (*const g_trigger[42])(void) = {
    l10_trig00, trig01, trig02, trig03, trig04, trig05, trig06, trig07,
    trig_noop, trig09, trig_noop, trig11, trig12, trig_noop, trig14, trig15,
    trig16, trig17, trig_noop, trig_noop, trig_noop, trig_noop, trig22,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig29,
    trig30, trig31, trig32, trig33, trig34, trig_noop, trig36, trig37,
    trig38, trig39, trig40, trig41,
};

/* Op 3: run trigger handler idx (call [4*arg + 0x1002BDA0]). */
void l10_trigger(i32 idx) {
    if (idx >= 0 && idx < 42)
        g_trigger[idx]();
}

/* Empty slots: the stub bodies are `push ebp; mov ebp,esp; ... ret`. */
static void trig_noop(void) {
}

/* trigger 00 - sub_10007530 - level setup: start points, state reset, zones, objects, actor placement */
void l10_trig00(void) {
    i32 ra, ry;   /* place_animal: random(65535) is evaluated before random(100) */

    /* 0x7542 */
    if (ST(ST_START_POINT(10)) == 1) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 8;
        ST(L10_ROOM8_ENTRY) = 3;
        ST(V_ARKHAN1_PROGRESS) = 0;
        ST(V_ZED84_TALK_STAGE) = 0;
    }
    /* 0x759C */
    if (ST(ST_START_POINT(10)) == 2) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 8;
        ST(L10_ROOM8_ENTRY) = 3;
        ST(V_ARKHAN1_PROGRESS) = 0;
        ST(V_ZED84_TALK_STAGE) = 1;
    }
    /* 0x75F6 */
    if (ST(ST_START_POINT(10)) == 3) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 8;
        ST(L10_ROOM8_ENTRY) = 3;
        ST(V_ARKHAN1_PROGRESS) = 2;
        ST(V_ZED84_TALK_STAGE) = 0;
    }
    /* 0x7650 */
    if (ST(ST_START_POINT(10)) == 4) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 8;
        ST(L10_ROOM8_ENTRY) = 3;
        ST(V_ARKHAN1_PROGRESS) = 2;
        ST(V_ZED84_TALK_STAGE) = 1;
    }
    /* 0x76AA */
    if (ST(ST_START_POINT(10)) == 5) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 23;
        ST(V_DELIA_PHASE) = 0;
        ST(V_RETURN_PART) = 1;
    }
    /* 0x76F5 */
    if (ST(ST_START_POINT(10)) == 6) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(ST_LEVEL) = 14;
        ST(ST_VIE(P_GUARDIAN15)) = 4;
        ST(V_DELIA_PHASE) = 5;
        ST(V_ROUTE_STAGE) = 1;
    }
    /* 0x775E */
    if (ST(ST_START_POINT(10)) == 7) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 8;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(ST_VIE(P_KALHI33)) = 0;
        ST(V_KALHI_PHASE) = 7;
        ST(L10_ROOM8_ENTRY) = 3;
        ST(V_DELIA_PHASE) = 0;
    }
    ST(L10_ZED_FORCE_MEET) = 0;
    ST(L10_UNUSED_68) = 0;
    ST(L10_COMPANION_MODE) = ST(L10_COMPANION_MODE_SAVED);
    ST(L10_LORY_FOLLOW_REQ) = 0;
    ST(L10_COMPANION_CHATTER_ARMED) = 0;
    ST(L10_COMPANION_CLOSE) = 0;
    ST(L10_ZED_APPROACH_STEP) = 0;
    ST(L10_COMPANION_CATCHUP_STEP) = 0;
    ST(L10_KALHI_RETURN_STEP) = 0;
    ST(L10_UNUSED_88) = 0;
    ST(V_EVENT15_STEP) = 0;
    ST(L10_DIALOG_TIMER_ARMED) = 0;
    ST(L10_GUARDIAN15_AMBUSH_STEP) = 0;
    ST(L10_KALHI_TALK_DONE) = 0;
    ST(L10_UNUSED_B8) = 0;
    ST(L10_UNUSED_BC) = 0;
    ST(L10_SUNSEER_STEP) = 0;
    ST(L10_KALHI_DEAD_NOTED) = 0;
    ST(L10_LEVER_ANIM_STEP) = 0;
    ST(L10_GUARDIAN15_CHASE_STEP) = 0;
    ST(L10_ESCORT_INTRO_DONE) = 0;
    ST(L10_KALHI_SWAP_REQ) = 0;
    ST(L10_RYLSADHAR_PLACED) = 0;
    ST(L10_UNUSED_FC) = 0;
    ST(L10_GUARDIAN15_TALK_DONE) = 0;
    ST(L10_UNUSED_114) = 0;
    ST(V_WEAPON_WARNED) = 0;
    ST(L10_GUARDIAN15_ALERT) = 0;
    ST(L10_GUARDIAN15_FIGHT) = 0;
    ST(L10_LORY_SCENE_DONE) = 0;
    ST(L10_AMBIENT_ZONE_NEAR) = 0;
    ST(V_EVENT_0) = 0;
    ST(V_EVENT_1) = 0;
    ST(V_EVENT_2) = 0;
    ST(V_EVENT_33) = 0;
    ST(V_EVENT_34) = 0;
    ST(V_EVENT_35) = 0;
    CB->evenement(0, -1);
    CB->evenement(1, -1);
    CB->evenement(2, -1);
    CB->evenement(11, -1);
    CB->evenement(12, -1);
    CB->evenement(33, -1);
    CB->evenement(34, -1);
    CB->evenement(35, -1);
    CB->evenement(38, -1);
    ST(ST_SCRIPT_BUSY) = 0;
    ST(V_ENGAGED_PERSONA) = 0;
    ST(V_LAST_WEAPON) = 0;
    /* 0x7A99 */
    if (ST(V_GUARDIAN8_STAGE) == 3) {
        ST(V_GUARDIAN8_STAGE) = 10;
    }
    /* 0x7ABA */
    if (ST(V_KALHI_RECRUIT) == 2 && ST(V_KALHI13_TALK_C) == 3) {
        ST(ST_VIE(P_KALHI13)) = 4;
        ST(ST_VIE(P_KALHI33)) = 4;
    }
    /* 0x7AFC */
    if (ST(V_KALHI_RECRUIT) == 4 && ST(L10_PLOT_6DC) == 4) {
        ST(ST_VIE(P_GUARDIAN25)) = 4;
        ST(L10_PLOT_6DC) = 5;
    }
    /* 0x7B3E */
    if (ST(L10_PLOT_BA8) == 4) {
        ST(L10_PLOT_BA8) = 5;
    }
    /* 0x7B5F */
    if (ST(ST_VIE(P_GUARDIAN15)) == VIE_DEAD && ST(V_GUARD_FIGHT_DONE) >= 2) {
        ST(ST_VIE(P_GUARDIAN15)) = 4;
    }
    /* 0x7B92 */
    if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
        CB->change_piece(10, 1);
    }
    /* 0x7BC6 */
    if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
        CB->change_piece(10, 2);
    }
    /* 0x7BFA */
    if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
        CB->change_piece(10, 3);
    }
    CB->ajoute_zone(1, 2550, -1100, 60, 450, 1);
    CB->ajoute_zone(2, 2550, -1622, 60, 450, 1);
    CB->ajoute_zone(3, 5170, -2620, 70, 180, 1);
    CB->ajoute_zone(4, 5654, -1350, 70, 500, 1);
    CB->ajoute_zone(6, 4475, -3420, 800, 70, 1);
    CB->ajoute_zone(14, 4380, -1243, 140, 140, 0);
    CB->ajoute_zone(15, 4400, -1422, 140, 140, 0);
    CB->ajoute_zone(16, 4595, -1416, 140, 140, 0);
    CB->ajoute_zone(17, 4590, -1250, 140, 140, 0);
    CB->ajoute_zone(18, 4462, -1322, 60, 60, 0);
    CB->ajoute_zone(11, 3406, -950, 150, 150, 0);
    /* 0x7D84 */
    if (ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->bruitage("L10AMB1", 4380, -1243);
    }
    /* 0x7DAB */
    if (ST(V_GUARD_FIGHT_DONE) == 2) {
        CB->bruitage("L10AMB2", 4380, -1243);
    }
    /* 0x7DCF */
    if (ST(ST_LEVEL) == 8) {
        /* 0x7DE1 */
        if (ST(L10_ROOM8_ENTRY) == 3) {
            CB->pos_perso(ST(ST_HERO), 2712, -1622, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 0xA, 0);
        }
        /* 0x7E32 */
        if (ST(L10_ROOM8_ENTRY) == 4) {
            CB->pos_perso(ST(ST_HERO), 2712, -1100, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 0xA, 0);
        }
    }
    /* 0x7E80 */
    if (ST(ST_LEVEL) == 11) {
        CB->pos_perso(ST(ST_HERO), 5495, -1300, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 0xA, 0);
    }
    /* 0x7ECE */
    if (ST(ST_LEVEL) == 14) {
        CB->pos_perso(ST(ST_HERO), 4450, 800, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 0xA, 0);
    }
    /* 0x7F1C */
    if (ST(ST_LEVEL) == 20) {
        CB->pos_perso(ST(ST_HERO), 4475, -3300, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 0xA, 0);
    }
    /* 0x7F6A */
    if (ST(ST_LEVEL) == 23) {
        CB->pos_perso(ST(ST_HERO), 5040, -2655, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 0xA, 0);
        /* 0x7FBB */
        if (ST(V_RETURN_PART) != 0 && ST(L10_GUARDIAN_CUTSCENE_STAGE) == 0 && ST(V_KALHI_KILLED) == 0) {
            CB->change_camera(4, CAM_LOCKED);
            CB->bloque(BLOQUE_ALL);
            ST(L10_GUARDIAN_CUTSCENE_STAGE) = 1;
            CB->free_anim(-10);
            CB->load_anim("GAR20", 0);
            CB->load_anim("GAR20par", 1);
            CB->load_anim("GAR45", 2);
            CB->load_anim("GAR45par", 3);
            CB->load_anim("GAR46", 4);
            CB->load_anim("GAR46par", 5);
            CB->load_anim("GARban", 6);
            CB->load_anim("GARbanpa", 7);
            CB->load_anim("Archi", 8);
            CB->load_anim("Archipar", 9);
            CB->pos_perso(P_SUNSEER1, 4490, -2860, 0x640, 1, "P18ARR01", 0x22, 0);
            CB->pos_perso(P_GUARDIAN15, 4500, -1980, 0x79B8, 1, "P55ARR04", 0x21, 0);
            ST(V_KALHI_KILLED) = 10;
        }
    }
    /* 0x811C */
    if (ST(ST_HERO) == P_ARKHAN1) {
        CB->ajoute_zone(29, 4476, 422, 325, 80, 1);
        /* 0x814C */
        if (ST(V_ARKHAN1_PROGRESS) == 0 && ST(V_ZED84_TALK_STAGE) == 0 && ST(L10_LORY_FOLLOW_STATE) == 0) {
            CB->pos_perso(P_LORY, 4300, -2300, 0xB798, 1, "P15ARR01", 0x1A, 0);
            CB->pos_perso(P_RYLSADHAR20, 4720, -3270, 0xFA0, 1, "P20ARR01", 0x1B, 0);
            CB->link_objet(P_RYLSADHAR20, "OB02", 0x1B, 1);
            CB->ajoute_zone(19, 4420, -2720, 100, 100, 1);
            CB->ajoute_zone(20, 4647, -3520, 100, 100, 1);
            CB->ajoute_zone(21, 5320, -2640, 70, 70, 1);
        }
        /* 0x8231 */
        if ((ST(V_ARKHAN1_PROGRESS) == 1 || (ST(V_ARKHAN1_PROGRESS) == 0 && ST(L10_LORY_FOLLOW_STATE) == 1)) && ST(V_ZED84_TALK_STAGE) == 0) {
            CB->pos_perso(P_LORY, 4380, -2760, 0xD228, 1, "P15ARR01", 0x1A, 0);
            CB->pos_perso(P_RYLSADHAR20, 4500, -2720, 0x2EE0, 1, "P20ARR01", 0x1B, 0);
            CB->link_objet(P_RYLSADHAR20, "OB02", 0x1B, 1);
            CB->ajoute_zone(19, 4420, -2720, 100, 100, 1);
            CB->ajoute_zone(20, 4647, -3520, 100, 100, 1);
            CB->ajoute_zone(21, 5320, -2640, 70, 70, 1);
        }
        /* 0x8328 */
        if (ST(V_ARKHAN1_PROGRESS) >= 2 && ST(V_ZED84_TALK_STAGE) == 0) {
            ST(L10_ZED_APPROACH_STEP) = 1;
            CB->pos_perso(P_ZED84, 4540, 304, 0, 1, "P84ARR01", 0xC, 0);
            CB->link_objet(P_ZED84, "OB68", 0x1D, 1);
        }
        /* 0x838E */
        if (ST(V_ZED84_TALK_STAGE) == 1) {
            CB->ajoute_zone(24, 2900, -1130, 10, 10, 0);
            CB->ajoute_zone(25, 2888, -1493, 10, 10, 0);
            /* 0x83D6 */
            if (ST(V_ARKHAN1_PROGRESS) >= 3) {
                CB->pos_perso(P_ZED84, 3333, -1300, 0x3C8C, 1, "P84ARR01", 0xC, 0);
            }
            /* 0x840D */
            if (ST(V_ARKHAN1_PROGRESS) < 3) {
                CB->pos_perso(P_ZED84, 1979, -1148, 0xBD5F, 1, "P84ARR01", 0xC, 0);
            }
            CB->link_objet(P_ZED84, "OB68", 0x1D, 1);
        }
    }
    /* 0x8458 */
    if (ST(ST_HERO) > 1) {
        CB->ajoute_zone(7, 4312, 754, 120, 100, 0);
    }
    /* 0x8485 */
    if (ST(ST_HERO) >= 2) {
        /* 0x8497 */
        if (ST(V_KALHI_PHASE) == 7 && ST(L10_KALHI_ESCORT_STARTED) == 0) {
            ST(L10_GUARDIAN15_RESULT) = 0;
            ST(L10_COMPANION_CHATTER_LINE) = 0;
            /* 0x84D9 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                ST(L10_COMPANION_MODE) = 1;
                ST(L10_COMPANION_CHATTER_ARMED) = 1;
                /* 0x8503 */
                if (ST(L10_ROOM8_ENTRY) == 3) {
                    CB->pos_perso(P_KALHI13, 2500, -1490, 0xBB80, 1, "P13ARR01", 0x1F, 0);
                }
                /* 0x853A */
                if (ST(L10_ROOM8_ENTRY) == 4) {
                    CB->pos_perso(P_KALHI13, 2500, -1150, 0xBB80, 1, "P13ARR01", 0x1F, 0);
                }
            } else if (ST(L10_ROOM8_ENTRY) != 3 && ST(L10_ROOM8_ENTRY) != 4) {
                CB->pos_perso(P_KALHI13, CB->divers(DV_DESC_POS, P_KALHI13, 0), CB->divers(DV_DESC_POS, P_KALHI13, 1), CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P13ARR03", 0x1F, 0);
            }
            /* 0x85EA */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->ajoute_zone(35, 4240, -780, 100, 100, 1);
                /* 0x8617 */
                if (ST(L10_ROOM8_ENTRY) == 3) {
                    CB->pos_perso(P_KALHI33, 2912, -1622, 0x7530, 1, "P33ARR01", 0x20, 0);
                }
                /* 0x864E */
                if (ST(L10_ROOM8_ENTRY) == 4) {
                    CB->pos_perso(P_KALHI33, 2912, -1100, 0x7530, 1, "P33ARR01", 0x20, 0);
                }
            }
        }
        /* 0x8685 */
        if (ST(V_KALHI_PHASE) == 7 && ST(L10_KALHI_ESCORT_STARTED) == 1) {
            /* 0x86A9 */
            if (ST(L10_GUARDIAN15_RESULT) == 0) {
                ST(L10_COMPANION_CATCHUP_STEP) = 1;
                /* 0x86CA */
                if (ST(ST_VIE(P_KALHI13)) == 1) {
                    CB->pos_perso(P_KALHI13, 4240, -800, CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P13ARR01", 0x1F, 0);
                }
                /* 0x870F */
                if (ST(ST_VIE(P_KALHI33)) == 1) {
                    CB->pos_perso(P_KALHI33, 4240, -800, CB->divers(DV_DESC_POS, P_KALHI33, 2), 1, "P33ARR01", 0x20, 0);
                }
            }
            /* 0x8754 */
            if (ST(L10_GUARDIAN15_RESULT) == 1) {
                ST(V_KALHI_KILLED) = 1;
                /* 0x8775 */
                if (ST(ST_VIE(P_KALHI13)) == 1) {
                    ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
                    CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13cad", 0x1F, 0);
                }
                /* 0x87B2 */
                if (ST(ST_VIE(P_KALHI33)) == 1) {
                    ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
                    CB->pos_perso(P_KALHI33, 0, 0, 0, 2, "P33cad", 0x20, 0);
                }
            }
            /* 0x87EF */
            if (ST(L10_GUARDIAN15_RESULT) == 2 || ST(L10_GUARDIAN15_RESULT) == 3) {
                ST(L10_GUARDIAN15_CHASE_STEP) = 4;
                /* 0x8822 */
                if (ST(ST_VIE(P_KALHI13)) == 1) {
                    ST(L10_GUARDIAN15_AMBUSH_STEP) = 9;
                    CB->pos_perso(P_KALHI13, 4312, 807, 0x4650, 1, "P13ARR01", 0x1F, 0);
                }
                /* 0x8868 */
                if (ST(ST_VIE(P_KALHI33)) == 1) {
                    ST(L10_GUARDIAN15_AMBUSH_STEP) = 9;
                    CB->pos_perso(P_KALHI33, 4312, 807, 0x4650, 1, "P33ARR01", 0x20, 0);
                }
            }
        }
        /* 0x88AE */
        if (ST(V_KALHI_PHASE) == 8) {
            ST(L10_KALHI_RETURN_STEP) = 1;
            /* 0x88CF */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->pos_perso(P_KALHI13, 2920, -1200, 0xC544, 1, "P13ARR03", 0x1F, 0);
            }
            /* 0x8906 */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->pos_perso(P_KALHI33, 2920, -1200, 0xC544, 1, "P33ARR03", 0x20, 0);
            }
            /* 0x893D */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD) {
                CB->pos_perso(P_KALHI13, CB->divers(DV_DESC_POS, P_KALHI13, 0), CB->divers(DV_DESC_POS, P_KALHI13, 1), CB->divers(DV_DESC_POS, P_KALHI13, 2), 2, "P13cad", 0x1F, 0);
            }
            /* 0x899F */
            if (ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
                CB->pos_perso(P_KALHI33, CB->divers(DV_DESC_POS, P_KALHI33, 0), CB->divers(DV_DESC_POS, P_KALHI33, 1), CB->divers(DV_DESC_POS, P_KALHI33, 2), 2, "P33cad", 0x20, 0);
            }
        }
        /* 0x8A01 */
        if (ST(ST_VIE(P_GUARDIAN15)) <= 1) {
            /* 0x8A13 */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD || ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
                ST(L10_KALHI_DEAD_NOTED) = 1;
                ST(V_KALHI_KILLED) = 1;
                /* 0x8A52 */
                if (ST(ST_LEVEL) == 8) {
                    CB->pos_perso(P_GUARDIAN15, 3333, -1300, 0x3C8C, 1, "P55GAR05", 0x21, 0);
                }
                /* 0x8A86 */
                if (ST(ST_LEVEL) == 20) {
                    CB->pos_perso(P_GUARDIAN15, 4494, -2634, 0x7148, 1, "P55GAR05", 0x21, 0);
                }
                /* 0x8ABA */
                if (ST(ST_LEVEL) == 23) {
                    CB->pos_perso(P_GUARDIAN15, 4494, -2634, 0xBB80, 1, "P55GAR05", 0x21, 0);
                }
                /* 0x8AEE */
                if (ST(ST_LEVEL) == 11) {
                    CB->pos_perso(P_GUARDIAN15, 5036, -1254, 0xAFC8, 1, "P55GAR05", 0x21, 0);
                }
            } else if (ST(L10_GUARDIAN15_RESULT) == 2 || ST(L10_GUARDIAN15_RESULT) == 3) {
                CB->pos_perso(P_GUARDIAN15, 4420, -140, 0x7530, 1, "P55ARR04", 0x21, 0);
            } else {
                CB->pos_perso(P_GUARDIAN15, 4480, 275, 0x88B8, 1, "P55ARR04", 0x21, 0);
            }
        }
        /* 0x8B9D */
        if (ST(ST_VIE(P_GUARDIAN15)) == VIE_DEAD) {
            CB->pos_perso(P_GUARDIAN15, CB->divers(DV_DESC_POS, P_GUARDIAN15, 0), CB->divers(DV_DESC_POS, P_GUARDIAN15, 1), CB->divers(DV_DESC_POS, P_GUARDIAN15, 2), 2, "P55cad", 0x21, 0);
        }
        /* 0x8BFF */
        if (ST(V_DELIA_PHASE) == 5) {
            CB->ajoute_zone(5, 4472, 400, 335, 90, 1);
            CB->ajoute_zone(9, 4472, 150, 335, 90, 1);
            CB->pos_perso(P_DELIA, 4600, 600, 0, 1, "P10ARR01", 0x1E, 0);
            /* 0x8C6F */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
                ST(ST_VIE(P_KALHI13)) = 4;
                ST(ST_VIE(P_KALHI33)) = 4;
            }
        }
        /* 0x8CB1 */
        if (ST(V_DELIA_PHASE) == 6) {
            CB->ajoute_zone(5, 4472, 400, 335, 90, 1);
            /* 0x8CDE */
            if (ST(ST_LEVEL) != 14) {
                ST(L10_COMPANION_MODE) = 1;
            } else {
                ST(L10_COMPANION_MODE) = 10;
                ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
            }
            CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 0x1E, 0);
            /* 0x8D6E */
            if (ST(ST_LEVEL) == 23) {
                CB->pos_perso(P_DELIA, 4590, -2637, 0xD1C1, 1, "P10ARR01", 0x1E, 0);
            }
        }
        /* 0x8DA5 */
        if (ST(V_ROUTE_STAGE) > 0) {
            /* 0x8DB7 */
            if (ST(V_ROUTE_STAGE) == 1) {
                CB->load_anim("ouvgril", 0);
                CB->load_anim("grouvc00", 1);
                CB->load_anim("grouvc01", 2);
                CB->load_anim("grouvc05", 3);
            }
            /* 0x8E06 */
            if (ST(L10_GUARDIAN22_TAKEN) == 0 && ST(ST_VIE(85)) != VIE_DEAD) {
                CB->print_msg("gardien22 pas mort", 0, 0xF0);
                ST(ST_VIE(85)) = 1;
            }
            /* 0x8E4B */
            if (ST(ST_VIE(85)) == 1) {
                CB->pos_perso(85, 4405, -646, 0x9470, 1, "P85cad39", 0x22, 0);
            }
            /* 0x8E82 */
            if (ST(ST_VIE(85)) == VIE_DEAD) {
                CB->pos_perso(85, 4405, -646, 0x9470, 2, "P85cad38", 0x22, 0);
            }
            CB->ajoute_zone(36, 4727, -500, 310, 310, 0);
            CB->ajoute_zone(37, 4560, -726, 200, 200, 0);
            CB->ajoute_zone(38, 4750, -755, 200, 200, 0);
            CB->ajoute_zone(39, 5053, -600, 200, 200, 0);
            CB->ajoute_zone(40, 4500, -200, 200, 200, 0);
        }
    }
    /* 0x8F5E */
    if (ST(V_ROUTE_STAGE) == 0 && ST(V_GUARD_FIGHT_DONE) == 2) {
        CB->print_msg("je tente d'enlever les sols", 0, 0x104);
        CB->vire_col(-29, 0);
        CB->vire_col(-30, 0);
        CB->vire_col(-31, 0);
        CB->vire_col(-32, 0);
        CB->vire_col(-33, 0);
    }
    /* 0x8FD5 */
    if (ST(V_DELIA_KILLED) == 10) {
        CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 2, "P10cad", 0x1E, 0);
    }
    /* 0x9037 */
    if (ST(ST_HERO) == P_ARKHAN1) {
        CB->ajoute_zone(22, 4470, 340, 340, 110, 0);
        CB->vire_col(10, 0);
    }
    /* 0x9074 */
    if (ST(ST_HERO) > 1) {
        CB->vire_col(9, 0);
    }
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 3000, ry, -1200, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 3550, ry, -1300, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 4000, ry, -1400, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 4500, ry, -1500, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 5000, ry, -1600, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 5500, ry, -1700, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 2500, ry, -1800, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 3000, ry, -2000, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 3500, ry, -2200, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 4000, ry, -2400, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 4500, ry, -2600, ra, 1000);
    ra = CB->random(65535);
    ry = CB->random(100) - 100;
    CB->place_animal(4, 5000, ry, -2800, ra, 1000);
    ST(ST_START_POINT(10)) = 0;
    CB->divers(DV_EVENT_SOUND, 0x10001, 0x44380000);
    CB->divers(DV_EVENT_SOUND, 0x20002, 0x1F400000);
    CB->divers(DV_EVENT_SOUND, 0x30003, 0x46500000);
}

/* trigger 01 - sub_1000EA67 - zone 1 (west door of the Guardian house, east side) */
static void trig01(void) {
    CB->print_msg("zone1 declenchee", 0, 0xF0);
    /* 0xEA8B */
    if (ST(V_ZED84_TALK_STAGE) == 1 || ST(V_ZED84_TALK_STAGE) == 2) {
        return;
    }
    /* 0xEAB4 */
    if (ST(V_EVENT_11) > 0) {
        CB->bloque(BLOQUE_ALL);
        return;
    }
    CB->select_pile(0, 2);
    CB->divers_empile(DV_MUSIC, 1, 4);
    CB->retire_zone(1);
    /* 0xEB00 */
    if (ST(L10_DELIA_RETURN_DONE) == 0 && ST(V_ROUTE_STAGE) == 1 && ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_DELIA_PHASE) == 20 && (ST(V_DELIA_KILLED) == 16 || ST(V_DELIA_KILLED) == 19 || ST(V_DELIA_KILLED) == 14 || ST(V_DELIA_KILLED) == 10)) {
        ST(L10_DELIA_RETURN_DONE) = 1;
    }
    /* 0xEB9F */
    if (ST(V_ZED84_TALK_STAGE) != 0 && ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0) {
        ST(L10_ZED_FORCE_MEET) = 1;
    } else {
        /* 0xEBE6 */
        if (ST(V_DELIA_PHASE) == 5) {
            ST(V_DELIA_PHASE) = 6;
        }
        /* 0xEC07 */
        if (ST(V_KALHI_PHASE) == 7 && ST(L10_ESCORT_INTRO_DONE) == 0 && ST(L10_GUARDIAN15_RESULT) == 0) {
            CB->bloque(BLOQUE_ALL);
            ST(V_SAVED_HERO) = ST(ST_HERO);
            ST(L10_ESCORT_INTRO_DONE) = 1;
            CB->print_msg("test3 passe \340 100 ln2167", 0, 0xF0);
            ST(L10_COMPANION_MODE) = 100;
            /* 0xEC8C */
            if (ST(ST_VIE(P_KALHI13)) == 1 && CB->distance(10, 31) < 1000) {
                CB->stop_joindre(-P_KALHI13);
                lieu_action_register("L10D109A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                lieu_action_register("L10D109B", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            }
            /* 0xECF6 */
            if (ST(ST_VIE(P_KALHI33)) == 1 && CB->distance(10, 32) < 1000) {
                CB->stop_joindre(-P_KALHI33);
                lieu_action_register("L10D109A", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                lieu_action_register("L10D109B", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            }
            CB->bloque(BLOQUE_NONE);
        }
        ST(L10_ROOM8_ENTRY) = 2;
        ST(ST_LEVEL) = 10;
        /* 0xED83 */
        if (ST(V_DELIA_PHASE) == 6 || ST(L10_DELIA_PRESENT_F20) == 1 || ST(L10_DELIA_RETURN_DONE) != 0) {
            CB->change_piece(8, 1);
        } else {
            CB->change_piece(8, 0);
        }
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
    }
    CB->select_pile(0, 1);
}

/* trigger 02 - sub_1000EDF5 - zone 2 */
static void trig02(void) {
    CB->print_msg("zone2 declenchee", 0, 0xF0);
    /* 0xEE19 */
    if (ST(V_ZED84_TALK_STAGE) == 1 || ST(V_ZED84_TALK_STAGE) == 2) {
        return;
    }
    /* 0xEE42 */
    if (ST(V_EVENT_11) > 0) {
        CB->bloque(BLOQUE_ALL);
        return;
    }
    CB->select_pile(0, 2);
    CB->divers_empile(DV_MUSIC, 1, 4);
    CB->retire_zone(2);
    /* 0xEE8E */
    if (ST(L10_DELIA_RETURN_DONE) == 0 && ST(V_ROUTE_STAGE) == 1 && ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_DELIA_PHASE) == 20 && (ST(V_DELIA_KILLED) == 16 || ST(V_DELIA_KILLED) == 19 || ST(V_DELIA_KILLED) == 14 || ST(V_DELIA_KILLED) == 10)) {
        ST(L10_DELIA_RETURN_DONE) = 1;
    }
    /* 0xEF2D */
    if (ST(V_ZED84_TALK_STAGE) != 0 && ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0) {
        ST(L10_ZED_FORCE_MEET) = 1;
    } else {
        /* 0xEF74 */
        if (ST(V_DELIA_PHASE) == 5) {
            ST(V_DELIA_PHASE) = 6;
        }
        /* 0xEF95 */
        if (ST(V_KALHI_PHASE) == 7 && ST(L10_ESCORT_INTRO_DONE) == 0 && ST(L10_GUARDIAN15_RESULT) == 0) {
            CB->bloque(BLOQUE_ALL);
            ST(V_SAVED_HERO) = ST(ST_HERO);
            ST(L10_ESCORT_INTRO_DONE) = 1;
            CB->print_msg("test3 passe \340 100 ln 2232", 0, 0xF0);
            ST(L10_COMPANION_MODE) = 100;
            /* 0xF01A */
            if (ST(ST_VIE(P_KALHI13)) == 1 && CB->distance(10, 31) < 1000) {
                CB->stop_joindre(-P_KALHI13);
                lieu_action_register("L10D109A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                lieu_action_register("L10D109B", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            }
            /* 0xF084 */
            if (ST(ST_VIE(P_KALHI33)) == 1 && CB->distance(10, 32) < 1000) {
                CB->stop_joindre(-P_KALHI33);
                lieu_action_register("L10D109A", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                lieu_action_register("L10D109B", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            }
            CB->bloque(BLOQUE_NONE);
        }
        ST(L10_ROOM8_ENTRY) = 1;
        ST(ST_LEVEL) = 10;
        /* 0xF111 */
        if (ST(V_DELIA_PHASE) == 6 || ST(L10_DELIA_PRESENT_F20) == 1 || ST(L10_DELIA_RETURN_DONE) != 0) {
            CB->change_piece(8, 1);
        } else {
            CB->change_piece(8, 0);
        }
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
    }
    CB->select_pile(0, 1);
}

/* trigger 03 - sub_1000F183 - zone 3 */
static void trig03(void) {
    CB->select_pile(0, 2);
    CB->divers(DV_MUSIC, 1, 4);
    CB->retire_zone(3);
    /* 0xF1BF */
    if (ST(V_LEFT_HOME) == 0) {
        ST(V_LEFT_HOME) = 1;
    }
    /* 0xF1E0 */
    if (ST(V_DELIA_PHASE) == 6) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        ST(L10_COMPANION_MODE) = 10;
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10ARR01_10");
        /* 0xF236 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L10D02A", P_ARKHAN2);
        }
        /* 0xF258 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L10D02B", P_ARKHAN3);
        }
        CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D03A", P_DELIA, "SP10DIA53", "SP10ARR01_10");
        lieu_action_register("L10D03B", P_DELIA, "SP10DIA54", "SP10ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xF2D1 */
    if (ST(V_KALHI_PHASE) == 7 && ST(L10_ESCORT_INTRO_DONE) == 0 && ST(L10_GUARDIAN15_RESULT) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        ST(L10_ESCORT_INTRO_DONE) = 1;
        ST(L10_COMPANION_MODE) = 10;
        /* 0xF344 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && CB->distance(10, 31) < 1000) {
            CB->stop_joindre(-P_KALHI13);
            lieu_action_register("L10D109A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            lieu_action_register("L10D109B", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
        }
        /* 0xF3AE */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->distance(10, 32) < 1000) {
            CB->stop_joindre(-P_KALHI33);
            lieu_action_register("L10D109A", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            lieu_action_register("L10D109B", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
        }
        CB->bloque(BLOQUE_NONE);
    }
    ST(ST_LEVEL) = 10;
    /* 0xF42C */
    if (ST(V_ROUTE_STAGE) < 3) {
        CB->change_piece(23, 0);
    } else {
        CB->change_piece(23, 1);
    }
    ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
}

/* trigger 04 - sub_1000F472 - zone 4 */
static void trig04(void) {
    /* 0xF484 */
    if (ST(V_EVENT_11) > 0) {
        CB->bloque(BLOQUE_ALL);
        return;
    }
    CB->select_pile(0, 2);
    CB->divers(DV_MUSIC, 1, 4);
    CB->retire_zone(4);
    /* 0xF4D5 */
    if (ST(V_DELIA_PHASE) == 6) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        CB->stop_joindre(-P_DELIA);
        ST(L10_COMPANION_MODE) = 10;
        CB->animation("SP10ARR01_10");
        /* 0xF52B */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L10D02A", P_ARKHAN2);
        }
        /* 0xF54D */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L10D02B", P_ARKHAN3);
        }
        CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D03A", P_DELIA, "SP10DIA53", "SP10ARR01_10");
        lieu_action_register("L10D03B", P_DELIA, "SP10DIA54", "SP10ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xF5C6 */
    if (ST(V_KALHI_PHASE) == 7 && ST(L10_ESCORT_INTRO_DONE) == 0 && ST(L10_GUARDIAN15_RESULT) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        ST(L10_ESCORT_INTRO_DONE) = 1;
        ST(L10_COMPANION_MODE) = 10;
        /* 0xF639 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && CB->distance(10, 31) < 1000) {
            CB->stop_joindre(-P_KALHI13);
            lieu_action_register("L10D109A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            lieu_action_register("L10D109B", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
        }
        /* 0xF6A3 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->distance(10, 32) < 1000) {
            CB->stop_joindre(-P_KALHI33);
            lieu_action_register("L10D109A", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            lieu_action_register("L10D109B", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
        }
        CB->bloque(BLOQUE_NONE);
    }
    ST(ST_LEVEL) = 10;
    CB->change_piece(11, -1);
    ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
}

/* trigger 05 - sub_1000F73E - zone 5 */
static void trig05(void) {
    /* 0xF750 */
    if (ST(V_DELIA_PHASE) == 6 && ST(L10_COMPANION_MODE) != 0) {
        ST(L10_COMPANION_MODE) = 10;
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10ARR01_10");
        return;
    }
}

/* trigger 06 - sub_1000F791 - zone 6 */
static void trig06(void) {
    CB->select_pile(0, 2);
    CB->divers(DV_MUSIC, 1, 4);
    CB->retire_zone(6);
    /* 0xF7CD */
    if (ST(L10_PLOT_BD0) == 1) {
        ST(L10_PLOT_BD0) = 2;
    }
    /* 0xF7EE */
    if (ST(V_LEFT_HOME) == 0) {
        ST(V_LEFT_HOME) = 1;
    }
    /* 0xF80F */
    if (ST(V_DELIA_PHASE) == 6) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        CB->stop_joindre(-P_DELIA);
        ST(L10_COMPANION_MODE) = 10;
        CB->animation("SP10ARR01_10");
        /* 0xF865 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L10D02A", P_ARKHAN2);
        }
        /* 0xF887 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L10D02B", P_ARKHAN3);
        }
        CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D03A", P_DELIA, "SP10DIA53", "SP10ARR01_10");
        lieu_action_register("L10D03B", P_DELIA, "SP10DIA54", "SP10ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xF900 */
    if (ST(V_KALHI_PHASE) == 7 && ST(L10_ESCORT_INTRO_DONE) == 0 && ST(L10_GUARDIAN15_RESULT) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        ST(L10_ESCORT_INTRO_DONE) = 1;
        ST(L10_COMPANION_MODE) = 10;
        /* 0xF973 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && CB->distance(10, 31) < 1000) {
            CB->stop_joindre(-P_KALHI13);
            lieu_action_register("L10D109A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            lieu_action_register("L10D109B", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
        }
        /* 0xF9DD */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->distance(10, 32) < 1000) {
            CB->stop_joindre(-P_KALHI33);
            lieu_action_register("L10D109A", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            lieu_action_register("L10D109B", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
        }
        CB->bloque(BLOQUE_NONE);
    }
    ST(ST_LEVEL) = 10;
    ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
    CB->change_piece(20, -1);
}

/* trigger 07 - sub_1000FA78 - zone 7 */
static void trig07(void) {
    /* 0xFA87 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(L10_GUARDIAN15_FIGHT) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x4650)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xFAF0 */
        if (ST(V_ROUTE_STAGE) > 0) {
            CB->pos_perso(85, -1, -1, -1, 4, "", 0x22, 0);
        }
        /* 0xFB29 */
        if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
            /* 0xFB3B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02lev07");
            }
            /* 0xFB58 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03lev07");
            }
        }
        /* 0xFB7D */
        if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
            /* 0xFB8F */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02lev06");
            }
            /* 0xFBAC */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03lev06");
            }
        }
        /* 0xFBC9 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("P02_65535");
        }
        /* 0xFBE6 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("P03_65535");
        }
        /* 0xFC03 */
        if (ST(V_DELIA_PHASE) == 6 && ST(L10_COMPANION_MODE) != 0) {
            ST(V_SAVED_HERO) = ST(ST_HERO);
            /* 0xFC3B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L10D02A", P_ARKHAN2);
            }
            /* 0xFC5D */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L10D02B", P_ARKHAN3);
            }
            CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L10D03A", P_DELIA, "SP10DIA53", "SP10ARR01_10");
            lieu_action_register("L10D03B", P_DELIA, "SP10DIA54", "SP10ARR01_10");
        }
        CB->animation("Sar_10");
        /* 0xFCD9 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->divers_empile(DV_BIG_ANIM, 2, 0);
        }
        /* 0xFCFD */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->divers_empile(DV_BIG_ANIM, 0, 0);
        }
        CB->divers_empile(DV_MUSIC, 1, 4);
        ST(ST_LEVEL) = 10;
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
        CB->change_piece(14, -1);
    }
    /* 0xFD5D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_GUARDIAN15_FIGHT) == 1 && CB->direction_spec(0x4650) && ST(ST_VIE(P_GUARDIAN15)) == 1) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xFDC9 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L11D06", P_ARKHAN2, "SG02ATT01", NULL);
        }
        /* 0xFDF1 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L11D07", P_ARKHAN3, "SG03ATT01", NULL);
        }
        CB->animation("Sar_20");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 09 - sub_1000FE46 - zone 9 */
static void trig09(void) {
    /* 0xFE55 */
    if (ST(L10_COMPANION_MODE) != 1) {
        ST(L10_COMPANION_MODE) = 1;
    }
}

/* trigger 11 - sub_1000FE77 - zone 11 (save point) */
static void trig11(void) {
    /* 0xFE86 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x5208)) {
        CB->bloque(BLOQUE_ALL);
        /* 0xFED4 */
        if (ST(ST_HERO) <= 1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", "Sar_10");
        }
        /* 0xFEFF */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0xFF2A */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 12 - sub_1000FF75 - zone 12 (Zed84, save point) */
static void trig12(void) {
    /* 0xFF84 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x5208) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 11) && !CB->direction_perso(P_ZED84)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x10005 */
        if (ST(ST_HERO) <= 1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", "Sar_10");
        }
        /* 0x10030 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0x1005B */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x100AE */
    if (CB->direction_perso(P_ZED84)) {
        /* 0x100BD */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->stop_joindre(-P_ZED84);
            CB->animation("SP84ARR01_10");
            CB->joindre(-P_ZED84, 0, 0, 0, ST(ST_HERO));
            /* 0x10128 */
            if (ST(L10_ZED_TALK_STAGE_7D4) == 0) {
                ST(L10_ZED_TALK_STAGE_7D4) = 1;
                /* 0x10149 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    lieu_action_register("L10D16", P_ARKHAN1, "SG00DIA04", NULL);
                }
                /* 0x10171 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    lieu_action_register("L10D18A", P_ARKHAN1, "SG00DIA04", NULL);
                    lieu_action_register("L10D18B", P_ARKHAN1, "SG00DIA04", NULL);
                    lieu_action_register("L10D19", P_ZED84, "SP84DIA59", "SP84ARR01_10");
                }
                lieu_action_register("L10D17A", P_ZED84, "SP84DIA54", "SP84ARR01_10");
                lieu_action_register("L10D17B", P_ZED84, "SP84DIA54", "SP84ARR01_10");
                CB->animation("Sar_10");
                /* 0x10202 */
                if (ST(L10_ZED_APPROACH_STEP) == 0) {
                    CB->joindre(-P_ZED84, 0, 0, 0xA, -1);
                    ST(V_DLG_RANDOM) = CB->random(3);
                    /* 0x1023E */
                    if (ST(V_DLG_RANDOM) == 0) {
                        lieu_action_register("L10D70", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                    /* 0x10269 */
                    if (ST(V_DLG_RANDOM) == 1) {
                        lieu_action_register("L10D71", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                    /* 0x10294 */
                    if (ST(V_DLG_RANDOM) == 2) {
                        lieu_action_register("L10D72", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x102DD */
            if (ST(L10_ZED_TALK_STAGE_7D4) == 1) {
                ST(L10_ZED_TALK_STAGE_7D4) = 2;
                /* 0x102FE */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    lieu_action_register("L10D20", P_ARKHAN1, "SG00DIA04", NULL);
                }
                /* 0x10326 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    lieu_action_register("L10D22", P_ARKHAN1, "SG00DIA04", NULL);
                }
                lieu_action_register("L10D21A", P_ZED84, "SP84DIA54", "SP84ARR01_10");
                lieu_action_register("L10D21B", P_ZED84, "SP84DIA56", "SP84ARR01_10");
                lieu_action_register("L10D21C", P_ZED84, "SP84DIA55", "SP84ARR01_10");
                CB->animation("Sar_10");
                /* 0x103A1 */
                if (ST(L10_ZED_APPROACH_STEP) == 0) {
                    CB->joindre(-P_ZED84, 0, 0, 0xA, -1);
                    ST(V_DLG_RANDOM) = CB->random(3);
                    /* 0x103DD */
                    if (ST(V_DLG_RANDOM) == 0) {
                        lieu_action_register("L10D70", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                    /* 0x10408 */
                    if (ST(V_DLG_RANDOM) == 1) {
                        lieu_action_register("L10D71", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                    /* 0x10433 */
                    if (ST(V_DLG_RANDOM) == 2) {
                        lieu_action_register("L10D72", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x1047C */
            if (ST(L10_ZED_TALK_STAGE_7D4) == 2) {
                /* 0x1048E */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    lieu_action_register("L10D24A", P_ARKHAN1, "SG00DIA04", NULL);
                    lieu_action_register("L10D24B", P_ARKHAN1, "SG00DIA02", NULL);
                }
                /* 0x104CC */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    lieu_action_register("L10D26A", P_ARKHAN1, "SG00DIA04", NULL);
                    lieu_action_register("L10D26B", P_ARKHAN1, "SG00DIA02", NULL);
                }
                lieu_action_register("L10D25A", P_ZED84, "SP84DIA54", "SP84ARR01_10");
                lieu_action_register("L10D25B", P_ZED84, "SP84DIA54", "SP84ARR01_10");
                lieu_action_register("L10D25C", P_ZED84, "SP84DIA56", "SP84ARR01_10");
                CB->animation("Sar_10");
                /* 0x1055D */
                if (ST(L10_ZED_APPROACH_STEP) == 0) {
                    CB->joindre(-P_ZED84, 0, 0, 0xA, -1);
                    ST(V_DLG_RANDOM) = CB->random(3);
                    /* 0x10599 */
                    if (ST(V_DLG_RANDOM) == 0) {
                        lieu_action_register("L10D70", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                    /* 0x105C4 */
                    if (ST(V_DLG_RANDOM) == 1) {
                        lieu_action_register("L10D71", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                    /* 0x105EF */
                    if (ST(V_DLG_RANDOM) == 2) {
                        lieu_action_register("L10D72", P_ZED84, "SP84ACT06", "SP84ARR01_10");
                    }
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x10635 */
        if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_PLAYER_STRIKE) = -1;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_ZED84);
            ST(L10_ZED_APPROACH_STEP) = 10;
            CB->animation("SP84ESA13");
            CB->animation("P84_65535");
            CB->joindre(-P_ZED84, 0, 0, 0, ST(ST_HERO));
            /* 0x106B7 */
            if (ST(L10_ZED_TALK_STAGE_7F4) == 0) {
                ST(L10_ZED_TALK_STAGE_7F4) = 1;
                lieu_action_register("L10D27", P_ZED84, "SP84DIA59", "SP84ARR02_10");
                CB->modify_var(&ST(L10_ZED_APPROACH_STEP), 2);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x10712 */
            if (ST(L10_ZED_TALK_STAGE_7F4) == 1) {
                ST(L10_ZED_TALK_STAGE_7F4) = 2;
                /* 0x1073A */
                if (CB->distance(32, 13) > 125) {
                    lieu_action_register("L10D01B", P_ZED84, "SP84DIA60", "SP84ACT01");
                    CB->evenement(2, 1600);
                } else {
                    lieu_action_register("L10D01B", P_ZED84, "SP84DIA60", "SP84ACT03");
                    CB->evenement(1, 200);
                    return;
                }
            }
        }
    }
    /* 0x107A9 */
    if (CB->direction_zone(22) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 22)) {
        trig22();
    }
}

/* trigger 14 - sub_100107E4 - zone 14 (chest, opening or using an item) */
static void trig14(void) {
    /* 0x107F3 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(18)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x1083E */
        if (ST(ST_PLAYER_WEAPON) == OB_RED_KEY || ST(ST_PLAYER_WEAPON) == OB_KONK_KEY || ST(ST_PLAYER_WEAPON) == OB_SORDOS_KEY || ST(ST_PLAYER_WEAPON) == OB_BOGD_KEY || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_KEY || ST(ST_PLAYER_WEAPON) == OB_STOCK_KEY) {
            /* 0x108AA */
            if (ST(V_ROUTE_STAGE) != 2) {
                /* 0x108BC */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    /* 0x108CE */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        /* 0x108E8 */
                        if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                            CB->animation("SP02ACT71");
                            lieu_action_register("L10D88", P_ARKHAN2, "P02ACT74", "SP02ACT77");
                        } else {
                            CB->animation("SP02ACT70");
                            lieu_action_register("L10D88", P_ARKHAN2, "P02ACT74", "SP02ACT76");
                        }
                    }
                    /* 0x10947 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        /* 0x10961 */
                        if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                            CB->animation("SP02ACT73");
                            lieu_action_register("L10D88", P_ARKHAN2, "P02ACT74", "SP02ACT79");
                        } else {
                            CB->animation("SP02ACT72");
                            lieu_action_register("L10D88", P_ARKHAN2, "SP02ACT74", "SP02ACT78");
                        }
                    }
                }
                /* 0x109C0 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    /* 0x109D2 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        /* 0x109EC */
                        if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                            CB->animation("SP03ACT71");
                            lieu_action_register("L10D89", P_ARKHAN3, "P03ACT74", "SP03ACT77");
                        } else {
                            CB->animation("SP03ACT70");
                            lieu_action_register("L10D89", P_ARKHAN3, "P03ACT74", "SP03ACT76");
                        }
                    }
                    /* 0x10A4B */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        /* 0x10A65 */
                        if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                            CB->animation("SP03ACT73");
                            lieu_action_register("L10D89", P_ARKHAN3, "P03ACT74", "SP03ACT79");
                        } else {
                            CB->animation("SP03ACT72");
                            lieu_action_register("L10D89", P_ARKHAN3, "P03ACT74", "SP03ACT78");
                        }
                    }
                }
            } else {
                /* 0x10AC9 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->dialogue("LCOD26", P_ARKHAN2);
                }
                /* 0x10AEB */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->dialogue("LCOD27", P_ARKHAN3);
                }
                CB->animation("Sar_50");
            }
            CB->animation("ar_50");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("ar_10");
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x10B51 */
        if (ST(ST_PLAYER_WEAPON) == OB_LARGE_KEY) {
            ST(ST_PLAYER_WEAPON) = 0;
            ST(V_ROUTE_STAGE) = 2;
            ST(L10_LEVER_ANIM_STEP) = 1;
            ST(L10_SAVED_CAMERA) = 5;
            /* 0x10B9F */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x10BB1 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP02ACT70");
                }
                /* 0x10BCE */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP02ACT72");
                }
                CB->animation("P02ATT33");
                CB->etat_objet("SOB96", -1, -1, -1, -1, -1, -1, -2, 0);
                /* 0x10C14 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("P02ACT76");
                }
                /* 0x10C31 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("P02ACT78");
                }
                lieu_action_register("L10D90", P_ARKHAN2, "G00DIA01", "Sar_10");
            }
            /* 0x10C67 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x10C79 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP03ACT70");
                }
                /* 0x10C96 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP03ACT72");
                }
                CB->animation("P03ATT33");
                CB->etat_objet("SOB96", -1, -1, -1, -1, -1, -1, -2, 0);
                /* 0x10CDC */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("P03ACT76");
                }
                /* 0x10CF9 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("P03ACT78");
                }
                lieu_action_register("L10D91", P_ARKHAN3, "G00DIA01", "Sar_10");
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x10D52 */
        if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE40 || ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE41) {
            /* 0x10D76 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x10D88 */
                if (ST(V_ROUTE_STAGE) == 1) {
                    /* 0x10D9A */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("SP02ACT70");
                    }
                    /* 0x10DB7 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("SP02ACT72");
                    }
                    lieu_action_register("L10D84", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    /* 0x10DED */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("SP02ACT76");
                    }
                    /* 0x10E0A */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("SP02ACT78");
                    }
                }
                /* 0x10E27 */
                if (ST(V_ROUTE_STAGE) == 2) {
                    CB->retire_zone(1);
                    CB->retire_zone(2);
                    CB->retire_zone(3);
                    CB->retire_zone(4);
                    CB->retire_zone(5);
                    CB->retire_zone(6);
                    CB->animation("SP02ATT27");
                    /* 0x10E86 */
                    if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE41) {
                        ST(V_EVENT_12) = 1;
                        CB->etat_objet("SOB41", -1, -1, -1, -1, -1, -1, -2, 0);
                        CB->link_objet(ST(ST_HERO), "OB41", 0x1B, 0);
                        CB->modify_var(&ST(ST_PLAYER_WEAPON), 0);
                        CB->evenement(12, 300);
                    }
                    /* 0x10F03 */
                    if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE40) {
                        ST(V_EVENT_11) = 1;
                        CB->etat_objet("SOB40", -1, -1, -1, -1, -1, -1, -2, 0);
                        CB->link_objet(ST(ST_HERO), "OB40", 0x1B, 0);
                        CB->modify_var(&ST(ST_PLAYER_WEAPON), 0);
                        CB->evenement(11, 6000);
                        CB->print_msg("evenement11 lanc\351", 0, 0xF0);
                    }
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar_10");
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x10FBB */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x10FCD */
                if (ST(V_ROUTE_STAGE) == 1) {
                    /* 0x10FDF */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("SP03ACT70");
                    }
                    /* 0x10FFC */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("SP03ACT72");
                    }
                    lieu_action_register("L10D85", P_ARKHAN3, "P03ACT74", "SP03ACT74");
                    /* 0x11032 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("SP03ACT76");
                    }
                    /* 0x1104F */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("SP03ACT78");
                    }
                }
                /* 0x1106C */
                if (ST(V_ROUTE_STAGE) == 2) {
                    CB->animation("SP03ATT27");
                    /* 0x11089 */
                    if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE41) {
                        ST(V_EVENT_12) = 1;
                        CB->etat_objet("SOB41", -1, -1, -1, -1, -1, -1, -2, 0);
                        CB->link_objet(ST(ST_HERO), "OB41", 0x1B, 0);
                        CB->modify_var(&ST(ST_PLAYER_WEAPON), 0);
                        CB->evenement(12, 300);
                    }
                    /* 0x11106 */
                    if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE40) {
                        ST(V_EVENT_11) = 1;
                        ST(V_ROUTE_STAGE) = 3;
                        CB->etat_objet("SOB40", -1, -1, -1, -1, -1, -1, -2, 0);
                        CB->link_objet(ST(ST_HERO), "OB40", 0x1B, 0);
                        CB->modify_var(&ST(ST_PLAYER_WEAPON), 0);
                        CB->evenement(11, 6000);
                        CB->retire_zone(1);
                        CB->retire_zone(2);
                    }
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar_10");
                CB->bloque(BLOQUE_NONE);
                return;
            }
            return;
        }
        /* 0x111D6 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x111E8 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0x111FA */
                if (ST(ST_VIE(P_DHORKAN95)) == 0 || ST(V_ROUTE_STAGE) >= 2) {
                    CB->animation("SP01ACT30");
                    lieu_action_register("L10D04A", P_ARKHAN1, "G00DIA07", NULL);
                    CB->bruitage("L10B01", -1, -1);
                    CB->animation("ar_1000");
                    CB->modify_var(&ST(ST_VIE(P_DHORKAN95)), 1);
                }
                /* 0x11273 */
                if (ST(ST_VIE(P_DHORKAN95)) == 1 && ST(V_ROUTE_STAGE) < 2) {
                    CB->animation("SG00OUV09");
                    CB->dialogue("LCOD12", P_ARKHAN1);
                    CB->animation("sar_10");
                }
            }
            /* 0x112BD */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x112CF */
                if (ST(ST_VIE(P_DHORKAN95)) == 0 || ST(V_ROUTE_STAGE) >= 2) {
                    /* 0x112FB */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP02ACT04");
                    }
                    /* 0x11320 */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP02ACT30");
                    }
                    lieu_action_register("L10D04B", P_ARKHAN2, "G00DIA07", NULL);
                    CB->bruitage("L10B01", -1, -1);
                    CB->animation("ar_1000");
                    CB->modify_var(&ST(ST_VIE(P_DHORKAN95)), 1);
                }
                /* 0x11387 */
                if (ST(ST_VIE(P_DHORKAN95)) == 1 && ST(V_ROUTE_STAGE) < 2) {
                    CB->animation("SG00OUV09");
                    CB->dialogue("LCOD13", P_ARKHAN2);
                    CB->animation("sar_10");
                }
            }
            /* 0x113D1 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x113E3 */
                if (ST(ST_VIE(P_DHORKAN95)) == 0 || ST(V_ROUTE_STAGE) >= 2) {
                    /* 0x1140F */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP03ACT04");
                    }
                    /* 0x11434 */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP03ACT30");
                    }
                    lieu_action_register("L10D04C", P_ARKHAN3, "G00DIA07", NULL);
                    CB->bruitage("L10B01", -1, -1);
                    CB->animation("ar_1000");
                    CB->modify_var(&ST(ST_VIE(P_DHORKAN95)), 1);
                }
                /* 0x1149B */
                if (ST(ST_VIE(P_DHORKAN95)) == 1 && ST(V_ROUTE_STAGE) < 2) {
                    CB->animation("SG00OUV09");
                    CB->dialogue("LCOD14", P_ARKHAN3);
                    CB->animation("sar_10");
                }
            }
        }
        /* 0x114E5 */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0x114F7 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0x11509 */
                if (ST(ST_VIE(P_DHORKAN95)) == 0 || ST(V_ROUTE_STAGE) >= 2) {
                    CB->animation("SP01ACT31");
                    lieu_action_register("L10D666", P_ARKHAN1, "G00DIA07", NULL);
                    CB->bruitage("L10B01", -1, -1);
                    CB->animation("ar_1000");
                    CB->modify_var(&ST(ST_VIE(P_DHORKAN95)), 1);
                }
                /* 0x11582 */
                if (ST(ST_VIE(P_DHORKAN95)) == 1 && ST(V_ROUTE_STAGE) < 2) {
                    CB->animation("SG00OUV09");
                    CB->dialogue("LCOD15", P_ARKHAN1);
                    CB->animation("sar_10");
                }
            }
            /* 0x115CC */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x115DE */
                if (ST(ST_VIE(P_DHORKAN95)) == 0 || ST(V_ROUTE_STAGE) >= 2) {
                    /* 0x1160A */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP02ACT05");
                    }
                    /* 0x1162F */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP02ACT31");
                    }
                    lieu_action_register("L10D667", P_ARKHAN2, "G00DIA07", NULL);
                    CB->bruitage("L10B01", -1, -1);
                    CB->animation("ar_1000");
                    CB->modify_var(&ST(ST_VIE(P_DHORKAN95)), 1);
                }
                /* 0x11696 */
                if (ST(ST_VIE(P_DHORKAN95)) == 1 && ST(V_ROUTE_STAGE) < 2) {
                    CB->animation("SG00OUV09");
                    CB->dialogue("LCOD16", P_ARKHAN2);
                    CB->animation("sar_10");
                }
            }
            /* 0x116E0 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x116F2 */
                if (ST(ST_VIE(P_DHORKAN95)) == 0 || ST(V_ROUTE_STAGE) >= 2) {
                    /* 0x1171E */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP03ACT05");
                    }
                    /* 0x11743 */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP03ACT31");
                    }
                    lieu_action_register("L10D668", P_ARKHAN3, "G00DIA07", NULL);
                    CB->bruitage("L10B01", -1, -1);
                    CB->animation("ar_1000");
                    CB->modify_var(&ST(ST_VIE(P_DHORKAN95)), 1);
                }
                /* 0x117AA */
                if (ST(V_ROUTE_STAGE) < 2) {
                    CB->animation("SG00OUV09");
                    CB->dialogue("LCOD17", P_ARKHAN3);
                    CB->animation("sar_10");
                }
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("ar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 15 - sub_10011804 - zone 15 */
static void trig15(void) {
    /* 0x11813 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(18)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x11852 */
        if (ST(ST_PLAYER_WEAPON) == OB_RED_KEY || ST(ST_PLAYER_WEAPON) == OB_KONK_KEY || ST(ST_PLAYER_WEAPON) == OB_SORDOS_KEY || ST(ST_PLAYER_WEAPON) == OB_BOGD_KEY || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_KEY || ST(ST_PLAYER_WEAPON) == OB_STOCK_KEY || ST(ST_PLAYER_WEAPON) == OB_LARGE_KEY) {
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x118DC */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x118EE */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x11908 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP02ACT71");
                        lieu_action_register("L10D86", P_ARKHAN2, "P02ACT74", "SP02ACT77");
                    } else {
                        CB->animation("SP02ACT70");
                        lieu_action_register("L10D86", P_ARKHAN2, "P02ACT74", "SP02ACT76");
                    }
                }
                /* 0x11967 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x11981 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP02ACT73");
                        lieu_action_register("L10D86", P_ARKHAN2, "P02ACT74", "SP02ACT79");
                    } else {
                        CB->animation("SP02ACT72");
                        lieu_action_register("L10D86", P_ARKHAN2, "SP02ACT74", "SP02ACT78");
                    }
                }
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x11A09 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x11A1B */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x11A35 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP03ACT71");
                        lieu_action_register("L10D87", P_ARKHAN3, "P03ACT74", "SP03ACT77");
                    } else {
                        CB->animation("SP03ACT70");
                        lieu_action_register("L10D87", P_ARKHAN3, "P03ACT74", "SP03ACT76");
                    }
                }
                /* 0x11A94 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x11AAE */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        CB->animation("SP03ACT73");
                        lieu_action_register("L10D87", P_ARKHAN3, "P03ACT74", "SP03ACT79");
                    } else {
                        CB->animation("SP03ACT72");
                        lieu_action_register("L10D87", P_ARKHAN3, "P03ACT74", "SP03ACT78");
                    }
                }
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            return;
        }
        trig14();
    }
}

/* trigger 16 - sub_10011B39 - zone 16 */
static void trig16(void) {
    /* 0x11B48 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(18)) {
        /* 0x11B7C */
        if (ST(ST_PLAYER_WEAPON) == OB_RED_KEY || ST(ST_PLAYER_WEAPON) == OB_KONK_KEY || ST(ST_PLAYER_WEAPON) == OB_SORDOS_KEY || ST(ST_PLAYER_WEAPON) == OB_BOGD_KEY || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_KEY || ST(ST_PLAYER_WEAPON) == OB_STOCK_KEY || ST(ST_PLAYER_WEAPON) == OB_LARGE_KEY) {
            trig15();
            return;
        }
        trig14();
    }
}

/* trigger 17 - sub_10011C02 - zone 17 */
static void trig17(void) {
    /* 0x11C11 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(18)) {
        /* 0x11C45 */
        if (ST(ST_PLAYER_WEAPON) == OB_RED_KEY || ST(ST_PLAYER_WEAPON) == OB_KONK_KEY || ST(ST_PLAYER_WEAPON) == OB_SORDOS_KEY || ST(ST_PLAYER_WEAPON) == OB_BOGD_KEY || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_KEY || ST(ST_PLAYER_WEAPON) == OB_STOCK_KEY || ST(ST_PLAYER_WEAPON) == OB_LARGE_KEY) {
            trig15();
            return;
        }
        trig14();
    }
}

/* trigger 22 - sub_10011CF7 - zone 22 */
static void trig22(void) {
    /* 0x11D06 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0) && ST(ST_HERO) == P_ARKHAN1) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x11D60 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            ST(V_DLG_RANDOM) = CB->random(2);
            /* 0x11D89 */
            if (ST(V_DLG_RANDOM) == 0) {
                lieu_action_register("L10D67", P_ARKHAN1, "SP01ATT08", NULL);
            }
            /* 0x11DB1 */
            if (ST(V_DLG_RANDOM) == 1) {
                lieu_action_register("L10D68", P_ARKHAN1, "SP01ATT08", NULL);
            }
        }
        /* 0x11DD9 */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->animation("SP01ATT08_1000");
            CB->animation("ar_10");
            CB->dialogue("L10D69A", P_ARKHAN1);
            lieu_action_register("L10D69B", P_ARKHAN1, "SP01ATT10", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->ajoute_zone(29, 4476, 422, 325, 80, 1);
        CB->bloque(BLOQUE_NONE);
    }
}

/* trigger 29 - sub_10011E99 - zone 29 */
static void trig29(void) {
    /* 0x11EA8 */
    if (ST(ST_UPDOWN_KEY) == 1 && CB->direction_spec(0)) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT_34) = 0;
        CB->evenement(34, -1);
        CB->retire_zone(29);
        lieu_action_register("LCOD66", P_ARKHAN1, "SP01ATT28", "ar_10");
        CB->bloque(BLOQUE_NONE);
        CB->evenement(34, 6000);
        return;
    }
}

/* trigger 30 - sub_10011F2E - zone 30 (Delia) */
static void trig30(void) {
    /* 0x11F3D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x5208) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 11) && !CB->direction_perso(P_DELIA)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x11FBE */
        if (ST(ST_HERO) <= 1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", "Sar_10");
        }
        /* 0x11FE9 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0x12014 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x12067 */
    if (CB->direction_perso(P_DELIA)) {
        /* 0x12076 */
        if (ST(V_PLAYER_STRIKE) > 1 && ST(ST_VIE(P_DELIA)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0) {
            ST(V_DELIA_PHASE) = 20;
            ST(V_DELIA_KILLED) = 10;
            ST(ST_SCRIPT_BUSY) = 1;
            ST(ST_VIE(P_DELIA)) = VIE_DEAD;
            CB->stop_joindre(-P_DELIA);
            CB->combat(P_DELIA, 0);
            CB->divers_empile(DV_BLOOD_AT_BONE, 10, 0);
            CB->select_pile(0, 1);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            return;
        }
        /* 0x1213B */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0x1216B */
            if (ST(ST_VIE(P_DELIA)) < 2) {
                ST(ST_SCRIPT_BUSY) = 1;
                CB->bloque(BLOQUE_ALL);
                /* 0x12194 */
                if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 0)) {
                    /* 0x121BD */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L10D13A", P_ARKHAN2, "SG00DIA02", "Sar");
                        lieu_action_register("L10D13B", P_ARKHAN2, "SG00DIA02", "Sar");
                    }
                    /* 0x12201 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L10D13C", P_ARKHAN3, "SG00DIA02", "Sar");
                        lieu_action_register("L10D13D", P_ARKHAN3, "SG00DIA02", "Sar");
                    }
                    lieu_action_register("L10D14", P_DELIA, "SP10DIA50", "SP10ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                }
                /* 0x12277 */
                if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 0)) {
                    /* 0x122A0 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0x122CB */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("LCOD19", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    lieu_action_register("L10D92", P_DELIA, "SP10DIA54", "SP10ARR01_10");
                    CB->animation("G00PRE82");
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                }
            }
            /* 0x1233E */
            if (ST(ST_VIE(P_DELIA)) == VIE_DEAD) {
                CB->fouille(P_DELIA);
            }
        }
        /* 0x12358 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == P_DELIA) {
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x12394 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L10D93", P_ARKHAN2);
            }
            /* 0x123B6 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L10D94", P_ARKHAN3);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        }
        /* 0x123E6 */
        if (ST(V_PLAYER_STRIKE) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DELIA)) == 1) {
            ST(L10_COMPANION_MODE) = 10;
            CB->stop_joindre(-P_DELIA);
            CB->bloque(BLOQUE_ALL);
            ST(V_PLAYER_STRIKE) = -1;
            /* 0x12456 */
            if (ST(V_DELIA_HITS) == 1) {
                ST(V_DELIA_PHASE) = 20;
                ST(V_DELIA_KILLED) = 10;
                ST(ST_VIE(P_DELIA)) = VIE_DEAD;
                CB->animation("SP10IMP51");
                CB->animation("P10cad_10");
                CB->animation("Sar_10");
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x124C3 */
            if (ST(V_DELIA_HITS) == 0) {
                ST(V_DELIA_HITS) = 1;
                CB->animation("SP10ESA01");
                CB->animation("P10GAR01_10");
                CB->dialogue("L10D95", P_DELIA);
                CB->animation("Sar_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                return;
            }
        }
    }
}

/* trigger 31 - sub_1001252F - zone 31 (Kalhi13) */
static void trig31(void) {
    /* 0x1253E */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x5208) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 11) && !CB->direction_perso(P_KALHI13)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x125BF */
        if (ST(ST_HERO) <= 1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", "Sar_10");
        }
        /* 0x125EA */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0x12615 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x12668 */
    if (CB->direction_perso(P_KALHI13)) {
        /* 0x12677 */
        if (ST(V_PLAYER_STRIKE) > 1 && ST(ST_VIE(P_KALHI13)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_GUARDIAN15_RESULT) != 2 && ST(L10_GUARDIAN15_RESULT) != 3) {
            ST(V_KALHI_KILLED) = 10;
            ST(ST_SCRIPT_BUSY) = 1;
            ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
            CB->select_pile(1, 2);
            CB->stop_joindre(-P_KALHI13);
            CB->combat(P_KALHI13, 0);
            CB->select_pile(0, 1);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        }
        /* 0x1274A */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN15)) > 1 && ST(L10_GUARDIAN15_CHASE_STEP) != 0 && ST(L10_GUARDIAN15_CHASE_STEP) < 5) {
            ST(L10_GUARDIAN15_CHASE_STEP) = 5;
            CB->bloque(BLOQUE_ALL);
            ST(V_KALHI_PHASE) = 10;
            /* 0x127D9 */
            if (ST(L10_GUARDIAN15_TALK_DONE) == 0) {
                ST(L10_GUARDIAN15_TALK_DONE) = 1;
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                /* 0x12834 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D51", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0x1285F */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D52", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D53", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                /* 0x128A3 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D54", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0x128CE */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D55", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D56", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
            CB->joindre(-P_KALHI13, 0, 0, 0x4650, -1);
            /* 0x12933 */
            if (CB->divers(DV_DESC_POS, P_KALHI13, 0) > 4460 || CB->divers(DV_DESC_POS, P_KALHI13, 2) < 750) {
                CB->joindre(-P_KALHI13, 4450, 800, -1, -1);
            }
            CB->animation("SP13ACT20");
            CB->animation("P13_65535");
            CB->divers_empile(DV_BIG_ANIM, 2, 0);
            CB->divers_empile(DV_MUSIC, 1, 4);
            ST(ST_LEVEL) = 10;
            CB->change_piece(14, -1);
            ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
        }
        /* 0x129E2 */
        if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_GUARDIAN15_FIGHT) == 0) {
            /* 0x12A21 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->bloque(BLOQUE_ALL);
                CB->print_msg("test3 passe \340 100 ln 3538", 0, 0xF0);
                ST(L10_COMPANION_MODE) = 100;
                ST(L10_COMPANION_CHATTER_ARMED) = 2;
                ST(ST_SCRIPT_BUSY) = 1;
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
                /* 0x12AAE */
                if (ST(L10_KALHI_TALK_COUNT) == 0) {
                    ST(L10_KALHI_TALK_COUNT) = 1;
                    /* 0x12ACF */
                    if (ST(L10_KALHI_TALK_DONE) == 1) {
                        /* 0x12AE1 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L10D58", P_ARKHAN2, "SG00DIA05", "Sar");
                        }
                        /* 0x12B0C */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L10D59", P_ARKHAN3, "SG00DIA05", "Sar");
                        }
                        lieu_action_register("L10D60", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                    }
                    /* 0x12B50 */
                    if (ST(L10_KALHI_TALK_DONE) == 0) {
                        ST(L10_KALHI_TALK_DONE) = 1;
                        /* 0x12B71 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L10D61A", P_ARKHAN2, "SG00DIA05", "Sar");
                        }
                        lieu_action_register("L10D61B", P_ARKHAN2, "SG00DIA05", "Sar");
                        /* 0x12BB5 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L10D62", P_ARKHAN3, "SG00DIA05", "Sar");
                        }
                        lieu_action_register("L10D63", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0x12C2B */
                if (ST(L10_KALHI_TALK_COUNT) == 1) {
                    /* 0x12C3D */
                    if (ST(ST_PLAYER_WEAPON) == 0 || (ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 0, 2)) || (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                        /* 0x12CBB */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L10D65", P_ARKHAN2, "SG00DIA06", "Sar_10");
                        }
                        /* 0x12CE6 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L10D66", P_ARKHAN3, "SG00DIA06", "Sar_10");
                        }
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x12D43 */
                    if (ST(ST_PLAYER_WEAPON) != 1 && ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        /* 0x12DAC */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L10D100", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                        }
                        /* 0x12DD7 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L10D101", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                        }
                        lieu_action_register("L10D102", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                        CB->animation("G00PRE82");
                        CB->animation("ar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x12E63 */
                    if (ST(ST_PLAYER_WEAPON) == 1) {
                        /* 0x12E75 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L03D518", P_ARKHAN2, "SG00DIA02", NULL);
                            CB->dialogue("L03D519", P_ARKHAN2);
                        }
                        /* 0x12EAD */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            CB->dialogue("L03D520", P_ARKHAN3);
                        }
                        lieu_pre_scene(ST(ST_HERO), P_KALHI13, 1, 6, 5);
                        CB->dialogue("L03D521", P_KALHI13);
                        CB->modify_var(&ST(L10_KALHI_SWAP_REQ), 1);
                        return;
                    }
                }
            }
            /* 0x12F16 */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD) {
                CB->fouille(P_KALHI13);
            }
        }
        /* 0x12F30 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == P_KALHI13 && ST(ST_SCRIPT_BUSY) == 0) {
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x12F7B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L10D93", P_ARKHAN2);
            }
            /* 0x12F9D */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L10D94", P_ARKHAN3);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        }
        /* 0x12FCD */
        if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI13)) == 1) {
            CB->select_pile(0, 2);
            CB->stop_joindre(-P_KALHI13);
            ST(L10_COMPANION_MODE) = 100;
            CB->bloque(BLOQUE_ALL);
            ST(V_PLAYER_STRIKE) = -1;
            CB->animation("SP13ESA15");
            CB->animation("P13_65535");
            /* 0x13051 */
            if (ST(V_KALHI13_TALK_C) == 0) {
                ST(V_KALHI13_TALK_C) = 1;
                lieu_action_register("L10D106", P_KALHI13, "P13DIA59_10", "SP13ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x130B9 */
            if (ST(V_KALHI13_TALK_C) == 1) {
                ST(V_KALHI13_TALK_C) = 2;
                lieu_action_register("L10D107", P_KALHI13, "P13DIA59_10", "SP13ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x13121 */
            if (ST(V_KALHI13_TALK_C) == 2) {
                ST(V_KALHI_PHASE) = 8;
                lieu_action_register("L10D108", P_KALHI13, "P13GAR26_10", "SP13GAR02_10");
                ST(L10_KALHI_TALK_MARK) = 3;
                CB->bloque(BLOQUE_NONE);
                ST(V_ENGAGED_PERSONA) = P_KALHI13;
                CB->combat(P_KALHI13, 0);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x1319A */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_GUARDIAN15_FIGHT) == 1 && ST(ST_VIE(P_KALHI13)) == 1 && ST(ST_VIE(P_GUARDIAN15)) == 1) {
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            /* 0x13205 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L11D06", P_ARKHAN2, "SG02ATT01", NULL);
            }
            /* 0x1322D */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L11D07", P_ARKHAN3, "SG03ATT01", NULL);
            }
            CB->animation("Sar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 32 - sub_10013277 - zone 32 (Kalhi33) */
static void trig32(void) {
    /* 0x13286 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x5208) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 11) && !CB->direction_perso(P_KALHI33)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x13307 */
        if (ST(ST_HERO) <= 1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", "Sar_10");
        }
        /* 0x13332 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0x1335D */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x133B0 */
    if (CB->direction_perso(P_KALHI33)) {
        /* 0x133BF */
        if (ST(V_PLAYER_STRIKE) > 1 && ST(ST_VIE(P_KALHI33)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_GUARDIAN15_RESULT) != 2 && ST(L10_GUARDIAN15_RESULT) != 3) {
            ST(V_KALHI_KILLED) = 10;
            ST(ST_SCRIPT_BUSY) = 1;
            ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
            CB->select_pile(1, 2);
            CB->stop_joindre(-P_KALHI33);
            CB->combat(P_KALHI33, 0);
            CB->divers_empile(DV_BLOOD_AT_BONE, 33, 0);
            CB->select_pile(0, 1);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            return;
        }
        /* 0x134A9 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(10, 7) < 250 && ST(L10_GUARDIAN15_CHASE_STEP) != 0 && ST(L10_GUARDIAN15_CHASE_STEP) < 5 && (ST(ST_VIE(P_GUARDIAN15)) > 1 || (ST(L10_GUARDIAN15_RESULT) == 3 && ST(L10_GUARDIAN15_FIGHT) != 1))) {
            ST(L10_GUARDIAN15_CHASE_STEP) = 5;
            CB->bloque(BLOQUE_ALL);
            ST(V_KALHI_PHASE) = 10;
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI33);
            CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
            /* 0x135AE */
            if (ST(ST_VIE(P_GUARDIAN15)) == 1 && ST(L10_GUARDIAN15_AMBUSH_STEP) >= 7) {
                ST(ST_VIE(P_GUARDIAN15)) = 4;
                CB->pos_perso(P_GUARDIAN15, 4480, 600, 0x88B8, 4, "P55ARR04", 0x21, 0);
            }
            /* 0x13606 */
            if (ST(L10_GUARDIAN15_RESULT) == 3) {
                /* 0x13618 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D47", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0x13643 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D48", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D49", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
            }
            /* 0x13687 */
            if (ST(L10_GUARDIAN15_RESULT) == 2 && ST(L10_GUARDIAN15_TALK_DONE) == 0) {
                ST(L10_GUARDIAN15_TALK_DONE) = 1;
                /* 0x136BA */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D51", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0x136E5 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D52", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D53", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                /* 0x13729 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L10D54", P_ARKHAN2, "SG00DIA05", "Sar");
                }
                /* 0x13754 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L10D55", P_ARKHAN3, "SG00DIA05", "Sar");
                }
                lieu_action_register("L10D56", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
            lieu_action_register("L10D57", P_KALHI33, "SP33DIA55", "SP33ARR01_10");
            CB->joindre(-P_KALHI33, 0, 0, 0x4650, -1);
            /* 0x137D2 */
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
        /* 0x13881 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(L10_GUARDIAN15_FIGHT) == 0) {
            /* 0x138C0 */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->bloque(BLOQUE_ALL);
                CB->print_msg("test3 passe \340 100 ln 3810", 0, 0xF0);
                ST(L10_COMPANION_MODE) = 100;
                ST(L10_COMPANION_CHATTER_ARMED) = 2;
                ST(ST_SCRIPT_BUSY) = 1;
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                CB->joindre(ST(ST_HERO), 0, 0, 0, P_KALHI33);
                /* 0x13949 */
                if (ST(ST_PLAYER_WEAPON) == 0 || (ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 0, 2)) || (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                    /* 0x139D9 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L10D44", P_ARKHAN2, "SG00DIA05", "Sar");
                    }
                    /* 0x13A04 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L10D45", P_ARKHAN3, "SG00DIA05", "Sar");
                    }
                    lieu_action_register("L10D46", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                    return;
                }
                /* 0x13A7A */
                if (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) != 1 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    /* 0x13AE3 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L10D100", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0x13B0E */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L10D101", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    lieu_action_register("L10D102", P_KALHI33, "SP13DIA54", "SP13ARR01_10");
                    CB->animation("G00PRE82");
                    CB->animation("ar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                    return;
                }
                /* 0x13B9A */
                if (ST(ST_PLAYER_WEAPON) == 1) {
                    /* 0x13BAC */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L10D130A", P_ARKHAN2, "SG00DIA02", NULL);
                        CB->dialogue("L10D130B", P_ARKHAN2);
                    }
                    /* 0x13BE4 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->dialogue("L10D131", P_ARKHAN3);
                    }
                    lieu_pre_scene(ST(ST_HERO), P_KALHI13, 1, 6, 5);
                    CB->dialogue("L10D132", P_KALHI13);
                    CB->modify_var(&ST(L10_KALHI_SWAP_REQ), 1);
                    return;
                }
            }
            /* 0x13C4D */
            if (ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
                CB->fouille(P_KALHI33);
            }
        }
        /* 0x13C67 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == P_KALHI33 && ST(ST_SCRIPT_BUSY) == 0) {
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x13CB2 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L10D93", P_ARKHAN2);
            }
            /* 0x13CD4 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L10D94", P_ARKHAN3);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        }
        /* 0x13D04 */
        if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI33)) == 1) {
            CB->stop_joindre(-P_KALHI33);
            ST(V_PLAYER_STRIKE) = -1;
            CB->bloque(BLOQUE_ALL);
            ST(L10_COMPANION_MODE) = 100;
            CB->animation("SP33ESA15");
            CB->animation("P33_65535");
            /* 0x13D7B */
            if (ST(V_KALHI13_TALK_C) == 0) {
                ST(V_KALHI13_TALK_C) = 1;
                lieu_action_register("L10D106", P_KALHI33, "P33DIA59_10", "SP33ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x13DE3 */
            if (ST(V_KALHI13_TALK_C) == 1) {
                ST(V_KALHI13_TALK_C) = 2;
                lieu_action_register("L10D107", P_KALHI33, "P33DIA59_10", "SP33ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L10_COMPANION_MODE), 1);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x13E4B */
            if (ST(V_KALHI13_TALK_C) == 2) {
                ST(V_KALHI_PHASE) = 8;
                lieu_action_register("L10D108", P_KALHI33, "P33GAR26", "SP33GAR02_10");
                CB->bloque(BLOQUE_NONE);
                ST(V_ENGAGED_PERSONA) = P_KALHI33;
                CB->combat(P_KALHI33, 0);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x13EB5 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L10_GUARDIAN15_FIGHT) == 1 && ST(ST_VIE(P_KALHI33)) == 1 && ST(ST_VIE(P_GUARDIAN15)) == 1) {
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            /* 0x13F20 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L11D06", P_ARKHAN2, "SG02ATT01", NULL);
            }
            /* 0x13F48 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L11D07", P_ARKHAN3, "SG03ATT01", NULL);
            }
            CB->animation("Sar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 33 - sub_10013F92 - zone 33 (Guardian15) */
static void trig33(void) {
    /* 0x13FA1 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x5208) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 11) && !CB->direction_perso(P_GUARDIAN15)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x14022 */
        if (ST(ST_HERO) <= 1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", "Sar_10");
        }
        /* 0x1404D */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0x14078 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x140C7 */
    if (ST(ST_ACTION_KEY) == 1 && CB->direction_perso(P_GUARDIAN15) && ST(ST_SCRIPT_BUSY) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x14104 */
        if (ST(V_ENGAGED_PERSONA) == P_GUARDIAN15 && ST(ST_VIE(P_GUARDIAN15)) == 1) {
            /* 0x14128 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD20", P_ARKHAN2);
            }
            /* 0x1414A */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD21", P_ARKHAN3);
            }
        }
        /* 0x14169 */
        if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN15)) == VIE_DEAD) {
            CB->fouille(P_GUARDIAN15);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
}

/* trigger 34 - sub_100141A2 - zone 34 (Guardian22) */
static void trig34(void) {
    /* 0x141B5 */
    if (CB->direction_perso(85) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0x141F4 */
        if (ST(ST_VIE(85)) < 2) {
            ST(L10_GUARDIAN22_TAKEN) = 1;
            ST(ST_VIE(85)) = VIE_DEAD;
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            /* 0x1423B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L10D110", P_ARKHAN2, "SG00DIA02", NULL);
                CB->dialogue("L10D112A", 85);
                CB->dialogue("L10D112B", 85);
            }
            /* 0x14283 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L10D111", P_ARKHAN3, "SG00DIA02", NULL);
                CB->dialogue("L10D113A", 85);
                CB->dialogue("L10D113B", 85);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->pos_perso(85, 4405, -646, 0x9470, 2, "P85cad38", 0x22, 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x1430E */
        if (ST(ST_VIE(85)) == VIE_DEAD) {
            CB->fouille(85);
        }
    }
    /* 0x14328 */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(85)) == 1) {
        ST(V_PLAYER_STRIKE) = -1;
        ST(ST_VIE(85)) = VIE_DEAD;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP85cad38_10");
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 36 - sub_100143A8 - zone 36 */
static void trig36(void) {
    /* 0x143BB */
    if (CB->direction_zone(36) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x14411 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x14423 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x1443D */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    CB->animation("SP02ACT71");
                    lieu_action_register("L10D114A", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    lieu_action_register("L10D114B", P_ARKHAN2, "SP02ACT74", NULL);
                } else {
                    CB->animation("SP02ACT70");
                    lieu_action_register("L10D114A", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    lieu_action_register("L10D114B", P_ARKHAN2, "SP02ACT74", NULL);
                }
            }
            /* 0x144C8 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x144E2 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    CB->animation("SP03ACT71");
                    lieu_action_register("L10D115A", P_ARKHAN3, "P03ACT74", "SP03ACT74");
                    lieu_action_register("L10D115B", P_ARKHAN3, "SP03ACT74", NULL);
                } else {
                    CB->animation("SP03ACT70");
                    lieu_action_register("L10D115A", P_ARKHAN3, "P03ACT74", "SP03ACT74");
                    lieu_action_register("L10D115B", P_ARKHAN3, "SP03ACT74", NULL);
                }
            }
        }
        /* 0x1456D */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0x14587 */
            if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0x14599 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02ACT85");
                }
                /* 0x145B6 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03ACT85");
                }
            }
            /* 0x145DB */
            if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0x145ED */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02ACT86");
                }
                /* 0x1460A */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03ACT86");
                }
            }
            /* 0x14627 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L10D116", P_ARKHAN2, "SG00DIA08", "Sar_10");
            }
            /* 0x14652 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L10D117", P_ARKHAN3, "SG00DIA08", "Sar_10");
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
}

/* trigger 37 - sub_1001468F - zone 37 */
static void trig37(void) {
    /* 0x146A2 */
    if (CB->direction_zone(37) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x146F8 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x1470A */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x14724 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    CB->animation("SP02ACT71");
                    lieu_action_register("L10D118A", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    lieu_action_register("L10D118B", P_ARKHAN2, "SP02ACT74", "SP02ACT77");
                } else {
                    CB->animation("SP02ACT70");
                    lieu_action_register("L10D118A", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    lieu_action_register("L10D118B", P_ARKHAN2, "SP02ACT74", "SP02ACT76");
                }
            }
            /* 0x147B5 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x147CF */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    CB->animation("SP03ACT71");
                    lieu_action_register("L10D119", P_ARKHAN3, "P03ACT74", "SP03ACT77");
                } else {
                    CB->animation("SP03ACT70");
                    lieu_action_register("L10D119", P_ARKHAN3, "P03ACT74", "SP03ACT76");
                }
            }
        }
        /* 0x1482E */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->animation("SG00PRE09");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
}

/* trigger 38 - sub_1001485D - zone 38 */
static void trig38(void) {
    /* 0x14870 */
    if (CB->direction_zone(38) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x148C6 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x148D8 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L10D120", P_ARKHAN2, "SG00DIA08", "Sar_10");
            }
            /* 0x14903 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L10D121", P_ARKHAN3, "SG00DIA08", "Sar_10");
            }
        }
        /* 0x1492E */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->animation("SG00PRE09");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
}

/* trigger 39 - sub_1001495D - zone 39 */
static void trig39(void) {
    /* 0x14970 */
    if (CB->direction_zone(39) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x149C6 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x149D8 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x149F2 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    CB->animation("SP02ACT71");
                    lieu_action_register("L10D122A", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    lieu_action_register("L10D122B", P_ARKHAN2, "SP02ACT74", "SP02ACT77");
                } else {
                    CB->animation("SP02ACT70");
                    lieu_action_register("L10D122A", P_ARKHAN2, "P02ACT74", "SP02ACT74");
                    lieu_action_register("L10D122B", P_ARKHAN2, "SP02ACT74", "SP02ACT76");
                }
            }
            /* 0x14A83 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x14A9D */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    CB->animation("SP03ACT71");
                    lieu_action_register("L10D123", P_ARKHAN3, "P03ACT74", "SP03ACT77");
                } else {
                    CB->animation("SP03ACT70");
                    lieu_action_register("L10D123", P_ARKHAN3, "P03ACT74", "SP03ACT76");
                }
            }
        }
        /* 0x14AFC */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->animation("SG00PRE09");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
}

/* trigger 40 - sub_10014B2B - zone 40 */
static void trig40(void) {
    /* 0x14B3E */
    if (CB->direction_zone(40) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x14B94 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x14BA6 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L10D124", P_ARKHAN2, "SG00DIA08", "Sar_10");
            }
            /* 0x14BD1 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L10D125", P_ARKHAN3, "SG00DIA08", "Sar_10");
            }
        }
        /* 0x14BFC */
        if (ST(ST_SPECIAL_MODE) == 0) {
            CB->animation("SG00PRE09");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
}

/* trigger 41 - sub_10009356 - trigger 41 (the room-entry events) */
static void trig41(void) {
    /* 0x9368 */
    if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
        CB->change_piece(10, 3);
        CB->print_msg("je fais le changepiece", 0, 0xF0);
    }
    /* 0x93AE */
    if (ST(V_DELIA_PHASE) < 5 || (ST(V_DELIA_PHASE) == 20 && ST(L10_DELIA_MUSIC_GATE) == 0)) {
        CB->divers(DV_MUSIC, 1, 0);
        CB->divers_empile(DV_MUSIC, 1, 1);
    }
    /* 0x9405 */
    if (ST(ST_LEVEL) == 20 && ST(V_PASSAGE_OPEN) == 0 && ST(V_GUARD_FIGHT_DONE) == 2) {
        ST(V_PASSAGE_OPEN) = 1;
        CB->divers_empile(DV_SCREEN, 3, 0);
    }
    /* 0x9467 */
    if (ST(V_SAVED_HERO) != ST(ST_HERO) && (ST(V_DELIA_PHASE) == 6 || (ST(V_KALHI_PHASE) == 7 && ST(L10_KALHI_ESCORT_STARTED) != 0 && (ST(ST_VIE(P_KALHI13)) == 1 || ST(ST_VIE(P_KALHI33)) == 1) && ST(L10_GUARDIAN15_RESULT) < 2))) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x94FC */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x950E */
            if (ST(V_DELIA_PHASE) == 6) {
                CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD48", P_DELIA, "SP10DIA54", "SP10ARR01_10");
            }
            /* 0x9556 */
            if (ST(V_KALHI_PHASE) == 7 && ST(ST_VIE(P_KALHI13)) == 1) {
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD49", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
            /* 0x95B0 */
            if (ST(V_KALHI_PHASE) == 7 && ST(ST_VIE(P_KALHI33)) == 1) {
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD49", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
            /* 0x960A */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD50", P_ARKHAN2, "SG00DIA02", NULL);
            }
            /* 0x9632 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("LCOD51", P_ARKHAN2, "SG00DIA02", NULL);
            }
        }
        /* 0x965A */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x966C */
            if (ST(V_DELIA_PHASE) == 6) {
                CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD44", P_DELIA, "SP10DIA54", "SP10ARR01_10");
            }
            /* 0x96B4 */
            if (ST(V_KALHI_PHASE) == 7 && ST(ST_VIE(P_KALHI13)) == 1) {
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD45", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
            /* 0x970E */
            if (ST(V_KALHI_PHASE) == 7 && ST(ST_VIE(P_KALHI33)) == 1) {
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD45", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
            /* 0x9768 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD46", P_ARKHAN3, "SG00DIA08", NULL);
            }
            /* 0x9790 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("LCOD47", P_ARKHAN3, "SG00DIA08", NULL);
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x97D1 */
    if (ST(V_ARKHAN1_PROGRESS) == 0 && ST(V_ZED84_TALK_STAGE) == 0 && ST(L10_LORY_FOLLOW_STATE) == 0) {
        CB->change_camera(9, CAM_LOCKED);
    }
    /* 0x9817 */
    if (ST(V_ZED84_TALK_STAGE) == 0 && ST(V_ARKHAN1_PROGRESS) == 0 && ST(ST_HERO) == P_ARKHAN1 && ST(L10_LORY_FOLLOW_STATE) == 0) {
        CB->joindre(P_RYLSADHAR20, 4500, -2720, 0x2EE0, -1);
        CB->joindre(P_LORY, 4380, -2760, 0xD228, -1);
        CB->modify_var(&ST(L10_LORY_FOLLOW_REQ), 1);
        CB->modify_var(&ST(L10_LORY_FOLLOW_STATE), 1);
    }
    /* 0x98C2 */
    if (ST(V_ARKHAN1_PROGRESS) < 3 && ST(V_ZED84_TALK_STAGE) == 1 && ST(ST_HERO) == P_ARKHAN1) {
        CB->select_pile(2, 1);
        CB->divers(DV_RUN, P_ZED84, 1);
        CB->joindre(-P_ZED84, 2900, -1130, -1, -1);
        CB->select_pile(0, 1);
    }
    /* 0x9940 */
    if (ST(V_DELIA_PHASE) == 5) {
        ST(V_DELIA_PHASE) = 6;
        ST(L10_COMPANION_MODE) = 10;
        ST(L10_COMPANION_MODE_SAVED) = ST(L10_COMPANION_MODE);
        CB->joindre(-P_DELIA, 4369, 285, 0x88B8, -1);
    }
    /* 0x999D */
    if (ST(V_KALHI_PHASE) == 7 && ST(ST_VIE(P_KALHI33)) == 1 && ST(L10_COMPANION_CATCHUP_STEP) == 0 && ST(L10_KALHI_ESCORT_STARTED) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(L10_COMPANION_CHATTER_LINE) = 1;
        CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D64A", P_KALHI33, "SP33DIA56", "SP33ARR01_10");
        lieu_action_register("L10D64B", P_KALHI33, "SP33DIA56", "SP33ARR01_10");
        lieu_action_register("L10D64C", P_KALHI33, "SP33DIA59", "SP33ARR01_10");
        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
        CB->modify_var(&ST(L10_COMPANION_CHATTER_ARMED), 1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x9A9C */
    if (ST(V_KALHI_PHASE) == 7 && ST(ST_VIE(P_KALHI13)) == 1 && ST(L10_COMPANION_CATCHUP_STEP) == 0 && ST(L10_KALHI_ESCORT_STARTED) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(L10_COMPANION_CHATTER_LINE) = 1;
        CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L10D64A", P_KALHI13, "SP13DIA56", "SP13ARR01_10");
        lieu_action_register("L10D64B", P_KALHI13, "SP13DIA56", "SP13ARR01_10");
        lieu_action_register("L10D64C", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
        CB->modify_var(&ST(L10_COMPANION_MODE), 1);
        CB->modify_var(&ST(L10_COMPANION_CHATTER_ARMED), 1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x9B9B */
    if (ST(L10_GUARDIAN_CUTSCENE_STAGE) == 1) {
        CB->change_camera(4, CAM_AUTO);
        ST(L10_GUARDIAN_CUTSCENE_STAGE) = 2;
        CB->play_anim(0, 1);
        CB->play_anim(2, 1);
        CB->play_anim(4, 1);
        CB->play_anim(6, 1);
        CB->play_anim(8, 1);
        lieu_action_register("L10D78A", P_SUNSEER1, "SP18DIA59", "SP18ARR01_10");
        lieu_action_register("L10D78B", P_SUNSEER1, "SP18DIA60", "SP18ARR01_10");
        lieu_action_register("L10D78C", P_SUNSEER1, "SP18DIA58", "SP18ARR01_10");
        lieu_action_register("L10D78D", P_SUNSEER1, "SP18DIA54", "SP18ARR01_10");
        lieu_action_register("L10D78E", P_SUNSEER1, "SP18DIA59", "SP18ARR01_10");
        lieu_action_register("L10D78F", P_SUNSEER1, "SP18DIA56", "SP18ARR01_10");
        lieu_action_register("L10D78G", P_SUNSEER1, "SP18DIA60", "SP18ARR01_10");
        lieu_action_register("L10D79", P_GUARDIAN15, "SP55ATT72", "SP55ARR04_10");
        CB->evenement(0, 20);
    }
}
