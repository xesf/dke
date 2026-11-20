/*
 * lieu14.c — Native hand-port of LIEU14.DLL (level-14 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu14_trig.c (reached through
 * l14_trigger, see lieu14_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..09; the
 * entry is at 0x100186E0, the state pointer global is dword_1002BEEC and the
 * trigger table is dword_1002BE00[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_10018DAC), lieu_pre_scene (sub_10018F5C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT follow LIEU03 instruction for instruction. The tick body
 * (sub_10009168) is level-14 specific.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu14_int.h"

static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * Per-frame body — sub_10009168 (thunk 0x100010B9), level-14 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    i32 t1 = CB->divers(DV_IN_ZONE, P_SORDOS, 15);
    snprintf(g_txt, sizeof(g_txt), "test:%d", t1);
    CB->print_msg(g_txt, 0, 0xF0);
    /* 0x91B1 */
    if (ST(V_DELIA_GONE) == 1 && ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_LOCKED_CAMERA) == 6 && ST(V_ENGAGED_PERSONA) == 35 && ST(ST_VIE(35)) != VIE_DEAD && ST(L14_PERSONA35_COMBAT) == 0) {
        CB->select_pile(1, 2);
        ST(L14_PERSONA35_COMBAT) = 1;
        CB->combat(35, -1);
        CB->stop_joindre(-35);
        CB->joindre(-35, 4430, 1352, 0x475B, -1);
        CB->select_pile(0, 1);
    }
    /* 0x9277 */
    if (ST(V_DELIA_GONE) == 1 && ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_LOCKED_CAMERA) != 6 && ST(V_ENGAGED_PERSONA) == 35 && ST(ST_VIE(35)) != VIE_DEAD && ST(L14_PERSONA35_COMBAT) == 1) {
        CB->stop_joindre(-35);
        CB->combat(35, 0);
        ST(L14_PERSONA35_COMBAT) = 0;
    }
    /* 0x9301 */
    if (ST(V_EVENT_0) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(2, 1);
        CB->stop_joindre(P_MALROT1);
        ST(V_ENGAGED_PERSONA) = P_MALROT1;
        ST(L14_FIGHT_STARTED) = 1;
        CB->combat(P_MALROT1, 0);
        CB->evenement(2, -1);
        ST(V_EVENT_2) = 0;
        ST(V_EVENT_0) = 0;
        CB->select_pile(0, 1);
        /* 0x93A0 */
        if (ST(ST_VIE(P_MALROT4)) == 1 && ST(L14_DELIA_KILLED_14) != 1 && ST(V_DELIA_PHASE) != 3 && ST(V_DELIA_PHASE) != 4) {
            CB->select_pile(1, 1);
            CB->evenement(1, -1);
            CB->animation("Sar_10");
            ST(V_EVENT_1) = 0;
            CB->stop_joindre(-P_MALROT4);
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
            CB->animation("P64ARR01_10");
            CB->modify_var(&ST(L14_MALROT4_ENGAGED), 1);
            ST(V_ENGAGED_PERSONA) = 0x1814;
            CB->combat(P_MALROT4, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x9487 */
    if (ST(V_EVENT_1) == 1 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && ST(L14_MALROT4_REACT_DONE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(V_EVENT_1) = 0;
        /* 0x94DB */
        if (ST(L14_MALROT4_REACT_STEP) == 1) {
            CB->select_pile(1, 1);
            ST(L14_MALROT4_REACT_STEP) = 0;
            CB->animation("SP64ARR01_10");
            ST(L14_TALK_RANDOM) = CB->random(3);
            /* 0x9528 */
            if (ST(L14_TALK_RANDOM) == 0) {
                CB->joindre(-P_MALROT4, 6245, 1272, 0xBC4D, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 2);
            }
            /* 0x956A */
            if (ST(L14_TALK_RANDOM) == 1) {
                CB->joindre(-P_MALROT4, 5737, 1624, 0x62CC, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 3);
            }
            /* 0x95AC */
            if (ST(L14_TALK_RANDOM) == 2) {
                CB->joindre(-P_MALROT4, 4332, 1306, 0x4260, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 4);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9613 */
        if (ST(L14_MALROT4_REACT_STEP) == 2) {
            CB->select_pile(1, 1);
            ST(L14_MALROT4_REACT_STEP) = 0;
            CB->animation("SP64ARR01_10");
            ST(L14_TALK_RANDOM) = CB->random(2);
            /* 0x9660 */
            if (ST(L14_TALK_RANDOM) == 0) {
                CB->joindre(-P_MALROT4, 5737, 1624, 0x62CC, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 3);
            }
            /* 0x96A2 */
            if (ST(L14_TALK_RANDOM) == 1) {
                CB->joindre(-P_MALROT4, 4332, 1306, 0x4260, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 4);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9709 */
        if (ST(L14_MALROT4_REACT_STEP) == 3) {
            CB->select_pile(1, 1);
            ST(L14_MALROT4_REACT_STEP) = 0;
            CB->animation("SP64ARR01_10");
            ST(L14_TALK_RANDOM) = CB->random(2);
            /* 0x9756 */
            if (ST(L14_TALK_RANDOM) == 0) {
                CB->joindre(-P_MALROT4, 6245, 1272, 0xBC4D, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 2);
            }
            /* 0x9798 */
            if (ST(L14_TALK_RANDOM) == 1) {
                CB->joindre(-P_MALROT4, 4332, 1306, 0x4260, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 4);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x97FF */
        if (ST(L14_MALROT4_REACT_STEP) == 4) {
            CB->select_pile(1, 1);
            ST(L14_MALROT4_REACT_STEP) = 0;
            CB->animation("SP64ARR01_10");
            ST(L14_TALK_RANDOM) = CB->random(2);
            /* 0x984C */
            if (ST(L14_TALK_RANDOM) == 0) {
                CB->joindre(-P_MALROT4, 6245, 1272, 0xBC4D, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 2);
            }
            /* 0x988E */
            if (ST(L14_TALK_RANDOM) == 1) {
                CB->joindre(-P_MALROT4, 5737, 1624, 0x62CC, -1);
                CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 3);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x98F5 */
    if (ST(L14_MALROT4_REACT_DONE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && CB->distance(1, 2) > 250) {
        CB->select_pile(1, 1);
        ST(L14_STANCE_WARNED_MALROT4) = 0;
        ST(L14_MALROT4_REACT_DONE) = 0;
        ST(L14_MALROT4_REACT_STEP) = 10;
        ST(L14_MALROT4_ENGAGED) = 0;
        CB->stop_joindre(-P_MALROT4);
        CB->joindre(P_MALROT4, 4787, 1423, 0xB798, -1);
        CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 1);
        CB->evenement(1, 7500);
        CB->select_pile(0, 1);
    }
    /* 0x99EA */
    if (ST(V_EVENT_2) == 1 && ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && ST(V_DELIA_PHASE) == 0 && ST(V_DELIA_GONE) == 0) {
        CB->select_pile(2, 1);
        ST(V_EVENT_2) = 0;
        /* 0x9A7D */
        if (CB->divers(DV_DESC_POS, P_MALROT1, 2) > 0xD6D8 || CB->divers(DV_DESC_POS, P_MALROT1, 2) < 0x2710) {
            CB->stop_joindre(-P_MALROT1);
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT1, 0);
            CB->animation("SP61ATT05");
            CB->animation("P61ATT04");
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT1, 1);
        }
        CB->evenement(2, 5000);
        CB->select_pile(0, 1);
    }
    /* 0x9B0E */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        /* 0x9B2F */
        if (ST(L14_MALROT4_CRI_STATE) == 64) {
            ST(L14_MALROT4_CRI_STATE) = 99;
            CB->dialogue("L14D235", P_MALROT4);
            ST(V_ENGAGED_PERSONA) = P_MALROT4;
            CB->combat(P_MALROT4, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            /* 0x9BA0 */
            if (ST(ST_VIE(P_MALROT1)) == 1) {
                CB->select_pile(2, 2);
                CB->evenement(2, -1);
                ST(V_EVENT_1) = 0;
                ST(V_ENGAGED_PERSONA) = 0x1814;
                CB->combat(P_MALROT1, 0);
                CB->select_pile(0, 1);
            }
            return;
        }
    }
    /* 0x9C1A */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && CB->direction_perso(P_PROVIDER)) {
        ST(V_CRI_USED) = 1;
        /* 0x9CA2 */
        if (ST(L14_PROVIDER_CRI_STATE) != 99) {
            ST(L14_PROVIDER_CRI_STATE) = 16;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0x22, 1000);
        return;
    }
    /* 0x9CE3 */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0x9D04 */
        if (ST(L14_PROVIDER_CRI_STATE) == 16) {
            ST(L14_PROVIDER_CRI_STATE) = 99;
            lieu_action_register("L14D236", P_PROVIDER, "SP16GAR03", "SP16GAR03");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            ST(L14_PROVIDER_ANGRY) = 1;
            ST(L14_FIGHT_STARTED) = 1;
            CB->combat(P_PROVIDER, 1);
            ST(V_ENGAGED_PERSONA) = P_PROVIDER;
            /* 0x9D97 */
            if (ST(ST_VIE(P_MALROT4)) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
                CB->select_pile(1, 1);
                CB->evenement(1, -1);
                CB->animation("Sar_10");
                ST(V_EVENT_1) = 0;
                CB->stop_joindre(-P_MALROT4);
                CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
                CB->animation("P64ARR01_10");
                CB->modify_var(&ST(L14_MALROT4_ENGAGED), 1);
                ST(V_ENGAGED_PERSONA) = 0x1814;
                CB->combat(P_MALROT4, 0);
                CB->select_pile(0, 1);
            }
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x9E67 */
    if (ST(L14_PROVIDER_CRI_STATE) == 99) {
        ST(L14_PROVIDER_CRI_STATE) = 16;
    }
    /* 0x9E88 */
    if (ST(V_EVENT_35) == 1) {
        ST(V_EVENT_35) = 0;
        /* 0x9EA9 */
        if (ST(L14_MALROT1_CRI_STATE) == 61) {
            ST(L14_MALROT1_CRI_STATE) = 99;
            lieu_action_register("L14D237", P_MALROT1, "SP61GAR02", "SP61GAR02");
            ST(V_ENGAGED_PERSONA) = P_MALROT1;
            CB->combat(P_MALROT1, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            /* 0x9F23 */
            if (ST(ST_VIE(P_MALROT4)) == 1 && ST(L14_DELIA_KILLED_14) != 1 && ST(V_DELIA_PHASE) != 3 && ST(V_DELIA_PHASE) != 4) {
                CB->select_pile(1, 1);
                CB->evenement(1, -1);
                CB->animation("Sar_10");
                ST(V_EVENT_1) = 0;
                CB->stop_joindre(-P_MALROT4);
                CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
                CB->animation("P64ARR01_10");
                CB->modify_var(&ST(L14_MALROT4_ENGAGED), 1);
                ST(V_ENGAGED_PERSONA) = 0x193D;
                CB->combat(P_MALROT4, 0);
                CB->select_pile(0, 1);
            }
            return;
        }
    }
    /* 0xA01D */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 24) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && CB->direction_perso(P_MALROT1)) {
        ST(V_CRI_USED) = 1;
        ST(L14_FIGHT_STARTED) = 1;
        /* 0xA0B4 */
        if (ST(L14_MALROT1_CRI_STATE) != 99) {
            ST(L14_MALROT1_CRI_STATE) = 61;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0x23, 1000);
        return;
    }
    /* 0xA0F5 */
    if (ST(L14_MALROT1_CRI_STATE) == 99) {
        ST(L14_MALROT1_CRI_STATE) = 16;
    }
    /* 0xA124 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_DELIA)) == 1 && CB->direction_perso(P_DELIA)) {
        ST(V_CRI_USED) = 1;
        /* 0xA1AC */
        if (ST(V_WEAPON_WARNED) != 99) {
            ST(V_WEAPON_WARNED) = 10;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0x25, 1000);
        return;
    }
    /* 0xA1ED */
    if (ST(V_WEAPON_WARNED) == 99) {
        ST(V_WEAPON_WARNED) = 10;
    }
    /* 0xA20E */
    if (ST(V_EVENT_37) == 1) {
        ST(V_EVENT_37) = 0;
        /* 0xA22F */
        if (ST(V_WEAPON_WARNED) == 10) {
            ST(V_WEAPON_WARNED) = 99;
            lieu_action_register("L16D122", P_DELIA, "SP10DIA50", "SP10ARR01_10");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xA295 */
    if (ST(V_EVENT_36) == 1) {
        CB->select_pile(2, 2);
        ST(V_EVENT_36) = 0;
        /* 0xA2C6 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->play_anim(0, 1);
            CB->play_anim(1, 1);
        }
        /* 0xA2F5 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->play_anim(2, 1);
            CB->play_anim(3, 1);
        }
        ST(L14_AMBIENT_ANIM_ON) = 1;
        CB->select_pile(0, 1);
    }
    /* 0xA343 */
    if (ST(ST_VIE(P_MALROT4)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(V_DELIA_GONE) == 0) {
        /* 0xA373 */
        if (ST(V_ENGAGED_PERSONA) == P_MALROT4) {
            ST(L14_FIGHT_STARTED) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            return;
        }
        /* 0xA3A5 */
        if (ST(V_ENGAGED_PERSONA) == 0x1814) {
            ST(V_ENGAGED_PERSONA) = P_MALROT1;
            return;
        }
        /* 0xA3C8 */
        if (ST(V_ENGAGED_PERSONA) == 0x1910) {
            ST(V_ENGAGED_PERSONA) = P_PROVIDER;
            return;
        }
        /* 0xA3EB */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            ST(ST_VIE(97)) = 1;
        }
    }
    /* 0xA40C */
    if (ST(ST_VIE(P_PROVIDER)) == VIE_DEAD) {
        /* 0xA41B */
        if (ST(V_ENGAGED_PERSONA) == P_PROVIDER) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
        /* 0xA439 */
        if (ST(V_ENGAGED_PERSONA) == 0x1910) {
            ST(V_ENGAGED_PERSONA) = P_MALROT4;
        }
        CB->ajoute_zone(38, 3315, 945, 275, 280, 0);
        CB->ajoute_zone(39, 3318, 957, 50, 50, 0);
    }
    /* 0xA493 */
    if (ST(ST_VIE(P_MALROT1)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(V_DELIA_GONE) == 0) {
        /* 0xA4C3 */
        if (ST(V_ENGAGED_PERSONA) == P_MALROT1) {
            ST(L14_FIGHT_STARTED) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            return;
        } else {
            ST(V_ENGAGED_PERSONA) = P_MALROT4;
        }
        CB->ajoute_zone(9, 3096, 1157, 470, 135, 0);
    }
    /* 0xA527 */
    if (ST(ST_VIE(35)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 35) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0xA554 */
    if (ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_DELIA) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(V_DELIA_KILLED) = 14;
        ST(V_DELIA_PHASE) = 20;
    }
    /* 0xA59F */
    if (ST(L14_PROVIDER_ACTIVE) >= 1) {
        /* 0xA5AE */
        if (ST(L14_PROVIDER_FOLLOWING) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && CB->distance(1, 3) > 480 && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L14_PROVIDER_FOLLOWING) = 0;
            CB->joindre(P_PROVIDER, 3267, 915, 0xCE17, -1);
            ST(L14_PROVIDER_GREET_ARMED) = 0;
        }
        /* 0xA62D */
        if (ST(L14_PROVIDER_FOLLOWING) == 0 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && CB->distance(1, 3) < 200 && ST(V_ENGAGED_PERSONA) == 0) {
            CB->select_pile(2, 2);
            ST(L14_PROVIDER_FOLLOWING) = 1;
            CB->joindre(P_PROVIDER, 0, 0, 0, ST(ST_HERO));
            CB->select_pile(0, 1);
        }
    }
    /* 0xA6D0 */
    if (ST(L14_PROVIDER_GREET_ARMED) == 1 && ST(L14_PROVIDER_STAGE) >= 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 3) < 250 && ST(ST_HERO) == P_ARKHAN2) {
        ST(L14_PROVIDER_GREET_ARMED) = 0;
        CB->dialogue("L14D263", P_PROVIDER);
    }
    /* 0xA76D */
    if (ST(L14_PROVIDER_GREET_ARMED) == 0 && ST(L14_PROVIDER_STAGE) >= 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 3) > 400) {
        ST(L14_PROVIDER_GREET_ARMED) = 1;
    }
    /* 0xA7F4 */
    if (CB->distance(1, 3) < 250 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(L14_PROVIDER_ANGRY) == 1 && ST(V_GUARD_FIGHT_DONE) != 2 && ST(L14_PROVIDER_THREAT_DONE) == 0) {
        CB->combat(P_PROVIDER, 1);
        ST(L14_PROVIDER_THREAT_DONE) = 1;
        /* 0xA864 */
        if (ST(V_ENGAGED_PERSONA) == P_MALROT4) {
            ST(V_ENGAGED_PERSONA) = 0x1910;
        }
        /* 0xA87F */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_PROVIDER;
        }
    }
    /* 0xA89D */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_DELIA_GONE) == 0 && CB->distance(1, 24) < 200 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD) {
        /* 0xA8FD */
        if (ST(ST_HERO) == P_ARKHAN2 && ST(L14_MALROT1_MEET_DONE) == 0 && ST(L14_MALROT1_TALKED) == 0) {
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L14_MALROT1_MEET_DONE) = 1;
            ST(L14_MALROT1_TALKED) = 1;
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT1, 1);
            CB->joindre(ST(ST_HERO), 0, 0, 0, P_MALROT1);
            CB->joindre(P_MALROT1, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP61GAR26");
            CB->animation("P61GAR02");
            CB->dialogue("L14D171", P_MALROT1);
            /* 0xA9DA */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L14D139A", P_ARKHAN2, "SG00DIA02", NULL);
            }
            /* 0xAA02 */
            if (ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("L14D139B", P_ARKHAN2, "SG00DIA02", NULL);
            }
            CB->dialogue("L14D140", P_MALROT1);
            CB->animation("P61GAR27");
            CB->animation("P61ARR01");
            CB->joindre(P_MALROT1, 3649, 1268, 0xE98, -1);
            CB->animation("P61ATT04");
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0xAA8C */
        if (ST(ST_HERO) == P_ARKHAN3 && ST(L14_MALROT1_MEET_DONE) == 0) {
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L14_MALROT1_MEET_DONE) = 1;
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT1, 1);
            CB->joindre(P_MALROT1, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP61GAR26");
            CB->animation("P61GAR02");
            CB->dialogue("L14D172", P_MALROT1);
            CB->bloque(BLOQUE_NONE);
            ST(L14_FIGHT_STARTED) = 1;
            ST(V_ENGAGED_PERSONA) = P_MALROT1;
            CB->combat(P_MALROT1, 0);
            /* 0xAB5F */
            if (ST(ST_VIE(P_MALROT4)) == 1 && ST(L14_DELIA_KILLED_14) != 1 && ST(V_DELIA_PHASE) != 3 && ST(V_DELIA_PHASE) != 4) {
                CB->select_pile(1, 1);
                CB->evenement(1, -1);
                CB->animation("Sar_10");
                ST(V_EVENT_1) = 0;
                CB->stop_joindre(-P_MALROT4);
                CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
                CB->animation("P64ARR01_10");
                CB->modify_var(&ST(L14_MALROT4_ENGAGED), 1);
                ST(V_ENGAGED_PERSONA) = 0x1814;
                CB->combat(P_MALROT4, 0);
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0xAC43 */
    if (ST(L14_MALROT_ANIM_STEP) == 1) {
        CB->select_pile(2, 1);
        ST(L14_MALROT_ANIM_STEP) = 40;
        /* 0xAC71 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->joindre(-P_KALHI13, 4781, 1328, -1, -1);
        }
        CB->modify_var(&ST(L14_MALROT_ANIM_STEP), 2);
        CB->select_pile(0, 1);
    }
    /* 0xACBA */
    if (ST(L14_MALROT_ANIM_STEP) == 2) {
        CB->select_pile(2, 1);
        /* 0xACDC */
        if (ST(ST_VIE(P_KALHI13)) == 1 && !CB->on_joindre(P_KALHI13)) {
            ST(L14_MALROT_ANIM_STEP) = 3;
            CB->stop_joindre(-P_KALHI13);
            CB->joindre(-P_KALHI13, 5181, 979, -1, -1);
            CB->animation("Sar_10");
            CB->modify_var(&ST(L14_MALROT_ANIM_STEP), 4);
        }
        CB->select_pile(0, 1);
    }
    /* 0xAD5A */
    if (ST(L14_MALROT_ANIM_STEP) == 4) {
        CB->select_pile(2, 1);
        /* 0xAD7C */
        if (ST(ST_VIE(P_KALHI13)) == 1 && !CB->on_joindre(P_KALHI13)) {
            ST(L14_MALROT_ANIM_STEP) = 5;
            CB->pos_perso(P_KALHI13, 0, 0, 0, 4, "", -1, 0);
            CB->modify_var(&ST(ST_VIE(P_KALHI13)), 1);
            CB->vire_col(0x1A, 1);
            CB->pos_perso(P_MALROT4, 5592, 1439, 0x2D9D, 1, "P64ARR01", 2, 601);
            CB->select_pile(1, 1);
            CB->animation("Sar_10");
            CB->modify_var(&ST(L14_MALROT4_REACT_STEP), 1);
            CB->evenement(1, 2000);
            CB->select_pile(0, 1);
            ST(L14_PROVIDER_ACTIVE) = 1;
            CB->retire_zone(9);
        }
        CB->select_pile(0, 1);
    }
    /* 0xAE97 */
    if (CB->distance(1, 4) > 700 && ST(L14_SCRATCH_80) == 0 && ST(V_DELIA_GONE) == 0) {
        CB->select_pile(2, 1);
        ST(L14_SCRATCH_80) = 1;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xAF09 */
    if (CB->divers(DV_WAS_HIT, P_MALROT4, 0) && ST(ST_VIE(P_MALROT4)) != VIE_DEAD) {
        ST(ST_VIE(P_MALROT4)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_MALROT4);
        CB->combat(P_MALROT4, -1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_MALROT4, 0);
        CB->animation("SP64IMP51");
        CB->animation("P64CAD");
        CB->pos_perso(P_MALROT4, 0, 0, 0, 2, "P64CAD", 2, 0);
        ST(L14_FIGHT_STARTED) = 1;
        ST(L14_MALROT4_ENGAGED) = 1;
        CB->select_pile(0, 1);
        /* 0xAFD6 */
        if (ST(ST_VIE(P_MALROT1)) == 1) {
            CB->select_pile(2, 2);
            CB->evenement(2, -1);
            ST(V_EVENT_1) = 0;
            ST(V_ENGAGED_PERSONA) = 0x1814;
            CB->combat(P_MALROT1, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0xB045 */
    if (CB->divers(DV_WAS_HIT, P_PROVIDER, 0) && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD) {
        ST(ST_VIE(P_PROVIDER)) = VIE_DEAD;
        CB->combat(P_PROVIDER, -1);
        ST(V_ENGAGED_PERSONA) = P_PROVIDER;
        CB->stop_joindre(-P_PROVIDER);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_PROVIDER, 0);
        CB->animation("SP16IMP53");
        CB->animation("P16CAD");
        CB->pos_perso(P_PROVIDER, 0, 0, 0, 2, "P16CAD", 3, 0);
    }
    /* 0xB0EB */
    if (CB->divers(DV_WAS_HIT, P_MALROT1, 0) && ST(ST_VIE(P_MALROT1)) != VIE_DEAD) {
        CB->select_pile(2, 2);
        ST(ST_VIE(P_MALROT1)) = VIE_DEAD;
        CB->stop_joindre(-P_MALROT1);
        CB->combat(P_MALROT1, -1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_MALROT1, 0);
        CB->animation("SP61IMP52");
        CB->animation("P61CAD");
        CB->pos_perso(P_MALROT1, 0, 0, 0, 2, "P61CAD", 0x18, 0);
        ST(L14_FIGHT_STARTED) = 1;
        CB->select_pile(0, 1);
        /* 0xB1A9 */
        if (ST(ST_VIE(P_MALROT4)) == 1 && ST(L14_DELIA_KILLED_14) != 1 && ST(V_DELIA_PHASE) != 3 && ST(V_DELIA_PHASE) != 4) {
            CB->select_pile(1, 1);
            CB->evenement(1, -1);
            CB->animation("Sar_10");
            ST(V_EVENT_1) = 0;
            CB->stop_joindre(-P_MALROT4);
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
            CB->animation("P64ARR01_10");
            CB->modify_var(&ST(L14_MALROT4_ENGAGED), 1);
            ST(V_ENGAGED_PERSONA) = 0x1814;
            CB->combat(P_MALROT4, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0xB298 */
    if (CB->divers(DV_WAS_HIT, 35, 0) && ST(ST_VIE(35)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, 35, 1);
        CB->combat(35, -1);
        /* 0xB2EE */
        if (ST(L14_PERSONA35_ENGAGED) == 0) {
            ST(V_ENGAGED_PERSONA) = 35;
            ST(L14_PERSONA35_ENGAGED) = 1;
        }
        /* 0xB32F */
        if ((CB->divers(DV_HP, 35, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, 35, (CB->divers(DV_HP, 35, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, 35, 1);
        }
        /* 0xB390 */
        if (CB->divers(DV_HP, 35, -1) <= 1) {
            ST(ST_VIE(35)) = VIE_DEAD;
            CB->stop_joindre(-35);
            CB->divers_empile(DV_BLOOD_AT_BONE, 35, 0);
            CB->animation("SP35IMP51");
            CB->animation("P35CAD");
            CB->pos_perso(35, 0, 0, 0, 2, "P35CAD", 4, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-35);
            CB->divers_empile(DV_BLOOD_AT_BONE, 35, 0);
            CB->animation("SP35IMP06");
            CB->animation("P35GAR01");
            CB->combat(35, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0xB461 */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && ST(V_EVENT15_STEP) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 3) < 200 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 1;
        CB->divers(DV_LOOK_AT_PLAYER, P_PROVIDER, 1);
        CB->dialogue("L14D167", P_PROVIDER);
        CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP16GAR26");
        CB->joindre_perm(P_PROVIDER, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB55A */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(V_EVENT15_STEP) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 3) < 250 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_PROVIDER, 0);
        CB->animation("SP16GAR27");
        CB->animation("P16ARR01_10");
        CB->dialogue("L14D168", P_PHEDORIA);
        CB->stop_joindre(-P_PROVIDER);
        CB->joindre(P_PROVIDER, 3267, 915, 0xCE17, -1);
        CB->animation("SP16ARR01");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB643 */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->distance(1, 3) > 300 && ST(V_EVENT15_STEP) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_PROVIDER, 0);
        CB->stop_joindre(-P_PROVIDER);
        CB->animation("SP16GAR27");
        CB->animation("P16ARR01_10");
        CB->joindre(P_PROVIDER, 3267, 915, 0xCE17, -1);
        CB->animation("P16ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB71C */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && ST(L14_MALROT4_ENGAGED) == 1 && ST(L14_STANCE_WARNED_MALROT4) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && CB->distance(1, 2) < 200) {
        CB->select_pile(1, 1);
        CB->evenement(1, -1);
        ST(V_EVENT_1) = 1;
        ST(L14_STANCE_WARNED_MALROT4) = 1;
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_MALROT4);
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
        CB->joindre(-P_MALROT4, 0, 0, 0, ST(ST_HERO));
        CB->dialogue("L14D169", P_MALROT4);
        CB->modify_var(&ST(L14_MALROT4_ENGAGED), 1);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xB86F */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L14_STANCE_WARNED_MALROT4) == 1 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(L14_MALROT4_ENGAGED) == 1 && CB->distance(1, 2) < 200) {
        CB->bloque(BLOQUE_ALL);
        ST(L14_STANCE_WARNED_MALROT4) = 0;
        CB->dialogue("L14D170A", P_MALROT4);
        CB->dialogue("L14D170B", P_MALROT4);
        CB->stop_joindre(-P_MALROT4);
        CB->animation("SP64ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB939 */
    if (ST(L14_MALROT1_SPOKEN) != 0 && CB->distance(1, 24) > 150 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L14_MALROT1_SPOKEN) = 0;
        CB->select_pile(2, 1);
        CB->joindre(P_MALROT1, 3649, 1268, 0xE98, -1);
        CB->animation("SP61ATT04");
        CB->evenement(2, 5000);
        CB->select_pile(0, 1);
    }
    /* 0xB9F9 */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L14_STANCE_WARNED_MALROT1) == 0 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && CB->distance(1, 24) < 150) {
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L14_STANCE_WARNED_MALROT1) = 1;
        CB->stop_joindre(P_MALROT1);
        CB->evenement(2, -1);
        ST(V_EVENT_2) = 0;
        CB->joindre_perm(P_MALROT1, ST(ST_HERO));
        CB->stop_joindre(P_MALROT1);
        CB->animation("SP61GAR26");
        CB->dialogue("L14D203", P_MALROT1);
        CB->animation("P61GAR02");
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xBB15 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L14_STANCE_WARNED_MALROT1) == 1 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD) {
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L14_STANCE_WARNED_MALROT1) = 0;
        /* 0xBB9C */
        if (CB->distance(1, 24) < 200) {
            CB->dialogue("L14D204", P_MALROT1);
        }
        CB->animation("SP61GAR27");
        CB->animation("P61ARR01");
        CB->joindre(P_MALROT1, 3649, 1268, 0xE98, -1);
        CB->bloque(BLOQUE_NONE);
        CB->evenement(2, 100);
        CB->select_pile(0, 1);
    }
    /* 0xBC12 */
    if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L14_STANCE_WARNED_MALROT1) == 1 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && CB->distance(1, 24) > 300) {
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L14_STANCE_WARNED_MALROT1) = 0;
        CB->animation("SP61GAR27");
        CB->animation("P61ARR01");
        CB->joindre(P_MALROT1, 3649, 1268, 0xE98, -1);
        CB->bloque(BLOQUE_NONE);
        CB->evenement(2, 100);
        CB->select_pile(0, 1);
    }
    /* 0xBCFC */
    if (CB->distance(1, 2) > 400 && ST(L14_MALROT4_TALK_STAGE) >= 1) {
        ST(L14_MALROT4_TALK_STAGE) = 0;
    }
    /* 0xBD38 */
    if (CB->distance(1, 25) > 400 && ST(L14_TALK_COOLDOWN_84) >= 1) {
        ST(L14_TALK_COOLDOWN_84) = 0;
    }
    /* 0xBD6B */
    if (ST(V_DELIA_PHASE) == 4 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != P_DELIA) {
        CB->select_pile(2, 1);
        /* 0xBDC4 */
        if (CB->distance(33, 1) < 100 && ST(L14_DELIA_JOINED) == 0) {
            ST(L14_DELIA_JOINED) = 1;
            CB->joindre_perm(-P_DELIA, ST(ST_HERO));
            CB->stop_joindre(P_DELIA);
            CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 0x21, 0);
        }
        /* 0xBE68 */
        if (ST(L14_DELIA_FOLLOW_ON) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0xBE9B */
            if (CB->distance(33, 1) >= 180) {
                CB->divers(DV_RUN, P_DELIA, 0);
                ST(L14_DELIA_JOINED) = 0;
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xBEF7 */
            if (CB->distance(33, 1) >= 300) {
                ST(L14_DELIA_JOINED) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0xBF5F */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 33)) {
        ST(V_DELIA_PHASE) = 20;
        ST(V_DELIA_KILLED) = 14;
        ST(L14_DELIA_KILLED_14) = 1;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_DELIA, -1);
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0x21, 0);
        CB->select_pile(0, 1);
    }
    /* 0xC049 */
    if (ST(ST_LOCKED_CAMERA) == 2 && ST(L14_AMBIENT_ANIM_ON) == 0 && ST(L14_CAMERA2_ENTERED) == 0) {
        /* 0xC07F */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->play_anim(0, 1);
            CB->play_anim(1, 1);
            CB->bruitage("L14AMB4", -1, -1);
            ST(L14_AMBIENT_ANIM_ON) = 1;
        }
        /* 0xC0CF */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->play_anim(2, 1);
            CB->play_anim(3, 1);
            CB->bruitage("L14AMB3", -1, -1);
            ST(L14_AMBIENT_ANIM_ON) = 1;
        }
    }
    /* 0xC122 */
    if (ST(ST_LOCKED_CAMERA) != 2 && ST(L14_AMBIENT_ANIM_ON) == 1) {
        ST(L14_AMBIENT_ANIM_ON) = 0;
        /* 0xC155 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->bruitage("L14AMB2", -1, -1);
        }
        /* 0xC179 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->bruitage("L14AMB1", -1, -1);
        }
    }
    /* 0xC19D */
    if (ST(ST_LOCKED_CAMERA) != 2 && ST(L14_AMBIENT_SWAP_DONE) == 1) {
        ST(L14_AMBIENT_SWAP_DONE) = 0;
    }
    /* 0xC1D0 */
    if (ST(L14_CAMERA2_ENTERED) == 1 && ST(L14_AMBIENT_SWAP_DONE) == 0 && ST(ST_LOCKED_CAMERA) == 2) {
        /* 0xC206 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->bruitage("L14AMB2", -1, -1);
        }
        /* 0xC22A */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->bruitage("L14AMB1", -1, -1);
        }
        /* 0xC24E */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->pause_anim(0, 0, 0);
            CB->pause_anim(1, 0, 0);
        }
        /* 0xC27B */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->pause_anim(2, 0, 0);
            CB->pause_anim(3, 0, 0);
        }
        ST(L14_AMBIENT_SWAP_DONE) = 1;
        ST(L14_CAMERA2_LOOT_STAGE) = 0;
    }
    /* 0xC2C9 */
    if (ST(ST_HERO) == 4 && ST(L14_DEATH_EVENT_SENT) == 0) {
        ST(L14_DEATH_EVENT_SENT) = 1;
        CB->evenement(0x2C, 3000);
    }
    /* 0xC306 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0xC32C */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0xC355 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0xC381 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0xC3AD */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0xC3D1 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0xC3EE */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
    /* 0xC41F */
    if (ST(L14_PERSONA35_ENGAGED) == 0 && ST(V_DELIA_GONE) == 1 && ST(ST_VIE(35)) != VIE_DEAD && ST(ST_LOCKED_CAMERA) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 4) < 650) {
        CB->bloque(BLOQUE_ALL);
        ST(L14_PERSONA35_ENGAGED) = 1;
        CB->combat(35, 0);
        ST(V_ENGAGED_PERSONA) = 35;
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x10018A79).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1002BF6C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu14_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x10018CA4);
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
        l14_trig00();   /* 0x10018CA9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x10018B8E — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x10018CFC): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x100186E0).
 * Jump table @ 0x10018D79, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu14_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu14: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l14_trigger(arg);             break;  /* dword_1002BE00[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
