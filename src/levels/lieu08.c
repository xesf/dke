/*
 * lieu08.c - Native hand-port of LIEU08.DLL (level-8 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu08_trig.c (reached through
 * l08_trigger, see lieu08_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x1002A6C0, the state pointer global is dword_1003FC5C and
 * the trigger table is dword_1003FB70[42].
 *
 * event5 (sub_10025584), event6 (sub_10028EBC), lieu_action_register
 * (sub_1002AD8C) and lieu_pre_scene (sub_100194DC) are the shared
 * lieu_common.c. event4 is sub_1002450A. The annotator does not match it to
 * lieu_event4 because the byte table after the function (disassembled as
 * junk instructions) differs, but the code itself is instruction for
 * instruction the shared event4 (same 99 cases, same jump targets), so
 * lieu_event4 is used. tick_body (sub_10005530, 26 KB) is level-8 specific.
 */

#include <stdlib.h>

#include "lieu08_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10005530 (thunk 0x100010B9), level-8 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x5542 */
    if (ST(V_EVENT_39) == 1) {
        ST(V_EVENT_39) = 0;
        /* 0x5563 */
        if (ST(ST_VIE(34)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
            CB->select_pile(2, 1);
            CB->evenement(39, 500);
            /* 0x55A5 */
            if (!CB->random(2)) {
                CB->divers_empile(DV_BLOOD_AT_BONE, 34, 0x10);
            }
            /* 0x55CB */
            if (CB->random(2) == 1) {
                CB->divers_empile(DV_BLOOD_AT_BONE, 34, 0xE);
            }
            /* 0x55F1 */
            if (CB->random(2) == 2) {
                CB->divers_empile(DV_BLOOD_AT_BONE, 34, 0);
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0x5622 */
    if (ST(V_EVENT_41) == 1) {
        ST(V_EVENT_41) = 0;
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x5668 */
    if (ST(ST_HERO) == 4 && ST(V_WEAPON_WARNED) == 0) {
        ST(V_WEAPON_WARNED) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x56AB */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x56DC */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x5705 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x5731 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x575D */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x5781 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x579E */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x57CD */
    if (ST(V_KALHI_PHASE) == 0xC8 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_LOCKED_CAMERA) == 0 && CB->distance(1, 32) < 500) {
        ST(V_KALHI_PHASE) = 5;
        lieu_action_register("L08D220", P_KALHI33, "SP33DIA50", "SP33ARR01_50");
        CB->modify_var(&ST(L08_KALHI33_STEP), 6);
    }
    /* 0x5865 */
    if (CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI33)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_KALHI13, 1);
        CB->combat(P_KALHI13, -1);
        ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
        CB->stop_joindre(-P_KALHI13);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_KALHI13, 0);
        CB->animation("SP13IMP52");
        CB->animation("P13CAD");
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13CAD", 0x21, 0);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x5972 */
    if (CB->divers(DV_WAS_HIT, P_KALHI33, 0) && ST(ST_VIE(P_KALHI33)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 32) && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_KALHI33, 1);
        CB->combat(P_KALHI33, -1);
        ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
        CB->stop_joindre(-P_KALHI33);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_KALHI33, 0);
        CB->animation("SP33IMP52");
        CB->animation("P33CAD");
        CB->pos_perso(P_KALHI33, 0, 0, 0, 2, "P33CAD", 0x20, 0);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x5A7F */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 30) && ST(L08_DELIA_ROUTE) == 0) {
        CB->select_pile(2, 2);
        CB->divers_empile(DV_WAS_HIT, P_DELIA, 1);
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(V_DELIA_PHASE) = 0x14;
        ST(V_DELIA_KILLED) = 8;
        CB->combat(P_DELIA, -1);
        CB->stop_joindre(-P_DELIA);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_DELIA, 0);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0x1E, 0);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x5BA2 */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(V_KALHI_PHASE) == 5 && ST(V_ENGAGED_PERSONA) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 1);
        /* 0x5C00 */
        if (CB->distance(31, 1) > 300 && ST(ST_COMBAT_STANCE) == 1 && ST(L08_KALHI33_STEP) == 1) {
            ST(L08_KALHI33_STEP) = 0x64;
            CB->animation("SP13GAR27");
            CB->modify_var(&ST(L08_KALHI33_STEP), 0);
        }
        /* 0x5C68 */
        if (CB->distance(31, 1) < 110 && ST(L08_FOLLOWER_NEAR) == 0) {
            ST(L08_FOLLOWER_NEAR) = 1;
            ST(L08_KALHI13_CHAT_READY) = 1;
            CB->joindre_perm(-P_KALHI13, ST(ST_HERO));
            CB->stop_joindre(P_KALHI13);
            CB->pos_perso(P_KALHI13, CB->divers(DV_DESC_POS, P_KALHI13, 0), CB->divers(DV_DESC_POS, P_KALHI13, 1), CB->divers(DV_DESC_POS, P_KALHI13, 2), 1, "P13ARR01", 0x1F, 1);
            /* 0x5D0F */
            if (ST(L08_KALHI13_D220_PENDING) == 1) {
                ST(L08_KALHI13_D220_PENDING) = 0;
                CB->dialogue("L08D220", P_KALHI13);
            }
        }
        /* 0x5D3A */
        if (ST(L08_FOLLOWER_MODE) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0 && ST(L08_KALHI33_STEP) == 0) {
            /* 0x5D7F */
            if (CB->distance(31, 1) >= 140) {
                CB->divers(DV_RUN, P_KALHI13, 0);
                ST(L08_FOLLOWER_NEAR) = 0;
                CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
            }
            /* 0x5DD8 */
            if (CB->distance(31, 1) >= 300) {
                CB->divers(DV_RUN, P_KALHI13, 1);
                ST(L08_FOLLOWER_NEAR) = 0;
                CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0x5E35 */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 1);
        /* 0x5E8A */
        if (ST(L08_KALHI33_STEP) == 3 && ST(ST_LOCKED_CAMERA) != 0 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ((CB->distance(32, 1) > 250 && CB->divers(DV_ANGLE_TO, P_KALHI33, ST(ST_HERO)) < 0x1770 && CB->divers(DV_DESC_POS, P_KALHI33, 2) < 0xDAC0 && CB->divers(DV_DESC_POS, P_KALHI33, 2) > 0xB3B0) || (CB->distance(32, 1) > 250 && CB->distance(1, 33) > 400))) {
            ST(L08_KALHI33_STEP) = 6;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_KALHI33);
            CB->joindre_perm(P_KALHI33, ST(ST_HERO));
            ST(V_DLG_RANDOM) = CB->random(3);
            /* 0x5FC4 */
            if (ST(V_DLG_RANDOM) == 0) {
                CB->dialogue("L08D321", P_KALHI33);
            }
            /* 0x5FE6 */
            if (ST(V_DLG_RANDOM) == 1) {
                CB->dialogue("L08D322", P_KALHI33);
            }
            /* 0x6008 */
            if (ST(V_DLG_RANDOM) == 2) {
                CB->dialogue("L08D323", P_KALHI33);
            }
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x6037 */
        if (ST(L08_KALHI33_STEP) == 6 && (CB->distance(32, 1) < 150 || ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD)) {
            CB->animation("SP33ARR01");
            /* 0x6081 */
            if (ST(V_EVENT15_STEP) != 0) {
                ST(V_EVENT15_STEP) = 0;
                ST(L08_KALHI33_CHAT_READY) = 0;
                ST(L08_KALHI33_POS_SAVED) = 0;
            }
            ST(L08_KALHI33_STEP) = 3;
            CB->stop_joindre(-P_KALHI33);
            CB->joindre(P_KALHI33, 2690, -1625, 0xC350, -1);
            return;
        }
        CB->select_pile(0, 1);
    }
    /* 0x6108 */
    if (ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI33)) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 1);
        /* 0x614E */
        if (ST(L08_KALHI33_STEP) == 1 && !CB->on_joindre(P_KALHI33) && CB->distance(32, 23) < 100) {
            ST(L08_KALHI33_STEP) = 2;
            CB->stop_joindre(-P_KALHI33);
            CB->retire_zone(23);
            CB->animation("SP33ARR01_50");
            CB->joindre_perm(P_KALHI33, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x61DA */
        if (ST(L08_KALHI33_STEP) == 2 && CB->distance(32, 1) < 150) {
            CB->select_pile(1, 2);
            ST(L08_KALHI33_STEP) = 0x63;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_KALHI33);
            CB->joindre(ST(ST_HERO), 0, 0, 0, P_KALHI33);
            lieu_action_register("L08D209A", P_KALHI33, "SP33ARR01", "SP33ARR01_50");
            lieu_action_register("L08D209B", P_KALHI33, "SP33ARR01", "SP33ARR01_50");
            CB->modify_var(&ST(L08_KALHI33_STEP), 3);
            CB->bloque(BLOQUE_NONE);
            CB->joindre(P_KALHI33, 2690, -1625, 0xC350, -1);
        }
        /* 0x62C2 */
        if (ST(L08_KALHI33_STEP) == 3 && !CB->on_joindre(P_KALHI33) && CB->distance(32, 10) < 100) {
            ST(L08_KALHI33_STEP) = 0xA;
        }
        CB->select_pile(0, 1);
    }
    /* 0x6319 */
    if (ST(ST_COMBAT_STANCE) == 0 && CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) == 1 && ST(V_KALHI_PHASE) == 6 && ST(L08_COMPANION_REUNION) == 1 && CB->distance(31, 1) < 120 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L08_COMPANION_REUNION) = 2;
        ST(V_KALHI_PHASE) = 5;
        CB->animation("SP13LEV01");
        lieu_action_register("L08D335A", P_KALHI13, "P13DIA60", "SP13ARR01_10");
        lieu_action_register("L08D335B", P_KALHI13, "P13DIA63", "SP13ARR01_10");
        lieu_action_register("L08D335C", P_KALHI13, "P13DIA64", "SP13ARR01_10");
        ST(V_KALHI13_TALK_C) = 2;
        CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x6487 */
    if (ST(ST_COMBAT_STANCE) == 0 && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(ST_VIE(P_KALHI33)) == 1 && ST(V_KALHI_PHASE) == 6 && ST(L08_COMPANION_REUNION) == 1 && CB->distance(32, 1) < 120 && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L08_COMPANION_REUNION) = 2;
        ST(V_KALHI_PHASE) = 5;
        ST(V_KALHI13_TALK_C) = 2;
        CB->animation("SP33LEV01");
        lieu_action_register("L08D335A", P_KALHI33, "P33DIA60", "SP33ARR01_10");
        lieu_action_register("L08D335B", P_KALHI33, "P33DIA59", "SP33ARR01_10");
        lieu_action_register("L08D335C", P_KALHI33, "P33DIA55", "SP33ARR01_10");
        CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(L08_KALHI33_STEP), 3);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x6617 */
    if (ST(ST_HERO) != 4 && ST(L08_KALHI13_CHAT_READY) == 1 && CB->divers(DV_IDLE_TIME, 0, 0) >= 10000 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 31) && ST(V_ENGAGED_PERSONA) == 0 && ST(V_GUARD_FIGHT_DONE) != 2 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
        CB->select_pile(1, 1);
        ST(L08_KALHI13_CHAT_READY) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x66DD */
        if (ST(L08_KALHI13_CHAT_STAGE) == 2) {
            lieu_action_register("L08D157", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
        }
        /* 0x6708 */
        if (ST(L08_KALHI13_CHAT_STAGE) == 1) {
            ST(L08_KALHI13_CHAT_STAGE) = 2;
            lieu_action_register("L08D156", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
        }
        /* 0x6742 */
        if (ST(L08_KALHI13_CHAT_STAGE) == 0) {
            ST(L08_KALHI13_CHAT_STAGE) = 1;
            lieu_action_register("L08D155", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        }
        CB->divers(DV_IDLE_TIME, 1, 0);
        CB->modify_var(&ST(L08_KALHI13_CHAT_READY), 1);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x67BC */
    if (ST(V_KALHI_PHASE) > 6 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(L08_G10_STATE) != 0x14 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 11) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 10) && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_LOCKED_CAMERA) == 0 && CB->distance(1, 33) < 600) {
        ST(L08_G10_STATE) = 0x14;
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN10);
        CB->joindre(P_GUARDIAN10, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L08D310", P_GUARDIAN10, "SP50GAR26", "SP50GAR03_50");
        CB->combat(P_GUARDIAN10, 0);
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x690A */
    if (ST(L08_G10_STATE) == 0x14 && ST(V_GUARD_FIGHT_DONE) != 2 && !CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && !CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0)) {
        /* 0x696A */
        if (CB->divers(DV_WAS_HIT, P_GUARDIAN10, 0) && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, P_GUARDIAN10, 1);
            CB->combat(P_GUARDIAN10, -1);
            CB->divers(DV_CINEMATIC, 0, 0);
            /* 0x69E3 */
            if ((CB->divers(DV_HP, P_GUARDIAN10, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_GUARDIAN10, (CB->divers(DV_HP, P_GUARDIAN10, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_GUARDIAN10, 1);
            }
            /* 0x6A44 */
            if (CB->divers(DV_HP, P_GUARDIAN10, -1) <= 1) {
                ST(ST_VIE(P_GUARDIAN10)) = VIE_DEAD;
                CB->stop_joindre(-P_GUARDIAN10);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN10, 0);
                CB->animation("SP50IMP53");
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_GUARDIAN10);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN10, 0);
                CB->animation("SP50IMP13");
                CB->animation("P50IMP48");
                CB->animation("P50GAR03");
                CB->combat(P_GUARDIAN10, 0);
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
            }
            CB->select_pile(0, 1);
        }
        /* 0x6B08 */
        if (ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
            ST(L08_G10_STATE) = 0x1E;
            ST(V_ENGAGED_PERSONA) = 0;
        }
    }
    /* 0x6B35 */
    if (ST(ST_HERO) != 4 && ST(V_DELIA_PHASE) == 7 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L08_DELIA_ROUTE) == 0) {
        CB->select_pile(2, 1);
        /* 0x6B9C */
        if (ST(ST_LOCKED_CAMERA) == 3 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -650 && ST(L08_DELIA_TRIG8_DONE) == 0) {
            ST(L08_DELIA_TRIG8_DONE) = 1;
            l08_trig08();
            return;
        }
        /* 0x6C03 */
        if (CB->distance(30, 1) < 100 && ST(L08_FOLLOWER_MODE) != 0x63 && ST(L08_FOLLOWER_NEAR) == 0) {
            ST(L08_FOLLOWER_NEAR) = 1;
            CB->joindre_perm(-P_DELIA, ST(ST_HERO));
            CB->stop_joindre(P_DELIA);
            CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 0x1E, 0);
        }
        /* 0x6CAD */
        if (ST(L08_FOLLOWER_MODE) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0x6CE0 */
            if (CB->distance(30, 1) >= 180) {
                CB->divers(DV_RUN, P_DELIA, 0);
                ST(L08_FOLLOWER_NEAR) = 0;
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0x6D39 */
            if (CB->distance(30, 1) >= 300) {
                ST(L08_FOLLOWER_NEAR) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
        /* 0x6D86 */
        if (ST(L08_FOLLOWER_MODE) == 0x63 && ST(ST_VIE(34)) == VIE_DEAD) {
            ST(L08_FOLLOWER_MODE) = 1;
        }
        CB->select_pile(0, 1);
    }
    /* 0x6DC3 */
    if (ST(ST_HERO) != 4 && ST(V_KALHI_PHASE) == 5 && ST(ST_VIE(P_KALHI13)) == 1 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        /* 0x6E3B */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
            /* 0x6E4D */
            if (ST(L08_KALHI13_STAYS) == 1) {
                ST(L08_KALHI13_STAYS) = 0;
            }
            /* 0x6E6B */
            if (ST(L08_KALHI13_ZONE_STEP) == 3) {
                ST(L08_FOLLOWER_MODE) = 1;
                ST(L08_KALHI13_CHAT_READY) = 1;
            }
            /* 0x6E92 */
            if (ST(L08_KALHI13_ZONE_STEP) == 0 || ST(L08_KALHI13_ZONE_STEP) == 3) {
                ST(L08_KALHI13_ZONE_STEP) = 1;
            }
        }
        /* 0x6ED0 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -675 && ST(L08_KALHI13_ZONE_STEP) == 1 && ST(ST_LOCKED_CAMERA) == 3) {
            ST(L08_KALHI13_ZONE_STEP) = 2;
        }
        CB->select_pile(0, 1);
        /* 0x6F19 */
        if (ST(L08_KALHI13_ZONE_STEP) == 2) {
            CB->select_pile(1, 1);
            ST(L08_KALHI13_ZONE_STEP) = 3;
            ST(L08_FOLLOWER_MODE) = 0x14;
            ST(L08_KALHI13_STAYS) = 1;
            ST(L08_KALHI13_CHAT_READY) = 0xA;
            CB->stop_joindre(-P_KALHI13);
            CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L08D158", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->select_pile(0, 1);
        }
    }
    /* 0x6FCF */
    if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_KALHI33)) == VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(V_KALHI_KILLED) = 1;
        /* 0x7014 */
        if (ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
        } else {
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
        }
    }
    /* 0x7040 */
    if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(V_KALHI_KILLED) = 1;
        /* 0x7085 */
        if (ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
        } else {
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
        }
    }
    /* 0x70B4 */
    if (ST(ST_HERO) != 4 && (ST(V_DELIA_PHASE) == 7 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(L08_DELIA_ROUTE) == 1)) {
        /* 0x710E */
        if (ST(L08_CREATURE34_ENGAGED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(L08_CREATURE34_SEEN) == 0 && (CB->distance(1, 34) < 600 || (ST(V_CRI_KEY) == 1 && ST(ST_LOCKED_CAMERA) == 0)) && ST(ST_VIE(34)) != VIE_DEAD) {
            ST(L08_CREATURE34_ENGAGED) = 1;
            ST(L08_CREATURE34_SEEN) = 1;
            CB->joindre(-34, 1237, -1327, -1, -1);
            CB->animation("SP34GAR01");
            CB->combat(34, 0);
            ST(V_ENGAGED_PERSONA) = 34;
        }
        /* 0x71ED */
        if (ST(ST_VIE(34)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) == 34 || ST(V_ENGAGED_PERSONA) == 0x154)) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
        /* 0x7229 */
        if (ST(V_ENGAGED_PERSONA) == 34 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -580) {
            CB->select_pile(2, 2);
            ST(V_ENGAGED_PERSONA) = 0x154;
            CB->combat(34, -1);
            CB->animation("SP34IMP06");
            CB->animation("P34GAR01");
            CB->select_pile(0, 1);
        }
        /* 0x72AD */
        if (ST(V_ENGAGED_PERSONA) == 0x154 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -680) {
            CB->select_pile(2, 2);
            ST(V_ENGAGED_PERSONA) = 34;
            CB->combat(34, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x7315 */
    if (ST(V_ENGAGED_PERSONA) == 0 && ST(V_PLAYER_STRIKE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7)) {
        /* 0x7398 */
        if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && !CB->divers(DV_WAS_HIT, P_GUARDIAN3, 0) && !CB->divers(DV_WAS_HIT, P_GUARDIAN4, 0) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 3)) {
            ST(V_ENGAGED_PERSONA) = 0x10F8;
            CB->select_pile(1, 1);
            CB->combat(P_GUARDIAN3, 0);
            CB->select_pile(2, 1);
            CB->combat(P_GUARDIAN4, 0);
            CB->dialogue("L08D303", P_GUARDIAN3);
            CB->select_pile(0, 1);
        }
        /* 0x74D8 */
        if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && !CB->divers(DV_WAS_HIT, P_GUARDIAN5, 0) && !CB->divers(DV_WAS_HIT, P_GUARDIAN6, 0) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 5)) {
            ST(V_ENGAGED_PERSONA) = 0x11C2;
            CB->select_pile(1, 1);
            CB->combat(P_GUARDIAN5, 0);
            CB->select_pile(2, 1);
            CB->combat(P_GUARDIAN6, 0);
            CB->dialogue("L08D304", P_GUARDIAN6);
            CB->select_pile(0, 1);
        }
        /* 0x7618 */
        if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && ST(L08_ARENA_UNLOCKED) != 0 && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD && ST(ST_VIE(P_GUARDIAN8)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && !CB->divers(DV_WAS_HIT, P_GUARDIAN7, 0) && !CB->divers(DV_WAS_HIT, P_GUARDIAN8, 0) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 7)) {
            ST(V_ENGAGED_PERSONA) = 0x128C;
            CB->select_pile(1, 1);
            CB->stop_joindre(-P_GUARDIAN7);
            CB->combat(P_GUARDIAN7, 0);
            CB->select_pile(2, 1);
            /* 0x7741 */
            if (CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN8, 0)) {
                ST(L08_G8_STATE) = 0xA;
                CB->retire_zone(7);
                CB->pos_perso(P_GUARDIAN8, 0, 0, 0, 4, "", -1, 0);
            }
            CB->dialogue("L08D305", P_GUARDIAN7);
            CB->select_pile(0, 1);
        }
    }
    /* 0x77A6 */
    if ((ST(V_DELIA_PHASE) == 7 || (ST(V_DELIA_PHASE) == 0x14 && ST(V_DELIA_KILLED) == 8) || ST(L08_DELIA_ROUTE) == 1) && CB->divers(DV_WAS_HIT, 34, 0) && ST(ST_VIE(34)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, 34, 1);
        CB->combat(34, -1);
        /* 0x7849 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = 34;
        }
        /* 0x787B */
        if ((CB->divers(DV_HP, 34, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, 34, (CB->divers(DV_HP, 34, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, 34, 1);
        }
        /* 0x78DC */
        if (CB->divers(DV_HP, 34, -1) <= 1) {
            ST(ST_VIE(34)) = VIE_DEAD;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->stop_joindre(-34);
            CB->divers_empile(DV_BLOOD_AT_BONE, 34, 0);
            CB->animation("SP34IMP51");
            CB->animation("P34CAD");
            CB->pos_perso(34, 0, 0, 0, 2, "P34CAD", 0x22, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-34);
            CB->divers_empile(DV_BLOOD_AT_BONE, 34, 0);
            CB->animation("SP34IMP06");
            CB->animation("P34GAR01");
            CB->combat(34, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x79BC */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x7A42 */
        if (CB->divers(DV_WAS_HIT, P_GUARDIAN5, 0) && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, P_GUARDIAN5, 1);
            CB->combat(P_GUARDIAN5, -1);
            /* 0x7AAC */
            if ((CB->divers(DV_HP, P_GUARDIAN5, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_GUARDIAN5, (CB->divers(DV_HP, P_GUARDIAN5, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_GUARDIAN5, 1);
            }
            /* 0x7B0D */
            if (CB->divers(DV_HP, P_GUARDIAN5, -1) <= 1) {
                ST(ST_VIE(P_GUARDIAN5)) = VIE_DEAD;
                CB->stop_joindre(-P_GUARDIAN5);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN5, 0);
                CB->animation("SP45IMP53");
                CB->animation("P45CAD");
                CB->pos_perso(P_GUARDIAN5, 0, 0, 0, 2, "P45CAD", 4, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_GUARDIAN5);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN5, 0);
                CB->animation("SP45IMP13");
                CB->animation("P45IMP48");
                CB->animation("P45GAR03");
                /* 0x7BD2 */
                if (ST(V_ENGAGED_PERSONA) != 0x11C2) {
                    CB->combat(P_GUARDIAN5, 0);
                    CB->select_pile(2, 1);
                    CB->combat(P_GUARDIAN6, 0);
                    CB->select_pile(1, 1);
                    ST(V_ENGAGED_PERSONA) = 0x11C2;
                } else {
                    CB->combat(P_GUARDIAN5, 0);
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0x7C51 */
        if (CB->divers(DV_WAS_HIT, P_GUARDIAN6, 0) && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            CB->divers_empile(DV_WAS_HIT, P_GUARDIAN6, 1);
            CB->combat(P_GUARDIAN6, -1);
            /* 0x7CBB */
            if ((CB->divers(DV_HP, P_GUARDIAN6, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_GUARDIAN6, (CB->divers(DV_HP, P_GUARDIAN6, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_GUARDIAN6, 1);
            }
            /* 0x7D1C */
            if (CB->divers(DV_HP, P_GUARDIAN6, -1) <= 1) {
                ST(ST_VIE(P_GUARDIAN6)) = VIE_DEAD;
                CB->stop_joindre(-P_GUARDIAN6);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN6, 0);
                CB->animation("SP46IMP53");
                CB->animation("P46CAD");
                CB->pos_perso(P_GUARDIAN6, 0, 0, 0, 2, "P46CAD", 5, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_GUARDIAN6);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN6, 0);
                CB->animation("SP46IMP13");
                CB->animation("P46IMP48");
                CB->animation("P46GAR03");
                /* 0x7DE1 */
                if (ST(V_ENGAGED_PERSONA) != 0x11C2) {
                    CB->combat(P_GUARDIAN6, 0);
                    CB->select_pile(1, 1);
                    CB->combat(P_GUARDIAN5, 0);
                    CB->select_pile(0, 1);
                    ST(V_ENGAGED_PERSONA) = 0x11C2;
                } else {
                    CB->combat(P_GUARDIAN6, 0);
                }
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0x7E55 */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_UNLOCKED) != 0 && CB->divers(DV_WAS_HIT, P_GUARDIAN7, 0) && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD) {
        ST(ST_VIE(P_GUARDIAN7)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_GUARDIAN7, -1);
        CB->stop_joindre(-P_GUARDIAN7);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN7, 0);
        CB->animation("SP47IMP55");
        CB->animation("P47CAD");
        CB->pos_perso(P_GUARDIAN7, 0, 0, 0, 2, "P47CAD", 6, 0);
        CB->select_pile(2, 2);
        /* 0x7F95 */
        if (CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN8, 0)) {
            ST(L08_G8_STATE) = 0xA;
            CB->retire_zone(7);
            CB->pos_perso(P_GUARDIAN8, 0, 0, 0, 4, "", -1, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x7FEA */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && ST(ST_VIE(P_GUARDIAN8)) != VIE_DEAD && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN7, 0) && ST(L08_ARENA_UNLOCKED) != 0) {
        /* 0x8066 */
        if (CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN8, 0)) {
            /* 0x8078 */
            if (ST(V_EVENT_33) == 1) {
                ST(L08_G7_STEP) = 0xA;
                ST(V_EVENT_33) = 0;
                CB->select_pile(1, 1);
                CB->dialogue("L08D147", P_GUARDIAN8);
                CB->joindre(-P_GUARDIAN8, -119, -1317, -1, -1);
                CB->joindre_perm(P_GUARDIAN7, P_GUARDIAN8);
                CB->dialogue("L08D148", P_GUARDIAN7);
                CB->change_camera(99, CAM_AUTO);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
            }
            /* 0x812F */
            if ((CB->distance(7, 35) > 400 && ST(L08_G8_STATE) == 5) || (ST(V_ENGAGED_PERSONA) != 0 && ST(L08_G8_STATE) == 5)) {
                ST(L08_G8_STATE) = 0xA;
                CB->select_pile(2, 2);
                CB->retire_zone(7);
                CB->pos_perso(P_GUARDIAN8, 0, 0, 0, 4, "", -1, 0);
                CB->select_pile(0, 1);
            }
            /* 0x81D0 */
            if (CB->distance(6, 7) > 750 && ST(L08_G7_STEP) == 0xA && ST(V_ENGAGED_PERSONA) == 0) {
                ST(L08_G7_STEP) = 0x63;
                ST(L08_G8_STATE) = 5;
                CB->select_pile(1, 1);
                CB->stop_joindre(-P_GUARDIAN7);
                CB->animation("SP47MAR20");
                CB->animation("P47MAR20");
                CB->animation("P47MAR20");
                CB->joindre(-P_GUARDIAN7, 734, -1438, 0xCA6D, -1);
                CB->animation("P47ARR04_50");
                CB->modify_var(&ST(L08_G7_STEP), 0x6F);
                CB->select_pile(0, 1);
            }
        }
        /* 0x82A5 */
        if (ST(V_ENGAGED_PERSONA) == 0 && CB->divers(DV_ANGLE_TO, P_GUARDIAN7, ST(ST_HERO)) < 0x4E20 && CB->distance(1, 6) < 500 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 35) && (ST(L08_G7_STEP) == 0x6F || ST(V_GUARDIAN8_STAGE) == 2) && ST(ST_VIE(P_GUARDIAN7)) != VIE_DEAD && ST(ST_LOCKED_CAMERA) == 1) {
            ST(L08_G7_STEP) = 3;
            CB->select_pile(1, 2);
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN7;
            /* 0x838D */
            if (CB->on_joindre(P_GUARDIAN7)) {
                CB->stop_joindre(-P_GUARDIAN7);
            }
            CB->animation("SP47GAR05");
            CB->bloque(BLOQUE_ALL);
            CB->dialogue("L08D149", P_GUARDIAN7);
            CB->combat(P_GUARDIAN7, 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
    }
    /* 0x83F2 */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        /* 0x8491 */
        if (ST(L08_G5_ALERT) == 0 && CB->distance(1, 4) < 500 && CB->divers(DV_ANGLE_TO, P_GUARDIAN5, ST(ST_HERO)) > 0xBB8 && ST(ST_LOCKED_CAMERA) == 3) {
            ST(L08_G5_ALERT) = 0xA;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN5, 1);
            CB->joindre(-P_GUARDIAN5, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L08D141", P_GUARDIAN5, "SP45GAR26", "SP45GAR03");
            CB->joindre_perm(P_GUARDIAN5, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x8594 */
        if ((ST(L08_G5_ALERT) == 2 || ST(L08_G5_ALERT) == 0xA) && ST(ST_HERO) != 4 && CB->distance(1, 4) > 350 && ST(V_ENGAGED_PERSONA) == 0 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -710) {
            CB->select_pile(1, 1);
            CB->stop_joindre(-P_GUARDIAN5);
            /* 0x8632 */
            if (ST(L08_G5_ALERT) == 0xA) {
                CB->animation("SP45GAR27");
            }
            CB->animation("P45ARR03");
            CB->joindre(-P_GUARDIAN5, -119, -1317, 0xC283, -1);
            ST(L08_G5_ALERT) = 1;
            CB->select_pile(0, 1);
        }
    }
    /* 0x8692 */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x8710 */
        if (ST(L08_G6_APPROACH) == 0 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && (CB->divers(DV_ANGLE_TO, P_GUARDIAN6, ST(ST_HERO)) < 0x2000 || CB->divers(DV_ANGLE_TO, P_GUARDIAN6, ST(ST_HERO)) > 0xE000) && CB->distance(1, 5) < 800 && ST(ST_LOCKED_CAMERA) != 5) {
            CB->select_pile(2, 1);
            /* 0x87CE */
            if (ST(ST_LOCKED_CAMERA) == 1 && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 876 && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 776 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < -1218 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > -1318) {
                CB->change_camera(0, CAM_LOCKED);
                CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) - 50), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            }
            CB->bloque(BLOQUE_ALL);
            ST(L08_G6_PATROL_STEP) = 0xA;
            CB->divers(DV_RUN, P_GUARDIAN6, 1);
            ST(L08_G6_APPROACH) = 0xA;
            CB->stop_joindre(-P_GUARDIAN6);
            CB->animation("SP46GAR26");
            CB->animation("P46GAR03");
            CB->joindre(-P_GUARDIAN6, -1, -1, -1, ST(ST_HERO));
            CB->dialogue("L08D124", P_GUARDIAN6);
            CB->change_camera(99, CAM_AUTO);
            CB->select_pile(0, 1);
        }
        /* 0x898B */
        if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(L08_G6_TALK_DONE) == 0) {
            /* 0x89BB */
            if (ST(L08_G6_PATROL_STEP) == 0 && CB->distance(5, 35) < 25) {
                ST(L08_G6_PATROL_STEP) = 1;
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_GUARDIAN6);
                CB->joindre(-P_GUARDIAN6, 1214, -1310, -1, -1);
                CB->select_pile(0, 1);
            }
            /* 0x8A30 */
            if (ST(L08_G6_PATROL_STEP) == 1 && CB->distance(5, 36) < 25) {
                ST(L08_G6_PATROL_STEP) = 0;
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_GUARDIAN6);
                CB->joindre(-P_GUARDIAN6, 1614, -1310, -1, -1);
                CB->select_pile(0, 1);
            }
        }
        /* 0x8AA8 */
        if (ST(L08_G6_APPROACH) == 2 && CB->distance(1, 5) > 350 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
            ST(L08_G6_APPROACH) = 1;
            CB->select_pile(2, 1);
            CB->divers(DV_RUN, P_GUARDIAN6, 0);
            CB->stop_joindre(-P_GUARDIAN6);
            CB->joindre(-P_GUARDIAN6, 1214, -1310, -1, -1);
            ST(L08_G6_PATROL_STEP) = 1;
            CB->select_pile(0, 1);
        }
    }
    /* 0x8B64 */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_LOCKED) == 0 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_HERO) != 4) {
        /* 0x8C06 */
        if ((ST(V_GUARDIAN4_STAGE) == 0 || ST(V_GUARDIAN4_STAGE) == 1) && (ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD || ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD) && ST(V_ENGAGED_PERSONA) != 0) {
            /* 0x8C5D */
            if (ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD) {
                ST(V_ENGAGED_PERSONA) = 0;
            }
            ST(V_ZED84_TALK_STAGE) = 1;
        }
        /* 0x8C9C */
        if (ST(V_GUARDIAN4_STAGE) == 2 && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_ZED84_TALK_STAGE) = 1;
        }
        /* 0x8CEA */
        if (ST(V_GUARDIAN4_STAGE) == 4 && ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
    }
    /* 0x8D3B */
    if (ST(V_EVENT_6) == 1 && ST(V_EVENT_10) == 1 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_LOCKED) == 0 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_HERO) != 4) {
        /* 0x8DDD */
        if (ST(ST_VIE(P_GUARDIAN5)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN6)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_ZED84_TALK_STAGE) = 0xA;
        }
        /* 0x8E2B */
        if (ST(ST_VIE(P_GUARDIAN5)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(V_ZED84_TALK_STAGE) == 0) {
            ST(V_ZED84_TALK_STAGE) = 0xF;
        }
        /* 0x8E6D */
        if (ST(ST_VIE(P_GUARDIAN6)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(V_ZED84_TALK_STAGE) == 0) {
            ST(V_ZED84_TALK_STAGE) = 0x10;
        }
    }
    /* 0x8EAF */
    if (ST(V_EVENT_6) == 2 && ST(V_EVENT_10) == 2 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(L08_ARENA_LOCKED) == 0 && ST(ST_HERO) != 4 && ST(L08_ARENA_UNLOCKED) != 0) {
        /* 0x8F51 */
        if (ST(L08_G8_STATE) != 0xA && ST(ST_VIE(P_GUARDIAN7)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN8)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_ZED84_TALK_STAGE) = 0x19;
            ST(L08_G7_DEAD_FLAG) = 0x63;
        }
        /* 0x8FC0 */
        if ((ST(L08_G8_STATE) == 0xA || (ST(L08_G8_STATE) == 0 && ST(V_GUARDIAN8_STAGE) == 1)) && ST(ST_VIE(P_GUARDIAN7)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_ZED84_TALK_STAGE) = 0x14;
            ST(L08_G7_DEAD_FLAG) = 0x63;
        }
    }
    /* 0x9041 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x905F */
        if (ST(L08_ARENA_ROUND_CMD) == 0x2B) {
            CB->select_pile(1, 1);
            ST(L08_ARENA_ROUND_CMD) = 0x63;
            CB->stop_joindre(P_GUARDIAN3);
            lieu_action_register("L08D01", P_GUARDIAN3, "SP43GAR03", "SP43GAR03");
            CB->combat(P_GUARDIAN3, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            /* 0x90DD */
            if (ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN4, 0)) {
                ST(V_ENGAGED_PERSONA) = 0x10F8;
                CB->combat(P_GUARDIAN4, 0);
            } else {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN3;
            }
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x914A */
        if (ST(L08_ARENA_ROUND_CMD) == 0x2C) {
            CB->select_pile(2, 1);
            ST(L08_ARENA_ROUND_CMD) = 0x63;
            CB->stop_joindre(P_GUARDIAN4);
            lieu_action_register("L08D02", P_GUARDIAN4, "SP44GAR02", "SP44GAR02");
            CB->combat(P_GUARDIAN4, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            /* 0x91C8 */
            if (ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN3, 0)) {
                ST(V_ENGAGED_PERSONA) = 0x10F8;
                CB->combat(P_GUARDIAN3, 0);
            } else {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
            }
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9235 */
        if (ST(L08_ARENA_ROUND_CMD) == 0x2D) {
            CB->select_pile(1, 1);
            ST(L08_ARENA_ROUND_CMD) = 0x63;
            CB->stop_joindre(P_GUARDIAN5);
            CB->dialogue("L08D270", P_GUARDIAN5);
            CB->combat(P_GUARDIAN5, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_ENGAGED_PERSONA), P_GUARDIAN5);
            CB->modify_var(&ST(V_CRI_USED), 0);
            /* 0x92BE */
            if (ST(ST_VIE(P_GUARDIAN6)) != VIE_DEAD) {
                ST(V_ENGAGED_PERSONA) = 0x11C2;
                CB->combat(P_GUARDIAN6, 0);
            } else {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN5;
            }
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9311 */
        if (ST(L08_ARENA_ROUND_CMD) == 0x2E) {
            CB->select_pile(2, 1);
            ST(L08_ARENA_ROUND_CMD) = 0x63;
            CB->stop_joindre(P_GUARDIAN6);
            lieu_action_register("L08D140", P_GUARDIAN6, "SP46GAR03", "SP46GAR03");
            CB->combat(P_GUARDIAN6, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_ENGAGED_PERSONA), P_GUARDIAN6);
            CB->modify_var(&ST(V_CRI_USED), 0);
            /* 0x93A3 */
            if (ST(ST_VIE(P_GUARDIAN5)) != VIE_DEAD) {
                ST(V_ENGAGED_PERSONA) = 0x11C2;
                CB->combat(P_GUARDIAN5, 0);
            } else {
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN6;
            }
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x93F9 */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x9489 */
        if (ST(V_GUARDIAN4_STAGE) == 0) {
            /* 0x949B */
            if (ST(L08_G3_PATROL_ARMED) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 2) > 220) {
                ST(L08_G3_PATROL_ARMED) = 0;
                ST(L08_G3_PATROL_STEP) = 0xA;
                ST(L08_G3_STANCE_TALK) = 0;
                CB->select_pile(1, 2);
                CB->select_pile(1, 1);
                CB->stop_joindre(-P_GUARDIAN3);
                /* 0x952F */
                if (ST(L08_G8_STATE) == 1) {
                    ST(L08_G8_STATE) = 0;
                    CB->animation("SP43GAR27");
                    CB->animation("P43ARR03");
                }
                CB->joindre(-P_GUARDIAN3, 381, -1562, 0x9088, -1);
                CB->animation("SP43ATT10");
                CB->animation("P43ATT03");
                CB->animation("P43ATT01");
                CB->modify_var(&ST(L08_G3_PATROL_STEP), 1);
                CB->evenement(1, 7500);
                CB->select_pile(0, 1);
            }
            /* 0x95D7 */
            if (ST(V_EVENT_1) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_EVENT_1) = 0;
                /* 0x9607 */
                if (ST(L08_G3_PATROL_STEP) == 1) {
                    ST(L08_G3_PATROL_STEP) = 0;
                    CB->select_pile(1, 1);
                    CB->animation("SP43ATT02");
                    CB->animation("P43ATT09");
                    CB->animation("P43ARR03");
                    ST(L08_PATROL_PICK) = CB->random(3);
                    /* 0x966A */
                    if (ST(L08_PATROL_PICK) == 0) {
                        CB->joindre(-P_GUARDIAN3, 506, -1572, 0xA882, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 2);
                    }
                    /* 0x96B7 */
                    if (ST(L08_PATROL_PICK) == 1) {
                        CB->joindre(-P_GUARDIAN3, 797, -777, 0xFED9, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 3);
                    }
                    /* 0x9704 */
                    if (ST(L08_PATROL_PICK) == 2) {
                        CB->joindre(-P_GUARDIAN3, -127, -1793, 0x4F3E, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 4);
                    }
                    CB->evenement(1, 1000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9776 */
                if (ST(L08_G3_PATROL_STEP) == 2) {
                    ST(L08_G3_PATROL_STEP) = 0;
                    CB->select_pile(1, 1);
                    CB->animation("SP43ATT08");
                    CB->animation("P43ARR03");
                    ST(L08_PATROL_PICK) = CB->random(3);
                    /* 0x97CE */
                    if (ST(L08_PATROL_PICK) == 0) {
                        CB->joindre(-P_GUARDIAN3, 381, -1562, 0x9088, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 5);
                    }
                    /* 0x981B */
                    if (ST(L08_PATROL_PICK) == 1) {
                        CB->joindre(-P_GUARDIAN3, 797, -777, 0xFED9, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 3);
                    }
                    /* 0x9868 */
                    if (ST(L08_PATROL_PICK) == 2) {
                        CB->joindre(-P_GUARDIAN3, -127, -1793, 0x4F3E, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 4);
                    }
                    CB->evenement(1, 1000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x98DA */
                if (ST(L08_G3_PATROL_STEP) == 3) {
                    ST(L08_G3_PATROL_STEP) = 0;
                    CB->select_pile(1, 1);
                    CB->animation("SP43ATT08");
                    CB->animation("P43ARR03");
                    ST(L08_PATROL_PICK) = CB->random(3);
                    /* 0x9932 */
                    if (ST(L08_PATROL_PICK) == 0) {
                        CB->joindre(-P_GUARDIAN3, 381, -1562, 0x9088, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 5);
                    }
                    /* 0x997F */
                    if (ST(L08_PATROL_PICK) == 1) {
                        CB->joindre(-P_GUARDIAN3, 506, -1572, 0xA882, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 2);
                    }
                    /* 0x99CC */
                    if (ST(L08_PATROL_PICK) == 2) {
                        CB->joindre(-P_GUARDIAN3, -127, -1793, 0x4F3E, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 4);
                    }
                    CB->evenement(1, 1000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9A3E */
                if (ST(L08_G3_PATROL_STEP) == 4) {
                    ST(L08_G3_PATROL_STEP) = 0;
                    CB->select_pile(1, 1);
                    CB->animation("SP43ATT08");
                    CB->animation("P43ARR03");
                    ST(L08_PATROL_PICK) = CB->random(3);
                    /* 0x9A96 */
                    if (ST(L08_PATROL_PICK) == 0) {
                        CB->joindre(-P_GUARDIAN3, 381, -1562, 0x9088, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 5);
                    }
                    /* 0x9AE3 */
                    if (ST(L08_PATROL_PICK) == 1) {
                        CB->joindre(-P_GUARDIAN3, 506, -1572, 0xA882, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 2);
                    }
                    /* 0x9B30 */
                    if (ST(L08_PATROL_PICK) == 2) {
                        CB->joindre(-P_GUARDIAN3, 797, -777, 0xFED9, -1);
                        CB->animation("SP43ARR03_50");
                        CB->modify_var(&ST(L08_G3_PATROL_STEP), 3);
                    }
                    CB->evenement(1, 1000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9BA2 */
                if (ST(L08_G3_PATROL_STEP) == 5) {
                    ST(L08_G3_PATROL_STEP) = 1;
                    CB->select_pile(1, 1);
                    CB->animation("SP43ATT10");
                    CB->animation("P43ATT03");
                    CB->animation("P43ATT01");
                    CB->evenement(1, 5000);
                    CB->select_pile(0, 1);
                    return;
                }
            }
        }
        /* 0x9C16 */
        if (ST(V_GUARDIAN4_STAGE) == 4) {
            /* 0x9C28 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x9C43 */
                if (CB->distance(1, 3) > 250 && CB->divers(DV_IDLE_TIME, 0, 0) < 2500 && ST(V_ENGAGED_PERSONA) == 0 && (ST(L08_G3_FIGHT_STAGE) == 0 || ST(L08_G3_FIGHT_STAGE) == 1) && (ST(L08_G4_PATROL_STATE) == 0 || ST(L08_G4_PATROL_STATE) == 1)) {
                    CB->select_pile(2, 2);
                    /* 0x9CD6 */
                    if (ST(ST_VIE(P_GUARDIAN4)) < 2 && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN4, 0)) {
                        CB->dialogue("L08D242", P_GUARDIAN4);
                        CB->combat(P_GUARDIAN4, 0);
                    }
                    CB->select_pile(1, 2);
                    /* 0x9D2F */
                    if (ST(ST_VIE(P_GUARDIAN3)) < 2) {
                        CB->dialogue("L08D243", P_GUARDIAN3);
                        CB->combat(P_GUARDIAN3, 0);
                    }
                    CB->select_pile(0, 1);
                    /* 0x9D6B */
                    if (ST(ST_VIE(P_GUARDIAN3)) < 2 && ST(ST_VIE(P_GUARDIAN4)) < 2) {
                        ST(V_ENGAGED_PERSONA) = 0x10F8;
                    }
                    /* 0x9D9B */
                    if (ST(ST_VIE(P_GUARDIAN3)) < 2 && ST(ST_VIE(P_GUARDIAN4)) == VIE_DEAD) {
                        ST(V_ENGAGED_PERSONA) = P_GUARDIAN3;
                    }
                    /* 0x9DCB */
                    if (ST(ST_VIE(P_GUARDIAN3)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN4)) < 2) {
                        ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
                    }
                }
                /* 0x9DFB */
                if (ST(L08_G3_FIGHT_STAGE) == 0xA) {
                    CB->select_pile(1, 1);
                    ST(L08_G3_FIGHT_STAGE) = 2;
                    CB->animation("SP43GAR27");
                    CB->animation("P43ARR03");
                    CB->joindre(-P_GUARDIAN3, 1007, -1293, 0x8000, -1);
                    CB->select_pile(0, 1);
                }
                /* 0x9E6B */
                if (ST(L08_G3_PATROL_ARMED) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 2) > 220) {
                    ST(L08_G3_PATROL_ARMED) = 0;
                    CB->select_pile(1, 1);
                    CB->stop_joindre(-P_GUARDIAN3);
                    CB->joindre(-P_GUARDIAN3, 1007, -1293, 0x8000, -1);
                    CB->select_pile(0, 1);
                }
            }
            /* 0x9EFA */
            if (ST(ST_HERO) == P_ARKHAN3 && CB->distance(1, 3) < 300 && ST(V_ENGAGED_PERSONA) == 0) {
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_GUARDIAN4);
                CB->animation("SP44GAR02");
                CB->combat(P_GUARDIAN4, 0);
                CB->dialogue("L08D222", P_GUARDIAN4);
                ST(V_ENGAGED_PERSONA) = 0x10F8;
                /* 0x9F80 */
                if (ST(ST_VIE(P_GUARDIAN3)) != VIE_DEAD) {
                    ST(V_ENGAGED_PERSONA) = 0x10F8;
                    CB->combat(P_GUARDIAN3, 0);
                } else {
                    ST(V_ENGAGED_PERSONA) = P_GUARDIAN4;
                }
                CB->bloque(BLOQUE_NONE);
            }
        }
    }
    /* 0x9FC4 */
    if (ST(V_EVENT_6) == 0 && ST(V_EVENT_10) == 0 && (ST(V_KALHI_PHASE) == 0 || ST(V_KALHI_PHASE) > 6) && (ST(V_DELIA_PHASE) == 0 || ST(V_DELIA_PHASE) > 7) && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0xA042 */
        if (ST(V_GUARDIAN4_STAGE) == 0) {
            /* 0xA051 */
            if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD) {
                /* 0xA075 */
                if (ST(L08_G4_WAYPOINT) == 0 && CB->distance(3, 36) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(P_GUARDIAN4, 833, -1360, -1, -1);
                    ST(L08_G4_WAYPOINT) = 1;
                    CB->select_pile(0, 1);
                }
                /* 0xA0F7 */
                if (CB->distance(3, 35) <= 50) {
                    /* 0xA109 */
                    if (ST(L08_G4_WAYPOINT) == 3) {
                        CB->select_pile(2, 1);
                        CB->stop_joindre(-P_GUARDIAN4);
                        CB->joindre(P_GUARDIAN4, 1896, -1376, -1, -1);
                        ST(L08_G4_WAYPOINT) = 0;
                        CB->select_pile(0, 1);
                    }
                    /* 0xA16B */
                    if (ST(L08_G4_WAYPOINT) == 1) {
                        CB->select_pile(2, 1);
                        CB->stop_joindre(-P_GUARDIAN4);
                        CB->joindre(P_GUARDIAN4, 815, -1890, -1, -1);
                        ST(L08_G4_WAYPOINT) = 2;
                        CB->select_pile(0, 1);
                    }
                }
                /* 0xA1CD */
                if (ST(L08_G4_WAYPOINT) == 2 && CB->distance(3, 37) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(P_GUARDIAN4, 833, -1360, -1, -1);
                    ST(L08_G4_WAYPOINT) = 3;
                    CB->select_pile(0, 1);
                }
            }
            /* 0xA248 */
            if (ST(L08_G4_PATROL_STATE) == 1 && CB->distance(1, 3) >= 220 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD) {
                ST(L08_G4_PATROL_STATE) = 0;
                CB->select_pile(2, 2);
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_GUARDIAN4);
                /* 0xA2D0 */
                if (ST(L08_G4_STANCE) == 1) {
                    ST(L08_G4_STANCE) = 0;
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->animation("SP44GAR27");
                    CB->animation("P44ARR02");
                }
                /* 0xA312 */
                if (ST(L08_G4_WAYPOINT) == 0) {
                    CB->joindre(P_GUARDIAN4, 1896, -1376, 0x4010, -1);
                }
                /* 0xA340 */
                if (ST(L08_G4_WAYPOINT) == 2) {
                    CB->joindre(P_GUARDIAN4, 815, -1890, -1, -1);
                }
                /* 0xA36B */
                if (ST(L08_G4_WAYPOINT) == 1 || ST(L08_G4_WAYPOINT) == 3) {
                    CB->joindre(P_GUARDIAN4, 833, -1360, -1, -1);
                }
                CB->select_pile(0, 1);
            }
        }
        /* 0xA3B5 */
        if (ST(V_GUARDIAN4_STAGE) == 2) {
            /* 0xA3C7 */
            if (ST(L08_G4_PATROL_STATE) == 1 && CB->distance(1, 3) >= 220 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD) {
                ST(L08_G4_PATROL_STATE) = 0;
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_GUARDIAN4);
                /* 0xA43F */
                if (ST(L08_G4_STANCE) == 1) {
                    ST(L08_G4_STANCE) = 0;
                    CB->animation("SP44GAR27");
                    CB->animation("P44ARR02");
                }
                /* 0xA476 */
                if (ST(L08_G4_WAYPOINT) == 9) {
                    CB->joindre(-P_GUARDIAN4, 815, -1890, -1, -1);
                }
                /* 0xA4A1 */
                if (ST(L08_G4_WAYPOINT) == 1) {
                    CB->joindre(-P_GUARDIAN4, 833, -1360, -1, -1);
                }
                /* 0xA4CC */
                if (ST(L08_G4_WAYPOINT) == 2) {
                    CB->joindre(-P_GUARDIAN4, 1175, -1360, -1, -1);
                }
                /* 0xA4F7 */
                if (ST(L08_G4_WAYPOINT) == 3) {
                    CB->joindre(-P_GUARDIAN4, 1434, -900, -1, -1);
                }
                /* 0xA522 */
                if (ST(L08_G4_WAYPOINT) == 4) {
                    CB->joindre(-P_GUARDIAN4, 2075, -900, -1, -1);
                }
                /* 0xA54D */
                if (ST(L08_G4_WAYPOINT) == 5) {
                    CB->joindre(-P_GUARDIAN4, 1958, -1828, -1, -1);
                }
                /* 0xA578 */
                if (ST(L08_G4_WAYPOINT) == 6) {
                    CB->joindre(-P_GUARDIAN4, 1274, -1915, -1, -1);
                }
                /* 0xA5A3 */
                if (ST(L08_G4_WAYPOINT) == 7) {
                    CB->joindre(-P_GUARDIAN4, 1175, -1360, -1, -1);
                }
                /* 0xA5CE */
                if (ST(L08_G4_WAYPOINT) == 8) {
                    CB->joindre(-P_GUARDIAN4, 833, -1360, -1, -1);
                }
                CB->select_pile(0, 1);
            }
            /* 0xA603 */
            if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD) {
                /* 0xA627 */
                if (ST(L08_G4_WAYPOINT) == 9 && CB->distance(3, 35) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 833, -1360, -1, -1);
                    ST(L08_G4_WAYPOINT) = 1;
                    CB->select_pile(0, 1);
                }
                /* 0xA6A2 */
                if (ST(L08_G4_WAYPOINT) == 1 && CB->distance(3, 36) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 1175, -1360, -1, -1);
                    ST(L08_G4_WAYPOINT) = 2;
                    CB->select_pile(0, 1);
                }
                /* 0xA71D */
                if (ST(L08_G4_WAYPOINT) == 2 && CB->distance(3, 37) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 1434, -900, -1, -1);
                    ST(L08_G4_WAYPOINT) = 3;
                    CB->select_pile(0, 1);
                }
                /* 0xA798 */
                if (ST(L08_G4_WAYPOINT) == 3 && CB->distance(3, 38) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 2075, -900, -1, -1);
                    ST(L08_G4_WAYPOINT) = 4;
                    CB->select_pile(0, 1);
                }
                /* 0xA813 */
                if (ST(L08_G4_WAYPOINT) == 4 && CB->distance(3, 39) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 1958, -1828, -1, -1);
                    ST(L08_G4_WAYPOINT) = 5;
                    CB->select_pile(0, 1);
                }
                /* 0xA88E */
                if (ST(L08_G4_WAYPOINT) == 5 && CB->distance(3, 40) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 1274, -1915, -1, -1);
                    ST(L08_G4_WAYPOINT) = 6;
                    CB->select_pile(0, 1);
                }
                /* 0xA909 */
                if (ST(L08_G4_WAYPOINT) == 6 && CB->distance(3, 22) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 1175, -1360, -1, -1);
                    ST(L08_G4_WAYPOINT) = 7;
                    CB->select_pile(0, 1);
                }
                /* 0xA984 */
                if (ST(L08_G4_WAYPOINT) == 7 && CB->distance(3, 37) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 833, -1360, -1, -1);
                    ST(L08_G4_WAYPOINT) = 8;
                    CB->select_pile(0, 1);
                }
                /* 0xA9FF */
                if (ST(L08_G4_WAYPOINT) == 8 && CB->distance(3, 36) <= 50) {
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_GUARDIAN4);
                    CB->joindre(-P_GUARDIAN4, 815, -1890, -1, -1);
                    ST(L08_G4_WAYPOINT) = 9;
                    CB->select_pile(0, 1);
                }
            }
        }
        /* 0xAA7A */
        if (ST(V_GUARDIAN4_STAGE) == 4) {
            /* 0xAA8C */
            if (ST(L08_G4_PATROL_STATE) == 0xA && CB->distance(1, 3) < 800) {
                CB->select_pile(0, 2);
                CB->bloque(BLOQUE_ALL);
                CB->select_pile(1, 2);
                CB->stop_joindre(-P_GUARDIAN3);
                CB->joindre(P_GUARDIAN3, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP43GAR26");
                CB->dialogue("L08D241", P_GUARDIAN3);
                CB->animation("P43GAR03");
                CB->select_pile(2, 2);
                CB->divers(DV_RUN, P_GUARDIAN4, 1);
                ST(L08_G4_PATROL_STATE) = 0xF;
                CB->joindre_perm(-P_GUARDIAN4, ST(ST_HERO));
                CB->stop_joindre(P_GUARDIAN4);
                CB->dialogue("L08D324", P_GUARDIAN4);
                CB->joindre(-P_GUARDIAN4, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xABB1 */
            if (ST(L08_G4_PATROL_STATE) == 0xF && CB->distance(1, 3) < 300) {
                ST(L08_G4_PATROL_STATE) = 0x14;
                CB->divers(DV_RUN, P_GUARDIAN4, 0);
            }
            /* 0xABFF */
            if (ST(L08_G4_PATROL_STATE) == 1 && CB->distance(1, 3) >= 220 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN4)) != VIE_DEAD) {
                ST(L08_G4_PATROL_STATE) = 0;
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_GUARDIAN4);
                CB->joindre(P_GUARDIAN4, 1536, -1409, 0, -1);
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0xAC9D */
    if (ST(V_KALHI_PHASE) == 5 && ST(L08_COMPANION_ENTRY_DONE) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0xACD3 */
        if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            CB->select_pile(1, 1);
            /* 0xAD04 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                /* 0xAD16 */
                if (ST(L08_CAM_ANIM_ARMED) == 0) {
                    /* 0xAD41 */
                    if (CB->distance(31, 33) + 50 < CB->distance(1, 33)) {
                        ST(L08_CAM_ANIM_ARMED) = 1;
                        ST(L08_KALHI13_GAR_STEP) = 1;
                        CB->combat(P_GUARDIAN10, -1);
                        CB->stop_joindre(-P_KALHI13);
                        CB->divers_empile(DV_CINEMATIC, P_KALHI13, 4);
                    }
                }
                /* 0xAD9E */
                if (ST(L08_CAM_ANIM_ARMED) == 1) {
                    i32 t1 = CB->distance(1, 33);
                    /* 0xADC8 */
                    if (t1 < CB->distance(31, 33)) {
                        ST(L08_CAM_ANIM_ARMED) = 0;
                        CB->divers(DV_CINEMATIC, 0, 0);
                        CB->animation("SP13GAR02");
                        CB->combat(P_GUARDIAN10, 0);
                    }
                }
            }
            /* 0xAE10 */
            if (ST(ST_VIE(P_KALHI33)) == 1) {
                /* 0xAE22 */
                if (ST(L08_CAM_ANIM_ARMED) == 0) {
                    /* 0xAE50 */
                    if (CB->distance(32, 33) + 300 < CB->distance(1, 33)) {
                        ST(L08_CAM_ANIM_ARMED) = 1;
                        ST(L08_KALHI13_GAR_STEP) = 1;
                        CB->combat(P_GUARDIAN10, -1);
                        CB->stop_joindre(-P_KALHI33);
                        CB->divers_empile(DV_CINEMATIC, P_KALHI33, 4);
                    }
                }
                /* 0xAEAD */
                if (ST(L08_CAM_ANIM_ARMED) == 1) {
                    i32 t2 = CB->distance(1, 33);
                    /* 0xAED7 */
                    if (t2 < CB->distance(32, 33)) {
                        ST(L08_CAM_ANIM_ARMED) = 0;
                        CB->divers(DV_CINEMATIC, 0, 0);
                        CB->animation("SP33GAR02");
                        CB->combat(P_GUARDIAN10, 0);
                    }
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0xAF2C */
        if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && (ST(ST_VIE(P_KALHI13)) == VIE_DEAD || ST(ST_VIE(P_KALHI33)) == VIE_DEAD) && ST(L08_KALHI13_GAR_STEP) == 1) {
            ST(L08_KALHI13_GAR_STEP) = 0xA;
            ST(V_KALHI_KILLED) = 1;
            CB->divers(DV_CINEMATIC, 0, 0);
            CB->combat(P_GUARDIAN10, 0);
        }
        /* 0xAFBD */
        if (ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0xAFDE */
            if (ST(L08_KALHI33_RUN_STEP) == 0 && ST(ST_LOCKED_CAMERA) == 1 && ST(ST_VIE(P_KALHI33)) == 1) {
                ST(L08_KALHI33_RUN_STEP) = 1;
                CB->select_pile(2, 1);
                CB->joindre(-P_GUARDIAN10, 0, 0, 0, P_KALHI33);
                CB->animation("SP50GAR26");
                CB->animation("P50GAR03_25");
                CB->joindre_perm(P_GUARDIAN10, P_KALHI33);
                CB->select_pile(0, 1);
                return;
            }
            /* 0xB07E */
            if (ST(L08_KALHI33_CHAT_READY) == 1 && ST(L08_KALHI33_RUN_STEP) == 1 && CB->distance(32, 33) > 350) {
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_GUARDIAN10);
                CB->joindre(P_GUARDIAN10, 1544, -1171, 0x8097, -1);
                ST(L08_KALHI33_RUN_STEP) = 2;
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0xB115 */
        if (ST(ST_VIE(P_KALHI33)) == 1 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0xB148 */
            if (ST(L08_KALHI33_STUCK_CHECK) == 0 && CB->distance(32, 33) < 1000) {
                /* 0xB175 */
                if (ST(ST_LOCKED_CAMERA) == 0 && ST(L08_KALHI33_CHAT_READY) == 0) {
                    i32 t3 = CB->distance(1, 33);
                    /* 0xB1B1 */
                    if (t3 > CB->distance(32, 33)) {
                        ST(L08_KALHI33_CHAT_READY) = 0xA;
                        CB->select_pile(1, 2);
                        CB->stop_joindre(-P_KALHI33);
                        CB->stop_joindre(-P_GUARDIAN10);
                        CB->bloque(BLOQUE_ALL);
                        lieu_action_register("L08D229", P_GUARDIAN10, "SP50GAR27", "SP50ARR03");
                        CB->joindre_perm(P_GUARDIAN10, P_KALHI33);
                        CB->select_pile(1, 1);
                        CB->joindre(-P_KALHI33, 0, 0, 0, P_GUARDIAN10);
                        lieu_action_register("L08D312A", P_KALHI33, "SP33DIA50", "SP33ARR01");
                        lieu_action_register("L08D312B", P_KALHI33, "SP33DIA56", "SP33ARR01_50");
                        CB->modify_var(&ST(L08_KALHI33_CHAT_READY), 1);
                        CB->joindre(-P_KALHI33, 2690, -1625, 0xC350, -1);
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                }
                /* 0xB2CE */
                if (ST(ST_LOCKED_CAMERA) == 0 && CB->distance(32, 1) > 200) {
                    i32 t4 = CB->distance(1, 33);
                    /* 0xB313 */
                    if (t4 < CB->distance(32, 33)) {
                        ST(L08_KALHI33_STUCK_CHECK) = 1;
                        ST(L08_G10_STATE) = 0x14;
                        CB->select_pile(1, 2);
                        CB->bloque(BLOQUE_ALL);
                        ST(L08_KALHI33_STEP) = 0x63;
                        CB->stop_joindre(-P_GUARDIAN10);
                        CB->stop_joindre(-P_KALHI33);
                        CB->joindre(P_GUARDIAN10, 0, 0, 0, ST(ST_HERO));
                        CB->joindre(P_KALHI33, 0, 0, 0, ST(ST_HERO));
                        lieu_action_register("L08D210", P_GUARDIAN10, "SP50GAR26", "SP50GAR03");
                        ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
                        CB->bloque(BLOQUE_NONE);
                        CB->combat(P_GUARDIAN10, 0);
                        CB->select_pile(0, 1);
                        return;
                    }
                }
            }
            /* 0xB406 */
            if (ST(L08_KALHI_FIGHT_TALK) == 0 && CB->divers(DV_CAN_FIGHT, 0, 2) && CB->distance(1, 33) < 500) {
                ST(L08_KALHI_FIGHT_TALK) = 1;
                CB->select_pile(2, 2);
                /* 0xB466 */
                if (ST(V_KALHI_PHASE) == 5) {
                    /* 0xB478 */
                    if (ST(ST_VIE(P_KALHI33)) == 1) {
                        CB->stop_joindre(-P_KALHI33);
                    }
                    /* 0xB495 */
                    if (ST(ST_VIE(P_KALHI13)) == 1) {
                        CB->stop_joindre(-P_KALHI13);
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
        }
        /* 0xB54B */
        if (ST(ST_VIE(P_KALHI13)) == 1 && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(L08_CREATURE34_ENGAGED) == 0 && CB->distance(1, 33) < 400) {
            ST(L08_CREATURE34_ENGAGED) = 1;
            CB->select_pile(2, 1);
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_HP, P_GUARDIAN10, 0x200);
            CB->stop_joindre(-P_KALHI13);
            CB->joindre(-P_GUARDIAN10, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L08D311", P_GUARDIAN10, "SP50GAR26", "SP50GAR03_50");
            CB->modify_var(&ST(V_ENGAGED_PERSONA), P_GUARDIAN10);
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
            CB->combat(P_GUARDIAN10, 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xB672 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(L08_CREATURE34_ENGAGED) == 1 && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            CB->select_pile(1, 1);
            ST(L08_KALHI13_D220_PENDING) = 1;
            /* 0xB700 */
            if (ST(L08_CAM_ANIM_ARMED) == 0xA) {
                ST(L08_KALHI13_D220_PENDING) = 0;
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                CB->dialogue("L08D230", P_KALHI13);
            }
            CB->animation("SP13ARR01_50");
            CB->modify_var(&ST(L08_FOLLOWER_MODE), 1);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xB77C */
        if (ST(V_EVENT_35) == 1) {
            ST(V_EVENT_35) = 0;
            CB->select_pile(2, 1);
            CB->animation("SP50IMP53");
            CB->modify_var(&ST(ST_VIE(P_GUARDIAN10)), VIE_DEAD);
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xB802 */
        if (CB->divers(DV_WAS_HIT, P_GUARDIAN10, 0) && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, P_GUARDIAN10, 1);
            CB->combat(P_GUARDIAN10, -1);
            CB->divers(DV_CINEMATIC, 0, 0);
            /* 0xB87B */
            if ((CB->divers(DV_HP, P_GUARDIAN10, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_GUARDIAN10, (CB->divers(DV_HP, P_GUARDIAN10, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_GUARDIAN10, 1);
            }
            /* 0xB8DC */
            if (CB->divers(DV_HP, P_GUARDIAN10, -1) <= 1) {
                ST(ST_VIE(P_GUARDIAN10)) = VIE_DEAD;
                CB->stop_joindre(-P_GUARDIAN10);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN10, 0);
                CB->animation("SP50IMP53");
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_GUARDIAN10);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN10, 0);
                CB->animation("SP50IMP13");
                CB->animation("P50IMP48");
                CB->animation("P50GAR03");
                CB->combat(P_GUARDIAN10, 0);
                ST(V_ENGAGED_PERSONA) = P_GUARDIAN10;
            }
            CB->select_pile(0, 1);
        }
        /* 0xB99D */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_KALHI33, 0) && ST(ST_VIE(P_KALHI33)) != VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre_perm(-P_KALHI33, ST(ST_HERO));
            CB->stop_joindre(P_KALHI33);
            lieu_action_register("L08D220", P_KALHI33, "SP33DIA54", "SP33ARR01_50");
            CB->modify_var(&ST(L08_KALHI33_STEP), 6);
            CB->joindre(P_KALHI33, 2690, -1625, 0xC350, -1);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0xBA9B */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->select_pile(0, 1);
        /* 0xBAC9 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            CB->animation("SP01ACT07");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("ar");
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xBB0F */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0xBB21 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                /* 0xBB3B */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT07");
                } else {
                    CB->animation("SP02ACT35");
                }
            }
            /* 0xBB68 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0xBB82 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP02ACT34");
                } else {
                    CB->animation("SP02ACT36");
                }
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("ar");
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xBBD8 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0xBBEA */
            if (ST(ST_SPECIAL_MODE) == 1) {
                /* 0xBC04 */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT07");
                } else {
                    CB->animation("SP03ACT35");
                }
            }
            /* 0xBC31 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                /* 0xBC4B */
                if (!CB->divers(DV_CAN_FIGHT, 9, 2)) {
                    CB->animation("SP03ACT34");
                } else {
                    CB->animation("SP03ACT36");
                }
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("ar");
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1002AA59).
 * Callback ingestion (0x1002A6DA..0x1002AA4A), the trigger-table fill and
 * the unused dword_1003FCDC = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu08_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1002AC84);
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
        l08_trig00();   /* 0x1002AC89: call sub_10001073 -> sub_10001200 */
        return 0;
    }
    /* 0x1002AB6E - the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1002ACDC): drain pending sound, run body.
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1002A6C0).
 * Jump table @ 0x1002AD59, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu08_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu08: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l08_trigger(arg);             break;  /* dword_1003FB70[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;  /* sub_1002450A == shared event4 */
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
