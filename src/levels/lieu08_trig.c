/*
 * lieu08_trig.c - Native hand-port of the LIEU08.DLL zone trigger
 * handlers (opcode 3). The entry point, INIT, tick and the tick body live
 * in lieu08.c; this file holds the 42-slot trigger table and its handlers.
 *
 * Ported from disassembly (ImageBase 0x10000000). Each function carries
 * its original address. Handlers call the engine only through g_lieu_cb.
 */

#include <stdio.h>
#include <stdlib.h>

#include "libsys/common.h"
#include "lieu_cb.h"
#include "levels/lieu08_int.h"

/* -------------------------------------------------------------------------
 * Forward decls for the trigger table.
 * ---------------------------------------------------------------------- */
static void trig02(void); static void trig03(void);
static void trig04(void); static void trig05(void);
static void trig06(void); static void trig07(void);
static void trig09(void); static void trig10(void);
static void trig11(void); static void trig12(void);
static void trig13(void); static void trig14(void);
static void trig15(void); static void trig16(void);
static void trig17(void); static void trig18(void);
static void trig19(void); static void trig20(void);
static void trig21(void); static void trig24(void);
static void trig25(void); static void trig26(void);
static void trig27(void); static void trig28(void);
static void trig30(void); static void trig31(void);
static void trig32(void); static void trig33(void);
static void trig34(void); static void trig41(void);
static void trig_noop(void);   /* the 10 stub slots */

/* dword_1003FB70[42] - filled at INIT with the 5-byte jmp thunks at
 * 0x10001000+5*i; static here. Slots 1, 22, 23, 29 and 35..40 are the
 * empty stub (identical to LIEU01 sub_1000D30E).
 *   idx handler        idx handler        idx handler
 *   0  10001200        14  10015897        28  10019862
 *   1  1000BC9A stub     15  10015F01        29  10019F02 stub
 *   2  1000BCA5        16  100161C1        30  10019F0D
 *   3  1000DD33        17  10016955        31  1001A8FC
 *   4  1000FB3D        18  10016FC2        32  1001BD20
 *   5  10011133        19  1001762F        33  1001CB83
 *   6  10012707        20  10017C5E        34  1001D014
 *   7  10012AC8        21  1001817A        35  1001D3FA stub
 *   8  10012CE2        22  10018875 stub     36  1001D405 stub
 *   9  1001324F        23  10018880 stub     37  1001D410 stub
 *  10  100137BC        24  1001888B        38  1001D41B stub
 *  11  10013FCD        25  10018D82        39  1001D426 stub
 *  12  100147DE        26  1001906D        40  1001D431 stub
 *  13  100151E1        27  10019528        41  10003DA2
 */
static void (*const g_trigger[42])(void) = {
    l08_trig00, trig_noop, trig02, trig03, trig04, trig05,
    trig06, trig07, l08_trig08, trig09, trig10, trig11,
    trig12, trig13, trig14, trig15, trig16, trig17,
    trig18, trig19, trig20, trig21, trig_noop, trig_noop,
    trig24, trig25, trig26, trig27, trig28, trig_noop,
    trig30, trig31, trig32, trig33, trig34, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig41,
};

/* Op 3: run trigger handler idx (call [4*arg + 0x1003FB70]). */
void l08_trigger(i32 idx) {
    if (idx >= 0 && idx < 42)
        g_trigger[idx]();
}

/* Empty slots: the stub bodies are `push ebp; mov ebp,esp; ... ret`. */
static void trig_noop(void) {
}

/* trigger 00 - sub_10001200 */
void l08_trig00(void) {
    /* 0x1212 */
    if (ST(ST_START_POINT(8)) == 1) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 2;
        ST(L08_FROM_ROOM2) = 1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_RETURN_PART) = 0;
        ST(V_EVENT_6) = 0;
        ST(V_EVENT_10) = 0;
        ST(V_GUARDIAN4_STAGE) = 0;
    }
    /* 0x1299 */
    if (ST(ST_START_POINT(8)) == 2) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 10;
        ST(L08_FROM_ROOM10) = 1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_RETURN_PART) = 0;
        ST(V_EVENT_6) = 0;
        ST(V_EVENT_10) = 0;
        ST(V_GUARDIAN4_STAGE) = 2;
    }
    /* 0x1320 */
    if (ST(ST_START_POINT(8)) == 3) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 10;
        ST(L08_FROM_ROOM10) = 1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_RETURN_PART) = 1;
        ST(V_EVENT_6) = 0;
        ST(V_EVENT_10) = 0;
        ST(V_GUARDIAN4_STAGE) = 3;
    }
    /* 0x13A7 */
    if (ST(ST_START_POINT(8)) == 4) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 2;
        ST(L08_FROM_ROOM2) = 1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_RETURN_PART) = 1;
        ST(V_EVENT_6) = 1;
        ST(V_EVENT_10) = 1;
        ST(L08_BONUS_ROUND) = 0;
    }
    /* 0x142E */
    if (ST(ST_START_POINT(8)) == 6) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(L08_ARENA_UNLOCKED) = 0;
        ST(ST_LEVEL) = 2;
        ST(L08_FROM_ROOM2) = 1;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(V_RETURN_PART) = 1;
        ST(V_EVENT_6) = 2;
        ST(V_EVENT_10) = 2;
        ST(V_GUARDIAN8_STAGE) = 0;
    }
    /* 0x14C4 */
    if (ST(ST_START_POINT(8)) == 7) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
        ST(ST_LEVEL) = 2;
        ST(L08_FROM_ROOM2) = 2;
        ST(V_KALHI_PHASE) = 5;
        ST(V_RETURN_PART) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
    }
    /* 0x154B */
    if (ST(ST_START_POINT(8)) == 8) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
        ST(L08_FROM_ROOM2) = 1;
        ST(ST_LEVEL) = 2;
        ST(V_KALHI_PHASE) = 5;
        ST(ST_VIE(P_KALHI33)) = 1;
    }
    /* 0x15C3 */
    if (ST(ST_START_POINT(8)) == 9) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
        ST(L08_FROM_ROOM2) = 2;
        ST(ST_LEVEL) = 2;
        ST(V_KALHI_PHASE) = 5;
        ST(ST_VIE(P_KALHI33)) = 1;
    }
    /* 0x163B */
    if (ST(ST_START_POINT(8)) == 0xA) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
        ST(L08_FROM_ROOM10) = 2;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(ST_LEVEL) = 10;
        ST(V_DELIA_PHASE) = 6;
        ST(ST_VIE(P_DELIA)) = 1;
    }
    /* 0x16C2 */
    if (ST(ST_START_POINT(8)) == 0xB) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(L08_FROM_ROOM10) = 1;
        ST(ST_LEVEL) = 10;
    }
    /* 0x16FE */
    if (ST(ST_START_POINT(8)) == 0xC) {
        ST(ST_LEVEL) = 10;
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_RETURN_PART) = 1;
        ST(L08_FROM_ROOM10) = 1;
        ST(V_GUARDIAN4_STAGE) = 3;
        ST(V_EVENT_6) = 0;
        ST(V_EVENT_10) = 0;
    }
    /* 0x1776 */
    if (ST(ST_START_POINT(8)) == 0xD) {
        ST(ST_LEVEL) = 10;
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_RETURN_PART) = 1;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(L08_FROM_ROOM10) = 1;
        ST(V_EVENT_6) = 1;
        ST(V_EVENT_10) = 1;
    }
    /* 0x17EE */
    if (ST(ST_START_POINT(8)) == 0xE) {
        ST(ST_HERO) = P_ARKHAN3;
        ST(ST_LEVEL) = 10;
        ST(L08_FROM_ROOM10) = 1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_RETURN_PART) = 1;
        ST(V_EVENT_6) = 1;
        ST(V_EVENT_10) = 1;
        ST(L08_BONUS_ROUND) = 0;
        ST(L08_FLAG_BD0) = 2;
    }
    /* 0x1884 */
    if (ST(ST_START_POINT(8)) == 0xF) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
        ST(L08_FROM_ROOM10) = 2;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(ST_LEVEL) = 10;
        ST(V_DELIA_PHASE) = 0x14;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(L08_DELIA_ROUTE) = 1;
    }
    /* 0x191A */
    if (ST(ST_START_POINT(8)) == 0x63) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_RETURN_PART) = 0;
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
        ST(V_KALHI_PHASE) = 0;
        ST(V_DELIA_PHASE) = 0;
    }
    ST(V_EVENT_0) = 0;
    ST(V_EVENT_1) = 0;
    ST(V_EVENT_2) = 0;
    ST(V_EVENT_33) = 0;
    ST(V_EVENT_34) = 0;
    ST(V_EVENT_35) = 0;
    ST(V_EVENT_36) = 0;
    ST(V_EVENT_37) = 0;
    ST(V_EVENT_38) = 0;
    ST(V_EVENT_39) = 0;
    ST(V_EVENT_41) = 0;
    CB->bruitage("L08AMB1", 0, -1340);
    /* 0x1A4C */
    if ((ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(V_DELIA_PHASE) == 6 || ST(V_DELIA_PHASE) == 7) && ST(ST_VIE(34)) != VIE_DEAD) {
        CB->divers(DV_MUSIC, 1, 0);
    }
    CB->ajoute_zone(8, -983, -1175, 76, 155, 1);
    CB->ajoute_zone(9, -983, -1530, 76, 155, 1);
    CB->ajoute_zone(10, 2550, -1625, 80, 490, 1);
    CB->ajoute_zone(11, 2550, -1055, 80, 490, 1);
    CB->ajoute_zone(12, -20, -954, 150, 155, 0);
    CB->ajoute_zone(13, 1006, -822, 100, 245, 0);
    CB->ajoute_zone(14, 832, -751, 555, 170, 0);
    CB->ajoute_zone(16, 690, -1189, 230, 230, 0);
    CB->ajoute_zone(17, 1531, -1074, 400, 250, 0);
    CB->ajoute_zone(18, 1235, -1563, 250, 250, 0);
    CB->ajoute_zone(19, 589, -1687, 345, 360, 0);
    CB->ajoute_zone(20, 1540, -1720, 345, 360, 0);
    CB->ajoute_zone(25, 831, -943, 253, 300, 0);
    /* 0x1C68 */
    if (ST(L08_TRIG21_DONE) == 0) {
        CB->ajoute_zone(21, 503, -908, 135, 145, 0);
    }
    CB->ajoute_zone(24, -260, -1359, 150, 130, 0);
    CB->ajoute_zone(26, 2459, -666, 85, 285, 0);
    /* 0x1CDA */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && (ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) != 0 && ST(V_DELIA_PHASE) != 0xB && ST(L08_DELIA_ROUTE) == 0))) {
        CB->ajoute_zone(27, 1083, -1608, 250, 250, 0);
    }
    CB->ajoute_zone(28, -55, -1092, 150, 140, 0);
    /* 0x1D76 */
    if ((ST(V_KALHI_PHASE) == 0 || (ST(V_KALHI_PHASE) > 6 && ST(V_KALHI_PHASE) != 0x64 && ST(V_KALHI_PHASE) != 0xC8)) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7)) {
        /* 0x1DE5 */
        if (ST(V_RETURN_PART) == 0 && ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0) {
            /* 0x1E1B */
            if (ST(V_GUARDIAN4_STAGE) == 0) {
                CB->ajoute_zone(35, 833, -1360, 10, 10, 0);
                CB->ajoute_zone(36, 1896, -1376, 10, 10, 0);
                CB->ajoute_zone(37, 815, -1890, 10, 10, 0);
            }
            /* 0x1E7E */
            if (ST(V_GUARDIAN4_STAGE) == 2) {
                CB->ajoute_zone(35, 815, -1890, 10, 10, 0);
                CB->ajoute_zone(36, 833, -1360, 10, 10, 0);
                CB->ajoute_zone(37, 1175, -1360, 10, 10, 0);
                CB->ajoute_zone(38, 1434, -900, 10, 10, 0);
                CB->ajoute_zone(39, 2075, -900, 10, 10, 0);
                CB->ajoute_zone(40, 1958, -1828, 10, 10, 0);
                CB->ajoute_zone(22, 1274, -1915, 10, 10, 0);
            }
        }
        /* 0x1F4D */
        if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->ajoute_zone(35, 1614, -1310, 10, 10, 0);
            CB->ajoute_zone(36, 1214, -1310, 10, 10, 0);
        }
        /* 0x1FB9 */
        if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && ST(V_ZED84_TALK_STAGE) != 0x19 && ST(L08_ARENA_UNLOCKED) != 0) {
            CB->ajoute_zone(35, 467, -1897, 250, 250, 0);
        }
    }
    /* 0x2022 */
    if (ST(L08_TRIG21_DONE) == 0) {
        ST(L08_UNUSED_144) = 0;
        CB->etat_objet("OB77B", 519, -750, -866, 0xC000, 0x8000, 0xEA00, -1, 0);
    }
    /* 0x206D */
    if (ST(ST_LEVEL) == 2) {
        /* 0x207F */
        if (ST(L08_FROM_ROOM2) == 1) {
            CB->pos_perso(ST(ST_HERO), -902, -1087, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        }
        /* 0x20D0 */
        if (ST(L08_FROM_ROOM2) == 2) {
            CB->pos_perso(ST(ST_HERO), -860, -1675, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        }
    }
    /* 0x211E */
    if (ST(ST_LEVEL) == 10) {
        /* 0x2130 */
        if (ST(L08_FROM_ROOM10) == 1) {
            CB->pos_perso(ST(ST_HERO), 2393, -1613, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        }
        /* 0x2181 */
        if (ST(L08_FROM_ROOM10) == 2) {
            CB->pos_perso(ST(ST_HERO), 2393, -1114, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        }
    }
    /* 0x21CF */
    if (ST(ST_LEVEL) != 2 && ST(ST_LEVEL) != 10) {
        CB->pos_perso(ST(ST_HERO), 30, -1136, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
    }
    /* 0x222F */
    if (ST(L08_DELIA_ROUTE) == 1) {
        /* 0x2241 */
        if (ST(V_EVENT_6) != 0x63 && ST(V_EVENT_10) != 0x63) {
            ST(V_EVENT_6) = 0x63;
            ST(V_EVENT_10) = 0x63;
        }
        /* 0x2283 */
        if (ST(ST_VIE(34)) != VIE_DEAD) {
            /* 0x2295 */
            if (ST(L08_ARENA_LOCKED) == 1) {
                /* 0x22A4 */
                if (ST(ST_LEVEL) == 2) {
                    CB->pos_perso(34, CB->divers(DV_DESC_POS, 0x22, 0), CB->divers(DV_DESC_POS, 0x22, 1), CB->divers(DV_DESC_POS, 0x22, 2), 1, "P34GAR01", 0x22, 0);
                }
                /* 0x2303 */
                if (ST(ST_LEVEL) == 10) {
                    /* 0x2315 */
                    if (ST(L08_FROM_ROOM10) == 1) {
                        CB->pos_perso(34, 2093, -1635, 0xC000, 1, "P34GAR01", 0x22, 0);
                    }
                    /* 0x234C */
                    if (ST(L08_FROM_ROOM10) == 2) {
                        CB->pos_perso(34, 2156, -1098, 0xC000, 1, "P34GAR01", 0x22, 0);
                    }
                }
            }
            /* 0x2383 */
            if (ST(L08_ARENA_LOCKED) == 0) {
                CB->pos_perso(34, 1073, -1490, 0x7D61, 1, "P34ATT01", 0x22, 0);
                ST(L08_ARENA_LOCKED) = 1;
            }
        }
        /* 0x23C9 */
        if (ST(ST_VIE(34)) == VIE_DEAD) {
            CB->pos_perso(34, CB->divers(DV_DESC_POS, 0x22, 0), CB->divers(DV_DESC_POS, 0x22, 1), CB->divers(DV_DESC_POS, 0x22, 2), 2, "P34CAD", 0x22, 0);
        }
    }
    /* 0x242B */
    if ((ST(V_KALHI_PHASE) == 0 || (ST(V_KALHI_PHASE) > 6 && ST(V_KALHI_PHASE) != 0x64 && ST(V_KALHI_PHASE) != 0xC8)) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7)) {
        /* 0x249A */
        if (ST(V_EVENT_6) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
            /* 0x24BE */
            if (ST(V_GUARDIAN4_STAGE) == 0) {
                CB->pos_perso(P_GUARDIAN4, 833, -1360, 0xCD78, 1, "P44ARR02", 3, 601);
            }
            /* 0x24F8 */
            if (ST(V_GUARDIAN4_STAGE) == 1 || ST(V_GUARDIAN4_STAGE) == 5) {
                /* 0x2519 */
                if (ST(ST_LEVEL) == 2) {
                    /* 0x252B */
                    if (ST(ST_VIE(P_GUARDIAN4)) < 2) {
                        CB->pos_perso(P_GUARDIAN4, 534, -1100, 0x5D7F, 1, "P44ARR02", 3, 601);
                    } else {
                        CB->pos_perso(P_GUARDIAN4, 0, 0, 0, 2, "P44CAD", 3, 0);
                    }
                }
                /* 0x2583 */
                if (ST(ST_LEVEL) == 10) {
                    /* 0x2595 */
                    if (ST(ST_VIE(P_GUARDIAN4)) < 2) {
                        CB->pos_perso(P_GUARDIAN4, 1381, -1418, 0xECAD, 1, "P44ARR02", 3, 601);
                    } else {
                        CB->pos_perso(P_GUARDIAN4, 0, 0, 0, 2, "P44CAD", 3, 0);
                    }
                }
            }
            /* 0x25F0 */
            if (ST(V_GUARDIAN4_STAGE) == 2) {
                /* 0x2602 */
                if (ST(ST_VIE(P_GUARDIAN4)) < 2) {
                    CB->pos_perso(P_GUARDIAN4, 815, -1890, 0, 1, "P44ARR02", 3, 601);
                } else {
                    CB->pos_perso(P_GUARDIAN4, 0, 0, 0, 2, "P44CAD", 3, 0);
                }
            }
            /* 0x265A */
            if (ST(V_GUARDIAN4_STAGE) == 3) {
                CB->pos_perso(P_GUARDIAN4, 1256, -1176, 0x79C2, 1, "P44ARR02", 3, 601);
            }
            /* 0x2694 */
            if (ST(V_GUARDIAN4_STAGE) == 4) {
                /* 0x26A6 */
                if (ST(ST_VIE(P_GUARDIAN4)) < 2) {
                    CB->pos_perso(P_GUARDIAN4, 1536, -1409, 0, 1, "P44ARR02", 3, 601);
                } else {
                    CB->pos_perso(P_GUARDIAN4, 0, 0, 0, 2, "P44CAD", 3, 0);
                }
            }
        }
        /* 0x26FE */
        if (ST(V_EVENT_6) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
            /* 0x2722 */
            if (ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD) {
                /* 0x2734 */
                if (ST(L08_BONUS_ROUND) == 0) {
                    /* 0x2757 */
                    if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
                        CB->pos_perso(P_GUARDIAN5, 1256, -1176, 0x79C2, 1, "P45ARR03", 4, 1);
                    }
                    /* 0x279F */
                    if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -700) {
                        CB->pos_perso(P_GUARDIAN5, 362, -1270, 0xE697, 1, "P45ARR03", 4, 1);
                    }
                }
                /* 0x27D6 */
                if (ST(L08_BONUS_ROUND) == 1 || ST(L08_BONUS_ROUND) == 0xB || ST(L08_BONUS_ROUND) == 0x15) {
                    CB->pos_perso(P_GUARDIAN5, -119, -1317, 0xC283, 1, "P45ARR03", 4, 1);
                }
                /* 0x2831 */
                if (ST(L08_BONUS_ROUND) == 2) {
                    /* 0x2840 */
                    if (ST(ST_LEVEL) == 2 && ST(ST_VIE(P_GUARDIAN5)) < 2) {
                        CB->pos_perso(P_GUARDIAN5, 534, -1100, 0x5D7F, 1, "P45GAR03", 4, 1);
                    }
                    /* 0x2886 */
                    if (ST(ST_LEVEL) == 10 && ST(ST_VIE(P_GUARDIAN5)) < 2) {
                        CB->pos_perso(P_GUARDIAN5, 1381, -1418, 0xECAD, 1, "P45GAR03", 4, 1);
                    }
                }
            } else {
                CB->pos_perso(P_GUARDIAN5, 0, 0, 0, 2, "P45CAD", 4, 0);
            }
        }
        /* 0x28F0 */
        if (ST(V_EVENT_6) == 2 && (ST(ST_LEVEL) == 10 || ST(L08_ARENA_UNLOCKED) == 1)) {
            /* 0x2923 */
            if (ST(L08_ARENA_UNLOCKED) == 0) {
                ST(L08_ARENA_UNLOCKED) = 1;
            }
            /* 0x2944 */
            if (ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD) {
                /* 0x2956 */
                if (ST(V_GUARDIAN8_STAGE) == 0) {
                    CB->pos_perso(P_GUARDIAN7, 690, -1320, 0x70A7, 1, "P47ARR04", 6, 1);
                }
                /* 0x298D */
                if (ST(V_GUARDIAN8_STAGE) == 2) {
                    /* 0x299F */
                    if (ST(L08_G7_DEAD_FLAG) == 0) {
                        CB->pos_perso(P_GUARDIAN7, 734, -1438, 0xCA6D, 1, "P47ARR04", 6, 1);
                    } else {
                        CB->pos_perso(P_GUARDIAN7, 734, -1438, 0xCA6D, 1, "P47GAR05", 6, 1);
                    }
                }
            } else {
                CB->pos_perso(P_GUARDIAN7, 0, 0, 0, 2, "P47CAD", 6, 0);
            }
        }
        /* 0x2A21 */
        if (ST(V_EVENT_10) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
            /* 0x2A45 */
            if (ST(V_GUARDIAN4_STAGE) == 0) {
                CB->pos_perso(P_GUARDIAN3, 388, -1523, 0x8243, 1, "P43ATT01", 2, 1);
            }
            /* 0x2A7C */
            if (ST(V_GUARDIAN4_STAGE) == 1 || ST(V_GUARDIAN4_STAGE) == 5) {
                /* 0x2A9D */
                if (ST(ST_LEVEL) == 2) {
                    /* 0x2AAF */
                    if (ST(ST_VIE(P_GUARDIAN3)) < 2) {
                        CB->pos_perso(P_GUARDIAN3, 750, -1353, 0x365F, 1, "P43ARR03", 2, 1);
                    } else {
                        CB->pos_perso(P_GUARDIAN3, 0, 0, 0, 2, "P43CAD", 2, 1);
                    }
                }
                /* 0x2B04 */
                if (ST(ST_LEVEL) == 10) {
                    /* 0x2B16 */
                    if (ST(ST_VIE(P_GUARDIAN3)) < 2) {
                        CB->pos_perso(P_GUARDIAN3, 1432, -1180, 0xB73F, 1, "P43ARR03", 2, 1);
                    } else {
                        CB->pos_perso(P_GUARDIAN3, 0, 0, 0, 2, "P43CAD", 2, 1);
                    }
                }
            }
            /* 0x2B6E */
            if (ST(V_GUARDIAN4_STAGE) == 3) {
                CB->pos_perso(P_GUARDIAN3, 1211, -1311, 0xF168, 1, "P43ARR03", 2, 1);
            }
            /* 0x2BA5 */
            if (ST(V_GUARDIAN4_STAGE) == 4) {
                /* 0x2BB7 */
                if (ST(ST_VIE(P_GUARDIAN3)) < 2) {
                    CB->pos_perso(P_GUARDIAN3, 1007, -1293, 0x8000, 1, "P43ARR03", 2, 1);
                } else {
                    CB->pos_perso(P_GUARDIAN3, 0, 0, 0, 2, "P43CAD", 2, 1);
                }
            }
        }
        /* 0x2C0F */
        if (ST(V_EVENT_10) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
            /* 0x2C33 */
            if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
                /* 0x2C45 */
                if (ST(L08_BONUS_ROUND) == 0) {
                    /* 0x2C68 */
                    if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
                        CB->pos_perso(P_GUARDIAN6, 1211, -1311, 0xF168, 1, "P46ARR03", 5, 1);
                    }
                    /* 0x2CB0 */
                    if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -700) {
                        CB->pos_perso(P_GUARDIAN6, 412, -1177, 0x7D55, 1, "P46ARR03", 5, 1);
                    }
                }
                /* 0x2CE7 */
                if (ST(L08_BONUS_ROUND) == 1 || (ST(L08_BONUS_ROUND) == 0x15 && ST(L08_FLAG_BD0) != 2)) {
                    CB->pos_perso(P_GUARDIAN6, 1614, -1310, 0xF8C4, 1, "P46ARR03", 5, 1);
                }
                /* 0x2D42 */
                if (ST(L08_BONUS_ROUND) == 2) {
                    /* 0x2D51 */
                    if (ST(ST_LEVEL) == 2 && ST(ST_VIE(P_GUARDIAN6)) < 2) {
                        CB->pos_perso(P_GUARDIAN6, 750, -1353, 0x365F, 1, "P46GAR03", 5, 1);
                    }
                    /* 0x2D97 */
                    if (ST(ST_LEVEL) == 10 && ST(ST_VIE(P_GUARDIAN6)) < 2) {
                        CB->pos_perso(P_GUARDIAN6, 1432, -1180, 0xB73F, 1, "P46GAR03", 5, 1);
                    }
                }
                /* 0x2DE0 */
                if (ST(L08_BONUS_ROUND) == 0xB || (ST(L08_BONUS_ROUND) == 0x15 && ST(L08_FLAG_BD0) == 2)) {
                    CB->pos_perso(P_GUARDIAN6, 6, -1196, 0x5E26, 1, "P46ARR03", 5, 1);
                }
            } else {
                CB->pos_perso(P_GUARDIAN6, 0, 0, 0, 2, "P46CAD", 5, 0);
            }
        }
        /* 0x2E5C */
        if (ST(V_EVENT_10) == 2 && ST(L08_ARENA_UNLOCKED) != 0) {
            /* 0x2E80 */
            if (ST(ST_VIE(P_GUARDIAN8)) != VIE_DEAD) {
                /* 0x2E92 */
                if (ST(V_GUARDIAN8_STAGE) == 0) {
                    CB->pos_perso(P_GUARDIAN8, 603, -1443, 0xE71D, 1, "P48ARR04", 7, 1);
                }
            } else if (ST(V_GUARDIAN8_STAGE) == 0) {
                CB->pos_perso(P_GUARDIAN8, 0, 0, 0, 2, "P48CAD", 7, 0);
            }
        }
    }
    ST(L08_KALHI13_ZONE_STEP) = 0;
    ST(L08_FOLLOWER_MODE) = 0;
    ST(L08_KALHI13_CHAT_READY) = 0;
    ST(L08_FOLLOWER_NEAR) = 0;
    ST(L08_KALHI33_STEP) = 0;
    ST(L08_COMPANION_REUNION) = 0;
    /* 0x2F4A */
    if (ST(V_KALHI_PHASE) == 5 && ST(L08_COMPANION_ENTRY_DONE) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(L08_KALHI13_ZONE_STEP) = 3;
        /* 0x2F8C */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->pos_perso(P_KALHI13, -115, -1330, CB->divers(DV_DESC_POS, P_KALHI33, 2), 1, "P13ARR01", 0x1F, 1);
        }
        /* 0x2FD1 */
        if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD) {
            CB->pos_perso(P_KALHI13, CB->divers(DV_DESC_POS, P_KALHI13, 0), CB->divers(DV_DESC_POS, P_KALHI13, 1), CB->divers(DV_DESC_POS, P_KALHI13, 2), 2, "P13CAD", 0x1F, 0);
        }
        /* 0x3033 */
        if (ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
            CB->pos_perso(P_KALHI33, CB->divers(DV_DESC_POS, P_KALHI33, 0), CB->divers(DV_DESC_POS, P_KALHI33, 1), CB->divers(DV_DESC_POS, P_KALHI33, 2), 2, "P33CAD", 0x20, 0);
        }
        /* 0x3095 */
        if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
            CB->pos_perso(P_GUARDIAN10, 1581, -1323, 0xC19B, 1, "P50ARR03", 0x21, 1);
        } else {
            CB->pos_perso(P_GUARDIAN10, CB->divers(DV_DESC_POS, P_GUARDIAN10, 0), CB->divers(DV_DESC_POS, P_GUARDIAN10, 1), CB->divers(DV_DESC_POS, P_GUARDIAN10, 2), 2, "P50CAD", 0x21, 0);
        }
    }
    /* 0x3121 */
    if (ST(V_KALHI_PHASE) == 5 && ST(L08_COMPANION_ENTRY_DONE) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x3157 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            ST(ST_VIE(P_KALHI33)) = 0;
            ST(L08_FOLLOWER_MODE) = 1;
            ST(L08_KALHI13_CHAT_READY) = 1;
            /* 0x3190 */
            if (ST(L08_FROM_ROOM2) == 1) {
                CB->pos_perso(P_KALHI13, -983, -1175, 0xD6BC, 1, "P13ARR01", 0x1F, 1);
            }
            /* 0x31C7 */
            if (ST(L08_FROM_ROOM2) == 2) {
                CB->pos_perso(P_KALHI13, -983, -1530, 0xC350, 1, "P13ARR01", 0x1F, 1);
            }
            CB->pos_perso(P_GUARDIAN10, 1581, -1323, 0xC19B, 1, "P50ARR03", 0x21, 1);
        }
        /* 0x3223 */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            ST(ST_VIE(P_KALHI13)) = 0;
            CB->pos_perso(P_GUARDIAN10, 1544, -1171, 0x8097, 1, "P50ARR03", 0x21, 1);
            /* 0x3269 */
            if (ST(L08_FROM_ROOM2) == 1) {
                CB->pos_perso(P_KALHI33, -860, -1675, 0xB2AC, 1, "P31ARR01", 0x20, 1);
            }
            /* 0x32A0 */
            if (ST(L08_FROM_ROOM2) == 2) {
                CB->pos_perso(P_KALHI33, -762, -929, 0xB2AC, 1, "P31ARR01", 0x20, 1);
            }
            CB->ajoute_zone(23, -115, -1330, 90, 90, 0);
        }
    }
    /* 0x32F2 */
    if (ST(V_KALHI_PHASE) == 0x64 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->pos_perso(P_GUARDIAN10, 1544, -1171, 0x8097, 1, "P50ARR03", 0x21, 1);
        CB->pos_perso(P_KALHI33, -115, -1330, CB->divers(DV_DESC_POS, P_KALHI33, 2), 1, "P33ARR01", 0x20, 1);
    }
    /* 0x3371 */
    if (ST(V_KALHI_PHASE) == 0xC8 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->pos_perso(P_GUARDIAN10, 1544, -1171, 0x8097, 2, "P50CAD", 0x21, 0);
        CB->pos_perso(P_KALHI33, 1552, -1409, 0x296E, 1, "P33ARR01", 0x20, 1);
    }
    /* 0x33DF */
    if (ST(V_KALHI_PHASE) == 6 && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(L08_COMPANION_REUNION) = 1;
        /* 0x3412 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->pos_perso(P_KALHI13, -130, -1348, CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P13ARR03", 0x1F, 1);
        }
        /* 0x3457 */
        if (ST(ST_VIE(P_KALHI33)) == 1) {
            CB->pos_perso(P_KALHI33, -130, -1348, CB->divers(DV_DESC_POS, P_KALHI33, 2), 1, "P33ARR03", 0x20, 1);
        }
        /* 0x349C */
        if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD) {
            CB->pos_perso(P_KALHI13, CB->divers(DV_DESC_POS, P_KALHI13, 0), CB->divers(DV_DESC_POS, P_KALHI13, 1), CB->divers(DV_DESC_POS, P_KALHI13, 2), 2, "P13CAD", 0x1F, 0);
        }
        /* 0x34FE */
        if (ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
            CB->pos_perso(P_KALHI33, CB->divers(DV_DESC_POS, P_KALHI33, 0), CB->divers(DV_DESC_POS, P_KALHI33, 1), CB->divers(DV_DESC_POS, P_KALHI33, 2), 2, "P33CAD", 0x20, 0);
        }
        /* 0x3560 */
        if (ST(ST_VIE(P_GUARDIAN10)) != 0xA) {
            CB->pos_perso(P_GUARDIAN10, CB->divers(DV_DESC_POS, P_GUARDIAN10, 0), CB->divers(DV_DESC_POS, P_GUARDIAN10, 1), CB->divers(DV_DESC_POS, P_GUARDIAN10, 2), 1, "P50ARR03", 0x21, 0);
        } else {
            CB->pos_perso(P_GUARDIAN10, CB->divers(DV_DESC_POS, P_GUARDIAN10, 0), CB->divers(DV_DESC_POS, P_GUARDIAN10, 1), CB->divers(DV_DESC_POS, P_GUARDIAN10, 2), 2, "P50CAD", 0x21, 0);
        }
    }
    /* 0x3617 */
    if (ST(V_EVENT_6) == 0x63 && ST(V_EVENT_10) == 0x63 && ST(L08_G10_STATE) != 0 && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x3683 */
        if (ST(L08_G10_STATE) == 1) {
            CB->pos_perso(P_GUARDIAN10, 0, 0, 0, 2, "P50CAD", 0x21, 0);
        }
        /* 0x36B1 */
        if (ST(L08_G10_STATE) == 2) {
            CB->pos_perso(P_GUARDIAN10, CB->divers(DV_DESC_POS, P_GUARDIAN10, 0), CB->divers(DV_DESC_POS, P_GUARDIAN10, 1), CB->divers(DV_DESC_POS, P_GUARDIAN10, 2), 1, "P50ARR03", 0x21, 0);
        }
    }
    /* 0x3713 */
    if (ST(V_EVENT_6) != 2 && ST(V_EVENT_10) != 2 && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x3749 */
        if (ST(L08_G10_STATE) == 0x14 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
            /* 0x376A */
            if (ST(ST_LEVEL) == 10) {
                CB->pos_perso(P_GUARDIAN10, 1381, -1418, 0xECAD, 1, "P50GAR03", 0x21, 1);
            }
            /* 0x379E */
            if (ST(ST_LEVEL) == 2) {
                CB->pos_perso(P_GUARDIAN10, 534, -1100, 0x5D7F, 1, "P50GAR03", 0x21, 0);
            }
        }
        /* 0x37D5 */
        if (ST(L08_G10_STATE) == 0x1E) {
            CB->pos_perso(P_GUARDIAN10, 0, 0, 0, 2, "P50CAD", 0x21, 0);
        }
    }
    /* 0x3803 */
    if (ST(V_DELIA_PHASE) != 0 && ST(V_DELIA_PHASE) != 0xB && ST(L08_DELIA_ROUTE) == 0) {
        /* 0x3839 */
        if (ST(V_DELIA_PHASE) == 6) {
            ST(L08_G7_DEAD_FLAG) = 0x63;
            ST(V_EVENT_6) = 0x63;
            ST(V_EVENT_10) = 0x63;
            ST(L08_FOLLOWER_MODE) = 1;
            ST(L08_ARENA_LOCKED) = 1;
            /* 0x3893 */
            if (ST(L08_FROM_ROOM10) == 1) {
                CB->pos_perso(P_DELIA, 2394, -1508, 0x6A3E, 1, "P10ARR02", 0x1E, 0);
            }
            /* 0x38CA */
            if (ST(L08_FROM_ROOM10) == 2) {
                CB->pos_perso(P_DELIA, 2394, -1253, 0x1644, 1, "P10ARR02", 0x1E, 0);
            }
            CB->pos_perso(34, 1073, -1490, 0x7D61, 1, "P34ATT01", 0x22, 0);
        }
        /* 0x3926 */
        if (ST(V_DELIA_PHASE) == 7) {
            /* 0x3938 */
            if (ST(ST_VIE(34)) == VIE_DEAD) {
                /* 0x394A */
                if (ST(ST_VIE(P_DELIA)) == 1) {
                    ST(L08_FOLLOWER_MODE) = 1;
                    CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR02", 0x1E, 0);
                }
                CB->pos_perso(34, 0, 0, 0, 2, "P34CAD", -1, 0);
            } else {
                CB->pos_perso(P_DELIA, 1310, -1243, 0x9D27, 2, "P10cad", 0x1E, 0);
                CB->pos_perso(34, 1073, -1490, 0x7D61, 1, "P34GAR01", 0x22, 0);
            }
        }
        /* 0x3A23 */
        if ((ST(V_DELIA_PHASE) == 0x14 || ST(V_DELIA_PHASE) == 0x63) && ST(V_DELIA_KILLED) == 8) {
            /* 0x3A59 */
            if (ST(ST_VIE(34)) != VIE_DEAD) {
                /* 0x3A6B */
                if (ST(V_DELIA_KILLED) == 8) {
                    CB->pos_perso(P_DELIA, 1310, -1243, 0x9D27, 2, "P10cad", 0x1E, 0);
                }
                /* 0x3AA2 */
                if (ST(L08_ARENA_LOCKED) == 1) {
                    CB->pos_perso(34, 1227, -1360, 0xE4F5, 1, "P34GAR01", 0x22, 0);
                }
                /* 0x3AD9 */
                if (ST(L08_ARENA_LOCKED) == 0) {
                    CB->pos_perso(34, 1073, -1490, 0x7D61, 1, "P34GAR01", 0x22, 0);
                    ST(L08_ARENA_LOCKED) = 1;
                }
            }
            /* 0x3B1F */
            if (ST(ST_VIE(34)) == VIE_DEAD) {
                /* 0x3B31 */
                if (ST(V_DELIA_KILLED) == 8) {
                    CB->pos_perso(P_DELIA, 1310, -1243, 0x9D27, 2, "P10cad", 0x1E, 0);
                }
                CB->pos_perso(34, 0, 0, 0, 2, "P34CAD", -1, 0);
            }
        }
    }
    ST(L08_ARENA_ROUND_CMD) = 0;
    ST(L08_PATROL_PICK) = 0;
    ST(L08_G6_PATROL_STEP) = 0;
    ST(L08_G7_STEP) = 0;
    ST(V_EVENT15_STEP) = 0;
    ST(L08_KALHI33_CHAT_READY) = 0;
    ST(L08_KALHI33_POS_SAVED) = 0;
    ST(L08_DELIA_TRIG8_DONE) = 0;
    ST(L08_G8_STATE) = 0;
    ST(L08_G4_STANCE) = 0;
    ST(L08_UNUSED_D4) = 0;
    ST(L08_UNUSED_D8) = 0;
    ST(L08_G5_TALK_DONE) = 0;
    ST(L08_G6_TALK_DONE) = 0;
    ST(L08_KALHI33_STUCK_CHECK) = 0;
    ST(L08_CREATURE34_ENGAGED) = 0;
    ST(L08_KALHI33_RUN_STEP) = 0;
    ST(L08_KALHI13_GAR_STEP) = 0;
    ST(L08_UNUSED_100) = 0;
    ST(L08_CAM_ANIM_ARMED) = 0;
    ST(V_WEAPON_WARNED) = 0;
    ST(L08_KALHI_FIGHT_TALK) = 0;
    ST(L08_KALHI13_D220_PENDING) = 0;
    ST(V_ENGAGED_PERSONA) = 0;
    ST(ST_SCRIPT_BUSY) = 0;
    ST(V_LAST_WEAPON) = 0;
    ST(L08_LAST_WEAPON_SEEN) = 0;
    /* 0x3D01 */
    if (ST(L08_FLAG_E24) == 2) {
        ST(L08_FLAG_E24) = 3;
    }
    /* 0x3D22 */
    if (ST(V_KALHI_PHASE) == 5 && (ST(V_EVENT_6) == 0 || ST(V_EVENT_6) == 1) && (ST(V_EVENT_10) == 0 || ST(V_EVENT_10) == 1)) {
        ST(V_EVENT_6) = 0x63;
        ST(V_EVENT_10) = 0x63;
    }
    ST(ST_START_POINT(8)) = 0;
}

/* trigger 02 - sub_1000BCA5 */
static void trig02(void) {
    /* 0xBCB4 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN3)) {
        CB->select_pile(1, 1);
        /* 0xBD3A */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xBDAC */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xBDD4 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xBDFC */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xBE64 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 8)) {
        l08_trig08();
    }
    /* 0xBE89 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 9)) {
        trig09();
    }
    /* 0xBEAE */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 10)) {
        trig10();
    }
    /* 0xBED3 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 11)) {
        trig11();
    }
    /* 0xBEEA */
    if (ST(ST_HERO) == P_ARKHAN1) {
        /* 0xBEFC */
        if (ST(L08_G3_PATROL_ARMED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(L08_G8_STATE) == 0 && (ST(L08_G3_PATROL_STEP) == 0 || ST(L08_G3_PATROL_STEP) == 1)) {
            ST(L08_G3_PATROL_ARMED) = 1;
            CB->select_pile(1, 2);
            CB->select_pile(1, 1);
            CB->evenement(1, -1);
            CB->bloque(BLOQUE_ALL);
            /* 0xBFAF */
            if (ST(L08_G3_PATROL_STEP) == 1) {
                CB->animation("SP43ATT02");
                CB->animation("P43ATT09");
                CB->animation("P43ARR03");
                CB->joindre(P_GUARDIAN3, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP43ARR03");
                CB->joindre_perm(P_GUARDIAN3, ST(ST_HERO));
            } else {
                CB->stop_joindre(-P_GUARDIAN3);
                CB->joindre(P_GUARDIAN3, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP43ARR03");
                CB->joindre_perm(P_GUARDIAN3, ST(ST_HERO));
            }
            CB->animation("sar_10");
            CB->modify_var(&ST(L08_G3_STANCE_TALK), 1);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0xC0A2 */
        if (ST(ST_COMBAT_STANCE) == 1 && ST(L08_G8_STATE) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_GUARDIAN3) && ST(L08_G3_STANCE_TALK) == 1) {
            ST(L08_G8_STATE) = 1;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_GUARDIAN3);
            lieu_action_register("L08D152", P_GUARDIAN3, "SP43GAR26", "SP43GAR03");
            CB->joindre_perm(P_GUARDIAN3, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0xC182 */
        if (ST(ST_COMBAT_STANCE) == 0 && ST(L08_G8_STATE) == 1 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_GUARDIAN3) && ST(L08_G3_STANCE_TALK) == 1) {
            ST(L08_G8_STATE) = 0;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_GUARDIAN3);
            CB->animation("SP43GAR27");
            CB->dialogue("L08D235", P_GUARDIAN3);
            CB->animation("SP43ARR03");
            CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
    } else if (ST(ST_HERO) == P_ARKHAN2) {
        /* 0xC289 */
        if (ST(L08_G3_PATROL_ARMED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(L08_G8_STATE) == 0) {
            ST(L08_G3_PATROL_ARMED) = 1;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_GUARDIAN3);
            CB->joindre(P_GUARDIAN3, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP43ARR03");
            CB->joindre_perm(P_GUARDIAN3, ST(ST_HERO));
            CB->animation("sar_10");
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0xC35E */
        if (ST(ST_COMBAT_STANCE) == 1 && ST(L08_G8_STATE) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_GUARDIAN3)) {
            ST(L08_G8_STATE) = 1;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_GUARDIAN3);
            lieu_action_register("L08D152", P_GUARDIAN3, "SP43GAR26", "SP43GAR03");
            CB->joindre_perm(P_GUARDIAN3, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0xC42C */
        if (ST(ST_COMBAT_STANCE) == 0 && ST(L08_G8_STATE) == 1 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_GUARDIAN3)) {
            ST(L08_G8_STATE) = 0;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_GUARDIAN3);
            CB->animation("SP43GAR27");
            CB->dialogue("L08D235", P_GUARDIAN3);
            CB->animation("SP43ARR03");
            CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
    }
    /* 0xC507 */
    if (ST(ST_ACTION_KEY) == 1 && CB->direction_zone(21) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 21) && !CB->direction_perso(P_GUARDIAN3)) {
        trig21();
        return;
    }
    /* 0xC566 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && (ST(L08_G3_STANCE_TALK) == 1 || ST(ST_HERO) > 1) && CB->direction_perso(P_GUARDIAN3)) {
        CB->bloque(BLOQUE_ALL);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_GUARDIAN3);
        CB->select_pile(1, 1);
        CB->joindre(-P_GUARDIAN3, 0, 0, 0, ST(ST_HERO));
        CB->select_pile(0, 1);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xC65C */
        if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
            /* 0xC6B1 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0xC6C3 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0xC6D5 */
                    if (ST(L08_G3_TALK_STAGE) == 0) {
                        ST(L08_G3_TALK_STAGE) = 1;
                        lieu_action_register("L08D03", P_ARKHAN1, "SP01ATT74", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D04", P_GUARDIAN3, "SP43ATT70", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xC76F */
                    if (ST(L08_G3_TALK_STAGE) == 1) {
                        ST(L08_G3_TALK_STAGE) = 2;
                        lieu_action_register("L08D05", P_ARKHAN1, "SG01DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D06A", P_GUARDIAN3, "SP43DIA56", "SP43ARR03_10");
                        lieu_action_register("L08D06B", P_GUARDIAN3, "SP43DIA53", "SP43ARR03_10");
                        lieu_action_register("L08D06C", P_GUARDIAN3, "SP43DIA58", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xC83B */
                    if (ST(L08_G3_TALK_STAGE) == 2) {
                        ST(L08_G3_TALK_STAGE) = 3;
                        lieu_action_register("L08D07", P_ARKHAN1, "SG01DIA07", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D08", P_GUARDIAN3, "SP43ATT51", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xC8D5 */
                    if (ST(L08_G3_TALK_STAGE) == 3) {
                        ST(L08_G3_TALK_STAGE) = 4;
                        lieu_action_register("L08D17", P_ARKHAN1, "SG01DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D18", P_GUARDIAN3, "SP43DIA59", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xC96F */
                    if (ST(L08_G3_TALK_STAGE) == 4) {
                        ST(L08_G3_TALK_STAGE) = 5;
                        lieu_action_register("L08D19", P_ARKHAN1, "SG01DIA06", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D20A", P_GUARDIAN3, "SP43DIA50", "SP43ARR03_10");
                        lieu_action_register("L08D20B", P_GUARDIAN3, "SP43DIA57", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xCA22 */
                    if (ST(L08_G3_TALK_STAGE) >= 5) {
                        ST(L08_G3_TALK_STAGE)++;
                        lieu_action_register("L08D236", P_ARKHAN1, "SG01DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        /* 0xCA60 */
                        if (ST(L08_G3_TALK_STAGE) == 6) {
                            lieu_action_register("L08D237", P_GUARDIAN3, "SP43DIA58", "SP43ARR03_10");
                        } else {
                            lieu_action_register("L08D238", P_GUARDIAN3, "SP43DIA60", "SP43ARR03_10");
                        }
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xCAE8 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0xCAFA */
                    if (ST(L08_G3_TALK_STAGE) == 0) {
                        ST(L08_G3_TALK_STAGE) = 1;
                        lieu_action_register("L08D10", P_ARKHAN1, "SP01ATT75", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D04", P_GUARDIAN3, "SP43ATT70", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xCB94 */
                    if (ST(L08_G3_TALK_STAGE) == 1) {
                        ST(L08_G3_TALK_STAGE) = 2;
                        lieu_action_register("L08D11", P_ARKHAN1, "SG01DIA03", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D12", P_GUARDIAN3, "SP43DIA50", "SP43ARR03_10");
                        lieu_action_register("L08D06B", P_GUARDIAN3, "SP43DIA59", "SP43ARR03_10");
                        lieu_action_register("L08D06C", P_GUARDIAN3, "SP43DIA58", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xCC60 */
                    if (ST(L08_G3_TALK_STAGE) == 2) {
                        ST(L08_G3_TALK_STAGE) = 3;
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D13A", P_ARKHAN1, "SG01DIA02", NULL);
                        lieu_action_register("L08D13B", P_ARKHAN1, "SG01DIA07", NULL);
                        lieu_action_register("L08D13C", P_ARKHAN1, "SG01DIA06", NULL);
                        lieu_action_register("L08D14A", P_GUARDIAN3, "SP43DIA54", "SP43ARR03_10");
                        lieu_action_register("L08D14B", P_GUARDIAN3, "SP43DIA63", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xCD3F */
                    if (ST(L08_G3_TALK_STAGE) == 3) {
                        ST(L08_G3_TALK_STAGE) = 4;
                        lieu_action_register("L08D23A", P_ARKHAN1, "SG01DIA08", NULL);
                        lieu_action_register("L08D23B", P_ARKHAN1, "SG01DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D24", P_GUARDIAN3, "SP43DIA56", "SP43ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xCDEF */
                    if (ST(L08_G3_TALK_STAGE) >= 4) {
                        ST(L08_G3_TALK_STAGE)++;
                        lieu_action_register("L08D239A", P_ARKHAN1, "SG01DIA08", NULL);
                        lieu_action_register("L08D239B", P_ARKHAN1, "SG01DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        /* 0xCE43 */
                        if (ST(L08_G3_TALK_STAGE) == 5) {
                            lieu_action_register("L08D240", P_GUARDIAN3, "SP43DIA59", "SP43ARR03_10");
                        } else {
                            lieu_action_register("L08D238", P_GUARDIAN3, "SP43DIA64", "SP43ARR03_10");
                        }
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
            /* 0xCECB */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0xCEDD */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0xCEEF */
                    if (ST(L08_G3_TALK_STAGE) == 0) {
                        ST(L08_G3_TALK_STAGE) = 1;
                        lieu_action_register("L08D126", P_ARKHAN2, "SG00ATT01", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D127", P_GUARDIAN3, "SP43DIA53", "SP43ARR03");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xCF89 */
                    if (ST(L08_G3_TALK_STAGE) == 1) {
                        ST(L08_G3_TALK_STAGE) = 2;
                        lieu_action_register("L08D100B", P_ARKHAN1, "SG01DIA03", NULL);
                        lieu_action_register("L08D100C", P_ARKHAN1, "SG01DIA03", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D129A", P_GUARDIAN3, "SP43DIA57", "SP43ARR03");
                        lieu_action_register("L08D129B", P_GUARDIAN3, "SP43DIA51", "SP43ARR03");
                        lieu_action_register("L08D129C", P_GUARDIAN3, "SP43DIA53", "SP43ARR03");
                        lieu_action_register("L08D129D", P_GUARDIAN3, "SP43DIA58", "SP43ARR03");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xD084 */
                    if (ST(L08_G3_TALK_STAGE) == 2) {
                        lieu_action_register("L08D130A", P_ARKHAN2, "SG02DIA07", NULL);
                        lieu_action_register("L08D130B", P_ARKHAN2, "SG02DIA06", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D131", P_GUARDIAN3, "SP43DIA62", "SP43ARR03");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xD125 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0xD137 */
                    if (ST(L08_G3_TALK_STAGE) == 0) {
                        ST(L08_G3_TALK_STAGE) = 1;
                        lieu_action_register("L08D132", P_ARKHAN2, "SG02DIA01", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D127", P_GUARDIAN3, "SP43ATT70", "SP43ARR03");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xD1D1 */
                    if (ST(L08_G3_TALK_STAGE) == 1) {
                        ST(L08_G3_TALK_STAGE) = 2;
                        lieu_action_register("L08D133A", P_ARKHAN2, "SG02DIA03", NULL);
                        lieu_action_register("L08D133B", P_ARKHAN2, "SG02DIA03", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D134", P_GUARDIAN3, "SP43DIA56", "SP43ARR03");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xD281 */
                    if (ST(L08_G3_TALK_STAGE) >= 2) {
                        lieu_action_register("L08D135", P_ARKHAN2, "SG02DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN3);
                        lieu_action_register("L08D131", P_GUARDIAN3, "SP43DIA54", "SP43ARR03");
                        CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
        }
        /* 0xD30C */
        if (ST(ST_PLAYER_WEAPON) == 0x18 && ST(ST_HERO) == P_ARKHAN1) {
            /* 0xD330 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                /* 0xD342 */
                if (ST(L08_TRIG2_TALK_DONE) == 0) {
                    /* 0xD354 */
                    if (ST(L08_G3_TALK_STAGE) == 0) {
                        ST(L08_G3_TALK_STAGE) = 1;
                    }
                    ST(L08_TRIG2_TALK_DONE) = 1;
                    lieu_action_register("L08D25", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN3);
                    lieu_action_register("L08D26", P_GUARDIAN3, "SP43ATT61", "SP43ARR03_10");
                    CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xD3FD */
                if (ST(L08_TRIG2_TALK_DONE) == 1) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    lieu_action_register("L08D27", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN3);
                    lieu_action_register("L08D28", P_GUARDIAN3, "SP43DIA50", "SP43ARR03_10");
                    CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0xD49C */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0xD4AE */
                if (ST(L08_TRIG2_TALK_DONE) == 0) {
                    /* 0xD4C0 */
                    if (ST(L08_G3_TALK_STAGE) == 0) {
                        ST(L08_G3_TALK_STAGE) = 1;
                    }
                    ST(L08_TRIG2_TALK_DONE) = 1;
                    lieu_action_register("L08D29", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN3);
                    lieu_action_register("L08D30", P_GUARDIAN3, "SP43ATT51", "SP43ARR03_10");
                    CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xD569 */
                if (ST(L08_TRIG2_TALK_DONE) == 1) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    lieu_action_register("L08D31", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN3);
                    lieu_action_register("L08D32", P_GUARDIAN3, "SP43DIA59", "SP43ARR03_10");
                    CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0xD608 */
        if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2)) {
            ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
            /* 0xD645 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0xD657 */
                if (ST(L08_G3_TALK_STAGE) == 0) {
                    ST(L08_G3_TALK_STAGE) = 1;
                }
                lieu_action_register("L08D33", P_ARKHAN1, "SG01PRE60", NULL);
                CB->stop_joindre(-P_GUARDIAN3);
                lieu_action_register("L08D34", P_GUARDIAN3, "SP43ATT61", "SP43ARR03_10");
                CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                CB->animation("Sar_150");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xD6F1 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("LCOD18", P_ARKHAN1, "SG01PRE60", NULL);
                CB->stop_joindre(-P_GUARDIAN3);
                lieu_action_register("L08D325", P_GUARDIAN3, "SP43ATT61", "SP43ARR03_10");
                CB->joindre_perm(-P_GUARDIAN3, ST(ST_HERO));
                CB->animation("Sar_150");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0xD77C */
    if (ST(ST_HERO) == P_ARKHAN1) {
        /* 0xD78E */
        if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(L08_G3_STANCE_TALK) == 1 && CB->direction_perso(P_GUARDIAN3)) {
            ST(V_CRI_USED) = 1;
            /* 0xD813 */
            if (ST(L08_ARENA_ROUND_CMD) != 0x63) {
                ST(L08_ARENA_ROUND_CMD) = 0x2B;
            }
            CB->bloque(BLOQUE_ALL);
            CB->evenement(0, 1000);
            return;
        }
    } else if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN3)) {
        ST(V_CRI_USED) = 1;
        /* 0xD8C6 */
        if (ST(L08_ARENA_ROUND_CMD) != 0x63) {
            ST(L08_ARENA_ROUND_CMD) = 0x2B;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0, 1000);
        return;
    }
    /* 0xD8FE */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN3)) {
        /* 0xD962 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("L08D37", P_ARKHAN1, "SP01ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xD9BF */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD20", P_ARKHAN2, "SP02ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xDA1C */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xDA76 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0;
        return;
    }
    /* 0xDAE7 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN3);
        return;
    }
    /* 0xDB36 */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN3)) {
        /* 0xDB8B */
        if (ST(ST_HERO) == P_ARKHAN1) {
            ST(V_PLAYER_STRIKE) = -1;
            CB->select_pile(1, 2);
            ST(V_EVENT_1) = 0;
            CB->evenement(1, -1);
            CB->stop_joindre(-P_GUARDIAN3);
            CB->dialogue("L08D01", P_GUARDIAN3);
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN3;
            CB->combat(P_GUARDIAN3, 0);
            /* 0xDC0C */
            if (ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN4, 0)) {
                ST(V_ENGAGED_PERSONA) = 0x10F8;
                CB->combat(P_GUARDIAN4, 0);
            } else {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN3;
            }
            CB->select_pile(0, 1);
            return;
            return;
        }
        ST(V_PLAYER_STRIKE) = -1;
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_GUARDIAN3);
        lieu_action_register("L08D232", P_GUARDIAN3, "SP43GAR03", "SP43GAR03_10");
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN3;
        CB->combat(P_GUARDIAN3, 0);
        /* 0xDCD2 */
        if (ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN4, 0)) {
            ST(V_ENGAGED_PERSONA) = 0x10F8;
            CB->combat(P_GUARDIAN4, 0);
        } else {
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN3;
        }
        CB->select_pile(0, 1);
        return;
    }
}

/* trigger 03 - sub_1000DD33 */
static void trig03(void) {
    /* 0xDD42 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN4)) {
        CB->select_pile(1, 1);
        /* 0xDDC8 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xDE3A */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xDE62 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xDE8A */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xDEF2 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 8)) {
        l08_trig08();
    }
    /* 0xDF17 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 9)) {
        trig09();
    }
    /* 0xDF3C */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 10)) {
        trig10();
    }
    /* 0xDF61 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 11)) {
        trig11();
    }
    /* 0xDF78 */
    if (ST(ST_HERO) == P_ARKHAN1) {
        /* 0xDF8A */
        if (ST(L08_G4_PATROL_STATE) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && ST(ST_COMBAT_STANCE) == 0 && ST(L08_G4_STANCE) == 0 && CB->direction_perso(P_GUARDIAN4)) {
            ST(L08_G4_PATROL_STATE) = 1;
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            /* 0xE01B */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->stop_joindre(-P_GUARDIAN4);
                CB->joindre(P_GUARDIAN4, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP44ARR02");
                CB->joindre_perm(P_GUARDIAN4, ST(ST_HERO));
            }
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
    } else if ((ST(L08_G4_PATROL_STATE) == 0 || ST(L08_G4_PATROL_STATE) == 0x14) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && ST(ST_COMBAT_STANCE) == 0 && ST(L08_G4_STANCE) == 0) {
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xE116 */
        if (ST(L08_G4_PATROL_STATE) == 0) {
            CB->stop_joindre(-P_GUARDIAN4);
            CB->joindre(P_GUARDIAN4, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP44ARR02");
            CB->joindre_perm(P_GUARDIAN4, ST(ST_HERO));
        }
        /* 0xE171 */
        if (ST(L08_G4_PATROL_STATE) == 0x14) {
            /* 0xE183 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->divers_empile(DV_RUN, P_GUARDIAN4, 0);
                CB->stop_joindre(-P_GUARDIAN4);
                CB->stop_joindre(P_GUARDIAN4);
                CB->stop_joindre(P_GUARDIAN4);
                lieu_action_register("L08D221", P_GUARDIAN4, "SP44GAR02", "SP44GAR27");
                CB->animation("P44ARR02");
                CB->joindre_perm(P_GUARDIAN4, ST(ST_HERO));
            }
            /* 0xE202 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->stop_joindre(-P_GUARDIAN4);
                CB->dialogue("L08D222", P_GUARDIAN4);
                ST(V_ENGAGED_PERSONA) = 0x10F8;
                CB->combat(P_GUARDIAN4, 0);
                /* 0xE248 */
                if (ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD) {
                    ST(V_ENGAGED_PERSONA) = 0x10F8;
                    CB->combat(P_GUARDIAN3, 0);
                } else {
                    ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
                }
            }
        }
        CB->bloque(BLOQUE_NONE);
        ST(L08_G4_PATROL_STATE) = 1;
        CB->select_pile(0, 1);
        return;
    }
    /* 0xE2AA */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(L08_G4_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_GUARDIAN4)) {
        ST(L08_G4_STANCE) = 1;
        ST(L08_G4_PATROL_STATE) = 1;
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN4);
        CB->joindre(P_GUARDIAN4, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L08D151", P_GUARDIAN4, "SP44GAR26", "SP44GAR02");
        CB->joindre_perm(P_GUARDIAN4, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xE3A4 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L08_G4_STANCE) == 1 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_GUARDIAN4)) {
        ST(L08_G4_STANCE) = 0;
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN4);
        CB->animation("SP44GAR27");
        CB->animation("P44ARR02");
        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xE46F */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN4)) {
        CB->bloque(BLOQUE_ALL);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_GUARDIAN4);
        CB->select_pile(2, 1);
        CB->joindre(-P_GUARDIAN4, 0, 0, 0, ST(ST_HERO));
        CB->select_pile(0, 1);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xE541 */
        if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(L08_LAST_WEAPON_SEEN))) {
            /* 0xE599 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0xE5AB */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0xE5BD */
                    if (ST(L08_G3_FIGHT_STAGE) == 0) {
                        ST(L08_G3_FIGHT_STAGE) = 1;
                        lieu_action_register("L08D39", P_ARKHAN1, "SP01ATT74", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D40", P_GUARDIAN4, "SP44ATT70", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xE657 */
                    if (ST(L08_G3_FIGHT_STAGE) == 1) {
                        ST(L08_G3_FIGHT_STAGE) = 2;
                        lieu_action_register("L08D05", P_ARKHAN1, "SG01DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D41", P_GUARDIAN4, "SP44DIA57", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xE6F1 */
                    if (ST(L08_G3_FIGHT_STAGE) == 2) {
                        ST(L08_G3_FIGHT_STAGE) = 3;
                        lieu_action_register("L08D48", P_ARKHAN1, "SG01DIA07", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        CB->dialogue("L08D49", P_GUARDIAN4);
                        CB->joindre(-P_GUARDIAN4, 0, 0, 0xA7F8, -1);
                        lieu_action_register("L08D50", P_GUARDIAN4, "SP44ATT50", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xE7B1 */
                    if (ST(L08_G3_FIGHT_STAGE) == 3) {
                        ST(L08_G3_FIGHT_STAGE) = 4;
                        lieu_action_register("L08D51", P_ARKHAN1, "SG01DIA06", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D52A", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                        lieu_action_register("L08D52B", P_GUARDIAN4, "SP44DIA54", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xE864 */
                    if (ST(L08_G3_FIGHT_STAGE) >= 4) {
                        lieu_action_register("L08D42", P_ARKHAN1, "SG01DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D43", P_GUARDIAN4, "SP44ATT60", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xE8EF */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0xE901 */
                    if (ST(L08_G3_FIGHT_STAGE) == 0) {
                        ST(L08_G3_FIGHT_STAGE) = 1;
                        lieu_action_register("L08D53", P_ARKHAN1, "SP01ATT75", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D54", P_GUARDIAN4, "SP44DIA59", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xE99B */
                    if (ST(L08_G3_FIGHT_STAGE) == 1) {
                        ST(L08_G3_FIGHT_STAGE) = 2;
                        lieu_action_register("L08D45", P_ARKHAN1, "SG00DIA01", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D41", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xEA35 */
                    if (ST(L08_G3_FIGHT_STAGE) == 2) {
                        ST(L08_G3_FIGHT_STAGE) = 3;
                        lieu_action_register("L08D55", P_ARKHAN1, "SG01DIA07", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        CB->dialogue("L08D49", P_GUARDIAN4);
                        CB->joindre(-P_GUARDIAN4, 0, 0, 0xDEA8, -1);
                        lieu_action_register("L08D50", P_GUARDIAN4, "SP44ATT50", "SP44ARR02_500");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xEAF5 */
                    if (ST(L08_G3_FIGHT_STAGE) == 3) {
                        ST(L08_G3_FIGHT_STAGE) = 4;
                        lieu_action_register("L08D46", P_ARKHAN1, "SG01DIA03", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D47A", P_GUARDIAN4, "SP44DIA55", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xEB8F */
                    if (ST(L08_G3_FIGHT_STAGE) == 4) {
                        lieu_action_register("L08D56", P_ARKHAN1, "SG01DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D57", P_GUARDIAN4, "SP44ATT51", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
            /* 0xEC1A */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0xEC2C */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0xEC3E */
                    if (ST(L08_G3_FIGHT_STAGE) == 0) {
                        ST(L08_G3_FIGHT_STAGE) = 1;
                        lieu_action_register("L08D98A", P_ARKHAN2, "SG02DIA02", NULL);
                        lieu_action_register("L08D98B", P_ARKHAN2, "SG02DIA06", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D99A", P_GUARDIAN4, "SP44DIA54", "SP44ARR02_10");
                        lieu_action_register("L08D99B", P_GUARDIAN4, "SP44DIA53", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xED07 */
                    if (ST(L08_G3_FIGHT_STAGE) == 1) {
                        ST(L08_G3_FIGHT_STAGE) = 2;
                        lieu_action_register("L08D100A", P_ARKHAN1, "SG01DIA02", NULL);
                        lieu_action_register("L08D100B", P_ARKHAN1, "SG01DIA03", NULL);
                        lieu_action_register("L08D100C", P_ARKHAN1, "SG01DIA06", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D101A", P_GUARDIAN4, "SP44DIA59", "SP44ARR02_10");
                        lieu_action_register("L08D101B", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                        lieu_action_register("L08D101C", P_GUARDIAN4, "SP44DIA60", "SP44ARR02_10");
                        lieu_action_register("L08D101D", P_GUARDIAN4, "SP44DIA65", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L08_G3_FIGHT_STAGE), 0xA);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xEE2F */
                    if (ST(L08_G3_FIGHT_STAGE) == 2) {
                        lieu_action_register("L08D102A", P_ARKHAN1, "SG01DIA07", NULL);
                        lieu_action_register("L08D102B", P_ARKHAN1, "SG01DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        ST(V_DLG_RANDOM) = CB->random(2);
                        /* 0xEE8F */
                        if (ST(V_DLG_RANDOM) == 0) {
                            lieu_action_register("L08D103A", P_GUARDIAN4, "SP44DIA62", "SP44ARR02_10");
                        }
                        /* 0xEEBA */
                        if (ST(V_DLG_RANDOM) == 1) {
                            lieu_action_register("L08D103Z", P_GUARDIAN4, "SP44DIA62", "SP44ARR02_10");
                        }
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xEF24 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0xEF36 */
                    if (ST(L08_G3_FIGHT_STAGE) == 0) {
                        ST(L08_G3_FIGHT_STAGE) = 1;
                        lieu_action_register("L08D104", P_ARKHAN1, "SP01ATT75", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D105A", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                        lieu_action_register("L08D105B", P_GUARDIAN4, "SP44DIA59", "SP44ARR02_10");
                        lieu_action_register("L08D105C", P_GUARDIAN4, "SP44DIA53", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xF002 */
                    if (ST(L08_G3_FIGHT_STAGE) == 1) {
                        ST(L08_G3_FIGHT_STAGE) = 2;
                        lieu_action_register("L08D106A", P_ARKHAN1, "SG01DIA03", NULL);
                        lieu_action_register("L08D106B", P_ARKHAN1, "SG01DIA04", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D107A", P_GUARDIAN4, "SP44DIA62", "SP44ARR02_10");
                        lieu_action_register("L08D107B", P_GUARDIAN4, "SP44DIA60", "SP44ARR02_10");
                        CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xF0CB */
                    if (ST(L08_G3_FIGHT_STAGE) >= 2) {
                        lieu_action_register("L08D108", P_ARKHAN1, "SG01DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN4);
                        lieu_action_register("L08D109", P_GUARDIAN4, "SP44DIA54", "SP44ARR02_10");
                        CB->combat(P_GUARDIAN4, 0);
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        /* 0xF140 */
                        if (ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD) {
                            ST(V_ENGAGED_PERSONA) = 0x10F8;
                            CB->combat(P_GUARDIAN3, 0);
                        } else {
                            ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
                        }
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
        }
        /* 0xF189 */
        if (ST(ST_PLAYER_WEAPON) == 0x18 && ST(ST_HERO) == P_ARKHAN1) {
            /* 0xF1AD */
            if (ST(ST_SPECIAL_MODE) == 1) {
                /* 0xF1BF */
                if (ST(L08_TRIG3_TALK_DONE) == 0) {
                    /* 0xF1D1 */
                    if (ST(L08_G3_FIGHT_STAGE) == 0) {
                        ST(L08_G3_FIGHT_STAGE) = 1;
                    }
                    ST(L08_TRIG3_TALK_DONE) = 1;
                    lieu_action_register("L08D58", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN4);
                    lieu_action_register("L08D59", P_GUARDIAN4, "SP44DIA57", "SP44ARR02_10");
                    CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xF27A */
                if (ST(L08_TRIG3_TALK_DONE) == 1) {
                    ST(L08_LAST_WEAPON_SEEN) = ST(ST_PLAYER_WEAPON);
                    lieu_action_register("L08D60", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN4);
                    lieu_action_register("L08D61A", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                    lieu_action_register("L08D61B", P_GUARDIAN4, "SP44DIA55", "SP44ARR02_10");
                    CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0xF335 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0xF347 */
                if (ST(L08_TRIG3_TALK_DONE) == 0) {
                    /* 0xF359 */
                    if (ST(L08_G3_FIGHT_STAGE) == 0) {
                        ST(L08_G3_FIGHT_STAGE) = 1;
                    }
                    ST(L08_TRIG3_TALK_DONE) = 1;
                    lieu_action_register("L08D62", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN4);
                    lieu_action_register("L08D59", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                    CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xF402 */
                if (ST(L08_TRIG3_TALK_DONE) == 1) {
                    ST(L08_LAST_WEAPON_SEEN) = ST(ST_PLAYER_WEAPON);
                    lieu_action_register("L08D63", P_ARKHAN1, "SG01PRE60", NULL);
                    CB->stop_joindre(-P_GUARDIAN4);
                    lieu_action_register("L08D64", P_GUARDIAN4, "SP44DIA59", "SP44ARR02_10");
                    CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                    CB->animation("Sar_150");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0xF4A4 */
        if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2)) {
            ST(L08_LAST_WEAPON_SEEN) = ST(ST_PLAYER_WEAPON);
            /* 0xF4E4 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0xF4F6 */
                if (ST(L08_G3_FIGHT_STAGE) == 0) {
                    ST(L08_G3_FIGHT_STAGE) = 1;
                }
                lieu_action_register("L08D65", P_ARKHAN1, "SG01PRE60", NULL);
                CB->stop_joindre(-P_GUARDIAN4);
                lieu_action_register("L08D66", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
                CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                CB->animation("Sar_150");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xF590 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("LCOD18", P_ARKHAN1, "SG01PRE60", NULL);
                CB->stop_joindre(-P_GUARDIAN4);
                lieu_action_register("L08D228", P_GUARDIAN4, "SP44ATT61", "SP44ARR02_10");
                CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                CB->animation("Sar_150");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0xF61B */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN4)) {
        ST(V_CRI_USED) = 1;
        /* 0xF68E */
        if (ST(L08_ARENA_ROUND_CMD) != 0x63) {
            ST(L08_ARENA_ROUND_CMD) = 0x2C;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0, 1000);
        return;
    }
    /* 0xF6C6 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN4)) {
        /* 0xF72A */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("L08D37", P_ARKHAN1, "SP01ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xF787 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD20", P_ARKHAN2, "SP02ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xF7E4 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xF83E */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0;
        return;
    }
    /* 0xF8AF */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN4);
        return;
    }
    /* 0xF8FE */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN4)) {
        /* 0xF953 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            ST(V_PLAYER_STRIKE) = -1;
            CB->select_pile(1, 2);
            CB->evenement(1, -1);
            ST(V_EVENT_1) = 0;
            CB->select_pile(2, 2);
            CB->stop_joindre(-P_GUARDIAN4);
            lieu_action_register("L08D69", P_GUARDIAN4, "SP44GAR02", "SP44GAR02");
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
            CB->combat(P_GUARDIAN4, 0);
            /* 0xF9ED */
            if (ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN3, 0)) {
                ST(V_ENGAGED_PERSONA) = 0x10F8;
                CB->combat(P_GUARDIAN3, 0);
            } else {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
            }
            CB->select_pile(0, 1);
            return;
            return;
        }
        ST(V_PLAYER_STRIKE) = -1;
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN4);
        CB->animation("SP44GAR02_100");
        CB->animation("P44IMP02");
        lieu_action_register("L08D231", P_GUARDIAN4, "P44GAR02", "SP44GAR02");
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
        CB->bloque(BLOQUE_NONE);
        CB->combat(P_GUARDIAN4, 0);
        /* 0xFADC */
        if (ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN3, 0)) {
            ST(V_ENGAGED_PERSONA) = 0x10F8;
            CB->combat(P_GUARDIAN3, 0);
        } else {
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
        }
        CB->select_pile(0, 1);
        return;
    }
}

/* trigger 04 - sub_1000FB3D */
static void trig04(void) {
    /* 0xFB4C */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN5)) {
        CB->select_pile(1, 1);
        /* 0xFBD2 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xFC44 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xFC6C */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xFC94 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xFCFC */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 8)) {
        l08_trig08();
    }
    /* 0xFD21 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 9)) {
        trig09();
    }
    /* 0xFD46 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 10)) {
        trig10();
    }
    /* 0xFD6B */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 11)) {
        trig11();
    }
    /* 0xFD82 */
    if ((ST(L08_G5_ALERT) == 1 || ST(L08_G5_ALERT) == 0xA) && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0xFDC7 */
        if (ST(L08_G5_ALERT) == 0xA) {
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre(-P_GUARDIAN5, 0, 0, 0, ST(ST_HERO));
            /* 0xFE11 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L08D117A", P_GUARDIAN5, "SP45GAR27", "SP45ARR03_10");
                /* 0xFE39 */
                if (ST(ST_COMBAT_STANCE) == 1) {
                    lieu_action_register("L08D117B", P_GUARDIAN5, "SP45GAR03", "SP45GAR03");
                    ST(L08_G5_TALK_DONE) = 1;
                }
                CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
            }
            /* 0xFE89 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L08D270", P_GUARDIAN5);
                CB->combat(P_GUARDIAN5, 0);
                /* 0xFEB8 */
                if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
                    ST(V_ENGAGED_PERSONA) = 0x11C2;
                    CB->combat(P_GUARDIAN6, 0);
                } else {
                    ST(V_ENGAGED_PERSONA) = P_GUARDIAN5;
                }
            }
            CB->bloque(BLOQUE_NONE);
            ST(L08_G5_ALERT) = 2;
            CB->select_pile(0, 1);
            return;
        }
        /* 0xFF1A */
        if (ST(ST_COMBAT_STANCE) == 0 && ST(L08_G5_TALK_DONE) == 0) {
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            ST(L08_G5_ALERT) = 2;
            CB->select_pile(0, 1);
        }
    }
    /* 0xFF90 */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(L08_G5_TALK_DONE) == 0 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L08_G5_TALK_DONE) = 1;
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN5);
        lieu_action_register("L08D116", P_GUARDIAN5, "SP45GAR26", "SP45GAR03");
        CB->joindre_perm(P_GUARDIAN5, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x4B */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L08_G5_TALK_DONE) == 1 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L08_G5_TALK_DONE) = 0;
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN5);
        CB->animation("SP45GAR27");
        CB->animation("P45ARR03");
        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x103 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN5)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x17E */
        if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
            /* 0x1D3 */
            if (ST(V_GUARD_FIGHT_DONE) == 0 && ST(ST_HERO) == P_ARKHAN2) {
                /* 0x1F7 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x209 */
                    if (ST(L08_G5_TALK_STAGE) == 0) {
                        ST(L08_G5_TALK_STAGE) = 1;
                        lieu_action_register("L08D244A", P_ARKHAN2, "SG00ATT01", NULL);
                        lieu_action_register("L08D244B", P_ARKHAN2, "SG00ATT01", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D245A", P_GUARDIAN5, "SP45DIA51", "SP45ARR03_10");
                        lieu_action_register("L08D245B", P_GUARDIAN5, "SP45DIA53", "SP45ARR03_10");
                        lieu_action_register("L08D245C", P_GUARDIAN5, "SP45DIA55", "SP45ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x2EB */
                    if (ST(L08_G5_TALK_STAGE) == 1) {
                        ST(L08_G5_TALK_STAGE) = 2;
                        lieu_action_register("L08D246A", P_ARKHAN2, "SG01DIA03", NULL);
                        lieu_action_register("L08D246B", P_ARKHAN2, "SG01DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D247A", P_GUARDIAN5, "SP45DIA56", "SP45ARR03_10");
                        lieu_action_register("L08D247B", P_GUARDIAN5, "SP45DIA57", "SP45ARR03_10");
                        lieu_action_register("L08D247C", P_GUARDIAN5, "SP45DIA54", "SP45ARR03_10");
                        lieu_action_register("L08D247D", P_GUARDIAN5, "SP45DIA59", "SP45ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x3E6 */
                    if (ST(L08_G5_TALK_STAGE) == 2) {
                        ST(V_DLG_RANDOM) = CB->random(2);
                        /* 0x40F */
                        if (ST(V_DLG_RANDOM) == 0) {
                            lieu_action_register("L08D248A", P_ARKHAN2, "SG02DIA06", NULL);
                            lieu_action_register("L08D248B", P_ARKHAN2, "SG02DIA07", NULL);
                        }
                        /* 0x44D */
                        if (ST(V_DLG_RANDOM) == 1) {
                            lieu_action_register("L08D249", P_ARKHAN2, "SG02DIA07", NULL);
                        }
                        CB->stop_joindre(-P_GUARDIAN5);
                        ST(V_DLG_RANDOM) = CB->random(2);
                        /* 0x497 */
                        if (ST(V_DLG_RANDOM) == 0) {
                            lieu_action_register("L08D250", P_GUARDIAN5, "SP45DIA62", "SP45ARR03_10");
                        }
                        /* 0x4C2 */
                        if (ST(V_DLG_RANDOM) == 1) {
                            lieu_action_register("L08D258", P_GUARDIAN5, "SP45DIA62", "SP45ARR03_10");
                        }
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0x52C */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x53E */
                    if (ST(L08_G5_TALK_STAGE) == 0) {
                        ST(L08_G5_TALK_STAGE) = 1;
                        lieu_action_register("L08D251A", P_ARKHAN2, "SG02DIA01", NULL);
                        lieu_action_register("L08D251B", P_ARKHAN2, "SG02DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D252A", P_GUARDIAN5, "SP45DIA59", "SP45ARR03_10");
                        lieu_action_register("L08D252B", P_GUARDIAN5, "SP45ATT70", "SP45ARR03_10");
                        /* 0x5C8 */
                        if (ST(L08_FLAG_740) > 1 && ST(L08_FLAG_740) < 5) {
                            lieu_action_register("L08D253A", P_ARKHAN2, "SG02DIA01", NULL);
                            lieu_action_register("L08D253B", P_ARKHAN2, "SG02DIA02", NULL);
                            lieu_action_register("L08D253C", P_ARKHAN2, "SG02DIA03", NULL);
                        }
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x66D */
                    if (ST(L08_G5_TALK_STAGE) == 1) {
                        ST(L08_G5_TALK_STAGE) = 2;
                        lieu_action_register("L08D254A", P_ARKHAN2, "SG02DIA05", NULL);
                        lieu_action_register("L08D254B", P_ARKHAN2, "SG02DIA03", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D255A", P_GUARDIAN5, "SP45DIA50", "SP45ARR03_10");
                        lieu_action_register("L08D255B", P_GUARDIAN5, "SP45DIA60", "SP45ARR03_10");
                        lieu_action_register("L08D255C", P_GUARDIAN5, "SP45DIA58", "SP45ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x74F */
                    if (ST(L08_G5_TALK_STAGE) >= 2) {
                        lieu_action_register("L08D256A", P_ARKHAN2, "SG02DIA05", NULL);
                        lieu_action_register("L08D256B", P_ARKHAN2, "SG02DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D257A", P_GUARDIAN5, "SP45DIA59", "SP45ARR03_10");
                        lieu_action_register("L08D257B", P_GUARDIAN5, "SP45DIA60", "SP45GAR26");
                        CB->combat(P_GUARDIAN5, 0);
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(V_ENGAGED_PERSONA), P_GUARDIAN5);
                        /* 0x807 */
                        if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
                            ST(V_ENGAGED_PERSONA) = 0x11C2;
                            CB->combat(P_GUARDIAN6, 0);
                        } else {
                            ST(V_ENGAGED_PERSONA) = P_GUARDIAN5;
                        }
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
            /* 0x850 */
            if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_HERO) == P_ARKHAN2) {
                /* 0x874 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x886 */
                    if (ST(L08_G5_TALK_STAGE) == 0) {
                        ST(L08_G5_TALK_STAGE) = 1;
                        lieu_action_register("L08D244A", P_ARKHAN2, "SG00ATT01", NULL);
                        lieu_action_register("L08D244B", P_ARKHAN2, "SG00ATT01", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D260A", P_GUARDIAN5, "SP45DIA50", "SP45ARR03_10");
                        lieu_action_register("L08D260B", P_GUARDIAN5, "SP45DIA63", "SP45ARR03_10");
                        lieu_action_register("L08D260C", P_GUARDIAN5, "SP45DIA65", "SP45GAR26");
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x968 */
                    if (ST(L08_G5_TALK_STAGE) >= 1) {
                        lieu_action_register("L08D261A", P_ARKHAN1, "SG01DIA03", NULL);
                        lieu_action_register("L08D261B", P_ARKHAN1, "SG01DIA07", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D262A", P_GUARDIAN5, "SP45DIA55", "SP45ARR03_10");
                        lieu_action_register("L08D262B", P_GUARDIAN5, "SP45DIA50", "SP45GAR26");
                        CB->combat(P_GUARDIAN5, 0);
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(V_ENGAGED_PERSONA), P_GUARDIAN5);
                        /* 0xA20 */
                        if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
                            ST(V_ENGAGED_PERSONA) = 0x11C2;
                            CB->combat(P_GUARDIAN6, 0);
                        } else {
                            ST(V_ENGAGED_PERSONA) = P_GUARDIAN5;
                        }
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xA69 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0xA7B */
                    if (ST(L08_G5_TALK_STAGE) == 0) {
                        ST(L08_G5_TALK_STAGE) = 1;
                        lieu_action_register("L08D263A", P_ARKHAN2, "SG02DIA01", NULL);
                        lieu_action_register("L08D263B", P_ARKHAN2, "SG02DIA07", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D264", P_GUARDIAN5, "SP45ATT70", "SP45ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB2B */
                    if (ST(L08_G5_TALK_STAGE) == 1) {
                        ST(L08_G5_TALK_STAGE) = 2;
                        lieu_action_register("L08D265A", P_ARKHAN2, "SG02DIA03", NULL);
                        lieu_action_register("L08D265B", P_ARKHAN2, "SG02DIA02", NULL);
                        lieu_action_register("L08D265C", P_ARKHAN2, "SG02DIA01", NULL);
                        lieu_action_register("L08D265D", P_ARKHAN2, "SG02DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        lieu_action_register("L08D266", P_GUARDIAN5, "SP45DIA53", "SP45ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xC07 */
                    if (ST(L08_G5_TALK_STAGE) >= 2) {
                        lieu_action_register("L08D267A", P_ARKHAN2, "SG02DIA06", NULL);
                        lieu_action_register("L08D267B", P_ARKHAN2, "SG02DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN5);
                        ST(V_DLG_RANDOM) = CB->random(2);
                        /* 0xC67 */
                        if (ST(V_DLG_RANDOM) == 0) {
                            lieu_action_register("L08D268", P_GUARDIAN5, "SP45DIA54", "SP45ARR03_10");
                        }
                        /* 0xC92 */
                        if (ST(V_DLG_RANDOM) == 1) {
                            lieu_action_register("L08D269", P_GUARDIAN5, "SP45DIA54", "SP45ARR03_10");
                        }
                        CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
                        CB->animation("Sar_150");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
        }
        /* 0xD01 */
        if (!CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != 0) {
            ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
            /* 0xD39 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
            }
            /* 0xD64 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("LCOD19", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
            }
            CB->stop_joindre(-P_GUARDIAN5);
            lieu_action_register("L08D119", P_GUARDIAN5, "SP45ATT61", "SP45ARR03_10");
            CB->joindre_perm(-P_GUARDIAN5, ST(ST_HERO));
            CB->animation("Sar_150");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xDF2 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN5)) {
        ST(V_CRI_USED) = 1;
        /* 0xE65 */
        if (ST(L08_ARENA_ROUND_CMD) != 0x63) {
            ST(L08_ARENA_ROUND_CMD) = 0x2D;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0, 1000);
        return;
    }
    /* 0xE9D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN5)) {
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xF0D */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD20", P_ARKHAN2, "SP03ATT51", NULL);
        }
        /* 0xF35 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
        }
        CB->dialogue("L08D270", P_GUARDIAN5);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xF88 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN5)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN5);
        return;
    }
    /* 0xFE2 */
    if (!CB->divers(DV_WAS_HIT, P_GUARDIAN5, 0) && ST(V_PLAYER_STRIKE) >= 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN5)) {
        /* 0x1043 */
        if (ST(V_PLAYER_STRIKE) == -1) {
        }
        CB->select_pile(1, 2);
        CB->bloque(BLOQUE_NONE);
        CB->stop_joindre(-P_GUARDIAN5);
        /* 0x107D */
        if (ST(L08_G5_TALK_DONE) == 0) {
            lieu_action_register("L08D123", P_GUARDIAN5, "SP45GAR26", "SP45GAR03");
        }
        /* 0x10A8 */
        if (ST(L08_G5_TALK_DONE) == 1) {
            lieu_action_register("L08D123", P_GUARDIAN5, "SP45GAR03", "SP45GAR03");
        }
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN5;
        CB->combat(P_GUARDIAN5, 0);
        /* 0x10EC */
        if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0x11C2;
            CB->combat(P_GUARDIAN6, 0);
        } else {
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN5;
        }
        CB->select_pile(0, 1);
        return;
    }
}

/* trigger 05 - sub_10011133 */
static void trig05(void) {
    /* 0x1142 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN6)) {
        CB->select_pile(1, 1);
        /* 0x11C8 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0x123A */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0x1262 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0x128A */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x12F2 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 8)) {
        l08_trig08();
    }
    /* 0x1317 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 9)) {
        trig09();
    }
    /* 0x133C */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 10)) {
        trig10();
    }
    /* 0x1361 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 11)) {
        trig11();
    }
    /* 0x1378 */
    if ((ST(L08_G6_APPROACH) == 1 || ST(L08_G6_APPROACH) == 0xA) && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0x13BD */
        if (ST(L08_G6_APPROACH) == 0xA) {
            ST(L08_G6_APPROACH) = 2;
            CB->select_pile(2, 1);
            CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN6, 1);
            CB->stop_joindre(-P_GUARDIAN6);
            /* 0x140B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x141A */
                if (ST(ST_COMBAT_STANCE) == 1) {
                    lieu_action_register("L08D114A", P_GUARDIAN6, "SP46GAR03", "SP46GAR03");
                    lieu_action_register("L08D114B", P_GUARDIAN6, "SP46GAR03", "SP46GAR03");
                    ST(L08_G6_TALK_DONE) = 1;
                } else {
                    lieu_action_register("L08D125", P_GUARDIAN6, "SP46GAR27", "SP46ARR03_10");
                }
                CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
            }
            /* 0x14A1 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L08D271", P_GUARDIAN6);
                ST(V_ENGAGED_PERSONA) = 0x11C2;
                CB->combat(P_GUARDIAN6, 0);
                /* 0x14DC */
                if (ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD) {
                    ST(V_ENGAGED_PERSONA) = 0x11C2;
                    CB->combat(P_GUARDIAN5, 0);
                } else {
                    ST(V_ENGAGED_PERSONA) = P_GUARDIAN6;
                }
            }
            CB->select_pile(0, 1);
            CB->bloque(BLOQUE_NONE);
            return;
        } else if (ST(L08_G6_APPROACH) == 1 && CB->direction_perso(P_GUARDIAN6) && ST(ST_COMBAT_STANCE) == 0 && ST(L08_G6_TALK_DONE) == 0) {
            ST(L08_G6_APPROACH) = 2;
            CB->select_pile(2, 1);
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN6, 1);
            CB->stop_joindre(-P_GUARDIAN6);
            CB->joindre_perm(P_GUARDIAN6, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x15F1 */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(L08_G6_TALK_DONE) == 0 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L08_G6_TALK_DONE) = 1;
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN6);
        lieu_action_register("L08D115", P_GUARDIAN6, "SP46GAR26", "SP46GAR03");
        CB->joindre_perm(P_GUARDIAN6, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x16AC */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L08_G6_TALK_DONE) == 1 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L08_G6_TALK_DONE) = 0;
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN6);
        CB->dialogue("L08D272", P_GUARDIAN6);
        CB->animation("SP46GAR27");
        CB->animation("P46ARR03");
        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x1774 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN6)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x17EF */
        if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(L08_LAST_WEAPON_SEEN))) {
            /* 0x1847 */
            if (ST(V_GUARD_FIGHT_DONE) == 0 && ST(ST_HERO) == P_ARKHAN2) {
                /* 0x186B */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x187D */
                    if (ST(L08_G7_TALK_STAGE) == 0) {
                        ST(L08_G7_TALK_STAGE) = 1;
                        lieu_action_register("L08D273A", P_ARKHAN2, "SG00ATT01", NULL);
                        lieu_action_register("L08D273B", P_ARKHAN2, "SG00ATT01", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D274A", P_GUARDIAN6, "SP46DIA53", "SP46ARR03_10");
                        lieu_action_register("L08D274B", P_GUARDIAN6, "SP46DIA55", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x1946 */
                    if (ST(L08_G7_TALK_STAGE) == 1) {
                        ST(L08_G7_TALK_STAGE) = 2;
                        lieu_action_register("L08D275", P_ARKHAN2, "SG00DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D276", P_GUARDIAN6, "SP46DIA56", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        lieu_action_register("L08D277", P_ARKHAN2, "SG00DIA03", NULL);
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x19F6 */
                    if (ST(L08_G7_TALK_STAGE) == 2) {
                        lieu_action_register("L08D278", P_ARKHAN2, "SG00DIA06", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        /* 0x1A29 */
                        if (ST(L08_G7_STEP) == 0) {
                            lieu_action_register("L08D279", P_GUARDIAN6, "SP46DIA62", "SP46ARR03_10");
                            ST(L08_G7_STEP) = 1;
                        } else {
                            lieu_action_register("L08D280", P_GUARDIAN6, "SP46DIA62", "SP46ARR03_10");
                            ST(L08_G7_STEP) = 0;
                        }
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0x1ACF */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x1AE1 */
                    if (ST(L08_G7_TALK_STAGE) == 0) {
                        ST(L08_G7_TALK_STAGE) = 1;
                        lieu_action_register("L08D281A", P_ARKHAN2, "SG00DIA05", NULL);
                        lieu_action_register("L08D281B", P_ARKHAN2, "SG00DIA01", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D282A", P_GUARDIAN6, "SP46ATT70", "SP46ARR03_10");
                        lieu_action_register("L08D282B", P_GUARDIAN6, "SP46DIA60", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x1BAA */
                    if (ST(L08_G7_TALK_STAGE) == 1) {
                        ST(L08_G7_TALK_STAGE) = 2;
                        lieu_action_register("L08D283", P_ARKHAN2, "SG00DIA03", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D284", P_GUARDIAN6, "SP46DIA56", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x1C44 */
                    if (ST(L08_G7_TALK_STAGE) >= 2) {
                        lieu_action_register("L08D285", P_ARKHAN2, "SG00DIA02", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D286", P_GUARDIAN6, "SP46DIA54", "SP46ARR03_10");
                        CB->combat(P_GUARDIAN6, 0);
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(V_ENGAGED_PERSONA), P_GUARDIAN6);
                        /* 0x1CCD */
                        if (ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD) {
                            ST(V_ENGAGED_PERSONA) = 0x11C2;
                            CB->combat(P_GUARDIAN5, 0);
                        } else {
                            ST(V_ENGAGED_PERSONA) = P_GUARDIAN6;
                        }
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
            /* 0x1D16 */
            if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_HERO) == P_ARKHAN2) {
                /* 0x1D3A */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x1D4C */
                    if (ST(L08_G7_TALK_STAGE) == 0) {
                        ST(L08_G7_TALK_STAGE) = 1;
                        lieu_action_register("L08D287A", P_ARKHAN2, "SG02ATT01", NULL);
                        lieu_action_register("L08D287B", P_ARKHAN2, "SG02DIA01", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D288", P_GUARDIAN6, "SP46DIA53", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        lieu_action_register("L08D289", P_ARKHAN2, "SG02DIA02", NULL);
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x1E12 */
                    if (ST(L08_G7_TALK_STAGE) == 1) {
                        ST(L08_G7_TALK_STAGE) = 2;
                        lieu_action_register("L08D290A", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L08D290B", P_ARKHAN2, "SG00DIA01", NULL);
                        lieu_action_register("L08D290C", P_ARKHAN2, "SG00DIA05", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D291", P_GUARDIAN6, "SP46DIA56", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        lieu_action_register("L08D292A", P_ARKHAN2, "SG00DIA07", NULL);
                        lieu_action_register("L08D292B", P_ARKHAN2, "SG00DIA06", NULL);
                        lieu_action_register("L08D292C", P_ARKHAN2, "SG00DIA04", NULL);
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x1F1A */
                    if (ST(L08_G7_TALK_STAGE) == 2) {
                        lieu_action_register("L08D293", P_ARKHAN2, "SG00DIA07", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D294", P_GUARDIAN6, "SP46DIA62", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0x1FA5 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x1FB7 */
                    if (ST(L08_G7_TALK_STAGE) == 0) {
                        ST(L08_G7_TALK_STAGE) = 1;
                        lieu_action_register("L08D295", P_ARKHAN2, "SG00ATT01", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D296A", P_GUARDIAN6, "SP46DIA53", "SP46ARR03_10");
                        lieu_action_register("L08D296B", P_GUARDIAN6, "SP46DIA54", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x206A */
                    if (ST(L08_G7_TALK_STAGE) == 1) {
                        ST(L08_G7_TALK_STAGE) = 2;
                        lieu_action_register("L08D297A", P_ARKHAN2, "SG00DIA01", NULL);
                        lieu_action_register("L08D297B", P_ARKHAN2, "SG00DIA02", NULL);
                        lieu_action_register("L08D297C", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L08D297D", P_ARKHAN2, "SG00DIA04", NULL);
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D298", P_GUARDIAN6, "SP46DIA56", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        lieu_action_register("L08D299A", P_ARKHAN2, "SG00DIA05", NULL);
                        lieu_action_register("L08D299B", P_ARKHAN2, "SG00DIA06", NULL);
                        lieu_action_register("L08D299C", P_ARKHAN2, "SG00DIA04", NULL);
                        lieu_action_register("L08D299D", P_ARKHAN2, "SG00DIA07", NULL);
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0x219E */
                    if (ST(L08_G7_TALK_STAGE) == 2) {
                        ST(V_DLG_RANDOM) = CB->random(2);
                        /* 0x21C7 */
                        if (ST(V_DLG_RANDOM) == 0) {
                            lieu_action_register("L08D300", P_ARKHAN2, "SG00DIA07", NULL);
                        }
                        /* 0x21EF */
                        if (ST(V_DLG_RANDOM) == 1) {
                            lieu_action_register("L08D301", P_ARKHAN2, "SG00DIA07", NULL);
                        }
                        CB->stop_joindre(-P_GUARDIAN6);
                        lieu_action_register("L08D302", P_GUARDIAN6, "SP46DIA62", "SP46ARR03_10");
                        CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
        }
        /* 0x227F */
        if (!CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != 0) {
            /* 0x22A3 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                ST(L08_LAST_WEAPON_SEEN) = ST(ST_PLAYER_WEAPON);
                lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                CB->stop_joindre(-P_GUARDIAN6);
                lieu_action_register("L08D138", P_GUARDIAN6, "SP46ATT61", "SP46ARR03_10");
                CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                CB->animation("Sar_50");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x2348 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("LCOD19", P_ARKHAN3, "SG02PRE60", NULL);
                CB->stop_joindre(-P_GUARDIAN6);
                lieu_action_register("L08D138", P_GUARDIAN6, "SP46ATT61", "SP46ARR03_10");
                CB->joindre_perm(-P_GUARDIAN6, ST(ST_HERO));
                CB->animation("Sar_50");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0x23D3 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN6)) {
        ST(V_CRI_USED) = 1;
        /* 0x2446 */
        if (ST(L08_ARENA_ROUND_CMD) != 0x63) {
            ST(L08_ARENA_ROUND_CMD) = 0x2E;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0, 1000);
        return;
    }
    /* 0x247E */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN6)) {
        /* 0x24E2 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD20", P_ARKHAN2, "SP02ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x253F */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x2599 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN6)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN6);
        return;
    }
    /* 0x25F3 */
    if (!CB->divers(DV_WAS_HIT, P_GUARDIAN6, 0) && ST(V_PLAYER_STRIKE) >= 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN6)) {
        CB->select_pile(2, 2);
        /* 0x2664 */
        if (ST(V_PLAYER_STRIKE) == -1) {
        }
        CB->bloque(BLOQUE_NONE);
        CB->stop_joindre(-P_GUARDIAN6);
        lieu_action_register("L08D140", P_GUARDIAN6, "SP46GAR03", "SP46GAR03");
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN6;
        CB->combat(P_GUARDIAN6, 0);
        /* 0x26C0 */
        if (ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0x11C2;
            CB->combat(P_GUARDIAN5, 0);
        } else {
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN6;
        }
        CB->select_pile(0, 1);
        return;
    }
}

/* trigger 06 - sub_10012707 */
static void trig06(void) {
    /* 0x2716 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN8)) {
        CB->select_pile(1, 1);
        /* 0x279C */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0x280E */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0x2836 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0x285E */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x28B8 */
    if (ST(L08_G7_INTRO_DONE) == 0 && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L08_G7_INTRO_DONE) = 1;
        CB->stop_joindre(-P_GUARDIAN7);
        /* 0x2902 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(L08_G8_STATE) = 5;
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN7;
            CB->select_pile(1, 2);
            CB->animation("SP47GAR05");
            CB->dialogue("L08D149", P_GUARDIAN7);
            CB->combat(P_GUARDIAN7, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x2971 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN7)) {
        /* 0x29D5 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD20", P_ARKHAN2, "SP02ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x2A32 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x2A8C */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN7)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN7);
        return;
    }
}

/* trigger 07 - sub_10012AC8 */
static void trig07(void) {
    /* 0x2AD7 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN8)) {
        CB->select_pile(1, 1);
        /* 0x2B5D */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0x2BCF */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0x2BF7 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0x2C1F */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x2C79 */
    if (ST(L08_TRIG7_DONE) == 0) {
        ST(L08_TRIG7_DONE) = 1;
    }
    /* 0x2C97 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN8)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN8);
        return;
    }
}

/* trigger 08 - sub_10012CE2 */
void l08_trig08(void) {
    CB->select_pile(0, 2);
    CB->retire_zone(8);
    ST(V_SAVED_HERO) = ST(ST_HERO);
    /* 0x2D23 */
    if (ST(L08_FLAG_170) == 1) {
        ST(L08_FLAG_170) = 2;
    }
    /* 0x2D44 */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x2DBF */
        if (ST(V_RETURN_PART) == 0) {
            ST(V_GUARDIAN4_STAGE) = 1;
        } else {
            ST(V_GUARDIAN4_STAGE) = 5;
        }
    }
    /* 0x2DF4 */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_BONUS_ROUND) = 2;
    }
    /* 0x2E7E */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_UNLOCKED) != 0 && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_G7_DEAD_FLAG) = 1;
    }
    /* 0x2F1A */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1) {
        /* 0x2F3B */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            /* 0x2F4D */
            if (ST(ST_VIE(P_GUARDIAN10)) >= 2) {
                ST(L08_G10_STATE) = 1;
            }
            /* 0x2F6E */
            if (ST(ST_VIE(P_GUARDIAN10)) < 2) {
                ST(L08_G10_STATE) = 2;
            }
            /* 0x2F8C */
            if (ST(L08_KALHI13_ZONE_STEP) != 3) {
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                ST(L08_KALHI13_ZONE_STEP) = 3;
                ST(L08_FOLLOWER_MODE) = 0x14;
                ST(L08_KALHI13_STAYS) = 1;
                ST(L08_KALHI13_CHAT_READY) = 0xA;
                CB->stop_joindre(-P_KALHI13);
                CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L08D158", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                ST(ST_LEVEL) = 8;
                CB->change_piece(2, 0);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x3073 */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
            /* 0x3097 */
            if (ST(L08_G10_STATE) != 0x14) {
                ST(L08_G10_STATE) = 0x14;
            }
            ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
            ST(V_KALHI_KILLED) = 1;
        }
    }
    /* 0x30D6 */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1) {
        /* 0x30FA */
        if (ST(L08_KALHI33_STEP) == 0 || ST(L08_KALHI33_STEP) == 2) {
            ST(V_KALHI_PHASE) = 0x64;
        }
        /* 0x312D */
        if (ST(L08_KALHI33_STEP) == 6) {
            ST(V_KALHI_PHASE) = 0xC8;
        }
    }
    /* 0x314E */
    if (ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(V_DELIA_PHASE) == 6 || ST(V_DELIA_PHASE) == 7) {
        CB->divers(DV_MUSIC, 1, 4);
    }
    /* 0x31BA */
    if (ST(V_GUARDIAN8_STAGE) == 1) {
        ST(V_GUARDIAN8_STAGE) = 2;
    }
    /* 0x31DB */
    if (ST(V_DELIA_PHASE) == 7) {
        /* 0x31EA */
        if (ST(V_ENGAGED_PERSONA) != 0) {
            ST(ST_VIE(P_DELIA)) = VIE_DEAD;
            ST(V_DELIA_PHASE) = 0x14;
            ST(V_DELIA_KILLED) = 8;
        } else {
            ST(L08_DELIA_ROUTE) = 1;
        }
    }
    ST(ST_LEVEL) = 8;
    CB->change_piece(2, 0);
}

/* trigger 09 - sub_1001324F */
static void trig09(void) {
    CB->select_pile(0, 2);
    CB->retire_zone(9);
    ST(V_SAVED_HERO) = ST(ST_HERO);
    /* 0x3290 */
    if (ST(L08_FLAG_170) == 1) {
        ST(L08_FLAG_170) = 2;
    }
    /* 0x32B1 */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x332C */
        if (ST(V_RETURN_PART) == 0) {
            ST(V_GUARDIAN4_STAGE) = 1;
        } else {
            ST(V_GUARDIAN4_STAGE) = 5;
        }
    }
    /* 0x3361 */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_BONUS_ROUND) = 2;
    }
    /* 0x33EB */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_UNLOCKED) != 0 && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_G7_DEAD_FLAG) = 1;
    }
    /* 0x3487 */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1) {
        /* 0x34A8 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            /* 0x34BA */
            if (ST(ST_VIE(P_GUARDIAN10)) >= 2) {
                ST(L08_G10_STATE) = 1;
            }
            /* 0x34DB */
            if (ST(ST_VIE(P_GUARDIAN10)) < 2) {
                ST(L08_G10_STATE) = 2;
            }
            /* 0x34F9 */
            if (ST(L08_KALHI13_ZONE_STEP) != 3) {
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                ST(L08_KALHI13_ZONE_STEP) = 3;
                ST(L08_FOLLOWER_MODE) = 0x14;
                ST(L08_KALHI13_STAYS) = 1;
                ST(L08_KALHI13_CHAT_READY) = 0xA;
                CB->stop_joindre(-P_KALHI13);
                CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L08D158", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                ST(ST_LEVEL) = 8;
                CB->change_piece(2, 0);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x35E0 */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
            /* 0x3604 */
            if (ST(L08_G10_STATE) != 0x14) {
                ST(L08_G10_STATE) = 0x14;
            }
            ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
            ST(V_KALHI_KILLED) = 1;
        }
    }
    /* 0x3643 */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1) {
        /* 0x3667 */
        if (ST(L08_KALHI33_STEP) == 0 || ST(L08_KALHI33_STEP) == 2) {
            ST(V_KALHI_PHASE) = 0x64;
        }
        /* 0x369A */
        if (ST(L08_KALHI33_STEP) == 6) {
            ST(V_KALHI_PHASE) = 0xC8;
        }
    }
    /* 0x36BB */
    if (ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(V_DELIA_PHASE) == 6 || ST(V_DELIA_PHASE) == 7) {
        CB->divers(DV_MUSIC, 1, 4);
    }
    /* 0x3727 */
    if (ST(V_GUARDIAN8_STAGE) == 1) {
        ST(V_GUARDIAN8_STAGE) = 2;
    }
    /* 0x3748 */
    if (ST(V_DELIA_PHASE) == 7) {
        /* 0x3757 */
        if (ST(V_ENGAGED_PERSONA) != 0) {
            ST(ST_VIE(P_DELIA)) = VIE_DEAD;
            ST(V_DELIA_PHASE) = 0x14;
            ST(V_DELIA_KILLED) = 8;
        } else {
            ST(L08_DELIA_ROUTE) = 1;
        }
    }
    ST(ST_LEVEL) = 8;
    CB->change_piece(2, 0);
}

/* trigger 10 - sub_100137BC */
static void trig10(void) {
    CB->select_pile(0, 2);
    CB->retire_zone(10);
    ST(V_SAVED_HERO) = ST(ST_HERO);
    /* 0x37FD */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x3878 */
        if (ST(V_RETURN_PART) == 0) {
            ST(V_GUARDIAN4_STAGE) = 1;
        } else {
            ST(V_GUARDIAN4_STAGE) = 5;
        }
    }
    /* 0x38AD */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_BONUS_ROUND) = 2;
    }
    /* 0x3937 */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_UNLOCKED) != 0 && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_G7_DEAD_FLAG) = 1;
    }
    ST(L08_FROM_ROOM10) = 3;
    ST(ST_LEVEL) = 8;
    /* 0x39EE */
    if (ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(V_DELIA_PHASE) == 6 || ST(V_DELIA_PHASE) == 7) {
        CB->divers(DV_MUSIC, 1, 4);
    }
    /* 0x3A5A */
    if (ST(V_DELIA_PHASE) == 7 && ST(ST_SCRIPT_BUSY) == 0) {
        /* 0x3A7B */
        if (ST(ST_VIE(34)) == VIE_DEAD) {
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_DELIA);
            ST(L08_FOLLOWER_MODE) = 0xA;
            /* 0x3ABB */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L10D02A", P_ARKHAN2);
            }
            /* 0x3ADD */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L10D02B", P_ARKHAN3);
            }
            /* 0x3AFF */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->change_piece(10, 0);
            }
            /* 0x3B1E */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
                CB->change_piece(10, 1);
            }
            /* 0x3B52 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
                CB->change_piece(10, 2);
            }
            /* 0x3B86 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
                CB->change_piece(10, 3);
            }
            /* 0x3BBA */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 3) {
                CB->change_piece(51, 0);
            }
            CB->bloque(BLOQUE_NONE);
            return;
        } else {
            ST(ST_VIE(P_DELIA)) = VIE_DEAD;
            ST(V_DELIA_PHASE) = 0x14;
            ST(V_DELIA_KILLED) = 8;
            /* 0x3C2A */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->change_piece(10, 0);
            }
            /* 0x3C49 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
                CB->change_piece(10, 1);
            }
            /* 0x3C7D */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
                CB->change_piece(10, 2);
            }
            /* 0x3CB1 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
                CB->change_piece(10, 3);
            }
            /* 0x3CE5 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 3) {
                CB->change_piece(51, 0);
            }
            return;
        }
    }
    /* 0x3D1B */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1) {
        /* 0x3D3C */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            CB->stop_joindre(-P_KALHI13);
            ST(L08_FOLLOWER_MODE) = 0xA;
            ST(L08_COMPANION_ENTRY_DONE) = 0;
            ST(V_KALHI_PHASE) = 7;
            ST(L08_G10_STATE) = 1;
        } else {
            ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
            ST(V_KALHI_KILLED) = 1;
        }
    }
    /* 0x3DB5 */
    if (ST(V_KALHI_PHASE) == 5 && (ST(ST_VIE(P_KALHI13)) == VIE_DEAD || ST(ST_VIE(P_KALHI33)) == VIE_DEAD) && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
        ST(L08_G10_STATE) = 1;
    }
    /* 0x3E0C */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1) {
        ST(L08_COMPANION_ENTRY_DONE) = 0;
        ST(V_KALHI_PHASE) = 7;
        /* 0x3E4E */
        if (ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
            ST(L08_G10_STATE) = 1;
        } else if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(L08_G10_STATE) = 2;
        } else {
            ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
            ST(V_KALHI_KILLED) = 1;
        }
    }
    /* 0x3EB5 */
    if (ST(V_GUARDIAN8_STAGE) == 1) {
        ST(V_GUARDIAN8_STAGE) = 2;
    }
    /* 0x3ED6 */
    if (ST(V_DELIA_PHASE) != 7) {
        /* 0x3EE8 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->change_piece(10, 0);
        }
        /* 0x3F07 */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
            CB->change_piece(10, 1);
        }
        /* 0x3F3B */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
            CB->change_piece(10, 2);
        }
        /* 0x3F6F */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
            CB->change_piece(10, 3);
        }
        /* 0x3FA3 */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 3) {
            CB->change_piece(51, 0);
        }
    }
}

/* trigger 11 - sub_10013FCD */
static void trig11(void) {
    CB->select_pile(0, 2);
    CB->retire_zone(11);
    ST(V_SAVED_HERO) = ST(ST_HERO);
    /* 0x400E */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x4089 */
        if (ST(V_RETURN_PART) == 0) {
            ST(V_GUARDIAN4_STAGE) = 1;
        } else {
            ST(V_GUARDIAN4_STAGE) = 5;
        }
    }
    /* 0x40BE */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_BONUS_ROUND) = 2;
    }
    /* 0x4148 */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_UNLOCKED) != 0 && ST(V_ENGAGED_PERSONA) != 0) {
        ST(L08_G7_DEAD_FLAG) = 1;
    }
    ST(ST_LEVEL) = 8;
    ST(L08_FROM_ROOM10) = 4;
    /* 0x41FF */
    if (ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(V_DELIA_PHASE) == 6 || ST(V_DELIA_PHASE) == 7) {
        CB->divers(DV_MUSIC, 1, 4);
    }
    /* 0x426B */
    if (ST(V_DELIA_PHASE) == 7 && ST(ST_SCRIPT_BUSY) == 0) {
        /* 0x428C */
        if (ST(ST_VIE(34)) == VIE_DEAD) {
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_DELIA);
            ST(L08_FOLLOWER_MODE) = 0xA;
            /* 0x42CC */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L10D02A", P_ARKHAN2);
            }
            /* 0x42EE */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L10D02B", P_ARKHAN3);
            }
            /* 0x4310 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->change_piece(10, 0);
            }
            /* 0x432F */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
                CB->change_piece(10, 1);
            }
            /* 0x4363 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
                CB->change_piece(10, 2);
            }
            /* 0x4397 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
                CB->change_piece(10, 3);
            }
            /* 0x43CB */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 3) {
                CB->change_piece(51, 0);
            }
            CB->bloque(BLOQUE_NONE);
            return;
        } else {
            ST(ST_VIE(P_DELIA)) = VIE_DEAD;
            ST(V_DELIA_PHASE) = 0x14;
            ST(V_DELIA_KILLED) = 8;
            /* 0x443B */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->change_piece(10, 0);
            }
            /* 0x445A */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
                CB->change_piece(10, 1);
            }
            /* 0x448E */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
                CB->change_piece(10, 2);
            }
            /* 0x44C2 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
                CB->change_piece(10, 3);
            }
            /* 0x44F6 */
            if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 3) {
                CB->change_piece(51, 0);
            }
            return;
        }
    }
    /* 0x452C */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1) {
        /* 0x454D */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            CB->stop_joindre(-P_KALHI13);
            ST(L08_FOLLOWER_MODE) = 0xA;
            ST(L08_COMPANION_ENTRY_DONE) = 0;
            ST(V_KALHI_PHASE) = 7;
            ST(L08_G10_STATE) = 1;
        } else {
            ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
            ST(V_KALHI_KILLED) = 1;
        }
    }
    /* 0x45C6 */
    if (ST(V_KALHI_PHASE) == 5 && (ST(ST_VIE(P_KALHI13)) == VIE_DEAD || ST(ST_VIE(P_KALHI33)) == VIE_DEAD) && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
        ST(L08_G10_STATE) = 1;
    }
    /* 0x461D */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1) {
        ST(L08_COMPANION_ENTRY_DONE) = 0;
        ST(V_KALHI_PHASE) = 7;
        /* 0x465F */
        if (ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
            ST(L08_G10_STATE) = 1;
        } else if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(L08_G10_STATE) = 2;
        } else {
            ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
            ST(V_KALHI_KILLED) = 1;
        }
    }
    /* 0x46C6 */
    if (ST(V_GUARDIAN8_STAGE) == 1) {
        ST(V_GUARDIAN8_STAGE) = 2;
    }
    /* 0x46E7 */
    if (ST(V_DELIA_PHASE) != 7) {
        /* 0x46F9 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->change_piece(10, 0);
        }
        /* 0x4718 */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 0) {
            CB->change_piece(10, 1);
        }
        /* 0x474C */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 1) {
            CB->change_piece(10, 2);
        }
        /* 0x4780 */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 2) {
            CB->change_piece(10, 3);
        }
        /* 0x47B4 */
        if (ST(ST_HERO) > 1 && ST(V_ROUTE_STAGE) == 3) {
            CB->change_piece(51, 0);
        }
    }
}

/* trigger 12 - sub_100147DE */
static void trig12(void) {
    c8 buf1[48], buf2[48];   /* DLL globals dword_1003F970 / dword_1003F770 */
    /* 0x47ED */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x4839 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x4885 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x48D1 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x491D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x4969 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x49B5 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x4A01 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x4A4D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x4A99 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x4AE5 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x4B31 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x6590)) {
        trig28();
        return;
    }
    /* 0x4B8A */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x1F40)) {
        /* 0x4BDC */
        if (ST(ST_PLAYER_WEAPON) != 0x18 && ST(ST_PLAYER_WEAPON) != 0x4D) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), 0x1F08, 1, "ar", 1, 0);
            /* 0x4C6C */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT07");
                CB->evenement(20, 250);
                CB->evenement(2, 2000);
                CB->animation("ar_1000");
                CB->animation("ar");
                CB->dialogue("L08D306", P_ARKHAN1);
                return;
            }
            /* 0x4CD4 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x4CE6 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x4D00 */
                    if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                        CB->animation("SP02ACT07");
                    } else {
                        CB->animation("SP02ACT35");
                    }
                }
                /* 0x4D2D */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x4D47 */
                    if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                        CB->animation("SP02ACT34");
                    } else {
                        CB->animation("SP02ACT36");
                    }
                }
                CB->evenement(20, 250);
                CB->evenement(2, 2000);
                CB->animation("ar_1000");
                CB->animation("ar");
                CB->dialogue("L08D307", P_ARKHAN2);
                return;
            }
            /* 0x4DBF */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x4DD1 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x4DEB */
                    if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                        CB->animation("SP03ACT07");
                    } else {
                        CB->animation("SP03ACT35");
                    }
                }
                /* 0x4E18 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x4E32 */
                    if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                        CB->animation("SP03ACT34");
                    } else {
                        CB->animation("SP03ACT36");
                    }
                }
                CB->evenement(20, 250);
                CB->evenement(2, 2000);
                CB->animation("ar_1000");
                CB->animation("ar");
                CB->dialogue("L08D308", P_ARKHAN3);
                return;
            }
        }
        /* 0x4EAA */
        if (ST(ST_PLAYER_WEAPON) == 0x4D) {
            CB->bloque(BLOQUE_ALL);
            CB->select_pile(0, 1);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), 0x1F08, 1, "ar", 1, 0);
            /* 0x4F35 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT03");
            }
            /* 0x4F52 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x4F64 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP02ACT03");
                }
                /* 0x4F81 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP02ACT33");
                }
            }
            /* 0x4F9E */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x4FB0 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP03ACT03");
                }
                /* 0x4FCD */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP03ACT33");
                }
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            ST(L08_FLAG_780) = 6;
            ST(ST_VARIANT_COUNT(ST(ST_USED_VARIANT))) = 3;
            snprintf(buf1, sizeof(buf1), "OB77%c", (char)(ST(ST_USED_VARIANT) + 0x41));
            CB->etat_objet(buf1, -1, -1, -1, -1, -1, -1, -2, 3);
            snprintf(buf2, sizeof(buf2), "OB24%c", (char)(ST(ST_USED_VARIANT) + 0x41));
            CB->etat_objet(buf2, -1, -1, -1, -1, -1, -1, -3, 0);
            CB->bloque(BLOQUE_NONE);
            CB->animation("ar");
            return;
        }
        /* 0x50BC */
        if (ST(ST_PLAYER_WEAPON) == 0x18) {
            CB->bloque(BLOQUE_ALL);
            CB->select_pile(0, 1);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), 0x1F08, 1, "ar", 1, 0);
            /* 0x5147 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                lieu_action_register("L08D70", P_ARKHAN1, "SG01ATT01", NULL);
            }
            /* 0x516F */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L08D71", P_ARKHAN2, "SG00ATT01", NULL);
            }
            /* 0x5197 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L08D72", P_ARKHAN3, "SG00ATT01", NULL);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("ar");
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 13 - sub_100151E1 */
static void trig13(void) {
    /* 0x51F0 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x523C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x5288 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x52D4 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x5320 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x536C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x53B8 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x5404 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x5450 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x549C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x54E8 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x5534 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 14) && CB->direction_spec(0)) {
        trig14();
        return;
    }
    /* 0x557D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x564F */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x5661 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x56A7 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x56F8 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x570A */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x5750 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x576A */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x57CB */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x57DD */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x5823 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x583D */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 14 - sub_10015897 */
static void trig14(void) {
    /* 0x58A6 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x58F2 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x593E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x598A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x59D6 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x5A22 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x5A6E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x5ABA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x5B06 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x5B52 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x5B9E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x5BEA */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x5CB9 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x5CCB */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x5D11 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x5D62 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x5D74 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x5DBA */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x5DD4 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x5E35 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x5E47 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x5E8D */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x5EA7 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 15 - sub_10015F01 */
static void trig15(void) {
    /* 0x5F10 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(15)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x5F79 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x5F8B */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x5FD1 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x6022 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x6034 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x607A */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x6094 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x60F5 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x6107 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x614D */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x6167 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 16 - sub_100161C1 */
static void trig16(void) {
    /* 0x61D0 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x621C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x6268 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x62B4 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x6300 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x634C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x6398 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x63E4 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x6430 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x647C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x64C8 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x6514 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(16)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x65E6 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("L08D77", P_ARKHAN1, "SG00PRE03", NULL);
        }
        /* 0x660E */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L08D78", P_ARKHAN1, "SG00PRE03", NULL);
        }
        /* 0x6636 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L08D79", P_ARKHAN1, "SG00PRE03", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x6679 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x674B */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x675D */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D83A", P_ARKHAN1, "SG01DIA08", NULL);
                lieu_action_register("L08D83B", P_ARKHAN1, "SG01DIA01", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x67B9 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT06");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x680A */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x681C */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D84", P_ARKHAN1, "SG01DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x6862 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP02ACT01");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x68B3 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x68C5 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D85", P_ARKHAN1, "SG01DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x690B */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP03ACT01");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 17 - sub_10016955 */
static void trig17(void) {
    /* 0x6964 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x69B0 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x69FC */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x6A48 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x6A94 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x6AE0 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x6B2C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x6B78 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x6BC4 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x6C10 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x6C5C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x6CA8 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(17)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x6D7A */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x6D8C */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x6DD2 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x6E23 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x6E35 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x6E7B */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x6E95 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x6EF6 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x6F08 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x6F4E */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x6F68 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 18 - sub_10016FC2 */
static void trig18(void) {
    /* 0x6FD1 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x701D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x7069 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x70B5 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x7101 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x714D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x7199 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x71E5 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x7231 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x727D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x72C9 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x7315 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(18)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x73E7 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x73F9 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x743F */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x7490 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x74A2 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x74E8 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x7502 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x7563 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x7575 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x75BB */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x75D5 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 19 - sub_1001762F */
static void trig19(void) {
    /* 0x763E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x768A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x76D6 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x7722 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x776E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x77BA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x7806 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x7852 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x789E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x78EA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x7936 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x7982 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(19)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x7A54 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x7A66 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D83A", P_ARKHAN1, "SG01DIA08", NULL);
                lieu_action_register("L08D83B", P_ARKHAN1, "SG01DIA01", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x7AC2 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT06");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x7B13 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x7B25 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D84", P_ARKHAN1, "SG01DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x7B6B */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP02ACT01");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x7BBC */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x7BCE */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D85", P_ARKHAN1, "SG01DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x7C14 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP03ACT01");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 20 - sub_10017C5E */
static void trig20(void) {
    /* 0x7C6D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x7CB9 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x7D05 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x7D51 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x7D9D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x7DE9 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x7E35 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x7E81 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x7ECD */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x7F19 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x7F65 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x7FB1 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(20)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x8083 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("L08D86A", P_ARKHAN1, "SG01DIA05", NULL);
            lieu_action_register("L08D86B", P_ARKHAN1, "SG01DIA06", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x80DF */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L08D88", P_ARKHAN1, "SG01DIA08", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x8125 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L08D90A", P_ARKHAN1, "SG01DIA05", NULL);
            lieu_action_register("L08D90B", P_ARKHAN1, "SG01DIA08", NULL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 21 - sub_1001817A */
static void trig21(void) {
    /* 0x8189 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x81D5 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x8221 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x826D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x82B9 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x8305 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x8351 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x839D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x83E9 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x8435 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x8481 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x84CD */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x1644)) {
        /* 0x8510 */
        if (ST(L08_TRIG21_DONE) == 0) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            ST(L08_TRIG21_DONE) = 1;
            CB->pos_perso(ST(ST_HERO), 534, -923, 0x10F4, 1, "ar", 1, 0);
            /* 0x8573 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SG01PRE07");
            }
            /* 0x8590 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SG02PRE07");
            }
            /* 0x85AD */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SG03PRE07");
            }
            CB->etat_objet("OB77B", -1, -1, -1, -1, -1, -1, -3, 0);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("ar");
            CB->bloque(BLOQUE_NONE);
            return;
            return;
        }
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x862D */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x863F */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x8685 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x86D6 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x86E8 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x872E */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x8748 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x87A9 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x87BB */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x8801 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x881B */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 24 - sub_1001888B */
static void trig24(void) {
    /* 0x889A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x88E6 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x8932 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x897E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x89CA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x8A16 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x8A62 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x8AAE */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x8AFA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x8B46 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x8B92 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x8BDE */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x38E5) && ST(ST_COMBAT_STANCE) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x8C38 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->dialogue("L08D328", P_ARKHAN1);
            /* 0x8C5A */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D329", P_ARKHAN1, "SG01DIA04", NULL);
            }
            /* 0x8C82 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("L08D330", P_ARKHAN1, "SP01CAKE", NULL);
            }
            CB->animation("Sar_50");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x8CD3 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L08D331A", P_ARKHAN2);
            CB->dialogue("L08D331B", P_ARKHAN2);
            CB->animation("Sar_50");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x8D2E */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L08D332A", P_ARKHAN3);
            CB->dialogue("L08D332B", P_ARKHAN3);
            CB->animation("Sar_50");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 25 - sub_10018D82 */
static void trig25(void) {
    /* 0x8D91 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(25)) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x8E63 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x8E75 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D83A", P_ARKHAN1, "SG01DIA08", NULL);
                lieu_action_register("L08D83B", P_ARKHAN1, "SG01DIA01", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x8ED1 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT06");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x8F22 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x8F34 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D84", P_ARKHAN1, "SG01DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x8F7A */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP02ACT01");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x8FCB */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x8FDD */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L08D85", P_ARKHAN1, "SG01DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x9023 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP03ACT01");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 26 - sub_1001906D */
static void trig26(void) {
    /* 0x907C */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x90C8 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x9114 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x9160 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x91AC */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x91F8 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x9244 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x9290 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x92DC */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x9328 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x9374 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x93C0 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000)) {
        CB->select_pile(1, 1);
        /* 0x9413 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0x9485 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0x94AD */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0x94D5 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 27 - sub_10019528 */
static void trig27(void) {
    /* 0x9537 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x9583 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x95CF */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(27)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x9638 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x964A */
            if (ST(ST_SPECIAL_MODE) == 1) {
                /* 0x9664 */
                if (CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT71");
                    lieu_action_register("LCOD32", P_ARKHAN2, "P02ACT75", "SP02ACT77");
                } else {
                    CB->animation("SP02ACT70");
                    lieu_action_register("LCOD32", P_ARKHAN2, "P02ACT74", "SP02ACT76");
                }
            }
            /* 0x96C3 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x96DD */
                if (CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT73");
                    lieu_action_register("LCOD32", P_ARKHAN2, "P02ACT75", "SP02ACT79");
                } else {
                    CB->animation("SP02ACT72");
                    lieu_action_register("LCOD32", P_ARKHAN2, "P02ACT74", "SP02ACT78");
                }
            }
        }
        /* 0x973C */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x974E */
            if (ST(ST_SPECIAL_MODE) == 1) {
                /* 0x9768 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT71");
                    lieu_action_register("LCOD33", P_ARKHAN3, "P03ACT74", "SP03ACT77");
                } else {
                    CB->animation("SP03ACT70");
                    lieu_action_register("LCOD33", P_ARKHAN3, "P03ACT74", "SP03ACT76");
                }
            }
            /* 0x97C7 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x97E1 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT73");
                    lieu_action_register("LCOD33", P_ARKHAN3, "P03ACT74", "SP03ACT79");
                } else {
                    CB->animation("SP03ACT72");
                    lieu_action_register("LCOD33", P_ARKHAN3, "P03ACT74", "SP03ACT78");
                }
            }
        }
        CB->animation("ar_50");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 28 - sub_10019862 */
static void trig28(void) {
    /* 0x9871 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_GUARDIAN3)) {
        trig02();
        return;
    }
    /* 0x98BD */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_perso(P_GUARDIAN4)) {
        trig03();
        return;
    }
    /* 0x9909 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_perso(P_GUARDIAN5)) {
        trig04();
        return;
    }
    /* 0x9955 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && CB->direction_perso(P_GUARDIAN6)) {
        trig05();
        return;
    }
    /* 0x99A1 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && CB->direction_perso(P_GUARDIAN7)) {
        trig06();
        return;
    }
    /* 0x99ED */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && CB->direction_perso(P_GUARDIAN8)) {
        trig07();
        return;
    }
    /* 0x9A39 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && CB->direction_perso(P_DELIA)) {
        trig30();
        return;
    }
    /* 0x9A85 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && CB->direction_perso(P_KALHI13)) {
        trig31();
        return;
    }
    /* 0x9AD1 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && CB->direction_perso(P_KALHI33)) {
        trig32();
        return;
    }
    /* 0x9B1D */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10)) {
        trig33();
        return;
    }
    /* 0x9B69 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 34) && CB->direction_perso(34)) {
        trig34();
        return;
    }
    /* 0x9BB5 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && (CB->direction_spec(0x1C20) || (CB->divers(DV_IN_ZONE, ST(ST_HERO), 12) && CB->direction_spec(0x6590)))) {
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x9CBA */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0x9CCC */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D299", P_ARKHAN1, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x9D12 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->animation("SP01ACT32");
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x9D63 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x9D75 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D302", P_ARKHAN2, "SG00DIA07", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x9DBB */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x9DD5 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT32");
                } else {
                    CB->animation("SP02ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0x9E36 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x9E48 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L02D303", P_ARKHAN3, "SG00DIA08", NULL);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x9E8E */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0x9EA8 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT32");
                } else {
                    CB->animation("SP03ACT02");
                }
                CB->animation("ar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("ar");
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 30 - sub_10019F0D */
static void trig30(void) {
    /* 0x9F1C */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_DELIA)) {
        CB->select_pile(1, 1);
        /* 0x9FA2 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xA014 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xA03C */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xA064 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xA0BB */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12) && !CB->direction_perso(P_DELIA)) {
        trig12();
        return;
    }
    /* 0xA107 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13) && !CB->direction_perso(P_DELIA)) {
        trig13();
        return;
    }
    /* 0xA153 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 14) && !CB->direction_perso(P_DELIA)) {
        trig14();
        return;
    }
    /* 0xA19F */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 16) && !CB->direction_perso(P_DELIA)) {
        trig16();
        return;
    }
    /* 0xA1EB */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 17) && !CB->direction_perso(P_DELIA)) {
        trig17();
        return;
    }
    /* 0xA237 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 18) && !CB->direction_perso(P_DELIA)) {
        trig18();
        return;
    }
    /* 0xA283 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 19) && !CB->direction_perso(P_DELIA)) {
        trig19();
        return;
    }
    /* 0xA2CF */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 20) && !CB->direction_perso(P_DELIA)) {
        trig20();
        return;
    }
    /* 0xA31B */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 21) && !CB->direction_perso(P_DELIA)) {
        trig21();
        return;
    }
    /* 0xA367 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 24) && !CB->direction_perso(P_DELIA)) {
        trig24();
        return;
    }
    /* 0xA3B3 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_DELIA)) {
        trig26();
        return;
    }
    /* 0xA3FF */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 27) && !CB->direction_perso(P_DELIA)) {
        trig27();
        return;
    }
    /* 0xA44B */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_DELIA)) {
        /* 0xA48E */
        if (ST(ST_VIE(P_DELIA)) < 2) {
            ST(ST_SCRIPT_BUSY) = 1;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_DELIA);
            /* 0xA4CF */
            if (ST(L08_FOLLOWER_MODE) != 0x63) {
                ST(L08_FOLLOWER_MODE) = 0x14;
            }
            /* 0xA4ED */
            if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                /* 0xA542 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L08D500", P_ARKHAN2, "SG00DIA02", NULL);
                }
                /* 0xA56A */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L08D501", P_ARKHAN3, "SG00DIA02", NULL);
                }
                lieu_action_register("L08D502", P_DELIA, "SP10DIA50", "SP10ARR02_10");
                CB->animation("Sar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                /* 0xA5C7 */
                if (ST(ST_VIE(34)) == VIE_DEAD) {
                    CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                }
                /* 0xA5ED */
                if (ST(ST_VIE(34)) != VIE_DEAD && ST(L08_FOLLOWER_MODE) == 0x63) {
                    CB->joindre_perm(P_DELIA, ST(ST_HERO));
                }
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0xA63E */
            if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2)) {
                ST(ST_PLAYER_WEAPON) = ST(V_LAST_WEAPON);
                /* 0xA67B */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L08D503", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                }
                /* 0xA6A6 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L08D504", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                }
                lieu_action_register("L08D505", P_DELIA, "SP10DIA54", "SP10ARR02_10");
                CB->animation("G00PRE82");
                CB->animation("Sar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                /* 0xA711 */
                if (ST(ST_VIE(34)) == VIE_DEAD) {
                    CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                }
                /* 0xA737 */
                if (ST(ST_VIE(34)) != VIE_DEAD && ST(L08_FOLLOWER_MODE) == 0x63) {
                    CB->joindre_perm(P_DELIA, ST(ST_HERO));
                }
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0xA788 */
        if (ST(ST_VIE(P_DELIA)) == VIE_DEAD) {
            CB->fouille(P_DELIA);
            return;
        }
    }
    /* 0xA7A7 */
    if (ST(V_PLAYER_STRIKE) >= 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DELIA)) == 1 && CB->direction_perso(P_DELIA)) {
        ST(L08_FOLLOWER_MODE) = 0xA;
        ST(V_PLAYER_STRIKE) = -1;
        CB->bloque(BLOQUE_ALL);
        CB->select_pile(1, 2);
        /* 0xA830 */
        if (CB->on_joindre(P_DELIA)) {
            CB->stop_joindre(P_DELIA);
        }
        ST(V_DELIA_PHASE) = 0x14;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(V_DELIA_KILLED) = 8;
        CB->divers_empile(DV_RANDOM, 0, 0);
        CB->divers_empile(DV_RANDOM, 0, 0);
        CB->divers_empile(DV_RANDOM, 0, 0);
        CB->divers_empile(DV_RANDOM, 0, 0);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_DELIA, 0);
        CB->animation("SP10IMP51");
        CB->animation("P10cad_10");
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
}

/* trigger 31 - sub_1001A8FC */
static void trig31(void) {
    /* 0xA90B */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_KALHI13)) {
        CB->select_pile(1, 1);
        /* 0xA991 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xAA03 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xAA2B */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xAA53 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xAAAA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12) && !CB->direction_perso(P_KALHI13)) {
        trig12();
        return;
    }
    /* 0xAAF6 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13) && !CB->direction_perso(P_KALHI13)) {
        trig13();
        return;
    }
    /* 0xAB42 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 14) && !CB->direction_perso(P_KALHI13)) {
        trig14();
        return;
    }
    /* 0xAB8E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 16) && !CB->direction_perso(P_KALHI13)) {
        trig16();
        return;
    }
    /* 0xABDA */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 17) && !CB->direction_perso(P_KALHI13)) {
        trig17();
        return;
    }
    /* 0xAC26 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 18) && !CB->direction_perso(P_KALHI13)) {
        trig18();
        return;
    }
    /* 0xAC72 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 19) && !CB->direction_perso(P_KALHI13)) {
        trig19();
        return;
    }
    /* 0xACBE */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 20) && !CB->direction_perso(P_KALHI13)) {
        trig20();
        return;
    }
    /* 0xAD0A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 21) && !CB->direction_perso(P_KALHI13)) {
        trig21();
        return;
    }
    /* 0xAD56 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 24) && !CB->direction_perso(P_KALHI13)) {
        trig24();
        return;
    }
    /* 0xADA2 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_KALHI13)) {
        trig26();
        return;
    }
    /* 0xADEE */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10) && !CB->direction_perso(P_KALHI13)) {
        trig33();
        return;
    }
    /* 0xAE50 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_KALHI13) && ST(ST_VIE(P_KALHI13)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->dialogue("L08D339", P_KALHI13);
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xAF0D */
    if (ST(ST_VIE(P_KALHI13)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0xAF2E */
        if (ST(L08_KALHI33_STEP) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_KALHI13)) {
            CB->bloque(BLOQUE_ALL);
            ST(L08_KALHI33_STEP) = 1;
            /* 0xAF7C */
            if (ST(V_KALHI_PHASE) == 6) {
                ST(L08_COMPANION_REUNION) = 0;
                CB->animation("SP13LEV01");
                CB->animation("P13_65535");
            } else {
                CB->stop_joindre(-P_KALHI13);
            }
            lieu_action_register("L08D233", P_KALHI13, "SP13GAR26", "SP13GAR02");
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xAFFF */
        if (ST(L08_KALHI33_STEP) == 1 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_perso(P_KALHI13)) {
            CB->bloque(BLOQUE_ALL);
            ST(L08_KALHI33_STEP) = 0x64;
            CB->stop_joindre(-P_KALHI13);
            CB->animation("SP13GAR27");
            lieu_action_register("L08D234", P_KALHI13, "P13ARR01", "SP13ARR01_10");
            /* 0xB07C */
            if (ST(V_KALHI_PHASE) == 6) {
                ST(V_KALHI_PHASE) = 5;
                lieu_action_register("L08D335A", P_KALHI13, "P13DIA60", "SP13ARR01_10");
                lieu_action_register("L08D335B", P_KALHI13, "P13DIA63", "SP13ARR01_10");
                lieu_action_register("L08D335C", P_KALHI13, "P13DIA64", "SP13ARR01_10");
            }
            CB->modify_var(&ST(L08_KALHI33_STEP), 0);
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xB120 */
    if (CB->direction_perso(P_KALHI13)) {
        /* 0xB12F */
        if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0) {
            /* 0xB15F */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->bloque(BLOQUE_ALL);
                ST(L08_FOLLOWER_MODE) = 0x64;
                ST(L08_KALHI13_CHAT_READY) = 2;
                ST(ST_SCRIPT_BUSY) = 1;
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                CB->joindre(ST(ST_HERO), 0, 0, 0, P_KALHI13);
                /* 0xB1D6 */
                if (ST(ST_PLAYER_WEAPON) == 0 || (ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 0, 2)) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                    /* 0xB23D */
                    if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
                        /* 0xB24F */
                        if (ST(L08_D175_STAGE) == 0) {
                            ST(L08_D175_STAGE) = 1;
                            /* 0xB270 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                /* 0xB282 */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    lieu_action_register("L08D175", P_ARKHAN2, "SG02DIA04", NULL);
                                }
                                /* 0xB2AA */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    lieu_action_register("L08D176", P_ARKHAN3, "SG03DIA04", NULL);
                                }
                                lieu_action_register("L08D177", P_KALHI13, "SP13DIA55", "SP13ARR01_10");
                            }
                            /* 0xB2EB */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                /* 0xB2FD */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    lieu_action_register("L08D178", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                                /* 0xB325 */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    lieu_action_register("L08D179", P_ARKHAN3, "SG00DIA04", NULL);
                                }
                                lieu_action_register("L08D180", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                            }
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xB398 */
                        if (ST(L08_D175_STAGE) == 1) {
                            /* 0xB3AA */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L08D203", P_ARKHAN2, "SG00DIA06", NULL);
                            }
                            /* 0xB3D2 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L08D204", P_ARKHAN3, "SG00DIA06", NULL);
                            }
                            lieu_action_register("L08D205", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    } else {
                        /* 0xB44A */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L04D66", P_ARKHAN2, "SG00DIA05", NULL);
                        }
                        /* 0xB472 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L04D67", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L04D68", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xB4E5 */
                if (ST(ST_PLAYER_WEAPON) == 1) {
                    /* 0xB4F7 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L02D114A", P_ARKHAN2, "SG00DIA02", NULL);
                        CB->dialogue("L02D114B", P_ARKHAN2);
                    }
                    /* 0xB52F */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->dialogue("L02D115", P_ARKHAN3);
                    }
                    /* 0xB551 */
                    if ((ST(ST_LOCKED_CAMERA) == 3 && CB->divers(DV_DESC_POS, P_KALHI13, 3) < -710) || (ST(ST_LOCKED_CAMERA) <= 1 && CB->distance(1, 33) > 1200 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) || (ST(ST_LOCKED_CAMERA) < 2 && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD)) {
                        lieu_pre_scene(ST(ST_HERO), P_KALHI13, 1, 6, 5);
                        CB->dialogue("L02D116", P_KALHI13);
                        CB->animation("Sar");
                        CB->divers_empile(DV_SCREEN, 5, 0);
                        ST(V_EVENT15_STEP) = CB->divers(DV_DESC_POS, P_KALHI13, 0);
                        ST(L08_KALHI33_CHAT_READY) = CB->divers(DV_DESC_POS, P_KALHI13, 1);
                        ST(L08_KALHI33_POS_SAVED) = CB->divers(DV_DESC_POS, P_KALHI13, 2);
                        CB->pos_perso(P_KALHI13, 0, 0, 0, 4, "", -1, 0);
                        ST(ST_VIE(P_KALHI13)) = 0;
                        CB->animation("Sar");
                        /* 0xB6BB */
                        if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
                            CB->pos_perso(P_GUARDIAN10, 1544, -1171, 0x8097, 1, "P50ARR03", 0x21, 1);
                        }
                        CB->pos_perso(P_KALHI33, ST(V_EVENT15_STEP), ST(L08_KALHI33_CHAT_READY), ST(L08_KALHI33_POS_SAVED), 1, "P33ARR01", 0x20, 1);
                        CB->divers_empile(DV_SCREEN, 3, 0);
                        lieu_action_register("L08D213", P_KALHI33, "SP33ATT01", "SP33ARR01");
                        CB->dialogue("L08D214", P_KALHI13);
                        CB->animation("Sar_50");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L08_KALHI33_STEP), 6);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    } else if (CB->divers(DV_DESC_POS, P_KALHI13, 3) > -710) {
                        lieu_action_register("L08D219", P_KALHI13, "SP13DIA53", "SP13ARR01_20");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    } else {
                        lieu_action_register("L08D212", P_KALHI13, "SP13DIA55", "SP13ARR01_20");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xB863 */
                if (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) != 1 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    /* 0xB8CC */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L08D336", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0xB8F7 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L08D337", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    lieu_action_register("L08D338", P_KALHI13, "SP13DIA64", "SP13ARR01_10");
                    CB->animation("G00PRE82");
                    CB->animation("ar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0xB983 */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD) {
                CB->fouille(P_KALHI13);
            }
        }
        /* 0xB99D */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == P_KALHI13) {
            /* 0xB9CD */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L08D333", P_ARKHAN2);
            }
            /* 0xB9EF */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L08D334", P_ARKHAN3);
            }
        }
        /* 0xBA0E */
        if (ST(V_PLAYER_STRIKE) >= 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI13)) == 1) {
            ST(V_PLAYER_STRIKE) = -1;
            /* 0xBA4A */
            if (ST(V_PLAYER_STRIKE) == 2) {
                ST(V_PLAYER_STRIKE) = -1;
                CB->stop_joindre(-P_KALHI13);
                CB->dialogue("L04D43", P_KALHI13);
                ST(V_KALHI13_TALK_C) = 2;
                ST(V_ENGAGED_PERSONA) = P_KALHI13;
                CB->combat(P_KALHI13, 0);
                return;
            }
            CB->bloque(BLOQUE_ALL);
            CB->select_pile(1, 2);
            CB->stop_joindre(-P_KALHI13);
            ST(L08_FOLLOWER_MODE) = 0x64;
            CB->animation("SP13ESA15");
            CB->animation("P13GAR02_10");
            /* 0xBAF8 */
            if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(L08_KALHI13_GAR_STEP) == 0 && (ST(ST_LOCKED_CAMERA) == 0 || ST(ST_LOCKED_CAMERA) == 1)) {
                /* 0xBB3D */
                if (ST(V_ENGAGED_PERSONA) == 0) {
                    CB->joindre_perm(-P_GUARDIAN10, ST(ST_HERO));
                    CB->stop_joindre(P_GUARDIAN10);
                    lieu_action_register("L08D327", P_GUARDIAN10, "SP50GAR26", "SP50GAR03_50");
                    ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
                    CB->combat(P_GUARDIAN10, 0);
                } else {
                    CB->dialogue("L08D327", P_GUARDIAN10);
                }
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xBBC4 */
            if (ST(V_KALHI13_TALK_C) == 2) {
                ST(V_KALHI_PHASE) = 6;
                CB->joindre_perm(-P_KALHI13, ST(ST_HERO));
                CB->dialogue("L08D341", P_KALHI13);
                ST(V_ENGAGED_PERSONA) = P_KALHI13;
                CB->stop_joindre(P_KALHI13);
                CB->animation("Sar_10");
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_KALHI13, 0);
                return;
            }
            /* 0xBC47 */
            if (ST(V_KALHI13_TALK_C) == 1) {
                ST(V_KALHI13_TALK_C) = 2;
                CB->joindre_perm(-P_KALHI13, ST(ST_HERO));
                CB->stop_joindre(P_KALHI13);
                CB->dialogue("L08D340", P_KALHI13);
            }
            /* 0xBC99 */
            if (ST(V_KALHI13_TALK_C) == 0) {
                ST(V_KALHI13_TALK_C) = 1;
                CB->joindre_perm(-P_KALHI13, ST(ST_HERO));
                CB->stop_joindre(P_KALHI13);
                CB->dialogue("L08D339", P_KALHI13);
            }
            CB->animation("Sar_50");
            CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(1, 1);
            return;
        }
    }
}

/* trigger 32 - sub_1001BD20 */
static void trig32(void) {
    /* 0xBD2F */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_KALHI33)) {
        CB->select_pile(1, 1);
        /* 0xBDB5 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xBE27 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xBE4F */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xBE77 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xBECE */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12) && !CB->direction_perso(P_KALHI33)) {
        trig12();
        return;
    }
    /* 0xBF1A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13) && !CB->direction_perso(P_KALHI33)) {
        trig13();
        return;
    }
    /* 0xBF66 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 14) && !CB->direction_perso(P_KALHI33)) {
        trig14();
        return;
    }
    /* 0xBFB2 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 16) && !CB->direction_perso(P_KALHI33)) {
        trig16();
        return;
    }
    /* 0xBFFE */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 17) && !CB->direction_perso(P_KALHI33)) {
        trig17();
        return;
    }
    /* 0xC04A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 18) && !CB->direction_perso(P_KALHI33)) {
        trig18();
        return;
    }
    /* 0xC096 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 19) && !CB->direction_perso(P_KALHI33)) {
        trig19();
        return;
    }
    /* 0xC0E2 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 20) && !CB->direction_perso(P_KALHI33)) {
        trig20();
        return;
    }
    /* 0xC12E */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 21) && !CB->direction_perso(P_KALHI33)) {
        trig21();
        return;
    }
    /* 0xC17A */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 24) && !CB->direction_perso(P_KALHI33)) {
        trig24();
        return;
    }
    /* 0xC1C6 */
    if (ST(ST_ACTION_KEY) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && CB->direction_perso(P_GUARDIAN10) && !CB->direction_perso(P_KALHI33)) {
        trig33();
        return;
    }
    /* 0xC228 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_KALHI33) && ST(ST_VIE(P_KALHI33)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->dialogue("L08D339", P_KALHI33);
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xC2E6 */
    if (CB->direction_perso(P_KALHI33)) {
        /* 0xC2F5 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0xC325 */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->bloque(BLOQUE_ALL);
                ST(ST_SCRIPT_BUSY) = 1;
                CB->stop_joindre(-P_KALHI33);
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                CB->joindre(ST(ST_HERO), 0, 0, 0, P_KALHI33);
                /* 0xC38F */
                if (ST(ST_PLAYER_WEAPON) == 0 || (ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 0, 2)) || (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                    /* 0xC40D */
                    if ((ST(V_KALHI13_TALK_B) == 0 || ST(V_KALHI13_TALK_B) == 1) && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
                        ST(V_KALHI13_TALK_B) = 2;
                        /* 0xC452 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L08D200A", P_ARKHAN2, "SG02DIA05", NULL);
                            lieu_action_register("L08D200B", P_ARKHAN2, "SG02DIA04", NULL);
                        }
                        /* 0xC490 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L08D201", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L08D202A", P_KALHI33, "SP33DIA05", "SP33ARR01_10");
                        lieu_action_register("L08D202B", P_KALHI33, "SP33DIA56", "SP33ARR01_10");
                        /* 0xC4EF */
                        if (CB->divers(DV_CAN_FIGHT, 0, 2)) {
                            lieu_action_register("L08D202C", P_KALHI33, "SP33DIA53", "SP33ARR01_10");
                        }
                        lieu_action_register("L08D202D", P_KALHI33, "SP33DIA59", "SP33ARR01_10");
                        lieu_action_register("L08D202E", P_KALHI33, "SP33DIA59", "SP33ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                        return;
                    }
                    /* 0xC586 */
                    if (ST(V_KALHI13_TALK_B) == 2 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
                        /* 0xC5AA */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L08D203", P_ARKHAN2, "SG00DIA06", NULL);
                        }
                        /* 0xC5D2 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L08D204", P_ARKHAN3, "SG00DIA06", NULL);
                        }
                        lieu_action_register("L08D205", P_KALHI33, "SP33DIA56", "SP33ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                        return;
                    }
                    /* 0xC64D */
                    if (ST(V_KALHI13_TALK_B) == 2 || ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
                        /* 0xC671 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L08D200B", P_ARKHAN2, "SG00DIA05", NULL);
                        }
                        /* 0xC699 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L08D201", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L08D202E", P_KALHI33, "SP33DIA60", "SP33ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                        return;
                    }
                }
                /* 0xC714 */
                if (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) != 1 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    /* 0xC77D */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L08D336", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0xC7A8 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L08D337", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    lieu_action_register("L08D338", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
                    CB->animation("G00PRE82");
                    CB->animation("ar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                    return;
                }
            }
            /* 0xC83C */
            if (ST(ST_VIE(P_KALHI33)) == VIE_DEAD) {
                CB->fouille(P_KALHI33);
                return;
            }
        }
        /* 0xC85B */
        if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == P_KALHI33) {
            CB->bloque(BLOQUE_ALL);
            /* 0xC887 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L08D333", P_ARKHAN2);
            }
            /* 0xC8A9 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L08D334", P_ARKHAN3);
            }
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xC8D5 */
        if (ST(V_PLAYER_STRIKE) >= 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI33)) == 1) {
            /* 0xC905 */
            if (ST(V_PLAYER_STRIKE) == 2) {
                ST(V_PLAYER_STRIKE) = -1;
                CB->stop_joindre(-P_KALHI33);
                CB->dialogue("L04D341", P_KALHI33);
                ST(V_KALHI13_TALK_C) = 2;
                ST(V_ENGAGED_PERSONA) = P_KALHI33;
                CB->combat(P_KALHI33, 0);
                return;
            }
            CB->stop_joindre(-P_KALHI33);
            CB->bloque(BLOQUE_ALL);
            ST(V_PLAYER_STRIKE) = -1;
            /* 0xC98D */
            if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(L08_KALHI13_GAR_STEP) == 0) {
                lieu_action_register("L08D326", P_GUARDIAN10, "SP50GAR26", "SP50GAR03_50");
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
                CB->combat(P_GUARDIAN10, 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xC9F0 */
            if (ST(V_KALHI13_TALK_C) == 2) {
                ST(V_KALHI_PHASE) = 6;
                CB->joindre_perm(-P_KALHI33, ST(ST_HERO));
                CB->dialogue("L08D341", P_KALHI33);
                ST(V_ENGAGED_PERSONA) = P_KALHI33;
                CB->stop_joindre(P_KALHI33);
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_KALHI33, 0);
                return;
            }
            /* 0xCA68 */
            if (ST(V_KALHI13_TALK_C) == 1) {
                ST(V_KALHI13_TALK_C) = 2;
                CB->joindre_perm(-P_KALHI33, ST(ST_HERO));
                CB->stop_joindre(P_KALHI33);
                CB->animation("SP33ESA16");
                CB->animation("P33GAR02_10");
                CB->dialogue("L08D340", P_KALHI33);
                CB->bloque(BLOQUE_NONE);
                CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                return;
            }
            /* 0xCAF9 */
            if (ST(V_KALHI13_TALK_C) == 0) {
                ST(V_KALHI13_TALK_C) = 1;
                CB->joindre_perm(-P_KALHI33, ST(ST_HERO));
                CB->stop_joindre(P_KALHI33);
                CB->animation("SP33ESA15");
                CB->animation("P33GAR02_10");
                CB->dialogue("L08D339", P_KALHI33);
                CB->bloque(BLOQUE_NONE);
                CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                return;
            }
        }
    }
}

/* trigger 33 - sub_1001CB83 */
static void trig33(void) {
    /* 0xCB92 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(P_GUARDIAN10)) {
        CB->select_pile(1, 1);
        /* 0xCC18 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xCC8A */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xCCB2 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xCCDA */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xCD31 */
    if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI33)) == 1 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && CB->direction_perso(P_GUARDIAN10)) {
        CB->select_pile(2, 2);
        /* 0xCD8A */
        if (ST(V_KALHI_PHASE) == 5) {
            /* 0xCD9C */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                CB->stop_joindre(-P_KALHI33);
                CB->joindre_perm(P_KALHI33, ST(ST_HERO));
            }
            /* 0xCDCF */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                CB->stop_joindre(-P_KALHI13);
                CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            }
        }
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN10);
        CB->joindre(P_GUARDIAN10, 0, 0, 0, ST(ST_HERO));
        ST(L08_G10_STATE) = 0x14;
        lieu_action_register("L08D310", P_GUARDIAN10, "SP50GAR26", "SP50GAR03_50");
        CB->combat(P_GUARDIAN10, 0);
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xCE98 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        /* 0xCEE5 */
        if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 1) {
            CB->dialogue("L08D313", P_ARKHAN2);
        }
        /* 0xCF19 */
        if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 0) {
            CB->dialogue("L08D314", P_ARKHAN2);
        }
        /* 0xCF4D */
        if (ST(ST_HERO) == P_ARKHAN3 && ST(ST_SPECIAL_MODE) == 1) {
            CB->dialogue("L08D315", P_ARKHAN3);
        }
        /* 0xCF81 */
        if (ST(ST_HERO) == P_ARKHAN3 && ST(ST_SPECIAL_MODE) == 0) {
            CB->dialogue("LCOD22D", P_ARKHAN3);
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xCFBA */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
        CB->fouille(P_GUARDIAN10);
        return;
    }
}

/* trigger 34 - sub_1001D014 */
static void trig34(void) {
    /* 0xD023 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xC000) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && !CB->direction_perso(34)) {
        CB->select_pile(1, 1);
        /* 0xD0A9 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(L08_CAM_ANIM_ARMED) == 0) {
            CB->joindre(-P_KALHI33, 2316, -1008, 0xE810, -1);
        }
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        /* 0xD11B */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("LCOD52", P_ARKHAN1, "SG00SAV01", NULL);
        }
        /* 0xD143 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", NULL);
        }
        /* 0xD16B */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", NULL);
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->animation("Sar_50");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xD1C2 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(34)) != VIE_DEAD && CB->direction_perso(34)) {
        /* 0xD226 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->bloque(BLOQUE_ALL);
            lieu_action_register("LCOD20", P_ARKHAN2, "SP02ATT51", NULL);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xD266 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->bloque(BLOQUE_ALL);
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xD2A3 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(34)) == VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        CB->retire_zone(34);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xD316 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SG00PRE09");
            /* 0xD333 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                CB->dialogue("LCOD82", P_ARKHAN2);
            }
            /* 0xD355 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->dialogue("LCOD83", P_ARKHAN2);
            }
        }
        /* 0xD377 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SG00PRE09");
            /* 0xD394 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                CB->dialogue("LCOD84", P_ARKHAN3);
            }
            /* 0xD3B6 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                CB->dialogue("LCOD85", P_ARKHAN3);
            }
        }
        CB->animation("ar_50");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 41 - sub_10003DA2 */
static void trig41(void) {
    /* 0x3DB4 */
    if ((ST(L08_DELIA_ROUTE) == 1 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(V_DELIA_PHASE) == 6 || ST(V_DELIA_PHASE) == 7) && ST(ST_VIE(34)) != VIE_DEAD) {
        CB->divers_empile(DV_MUSIC, 1, 1);
    }
    /* 0x3E32 */
    if (ST(L08_DELIA_ROUTE) == 0 && ST(V_SAVED_HERO) != ST(ST_HERO) && (ST(V_DELIA_PHASE) == 7 || (ST(V_KALHI_PHASE) == 5 && ST(L08_COMPANION_ENTRY_DONE) != 0 && (ST(ST_VIE(P_KALHI13)) == 1 || ST(ST_VIE(P_KALHI33)) == 1)))) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x3ED2 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0x3EE4 */
            if (ST(V_DELIA_PHASE) == 7) {
                CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD48", P_DELIA, "SP10DIA54", "SP10ARR01_10");
            }
            /* 0x3F2C */
            if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1) {
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD49", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
            /* 0x3F86 */
            if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1) {
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD49", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
            /* 0x3FE0 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD50", P_ARKHAN2, "SG00DIA02", NULL);
            }
            /* 0x4008 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("LCOD51", P_ARKHAN2, "SG00DIA02", NULL);
            }
        }
        /* 0x4030 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0x4042 */
            if (ST(V_DELIA_PHASE) == 7) {
                CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD44", P_DELIA, "SP10DIA54", "SP10ARR01_10");
            }
            /* 0x408A */
            if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1) {
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD45", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            }
            /* 0x40E4 */
            if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1) {
                CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("LCOD45", P_KALHI33, "SP33DIA54", "SP33ARR01_10");
            }
            /* 0x413E */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD46", P_ARKHAN3, "SG00DIA08", NULL);
            }
            /* 0x4166 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("LCOD47", P_ARKHAN3, "SG00DIA08", NULL);
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x41A7 */
    if (ST(V_KALHI_PHASE) == 5 && ST(L08_COMPANION_ENTRY_DONE) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(L08_COMPANION_ENTRY_DONE) = 1;
    }
    /* 0x41EC */
    if (ST(L08_G7_DEAD_FLAG) == 1 && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN7, 0)) {
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN7;
        CB->combat(P_GUARDIAN7, 0);
    }
    /* 0x4243 */
    if ((ST(V_DELIA_PHASE) == 7 || ST(V_DELIA_PHASE) == 0x14 || ST(V_DELIA_PHASE) == 0x63) && ST(ST_VIE(34)) != VIE_DEAD && (ST(V_DELIA_KILLED) == 8 || ST(L08_CREATURE34_SEEN) == 1) && ST(V_GUARD_FIGHT_DONE) == 2) {
        CB->select_pile(1, 1);
        ST(V_ENGAGED_PERSONA) = 34;
        CB->combat(34, 0);
        CB->select_pile(0, 1);
    }
    /* 0x42F7 */
    if (ST(V_DELIA_PHASE) == 6 && ST(V_GUARD_FIGHT_DONE) == 2) {
        ST(V_DELIA_PHASE) = 7;
        ST(L08_FOLLOWER_MODE) = 0x63;
        /* 0x4333 */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
        }
        CB->joindre(ST(ST_HERO), 0, 0, 0, P_DELIA);
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        CB->change_camera(0, CAM_LOCKED);
        CB->joindre(P_DELIA, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L08D215A", P_DELIA, "SP10DIA50", "SP10DIA51");
        lieu_action_register("L08D215B", P_DELIA, "SP10DIA51", "SP10DIA51");
        lieu_action_register("L08D215C", P_DELIA, "SP10DIA54", "SP10ARR01");
        CB->change_camera(99, CAM_AUTO);
        /* 0x440D */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L08D217A", P_ARKHAN2, "SG02DIA01", NULL);
            lieu_action_register("L08D217B", P_ARKHAN2, "SP02ATT51", NULL);
        }
        /* 0x444B */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L08D218", P_ARKHAN3, "SP03ATT51", NULL);
        }
        CB->animation("SP10ARR02");
        CB->joindre_perm(P_DELIA, ST(ST_HERO));
        CB->evenement(39, 500);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x44AC */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(L08_KALHI33_D_DONE) == 0) {
        CB->select_pile(2, 1);
        CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN10, 1);
        CB->select_pile(1, 1);
        ST(L08_KALHI13_CHAT_STAGE) = 1;
        ST(L08_KALHI33_D_DONE) = 1;
        /* 0x4544 */
        if (ST(L08_FROM_ROOM2) == 1) {
            CB->joindre(P_KALHI33, -231, -1544, 0xE571, -1);
        }
        /* 0x4572 */
        if (ST(L08_FROM_ROOM2) == 2) {
            CB->joindre(P_KALHI33, -199, -1221, 0x8E3C, -1);
        }
        CB->animation("SP33ARR01_10");
        CB->bloque(BLOQUE_ALL);
        CB->joindre(P_KALHI33, -115, -1330, -1, -1);
        CB->animation("SP33ARR01_10");
        CB->joindre_perm(P_KALHI33, ST(ST_HERO));
        CB->stop_joindre(P_KALHI33);
        lieu_action_register("L08D153", P_KALHI33, "SP33DIA56", "SP33ARR01");
        CB->animation("SP33ARR01_10");
        CB->modify_var(&ST(L08_KALHI33_STEP), 1);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x4648 */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1 && ST(L08_KALHI13_ZONE_STEP) == 3 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(2, 1);
        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
        CB->select_pile(0, 1);
        return;
    }
    /* 0x46C5 */
    if (ST(V_KALHI_PHASE) == 0x64 && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(V_KALHI_PHASE) = 5;
        ST(L08_KALHI33_STEP) = 2;
        CB->select_pile(1, 1);
        CB->joindre(P_KALHI33, 0, 0, 0, ST(ST_HERO));
        CB->select_pile(0, 1);
    }
    /* 0x4741 */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || (ST(V_KALHI_PHASE) > 6 && ST(V_KALHI_PHASE) != 0xC8)) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x47D4 */
        if (ST(V_GUARDIAN4_STAGE) == 0) {
            ST(L08_G4_WAYPOINT) = 3;
            ST(L08_G3_PATROL_STEP) = 1;
            CB->evenement(1, 10000);
        }
        /* 0x4814 */
        if (ST(V_GUARDIAN4_STAGE) == 1 || ST(V_GUARDIAN4_STAGE) == 5) {
            /* 0x4838 */
            if (ST(ST_VIE(P_GUARDIAN3)) < 2 && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN3, 0)) {
                CB->combat(P_GUARDIAN3, 0);
            }
            /* 0x4871 */
            if (ST(ST_VIE(P_GUARDIAN4)) < 2 && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN4, 0)) {
                CB->combat(P_GUARDIAN4, 0);
            }
            ST(V_ENGAGED_PERSONA) = 0x10F8;
            /* 0x48B6 */
            if (ST(V_GUARDIAN4_STAGE) == 5) {
                ST(V_GUARDIAN4_STAGE) = 4;
            }
        }
        /* 0x48D7 */
        if (ST(V_GUARDIAN4_STAGE) == 2) {
            ST(L08_G4_WAYPOINT) = 9;
        }
        /* 0x48F8 */
        if (ST(V_GUARDIAN4_STAGE) == 3) {
            ST(V_GUARDIAN4_STAGE) = 4;
            ST(L08_G4_PATROL_STATE) = 0xA;
            ST(L08_G3_TALK_STAGE) = 0;
            ST(L08_G3_FIGHT_STAGE) = 0;
            CB->change_camera(0, CAM_LOCKED);
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre(-P_GUARDIAN3, 0, 0, 0, P_GUARDIAN4);
            CB->select_pile(2, 1);
            CB->joindre(-P_GUARDIAN4, 0, 0, 0, P_GUARDIAN3);
            lieu_action_register("L08D92", P_GUARDIAN3, "SP43DIA53", "SP43ARR03");
            lieu_action_register("L08D93A", P_GUARDIAN4, "SP44DIA50", "SP44ARR02_10");
            lieu_action_register("L08D93B", P_GUARDIAN4, "SP44DIA51", "SP44ARR02_10");
            lieu_action_register("L08D93C", P_GUARDIAN4, "SP44DIA54", "SP44ARR02_10");
            lieu_action_register("L08D94A", P_GUARDIAN3, "SP43DIA56", "SP43DIA54");
            lieu_action_register("L08D94B", P_GUARDIAN3, "SP43DIA54", "SP43ARR03");
            lieu_action_register("L08D95", P_GUARDIAN4, "SP44DIA56", "SP44ARR02_10");
            lieu_action_register("L08D96A", P_GUARDIAN3, "SP43DIA60", "SP43DIA57");
            lieu_action_register("L08D96B", P_GUARDIAN3, "SP43DIA57", "SP43ARR03");
            lieu_action_register("L08D97A", P_GUARDIAN4, "SP44DIA57", "SP44ARR02_10");
            CB->animation("P44ARR02_200");
            CB->change_camera(99, CAM_AUTO);
            lieu_action_register("L08D97B", P_GUARDIAN4, "P44DIA62", "SP44ARR02_10");
            CB->bloque(BLOQUE_NONE);
            CB->joindre(-P_GUARDIAN4, 1536, -1409, 0, -1);
            CB->animation("SP43ARR03_1000");
            CB->animation("P43ARR03");
            CB->joindre(-P_GUARDIAN3, 1007, -1293, 0x8000, -1);
            CB->select_pile(0, 1);
        }
    }
    /* 0x4B32 */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && ST(V_ZED84_TALK_STAGE) < 0xA && (ST(V_KALHI_PHASE) == 0 || (ST(V_KALHI_PHASE) > 6 && ST(V_KALHI_PHASE) != 0xC8)) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2 && ST(V_ZED84_TALK_STAGE) < 0xA) {
        /* 0x4BE9 */
        if (ST(L08_BONUS_ROUND) == 0xB) {
            ST(L08_BONUS_ROUND) = 1;
            CB->bloque(BLOQUE_ALL);
            lieu_action_register("L08D223", P_GUARDIAN5, "SP45DIA53", "SP45ARR03");
            /* 0x4C3F */
            if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
                CB->change_camera(3, CAM_LOCKED);
            }
            lieu_action_register("L08D224A", P_GUARDIAN6, "SP46DIA55", "SP46ARR03");
            lieu_action_register("L08D224B", P_GUARDIAN6, "SP46DIA59", "SP46ARR03");
            lieu_action_register("L08D225A", P_GUARDIAN5, "SP45DIA56", "SP45ARR03");
            lieu_action_register("L08D225B", P_GUARDIAN5, "SP45DIA60", "SP45ARR03");
            lieu_action_register("L08D226", P_GUARDIAN6, "SP46DIA57", "SP46ARR03");
            lieu_action_register("L08D227A", P_GUARDIAN5, "SP45DIA53", "SP45ARR03");
            lieu_action_register("L08D227B", P_GUARDIAN5, "SP45DIA54", "SP45ARR03");
            /* 0x4D21 */
            if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
                CB->change_camera(99, CAM_AUTO);
            }
            CB->evenement(41, 2500);
            CB->joindre(-P_GUARDIAN6, 1614, -1310, 0xF8C4, -1);
        }
        /* 0x4D6F */
        if (ST(L08_BONUS_ROUND) == 0x15 && ST(L08_FLAG_BD0) == 2) {
            ST(L08_BONUS_ROUND) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->joindre(P_GUARDIAN5, 0, 0, 0, P_GUARDIAN6);
            CB->joindre(P_GUARDIAN6, 0, 0, 0, P_GUARDIAN5);
            /* 0x4DE4 */
            if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
                CB->change_camera(3, CAM_LOCKED);
            }
            lieu_action_register("L08D316", P_GUARDIAN5, "SP45DIA53", "SP45ARR03");
            lieu_action_register("L08D317A", P_GUARDIAN6, "SP46DIA59", "SP46ARR03");
            lieu_action_register("L08D317B", P_GUARDIAN6, "SP46DIA56", "SP46ARR03");
            lieu_action_register("L08D318A", P_GUARDIAN5, "SP45DIA55", "SP45ARR03");
            lieu_action_register("L08D318B", P_GUARDIAN5, "SP45DIA60", "SP45ARR03");
            lieu_action_register("L08D319", P_GUARDIAN6, "SP46DIA57", "SP46ARR03");
            lieu_action_register("L08D320", P_GUARDIAN5, "SP45DIA51", "SP45ARR03");
            /* 0x4EC6 */
            if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
                CB->change_camera(99, CAM_AUTO);
            }
            CB->evenement(41, 2500);
            CB->joindre(-P_GUARDIAN6, 1614, -1310, 0xF8C4, -1);
        }
        /* 0x4F14 */
        if (ST(L08_BONUS_ROUND) == 0) {
            /* 0x4F26 */
            if (ST(L08_FLAG_BD0) == 2) {
                ST(L08_BONUS_ROUND) = 0xB;
                CB->bloque(BLOQUE_ALL);
                /* 0x4F63 */
                if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -700) {
                    CB->change_camera(1, CAM_LOCKED);
                } else {
                    CB->change_camera(0, CAM_LOCKED);
                }
                lieu_action_register("L08D316", P_GUARDIAN5, "SP45DIA53", "SP45ARR03");
                lieu_action_register("L08D317A", P_GUARDIAN6, "SP46DIA59", "SP46ARR03");
                lieu_action_register("L08D317B", P_GUARDIAN6, "SP46DIA54", "SP46ARR03");
                lieu_action_register("L08D318A", P_GUARDIAN5, "SP45DIA56", "SP45ARR03");
                lieu_action_register("L08D318B", P_GUARDIAN5, "SP45DIA60", "SP45ARR03");
                lieu_action_register("L08D319", P_GUARDIAN6, "SP46DIA57", "SP46ARR03");
                lieu_action_register("L08D320", P_GUARDIAN5, "SP45DIA64", "SP45ARR03");
                CB->joindre(-P_GUARDIAN6, 1614, -1310, 0xF8C4, -1);
                CB->animation("SP45ARR03");
                CB->change_camera(99, CAM_AUTO);
                CB->evenement(41, 2500);
                CB->joindre(P_GUARDIAN5, 100, -1156, -1, -1);
                CB->animation("SP45ARR03");
                CB->joindre(P_GUARDIAN5, -119, -1317, 0xC283, -1);
            } else {
                ST(L08_BONUS_ROUND) = 0x15;
                CB->bloque(BLOQUE_ALL);
                lieu_action_register("L08D223", P_GUARDIAN5, "SP45DIA53", "SP45ARR03");
                /* 0x5116 */
                if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -700) {
                    CB->change_camera(1, CAM_LOCKED);
                } else {
                    CB->change_camera(0, CAM_LOCKED);
                }
                lieu_action_register("L08D224A", P_GUARDIAN6, "SP46DIA56", "SP46ARR03");
                lieu_action_register("L08D224B", P_GUARDIAN6, "SP46DIA59", "SP46ARR03");
                lieu_action_register("L08D225A", P_GUARDIAN5, "SP45DIA55", "SP45ARR03");
                lieu_action_register("L08D225B", P_GUARDIAN5, "SP45DIA60", "SP45ARR03");
                lieu_action_register("L08D226", P_GUARDIAN6, "SP46DIA57", "SP46ARR03");
                lieu_action_register("L08D227A", P_GUARDIAN5, "SP45DIA58", "SP45ARR03");
                lieu_action_register("L08D227B", P_GUARDIAN5, "SP45DIA62", "SP45ARR03");
                CB->joindre(-P_GUARDIAN6, 1614, -1310, 0xF8C4, -1);
                CB->animation("SP45ARR03");
                CB->joindre(-P_GUARDIAN5, 448, -1372, -1, -1);
                CB->animation("SP45ARR03");
                CB->change_camera(99, CAM_AUTO);
                CB->evenement(41, 2500);
                CB->joindre(P_GUARDIAN5, 100, -1156, -1, -1);
                CB->animation("SP45ARR03");
                CB->joindre(P_GUARDIAN5, -119, -1317, 0xC283, -1);
            }
        }
        /* 0x52A4 */
        if (ST(L08_BONUS_ROUND) == 2) {
            /* 0x52B6 */
            if (ST(ST_VIE(P_GUARDIAN5)) < 2) {
                CB->combat(P_GUARDIAN5, 0);
            }
            /* 0x52D5 */
            if (ST(ST_VIE(P_GUARDIAN6)) < 2) {
                CB->combat(P_GUARDIAN6, 0);
            }
            ST(V_ENGAGED_PERSONA) = 0x11C2;
        }
    }
    /* 0x5300 */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || (ST(V_KALHI_PHASE) > 6 && ST(V_KALHI_PHASE) != 0xC8)) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_DELIA_ROUTE) == 0 && ST(L08_ARENA_UNLOCKED) != 0 && ST(V_GUARDIAN8_STAGE) == 0) {
        ST(V_GUARDIAN8_STAGE) = 1;
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        CB->change_camera(2, CAM_LOCKED);
        /* 0x53F1 */
        if (ST(ST_VIE(P_KABAK)) != VIE_DEAD) {
            lieu_action_register("L08D143", P_GUARDIAN7, "SP47DIA54", "SP47ARR04");
            lieu_action_register("L08D144A", P_GUARDIAN8, "SP48DIA54", "SP48ARR04");
        }
        lieu_action_register("L08D144B", P_GUARDIAN8, "SP48DIA57", "SP48DIA54");
        lieu_action_register("L08D144C", P_GUARDIAN8, "SP48DIA54", "SP48ARR04_50");
        lieu_action_register("L08D145A", P_GUARDIAN7, "SP47DIA57", "SP47ARR04");
        lieu_action_register("L08D145B", P_GUARDIAN7, "SP47DIA54", "SP47ARR04");
        lieu_action_register("L08D146A", P_GUARDIAN8, "SP48DIA57", "SP48ARR04");
        lieu_action_register("L08D146B", P_GUARDIAN8, "SP48DIA54", "SP48ARR04");
        CB->evenement(33, 1000);
        CB->select_pile(0, 1);
    }
    /* 0x54E8 */
    if (ST(L08_G10_STATE) == 0x14 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
        CB->combat(P_GUARDIAN10, 0);
    }
}

