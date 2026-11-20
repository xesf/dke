/*
 * lieu05_trig.c — Native hand-port of the LIEU05.DLL zone trigger
 * handlers (opcode 3). The entry point, INIT, tick and events live in
 * lieu05.c; this file holds the 42-slot trigger table and its handlers.
 *
 * Ported from disassembly (ImageBase 0x10000000). Each function carries
 * its original address. Handlers call the engine only through g_lieu_cb.
 */

#include <stdio.h>
#include <stdlib.h>

#include "libsys/common.h"
#include "lieu_cb.h"
#include "levels/lieu05_int.h"

/* Global sprintf buffer of the debug overlay (dword_10024E40). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * Forward decls for the trigger table.
 * ---------------------------------------------------------------------- */
static void trig01(void);
static void trig02(void);
static void trig03(void);
static void trig04(void);
static void trig05(void);
static void trig07(void);
static void trig08(void);
static void trig10(void);
static void trig11(void);
static void trig12(void);
static void trig13(void);
static void trig14(void);
static void trig15(void);
static void trig16(void);
static void trig18(void);
static void trig41(void);
static void trig_noop(void);   /* the 25 stub slots */

/* dword_10025040[42] — filled at INIT with the 5-byte jmp thunks at
 * 0x10001000+5*i; static here. Slots 6, 9, 17 and 19..40 are the stub
 * (sub_1000EA2A and copies: push ebp; mov ebp,esp; ... ret), identical to
 * LIEU01 trig06.
 *   idx handler        idx handler        idx handler
 *    0  100073F0       14  1000F81D       28  1000FDB6 stub
 *    1  1000A2DE       15  1000F9C7       29  1000FDC1 stub
 *    2  1000A5F6       16  1000FB71       30  1000FDCC stub
 *    3  1000DA59       17  1000FC5E stub  31  1000FDD7 stub
 *    4  1000E274       18  1000FC69       32  1000FDE2 stub
 *    5  1000E2E0       19  1000FD53 stub  33  1000FDED stub
 *    6  1000EA2A stub  20  1000FD5E stub  34  1000FDF8 stub
 *    7  1000EA35       21  1000FD69 stub  35  1000FE03 stub
 *    8  1000ED4E       22  1000FD74 stub  36  1000FE0E stub
 *    9  1000EFA9 stub  23  1000FD7F stub  37  1000FE19 stub
 *   10  1000EFB4       24  1000FD8A stub  38  1000FE24 stub
 *   11  1000F161       25  1000FD95 stub  39  1000FE2F stub
 *   12  1000F3A9       26  1000FDA0 stub  40  1000FE3A stub
 *   13  1000F673       27  1000FDAB stub  41  10008462
 */
static void (*const g_trigger[42])(void) = {
    l05_trig00, trig01, trig02, trig03, trig04, trig05,
    trig_noop, trig07, trig08, trig_noop, trig10, trig11,
    trig12, trig13, trig14, trig15, trig16, trig_noop,
    trig18, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig41,
};

/* Op 3: run trigger handler idx (call [4*arg + 0x10025040]). */
void l05_trigger(i32 idx) {
    if (idx >= 0 && idx < 42)
        g_trigger[idx]();
}

/* Empty slots: the stub bodies are `push ebp; mov ebp,esp; ... ret`. */
static void trig_noop(void) {
}

/* trigger 00 — sub_100073F0 — level setup: start-point presets, state reset, zones, Bogdaran and Delia placement */
void l05_trig00(void) {
    /* 0x7402 */
    if (ST(ST_START_POINT(5)) == 1) {
        ST(ST_HERO) = P_ARKHAN1;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
    }
    /* 0x743E */
    if (ST(ST_START_POINT(5)) == 2) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 0;
        ST(L05_START_PRESET_E3C) = 1;
        ST(L05_FLAG_B08) = 0;
        ST(L05_BOGDARAN_QUEST) = 0;
        ST(V_DELIA_PHASE) = 0;
    }
    /* 0x74C5 */
    if (ST(ST_START_POINT(5)) == 3) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 1;
        ST(L05_FLAG_B08) = 0;
        ST(L05_BOGDARAN_QUEST) = 0;
        ST(V_DELIA_PHASE) = 0;
    }
    /* 0x754C */
    if (ST(ST_START_POINT(5)) == 4) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 2;
        ST(L05_FLAG_B08) = 1;
        ST(L05_BOGDARAN_QUEST) = 0;
        ST(V_DELIA_PHASE) = 0;
    }
    /* 0x75D3 */
    if (ST(ST_START_POINT(5)) == 5) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 1;
        ST(L05_FLAG_B08) = 0;
        ST(L05_BOGDARAN_QUEST) = 1;
        ST(V_DELIA_PHASE) = 0;
    }
    /* 0x7669 */
    if (ST(ST_START_POINT(5)) == 6) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 2;
        ST(L05_FLAG_B08) = 1;
        ST(L05_BOGDARAN_QUEST) = 3;
        ST(V_DELIA_PHASE) = 0;
    }
    /* 0x76F0 */
    if (ST(ST_START_POINT(5)) == 7) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 2;
        ST(L05_FLAG_B08) = 1;
        ST(L05_BOGDARAN_QUEST) = 3;
        ST(V_DELIA_PHASE) = 1;
    }
    /* 0x7777 */
    if (ST(ST_START_POINT(5)) == 8) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_DELIA)) = 1;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 2;
        ST(L05_FLAG_B08) = 1;
        ST(L05_BOGDARAN_QUEST) = 4;
        ST(V_DELIA_PHASE) = 10;
    }
    /* 0x780D */
    if (ST(ST_START_POINT(5)) == 9) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_DELIA)) = 1;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_DELIA_FIGHT_PHASE) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 2;
        ST(L05_FLAG_B08) = 1;
        ST(L05_BOGDARAN_QUEST) = 5;
        ST(V_DELIA_PHASE) = 11;
    }
    /* 0x78B2 */
    if (ST(ST_START_POINT(5)) == 10) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(L05_DUEL_MODE) = 2;
        ST(ST_VIE(P_BOGDARAN)) = VIE_DEAD;
        ST(L05_BOGDARAN_QUEST) = 4;
        ST(V_DELIA_PHASE) = 11;
        ST(V_EVENT_8) = 2;
    }
    /* 0x7939 */
    if (ST(ST_START_POINT(5)) == 11) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_LEVEL) = 45;
        ST(ST_VIE(P_BOGDARAN)) = 1;
        ST(L05_BOGDARAN_STAGE) = 1;
        ST(L05_START_PRESET_E3C) = 2;
        ST(L05_FLAG_B08) = 1;
        ST(L05_BOGDARAN_QUEST) = 4;
    }
    ST(L05_BOGDARAN_TALKING) = 0;
    ST(L05_BOGDARAN_RETURN) = 0;
    ST(L05_BOGDARAN_MET) = 0;
    ST(L05_UNUSED_70) = 0;
    ST(L05_D65_SAID) = 0;
    ST(L05_UNUSED_78) = 0;
    ST(L05_TRIG3_TALK_STAGE) = 0;
    ST(L05_QUIP_PICK) = 0;
    ST(L05_WEAPON_GIVE_STEP) = 0;
    ST(L05_D13_STEP) = 0;
    ST(V_EVENT15_STEP) = 0;
    ST(L05_UNUSED_90) = 0;
    ST(L05_DELIA_FOLLOW_STEP) = 0;
    ST(L05_D151_STAGE) = 0;
    ST(L05_DELIA_D21_STEP) = 0;
    ST(L05_UNUSED_BC) = 0;
    ST(L05_D201_SAID) = 0;
    ST(L05_BOGDARAN_LEFT) = 0;
    ST(L05_FIRST_TALK_DONE) = 0;
    ST(L05_UNUSED_E0) = 0;
    ST(L05_TALK_IN_FIGHT_OK) = 0;
    ST(V_WEAPON_WARNED) = 0;
    ST(L05_BOGDARAN_FIGHT_STEP) = 0;
    ST(L05_D150_ARMED) = 0;
    ST(L05_BOGDARAN_FIGHT_STEP) = 1;
    ST(V_EVENT_0) = 0;
    ST(V_EVENT_1) = 0;
    ST(V_EVENT_33) = 0;
    ST(V_END_GAME_REQ) = 0;
    ST(V_ENGAGED_PERSONA) = 0;
    ST(ST_SCRIPT_BUSY) = 0;
    ST(V_LAST_WEAPON) = 0;
    CB->ajoute_zone(3, -2157, 89, 120, 84, 0);
    CB->ajoute_zone(4, -2280, -170, 120, 50, 1);
    CB->ajoute_zone(7, -2054, -330, 125, 90, 0);
    CB->ajoute_zone(8, -1874, -330, 230, 90, 0);
    CB->ajoute_zone(9, -2182, 186, 60, 60, 0);
    CB->ajoute_zone(10, -2230, -300, 90, 75, 0);
    CB->ajoute_zone(12, -2152, -302, 70, 80, 0);
    CB->ajoute_zone(13, -1890, -160, 95, 70, 0);
    CB->ajoute_zone(14, -1885, 35, 100, 80, 0);
    CB->ajoute_zone(15, -1890, -70, 95, 70, 0);
    CB->ajoute_zone(16, -2075, 100, 130, 120, 0);
    CB->ajoute_zone(17, -2025, 200, 40, 40, 0);
    CB->ajoute_zone(18, -2000, 35, 125, 70, 0);
    CB->bruitage("L05AMB1", -1, -1);
    CB->evenement(33, -1);
    /* 0x7CDF */
    if (ST(ST_LEVEL) == 45) {
        /* 0x7CF1 */
        if (ST(V_DELIA_PHASE) != 10) {
            CB->pos_perso(ST(ST_HERO), -2200, -225, 0xC12A, 1, "ar", 6, 0);
        }
        /* 0x7D2E */
        if (ST(V_DELIA_PHASE) == 10) {
            CB->pos_perso(ST(ST_HERO), -2172, -300, 0xDB56, 1, "ar", 6, 0);
        }
    }
    /* 0x7D6B */
    if (ST(ST_VIE(P_BOGDARAN)) != VIE_DEAD) {
        /* 0x7D7D */
        if (ST(L05_BOGDARAN_STAGE) == 0) {
            CB->ajoute_zone(1, -2007, -107, 90, 90, 0);
            CB->pos_perso(P_BOGDARAN, -1893, -60, 0xC738, 1, "P08ATT02", 2, 0);
            CB->link_objet(8, "OB88", 0x1B, 1);
        }
        /* 0x7DDD */
        if (ST(L05_BOGDARAN_STAGE) == 1) {
            /* 0x7DEF */
            if (ST(V_DELIA_PHASE) < 10 || ST(V_DELIA_PHASE) == 20) {
                CB->pos_perso(P_BOGDARAN, -2005, -109, 0x6590, 1, "P08ATT02", 2, 0);
                CB->link_objet(8, "OB88", 0x1B, 1);
            }
            /* 0x7E49 */
            if (ST(V_DELIA_PHASE) >= 10 && ST(V_DELIA_PHASE) != 20) {
                /* 0x7E6D */
                if (ST(L05_DELIA_FIGHT_PHASE) == 0) {
                    CB->pos_perso(P_BOGDARAN, -2005, -109, 0x6590, 1, "P08ATT02", 2, 0);
                    /* 0x7EA1 */
                    if (ST(L05_BOGDARAN_QUEST) <= 1) {
                        CB->link_objet(8, "OB88", 0x1B, 1);
                    }
                    /* 0x7EC7 */
                    if (ST(L05_BOGDARAN_QUEST) == 2) {
                        CB->link_objet(8, "OB41", 0x1B, 1);
                    }
                    /* 0x7EED */
                    if (ST(L05_BOGDARAN_QUEST) >= 3) {
                        CB->link_objet(8, "OB40", 0x1B, 1);
                        CB->divers(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE41);
                    }
                }
                /* 0x7F25 */
                if (ST(L05_DELIA_FIGHT_PHASE) >= 1) {
                    ST(L05_BOGDARAN_LEFT) = 1;
                    CB->pos_perso(P_BOGDARAN, -1995, 30, 0xBA54, 1, "P08ATT04", 2, 0);
                }
            }
        }
        /* 0x7F68 */
        if (ST(L05_BOGDARAN_STAGE) == 10 || ST(L05_BOGDARAN_STAGE) == 11) {
            ST(L05_BOGDARAN_STAGE) = 11;
            CB->pos_perso(P_BOGDARAN, -1890, -160, 0x6590, 1, "P08ARR03", 2, 0);
        }
    }
    /* 0x7FC0 */
    if (ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD) {
        CB->ajoute_zone(1, -2007, -107, 90, 90, 0);
        CB->pos_perso(P_BOGDARAN, CB->divers(DV_DESC_POS, P_BOGDARAN, 0), CB->divers(DV_DESC_POS, P_BOGDARAN, 1), CB->divers(DV_DESC_POS, P_BOGDARAN, 2), 2, "P08cad", 2, 0);
    }
    /* 0x803A */
    if (ST(V_DELIA_PHASE) == 10) {
        CB->pos_perso(P_DELIA, -2170, -180, 0xCF08, 1, "P10ARR01", 5, 0);
        /* 0x8071 */
        if (ST(L05_BOGDARAN_QUEST) != 10) {
            ST(L05_BOGDARAN_QUEST) = 4;
        }
        ST(V_EVENT_8) = 2;
        CB->evenement(8, -1);
    }
    /* 0x80B1 */
    if (ST(V_DELIA_PHASE) == 11 && ST(L05_BOGDARAN_STAGE) < 10 && ST(ST_VIE(P_DELIA)) == 1) {
        /* 0x80E7 */
        if (ST(L05_DELIA_FIGHT_PHASE) == 0) {
            ST(V_EVENT_8) = 2;
            CB->pos_perso(P_DELIA, -1952, 36, 0x43F8, 1, "P10ARR01", 5, 0);
            CB->evenement(8, -1);
        }
        /* 0x813A */
        if (ST(L05_DELIA_FIGHT_PHASE) >= 1) {
            ST(L05_BOGDARAN_LEFT) = 1;
            ST(L05_TALK_IN_FIGHT_OK) = 1;
            CB->pos_perso(P_DELIA, -1952, 18, 0x36B0, 1, "P10PEL02", 5, 0);
        }
    }
    /* 0x818C */
    if (ST(V_DELIA_PHASE) == 11 && (ST(L05_BOGDARAN_STAGE) == 10 || ST(L05_BOGDARAN_STAGE) == 11)) {
        ST(L05_BOGDARAN_STAGE) = 11;
        /* 0x81D1 */
        if (ST(ST_VIE(P_DELIA)) == 1) {
            CB->pos_perso(P_DELIA, -2170, 12, 0x7D00, 1, "P10ARR01", 5, 0);
        }
    }
    /* 0x8205 */
    if (ST(L05_BOGDARAN_QUEST) == 3) {
        CB->divers_empile(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE41);
        CB->etat_objet("OB41", -2095, 500, -93, 0x5A00, 0x23D0, 0x4200, -1, 0);
        CB->ajoute_zone(11, -2083, -205, 90, 80, 0);
        CB->divers(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE41);
    }
    /* 0x8280 */
    if (ST(L05_OB16_STATE) != 0 && ST(L05_OB16_STATE) != 2 && ST(ST_HERO) > 1 && ST(V_GUARD_FIGHT_DONE) >= 2) {
        ST(L05_OB16_STATE) = 0;
    }
    /* 0x82D7 */
    if (ST(L05_OB16_STATE) == 0 && ST(ST_HERO) > 1 && ST(V_GUARD_FIGHT_DONE) < 2) {
        CB->etat_objet("OB16", -2094, 531, -334, 0, 0, 0, -1, 3);
    }
    /* 0x8331 */
    if (ST(L05_OB16_STATE) == 0 && ST(ST_HERO) > 1 && ST(V_GUARD_FIGHT_DONE) >= 2) {
        CB->etat_objet("OB06B", -2094, 531, -334, 0, 0, 0, -1, 3);
    }
    ST(ST_START_POINT(5)) = 99;
    CB->divers(DV_MUSIC, 1, 0);
    /* 0x83AC */
    if (ST(L05_DUEL_MODE) == 1 && ST(ST_VIE(P_DELIA)) == 1) {
        CB->pos_perso(P_DELIA, -2150, -290, 0x1DB0, 1, "P10ARR01", 5, 0);
    }
    /* 0x83F5 */
    if (ST(L05_DUEL_MODE) == 2 && ST(ST_VIE(P_DELIA)) == VIE_DEAD) {
        CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 2, "P10cad", 5, 0);
    }
}

/* trigger 01 — sub_1000A2DE (thunk 0x10001069) */
static void trig01(void) {
    /* 0xA2ED */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && CB->direction_perso(P_BOGDARAN)) {
        trig02();
        return;
    }
    /* 0xA348 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
        /* 0xA36A */
        if (CB->direction_spec(0x32C8) || CB->direction_spec(0x59D8) || CB->direction_spec(0x80E8)) {
            CB->bloque(BLOQUE_ALL);
            CB->print_msg("acton ln900", 0, 0xF0);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->stop_joindre(-P_BOGDARAN);
            /* 0xA3D6 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->evenement(2, -1);
            }
            /* 0xA3F8 */
            if (ST(L05_D22_STAGE) == 0) {
                ST(L05_D22_STAGE) = 1;
                CB->animation("SG00PRE03");
                /* 0xA424 */
                if (ST(ST_VIE(P_BOGDARAN)) < 2 && ST(ST_HERO) == P_ARKHAN1) {
                    lieu_action_register("L05D22", P_BOGDARAN, "SP08DIA59", "SP08ARR03_10");
                }
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                /* 0xA47D */
                if (ST(ST_VIE(P_BOGDARAN)) < 2) {
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                }
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xA4B2 */
            if (ST(L05_D22_STAGE) == 1) {
                ST(L05_D22_STAGE) = 2;
                CB->animation("SG00PRE03");
                /* 0xA4DE */
                if (ST(ST_VIE(P_BOGDARAN)) < 2 && ST(ST_HERO) == P_ARKHAN1) {
                    lieu_action_register("L05D23A", P_BOGDARAN, "SP08DIA60", "SP08ARR03_10");
                    lieu_action_register("L05D23B", P_BOGDARAN, "SP08DIA59", "SP08ARR03_10");
                }
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                /* 0xA550 */
                if (ST(ST_VIE(P_BOGDARAN)) < 2) {
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                }
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xA585 */
            if (ST(L05_D22_STAGE) == 2) {
                CB->animation("SG00PRE03");
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                /* 0xA5BE */
                if (ST(ST_VIE(P_BOGDARAN)) < 2) {
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                }
                CB->bloque(BLOQUE_NONE);
                return;
            }
            return;
        }
        trig02();
    }
}

/* trigger 02 — sub_1000A5F6 (thunk 0x100010BE) */
static void trig02(void) {
    /* 0xA608 */
    if (ST(L05_DUEL_MODE) == 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1) {
        CB->print_msg("combat on", 0, 0xF5);
        CB->combat(P_BOGDARAN, 0);
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
    }
    /* 0xA663 */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(5) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5)) {
        trig05();
    }
    /* 0xA6DB */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 11)) {
        trig11();
    }
    /* 0xA740 */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xC350) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13)) {
        trig13();
    }
    /* 0xA7B8 */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xDEA8) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 14)) {
        trig14();
    }
    /* 0xA830 */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xC350) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 15)) {
        trig15();
    }
    /* 0xA8A8 */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(17) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 16)) {
        trig16();
    }
    /* 0xA920 */
    if (ST(ST_ACTION_KEY) == 1 && !CB->direction_perso(P_BOGDARAN) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 18)) {
        trig18();
    }
    /* 0xA995 */
    if (ST(ST_ACTION_KEY) == 1 && (ST(V_ENGAGED_PERSONA) == 0 || ST(V_ENGAGED_PERSONA) == P_DELIA) && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_perso(P_BOGDARAN) && ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD) {
        /* 0xA9F9 */
        if (ST(L05_BOGDARAN_QUEST) == 2) {
            ST(L05_BOGDARAN_QUEST) = 10;
            CB->divers_empile(DV_PERSONA_ITEMS, P_BOGDARAN, OB_OBJ_BEFORE41);
        }
        /* 0xAA2C */
        if (ST(L05_BOGDARAN_QUEST) == 4) {
            ST(L05_BOGDARAN_QUEST) = 10;
            CB->divers_empile(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE41);
            CB->divers_empile(DV_PERSONA_ITEMS, P_BOGDARAN, OB_OBJ_BEFORE40);
        }
        CB->fouille(P_BOGDARAN);
        return;
    }
    /* 0xAA7E */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_perso(P_BOGDARAN) && ST(ST_VIE(P_BOGDARAN)) == 1) {
        CB->print_msg("acton ln1002", 0, 0xF0);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->evenement(2, -1);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_BOGDARAN);
        CB->stop_joindre(-P_BOGDARAN);
        /* 0xAB34 */
        if (ST(ST_PLAYER_WEAPON) == 0 || ST(ST_PLAYER_WEAPON) == 40 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON)) || (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 0)) {
            /* 0xABFF */
            if (ST(ST_HERO) == P_ARKHAN1) {
                /* 0xAC11 */
                if (ST(L05_BOGDARAN_D13_STATE) == 2) {
                    /* 0xAC23 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L05D24A", P_ARKHAN1, "SG01DIA05", "Sar_10");
                        lieu_action_register("L05D24B", P_ARKHAN1, "SP01ATT70", "Sar_10");
                    }
                    /* 0xAC67 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L05D25", P_ARKHAN1, "SG00DIA05", "Sar_10");
                    }
                    lieu_action_register("L05D26", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                }
                /* 0xACDA */
                if (ST(L05_BOGDARAN_D13_STATE) == 1) {
                    /* 0xACEC */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L05D27", P_ARKHAN1, "SG01DIA01", "Sar_10");
                    }
                    /* 0xAD17 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L05D28", P_ARKHAN1, "SG01DIA01", "Sar_10");
                    }
                    CB->modify_var(&ST(L05_D13_STEP), 1);
                }
                /* 0xAD59 */
                if (ST(L05_BOGDARAN_D13_STATE) == 0) {
                    ST(L05_BOGDARAN_D13_STATE) = 1;
                    /* 0xAD7A */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L05D29", P_ARKHAN1, "SP01ATT75", "Sar_10");
                    }
                    /* 0xADA5 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L05D30", P_ARKHAN1, "SP01ATT74", "Sar_10");
                    }
                    lieu_action_register("L05D31", P_BOGDARAN, "SP08DIA05", "SP08ARR03_10");
                    /* 0xADE9 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L05D32", P_ARKHAN1, "SG00DIA01", "Sar_10");
                    }
                    /* 0xAE14 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L05D33", P_ARKHAN1, "SG00DIA01", "Sar_10");
                    }
                    lieu_action_register("L05D34", P_BOGDARAN, "SP08DIA64", "SP08ARR03_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                }
                return;
            }
            /* 0xAE8C */
            if (ST(ST_HERO) >= 2) {
                /* 0xAE9E */
                if (ST(L05_BOGDARAN_STAGE) == 0) {
                    CB->evenement(2, -1);
                    CB->stop_joindre(P_BOGDARAN);
                    CB->animation("SP08ARR02");
                    /* 0xAED6 */
                    if (ST(L05_BOGDARAN_TOGGLE) == 0) {
                        ST(L05_BOGDARAN_TOGGLE) = 1;
                        /* 0xAEFE */
                        if (CB->distance(2, 6) < 100) {
                            CB->animation("SP08MAR29");
                        }
                        /* 0xAF1B */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0xAF2D */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D35", P_ARKHAN2, "SG00DIA03", "Sar_10");
                            }
                            /* 0xAF58 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D36", P_ARKHAN2, "SG00DIA03", "Sar_10");
                            }
                        }
                        /* 0xAF83 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0xAF95 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D37", P_ARKHAN3, "SG00DIA03", "Sar_10");
                            }
                            /* 0xAFC0 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("LCOD22A", P_ARKHAN3, "SG00DIA03", "Sar_10");
                            }
                        }
                        lieu_action_register("L05D39A", P_BOGDARAN, "SP08DIA01", "SP08DIA02");
                        lieu_action_register("L05D39B", P_BOGDARAN, "SP08DIA64", "SP08ARR02_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB051 */
                    if (ST(L05_BOGDARAN_TOGGLE) == 1) {
                        ST(L05_BOGDARAN_TOGGLE) = 0;
                        /* 0xB079 */
                        if (CB->distance(2, 6) < 100) {
                            CB->animation("SP08MAR29");
                        }
                        /* 0xB096 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0xB0A8 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D40", P_ARKHAN2, "SG00DIA03", "Sar_10");
                            }
                            /* 0xB0D3 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D41", P_ARKHAN2, "SG00DIA03", "Sar_10");
                            }
                        }
                        /* 0xB0FE */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0xB110 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D42", P_ARKHAN3, "SG00DIA03", "Sar_10");
                            }
                            /* 0xB13B */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("LCOD22B", P_ARKHAN3, "SG00DIA03", "Sar_10");
                            }
                        }
                        lieu_action_register("L05D44A", P_BOGDARAN, "SP08DIA05", "SP08DIA01");
                        lieu_action_register("L05D44B", P_BOGDARAN, "SP08DIA01", "SP08ARR02_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                }
                /* 0xB1CC */
                if (ST(L05_BOGDARAN_STAGE) == 1) {
                    /* 0xB1DE */
                    if (ST(L05_D45_STAGE) == 0 && ST(V_GUARD_FIGHT_DONE) != 0) {
                        ST(L05_D45_STAGE) = 1;
                        /* 0xB211 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D45A", P_ARKHAN2, "SG00DIA08", "Sar_10");
                            lieu_action_register("L05D45B", P_ARKHAN2, "SG00DIA08", "Sar_10");
                        }
                        /* 0xB255 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D46", P_ARKHAN3, "SG00DIA08", "Sar_10");
                        }
                        lieu_action_register("L05D47A", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                        lieu_action_register("L05D47B", P_BOGDARAN, "SP08DIA59", "SP08ARR03_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB2E6 */
                    if (ST(L05_D48_SAID) == 0 && ST(L05_START_PRESET_E3C) == 1 && ST(L05_FLAG_E40) == 0) {
                        ST(L05_D48_SAID) = 1;
                        /* 0xB32B */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D48", P_ARKHAN2, "SG00DIA08", "Sar_10");
                        }
                        /* 0xB356 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D49", P_ARKHAN3, "SG00DIA08", "Sar_10");
                        }
                        lieu_action_register("L05D50A", P_BOGDARAN, "SP08DIA55", "SP08ARR03_10");
                        lieu_action_register("L05D50B", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                        lieu_action_register("L05D50C", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                        lieu_action_register("L05D50D", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB419 */
                    if (ST(L05_D53_SAID) == 0 && ST(V_GUARD_FIGHT_DONE) != 0 && ST(V_CRYSTAL_KEY_STAGE) == 1 && ST(V_CRYSTAL_PROGRESS) < 3) {
                        ST(L05_D53_SAID) = 1;
                        /* 0xB470 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D53A", P_ARKHAN2, "SG00DIA08", "Sar_10");
                            lieu_action_register("L05D53B", P_ARKHAN2, "SG00DIA08", "Sar_10");
                        }
                        /* 0xB4B4 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D54", P_ARKHAN3, "SG00DIA08", "Sar_10");
                        }
                        lieu_action_register("L05D55A", P_BOGDARAN, "SP08DIA03", "SP08ARR03_10");
                        lieu_action_register("L05D55B", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB545 */
                    if (ST(L05_D56_SAID) == 0 && ST(ST_VIE(P_THANANDAR24)) < 2) {
                        ST(L05_D56_SAID) = 1;
                        /* 0xB578 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D56", P_ARKHAN2, "SG00DIA08", "Sar_10");
                        }
                        /* 0xB5A3 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D57", P_ARKHAN3, "SG00DIA08", "Sar_10");
                        }
                        lieu_action_register("L05D58A", P_BOGDARAN, "SP08DIA59", "SP08ARR03_10");
                        lieu_action_register("L05D58B", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
                        lieu_action_register("L05D58C", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB64D */
                    if (ST(L05_D59_SAID) == 0 && ST(L05_FLAG_EB0) != 0 && ST(V_EVENT_3) == 0 && ST(V_EVENT_4) == 0) {
                        ST(L05_D59_SAID) = 1;
                        /* 0xB6A4 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D59", P_ARKHAN2, "SG00DIA08", "Sar_10");
                        }
                        /* 0xB6CF */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D60", P_ARKHAN3, "SG00DIA08", "Sar_10");
                        }
                        lieu_action_register("L05D61", P_BOGDARAN, "SP08DIA03", "SP08ARR03_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB747 */
                    if (ST(L05_D62_SAID) == 0 && ST(V_EVENT_3) == 0 && ST(V_EVENT_4) == 0) {
                        ST(L05_D62_SAID) = 1;
                        /* 0xB78C */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D62", P_ARKHAN2, "SG00DIA08", "Sar_10");
                        }
                        /* 0xB7B7 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D63", P_ARKHAN3, "SG00DIA08", "Sar_10");
                        }
                        lieu_action_register("L05D64A", P_BOGDARAN, "SP08DIA57", "SP08ARR03_10");
                        lieu_action_register("L05D64B", P_BOGDARAN, "SP08DIA60", "SP08ARR03_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB848 */
                    if (ST(L05_BOGDARAN_QUEST) == 2 && ST(L05_D65_SAID) == 0) {
                        ST(L05_D65_SAID) = 1;
                        /* 0xB875 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            /* 0xB887 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D65", P_ARKHAN2, "SG00DIA08", "Sar_10");
                            }
                            /* 0xB8B2 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D66", P_ARKHAN3, "SG00DIA08", "Sar_10");
                            }
                            lieu_action_register("L05D67", P_BOGDARAN, "SP08DIA55", "SP08ARR03_2000");
                        }
                        /* 0xB8F6 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            /* 0xB908 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D209", P_ARKHAN2, "SG00DIA08", "Sar_10");
                            }
                            /* 0xB933 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D210", P_ARKHAN3, "SG00DIA08", "Sar_10");
                            }
                            lieu_action_register("L05D211A", P_BOGDARAN, "SP08DIA55", "SP08ARR03_2000");
                        }
                        lieu_action_register("L05D15", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xB9C4 */
                    if ((ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) == 20) && ST(L05_BOGDARAN_QUEST) == 3 && ST(L05_D72_STAGE) == 0) {
                        ST(L05_D72_STAGE) = 1;
                        /* 0xBA1B */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L05D68", P_ARKHAN2, "SG00DIA02", NULL);
                        }
                        /* 0xBA43 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D210", P_ARKHAN3, "SG00DIA02", NULL);
                        }
                        lieu_action_register("L05D70A", P_BOGDARAN, "SP08DIA03", "SP08ARR03_20");
                        lieu_action_register("L05D70B", P_BOGDARAN, "SP08DIA53", "SP08ARR03_20");
                        lieu_action_register("L05D70C", P_BOGDARAN, "SP08DIA54", "SP08ARR03_20");
                        lieu_action_register("L05D70D", P_BOGDARAN, "SP08DIA55", "SP08ARR03_20");
                        lieu_action_register("L05D70E", P_BOGDARAN, "SP08DIA53", "SP08ARR03_20");
                        lieu_action_register("L05D70F", P_BOGDARAN, "SP08DIA54", "SP08ARR03_20");
                        /* 0xBB01 */
                        if (ST(ST_VIE(P_DELIA)) < 2 || ST(V_DELIA_PHASE) == 0) {
                            ST(V_DELIA_PHASE) = 1;
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xBB68 */
                        if (ST(ST_VIE(P_DELIA)) >= 2 && ST(L05_D72_STAGE) == 1) {
                            ST(L05_D72_STAGE) = 2;
                            ST(L05_D45_STAGE) = 1;
                            /* 0xBBAA */
                            if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 1 && ST(V_DELIA_PHASE) == 20) {
                                lieu_action_register("L05D72", P_ARKHAN2, "SG00DIA06", "Sar_10");
                            }
                            /* 0xBBF9 */
                            if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D73", P_ARKHAN2, "SG00DIA06", "Sar_10");
                                lieu_action_register("L05D74", P_BOGDARAN, "SP08DIA53", "SP08ARR03_20");
                                lieu_action_register("L05D75", P_ARKHAN2, "SG00DIA01", "Sar_10");
                            }
                            /* 0xBC68 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D76", P_ARKHAN3, "SG00DIA01", "Sar_10");
                            }
                            lieu_action_register("L05D77A", P_BOGDARAN, "SP08DIA63", "SP08ARR03_20");
                            lieu_action_register("L05D77B", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
                            CB->animation("SG00ATT01");
                            lieu_action_register("L05D77C", P_BOGDARAN, "SP08DIA61", "SP08ARR03_20");
                            lieu_action_register("L05D77D", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
                            ST(V_DELIA_PHASE) = 99;
                            CB->bloque(BLOQUE_NONE);
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
                            ST(L05_BOGDARAN_STAGE) = 10;
                            CB->combat(P_BOGDARAN, 0);
                            return;
                        }
                    }
                    /* 0xBD57 */
                    if (ST(V_DELIA_PHASE) != 0 && ST(V_DELIA_PHASE) < 11 && ST(L05_D65_SAID) == 0) {
                        ST(L05_D65_SAID) = 1;
                        /* 0xBD96 */
                        if (ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
                            /* 0xBDA8 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D78", P_ARKHAN2, "SG00DIA05", "Sar_10");
                            }
                            /* 0xBDD3 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D79", P_ARKHAN3, "SG00DIA05", "Sar_10");
                            }
                            lieu_action_register("L05D80A", P_BOGDARAN, "SP08ATT04", "SP08ARR03_20");
                            lieu_action_register("L05D80B", P_BOGDARAN, "SP08DIA55", "SP08ARR03_20");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                    /* 0xBE64 */
                    if (ST(V_DELIA_PHASE) >= 11 && ST(ST_VIE(P_DELIA)) == 1) {
                        /* 0xBE88 */
                        if (ST(L05_DELIA_FIGHT_PHASE) == 0) {
                            ST(L05_DELIA_FIGHT_PHASE) = 1;
                            /* 0xBEA9 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D87", P_ARKHAN2, "SG00DIA05", "Sar_10");
                            }
                            /* 0xBED4 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D88", P_ARKHAN3, "SG00DIA05", "Sar_10");
                            }
                            CB->stop_joindre(P_BOGDARAN);
                            CB->animation("SP08ARR03_10");
                            CB->modify_var(&ST(L05_WEAPON_GIVE_STEP), 1);
                            CB->joindre(-P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                            return;
                        }
                        /* 0xBF4E */
                        if (ST(L05_DELIA_FIGHT_PHASE) == 1) {
                            ST(L05_DELIA_FIGHT_PHASE) = 2;
                            /* 0xBF6F */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D89", P_ARKHAN2, "SG00DIA07", "Sar_10");
                            }
                            /* 0xBF9A */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D90", P_ARKHAN3, "SG00DIA07", "Sar_10");
                            }
                            lieu_action_register("L05D91A", P_BOGDARAN, "SP08DIA56", "SP08ARR03_20");
                            lieu_action_register("L05D91B", P_BOGDARAN, "SP08DIA63", "SP08ARR03_20");
                            lieu_action_register("L05D91C", P_BOGDARAN, "SP08DIA56", "SP08ARR03_20");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xC044 */
                        if (ST(L05_DELIA_FIGHT_PHASE) == 2 && ST(L05_BOGDARAN_LEFT) == 0) {
                            /* 0xC068 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D92", P_ARKHAN2, "SG00DIA07", "Sar_10");
                            }
                            /* 0xC093 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D93", P_ARKHAN3, "SG00DIA07", "Sar_10");
                            }
                            lieu_action_register("L05D94", P_BOGDARAN, "SP08DIA56", "SP08ARR03_20");
                            /* 0xC0D7 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D95", P_ARKHAN2, "SG00DIA07", "Sar_10");
                            }
                            /* 0xC102 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D96", P_ARKHAN3, "SG00DIA07", "Sar_10");
                            }
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xC161 */
                        if (ST(L05_DELIA_FIGHT_PHASE) == 2 && ST(L05_BOGDARAN_LEFT) == 1) {
                            ST(L05_BOGDARAN_LEFT) = 2;
                            /* 0xC194 */
                            if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D81", P_ARKHAN2, "SG00DIA07", "Sar_10");
                            }
                            /* 0xC1D1 */
                            if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D97", P_ARKHAN2, "SG00DIA07", "Sar_10");
                            }
                            /* 0xC20E */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D98", P_ARKHAN3, "SG00DIA07", "Sar_10");
                            }
                            lieu_action_register("L05D99", P_BOGDARAN, "SP08DIA03", "SP08ARR03_20");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xC286 */
                        if (ST(L05_DELIA_FIGHT_PHASE) == 2 && ST(L05_BOGDARAN_LEFT) == 3) {
                            /* 0xC2AA */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D92", P_ARKHAN2, "SG00DIA07", "Sar_10");
                            }
                            /* 0xC2D5 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D93", P_ARKHAN3, "SG00DIA07", "Sar_10");
                            }
                            lieu_action_register("L05D82", P_BOGDARAN, "SP08DIA54", "SP08ARR03_20");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                    /* 0xC34D */
                    if (ST(V_DELIA_PHASE) >= 11 && ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(L05_D72_STAGE) == 1) {
                        ST(V_DELIA_PHASE) = 99;
                        ST(L05_D45_STAGE) = 2;
                        /* 0xC3A1 */
                        if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L05D72", P_ARKHAN2, "SG00DIA01", "Sar_10");
                        }
                        /* 0xC3DE */
                        if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L05D73", P_ARKHAN2, "SG00DIA01", "Sar_10");
                            lieu_action_register("L05D74", P_BOGDARAN, "SP08DIA53", "SP08ARR03_20");
                            lieu_action_register("L05D75", P_ARKHAN2, "SG00DIA01", "Sar_10");
                        }
                        /* 0xC44D */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D76", P_ARKHAN3, "SG00DIA01", "Sar_10");
                        }
                        lieu_action_register("L05D77A", P_BOGDARAN, "SP08DIA63", "SP08ARR03_20");
                        lieu_action_register("L05D77B", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
                        CB->animation("SG00ATT01");
                        lieu_action_register("L05D77C", P_BOGDARAN, "SP08DIA61", "SP08ARR03_20");
                        lieu_action_register("L05D77D", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
                        ST(L05_BOGDARAN_STAGE) = 10;
                        CB->combat(P_BOGDARAN, 0);
                        return;
                    } else {
                        /* 0xC532 */
                        if (ST(V_GUARD_FIGHT_DONE) == 0) {
                            /* 0xC544 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D103", P_ARKHAN2, "SG00DIA03", "Sar_10");
                            }
                            /* 0xC56F */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D104", P_ARKHAN3, "SG00DIA03", "Sar_10");
                            }
                            lieu_action_register("L05D105", P_BOGDARAN, "SP08DIA53", "SP08ARR03_20");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                        /* 0xC5E7 */
                        if (ST(V_GUARD_FIGHT_DONE) != 0) {
                            /* 0xC5F9 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D108", P_ARKHAN2, "SG00DIA03", "Sar_10");
                            }
                            /* 0xC624 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D109", P_ARKHAN3, "SG00DIA03", "Sar_10");
                            }
                            lieu_action_register("L05D110A", P_BOGDARAN, "SP08DIA60", "SP08ARR03_20");
                            lieu_action_register("L05D110B", P_BOGDARAN, "SP08DIA60", "SP08ARR03_20");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                            CB->bloque(BLOQUE_NONE);
                            return;
                        }
                    }
                }
            }
        }
        /* 0xC6B5 */
        if (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) != 40 && ST(ST_PLAYER_WEAPON) != 41 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
            snprintf(g_txt, sizeof(g_txt), "use=%d", ST(ST_PLAYER_WEAPON));
            CB->print_msg(g_txt, 0, 0xF0);
            /* 0xC74C */
            if (ST(ST_HERO) == P_ARKHAN1) {
                lieu_action_register("L05D111", P_ARKHAN1, "SG00PRE80", "SG00PRE81");
            }
            /* 0xC777 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
            }
            /* 0xC7A2 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("LCOD19", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
            }
            /* 0xC7CD */
            if (ST(ST_PLAYER_WEAPON) != 1 && ST(ST_PLAYER_WEAPON) != 6 && ST(ST_PLAYER_WEAPON) != 7 && ST(ST_PLAYER_WEAPON) != 8 && ST(ST_PLAYER_WEAPON) != 23 && ST(ST_PLAYER_WEAPON) != 27 && ST(ST_PLAYER_WEAPON) != 33 && ST(ST_PLAYER_WEAPON) != 55) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                lieu_action_register("L05D114", P_BOGDARAN, "SP08ATT60", "SP08ARR03_10");
            }
            /* 0xC88A */
            if (ST(ST_PLAYER_WEAPON) == 1) {
                /* 0xC89C */
                if (ST(L05_D114_STAGE) == 0) {
                    ST(L05_D114_STAGE) = 1;
                    /* 0xC8BD */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L05D115", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0xC8E8 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L05D116", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    CB->animation("SP08PRE90_1000");
                    lieu_action_register("L05D117A", P_BOGDARAN, "P08ATT62", "SP08ARR03_10");
                    lieu_action_register("L05D117B", P_BOGDARAN, "SP08ATT62", "SP08ARR03_10");
                    CB->animation("G00PRE82");
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xC99A */
                if (ST(L05_D114_STAGE) == 1) {
                    /* 0xC9AC */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L05D118", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0xC9D7 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L05D119", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    lieu_action_register("L05D120", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                }
            }
            /* 0xCA1B */
            if (ST(ST_PLAYER_WEAPON) == 6) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                CB->animation("SP08PRE90_800");
                lieu_action_register("L05D121A", P_BOGDARAN, "P08DIA63", "SP08ARR03_10");
                lieu_action_register("L05D121B", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
                lieu_action_register("L05D121C", P_BOGDARAN, "SP08DIA59", "SP08ARR03_10");
            }
            /* 0xCA97 */
            if (ST(ST_PLAYER_WEAPON) == 7) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                CB->animation("SP08PRE90_1000");
                lieu_action_register("L05D122A", P_BOGDARAN, "P08ATT04", "SP08ARR03_10");
                lieu_action_register("L05D122B", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
            }
            /* 0xCAFA */
            if (ST(ST_PLAYER_WEAPON) == 8) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                lieu_action_register("L05D123A", P_BOGDARAN, "SP08DIA63", "SP08ARR03_10");
                lieu_action_register("L05D123B", P_BOGDARAN, "SP08DIA63", "SP08ARR03_10");
            }
            /* 0xCB52 */
            if (ST(ST_PLAYER_WEAPON) == 23) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                lieu_action_register("L05D124A", P_BOGDARAN, "SP08PRE90", "SP08ARR03_10");
                lieu_action_register("L05D124B", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
                lieu_action_register("L05D124C", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
                lieu_action_register("L05D124D", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
                /* 0xCBDC */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D125", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                }
                /* 0xCC07 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D126", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                }
            }
            /* 0xCC32 */
            if (ST(ST_PLAYER_WEAPON) == 27) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                CB->animation("SP08PRE90_1000");
                lieu_action_register("L05D135A", P_BOGDARAN, "P08DIA56", "SP08ARR03_10");
                lieu_action_register("L05D135B", P_BOGDARAN, "SP08DIA56", "SP08ARR03_10");
                lieu_action_register("L05D135C", P_BOGDARAN, "SP08DIA56", "SP08ARR03_10");
                lieu_action_register("L05D135D", P_BOGDARAN, "SP08DIA56", "SP08ARR03_10");
                /* 0xCCC7 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D125", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                }
                /* 0xCCF2 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D126", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                }
            }
            /* 0xCD1D */
            if (ST(ST_PLAYER_WEAPON) == 33) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                CB->animation("SP08PRE90_1000");
                lieu_action_register("L05D138A", P_BOGDARAN, "P08DIA51", "SP08ARR03_10");
                lieu_action_register("L05D138B", P_BOGDARAN, "SP08DIA50", "SP08ARR03_10");
                lieu_action_register("L05D138C", P_BOGDARAN, "SP08DIA51", "SP08ARR03_10");
            }
            /* 0xCD99 */
            if (ST(ST_PLAYER_WEAPON) == 55) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                CB->animation("SP08PRE90_1000");
                lieu_action_register("L05D141A", P_BOGDARAN, "P08DIA03", "SP08ARR03_10");
                lieu_action_register("L05D141B", P_BOGDARAN, "SP08DIA03", "SP08ARR03_10");
                lieu_action_register("L05D141C", P_BOGDARAN, "SP08DIA03", "SP08ARR03_10");
            }
            CB->animation("G00PRE82");
            CB->animation("ar_20");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xCE5F */
        if (ST(ST_PLAYER_WEAPON) == 1) {
            /* 0xCE71 */
            if (ST(L05_D114_STAGE) == 0) {
                ST(L05_D114_STAGE) = 1;
                /* 0xCE92 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D115", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                }
                /* 0xCEBD */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D116", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                }
                CB->animation("SP08PRE90_1000");
                lieu_action_register("L05D117A", P_BOGDARAN, "P08ATT62", "SP08ARR03_10");
                lieu_action_register("L05D117B", P_BOGDARAN, "SP08ATT62", "SP08ARR03_10");
                CB->animation("G00PRE82");
                CB->animation("ar_20");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xCF6F */
            if (ST(L05_D114_STAGE) == 1) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                /* 0xCF95 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D118", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                }
                /* 0xCFC0 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D119", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                }
                lieu_action_register("L05D120", P_BOGDARAN, "SP08DIA54", "SP08ARR03_10");
            }
        }
        /* 0xD004 */
        if (ST(ST_PLAYER_WEAPON) == 41) {
            /* 0xD016 */
            if (ST(L05_D142_SAID) == 0) {
                ST(L05_D142_SAID) = 1;
                /* 0xD037 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D142A", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    CB->dialogue("L05D142B", P_ARKHAN2);
                    CB->dialogue("L05D142C", P_ARKHAN2);
                }
                /* 0xD082 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D143A", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    CB->dialogue("L05D143B", P_ARKHAN3);
                }
                lieu_action_register("L05D144A", P_BOGDARAN, "SP08PRE90", "SP08ARR03_10");
                lieu_action_register("L05D144B", P_BOGDARAN, "SP08ATT04", "SP08ARR03_10");
                CB->animation("G00PRE82");
                CB->animation("ar_20");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xD139 */
            if (ST(L05_D142_SAID) == 1) {
                ST(L05_BOGDARAN_QUEST) = 2;
                /* 0xD15A */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D83", P_ARKHAN2, "SG00DIA02", NULL);
                }
                /* 0xD182 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D84", P_ARKHAN3, "SG00DIA02", NULL);
                }
                lieu_action_register("L05D85", P_BOGDARAN, "SP08DIA56", "SP08ARR03_10");
                CB->animation("P08_65535");
                lieu_pre_scene(ST(ST_HERO), P_BOGDARAN, 41, 6, 2);
                CB->animation("P08ARR03_10");
                CB->divers(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE40);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0xD237 */
    if (ST(ST_ACTION_KEY) == 1 && (ST(V_ENGAGED_PERSONA) == P_BOGDARAN || ST(V_ENGAGED_PERSONA) == P_DELIA) && ST(ST_VIE(P_BOGDARAN)) == 1 && CB->direction_perso(P_BOGDARAN) && ST(ST_SCRIPT_BUSY) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xD2A7 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->dialogue("L05D147", P_ARKHAN1);
        }
        /* 0xD2C9 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L05D148", P_ARKHAN2);
        }
        /* 0xD2EB */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L05D149", P_ARKHAN3);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
    /* 0xD31B */
    if (ST(V_PLAYER_STRIKE) > 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_BOGDARAN) && ST(ST_VIE(P_BOGDARAN)) == 1) {
        ST(V_PLAYER_STRIKE) = -1;
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
        CB->stop_joindre(-P_BOGDARAN);
        CB->combat(P_BOGDARAN, 0);
        CB->evenement(33, 1000);
    }
    /* 0xD3A1 */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        CB->divers_empile(DV_BLOOD_AT_BONE, P_BOGDARAN, 0);
        /* 0xD3D4 */
        if (ST(V_DELIA_PHASE) == 11 && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
            CB->print_msg("joindreperm ln1650", 0, 0x122);
            CB->select_pile(2, 2);
            CB->stop_joindre(-P_DELIA);
            CB->animation("SP10GAR01");
            CB->joindre_perm(P_DELIA, ST(ST_HERO));
        }
        CB->select_pile(0, 1);
        return;
    }
    /* 0xD455 */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_BOGDARAN) && ST(ST_VIE(P_BOGDARAN)) == 1) {
        ST(V_PLAYER_STRIKE) = -1;
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_BOGDARAN);
        CB->evenement(2, -1);
        CB->animation("SP08ARR03_10");
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
        /* 0xD4E4 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0xD4F6 */
            if (ST(L05_D150_FOLLOWUP) == 0) {
                ST(L05_D150_FOLLOWUP) = 1;
                CB->joindre(P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                CB->print_msg("dialogue ln 1822", 0, 0xF0);
                lieu_action_register("L05D150A", P_BOGDARAN, "SP08GAR01", "SP08ARR03_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
                CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xD5A4 */
            if (ST(L05_D150_FOLLOWUP) == 1) {
                ST(L05_BOGDARAN_STAGE) = 10;
                CB->joindre(P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L05D150B", P_BOGDARAN, "SP08GAR01", "SP08ARR03_10");
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_BOGDARAN, 0);
                ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
                return;
            }
        }
        /* 0xD621 */
        if (ST(ST_HERO) >= 2) {
            CB->animation("SP08ESM01");
            CB->animation("P08GAR01_10");
            /* 0xD649 */
            if (ST(L05_BOGDARAN_STAGE) == 0) {
                ST(L05_BOGDARAN_STAGE) = 1;
                ST(L05_D151_STAGE) = 2;
                CB->dialogue("L05D151", P_BOGDARAN);
                /* 0xD689 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D152", P_ARKHAN2, "SG00DIA08", NULL);
                }
                /* 0xD6B1 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D153", P_ARKHAN3, "SG00DIA08", NULL);
                }
                CB->dialogue("L05D154", P_BOGDARAN);
                /* 0xD6E9 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D155", P_ARKHAN2, "SG00DIA04", NULL);
                }
                /* 0xD711 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D156", P_ARKHAN3, "SG00DIA04", NULL);
                }
                lieu_action_register("L05D157", P_BOGDARAN, "SP08GAR01", "SP08ARR02_10");
                /* 0xD752 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L05D158", P_ARKHAN2, "SG00DIA08", NULL);
                }
                /* 0xD77A */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L05D159", P_ARKHAN3, "SG00DIA08", NULL);
                }
                lieu_action_register("L05D160A", P_BOGDARAN, "SP08ARR02", "SP08ARR03_10");
                lieu_action_register("L05D160B", P_BOGDARAN, "SP08ARR02", "SP08ARR03_10");
                CB->animation("Sar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
                CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xD824 */
            if (ST(L05_BOGDARAN_STAGE) == 1) {
                /* 0xD836 */
                if (ST(L05_D150B_SAID) == 0) {
                    ST(L05_D150B_SAID) = 1;
                    CB->joindre(P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                    CB->print_msg("dialogue ln 1881", 0, 0xF0);
                    lieu_action_register("L05D150A", P_BOGDARAN, "P08GAR01", "SP08ARR03_10");
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    /* 0xD8CC */
                    if (ST(V_DELIA_PHASE) == 11 && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
                        CB->stop_joindre(-P_BOGDARAN);
                        CB->joindre(P_BOGDARAN, 0, 0, 0, P_DELIA);
                    } else {
                        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                    }
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xD936 */
                if (ST(L05_D150B_SAID) == 1) {
                    CB->joindre(P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                    CB->print_msg("dialogue ln 1895", 0, 0xF0);
                    lieu_action_register("L05D150B", P_BOGDARAN, "P08GAR01", "SP08ARR03_10");
                    ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
                    CB->bloque(BLOQUE_NONE);
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->combat(P_BOGDARAN, 0);
                    ST(L05_BOGDARAN_STAGE) = 10;
                    /* 0xD9DC */
                    if (ST(V_DELIA_PHASE) == 11 && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
                        CB->print_msg("joindreperm ln1750", 0, 0x127);
                        CB->select_pile(1, 2);
                        CB->stop_joindre(-P_DELIA);
                        CB->animation("SP10GAR01");
                        CB->joindre_perm(P_DELIA, ST(ST_HERO));
                        CB->select_pile(0, 1);
                    }
                    return;
                }
            }
        }
    }
}

/* trigger 03 — sub_1000DA59 (thunk 0x100010EB) */
static void trig03(void) {
    /* 0xDA6C */
    if (CB->direction_zone(9) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->pos_perso(ST(ST_HERO), -2169, 73, 0, 1, "ar", 6, 0);
        /* 0xDAD8 */
        if (ST(V_EVENT_8) != 2) {
            CB->animation("sar_1");
            /* 0xDAF5 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("P01ACT07");
                CB->animation("ar_10");
                /* 0xDB1A */
                if (ST(L05_TRIG3_TALK_STAGE) == 0) {
                    ST(L05_TRIG3_TALK_STAGE) = 1;
                    CB->dialogue("L05D162", P_ARKHAN1);
                }
            }
            /* 0xDB48 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0xDB62 */
                if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0xDB74 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("P02ACT07");
                    }
                    /* 0xDB91 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("P02ACT34");
                    }
                }
                /* 0xDBB6 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0xDBC8 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("P02ACT35");
                    }
                    /* 0xDBE5 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("P02ACT36");
                    }
                }
                CB->animation("ar_10");
                /* 0xDC0A */
                if (ST(L05_TRIG3_TALK_STAGE) == 0) {
                    ST(L05_TRIG3_TALK_STAGE) = 1;
                    CB->dialogue("L05D163", P_ARKHAN2);
                }
            }
            /* 0xDC38 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0xDC52 */
                if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0xDC64 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("P03ACT07");
                    }
                    /* 0xDC81 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("P03ACT34");
                    }
                }
                /* 0xDCA6 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0xDCB8 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("P03ACT35");
                    }
                    /* 0xDCD5 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("P03ACT36");
                    }
                }
                CB->animation("ar_10");
                /* 0xDCFA */
                if (ST(L05_TRIG3_TALK_STAGE) == 0) {
                    ST(L05_TRIG3_TALK_STAGE) = 1;
                    CB->dialogue("L05D164", P_ARKHAN3);
                }
            }
            /* 0xDD28 */
            if (ST(ST_VIE(P_BOGDARAN)) < 2 && (ST(ST_HERO) == P_ARKHAN1 || (ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 1)) && ST(L05_TRIG3_TALK_STAGE) == 1) {
                ST(L05_TRIG3_TALK_STAGE) = 2;
                CB->change_camera(0, CAM_LOCKED);
                CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
                /* 0xDDAA */
                if (ST(ST_PLAYER_WEAPON) != 41) {
                    CB->dialogue("L05D165A", P_BOGDARAN);
                    CB->dialogue("L05D165B", P_BOGDARAN);
                }
                /* 0xDDDC */
                if (ST(ST_PLAYER_WEAPON) == 41) {
                    CB->dialogue("L05D165B", P_BOGDARAN);
                }
                CB->change_camera(0, CAM_AUTO);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0xDE24 */
        if (ST(V_EVENT_8) == 2) {
            /* 0xDE36 */
            if (ST(ST_PLAYER_WEAPON) != 41) {
                CB->animation("sar_1");
                /* 0xDE53 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    /* 0xDE6D */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0xDE7F */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            CB->animation("P02ACT07");
                        }
                        /* 0xDE9C */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            CB->animation("P02ACT34");
                        }
                    }
                    /* 0xDEC1 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0xDED3 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            CB->animation("P02ACT35");
                        }
                        /* 0xDEF0 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            CB->animation("P02ACT36");
                        }
                    }
                    CB->bruitage("L05B01", -2178, 53);
                    CB->animation("ar_10");
                    /* 0xDF27 */
                    if (ST(L05_TRIG3_TALK_STAGE) == 0) {
                        ST(L05_TRIG3_TALK_STAGE) = 1;
                        CB->dialogue("L05D168", P_ARKHAN2);
                    }
                }
                /* 0xDF55 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    /* 0xDF6F */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0xDF81 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            CB->animation("P03ACT07");
                        }
                        /* 0xDF9E */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            CB->animation("P03ACT34");
                        }
                    }
                    /* 0xDFC3 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0xDFD5 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            CB->animation("P03ACT35");
                        }
                        /* 0xDFF2 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            CB->animation("P03ACT36");
                        }
                    }
                    CB->bruitage("L05B01", -2178, 53);
                    CB->animation("ar_10");
                    /* 0xE029 */
                    if (ST(L05_TRIG3_TALK_STAGE) == 0) {
                        ST(L05_TRIG3_TALK_STAGE) = 1;
                        CB->dialogue("L05D169", P_ARKHAN3);
                    }
                }
                /* 0xE057 */
                if (ST(ST_VIE(P_BOGDARAN)) < 2 && (ST(ST_HERO) == P_ARKHAN1 || (ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 1)) && ST(L05_TRIG3_TALK_STAGE) == 1) {
                    ST(L05_TRIG3_TALK_STAGE) = 2;
                    CB->change_camera(0, CAM_LOCKED);
                    CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
                    CB->dialogue("L05D170", P_BOGDARAN);
                    CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
                    CB->change_camera(0, CAM_AUTO);
                }
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xE131 */
            if (ST(ST_PLAYER_WEAPON) == 41) {
                CB->pos_perso(ST(ST_HERO), -2173, 77, 0xD1B, 1, "ar", 6, 0);
                CB->animation("sar_1");
                /* 0xE176 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    /* 0xE188 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("P02ACT03");
                    }
                    /* 0xE1A5 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("P02ACT33");
                    }
                }
                /* 0xE1C2 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    /* 0xE1D4 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->animation("P03ACT03");
                    }
                    /* 0xE1F1 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->animation("P03ACT33");
                    }
                }
                CB->etat_objet("OB41", -1, -1, -1, -1, -1, -1, -2, 3);
                CB->etat_objet("OB40", -1, -1, -1, -1, -1, -1, -3, 0);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
            }
        }
    }
    /* 0xE264 */
    if (CB->direction_zone(17)) {
        trig16();
    }
}

/* trigger 04 — sub_1000E274 (thunk 0x10001082) */
static void trig04(void) {
    /* 0xE286 */
    if (ST(ST_VIE(P_BOGDARAN)) != VIE_DEAD && ST(L05_BOGDARAN_QUEST) == 2) {
        ST(L05_BOGDARAN_QUEST) = 3;
    }
    ST(ST_LEVEL) = 5;
    CB->divers_empile(DV_MUSIC, 1, 4);
    CB->change_piece(45, -1);
}

/* trigger 05 — sub_1000E2E0 (thunk 0x100010AA) */
static void trig05(void) {
    /* 0xE2F3 */
    if (CB->direction_perso(P_DELIA)) {
        /* 0xE302 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0xE332 */
            if (ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD && ST(ST_VIE(P_DELIA)) < 2) {
                lieu_action_register("L05D04", P_DELIA, "SP10DIA59", "SP10GAR01_10");
                return;
            }
            /* 0xE374 */
            if (ST(ST_VIE(P_DELIA)) < 2) {
                CB->print_msg("acton ln1986", 0, 0xF0);
                ST(ST_SCRIPT_BUSY) = 1;
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_DELIA);
                ST(L05_DELIA_FOLLOW_STEP) = 5;
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DELIA);
                /* 0xE3E6 */
                if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(V_LAST_WEAPON))) {
                    /* 0xE43B */
                    if (ST(L05_D171_SAID) == 0) {
                        ST(L05_D171_SAID) = 1;
                        /* 0xE45C */
                        if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L05D171", P_ARKHAN2, "SG00DIA04", NULL);
                        }
                        /* 0xE496 */
                        if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L05D172", P_ARKHAN2, "SG00DIA04", NULL);
                        }
                        /* 0xE4D0 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L05D212", P_ARKHAN3, "SG00DIA04", NULL);
                        }
                        lieu_action_register("L05D173", P_DELIA, "SP10DIA50", "SP10ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(L05_DELIA_FOLLOW_STEP), 3);
                        return;
                    }
                    /* 0xE546 */
                    if (ST(L05_D171_SAID) == 1) {
                        /* 0xE558 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            /* 0xE56A */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D174", P_ARKHAN2, "SG00DIA04", NULL);
                            }
                            /* 0xE592 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D175", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            lieu_action_register("L05D176", P_DELIA, "SP10DIA50", "SP10ARR01_10");
                            lieu_action_register("L05D177A", P_BOGDARAN, "SP08DIA57", "SP08ARR03_20");
                            lieu_action_register("L05D177B", P_BOGDARAN, "SP08DIA57", "SP08ARR03_20");
                        }
                        /* 0xE605 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            /* 0xE617 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L05D178A", P_ARKHAN2, "SG00DIA04", NULL);
                                lieu_action_register("L05D178B", P_ARKHAN2, "SG00DIA04", NULL);
                            }
                            /* 0xE655 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L05D179", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            lieu_action_register("L05D180A", P_DELIA, "SP10DIA50", "SP10ARR01_10");
                            lieu_action_register("L05D180B", P_DELIA, "SP10DIA50", "SP10ARR01_10");
                            lieu_action_register("L05D181", P_BOGDARAN, "SP08DIA57", "SP08ARR03_20");
                        }
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(L05_DELIA_FOLLOW_STEP), 3);
                    }
                }
                /* 0xE6F8 */
                if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    /* 0xE74F */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81");
                    }
                    /* 0xE77A */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("LCOD19", P_ARKHAN3, "SG00PRE80", "SG00PRE81");
                    }
                    lieu_action_register("L05D86", P_DELIA, "SP10DIA54", "SP10ARR01_10");
                    CB->animation("G00PRE82");
                    CB->animation("ar_20");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(L05_DELIA_FOLLOW_STEP), 3);
                }
            }
            /* 0xE804 */
            if (ST(ST_VIE(P_DELIA)) == VIE_DEAD) {
                CB->fouille(P_DELIA);
            }
        }
        /* 0xE81E */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && (ST(V_ENGAGED_PERSONA) == P_DELIA || ST(V_ENGAGED_PERSONA) == P_BOGDARAN) && ST(ST_VIE(P_DELIA)) == 1) {
            CB->bloque(BLOQUE_ALL);
            /* 0xE87A */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0xE88C */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->dialogue("L45D01", P_ARKHAN2);
                }
                /* 0xE8AE */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->dialogue("L45D02", P_ARKHAN2);
                }
            }
            /* 0xE8D0 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0xE8E2 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->dialogue("L45D03", P_ARKHAN3);
                }
                /* 0xE904 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->dialogue("LCOD22C", P_ARKHAN3);
                }
            }
            CB->bloque(BLOQUE_NONE);
        }
        /* 0xE92B */
        if (ST(V_PLAYER_STRIKE) > 1 && ST(ST_SCRIPT_BUSY) == 0 && (ST(V_ENGAGED_PERSONA) == 0 || ST(V_ENGAGED_PERSONA) == P_BOGDARAN) && ST(ST_VIE(P_DELIA)) == 1) {
            ST(V_PLAYER_STRIKE) = -1;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->divers(DV_HP, P_DELIA, 1);
            CB->stop_joindre(-P_DELIA);
            CB->combat(P_DELIA, 0);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_DELIA, 0);
            /* 0xE9D0 */
            if (ST(ST_VIE(P_BOGDARAN)) == 1) {
                CB->stop_joindre(-P_BOGDARAN);
                lieu_action_register("L05D39A", P_BOGDARAN, "SP08DIA01", "SP08GAR01_10");
                CB->combat(P_BOGDARAN, 0);
                ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
            }
            CB->select_pile(0, 1);
            return;
        }
    }
}

/* trigger 07 — sub_1000EA35 (thunk 0x1000101E) */
static void trig07(void) {
    /* 0xEA44 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x7530)) {
        CB->bloque(BLOQUE_ALL);
        CB->print_msg("acton ln2168", 0, 0xF0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xEAA1 */
        if (ST(L05_OB16_STATE) != 0 || (ST(L05_OB16_STATE) == 0 && ST(ST_HERO) == P_ARKHAN1)) {
            CB->animation("SG00PRE01");
            /* 0xEAE2 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->dialogue("L05D182", P_ARKHAN1);
            }
            /* 0xEB04 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L05D183", P_ARKHAN2);
            }
            /* 0xEB26 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L05D184", P_ARKHAN3);
            }
            /* 0xEB48 */
            if (ST(ST_VIE(P_BOGDARAN)) < 2 && (ST(ST_HERO) == P_ARKHAN1 || (ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 1))) {
                CB->divers_empile(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
                CB->dialogue("L05D194", P_BOGDARAN);
                CB->animation("ar_10");
                CB->divers_empile(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
            } else {
                CB->animation("ar_10");
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0xEBF8 */
        if (ST(L05_OB16_STATE) == 0 && ST(ST_HERO) > 1) {
            CB->animation("SG00PRE05");
            /* 0xEC27 */
            if (ST(V_GUARD_FIGHT_DONE) < 2) {
                ST(L05_OB16_STATE) = 1;
                CB->etat_objet("OB16", -1, -1, -1, -1, -1, -1, -3, 0);
            }
            /* 0xEC66 */
            if (ST(V_GUARD_FIGHT_DONE) >= 2) {
                ST(L05_OB16_STATE) = 2;
                CB->etat_objet("OB06B", -1, -1, -1, -1, -1, -1, -3, 0);
            }
            /* 0xECA5 */
            if (ST(ST_VIE(P_BOGDARAN)) < 2 && (ST(ST_HERO) == P_ARKHAN1 || (ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 1))) {
                CB->divers_empile(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
                CB->dialogue("L05D194", P_BOGDARAN);
                CB->animation("ar_10");
                CB->divers_empile(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
            } else {
                CB->animation("ar_10");
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
    }
}

/* trigger 08 — sub_1000ED4E (thunk 0x10001055) */
static void trig08(void) {
    /* 0xED5D */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && (CB->direction_spec(0x7530) || CB->direction_spec(0xAFC8))) {
        CB->bloque(BLOQUE_ALL);
        CB->print_msg("acton ln2230", 0, 0xF0);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xEDCD */
        if (ST(ST_HERO) == P_ARKHAN1) {
            /* 0xEDDF */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L05D185", P_ARKHAN1, "SG00PRE02", NULL);
            }
            /* 0xEE07 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("L05D186", P_ARKHAN1, "SP01ACT32", NULL);
            }
        }
        /* 0xEE2F */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0xEE41 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L05D187", P_ARKHAN2, "SG00PRE02", NULL);
            }
            /* 0xEE69 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0xEE83 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    lieu_action_register("L05D188", P_ARKHAN2, "SP02ACT85", NULL);
                }
                /* 0xEEB3 */
                if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    lieu_action_register("L05D188", P_ARKHAN2, "SP02ACT86", NULL);
                }
            }
        }
        /* 0xEEDB */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0xEEED */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("LCOD11", P_ARKHAN3, "SG00PRE02", NULL);
            }
            /* 0xEF15 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0xEF2F */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    lieu_action_register("L05D190", P_ARKHAN3, "SP03ACT85", NULL);
                }
                /* 0xEF5F */
                if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    lieu_action_register("L05D190", P_ARKHAN3, "SP03ACT86", NULL);
                }
            }
        }
        CB->animation("ar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 10 — sub_1000EFB4 (thunk 0x1000109B) */
static void trig10(void) {
    /* 0xEFC3 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x7530)) {
        CB->bloque(BLOQUE_ALL);
        CB->print_msg("acton ln2287", 0, 0xF0);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(V_DLG_RANDOM) = CB->random(3);
        CB->animation("SG00PRE03");
        /* 0xF042 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->dialogue("L05D191", P_ARKHAN1);
        }
        /* 0xF064 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L05D192", P_ARKHAN2);
        }
        /* 0xF086 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L05D193", P_ARKHAN3);
        }
        CB->animation("ar_10");
        /* 0xF0B3 */
        if (ST(ST_VIE(P_BOGDARAN)) < 2 && (ST(ST_HERO) == P_ARKHAN1 || (ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 1))) {
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
            CB->dialogue("L05D194", P_BOGDARAN);
            CB->animation("ar_10");
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
        } else {
            CB->animation("ar_10");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 11 — sub_1000F161 (thunk 0x10001028) */
static void trig11(void) {
    /* 0xF170 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->print_msg("action sur zone11", 0, 0xF0);
        /* 0xF1A3 */
        if (ST(L05_BOGDARAN_QUEST) == 3) {
            /* 0xF1B5 */
            if (ST(ST_VIE(P_BOGDARAN)) == 1 && ST(L05_D151_STAGE) == 1) {
                return;
            }
            /* 0xF1DE */
            if (ST(ST_VIE(P_BOGDARAN)) != VIE_DEAD && ST(L05_D151_STAGE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
                CB->bloque(BLOQUE_ALL);
                ST(ST_SCRIPT_BUSY) = 1;
                ST(L05_D151_STAGE) = 1;
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_BOGDARAN);
                CB->animation("SG00PRE06");
                lieu_action_register("L05D195", P_BOGDARAN, "SP08DIA59", "SP08ARR03");
                CB->animation("ar_100");
                /* 0xF283 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0xF295 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->dialogue("L05D196", P_ARKHAN2);
                    }
                    /* 0xF2B7 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->dialogue("LCOD22", P_ARKHAN3);
                    }
                }
                CB->animation("Sar_200");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xF302 */
            if (ST(ST_VIE(P_BOGDARAN)) != VIE_DEAD && ST(L05_D151_STAGE) == 1) {
                trig02();
            }
            /* 0xF32B */
            if (ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD) {
                CB->bloque(BLOQUE_ALL);
                ST(L05_BOGDARAN_QUEST) = 10;
                ST(ST_SCRIPT_BUSY) = 1;
                CB->animation("SG00PRE06");
                CB->retire_zone(11);
                CB->etat_objet("SOB41", -1, -1, -1, -1, -1, -1, -3, 0);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
            }
        }
    }
}

/* trigger 12 — sub_1000F3A9 (thunk 0x10001019) */
static void trig12(void) {
    /* 0xF3B8 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x7530)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(V_DLG_RANDOM) = CB->random(1);
        /* 0xF41A */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->animation("SG00PRE03");
            /* 0xF437 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->dialogue("LCOD09", P_ARKHAN1);
            }
            /* 0xF459 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD10", P_ARKHAN2);
            }
            /* 0xF47B */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD11", P_ARKHAN3);
            }
            CB->animation("ar_10");
            /* 0xF4A8 */
            if (ST(L05_D201_SAID) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1 && ST(L05_BOGDARAN_STAGE) == 1) {
                ST(L05_D201_SAID) = 1;
                CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
                CB->dialogue("L05D201", P_BOGDARAN);
                CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
            }
            CB->animation("ar_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xF54A */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->animation("SG00PRE01");
            /* 0xF567 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->dialogue("LCOD06", P_ARKHAN1);
            }
            /* 0xF589 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD07", P_ARKHAN2);
            }
            /* 0xF5AB */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD08", P_ARKHAN3);
            }
            CB->animation("ar_10");
            /* 0xF5D8 */
            if (ST(L05_D201_SAID) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1 && ST(L05_BOGDARAN_STAGE) == 1) {
                ST(L05_D201_SAID) = 1;
                CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
                CB->dialogue("L05D201", P_BOGDARAN);
                CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
            }
            CB->animation("ar_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 13 — sub_1000F673 (thunk 0x100010A0) */
static void trig13(void) {
    /* 0xF682 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xC350)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xF6DC */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0xF6EE */
            if (ST(ST_HERO) == P_ARKHAN0 || ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT28");
            }
            /* 0xF71D */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ACT28");
            }
            /* 0xF73A */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ACT28");
            }
            CB->animation("ar_200");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xF780 */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0xF792 */
            if (ST(ST_HERO) == P_ARKHAN0 || ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT29");
            }
            /* 0xF7C1 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ACT29");
            }
            /* 0xF7DE */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ACT29");
            }
            CB->animation("ar_200");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 14 — sub_1000F81D (thunk 0x100010D2) */
static void trig14(void) {
    /* 0xF82C */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xDEA8)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xF886 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0xF898 */
            if (ST(ST_HERO) == P_ARKHAN0 || ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT28");
            }
            /* 0xF8C7 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ACT28");
            }
            /* 0xF8E4 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ACT28");
            }
            CB->animation("ar_200");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xF92A */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0xF93C */
            if (ST(ST_HERO) == P_ARKHAN0 || ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT29");
            }
            /* 0xF96B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ACT29");
            }
            /* 0xF988 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ACT29");
            }
            CB->animation("ar_200");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 15 — sub_1000F9C7 (thunk 0x100010E6) */
static void trig15(void) {
    /* 0xF9D6 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xC350)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xFA30 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0xFA42 */
            if (ST(ST_HERO) == P_ARKHAN0 || ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT28");
            }
            /* 0xFA71 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ACT28");
            }
            /* 0xFA8E */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ACT28");
            }
            CB->animation("ar_200");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xFAD4 */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0xFAE6 */
            if (ST(ST_HERO) == P_ARKHAN0 || ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SP01ACT29");
            }
            /* 0xFB15 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ACT29");
            }
            /* 0xFB32 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ACT29");
            }
            CB->animation("ar_200");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 16 — sub_1000FB71 (thunk 0x10001096) */
static void trig16(void) {
    /* 0xFB80 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_zone(17)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->animation("SG00PRE01");
        /* 0xFBD6 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->dialogue("LCOD06", P_ARKHAN1);
        }
        /* 0xFBF8 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("LCOD07", P_ARKHAN2);
        }
        /* 0xFC1A */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("LCOD08", P_ARKHAN3);
        }
        CB->animation("ar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 18 — sub_1000FC69 (thunk 0x100010CD) */
static void trig18(void) {
    /* 0xFC78 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->animation("SG00PRE02");
        /* 0xFCCB */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->dialogue("LCOD06", P_ARKHAN1);
        }
        /* 0xFCED */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("LCOD07", P_ARKHAN2);
        }
        /* 0xFD0F */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("LCOD08", P_ARKHAN3);
        }
        CB->animation("ar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 41 — sub_10008462 (thunk 0x1000102D) */
static void trig41(void) {
    CB->divers_empile(DV_MUSIC, 1, 1);
    /* 0x8486 */
    if (ST(V_DELIA_PHASE) == 10 && ST(L05_DELIA_D21_STEP) == 0 && ST(V_EVENT15_STEP) == 0) {
        ST(V_EVENT15_STEP) = 1;
        ST(V_DELIA_PHASE) = 11;
        /* 0x84DA */
        if (ST(ST_VIE(P_BOGDARAN)) == 1) {
            CB->bloque(BLOQUE_ALL);
            ST(L05_BOGDARAN_MET) = 4;
            CB->animation("SP08ATT02_10");
            lieu_action_register("L05D01", P_BOGDARAN, "SP08DIA56", "SP08ARR03_10");
            CB->joindre(P_DELIA, -2129, -37, 0xAFC8, -1);
            CB->modify_var(&ST(L05_DELIA_D21_STEP), 1);
            CB->joindre_perm(P_BOGDARAN, P_DELIA);
        }
        /* 0x8567 */
        if (ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
            CB->bloque(BLOQUE_ALL);
            lieu_action_register("L05D02", P_DELIA, "SP10DIA59", "SP10GAR01_10");
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0x85B7 */
    if (ST(L05_BOGDARAN_STAGE) == 11 && ST(L05_DUEL_MODE) == 0) {
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
        ST(L05_BOGDARAN_STAGE) = 12;
        /* 0x85F6 */
        if (ST(ST_VIE(P_BOGDARAN)) == 1) {
            CB->joindre(-P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L05D03", P_BOGDARAN, "SP08DIA63", "SP08GAR01_10");
        }
        /* 0x863E */
        if (ST(V_DELIA_PHASE) == 11 && ST(ST_VIE(P_DELIA)) == 1) {
            ST(L05_DUEL_MODE) = 1;
            /* 0x866E */
            if (ST(ST_COMBAT_STANCE) == 0) {
                CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
            }
        }
    }
}
