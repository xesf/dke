/*
 * lieu05.c — Native hand-port of LIEU05.DLL (level-5 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu05_trig.c (reached through
 * l05_trigger, see lieu05_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x100120E0, the state pointer
 * global is dword_1002512C and the trigger table is dword_10025040[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_100127AC), lieu_pre_scene (sub_1001295C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are instruction-for-instruction identical to LIEU03. tick_body
 * (sub_10008696) is level-5 specific.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu05_int.h"

/* Global sprintf buffer of the debug overlay (dword_10024E40). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * Per-frame body — sub_10008696 (thunk 0x100010B9), level-5 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    snprintf(g_txt, sizeof(g_txt), "combat_on=%d", ST(V_ENGAGED_PERSONA));
    CB->print_msg(g_txt, 0, 0x122);
    /* 0x86D8 */
    if (ST(L05_DUEL_MODE) == 1 && ST(ST_VIE(P_DELIA)) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(V_ENGAGED_PERSONA) = P_DELIA;
        CB->combat(P_DELIA, 0);
    }
    /* 0x8724 */
    if (ST(V_DELIA_PHASE) == 11 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        CB->bloque(BLOQUE_ALL);
        lieu_action_register("L05D04", P_DELIA, "SP10DIA59", "SP10GAR01_10");
        CB->bloque(BLOQUE_NONE);
        CB->stop_joindre(-P_DELIA);
        CB->combat(P_DELIA, 0);
        CB->divers_empile(DV_HP, P_DELIA, 1);
        ST(V_ENGAGED_PERSONA) = P_DELIA;
    }
    /* 0x87CB */
    if (ST(L05_DUEL_MODE) == 2 && ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(ST_VIE(P_BOGDARAN)) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->bloque(BLOQUE_ALL);
        lieu_action_register("L05D77D", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
        CB->bloque(BLOQUE_NONE);
        CB->stop_joindre(-P_BOGDARAN);
        CB->combat(P_BOGDARAN, 0);
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
    }
    /* 0x8860 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_BOGDARAN) && ST(ST_VIE(P_BOGDARAN)) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT_0) = 0;
        CB->evenement(0, 2000);
    }
    /* 0x891D */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 2;
        lieu_action_register("L05D39A", P_BOGDARAN, "SP08DIA01", "SP08GAR01_10");
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8999 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_DELIA) && ST(ST_VIE(P_DELIA)) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 5)) {
        ST(V_CRI_USED) = 1;
        ST(V_EVENT_1) = 0;
        CB->evenement(1, 1000);
    }
    /* 0x8A4E */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 2;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP10DIA50");
        CB->dialogue("L16D122", P_DELIA);
        CB->animation("SP10GAR01_10");
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8AE2 */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 1500);
    }
    /* 0x8B25 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        ST(V_DEATH_EVENT_SENT) = 1;
        /* 0x8B65 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x8B8E */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x8BBA */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x8BE6 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8C0A */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x8C27 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x8C53 */
    if (ST(ST_VIE(P_BOGDARAN)) == VIE_DEAD) {
        /* 0x8C62 */
        if (ST(L05_BOGDARAN_FIGHT_STEP) != 0) {
            CB->print_msg("pile 2 videe", 0, 0x10E);
            CB->select_pile(2, 2);
            ST(L05_BOGDARAN_FIGHT_STEP) = 0;
        }
        /* 0x8C9F */
        if (ST(V_ENGAGED_PERSONA) == P_BOGDARAN && (ST(V_DELIA_PHASE) < 10 || (ST(V_DELIA_PHASE) > 10 && ST(ST_VIE(P_DELIA)) == VIE_DEAD))) {
            ST(ST_SCRIPT_BUSY) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_EVENT_8) = 1;
            CB->ajoute_zone(1, -2007, -107, 90, 90, 0);
            CB->evenement(8, 120000);
            return;
        }
        /* 0x8D38 */
        if (ST(V_ENGAGED_PERSONA) == P_BOGDARAN && ST(V_DELIA_PHASE) >= 10 && ST(ST_VIE(P_DELIA)) == 1) {
            ST(ST_SCRIPT_BUSY) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->dialogue("L05D04", P_DELIA);
            ST(L05_DUEL_MODE) = 1;
            CB->stop_joindre(-P_DELIA);
            CB->print_msg("mise en combat ln 489", 0, 0x10E);
            CB->combat(P_DELIA, 0);
            ST(V_ENGAGED_PERSONA) = P_DELIA;
            return;
        }
        /* 0x8DDD */
        if (ST(V_ENGAGED_PERSONA) == P_BOGDARAN) {
            ST(ST_SCRIPT_BUSY) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_EVENT_8) = 1;
            CB->evenement(8, 120000);
            return;
        }
    }
    /* 0x8E2B */
    if (ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(V_DELIA_KILLED) == 0) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(L05_DUEL_MODE) = 2;
        ST(V_DELIA_KILLED) = 5;
    }
    /* 0x8E79 */
    if (ST(L05_BOGDARAN_STAGE) == 12) {
        ST(L05_BOGDARAN_STAGE) = 11;
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
        /* 0x8EA6 */
        if (ST(ST_VIE(P_BOGDARAN)) == 1) {
            CB->combat(P_BOGDARAN, 0);
        }
        /* 0x8EC5 */
        if (ST(ST_VIE(P_DELIA)) == 1 && ST(V_DELIA_PHASE) == 11) {
            CB->print_msg("mise en combat ln524", 0, 0x109);
            CB->combat(P_DELIA, 0);
            ST(V_ENGAGED_PERSONA) = P_DELIA;
        }
    }
    /* 0x8F1C */
    if (CB->divers(DV_WAS_HIT, P_BOGDARAN, 0) && ST(ST_VIE(P_BOGDARAN)) != VIE_DEAD && ST(ST_VIE(P_BOGDARAN)) != 99) {
        ST(ST_VIE(P_BOGDARAN)) = 99;
        ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
        CB->select_pile(1, 2);
        CB->combat(P_BOGDARAN, -1);
        CB->stop_joindre(-P_BOGDARAN);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_BOGDARAN, 0);
        CB->animation("SP08IMP51");
        CB->modify_var(&ST(ST_VIE(P_BOGDARAN)), 2);
        CB->animation("P08CAD");
        CB->pos_perso(P_BOGDARAN, 0, 0, 0, 2, "P08CAD", 2, 0);
        /* 0x8FF3 */
        if (ST(V_DELIA_PHASE) == 11 && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
            CB->print_msg("joindreperm ln548", 0, 0x118);
            CB->select_pile(2, 2);
            CB->stop_joindre(-P_DELIA);
            CB->animation("SP10GAR01");
            CB->joindre_perm(P_DELIA, ST(ST_HERO));
        }
        CB->select_pile(0, 1);
    }
    /* 0x907A */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(V_DELIA_KILLED) = 5;
        CB->print_msg("pile 2 videe ln563", 0, 0x118);
        CB->select_pile(2, 2);
        CB->combat(P_DELIA, -1);
        ST(V_ENGAGED_PERSONA) = 0;
        CB->stop_joindre(-P_DELIA);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_DELIA, 0);
        CB->animation("SP10IMP51");
        CB->animation("P10_65535");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 5, 0);
        CB->divers_empile(DV_SCREEN, 6, 0);
        ST(L05_DUEL_MODE) = 2;
        /* 0x9175 */
        if (ST(ST_VIE(P_BOGDARAN)) == 1) {
            ST(L05_BOGDARAN_FIGHT_STEP) = 1;
            lieu_action_register("L05D77D", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
            CB->stop_joindre(-P_BOGDARAN);
            CB->combat(P_BOGDARAN, 0);
            ST(V_ENGAGED_PERSONA) = P_BOGDARAN;
        }
        CB->select_pile(0, 1);
    }
    /* 0x91DA */
    if (ST(L05_BOGDARAN_MET) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1 && CB->distance(2, 6) < 190 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(0, 2);
        ST(L05_BOGDARAN_RETURN) = 0;
        ST(L05_BOGDARAN_MET) = 1;
        ST(L05_BOGDARAN_TALKING) = 1;
        CB->stop_joindre(-P_BOGDARAN);
        /* 0x9264 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->bloque(BLOQUE_ALL);
            CB->evenement(2, -1);
            CB->joindre(-P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
            /* 0x92AE */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L05D05", P_ARKHAN1, "SP01DIA11", "Sar_100");
            }
            /* 0x92D9 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("L05D06", P_ARKHAN1, "SP01DIA12", "Sar_100");
            }
            CB->modify_var(&ST(L05_BOGDARAN_RETURN), 0);
            CB->animation("SP08PRE73");
            CB->animation("P08ARR03_10");
            CB->bloque(BLOQUE_NONE);
            /* 0x9333 */
            if (ST(L05_BOGDARAN_D13_STATE) != 0) {
                CB->evenement(2, 6000);
            }
            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9385 */
        if (ST(ST_HERO) == P_ARKHAN2 || ST(ST_HERO) == P_ARKHAN3) {
            /* 0x93A9 */
            if (ST(L05_BOGDARAN_STAGE) == 0 && ST(L05_FIRST_TALK_DONE) == 0) {
                ST(L05_BOGDARAN_RETURN) = 0;
                ST(L05_FIRST_TALK_DONE) = 1;
                CB->bloque(BLOQUE_ALL);
                CB->animation("SP08PRE73");
                CB->animation("P08ARR03_10");
                CB->joindre(-P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                /* 0x942D */
                if (CB->distance(2, 6) < 100) {
                    CB->animation("SP08MAR29");
                }
                /* 0x9447 */
                if (ST(ST_COMBAT_STANCE) == 1) {
                    ST(V_PLAYER_STRIKE) = -1;
                    CB->print_msg("dialogues en combat", 0, 0xF0);
                    CB->animation("SP08ESM01");
                    lieu_action_register("L05D11A", P_BOGDARAN, "SP08GAR01", "SP08ARR02_100");
                    lieu_action_register("L05D12B", P_BOGDARAN, "SP08GAR01", "SP08ARR02_100");
                } else {
                    lieu_action_register("L05D11A", P_BOGDARAN, "SP08DIA01", "SP08DIA01");
                    lieu_action_register("L05D12B", P_BOGDARAN, "SP08DIA64", "SP08GAR01_10");
                }
                CB->bloque(BLOQUE_NONE);
                CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                CB->select_pile(0, 1);
                return;
            }
            /* 0x951B */
            if (ST(L05_BOGDARAN_STAGE) != 0) {
                /* 0x952D */
                if (ST(L05_TALK_IN_FIGHT_OK) == 1) {
                    CB->bloque(BLOQUE_ALL);
                    /* 0x9552 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0x9564 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x9576 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D07", P_ARKHAN2, "SP02DIA13", "Sar_100");
                            }
                            /* 0x95A1 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D08", P_ARKHAN2, "SP02DIA14", "Sar_100");
                            }
                        }
                        /* 0x95CC */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0x95DE */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D09", P_ARKHAN3, "SP03DIA13", "Sar_100");
                            }
                            /* 0x9609 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D10", P_ARKHAN3, "SP03DIA14", "Sar_100");
                            }
                        }
                    }
                    /* 0x963C */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0x964E */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x9660 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D07", P_ARKHAN2, "SP02DIA11", "Sar_100");
                            }
                            /* 0x968B */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D08", P_ARKHAN2, "SP02DIA12", "Sar_100");
                            }
                        }
                        /* 0x96B6 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0x96C8 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L05D09", P_ARKHAN3, "SP03DIA11", "Sar_100");
                            }
                            /* 0x96F3 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L05D10", P_ARKHAN3, "SP03DIA12", "Sar_100");
                            }
                        }
                    }
                    CB->animation("SP10ARR01_10");
                    CB->modify_var(&ST(L05_DELIA_FOLLOW_STEP), 1);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre(-P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
                    CB->animation("SP08ARR03_10");
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                    CB->select_pile(0, 1);
                    return;
                } else {
                    /* 0x979D */
                    if (ST(L05_BOGDARAN_QUEST) != 4 || ST(L05_DELIA_FIGHT_PHASE) != 0) {
                        CB->animation("SP08PRE73");
                    }
                    CB->animation("P08ARR03_10");
                    CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
                    CB->select_pile(0, 1);
                    return;
                }
            }
        }
    }
    /* 0x97FC */
    if (ST(L05_BOGDARAN_MET) == 1 && ST(L05_BOGDARAN_RETURN) == 0 && ST(L05_BOGDARAN_FIGHT_STEP) == 3 && CB->distance(2, 6) > 260 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1 && ST(ST_HERO) == P_ARKHAN1 && ST(L05_BOGDARAN_LEFT) == 0) {
        CB->select_pile(0, 2);
        ST(L05_BOGDARAN_TALKING) = 0;
        ST(L05_BOGDARAN_RETURN) = 1;
        ST(L05_BOGDARAN_MET) = 0;
        CB->stop_joindre(-P_BOGDARAN);
        CB->link_objet(8, "OB88", 0x1B, 1);
        /* 0x98DC */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->joindre(-P_BOGDARAN, -1893, -60, 0xC738, -1);
            CB->animation("SP08ARR01_10");
        }
        /* 0x9912 */
        if (ST(ST_HERO) >= 2 && ST(L05_BOGDARAN_STAGE) == 1) {
            CB->joindre(-P_BOGDARAN, -2005, -98, 0x5167, -1);
            CB->animation("SP08ATT02_10");
        }
        CB->select_pile(0, 1);
    }
    /* 0x9967 */
    if (ST(L05_D13_STEP) == 1) {
        CB->stop_joindre(-P_BOGDARAN);
        ST(L05_D13_STEP) = 2;
        ST(L05_BOGDARAN_D13_STATE) = 2;
        lieu_action_register("L05D13A", P_BOGDARAN, "SP08ATT04", "SP08ARR03_20");
        lieu_action_register("L05D13B", P_BOGDARAN, "SP08ATT07", "SP08ARR03_20");
        CB->animation("Sar_20");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9A0E */
    if (ST(V_EVENT_2) == 1 && ST(L05_BOGDARAN_TALKING) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1) {
        /* 0x9A4D */
        if (ST(ST_SCRIPT_BUSY) == 0) {
            CB->stop_joindre(P_BOGDARAN);
            CB->bloque(BLOQUE_ALL);
            ST(V_EVENT_2) = 0;
            ST(L05_QUIP_PICK) = (ST(ST_GAME_TIME) % 3) + 1;
            switch (ST(L05_QUIP_PICK)) {   /* [ebp-4] copy, tests 1/2/3 */
            case 1: lieu_action_register("L05D14", P_BOGDARAN, "SP08DIA60", "SP08ARR03_20"); break;
            case 2: lieu_action_register("L05D15", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20"); break;
            case 3: lieu_action_register("L05D16", P_BOGDARAN, "SP08ATT07", "SP08ARR03_20"); break;
            default: break;
            }
            CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x9B56 */
        if (ST(ST_SCRIPT_BUSY) == 1) {
            ST(V_EVENT_2) = 0;
            CB->evenement(2, 4000);
        }
    }
    /* 0x9B87 */
    if (ST(V_EVENT_2) == 1 && ST(L05_BOGDARAN_TALKING) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_BOGDARAN)) == 1) {
        /* 0x9BC6 */
        if (ST(ST_SCRIPT_BUSY) == 0) {
            ST(V_EVENT_2) = 0;
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 1);
            ST(L05_QUIP_PICK) = (ST(ST_GAME_TIME) % 3) + 1;
            switch (ST(L05_QUIP_PICK)) {   /* [ebp-8] copy, tests 1/2/3 */
            case 1: CB->dialogue("L05D14", P_BOGDARAN); break;
            case 2: CB->dialogue("L05D15", P_BOGDARAN); break;
            case 3: CB->dialogue("L05D16", P_BOGDARAN); break;
            default: break;
            }
            CB->divers(DV_LOOK_AT_PLAYER, P_BOGDARAN, 0);
            CB->bloque(BLOQUE_NONE);
            CB->evenement(2, 12000);
        }
        /* 0x9CC7 */
        if (ST(ST_SCRIPT_BUSY) == 1) {
            ST(V_EVENT_2) = 0;
            CB->evenement(2, 4000);
        }
    }
    /* 0x9CF8 */
    if (ST(L05_WEAPON_GIVE_STEP) == 1) {
        ST(ST_PLAYER_WEAPON) = 40;
        ST(V_WEAPON_WARNED) = 1;
        lieu_action_register("L05D17", P_BOGDARAN, "SP08DIA03", "SP08ARR03_20");
        /* 0x9D41 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L05D18", P_ARKHAN2, "SG00DIA05", "Sar_10");
        }
        /* 0x9D6C */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L05D19", P_ARKHAN3, "SG00DIA05", "Sar_10");
        }
        lieu_action_register("L05D20A", P_BOGDARAN, "SP08DIA56", "SP08ARR03_20");
        lieu_action_register("L05D20B", P_BOGDARAN, "SP08DIA57", "SP08ARR03_20");
        lieu_action_register("L05D20C", P_BOGDARAN, "SP08DIA59", "SP08ARR03_20");
        ST(L05_WEAPON_GIVE_STEP) = 2;
        ST(L05_BOGDARAN_MET) = 1;
        ST(L05_BOGDARAN_QUEST) = 5;
        lieu_pre_scene(ST(ST_HERO), P_BOGDARAN, 40, 6, 2);
        CB->link_objet(8, "OB41", 0x1B, 0);
        CB->divers(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE41);
        CB->divers(DV_PERSONA_ITEMS, P_BOGDARAN, -OB_OBJ_BEFORE40);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("SP08ARR03_10");
        CB->joindre_perm(P_BOGDARAN, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9E99 */
    if (ST(L05_DELIA_D21_STEP) == 1 && !CB->on_joindre(P_DELIA)) {
        ST(L05_DELIA_D21_STEP) = 2;
        CB->animation("SP10ARR01_10");
        CB->dialogue("L05D21A", P_DELIA);
        CB->dialogue("L05D21B", P_DELIA);
        CB->dialogue("L05D21C", P_DELIA);
        CB->animation("Sar_10");
        CB->modify_var(&ST(L05_DELIA_FOLLOW_STEP), 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9F43 */
    if (ST(L05_DELIA_FOLLOW_STEP) == 1 && ST(ST_COMBAT_STANCE) == 0) {
        ST(L05_DELIA_FOLLOW_STEP) = 2;
        /* 0x9F73 */
        if (ST(ST_VIE(P_DELIA)) == 1 && ST(V_DELIA_PHASE) == 11) {
            CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
            CB->modify_var(&ST(L05_DELIA_FOLLOW_STEP), 3);
        }
    }
    /* 0x9FCB */
    if (ST(L05_DELIA_FOLLOW_STEP) == 3 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_DELIA)) == 1 && !CB->on_joindre(P_DELIA)) {
        ST(L05_DELIA_FOLLOW_STEP) = 4;
        CB->print_msg("joindreperm ln860", 0, 0x11D);
        CB->joindre_perm(P_DELIA, ST(ST_HERO));
    }
    /* 0xA045 */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->divers(DV_CAN_FIGHT, 9, 2) && !CB->divers(DV_WAS_HIT, P_BOGDARAN, 0) && !CB->direction_perso(P_BOGDARAN) && ST(ST_VIE(P_BOGDARAN)) == 1 && ST(ST_VIE(P_DELIA)) < 2 && ST(L05_D150_ARMED) == 0) {
        ST(L05_D150_ARMED) = 1;
        CB->joindre(P_BOGDARAN, 0, 0, 0, ST(ST_HERO));
        CB->print_msg("dialogue ln 927", 0, 0xF0);
        lieu_action_register("L05D150A", P_BOGDARAN, "SP08GAR01", "SP08GAR01_10");
        CB->modify_var(&ST(L05_D150_ARMED), 0);
        return;
    }
    /* 0xA147 */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(ST_SCRIPT_BUSY) == 0 && (ST(V_ENGAGED_PERSONA) == 0 || ST(V_ENGAGED_PERSONA) == P_BOGDARAN) && ST(ST_VIE(P_DELIA)) == 1 && (ST(V_DELIA_PHASE) == 10 || ST(V_DELIA_PHASE) == 11) && CB->direction_perso(P_DELIA) && CB->distance(6, 5) < 150) {
        CB->print_msg("acton ln2106", 0, 0xF0);
        ST(V_ENGAGED_PERSONA) = P_DELIA;
        ST(ST_SCRIPT_BUSY) = 1;
        CB->divers(DV_HP, P_DELIA, 1);
        ST(L05_DELIA_FOLLOW_STEP) = 10;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP10ESA01");
        CB->animation("P10GAR01_10");
        CB->dialogue("L45D04", P_DELIA);
        CB->animation("SP10ARR01_10");
        /* 0xA271 */
        if (ST(ST_VIE(P_BOGDARAN)) == 1) {
            lieu_action_register("L05D39A", P_BOGDARAN, "SP08DIA01", "SP08GAR01_10");
            CB->stop_joindre(-P_BOGDARAN);
            CB->combat(P_BOGDARAN, 0);
        }
        CB->bloque(BLOQUE_NONE);
        CB->stop_joindre(-P_DELIA);
        CB->combat(P_DELIA, 0);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x10012479).
 * Callback ingestion (0x100120FA..0x10012474), the trigger-table fill and
 * the unused dword_100251AC = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu05_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x100126A2);
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
        l05_trig00();   /* 0x100126A9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1001258E — the hero is dead: play the death ending */
    CB->divers_empile(DV_SCREEN, 1, 0);
    ST(V_DEATH_EVENT_SENT) = 1;
    if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0))
        CB->divers(DV_HP, P_ARKHAN0, 0);
    if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0))
        CB->divers(DV_HP, P_ARKHAN1, 0);
    if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0))
        CB->divers(DV_HP, P_ARKHAN2, 0);
    if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0))
        CB->divers(DV_HP, P_ARKHAN3, 0);
    if (ST(V_GUARD_FIGHT_DONE) == 0)
        CB->cinematique("SC22_500");
    if (ST(V_GUARD_FIGHT_DONE) != 0)
        CB->cinematique("SC23_500");
    CB->divers_empile(DV_END_GAME, 0, 0);
    return 1;
}

/* -------------------------------------------------------------------------
 * TICK (opcode -1, 0x100126FC): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x100120E0).
 * Jump table @ 0x10012779, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu05_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu05: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l05_trigger(arg);             break;  /* dword_10025040[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
