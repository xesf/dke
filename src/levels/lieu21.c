/*
 * lieu21.c -- Native hand-port of LIEU21.DLL (level-21 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu21_trig.c (reached through
 * l21_trigger, see lieu21_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000EA60, the state
 * pointer global is dword_1001F66C and the trigger table is
 * dword_1001F580[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), lieu_action_register,
 * lieu_pre_scene and sub_10001200 (float reset) are the shared
 * lieu_common.c. event6 (sub_1000D25C) is the shared lieu_event6 too: the
 * instruction stream is the same as LIEU03's sub_10005BEC (the annotator
 * does not flag it only because it drops the two epilogue bytes
 * `pop edi; pop esi` at the jump target 0x1000D6BD). The entry and INIT are
 * instruction-for-instruction identical to LIEU03's. tick_body
 * (sub_100020B8) is level-21 specific.
 */

#include <stdlib.h>

#include "lieu21_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body -- sub_100020B8 (thunk 0x100010B9), level-21 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue (0x10004210).
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x20CA */
    if (ST(V_EVENT_36) == 1) {
        ST(V_EVENT_36) = 0;
        CB->select_pile(0, 1);
        CB->bruitage("L21AMB3", 4475, -6756);
        CB->change_piece(-21, 2);
        CB->evenement(37, 550);
    }
    /* 0x212D */
    if (ST(V_EVENT_37) == 1) {
        ST(V_EVENT_37) = 0;
        CB->select_pile(1, 2);
        CB->change_camera(1, CAM_LOCKED);
        CB->divers_empile(DV_P25_SOUND, 4160, -6524);
        CB->animation("SP25ARR01");
        CB->joindre(-P_THANANDAR25, 4301, -6292, -1, -1);
        CB->evenement(39, 1200);
        CB->animation("SP25ARR01");
        CB->pos_perso(P_THANANDAR25, 4301, -6292, CB->divers(DV_DESC_POS, P_THANANDAR25, 2), 1, "P25ARR01", 5, 0);
        CB->select_pile(2, 2);
        /* 0x2205 */
        if (ST(V_EVENT_3) == 1) {
            CB->joindre(-P_RYLSADHAR21, 3791, -6207, -1, -1);
        }
        /* 0x2230 */
        if (ST(V_EVENT_4) == 1) {
            CB->joindre(-P_ZED27, 3791, -6207, -1, -1);
        }
        CB->select_pile(0, 1);
    }
    /* 0x2268 */
    if (ST(V_EVENT_38) == 1) {
        ST(V_EVENT_38) = 0;
        CB->joindre(-ST(ST_HERO), 4470, -6122, 0x3E7A, -1);
    }
    /* 0x22AF */
    if (ST(V_EVENT_39) == 1) {
        ST(V_EVENT_39) = 0;
        CB->change_camera(2, CAM_LOCKED);
        /* 0x22E0 */
        if (ST(V_EVENT_3) == 1) {
            CB->pos_perso(P_RYLSADHAR21, -1, -1, -1, 4, "", -1, 0);
        }
        /* 0x2311 */
        if (ST(V_EVENT_4) == 1) {
            CB->pos_perso(P_ZED27, -1, -1, -1, 4, "", -1, 0);
        }
        CB->joindre(ST(ST_HERO), 0, 0, 0, P_THANANDAR25);
        CB->joindre(P_THANANDAR25, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L21D45A", P_THANANDAR25, "SP25DIA02", "SP25ARR01");
        /* 0x2391 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L21D200", P_ARKHAN2, "SG00DIA01", NULL);
        }
        /* 0x23B9 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L21D201", P_ARKHAN3, "SG00DIA01", NULL);
        }
        lieu_action_register("L21D45B", P_THANANDAR25, "SP25DIA03", "SP25ARR01");
        /* 0x23FA */
        if (ST(V_EVENT_3) == 1 && ST(V_EVENT_4) == 0) {
            lieu_action_register("L21D46A", P_THANANDAR25, "SP25DIA01", "SP25ARR01");
            lieu_action_register("L21D46B", P_THANANDAR25, "SP25DIA02", "SP25ARR01");
            lieu_action_register("L21D46C", P_THANANDAR25, "SP25DIA03", "SP25ARR01");
            lieu_action_register("L21D46D", P_THANANDAR25, "SP25DIA04", "SP25ARR01");
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x248A */
        if (ST(V_EVENT_3) == 0 && ST(V_EVENT_4) == 1) {
            lieu_action_register("L21D46A", P_THANANDAR25, "SP25DIA04", "SP25ARR01");
            lieu_action_register("L21D47", P_THANANDAR25, "SP25DIA03", "SP25ARR01");
            lieu_action_register("L21D46C", P_THANANDAR25, "SP25DIA02", "SP25ARR01");
            lieu_action_register("L21D46D", P_THANANDAR25, "SP25DIA01", "SP25ARR01");
            CB->bloque(BLOQUE_NONE);
        }
        ST(V_ENGAGED_PERSONA) = P_THANANDAR25;
        CB->combat(P_THANANDAR25, 0);
        CB->change_camera(99, CAM_AUTO);
        CB->select_pile(0, 1);
    }
    /* 0x2558 */
    if (CB->divers(DV_WAS_HIT, P_SUNSEER1, 0) && ST(ST_VIE(P_SUNSEER1)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(V_GUARD_FIGHT_DONE) != 2) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_SUNSEER1, 1);
        CB->combat(P_SUNSEER1, -1);
        ST(ST_VIE(P_SUNSEER1)) = VIE_DEAD;
        CB->stop_joindre(-P_SUNSEER1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_SUNSEER1, 0);
        CB->animation("SP18IMP51");
        CB->animation("P18CAD");
        CB->pos_perso(P_SUNSEER1, 0, 0, 0, 2, "P18CAD", 2, 0);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->select_pile(0, 1);
    }
    /* 0x265D */
    if (ST(L21_END_REQUESTED) == 0 && ST(V_GUARD_FIGHT_DONE) == 0 && ST(ST_HERO) != 4 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -600 && ((ST(ST_LOCKED_CAMERA) == 1 && CB->direction_spec(0x99BC)) || (ST(ST_LOCKED_CAMERA) == 2 && CB->direction_spec(0x642F)))) {
        ST(L21_END_REQUESTED) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x272B */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 4100) {
            CB->change_camera(6, CAM_LOCKED);
        }
        /* 0x274D */
        if (ST(ST_HERO) == P_ARKHAN1) {
            lieu_action_register("L21D90", P_ARKHAN1, "SG01DIA02", NULL);
        }
        /* 0x2775 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L21D91", P_ARKHAN2, "SG02DIA02", NULL);
        }
        /* 0x279D */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L21D92", P_ARKHAN3, "SG03DIA02", NULL);
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x27CD */
    if (ST(L21_END_REQUESTED) == 1 && ST(ST_HERO) != 4 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -700) {
        /* 0x2825 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 4100) {
            CB->change_camera(99, CAM_AUTO);
        }
        ST(L21_END_REQUESTED) = 0;
    }
    /* 0x2856 */
    if (ST(V_DEATH_EVENT_SENT) == 1 && ST(V_WEAPON_WARNED) == 0) {
        ST(V_WEAPON_WARNED) = 1;
        /* 0x288E */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x28B7 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x28E3 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x290F */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x2933 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x2950 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x297C */
    if (ST(ST_HERO) == 4 && ST(V_WEAPON_WARNED) == 0) {
        ST(V_WEAPON_WARNED) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x29BF */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x29F0 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x2A19 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x2A45 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x2A71 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x2A95 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x2AB2 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x2ADE */
    if (ST(V_GUARD_FIGHT_DONE) == 1) {
        /* 0x2AED */
        if (ST(V_ENGAGED_PERSONA) == P_THANANDAR25 && CB->divers(DV_HP, ST(ST_HERO), -1) < (ST(L21_HERO_HP_SNAPSHOT) - 10)) {
            CB->select_pile(0, 1);
            ST(V_ENGAGED_PERSONA) = 0;
            CB->bloque(BLOQUE_ALL);
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            /* 0x2B5E */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation(lieu_anim_name(P_ARKHAN2, 24));
            }
            /* 0x2B8A */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation(lieu_anim_name(P_ARKHAN3, 24));
            }
            CB->combat(P_THANANDAR25, -1);
            CB->animation("P25ARR01");
            CB->joindre(P_THANANDAR25, 0, 0, 0, ST(ST_HERO));
            ST(L21_THANANDAR_KNOCKED_OUT) = 1;
            lieu_action_register("L21D71", P_THANANDAR25, "SP25DIA01", "SP25ARR01");
            CB->animation("SP25ARR01_50");
            CB->evenement(41, 1500);
            CB->joindre(-P_THANANDAR25, 4632, -5774, -1, -1);
            CB->select_pile(0, 1);
        }
        /* 0x2C57 */
        if (ST(V_EVENT_41) == 1) {
            ST(V_EVENT_41) = 0;
            CB->divers_empile(DV_SCREEN, 5, 0);
            CB->pos_perso(P_THANANDAR25, -1, -1, -1, 4, "", -1, 0);
            /* 0x2CB0 */
            if (CB->distance(1, 10) < 100) {
                CB->pos_perso(ST(ST_HERO), 4551, -6020, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            }
            CB->ajoute_zone(20, 3927, -6182, 50, 235, 0);
            CB->ajoute_zone(16, 3782, -6127, 120, 70, 1);
            CB->retire_zone(13);
            /* 0x2D45 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02ATT01_2000");
            }
            /* 0x2D62 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03ATT01_2000");
            }
            CB->modify_var(&ST(L21_SCENE_SYNC), 2);
        }
        /* 0x2D90 */
        if (ST(L21_SCENE_SYNC) == 2) {
            ST(L21_SCENE_SYNC) = 0;
            CB->divers_empile(DV_SCREEN, 3, 0);
            CB->animation(lieu_anim_name(P_ARKHAN2, 6));
            CB->animation("ar_50");
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x2DED */
        if (ST(L21_CAMERA_STATE) == 0 && CB->distance(1, 20) < 50) {
            CB->change_camera(7, CAM_LOCKED);
            ST(L21_CAMERA_STATE) = 1;
        }
        /* 0x2E37 */
        if (ST(L21_CAMERA_STATE) == 1 && CB->distance(1, 20) > 200) {
            CB->change_camera(99, CAM_AUTO);
            ST(L21_CAMERA_STATE) = 0;
        }
    }
    /* 0x2E83 */
    if (ST(V_GUARD_FIGHT_DONE) == 2) {
        /* 0x2E95 */
        if (ST(V_PASSAGE_OPEN) == 0) {
            /* 0x2EA7 */
            if (ST(L21_CAMERA_STATE) == 0 && CB->distance(1, 20) < 50) {
                CB->change_camera(7, CAM_LOCKED);
                ST(L21_CAMERA_STATE) = 1;
            }
        } else if (ST(L21_CAMERA_STATE) == 0 && CB->distance(1, 20) < 50) {
            ST(ST_LEVEL) = 21;
            CB->evenement(EV_MUSIC_LOOP_A, -1);
            CB->change_piece(22, 0);
            ST(L21_CAMERA_STATE) = 1;
        }
    }
    /* 0x2F59 */
    if (ST(V_ALERT_STAGE) == 2) {
        /* 0x2F6B */
        if (ST(L21_CAMERA_STATE) == 0 && CB->distance(1, 20) < 50) {
            CB->change_camera(7, CAM_LOCKED);
            ST(L21_CAMERA_STATE) = 1;
        }
        /* 0x2FB5 */
        if (ST(L21_CAMERA_STATE) == 1 && CB->distance(1, 20) > 200) {
            CB->change_camera(99, CAM_AUTO);
            ST(L21_CAMERA_STATE) = 0;
        }
    }
    /* 0x3001 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x301F */
        if (ST(L21_EVENT1_PHASE) == 0) {
            ST(L21_EVENT1_PHASE) = 1;
            CB->ajoute_zone(20, 3927, -6182, 50, 235, 1);
            CB->ajoute_zone(16, 3782, -6127, 120, 70, 1);
            CB->joindre(-ST(ST_HERO), 4115, -6213, 0x2A82, -1);
            CB->select_pile(1, 1);
            CB->change_camera(6, CAM_LOCKED);
            CB->bruitage("L21AMB4", 3825, -6073);
            CB->divers_empile(DV_BIG_ANIM, 1, 0);
            CB->evenement(1, 11500);
            CB->select_pile(0, 1);
        } else {
            CB->bloque(BLOQUE_NONE);
            ST(L21_EVENT1_PHASE) = 0;
            CB->bruitage("L21AMB3", 4475, -6756);
            CB->change_piece(-21, 2);
            CB->change_camera(99, CAM_AUTO);
        }
    }
    /* 0x314E */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(L21_LORY_SCENE_ARMED) == 1) {
        /* 0x3172 */
        if (ST(V_EVENT_2) == 1) {
            ST(V_EVENT_2) = 0;
            /* 0x3190 */
            if (ST(L21_LORY_STATE) == 0) {
                CB->select_pile(1, 1);
                CB->joindre(P_LORY, 4853, -5583, 0xE020, -1);
                CB->evenement(2, 2000);
                ST(L21_LORY_STATE) = 1;
                CB->select_pile(0, 1);
            } else {
                CB->select_pile(2, 1);
                CB->joindre(P_PHEDORIA, 4403, -6106, 0xDCCB, -1);
                CB->bloque(BLOQUE_NONE);
                ST(L21_PHEDORIA_STATE) = 1;
                CB->select_pile(0, 1);
            }
        }
        /* 0x3246 */
        if (ST(L21_LORY_STATE) == 1 && CB->distance(3, 11) < 25) {
            CB->select_pile(1, 1);
            CB->stop_joindre(P_LORY);
            CB->pos_perso(P_LORY, -1, -1, -1, 4, "", -1, 0);
            ST(L21_LORY_STATE) = 2;
            ST(L21_LORY_ARRIVED_TIME) = ST(ST_GAME_TIME);
            CB->retire_zone(11);
            CB->select_pile(0, 1);
        }
        /* 0x32E3 */
        if (ST(L21_PHEDORIA_STATE) == 1 && CB->distance(4, 12) < 25) {
            CB->select_pile(2, 1);
            CB->joindre(-P_PHEDORIA, 4853, -5683, 0xE020, -1);
            ST(L21_PHEDORIA_STATE) = 2;
            CB->select_pile(0, 1);
        }
        /* 0x3350 */
        if (ST(L21_PHEDORIA_STATE) == 2 && CB->distance(4, 21) < 25) {
            CB->select_pile(2, 1);
            CB->stop_joindre(P_PHEDORIA);
            CB->pos_perso(P_PHEDORIA, -1, -1, -1, 4, "", -1, 0);
            ST(L21_PHEDORIA_STATE) = 3;
            CB->retire_zone(21);
            CB->retire_zone(12);
            CB->select_pile(0, 1);
        }
        /* 0x33E4 */
        if (ST(V_EVENT_34) == 1) {
            ST(V_EVENT_34) = 0;
            CB->divers(DV_HP, ST(ST_HERO), 0);
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            CB->animation(lieu_anim_name(P_ARKHAN2, 26));
            CB->modify_var(&ST(V_DEATH_EVENT_SENT), 1);
        }
        /* 0x3466 */
        if (ST(V_EVENT_35) == 1) {
            ST(V_EVENT_35) = 0;
            CB->animation(lieu_anim_name(P_ARKHAN2, 24));
            CB->animation(lieu_anim_name(P_ARKHAN2, 6));
            CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP26GAR17");
            CB->animation("P26GAR17");
            CB->evenement(34, 250);
            CB->animation("G26FRA01");
            CB->animation("P26GAR03");
        }
        /* 0x3511 */
        if (ST(ST_COMBAT_STANCE) == 1 && ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 9, 2) && (CB->direction_perso(P_LORY) || CB->direction_perso(P_PHEDORIA))) {
            CB->bloque(BLOQUE_ALL);
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            ST(L21_LORY_STATE) = 100;
            ST(L21_PHEDORIA_STATE) = 100;
            CB->select_pile(1, 2);
            CB->joindre(-P_LORY, 0, 0, 0, ST(ST_HERO));
            CB->select_pile(2, 2);
            CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP26GAR03");
            CB->dialogue("L20D38", P_PHEDORIA);
            CB->animation("SG26FRA01");
            CB->evenement(34, 750);
            CB->animation("P26GAR03");
            CB->select_pile(0, 1);
        }
    }
    /* 0x364F */
    if (ST(V_GUARD_FIGHT_DONE) == 0 && ST(V_EVENT_3) == 0 && ST(V_EVENT_4) == 0) {
        /* 0x3685 */
        if (ST(L21_SUNSEER1_STATE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_SUNSEER1)) != VIE_DEAD && CB->distance(1, 2) > 300) {
            CB->divers(DV_LOOK_AT_PLAYER, P_SUNSEER1, 0);
            ST(L21_SUNSEER1_STATE) = 0;
            CB->stop_joindre(-P_SUNSEER1);
            CB->joindre(P_SUNSEER1, 4483, -7321, 0x850C, -1);
        }
        /* 0x371B */
        if (ST(L21_SUNSEER1_STATE) == 0 && CB->direction_perso(P_SUNSEER1) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_SUNSEER1)) != VIE_DEAD && CB->distance(1, 2) < 200) {
            /* 0x377C */
            if (ST(L21_SUNSEER1_FATE) == 0) {
                ST(L21_SUNSEER1_FATE) = 1;
            }
            /* 0x379A */
            if (ST(ST_COMBAT_STANCE) == 1 && ST(V_PLAYER_STRIKE) != 0) {
                CB->select_pile(1, 2);
                ST(V_ENGAGED_PERSONA) = P_SUNSEER1;
                CB->divers(DV_LOOK_AT_PLAYER, P_SUNSEER1, 0);
                CB->combat(P_SUNSEER1, 1);
                CB->select_pile(0, 1);
                return;   /* jmp 0x10004210 */
            } else {
                ST(L21_SUNSEER1_STATE) = 5;
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_SUNSEER1);
                CB->joindre(P_SUNSEER1, 0, 0, 0, ST(ST_HERO));
                CB->modify_var(&ST(L21_SUNSEER1_STATE), 2);
            }
        }
        /* 0x3869 */
        if (ST(L21_SUNSEER1_STATE) == 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_SUNSEER1)) != VIE_DEAD && !CB->on_joindre(P_SUNSEER1)) {
            /* 0x38AF */
            if (ST(ST_HERO) > 1 && ST(L21_TALK_SUNSEER1_A2) < 2) {
                CB->divers(DV_LOOK_AT_PLAYER, P_SUNSEER1, 1);
                CB->animation("SP18GAR01");
                CB->dialogue("L21D19", P_SUNSEER1);
            }
            CB->joindre_perm(P_SUNSEER1, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            ST(L21_SUNSEER1_STATE) = 1;
        }
        /* 0x392D */
        if (ST(V_EVENT_33) == 1) {
            ST(V_EVENT_33) = 0;
            /* 0x394E */
            if (ST(L21_CAMERA_STATE) == 18) {
                /* 0x3960 */
                if (ST(ST_HERO) == P_ARKHAN1) {
                    CB->select_pile(1, 1);
                    CB->stop_joindre(-P_SUNSEER1);
                    CB->animation("SP18GAR01");
                    CB->divers(DV_LOOK_AT_PLAYER, P_SUNSEER1, 1);
                    CB->joindre_perm(P_SUNSEER1, ST(ST_HERO));
                    CB->select_pile(2, 1);
                    CB->dialogue("L21D20A", P_SUNSEER1);
                    CB->dialogue("L21D20B", P_SUNSEER1);
                }
                /* 0x39F0 */
                if (ST(ST_HERO) > 1) {
                    CB->select_pile(1, 1);
                    ST(ST_VIE(P_SUNSEER1)) = VIE_DEAD;
                    ST(L21_SUNSEER1_FATE) = 2;
                    CB->combat(P_SUNSEER1, -1);
                    CB->stop_joindre(-P_SUNSEER1);
                    CB->divers(DV_LOOK_AT_PLAYER, P_SUNSEER1, 0);
                    CB->animation("SP18IMP16");
                    CB->animation("P18_65535");
                    CB->pos_perso(P_SUNSEER1, 0, 0, 0, 2, "P18CAD", 2, 0);
                    CB->divers_empile(DV_SCREEN, 2, 0);
                }
            }
            CB->animation("Sar_10");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x3AD5 */
        if (ST(ST_VIE(P_SUNSEER1)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            /* 0x3B02 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->pos_perso(P_ZED84, 4575, -5903, 0x8256, 1, "P84ARR01", 6, 0);
                CB->link_objet(P_ZED84, "OB68", 29, 1);
            }
            /* 0x3B4D */
            if (ST(ST_HERO) > 1) {
                ST(L21_SUNSEER1_FATE) = 2;
            }
        }
        /* 0x3B6E */
        if (ST(ST_HERO) == P_ARKHAN1 && ST(ST_VIE(P_SUNSEER1)) == VIE_DEAD) {
            /* 0x3B92 */
            if (ST(L21_ZED_AMBUSH) == 0 && (CB->distance(1, 6) < 600 || ST(ST_LOCKED_CAMERA) == 2 || CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > -6627)) {
                ST(L21_ZED_AMBUSH) = 1;
                CB->bloque(BLOQUE_ALL);
                CB->change_camera(2, CAM_LOCKED);
                CB->joindre(-P_ZED84, 4569, -6103, 0x7F56, -1);
                CB->dialogue("L21D22", P_ZED84);
                lieu_action_register("L21D23", P_ZED84, "SP84DIA60", "SP84ARR01");
                CB->joindre(-P_ZED84, 0, 0, 0, ST(ST_HERO));
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_ZED84);
                CB->change_camera(99, CAM_AUTO);
                lieu_action_register("L21D24", P_ARKHAN1, "SG01DIA08", NULL);
                lieu_action_register("L21D25A", P_ZED84, "SP84DIA59", "SP84ARR01");
                lieu_action_register("L21D25B", P_ZED84, "SP84DIA60", "SP84ACT01");
                CB->evenement(0, 1600);
            }
            /* 0x3D02 */
            if (ST(V_EVENT_0) == 1) {
                ST(V_EVENT_0) = 0;
                CB->divers(DV_HP, ST(ST_HERO), 0);
                CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
                /* 0x3D53 */
                if (ST(ST_PLAYER_WEAPON) == 0) {
                    CB->animation("SP01IMP51");
                }
                /* 0x3D70 */
                if (ST(ST_PLAYER_WEAPON) == OB_AX26) {
                    CB->animation("SP01IMP53");
                }
                /* 0x3D8D */
                if (ST(ST_PLAYER_WEAPON) == OB_SABER38) {
                    CB->animation("SP01IMP52");
                }
                CB->animation("P84ARR01");
                CB->joindre(-P_ZED84, -1, -1, -1, ST(ST_HERO));
            }
            /* 0x3DD5 */
            if (ST(L21_ZED_AMBUSH) == 1 && CB->distance(1, 6) < 100) {
                ST(L21_ZED_AMBUSH) = 2;
                CB->stop_joindre(-P_ZED84);
                CB->animation("SP84ACT05");
                CB->dialogue("L21D44", P_ZED84);
                CB->animation("P84ARR01_50");
                CB->modify_var(&ST(V_DEATH_EVENT_SENT), 1);
            }
        }
    }
    /* 0x3E57 */
    if (ST(V_RETURN_PART) == 1 && ST(V_GUARD_FIGHT_DONE) == 2) {
        /* 0x3E7B */
        if (ST(V_EVENT_33) == 1) {
            ST(V_EVENT_33) = 0;
            /* 0x3E9C */
            if (ST(L21_CAMERA_STATE) == 19) {
                CB->dialogue("L21D89", P_SUNSEER2);
            }
            CB->animation("Sar_10");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x3EF2 */
        if (ST(V_LORY_STAGE) != 0 && ST(ST_VIE(P_SUNSEER2)) != VIE_DEAD) {
            /* 0x3F16 */
            if (ST(L21_SUNSEER2_GREETED) == 0 && CB->distance(1, 7) < 300) {
                ST(L21_SUNSEER2_GREETED) = 1;
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                CB->joindre(P_SUNSEER2, 0, 0, 0, ST(ST_HERO));
                /* 0x3F8A */
                if (ST(L21_TALK_SUNSEER2) < 2) {
                    lieu_action_register("L21D73", P_SUNSEER2, "SP19DIA51", "SP19DIA51");
                }
                /* 0x3FB5 */
                if (ST(L21_TALK_SUNSEER2) == 2) {
                    lieu_action_register("L21D84B", P_SUNSEER2, "SP19ATT02", "SP19ATT04");
                }
                CB->joindre_perm(P_SUNSEER2, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;   /* jmp 0x10004210 */
            }
            /* 0x4004 */
            if (ST(ST_VIE(P_SUNSEER2)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
                ST(V_ENGAGED_PERSONA) = 0;
            }
            /* 0x4031 */
            if (CB->divers(DV_WAS_HIT, P_SUNSEER2, 0) && ST(ST_VIE(P_SUNSEER2)) != VIE_DEAD) {
                CB->select_pile(1, 2);
                CB->divers_empile(DV_WAS_HIT, P_SUNSEER2, 1);
                CB->combat(P_SUNSEER2, -1);
                ST(ST_VIE(P_SUNSEER2)) = VIE_DEAD;
                CB->stop_joindre(-P_SUNSEER2);
                CB->divers_empile(DV_BLOOD_AT_BONE, P_SUNSEER2, 0);
                CB->animation("SP19IMP51");
                CB->animation("P19CAD");
                CB->pos_perso(P_SUNSEER2, 0, 0, 0, 2, "P19CAD", 7, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
                CB->select_pile(0, 1);
            }
        }
        /* 0x410C */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_SUNSEER2)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
    }
    /* 0x4139 */
    if (ST(L21_THANANDAR_SCENE) == 1 && ST(V_ENGAGED_PERSONA) != 0 && ST(V_EVENT15_STEP) == 0 && CB->distance(5, 13) > 100) {
        /* 0x4185: ebx = distance(5, 13) - 100; compared with distance(1, 13) */
        if (CB->distance(5, 13) - 100 > CB->distance(1, 13)) {
            ST(V_EVENT15_STEP) = 1;
            CB->combat(P_THANANDAR25, -1);
            CB->divers_empile(DV_P25_SOUND, 3935, -6194);
            CB->animation("SP25ARR01_500");
            CB->modify_var(&ST(V_EVENT15_STEP), 0);
            CB->combat(P_THANANDAR25, 0);
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) -- arming tail of FonctionEditor case 1 (0x1000EDF9).
 * Callback ingestion (0x1000EA7A..0x1000EDEF), the trigger-table fill and
 * the unused state+0xC8 copy are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu21_trig.c. The DLL has no
 * writable globals of its own in the level code, so there is nothing to
 * reset here.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending,
 * 0x1000F022); every other path returns state[0].
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
        l21_trig00();   /* 0x1000F029: call sub_10001073 -> sub_10001200 */
        return 0;
    }
    /* 0x1000EF0E -- the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000F07C): drain pending sound, run body.
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
 * Entry point -- @FonctionEditor@16 (thunk 0x10001046 -> 0x1000EA60).
 * Jump table @ 0x1000F0F9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu21_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu21: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l21_trigger(arg);             break;  /* dword_1001F580[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
