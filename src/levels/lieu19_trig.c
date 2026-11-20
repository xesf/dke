/*
 * lieu19_trig.c -- Native hand-port of the LIEU19.DLL zone trigger
 * handlers (opcode 3). The entry point, INIT, tick and events live in
 * lieu19.c; this file holds the 42-slot trigger table and its handlers.
 *
 * Ported from disassembly (ImageBase 0x10000000). Each function carries
 * its original address. Handlers call the engine only through g_lieu_cb.
 */

#include <stdio.h>
#include <stdlib.h>

#include "libsys/common.h"
#include "lieu_cb.h"
#include "levels/lieu19_int.h"

static void trig03(void); static void trig10(void); static void trig11(void);
static void trig12(void); static void trig13(void); static void trig14(void);
static void trig15(void); static void trig17(void); static void trig18(void);
static void trig19(void); static void trig21(void); static void trig22(void);
static void trig23(void); static void trig41(void);
static void trig_noop(void);   /* the 27 stub slots */

/* dword_1001E720[42] -- filled at INIT with the 5-byte jmp thunks; static
 * here. A slot is "stub" when it is the empty handler shared with LIEU01.
 *   idx handler        idx handler        idx handler
 *    0  100073F0         14  1000B82B         28  1000C3AC stub
 *    1  1000945E stub    15  1000BBD4         29  1000C3B7 stub
 *    2  10009469 stub    16  1000BCE7 stub    30  1000C3C2 stub
 *    3  10009474         17  1000BCF2         31  1000C3CD stub
 *    4  1000B03E stub    18  1000BEA9         32  1000C3D8 stub
 *    5  1000B049 stub    19  1000BFBC         33  1000C3E3 stub
 *    6  1000B054 stub    20  1000C0D9 stub    34  1000C3EE stub
 *    7  1000B05F stub    21  1000C0E4         35  1000C3F9 stub
 *    8  1000B06A stub    22  1000C1CD         36  1000C404 stub
 *    9  1000B075 stub    23  1000C2AD         37  1000C40F stub
 *   10  1000B080         24  1000C380 stub    38  1000C41A stub
 *   11  1000B1D8         25  1000C38B stub    39  1000C425 stub
 *   12  1000B20E         26  1000C396 stub    40  1000C430 stub
 *   13  1000B693         27  1000C3A1 stub    41  10007EAE
 */
static void (*const g_trigger[42])(void) = {
    l19_trig00, trig_noop, trig_noop, trig03,
    trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig10, trig11,
    trig12, trig13, trig14, trig15,
    trig_noop, trig17, trig18, trig19,
    trig_noop, trig21, trig22, trig23,
    trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig41,
};

/* Op 3: run trigger handler idx (call [4*arg + 0x1001E720]). */
void l19_trigger(i32 idx) {
    if (idx >= 0 && idx < 42)
        g_trigger[idx]();
}

/* Empty slots: the stub bodies are `push ebp; mov ebp,esp; ... ret`. */
static void trig_noop(void) {
}

/* trigger 00 -- sub_100073F0 -- level setup: start points, state reset, zones, objects, actor placement */
void l19_trig00(void) {
    /* 0x7402 */
    if (ST(ST_START_POINT(19)) == 1) {
        ST(ST_HERO) = P_ARKHAN3;
        ST(V_RETURN_PART) = 1;
        /* 0x742F */
        if (ST(ST_LEVEL) != 16 && ST(ST_LEVEL) != 19 && ST(ST_LEVEL) != 48) {
            ST(ST_LEVEL) = 14;
        }
    }
    /* 0x746B */
    if (ST(ST_START_POINT(19)) == 2) {
        ST(L19_FLAG_0E3C) = 1;
        ST(ST_HERO) = P_ARKHAN3;
        ST(V_GUARD_FIGHT_DONE) = 2;
        /* 0x74A7 */
        if (ST(ST_LEVEL) != 16 && ST(ST_LEVEL) != 19 && ST(ST_LEVEL) != 48) {
            ST(ST_LEVEL) = 14;
        }
    }
    /* 0x74E3 */
    if (ST(ST_START_POINT(19)) == 3) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(L19_KALHI_MALROT_SCENE) = 5;
        /* 0x751F */
        if (ST(ST_LEVEL) != 16 && ST(ST_LEVEL) != 19 && ST(ST_LEVEL) != 48) {
            ST(ST_LEVEL) = 14;
        }
    }
    /* 0x755B */
    if (ST(ST_START_POINT(19)) == 4) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(ST_LEVEL) = 16;
        ST(V_DELIA_PHASE) = 2;
    }
    ST(ST_START_POINT(19)) = 0;
    ST(V_EVENT_0) = 0;
    ST(V_EVENT_1) = 0;
    ST(V_EVENT_2) = 0;
    CB->evenement(0, -1);
    CB->evenement(1, -1);
    CB->evenement(2, -1);
    CB->divers(DV_MUSIC, 1, 0);
    ST(L19_MALROT_WATCHING_A2) = 0;
    ST(L19_KALHI_SCENE_STARTED) = 0;
    ST(L19_KALHI_LEAVING) = 0;
    ST(L19_UNUSED_070) = 0;
    ST(L19_UNUSED_074) = 0;
    ST(L19_DELIA_FOLLOWING) = 0;
    ST(L19_DELIA_CLOSE) = 0;
    ST(L19_MALROT_CRY_STATE) = 0;
    ST(L19_DEATH_EVENT_SENT) = 0;
    ST(L19_DELIA_CRY_STATE) = 0;
    ST(V_EVENT15_STEP) = 0;
    ST(L19_MALROT_FIGHT_STARTED_A3) = 0;
    ST(ST_SCRIPT_BUSY) = 0;
    ST(V_ENGAGED_PERSONA) = 0;
    ST(V_LAST_WEAPON) = 0;
    ST(L19_UNUSED_118) = 0;
    CB->bruitage("L19AMB2", -1, -1);
    CB->ajoute_zone(10, 3627, 3146, 110, 40, 1);
    CB->ajoute_zone(11, 3907, 2307, 141, 35, 1);
    CB->ajoute_zone(13, 4106, 3020, 210, 275, 0);
    CB->ajoute_zone(14, 3917, 3085, 415, 85, 0);
    CB->ajoute_zone(15, 3841, 2601, 65, 100, 0);
    CB->ajoute_zone(16, 3719, 2600, 50, 50, 0);
    CB->ajoute_zone(17, 3307, 2993, 65, 215, 1);
    CB->ajoute_zone(18, 3191, 3084, 110, 82, 0);
    CB->ajoute_zone(19, 2449, 3377, 185, 170, 0);
    CB->ajoute_zone(20, 2429, 3407, 50, 50, 0);
    CB->ajoute_zone(21, 3301, 2930, 175, 195, 0);
    CB->ajoute_zone(22, 2363, 3240, 66, 271, 1);
    CB->ajoute_zone(23, 3309, 3084, 90, 105, 0);
    CB->ajoute_zone(24, 3310, 3156, 30, 50, 0);
    /* 0x789A */
    if (ST(ST_LEVEL) == 14) {
        CB->pos_perso(ST(ST_HERO), 3904, 2364, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
    }
    /* 0x78E8 */
    if (ST(ST_LEVEL) == 16) {
        CB->pos_perso(ST(ST_HERO), 3611, 3084, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
    }
    /* 0x7936 */
    if (ST(ST_LEVEL) == 48) {
        CB->pos_perso(ST(ST_HERO), 2479, 3236, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
    }
    /* 0x7987 */
    if (ST(ST_VIE(P_KALHI13)) != VIE_DEAD && ST(L19_KALHI_MALROT_SCENE) == 5 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
        CB->pos_perso(P_KALHI13, 3935, 3018, 0xABAF, 1, "P13ARR02", 2, 0);
        CB->pos_perso(P_MALROT5, 4006, 3023, 0x365F, 1, "P65ARR01", 3, 0);
    }
    /* 0x7A07 */
    if (ST(L19_MALROT_HOSTILE) == 1 && ST(L19_KALHI_MALROT_SCENE) != 5) {
        /* 0x7A28 */
        if (ST(ST_LEVEL) == 14) {
            CB->pos_perso(P_MALROT5, 3873, 2776, 0x8178, 1, "P65GAR02", 3, 0);
        }
        /* 0x7A5C */
        if (ST(ST_LEVEL) == 16) {
            CB->pos_perso(P_MALROT5, 3808, 2988, 0x2485, 1, "P65GAR02", 3, 0);
        }
        /* 0x7A90 */
        if (ST(ST_LEVEL) == 48) {
            CB->pos_perso(P_MALROT5, 2998, 3087, 0x1F94, 1, "P65GAR02", 3, 0);
        }
    }
    /* 0x7AC7 */
    if (ST(L19_KALHI_MALROT_SCENE) == 6 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
        CB->pos_perso(P_MALROT5, 4006, 3023, 0x365F, 1, "P65ARR01", 3, 0);
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
    }
    /* 0x7B22 */
    if (ST(V_RETURN_PART) == 1) {
        /* 0x7B34 */
        if (ST(L19_MALROT_PLACE) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(L19_MALROT_HOSTILE) == 0) {
            CB->pos_perso(P_MALROT5, 4052, 3033, 0x7439, 1, "P65ARR01", 3, 0);
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
        }
        /* 0x7BA1 */
        if (ST(L19_MALROT_PLACE) == 2 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(L19_MALROT_HOSTILE) == 0 && ST(L19_KALHI_MALROT_SCENE) != 5) {
            CB->pos_perso(P_MALROT5, 3030, 3123, 0x8100, 1, "P65ARR01", 3, 0);
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
        }
    }
    /* 0x7C20 */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(L19_KALHI_MALROT_SCENE) != 5 && ST(L19_MALROT_HOSTILE) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
        CB->pos_perso(P_MALROT5, 3030, 3123, 0x8100, 1, "P65ARR01", 3, 0);
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
    }
    /* 0x7C9F */
    if (ST(V_DELIA_PHASE) == 2 && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
        CB->pos_perso(P_DELIA, 3520, 3082, 0x95FF, 1, "P10ARR01", 0xC, 0);
    }
    /* 0x7CE8 */
    if (ST(V_DELIA_PHASE) == 3 && ST(ST_VIE(P_DELIA)) == 1) {
        CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 0xC, 0);
    }
    /* 0x7D5C */
    if (ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(V_DELIA_KILLED) == 19) {
        CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 2, "P10cad", 0xC, 0);
    }
    /* 0x7DD0 */
    if (ST(ST_VIE(P_MALROT5)) == VIE_DEAD) {
        CB->pos_perso(P_MALROT5, 0, 0, 0, 2, "P65cad", 3, 0);
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT5, 1);
    }
    /* 0x7E10 */
    if (ST(V_KONKALITE2_FIGHT) == 60) {
        ST(V_KONKALITE2_FIGHT) = 65;
    }
    /* 0x7E31 */
    if (ST(L19_ENTRY_STEP) == 1) {
        /* 0x7E43 */
        if (ST(L19_FLAG_0F78) == 0) {
            ST(L19_FLAG_0F24) = 1;
        }
        ST(L19_ENTRY_STEP) = 2;
    }
    /* 0x7E73 */
    if (ST(L19_ENTRY_STEP) == 3) {
        ST(L19_ENTRY_STEP) = 2;
        /* 0x7E94 */
        if (ST(L19_FLAG_0F78) == 0) {
            ST(L19_FLAG_0F24) = 1;
        }
    }
}

/* trigger 03 -- trig03_10009474 -- save point / Malrot conversations */
static void trig03(void) {
    /* 0x9483 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(24) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 23) && !CB->direction_perso(P_MALROT5)) {
        CB->bloque(BLOQUE_ALL);
        /* 0x94F5 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0x9520 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x956F */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_perso(P_MALROT5)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x95D8 */
        if (ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
            /* 0x95EA */
            if (ST(ST_HERO) == P_ARKHAN2 || ST(ST_HERO) == P_ARKHAN3) {
                CB->stop_joindre(-P_MALROT5);
                CB->joindre(P_MALROT5, 0, 0, 0, ST(ST_HERO));
                CB->joindre(ST(ST_HERO), 0, 0, 0, 65);
            }
            /* 0x964F */
            if (ST(ST_PLAYER_WEAPON) == 0 || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                /* 0x968D */
                if (ST(V_GUARD_FIGHT_DONE) != 2) {
                    /* 0x969F */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0x96B1 */
                        if (ST(L19_TALK_MALROT_A2) == 0) {
                            /* 0x96C3 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D09", P_ARKHAN2, "SG02DIA05", NULL);
                                lieu_action_register("L19D10A", P_MALROT5, "SP65DIA58", "SP65ARR01");
                                lieu_action_register("L19D10B", P_MALROT5, "SP65DIA54", "SP65ARR01");
                                CB->animation("ar_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                ST(L19_TALK_MALROT_A2) = 1;
                                return;
                            }
                            /* 0x9755 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D10", P_ARKHAN2, "SG02DIA03", NULL);
                                lieu_action_register("L19D11", P_MALROT5, "SP65DIA60", "SP65ARR01");
                                lieu_action_register("L19D12", P_ARKHAN2, "SG02DIA07", NULL);
                                lieu_action_register("L19D13", P_MALROT5, "SP65DIA60", "SP65ARR01");
                                CB->animation("ar_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                ST(L19_TALK_MALROT_A2) = 1;
                                return;
                            }
                        }
                        /* 0x97FD */
                        if (ST(L19_TALK_MALROT_A2) == 1) {
                            lieu_action_register("L19D15", P_ARKHAN2, "SG02DIA06", NULL);
                            lieu_action_register("L19D16A", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            lieu_action_register("L19D16B", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D16C", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            /* 0x9870 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D17A", P_ARKHAN2, "SG02DIA05", NULL);
                                lieu_action_register("L19D17B", P_ARKHAN2, "SG02DIA06", NULL);
                            }
                            /* 0x98AE */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D18", P_ARKHAN2, "SG02DIA11", NULL);
                            }
                            lieu_action_register("L19D19", P_MALROT5, "SP65DIA59", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A2) = 2;
                            return;
                        }
                        /* 0x9927 */
                        if (ST(L19_TALK_MALROT_A2) == 2) {
                            /* 0x9939 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D20A", P_ARKHAN2, "SG02DIA06", NULL);
                                lieu_action_register("L19D20B", P_ARKHAN2, "SG02DIA07", NULL);
                                lieu_action_register("L19D21A", P_MALROT5, "SP65DIA58", "SP65ARR01");
                                lieu_action_register("L19D21B", P_MALROT5, "SP65DIA60", "SP65ARR01");
                                lieu_action_register("L19D22A", P_ARKHAN2, "SG02DIA03", NULL);
                                lieu_action_register("L19D22B", P_ARKHAN2, "SG02DIA11", NULL);
                                lieu_action_register("L19D22C", P_ARKHAN2, "SG02DIA07", NULL);
                                CB->animation("ar_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                ST(L19_TALK_MALROT_A2) = 3;
                                return;
                            }
                            /* 0x9A23 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D23A", P_ARKHAN2, "SG02DIA11", NULL);
                                lieu_action_register("L19D23B", P_ARKHAN2, "SG02DIA12", NULL);
                                lieu_action_register("L19D21A", P_MALROT5, "SP65DIA58", "SP65ARR01");
                                lieu_action_register("L19D21B", P_MALROT5, "SP65DIA60", "SP65ARR01");
                                lieu_action_register("L19D24A", P_ARKHAN2, "SG02DIA06", NULL);
                                lieu_action_register("L19D24B", P_ARKHAN2, "SG02DIA06", NULL);
                                lieu_action_register("L19D24C", P_ARKHAN2, "SG02DIA07", NULL);
                                lieu_action_register("L19D24D", P_ARKHAN2, "SG02DIA09", NULL);
                                lieu_action_register("L19D24E", P_ARKHAN2, "SG02DIA05", NULL);
                                CB->animation("ar_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                ST(L19_TALK_MALROT_A2) = 3;
                                return;
                            }
                        }
                        /* 0x9B39 */
                        if (ST(L19_TALK_MALROT_A2) == 3) {
                            /* 0x9B4B */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D25", P_ARKHAN2, "SG02DIA05", NULL);
                            }
                            /* 0x9B73 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D26", P_ARKHAN2, "SG02DIA05", NULL);
                            }
                            lieu_action_register("L19D27A", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            lieu_action_register("L19D27B", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            lieu_action_register("L19D28", P_ARKHAN2, "SG02DIA02", NULL);
                            lieu_action_register("L19D29", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            /* 0x9BFC */
                            if (ST(V_ARMAL_STAGE) == 2) {
                                lieu_action_register("L19D30", P_ARKHAN2, "SG02DIA06", NULL);
                            }
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A2) = 4;
                            return;
                        }
                        /* 0x9C5C */
                        if (ST(L19_TALK_MALROT_A2) == 4 && ST(V_ARMAL_STAGE) <= 1 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D31", P_ARKHAN2, "SG02DIA05", NULL);
                            lieu_action_register("L19D32A", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            lieu_action_register("L19D32B", P_MALROT5, "SP65DIA61", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A2) = 5;
                            return;
                        }
                        /* 0x9D12 */
                        if (ST(L19_TALK_MALROT_A2) == 5 && ST(V_ARMAL_STAGE) <= 1 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D33A", P_ARKHAN2, "SG02DIA08", NULL);
                            lieu_action_register("L19D33B", P_ARKHAN2, "SG02DIA10", NULL);
                            lieu_action_register("L19D34A", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D34B", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D34C", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            lieu_action_register("L19D35", P_ARKHAN2, "SG02DIA10", NULL);
                            lieu_action_register("L19D36A", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D36B", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            lieu_action_register("L19D36C", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D36D", P_MALROT5, "SP65DIA59", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A2) = 6;
                            return;
                        }
                        /* 0x9E71 */
                        if ((ST(L19_TALK_MALROT_A2) == 6 || (ST(L19_TALK_MALROT_A2) >= 4 && ST(V_ARMAL_STAGE) >= 2)) && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D37", P_ARKHAN2, "SG02DIA05", NULL);
                            ST(V_DLG_RANDOM) = CB->random(2);
                            /* 0x9EE6 */
                            if (ST(V_DLG_RANDOM) == 0) {
                                lieu_action_register("L19D38", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            }
                            /* 0x9F11 */
                            if (ST(V_DLG_RANDOM) == 1) {
                                lieu_action_register("L19D39", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            }
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0x9F65 */
                        if (ST(L19_TALK_MALROT_A2) >= 4 && ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L19D40A", P_ARKHAN2, "SG02DIA08", NULL);
                            lieu_action_register("L19D40B", P_ARKHAN2, "SG02DIA08", NULL);
                            lieu_action_register("L19D41", P_MALROT5, "SP65DIA61", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            ST(L19_MALROT_HOSTILE) = 1;
                            CB->combat(P_MALROT5, 0);
                            ST(V_ENGAGED_PERSONA) = P_MALROT5;
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                    /* 0xA01F */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        /* 0xA031 */
                        if (ST(L19_TALK_MALROT_A3) == 0 && ST(ST_SPECIAL_MODE) == 1) {
                            /* 0xA055 */
                            if (ST(L19_MALROT_A3_STEP) == 1) {
                                CB->animation("SP65GAR27");
                                CB->animation("P65ARR01_50");
                            }
                            lieu_action_register("L19D42", P_ARKHAN3, "SG03DIA02", NULL);
                            lieu_action_register("L19D43A", P_MALROT5, "SP65DIA51", "SP65ARR01");
                            lieu_action_register("L19D43B", P_MALROT5, "SP65DIA51", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A3) = 1;
                            ST(L19_MALROT_A3_STEP) = 2;
                            return;
                        }
                        /* 0xA10C */
                        if (ST(L19_TALK_MALROT_A3) == 1 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D44", P_ARKHAN3, "SG03DIA04", NULL);
                            CB->animation("SP65GAR26");
                            lieu_action_register("L19D45A", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            lieu_action_register("L19D45B", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            lieu_action_register("L19D46A", P_ARKHAN3, "SG03DIA06", NULL);
                            lieu_action_register("L19D46B", P_ARKHAN3, "SG03DIA10", NULL);
                            lieu_action_register("L19D47", P_MALROT5, "SP65DIA61", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A3) = 2;
                            return;
                        }
                        /* 0xA200 */
                        if (ST(L19_TALK_MALROT_A3) == 2 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D48", P_ARKHAN3, "SG03DIA02", NULL);
                            lieu_action_register("L19D49A", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D49B", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A3) = 3;
                            return;
                        }
                        /* 0xA2A4 */
                        if (ST(L19_TALK_MALROT_A3) == 3 && ST(ST_SPECIAL_MODE) == 1) {
                            /* 0xA2C8 */
                            if (ST(V_ARMAL_STAGE) <= 1) {
                                lieu_action_register("L19D50", P_ARKHAN3, "SG03DIA10", NULL);
                                lieu_action_register("L19D51A", P_MALROT5, "SP65DIA54", "SP65ARR01");
                                lieu_action_register("L19D51B", P_MALROT5, "SP65DIA60", "SP65ARR01");
                                CB->animation("ar_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                ST(L19_TALK_MALROT_A3) = 4;
                                return;
                            }
                            /* 0xA35A */
                            if (ST(V_ARMAL_STAGE) >= 2) {
                                lieu_action_register("L19D52", P_ARKHAN3, "SG03DIA10", NULL);
                                lieu_action_register("L19D53A", P_MALROT5, "SP65DIA60", "SP65ARR01");
                                lieu_action_register("L19D53B", P_MALROT5, "SP65DIA61", "SP65ARR01");
                                CB->animation("ar_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                ST(L19_TALK_MALROT_A3) = 4;
                                return;
                            }
                        }
                        /* 0xA3EC */
                        if (ST(L19_TALK_MALROT_A3) == 4 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D52", P_ARKHAN3, "SG03DIA10", NULL);
                            lieu_action_register("L19D53A", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            lieu_action_register("L19D53B", P_MALROT5, "SP65DIA61", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xA481 */
                        if (ST(L19_TALK_MALROT_A3) >= 0 && ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("LCOD22A", P_ARKHAN3, "SG03DIA02", NULL);
                            /* 0xA4BB */
                            if (ST(L19_MALROT_A3_STEP) != 1) {
                                lieu_action_register("L19D54", P_MALROT5, "SP65DIA61", "SP65ARR01");
                            }
                            /* 0xA4E6 */
                            if (ST(L19_MALROT_A3_STEP) == 1) {
                                CB->dialogue("L19D54", P_MALROT5);
                            }
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            ST(L19_MALROT_HOSTILE) = 1;
                            ST(V_ENGAGED_PERSONA) = P_MALROT5;
                            CB->combat(P_MALROT5, 0);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                }
                /* 0xA559 */
                if (ST(V_GUARD_FIGHT_DONE) == 2) {
                    /* 0xA56B */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0xA57D */
                        if (ST(L19_TALK_MALROT_A2_B) == 0) {
                            /* 0xA58F */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D55", P_ARKHAN2, "SG02DIA06", NULL);
                            }
                            /* 0xA5B7 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D56", P_ARKHAN2, "SG02DIA07", NULL);
                            }
                            lieu_action_register("L19D57", P_MALROT5, "SP65DIA61", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A2_B) = 1;
                            return;
                        }
                        /* 0xA630 */
                        if (ST(L19_TALK_MALROT_A2_B) == 1) {
                            /* 0xA642 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D58", P_ARKHAN2, "SG02DIA10", NULL);
                            }
                            /* 0xA66A */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D59", P_ARKHAN2, "SG02DIA10", NULL);
                            }
                            lieu_action_register("L19D60A", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D60B", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D60C", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            lieu_action_register("L19D60D", P_MALROT5, "SP65DIA54", "SP65ARR01");
                            lieu_action_register("L19D60E", P_MALROT5, "SP65DIA59", "SP65ARR01");
                            lieu_action_register("L19D60F", P_MALROT5, "SP65DIA58", "SP65ARR01");
                            /* 0xA728 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L19D61", P_ARKHAN2, "SG02DIA05", NULL);
                            }
                            /* 0xA750 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L19D62", P_ARKHAN2, "SG02DIA08", NULL);
                            }
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A2_B) = 2;
                            return;
                        }
                        /* 0xA7B0 */
                        if (ST(L19_TALK_MALROT_A2_B) == 2) {
                            lieu_action_register("L19D63", P_ARKHAN2, "SG02DIA10", NULL);
                            lieu_action_register("L19D64", P_MALROT5, "SP65DIA60", "SP65ARR01");
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                    /* 0xA81A */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        /* 0xA82C */
                        if (ST(L19_TALK_MALROT_A3_B) == 0 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D65", P_ARKHAN3, "SG03DIA11", NULL);
                            CB->dialogue("L19D66A", P_MALROT5);
                            CB->dialogue("L19D66B", P_MALROT5);
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A3_B) = 1;
                            return;
                        }
                        /* 0xA8BE */
                        if (ST(L19_TALK_MALROT_A3_B) == 1 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D67", P_ARKHAN3, "SG03DIA05", NULL);
                            CB->dialogue("L19D68", P_MALROT5);
                            CB->dialogue("L19D43A", P_MALROT5);
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            ST(L19_TALK_MALROT_A3_B) = 2;
                            return;
                        }
                        /* 0xA950 */
                        if (ST(L19_TALK_MALROT_A3_B) == 2 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L19D69", P_ARKHAN3, "SG03DIA07", NULL);
                            CB->dialogue("L19D70", P_MALROT5);
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xA9C3 */
                        if (ST(L19_TALK_MALROT_A3_B) >= 0 && ST(ST_SPECIAL_MODE) == 0) {
                            /* 0xA9E7 */
                            if (ST(L19_MALROT_A3_STEP) != 1) {
                                CB->animation("SP65GAR26");
                            }
                            CB->animation("P65GAR02");
                            lieu_action_register("LCOD22B", P_ARKHAN3, "SG03DIA02", NULL);
                            CB->dialogue("L19D54", P_MALROT5);
                            CB->animation("ar_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            ST(L19_MALROT_HOSTILE) = 1;
                            ST(V_ENGAGED_PERSONA) = P_MALROT5;
                            CB->combat(P_MALROT5, 0);
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                }
            }
            /* 0xAA86 */
            if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                /* 0xAAC9 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    lieu_action_register("LCOD18", P_ARKHAN2, "SG02DIA05", NULL);
                    lieu_action_register("L19D97", P_MALROT5, "SP65DIA59", "SP65ARR01");
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xAB47 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    /* 0xAB59 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        lieu_action_register("LCOD19", P_ARKHAN3, "SG02DIA05", NULL);
                        lieu_action_register("L19D97", P_MALROT5, "SP65DIA59", "SP65ARR01");
                        CB->animation("Sar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xABD7 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("LCOD22C", P_ARKHAN3, "SG03DIA02", NULL);
                        lieu_action_register("L19D54", P_MALROT5, "SP65DIA51", "SP65ARR01");
                        CB->animation("ar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        ST(L19_MALROT_HOSTILE) = 1;
                        ST(V_ENGAGED_PERSONA) = P_MALROT5;
                        CB->combat(P_MALROT5, 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
            /* 0xAC69 */
            if (ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 0, 2)) {
                /* 0xAC92 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L19D99", P_ARKHAN2, "SG02DIA05", NULL);
                    lieu_action_register("L19D100", P_MALROT5, "SP65DIA59", "SP65ARR01");
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xACFC */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    /* 0xAD0E */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L19D101", P_ARKHAN3, "SG03DIA05", NULL);
                        lieu_action_register("L19D100", P_MALROT5, "SP65DIA59", "SP65ARR01");
                        CB->animation("Sar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xAD78 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("LCOD22D", P_ARKHAN3, "SG03DIA02", NULL);
                        lieu_action_register("L19D54", P_MALROT5, "SP65DIA51", "SP65ARR01");
                        CB->animation("ar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        ST(L19_MALROT_HOSTILE) = 1;
                        ST(V_ENGAGED_PERSONA) = P_MALROT5;
                        CB->combat(P_MALROT5, 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
            }
        }
        /* 0xAE0A */
        if (ST(ST_VIE(P_MALROT5)) == VIE_DEAD) {
            CB->animation("Sar_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->fouille(P_MALROT5);
            return;
        }
    }
    /* 0xAE45 */
    if (ST(V_PLAYER_STRIKE) >= 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
        CB->stop_joindre(P_MALROT5);
        ST(L19_MALROT_HOSTILE) = 1;
        ST(V_ENGAGED_PERSONA) = P_MALROT5;
        CB->combat(P_MALROT5, 0);
    }
    /* 0xAEB7 */
    if (ST(V_PLAYER_STRIKE) == 2 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
        ST(V_PLAYER_STRIKE) = -1;
        CB->stop_joindre(P_MALROT5);
        CB->divers_empile(DV_BLOOD_AT_BONE, 65, 0);
        CB->animation("SP65IMP52");
        CB->animation("P65CAD");
        CB->pos_perso(P_MALROT5, 0, 0, 0, 2, "P65CAD", 3, 0);
    }
    /* 0xAF51 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == P_MALROT5 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD && CB->direction_perso(P_MALROT5)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xAFCC */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD20", P_ARKHAN2, "SP03ATT51", NULL);
        }
        /* 0xAFF4 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD21", P_ARKHAN3, "SP03ATT51", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 10 -- trig10_1000B080 -- exit to room 16 */
static void trig10(void) {
    CB->divers(DV_MUSIC, 1, 4);
    /* 0xB0A4 */
    if (ST(V_DELIA_PHASE) == 2) {
        ST(V_DELIA_PHASE) = 3;
    }
    /* 0xB0C5 */
    if (ST(V_DELIA_PHASE) == 3 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_GUARD_FIGHT_DONE) == 2) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->stop_joindre(-P_DELIA);
        CB->dialogue("L19D93", P_ARKHAN2);
        CB->change_piece(16, -1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xB15E */
    if (ST(L19_MALROT_PLACE) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->retire_zone(10);
        CB->divers_empile(DV_SCREEN, 1, 0);
        CB->change_piece(16, -1);
        return;
    }
    ST(ST_LEVEL) = 19;
    /* 0xB1BD */
    if (ST(ST_SCRIPT_BUSY) == 0) {
        CB->change_piece(16, -1);
    }
}

/* trigger 11 -- trig11_1000B1D8 -- exit to room 14 */
static void trig11(void) {
    CB->divers(DV_MUSIC, 1, 4);
    ST(ST_LEVEL) = 19;
    CB->change_piece(14, 0);
}

/* trigger 12 -- trig12_1000B20E -- Delia interaction, hit and search */
static void trig12(void) {
    /* 0xB21D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DELIA)) == 1) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xB276 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0xB288 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L19D90", P_ARKHAN2, "SG02DIA05", NULL);
                lieu_action_register("L19D91", P_DELIA, "SP10ATT51", "SP10ARR01");
                CB->animation("Sar_20");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xB2F2 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("L19D92A", P_ARKHAN2, "SG02DIA05", NULL);
                lieu_action_register("L19D92B", P_ARKHAN2, "SG02DIA05", NULL);
                lieu_action_register("L19D91", P_DELIA, "SP10ATT51", "SP10ARR01");
                CB->animation("Sar_20");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0xB372 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L14D239", P_ARKHAN3, "SG00DIA02", NULL);
            lieu_action_register("L14D240", P_DELIA, "SP10DIA50", "SP10ARR01_10");
            CB->animation("Sar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xB3D9 */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_COMBAT_STANCE) == 1) {
        ST(V_PLAYER_STRIKE) = -1;
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_DELIA);
        CB->bloque(BLOQUE_ALL);
        /* 0xB45C */
        if (ST(V_DELIA_HITS) == 1) {
            ST(V_DELIA_KILLED) = 19;
            ST(V_DELIA_PHASE) = 20;
            ST(ST_VIE(P_DELIA)) = VIE_DEAD;
            CB->animation("SP10IMP51");
            CB->animation("P10cad");
            CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0xC, 0);
            CB->animation("Sar_10");
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xB4F2 */
        if (ST(V_DELIA_HITS) == 0) {
            ST(ST_SCRIPT_BUSY) = 1;
            ST(V_DELIA_HITS) = 1;
            CB->animation("SP10ESA01");
            CB->animation("P10ARR02_50");
            CB->dialogue("L10D95", P_DELIA);
            CB->animation("P10ARR02_50");
            CB->modify_var(&ST(L19_KALHI_LEAVING), 1);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0xB58C */
    if (ST(V_PLAYER_STRIKE) == 2 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DELIA)) == 1 && CB->direction_perso(P_DELIA) && ST(ST_COMBAT_STANCE) == 1) {
        CB->stop_joindre(-P_DELIA);
        ST(V_ENGAGED_PERSONA) = P_DELIA;
        ST(ST_SCRIPT_BUSY) = 1;
        ST(V_PLAYER_STRIKE) = -1;
        CB->combat(P_DELIA, 1);
        CB->animation("SP10ARR01_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        return;
    }
    /* 0xB64D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DELIA)) == VIE_DEAD) {
        CB->fouille(P_DELIA);
    }
}

/* trigger 13 -- trig13_1000B693 -- object OB42 / examine */
static void trig13(void) {
    /* 0xB6A2 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xD7A0)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xB70B */
        if (ST(L19_OB42_EXAMINED) == 0) {
            CB->animation("SG00PRE08");
            CB->etat_objet("OB42", -1, -1, -1, -1, -1, -1, -3, 4);
            CB->animation("ar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            ST(L19_OB42_EXAMINED) = 1;
            return;
        }
        /* 0xB77E */
        if (ST(L19_OB42_EXAMINED) == 1) {
            /* 0xB790 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L19D73", P_ARKHAN2, "SG02DIA03", NULL);
                CB->animation("ar_20");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xB7E1 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L19D74", P_ARKHAN3, "SG03DIA03", NULL);
                CB->animation("ar_20");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
}

/* trigger 14 -- trig14_1000B82B -- examine (direction_spec 1643) */
static void trig14(void) {
    /* 0xB83A */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x66B)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xB8A3 */
        if (ST(L19_MALROT_PLACE) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
            /* 0xB8CF */
            if (CB->divers(DV_CAN_FIGHT, 9, 2)) {
                /* 0xB8E1 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L19D75", P_ARKHAN2, "SP02ACT54", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xB932 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L19D76", P_ARKHAN3, "SP03ACT54", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0xB98B */
            if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                /* 0xB99D */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L19D75", P_ARKHAN2, "SP02ACT53", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xB9EE */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L19D76", P_ARKHAN3, "SP03ACT53", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0xBA3F */
        if (ST(L19_MALROT_PLACE) >= 1 || ST(V_GUARD_FIGHT_DONE) == 2) {
            /* 0xBA6B */
            if (CB->divers(DV_CAN_FIGHT, 9, 2)) {
                /* 0xBA7D */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L19D77", P_ARKHAN2, "SP02ACT54", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xBACE */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L19D78", P_ARKHAN3, "SP03ACT54", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0xBB27 */
            if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                /* 0xBB39 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L19D77", P_ARKHAN2, "SP02ACT53", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xBB8A */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L19D78", P_ARKHAN3, "SP03ACT53", NULL);
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
    }
}

/* trigger 15 -- trig15_1000BBD4 -- examine zone 16 */
static void trig15(void) {
    /* 0xBBE3 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(16)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xBC4C */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L19D79", P_ARKHAN2, "SG02DIA01", NULL);
            CB->animation("ar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xBC9D */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L19D80", P_ARKHAN3, "SG03DIA01", NULL);
            CB->animation("ar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 17 -- trig17_1000BCF2 -- Malrot warning and save point */
static void trig17(void) {
    /* 0xBD04 */
    if (ST(ST_VIE(P_MALROT5)) != VIE_DEAD && ST(L19_KALHI_MALROT_SCENE) == 0 && ST(V_GUARD_FIGHT_DONE) == 2 && (ST(ST_LEVEL) == 14 || ST(ST_LEVEL) == 16) && ST(ST_HERO) == P_ARKHAN2 && ST(L19_KALHI_LEAVING) == 0) {
        ST(L19_KALHI_LEAVING) = 1;
        CB->joindre(P_MALROT5, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L19D81", P_MALROT5, "SP65DIA51", "SP65ARR01");
        CB->joindre(P_MALROT5, 3030, 3123, 0x8100, -1);
    }
    /* 0xBDD4 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(24) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 23)) {
        CB->bloque(BLOQUE_ALL);
        /* 0xBE33 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0xBE5E */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 18 -- trig18_1000BEA9 -- examine */
static void trig18(void) {
    /* 0xBEB8 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0xF629)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xBF21 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L19D83", P_ARKHAN3, "SG02DIA02", NULL);
            CB->animation("ar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xBF72 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L19D84", P_ARKHAN3, "SG03DIA02", NULL);
            CB->animation("ar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 19 -- trig19_1000BFBC -- examine zone 20 */
static void trig19(void) {
    /* 0xBFCB */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(20)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xC034 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SG02ATT01");
            CB->dialogue("L19D85", P_ARKHAN2);
            CB->animation("Sar_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xC08A */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SG03ATT01");
            CB->dialogue("L19D86", P_ARKHAN3);
            CB->animation("Sar_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 21 -- trig21_1000C0E4 -- examine */
static void trig21(void) {
    /* 0xC0F3 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_spec(0x85D0)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->animation("SG00PRE02");
        /* 0xC167 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("LCOD10", P_ARKHAN2);
        }
        /* 0xC189 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("LCOD11", P_ARKHAN3);
        }
        CB->animation("ar_20");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 22 -- trig22_1000C1CD -- exit to room 48 */
static void trig22(void) {
    /* 0xC1DF */
    if (ST(V_DELIA_PHASE) == 2) {
        ST(V_DELIA_PHASE) = 3;
    }
    /* 0xC200 */
    if (ST(V_DELIA_PHASE) == 3 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_SAVED_HERO) = ST(ST_HERO);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->stop_joindre(-P_DELIA);
        CB->dialogue("L19D93", P_ARKHAN2);
        ST(ST_LEVEL) = 19;
        CB->change_piece(48, 0);
        CB->bloque(BLOQUE_NONE);
        CB->retire_zone(22);
        return;
    }
    ST(ST_LEVEL) = 19;
    CB->change_piece(48, 0);
}

/* trigger 23 -- trig23_1000C2AD -- save point */
static void trig23(void) {
    /* 0xC2BC */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && CB->direction_zone(24)) {
        CB->bloque(BLOQUE_ALL);
        /* 0xC30A */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD53", P_ARKHAN2, "SG00SAV01", "Sar_10");
        }
        /* 0xC335 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD54", P_ARKHAN3, "SG00SAV01", "Sar_10");
        }
        CB->animation("P00_65535");
        CB->divers_empile(DV_SAVE_SCREEN, 0, 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 41 -- trig41_10007EAE -- hero swap with Delia, Malrot fight start */
static void trig41(void) {
    CB->divers(DV_MUSIC, 1, 1);
    /* 0x7EDD */
    if (ST(V_SAVED_HERO) != ST(ST_HERO) && ST(V_DELIA_PHASE) == 3) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x7F18 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("LCOD48", P_DELIA, "SP10DIA54", "SP10ARR01_10");
            /* 0x7F60 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD50", P_ARKHAN2, "SG00DIA05", NULL);
            }
            /* 0x7F88 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("LCOD51", P_ARKHAN2, "SG00DIA05", NULL);
            }
        }
        /* 0x7FB0 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("LCOD44", P_DELIA, "SP10DIA54", "SP10ARR01_10");
            /* 0x7FF8 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD46", P_ARKHAN3, "SG00DIA05", NULL);
            }
            /* 0x8020 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("LCOD47", P_ARKHAN3, "SG00DIA05", NULL);
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8061 */
    if ((ST(V_DELIA_PHASE) == 2 || ST(V_DELIA_PHASE) == 3) && ST(L19_DELIA_FOLLOWING) == 0) {
        ST(V_DELIA_PHASE) = 3;
        ST(L19_DELIA_FOLLOWING) = 1;
    }
    /* 0x80AF */
    if (ST(L19_MALROT_HOSTILE) == 1 && ST(L19_KALHI_MALROT_SCENE) != 5 && ST(ST_VIE(P_MALROT5)) != VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = P_MALROT5;
        CB->combat(P_MALROT5, 0);
    }
}
