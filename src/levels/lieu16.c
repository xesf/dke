/*
 * lieu16.c — Native hand-port of LIEU16.DLL (level-16 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu16_trig.c (reached through
 * l16_trigger, see lieu16_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x10012BC0, the state
 * pointer global is dword_10024D9C and the trigger table is
 * dword_10024CB0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1001328C), lieu_pre_scene and sub_10001200
 * (float reset) are the shared lieu_common.c; the entry and INIT are
 * instruction-for-instruction identical to LIEU03's. tick_body
 * (sub_100081B1) is level-16 specific.
 */

#include <stdlib.h>

#include "lieu16_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body — sub_100081B1 (thunk 0x100010B9), level-16 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x81C3 */
    if (ST(V_DELIA_PHASE) == 2 && ST(L16_DELIA_FOLLOW_ARMED) == 0) {
        ST(L16_DELIA_FOLLOW_ARMED) = 1;
    }
    /* 0x81F6 */
    if (ST(ST_LOCKED_CAMERA) == 3 && ST(V_WEAPON_WARNED) == 1) {
        ST(V_WEAPON_WARNED) = 0;
    }
    /* 0x8226 */
    if (ST(L16_DELIA_ATTACKED) == 1 && ST(ST_VIE(P_DELIA)) == VIE_DEAD) {
        ST(L16_DELIA_ATTACKED) = 2;
        ST(V_DELIA_KILLED) = 16;
        ST(V_DELIA_PHASE) = 20;
    }
    /* 0x8274 */
    if (ST(ST_LOCKED_CAMERA) == 0 && ST(V_DELIA_PHASE) == 2 && ST(ST_VIE(P_MAOR)) == VIE_DEAD && ST(ST_VIE(90)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) == 2) {
        ST(ST_VIE(90)) = VIE_DEAD;
    }
    /* 0x82EB */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_DELIA)) == 1 && CB->direction_perso(P_DELIA)) {
        ST(V_CRI_USED) = 1;
        /* 0x8373 */
        if (ST(L16_DELIA_CRY_STATE) != 99) {
            ST(L16_DELIA_CRY_STATE) = 10;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(33, 1000);
        return;
    }
    /* 0x83B4 */
    if (ST(L16_DELIA_CRY_STATE) == 99) {
        ST(L16_DELIA_CRY_STATE) = 10;
    }
    /* 0x83E3 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_MAOR)) != VIE_DEAD && CB->direction_perso(P_MAOR)) {
        ST(V_CRI_USED) = 1;
        /* 0x846B */
        if (ST(L16_MAOR_CRY_STATE) != 99) {
            ST(L16_MAOR_CRY_STATE) = 17;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(34, 1000);
        return;
    }
    /* 0x84BA */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 5) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_MAOR)) != VIE_DEAD && CB->direction_perso(P_MAOR)) {
        ST(V_CRI_USED) = 1;
        /* 0x8542 */
        if (ST(L16_MAOR_CRY_STATE) != 99) {
            ST(L16_MAOR_CRY_STATE) = 17;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(34, 1000);
        return;
    }
    /* 0x8591 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_MAOR)) != VIE_DEAD && CB->direction_perso(P_MAOR)) {
        ST(V_CRI_USED) = 1;
        /* 0x8619 */
        if (ST(L16_MAOR_CRY_STATE) != 99) {
            ST(L16_MAOR_CRY_STATE) = 17;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(34, 1000);
        return;
    }
    /* 0x865A */
    if (ST(L16_MAOR_CRY_STATE) == 99) {
        ST(L16_MAOR_CRY_STATE) = 17;
    }
    /* 0x867B */
    if (ST(L16_MAOR_CHASING) == 1 && CB->distance(1, 2) > 180 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MAOR)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 5)) {
        CB->select_pile(1, 2);
        ST(L16_MAOR_PATROL_STEP) = 1;
        CB->joindre(-P_MAOR, 3810, 3721, 0x7DF4, -1);
        CB->evenement(1, 50);
        ST(L16_MAOR_CHASING) = 0;
        ST(L16_MAOR_ZONE2_STARTED) = 0;
        CB->select_pile(0, 1);
    }
    /* 0x8792 */
    if (!CB->on_joindre(P_DELIA) && ST(L16_DELIA_SENT_AWAY) == 0 && ST(L16_MAOR_ENCOUNTER) == 1) {
        ST(L16_MAOR_ENCOUNTER) = 2;
        ST(L16_DELIA_SENT_AWAY) = 1;
        CB->joindre(P_DELIA, 2716, 3801, 0x2823, -1);
        CB->modify_var(&ST(L16_DELIA_WALK_SYNC), 1);
    }
    /* 0x8814 */
    if (!CB->on_joindre(P_DELIA) && ST(L16_DELIA_WALK_SYNC) == 1) {
        CB->select_pile(2, 1);
        CB->joindre(P_DELIA, 2568, 3816, -1, -1);
        CB->animation("SP10ARR01_10");
        CB->ajoute_zone(18, 3308, 3125, 140, 65, 1);
        CB->pos_perso(P_DELIA, -1, -1, -1, 4, "", -1, 0);
        ST(L16_DELIA_WALK_SYNC) = 4;
        CB->select_pile(0, 1);
    }
    /* 0x88C5 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        CB->change_camera(0, CAM_LOCKED);
        CB->change_camera(0, CAM_AUTO);
        CB->joindre(ST(ST_HERO), 3325, 3298, 0x2E1, -1);
        CB->animation("ar");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8935 */
    if (ST(V_EVENT_1) == 1 && ST(ST_VIE(P_MAOR)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) != P_MAOR && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(V_EVENT_1) = 0;
        /* 0x8989 */
        if (ST(L16_MAOR_PATROL_STEP) == 1) {
            ST(L16_MAOR_PATROL_STEP) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP17ACT06");
            CB->animation("P17ARR01");
            ST(L16_RANDOM_STEP) = CB->random(1);
            /* 0x89E1 */
            if (ST(L16_RANDOM_STEP) == 0) {
                CB->joindre(-P_MAOR, 3810, 3721, 0x7DF4, -1);
                CB->modify_var(&ST(L16_MAOR_PATROL_STEP), 2);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x8A48 */
        if (ST(L16_MAOR_PATROL_STEP) == 2) {
            ST(L16_MAOR_PATROL_STEP) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP17ACT08");
            CB->animation("P17ACT05");
            CB->animation("P17ACT05");
            CB->animation("P17ACT09");
            ST(L16_RANDOM_STEP) = CB->random(2);
            /* 0x8AB6 */
            if (ST(L16_RANDOM_STEP) == 0) {
                CB->joindre(-P_MAOR, 3589, 3727, 0x7B66, -1);
                CB->modify_var(&ST(L16_MAOR_PATROL_STEP), 1);
            }
            /* 0x8AF8 */
            if (ST(L16_RANDOM_STEP) == 1) {
                CB->joindre(-P_MAOR, 3884, 3457, 0xB17C, -1);
                CB->modify_var(&ST(L16_MAOR_PATROL_STEP), 3);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x8B5F */
        if (ST(L16_MAOR_PATROL_STEP) == 3) {
            ST(L16_MAOR_PATROL_STEP) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP17ACT07");
            CB->animation("P17ARR01");
            ST(L16_RANDOM_STEP) = CB->random(2);
            /* 0x8BB7 */
            if (ST(L16_RANDOM_STEP) == 0) {
                CB->joindre(-P_MAOR, 3589, 3727, 0x7B66, -1);
                CB->modify_var(&ST(L16_MAOR_PATROL_STEP), 1);
            }
            /* 0x8BF9 */
            if (ST(L16_RANDOM_STEP) == 1) {
                CB->joindre(-P_MAOR, 3810, 3721, 0x7DF4, -1);
                CB->modify_var(&ST(L16_MAOR_PATROL_STEP), 2);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x8C60 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->joindre(ST(ST_HERO), 3447, 3356, 0x30E1, -1);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8CAB */
    if (ST(V_EVENT_35) == 1) {
        lieu_action_register("L16D666", P_MAOR, "SP17DIA51", "SP17ARR01_10");
        lieu_action_register("L16D667", P_MAOR, "SP17DIA51", "SP17ARR01_10");
        lieu_action_register("L16D668", P_MAOR, "SP17DIA51", "SP17ARR01_10");
        lieu_action_register("L16D669", P_MAOR, "SP17DIA51", "SP17ARR01_10");
        lieu_action_register("L16D670", P_MAOR, "SP17DIA51", "SP17ARR01_10");
    }
    /* 0x8D3A */
    if (ST(V_EVENT_36) == 1) {
        ST(V_EVENT_36) = 0;
        /* 0x8D5B */
        if (ST(ST_VIE(P_MAOR)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->dialogue("L16D999", -1);
        }
    }
    /* 0x8D8F */
    if (ST(V_EVENT15_STEP) == 1 && ST(ST_UPDOWN_KEY) == 1) {
        ST(V_EVENT15_STEP) = 0;
        CB->change_camera(1, CAM_AUTO);
    }
    /* 0x8DCC */
    if (ST(L16_CAM_RESET_PENDING) == 1 && ST(ST_UPDOWN_KEY) == 1) {
        ST(L16_CAM_RESET_PENDING) = 0;
        CB->change_camera(4, CAM_AUTO);
    }
    /* 0x8E0A */
    if (!CB->on_joindre(P_MAOR) && ST(L16_MAOR_WALKING) == 1) {
        CB->combat(P_MAOR, 0);
        ST(V_ENGAGED_PERSONA) = P_MAOR;
        ST(L16_MAOR_WALKING) = 0;
    }
    /* 0x8E56 */
    if (ST(ST_VIE(P_MAOR)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(L16_MAOR_HOSTILE) = 0;
    }
    /* 0x8E92 */
    if (ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_DELIA) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x8EBF */
    if (ST(ST_VIE(90)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 90 && ST(ST_HERO) != 4) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(L16_CREATURE_HOSTILE) = 0;
        /* 0x8F0D */
        if (ST(ST_VIE(P_DELIA)) == VIE_DEAD && ST(L16_DELIA_ESCAPE_STAGE) != 99) {
            ST(L16_DELIA_ESCAPE_STAGE) = 2;
        }
        /* 0x8F40 */
        if (ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
            CB->bloque(BLOQUE_ALL);
            /* 0x8F5D */
            if (ST(ST_LOCKED_CAMERA) != 3) {
                CB->change_camera(3, CAM_LOCKED);
            }
            CB->animation("P10ARR01_1000");
            CB->animation("P10ARR01_10");
            lieu_action_register("L16D125", P_DELIA, "SP10DIA51", "SP10ARR01_10");
            CB->divers(DV_COMBAT_STANCE, 0, 0);
            CB->joindre(ST(ST_HERO), 0, 0, 0, 10);
            CB->vire_col(21, 0);
            /* 0x8FE3 */
            if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L16D203", P_ARKHAN2, "SG00DIA05", NULL);
            }
            /* 0x901D */
            if (ST(ST_HERO) == P_ARKHAN2 && ST(ST_SPECIAL_MODE) == 0) {
                lieu_action_register("L16D204", P_ARKHAN2, "SG00DIA05", NULL);
            }
            /* 0x9057 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L16D205", P_ARKHAN3, "SG00DIA05", NULL);
            }
            /* 0x9088 */
            if (CB->distance(1, 3) > 550 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -2200) {
                CB->joindre(-P_DELIA, 2972, 3517, -1, -1);
                CB->animation("SP10ARR01_10");
            } else {
                ST(L16_DELIA_WAIT_RUN) = 1;
            }
            CB->bloque(BLOQUE_NONE);
            CB->change_camera(99, CAM_AUTO);
            return;
        }
    }
    /* 0x910C */
    if (ST(L16_DELIA_WAIT_RUN) == 1 && ST(ST_HERO) != 4 && CB->distance(1, 3) > 450 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -2200) {
        ST(L16_DELIA_WAIT_RUN) = 666;
        CB->bloque(BLOQUE_ALL);
        CB->joindre(-P_DELIA, 2972, 3517, -1, -1);
        CB->animation("SP10ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x91B9 */
    if (CB->divers(DV_WAS_HIT, P_MAOR, 0) && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_MAOR, 1);
        CB->combat(P_MAOR, -1);
        CB->stop_joindre(-P_MAOR);
        CB->evenement(1, -1);
        ST(V_EVENT_1) = 0;
        /* 0x9245 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_MAOR;
        }
        ST(L16_MAOR_HOSTILE) = 1;
        /* 0x9286 */
        if ((CB->divers(DV_HP, P_MAOR, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_MAOR, (CB->divers(DV_HP, P_MAOR, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_MAOR, 1);
        }
        /* 0x92E7 */
        if (CB->divers(DV_HP, P_MAOR, -1) <= 1) {
            CB->stop_joindre(-P_MAOR);
            ST(ST_VIE(P_MAOR)) = VIE_DEAD;
            CB->select_pile(2, 2);
            CB->divers_empile(DV_BLOOD_AT_BONE, 17, 0);
            CB->select_pile(1, 1);
            CB->animation("SP17IMP53");
            CB->animation("P17CAD");
            CB->pos_perso(P_MAOR, 0, 0, 0, 2, "P17CAD", 2, 0);
        } else {
            CB->stop_joindre(-P_MAOR);
            CB->divers_empile(DV_BLOOD_AT_BONE, 17, 0);
            CB->animation("SP17IMP13");
            CB->animation("P17IMP48");
            CB->animation("P17GAR03");
            CB->combat(P_MAOR, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x93DC */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(V_DELIA_KILLED) = 16;
        ST(V_DELIA_PHASE) = 20;
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_DELIA);
        CB->divers_empile(DV_BLOOD_AT_BONE, 10, 0);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 3, 0);
        /* 0x948C */
        if (ST(ST_VIE(P_MAOR)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->dialogue("L16D999", -1);
        }
        /* 0x94C0 */
        if (ST(L16_CREATURE_GATE) == 1) {
            CB->vire_col(21, 0);
        }
        /* 0x94DF */
        if (ST(L16_DELIA_ESCAPE_STAGE) == 0) {
            ST(L16_DELIA_ESCAPE_STAGE) = 99;
        }
        CB->select_pile(0, 1);
    }
    /* 0x9515 */
    if (CB->divers(DV_WAS_HIT, 90, 0) && ST(ST_VIE(90)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, 90, 1);
        CB->combat(90, -1);
        ST(V_ENGAGED_PERSONA) = 90;
        /* 0x958B */
        if ((CB->divers(DV_HP, 90, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, 90, (CB->divers(DV_HP, 90, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, 90, 1);
        }
        /* 0x95EC */
        if (CB->divers(DV_HP, 90, -1) <= 1) {
            ST(ST_VIE(90)) = VIE_DEAD;
            /* 0x960E */
            if (CB->on_joindre(90)) {
                CB->stop_joindre(-90);
            }
            CB->animation("SP90IMP52");
            CB->animation("P90CAD");
            CB->pos_perso(90, 0, 0, 0, 2, "P90CAD", 6, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            /* 0x9675 */
            if (CB->on_joindre(90)) {
                CB->stop_joindre(-90);
            }
            CB->divers_empile(DV_BLOOD_AT_BONE, 90, 0);
            CB->animation("SP90IMP11");
            CB->animation("P90IMP46");
            CB->animation("P90GAR01");
            CB->combat(90, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x96DF */
    if (ST(ST_VIE(P_DELIA)) != VIE_DEAD && ST(L16_CREATURE_GATE) != 1) {
        /* 0x9703 */
        if (ST(L16_DELIA_CAMERA_SEEN) == 0 && (ST(ST_LOCKED_CAMERA) == 4 || ST(ST_LOCKED_CAMERA) == 5) && ST(V_DELIA_PHASE) < 3) {
            ST(L16_DELIA_CAMERA_SEEN) = 1;
            CB->divers(DV_SCREEN, 1, 0);
            /* 0x976C */
            if (ST(L16_DELIA_WARNING_STAGE) == 0) {
                CB->pos_perso(P_DELIA, 2556, 3750, 0xE04F, 1, "P10ASS00", 3, 0);
            }
            /* 0x97A3 */
            if (ST(L16_DELIA_WARNING_STAGE) == 1 || ST(V_GUARD_FIGHT_DONE) == 2) {
                CB->pos_perso(P_DELIA, 2599, 3814, 0xB2E4, 1, "P10GAR01", 3, 0);
            }
            CB->divers_empile(DV_SCREEN, 3, 0);
        }
        /* 0x97FE */
        if (ST(L16_DELIA_CAMERA_SEEN) == 1 && ST(V_DELIA_PHASE) != 2 && ST(ST_LOCKED_CAMERA) == 3) {
            CB->divers(DV_SCREEN, 1, 0);
            ST(L16_DELIA_CAMERA_SEEN) = 0;
            CB->pos_perso(P_DELIA, -1, -1, -1, 4, "", -1, 0);
            CB->divers_empile(DV_SCREEN, 3, 0);
        }
    }
    /* 0x9886 */
    if (ST(V_GUARD_FIGHT_DONE) != 2) {
        /* 0x9898 */
        if (ST(L16_MAOR_CAMERA_ZONE) == 1 && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
            /* 0x98BC */
            if (ST(ST_LOCKED_CAMERA) != 4 && ST(ST_LOCKED_CAMERA) != 5) {
                CB->select_pile(1, 2);
                ST(L16_MAOR_PATROL_STEP) = 1;
                CB->evenement(1, 1000);
                ST(L16_MAOR_CAMERA_ZONE) = 0;
                /* 0x991E */
                if (ST(V_DELIA_HITS) == 0 && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
                    CB->divers(DV_SCREEN, 1, 0);
                    CB->pos_perso(P_MAOR, 3529, 3745, 0x7F4E, 1, "P17ARR01", 2, 1);
                    CB->divers_empile(DV_SCREEN, 3, 0);
                }
                /* 0x998B */
                if ((ST(V_DELIA_HITS) >= 1 || ST(ST_VIE(P_DELIA)) == VIE_DEAD || ST(L16_DELIA_HIT_FLAG) == 1) && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
                    CB->pos_perso(P_MAOR, 3500, 3340, 0x2D7C, 1, "P17GAR03", 2, 1);
                    ST(L16_MAOR_HOSTILE) = 1;
                    /* 0x9A04 */
                    if (ST(L16_MAOR_ANGRY_LINE_SAID) == 0) {
                        CB->dialogue("L16D998", P_MAOR);
                        ST(L16_MAOR_ANGRY_LINE_SAID) = 1;
                    }
                    CB->evenement(1, -1);
                    CB->combat(P_MAOR, 0);
                    ST(V_ENGAGED_PERSONA) = P_MAOR;
                    ST(V_EVENT_1) = 0;
                }
                /* 0x9A6A */
                if (ST(L16_MAOR_HOSTILE) == 1 && ST(V_GUARD_FIGHT_DONE) != 2) {
                    CB->pos_perso(P_MAOR, 3500, 3340, 0x2D7C, 1, "P17GAR03", 2, 1);
                    CB->evenement(1, -1);
                    CB->combat(P_MAOR, 0);
                    ST(V_ENGAGED_PERSONA) = P_MAOR;
                    ST(V_EVENT_1) = 0;
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0x9AF8 */
        if (ST(L16_MAOR_CAMERA_ZONE) == 0 && ST(ST_VIE(P_MAOR)) != VIE_DEAD && (ST(ST_LOCKED_CAMERA) == 4 || ST(ST_LOCKED_CAMERA) == 5)) {
            CB->select_pile(1, 2);
            CB->evenement(1, -1);
            ST(V_EVENT_1) = 0;
            ST(L16_MAOR_CAMERA_ZONE) = 1;
            CB->divers(DV_SCREEN, 1, 0);
            CB->pos_perso(P_MAOR, -1, -1, -1, 4, "", -1, 0);
            CB->divers_empile(DV_SCREEN, 3, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x9BCE */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_DELIA_PHASE) == 2 && ST(L16_CREATURE_GATE) == 0 && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
        /* 0x9C16 */
        if (ST(L16_CAMERA_LOCKED_45) == 1 && ST(ST_LOCKED_CAMERA) != 4 && ST(ST_LOCKED_CAMERA) != 5) {
            CB->select_pile(1, 3);
            CB->select_pile(0, 1);
            CB->select_pile(1, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L16_CAMERA_LOCKED_45) = 0;
            ST(V_DELIA_PHASE) = 10;
            ST(L16_CREATURE_GATE) = 1;
            ST(L16_DELIA_FOLLOW_ARMED) = 0;
            CB->divers(DV_SCREEN, 1, 0);
            CB->pos_perso(ST(ST_HERO), 2918, 3602, 0xA23B, 1, "ar", 1, 0);
            CB->pos_perso(90, 3356, 3323, 0x2994, 1, "P90GAR01", 6, 1);
            CB->pos_perso(P_DELIA, 2835, 3690, 0x9D66, 1, "P10GAR01", 3, 0);
            CB->stop_joindre(-P_DELIA);
            CB->vire_col(21, 1);
            CB->joindre_perm(P_DELIA, ST(ST_HERO));
            ST(L16_CREATURE_HOSTILE) = 1;
            CB->pos_perso(P_DELIA, 2835, 3690, 0x9D66, 1, "P10GAR01", 3, 0);
            CB->divers_empile(DV_SCREEN, 3, 0);
            lieu_action_register("L16D25", P_DELIA, "SP10DIA51", "SP10ARR01_10");
            CB->animation("SP90ATT01");
            CB->animation("P90GAR01_10");
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x9E02 */
        if (ST(L16_CAMERA_LOCKED_45) == 0 && (ST(ST_LOCKED_CAMERA) == 4 || ST(ST_LOCKED_CAMERA) == 5)) {
            ST(L16_CAMERA_LOCKED_45) = 1;
        }
    }
    /* 0x9E47 */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_DELIA_PHASE) == 2 && ST(L16_CREATURE_GATE) == 0 && ST(ST_VIE(P_MAOR)) == VIE_DEAD) {
        /* 0x9E8F */
        if (ST(L16_CAMERA_LOCKED_45) == 1 && ST(ST_LOCKED_CAMERA) != 4 && ST(ST_LOCKED_CAMERA) != 5) {
            CB->select_pile(1, 3);
            CB->select_pile(0, 1);
            CB->select_pile(1, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L16_CAMERA_LOCKED_45) = 0;
            ST(L16_CREATURE_GATE) = 1;
            CB->divers(DV_SCREEN, 1, 0);
            CB->pos_perso(ST(ST_HERO), 2918, 3602, 0xA23B, 1, "ar", 1, 0);
            CB->pos_perso(P_DELIA, 2835, 3690, 0x9D66, 1, "P10GAR01", 3, 0);
            CB->divers_empile(DV_SCREEN, 3, 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x9F97 */
        if (ST(L16_CAMERA_LOCKED_45) == 0 && (ST(ST_LOCKED_CAMERA) == 4 || ST(ST_LOCKED_CAMERA) == 5)) {
            ST(L16_CAMERA_LOCKED_45) = 1;
        }
    }
    /* 0x9FE5 */
    if (CB->distance(1, 3) < 200 && ST(ST_VIE(P_DELIA)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && ST(V_WEAPON_WARNED) == 0) {
        /* 0xA02D */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0xA03F */
            if (ST(L16_DELIA_WARNING_STAGE) == 0 && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
                CB->bloque(BLOQUE_ALL);
                CB->divers(DV_LOOK_AT_PLAYER, P_DELIA, 1);
                CB->animation("SP10ASS02");
                CB->animation("P10GAR01");
                CB->joindre(P_DELIA, 2599, 3799, 0xB2E4, -1);
                CB->joindre(ST(ST_HERO), 0, 0, 0, 10);
                CB->dialogue("L16D49", P_DELIA);
                CB->animation("SP10GAR01");
                CB->bloque(BLOQUE_NONE);
                ST(L16_DELIA_WARNING_STAGE) = 1;
                ST(V_WEAPON_WARNED) = 1;
                return;
            }
            /* 0xA111 */
            if (ST(L16_DELIA_WARNING_STAGE) == 1 && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
                CB->bloque(BLOQUE_ALL);
                /* 0xA140 */
                if (ST(L16_DELIA_WEAPON_STAGE) == 0) {
                    CB->dialogue("L16D51", P_DELIA);
                }
                /* 0xA162 */
                if (ST(L16_DELIA_WEAPON_STAGE) == 1) {
                    CB->dialogue("L16D50", P_DELIA);
                }
                /* 0xA184 */
                if (ST(L16_DELIA_WEAPON_STAGE) >= 2) {
                    CB->dialogue("L16D51", P_DELIA);
                }
                ST(V_WEAPON_WARNED) = 1;
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0xA1C2 */
            if (ST(ST_VIE(P_MAOR)) == VIE_DEAD) {
                /* 0xA1D4 */
                if (ST(L16_DELIA_WARNING_STAGE) == 0) {
                    CB->bloque(BLOQUE_ALL);
                    CB->vire_col(14, 0);
                    CB->divers(DV_LOOK_AT_PLAYER, P_DELIA, 1);
                    CB->animation("SP10ASS02");
                    CB->animation("P10GAR01");
                    CB->joindre(P_DELIA, 2578, 3814, 0xB2E4, -1);
                    CB->joindre(ST(ST_HERO), 0, 0, 0, 10);
                    CB->dialogue("L16D52", P_DELIA);
                    ST(V_WEAPON_WARNED) = 1;
                    CB->animation("SP10GAR01");
                    CB->vire_col(14, 1);
                    CB->bloque(BLOQUE_NONE);
                    ST(L16_DELIA_WARNING_STAGE) = 1;
                    return;
                }
                /* 0xA2B1 */
                if (ST(L16_DELIA_WARNING_STAGE) == 1) {
                    CB->bloque(BLOQUE_ALL);
                    CB->dialogue("L16D52", P_DELIA);
                    ST(V_WEAPON_WARNED) = 1;
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0xA2FA */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0xA30C */
            if (ST(L16_DELIA_WARNING_STAGE) == 0) {
                CB->bloque(BLOQUE_ALL);
                CB->vire_col(14, 0);
                CB->divers(DV_LOOK_AT_PLAYER, P_DELIA, 1);
                CB->animation("SP10ASS02");
                CB->animation("P10GAR01");
                CB->joindre(P_DELIA, 2578, 3814, 0xB2E4, -1);
                CB->joindre(ST(ST_HERO), 0, 0, 0, 10);
                CB->dialogue("L16D49", P_DELIA);
                CB->animation("SP10GAR01");
                CB->vire_col(14, 1);
                CB->bloque(BLOQUE_NONE);
                ST(V_WEAPON_WARNED) = 1;
                ST(L16_DELIA_WARNING_STAGE) = 1;
                return;
            }
            /* 0xA3E9 */
            if (ST(L16_DELIA_WARNING_STAGE) == 1 || ST(ST_VIE(P_MAOR)) == VIE_DEAD) {
                CB->bloque(BLOQUE_ALL);
                CB->dialogue("L16D52", P_DELIA);
                ST(V_WEAPON_WARNED) = 1;
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0xA44D */
    if (CB->distance(1, 6) < 300 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(90)) != VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 90;
        CB->combat(90, 0);
    }
    /* 0xA499 */
    if (ST(V_DELIA_PHASE) == 10 && ST(ST_VIE(90)) == VIE_DEAD) {
        ST(V_DELIA_PHASE) = 2;
    }
    /* 0xA4CC */
    if (ST(V_DELIA_PHASE) == 2 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) != P_DELIA) {
        CB->select_pile(2, 1);
        /* 0xA525 */
        if (CB->distance(3, 1) < 100 && ST(L16_DELIA_CLOSE) == 0) {
            ST(L16_DELIA_CLOSE) = 1;
            CB->joindre_perm(-P_DELIA, ST(ST_HERO));
            CB->stop_joindre(P_DELIA);
            CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 3, 0);
        }
        /* 0xA5C9 */
        if (ST(ST_VIE(90)) == VIE_DEAD) {
            /* 0xA5E4 */
            if (CB->distance(1, 33) > 150 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < 3500 && ST(L16_DELIA_FOLLOW_ARMED) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0 && CB->divers(DV_DESC_POS, P_DELIA, 3) < -2190) {
                /* 0xA667 */
                if (CB->distance(3, 1) >= 120) {
                    CB->divers(DV_RUN, P_DELIA, 0);
                    ST(L16_DELIA_CLOSE) = 0;
                    CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
                }
                /* 0xA6C3 */
                if (CB->distance(3, 1) >= 200) {
                    ST(L16_DELIA_CLOSE) = 0;
                    CB->divers(DV_RUN, P_DELIA, 1);
                    CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
                }
            }
        } else if ((ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0xA73A */
            if (CB->distance(3, 1) >= 120) {
                CB->divers(DV_RUN, P_DELIA, 0);
                ST(L16_DELIA_CLOSE) = 0;
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xA796 */
            if (CB->distance(3, 1) >= 200) {
                ST(L16_DELIA_CLOSE) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0xA7F6 */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        /* 0xA817 */
        if (ST(L16_DELIA_CRY_STATE) == 10) {
            ST(L16_DELIA_CRY_STATE) = 99;
            /* 0xA838 */
            if (ST(ST_VIE(P_MAOR)) == VIE_DEAD || ST(V_GUARD_FIGHT_DONE) == 2) {
                lieu_action_register("L16D122", P_DELIA, "SP10DIA50", "SP10ARR01_10");
            }
            /* 0xA875 */
            if (ST(V_GUARD_FIGHT_DONE) != 2 && ST(ST_VIE(P_MAOR)) != VIE_DEAD) {
                lieu_action_register("L16D906", P_DELIA, "SP10DIA50", "SP10ARR01_10");
                CB->dialogue("L16D999", -1);
                ST(V_DELIA_HITS) = 1;
            }
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xA8FD */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0xA91E */
        if (ST(L16_MAOR_CRY_STATE) == 17) {
            ST(L16_MAOR_CRY_STATE) = 99;
            /* 0xA93F */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L16D350", P_MAOR, "SP17DIA50", "SP17ARR01_10");
            }
            /* 0xA96A */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L16D351", P_MAOR, "SP17DIA50", "SP17ARR01_10");
            }
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0xA9C1 */
    if (ST(ST_HERO) == 4 && ST(L16_DEATH_EVENT_SENT) == 0) {
        ST(L16_DEATH_EVENT_SENT) = 1;
        CB->evenement(44, 3000);
    }
    /* 0xAA04 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0xAA2A */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0xAA53 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0xAA7F */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0xAAAB */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0xAACF */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0xAAEC */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x10012F59).
 * Callback ingestion (0x10012BDA..0x10012D9B), the trigger-table fill and
 * the unused dword_10024E1C = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu16_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x10013184);
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
        l16_trig00();   /* 0x10013189: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x10013075 — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x100131DC): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x10012BC0).
 * Jump table @ 0x10013259, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu16_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu16: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l16_trigger(arg);             break;  /* dword_10024CB0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
