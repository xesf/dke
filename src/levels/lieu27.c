/*
 * lieu27.c - Native hand-port of LIEU27.DLL (level-27 scripting): the
 * @FonctionEditor@16 entry, INIT, trigger 0, the per-frame tick and the tick
 * body. The other zone trigger handlers live in lieu27_trig.c (reached
 * through l27_trigger, see lieu27_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000D2D0, the state
 * pointer global is dword_1001D93C and the trigger table is
 * dword_1001D850[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register and lieu_pre_scene (pre90_scene) are the shared
 * lieu_common.c; the entry and INIT follow LIEU25. Trigger 0 and the tick
 * body (sub_100082DC) are level-27 specific. The level code uses one DLL
 * .data global, dword_1001AB34 (g_dest_room), reset at the top of INIT.
 */

#include <stdlib.h>

#include "lieu27_int.h"

i32 g_dest_room;   /* dword_1001AB34 */

/* The DLL is loaded afresh for every room: its globals restart. Called by
 * INIT, and by tools/lieu_difftest_harness.c before every case. */
void l27_reset_dll_globals(void) {
    g_dest_room = 0;
}

/* -------------------------------------------------------------------------
 * Trigger 0 - sub_100073F0 (thunk sub_10001073): INIT default, seeds the
 * story state per start point and places the room.
 * ---------------------------------------------------------------------- */
void l27_trig00(void) {
    /* 0x7402 */
    if (ST(ST_START_POINT(27)) == 1) {
        ST(V_EVENT_17) = 2;
        ST(ST_HERO) = P_ARKHAN1;
        ST(L27_PHEDORIA_STAGE) = 10;
    }
    /* 0x7441 */
    if (ST(ST_START_POINT(27)) == 2) {
        ST(V_RETURN_PART) = 1;
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_17) = 3;
        ST(L27_PHEDORIA_STAGE) = 10;
    }
    /* 0x748F */
    if (ST(ST_START_POINT(27)) == 3) {
        ST(V_RETURN_PART) = 1;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(ST_HERO) = P_ARKHAN2;
        ST(L27_PHEDORIA_STAGE) = 5;
        ST(V_EVENT_17) = 3;
    }
    /* 0x74EC */
    if (ST(ST_START_POINT(27)) == 4) {
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_EVENT_17) = 3;
        ST(L27_PHEDORIA_STAGE) = 10;
    }
    /* 0x752B */
    if (ST(ST_START_POINT(27)) == 5) {
        ST(V_RETURN_PART) = 1;
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_LORY_STAGE) = 1;
        ST(L27_STORY_MARK_C48) = 99;
        ST(V_GUARD_FIGHT_DONE) = 2;
    }
    /* 0x7588 */
    if (ST(ST_START_POINT(27)) == 6) {
        ST(V_RETURN_PART) = 1;
        ST(V_GUARD_FIGHT_DONE) = 2;
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_LORY_STAGE) = 2;
        ST(ST_LEVEL) = 29;
        ST(L27_STORY_MARK_C48) = 100;
    }
    ST(ST_START_POINT(27)) = 0;
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
    /* 0x76A5 */
    if (ST(V_LORY_STAGE) == 0) {
        CB->divers(DV_MUSIC, 1, 0);
    }
    /* 0x76C9 */
    if (ST(V_LORY_STAGE) != 0) {
        CB->divers(DV_MUSIC, 2, 0);
    }
    ST(L27_UNUSED_64) = 0;
    ST(L27_UNUSED_68) = 0;
    ST(L27_UNUSED_6C) = 0;
    ST(L27_LORY_PATH_STEP) = 0;
    ST(L27_PHEDORIA_PATH_STEP) = 0;
    ST(L27_KONKALITE_SCENE_TIME) = 0;
    ST(L27_KONKALITE_SCENE_ON) = 0;
    ST(L27_ZONE33_ENTERED) = 0;
    ST(ST_SCRIPT_BUSY) = 0;
    ST(V_ENGAGED_PERSONA) = 0;
    /* 0x776B */
    if (ST(V_RETURN_PART) == 0 && ST(V_GUARD_FIGHT_DONE) == 0 && ST(V_EVENT_17) == 2) {
        CB->load_anim("227CIN01", 0);
    } else {
        /* 0x77B3 */
        if (ST(V_GUARD_FIGHT_DONE) == 0 || (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_PASSAGE_OPEN) == 0)) {
            CB->load_anim("227OUV28", 1);
            CB->load_anim("227OUV29", 2);
            CB->load_anim("227OUV27", 3);
        }
        /* 0x7819 */
        if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_PASSAGE_OPEN) == 1) {
            CB->load_anim("227NUI28", 1);
            CB->load_anim("227NUI29", 2);
            CB->load_anim("227OUV20", 3);
        }
    }
    ST(V_EVENT15_STEP) = 0;
    ST(L27_UNUSED_90) = 0;
    ST(L27_UNUSED_94) = 0;
    ST(L27_UNUSED_98) = 0;
    ST(L27_GATE_SCENE_DONE) = 0;
    ST(L27_CAMERA_LOCK_ZONE) = 0;
    ST(L27_UNUSED_F0) = 0;
    /* 0x78D3 */
    if (ST(ST_LEVEL) != 28 && ST(ST_LEVEL) != 29 && ST(ST_LEVEL) != 20) {
        CB->pos_perso(ST(ST_HERO), 5756, -4518, 0xBF24, 1, "ar", 1, 0);
    } else {
        /* 0x7930 */
        if (ST(ST_LEVEL) == 20) {
            CB->pos_perso(ST(ST_HERO), 5756, -4518, 0xBF24, 1, "ar", 1, 0);
        }
        /* 0x796A */
        if (ST(ST_LEVEL) == 28) {
            CB->pos_perso(ST(ST_HERO), 7429, -4815, 0x39ED, 1, "ar", 1, 0);
        }
        /* 0x79A4 */
        if (ST(ST_LEVEL) == 29) {
            CB->pos_perso(ST(ST_HERO), 7480, -4490, 0x3B87, 1, "ar", 1, 0);
        }
    }
    CB->ajoute_zone(2, 7517, -4823, 80, 150, 0);
    /* 0x79FF */
    if (ST(V_RETURN_PART) == 1 && ST(ST_VIE(36)) != 1 && ST(L27_STORY_MARK_C48) != 100) {
        CB->ajoute_zone(6, 5625, -4509, 300, 150, 0);
    }
    /* 0x7A56 */
    if (ST(V_RETURN_PART) == 1) {
        CB->ajoute_zone(40, 5712, -5051, 150, 140, 0);
        CB->ajoute_zone(39, 5716, -4782, 165, 220, 0);
        CB->vire_col(0x28, 1);
        CB->vire_col(0x2F, 1);
        CB->vire_col(0x29, 0);
        CB->vire_col(0x2A, 1);
        CB->vire_col(0x2B, 0);
        CB->vire_col(0x2C, 0);
        CB->vire_col(0x2E, 0);
        CB->vire_col(0x30, 0);
        CB->ajoute_zone(15, 6080, -5132, 320, 115, 1);
        CB->ajoute_zone(16, 7080, -5132, 320, 115, 1);
        CB->ajoute_zone(17, 7080, -4168, 320, 115, 1);
        CB->ajoute_zone(18, 6610, -4168, 460, 115, 1);
        CB->ajoute_zone(19, 6150, -4168, 460, 115, 1);
        CB->ajoute_zone(20, 5980, -4168, 10, 10, 0);
        CB->ajoute_zone(21, 6180, -4168, 10, 10, 0);
        CB->ajoute_zone(22, 6380, -4168, 10, 10, 0);
        CB->ajoute_zone(23, 6580, -4168, 10, 10, 0);
        CB->ajoute_zone(24, 6780, -4168, 10, 10, 0);
        CB->ajoute_zone(25, 6980, -4168, 10, 10, 0);
        CB->ajoute_zone(26, 7180, -4168, 10, 10, 0);
        CB->ajoute_zone(27, 7180, -5132, 10, 10, 0);
        CB->ajoute_zone(28, 6980, -5132, 10, 10, 0);
        CB->ajoute_zone(29, 6180, -5132, 10, 10, 0);
        CB->ajoute_zone(30, 5980, -5132, 10, 10, 0);
    }
    /* 0x7CDA */
    if (ST(V_EVENT_17) == 2) {
        CB->pos_perso(ST(ST_HERO), 5876, -4516, 0xA7E8, 1, "ar", 1, 0);
        CB->pos_perso(P_KONKALITE4, 5943, -4930, 0x122C, 1, "P74GAR03", 0x24, 1);
    }
    /* 0x7D3C */
    if (ST(L27_PHEDORIA_STAGE) == 4 || ST(L27_PHEDORIA_STAGE) == 5) {
        /* 0x7D60 */
        if (ST(L27_LEAVE_TIME) != -1) {
            ST(L27_AWAY_TIME_MS) = (ST(ST_GAME_TIME) - ST(L27_LEAVE_TIME));
        } else {
            ST(L27_AWAY_TIME_MS) = -1;
        }
        CB->vire_col(0x29, 0);
        CB->vire_col(0x28, 1);
        CB->vire_col(0x2F, 1);
        CB->vire_col(0x2A, 1);
        CB->vire_col(0x2B, 0);
        CB->vire_col(0x2C, 0);
        CB->vire_col(0x2E, 0);
        CB->pos_perso(ST(ST_HERO), 5750, -4646, 0xAC32, 1, "ar", 1, 0);
        /* 0x7E38 */
        if (ST(L27_AWAY_TIME_MS) >= 0 && ST(L27_AWAY_TIME_MS) < 6000) {
            CB->pos_perso(P_LORY, 5805, -4974, 0xB6B6, 1, "P15ARR01", 8, 0);
            CB->pos_perso(P_PHEDORIA, 5916, -4996, 0xC4EB, 1, "P26ARR01", 9, 0);
            CB->ajoute_zone(10, 6468, -4952, 20, 20, 0);
            CB->ajoute_zone(11, 6680, -4824, 20, 20, 0);
            CB->ajoute_zone(12, 6680, -4480, 20, 20, 0);
            CB->ajoute_zone(13, 7437, -4500, 20, 20, 0);
            CB->ajoute_zone(14, 7901, -4274, 20, 20, 0);
        }
        /* 0x7F33 */
        if (ST(L27_AWAY_TIME_MS) >= 6000) {
            ST(L27_PHEDORIA_STAGE) = 10;
        }
    }
    /* 0x7F54 */
    if (ST(L27_PHEDORIA_STAGE) == 4 || ST(L27_PHEDORIA_STAGE) == 5) {
        CB->ajoute_zone(3, 7513, -4475, 80, 150, 1);
    } else {
        CB->ajoute_zone(3, 7513, -4475, 80, 150, 0);
    }
    /* 0x7FB9 */
    if (ST(L27_STORY_MARK_C48) == 100) {
        CB->vire_col(0x2E, 1);
        CB->ajoute_zone(38, 6560, -4650, 20, 20, 0);
        CB->pos_perso(36, 5756, -4518, 0xBF24, 1, "P36GAR01", 0x25, 1);
    }
    /* 0x801B */
    if (ST(L27_STORY_MARK_C48) == 111) {
        i32 pz = CB->divers(DV_DESC_POS, 36, 2);   /* arguments are evaluated last to first */
        i32 pa = CB->divers(DV_DESC_POS, 36, 1);
        i32 px = CB->divers(DV_DESC_POS, 36, 0);
        CB->pos_perso(36, px, pa, pz, 2, "P36CAD", 0x25, 0);
    }
    CB->ajoute_zone(33, 6554, -5036, 175, 155, 0);
    CB->ajoute_zone(34, 6970, -4658, 150, 200, 0);
    CB->ajoute_zone(35, 6426, -4618, 175, 155, 0);
}

/* -------------------------------------------------------------------------
 * Per-frame body - sub_100082DC (thunk 0x100010B9), level-27 specific.
 * Ported from disassembly (labels in comments = original addresses).
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x82EE */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(0x2C, 3000);
    }
    /* 0x8331 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x8362 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x838B */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x83B7 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x83E3 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8407 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x8424 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x8456 */
    if (CB->divers(DV_PLAYER_ZONE, 0, 0) == 51 && CB->direction_zone(33) && ST(L27_ZONE33_ENTERED) == 0) {
        ST(L27_ZONE33_ENTERED) = 1;
        CB->select_pile(0, 2);
        CB->bloque(BLOQUE_ALL);
        /* 0x84AE */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L51D111", P_ARKHAN2, "SG02DIA01", NULL);
        }
        /* 0x84D6 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L51D112", P_ARKHAN3, "SG03DIA01", NULL);
        }
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8506 */
    if (ST(L27_STORY_MARK_C48) == 100) {
        /* 0x8520 */
        if (CB->divers(DV_WAS_HIT, 36, 0) && ST(ST_VIE(36)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, 36, 1);
            CB->combat(36, -1);
            /* 0x858A */
            if ((CB->divers(DV_HP, 36, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, 36, (CB->divers(DV_HP, 36, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, 36, 1);
            }
            /* 0x85EB */
            if (CB->divers(DV_HP, 36, -1) <= 1) {
                ST(ST_VIE(36)) = VIE_DEAD;
                CB->divers_empile(DV_BLOOD_AT_BONE, 36, 0);
                CB->animation("SP36IMP51");
                CB->animation("P36CAD");
                CB->pos_perso(36, 0, 0, 0, 2, "P36CAD", 0x25, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->divers_empile(DV_BLOOD_AT_BONE, 36, 0);
                CB->animation("SP36IMP06");
                CB->animation("P36GAR01");
                CB->combat(36, 1);
            }
            CB->select_pile(0, 1);
        }
        /* 0x86AC */
        if (ST(L27_CAMERA_LOCK_ZONE) == 0 && CB->distance(1, 37) < 300) {
            ST(L27_CAMERA_LOCK_ZONE) = 1;
            CB->change_camera(2, CAM_LOCKED);
        }
        /* 0x86F8 */
        if (ST(L27_CAMERA_LOCK_ZONE) == 1 && CB->distance(1, 37) > 500) {
            ST(L27_CAMERA_LOCK_ZONE) = 0;
            CB->change_camera(99, CAM_AUTO);
        }
        /* 0x8741 */
        if (ST(V_ENGAGED_PERSONA) == 36 && ST(ST_VIE(36)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
            CB->ajoute_zone(32, 5653, -4501, 70, 170, 1);
            ST(L27_STORY_MARK_C48) = 111;
            CB->vire_col(0x2A, 0);
        }
    }
    /* 0x87AB */
    if (ST(L27_STORY_MARK_C48) == 111 && ST(L27_CAMERA_LOCK_ZONE) == 1 && CB->distance(1, 37) > 500) {
        ST(L27_CAMERA_LOCK_ZONE) = 0;
        CB->change_camera(99, CAM_AUTO);
    }
    /* 0x8809 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x8825 */
        if (g_dest_room == 20) {
            CB->evenement(2, 1000);
            CB->play_anim(3, 0);
            return;
        }
        /* 0x8854 */
        if (g_dest_room == 28) {
            CB->change_piece(28, 0);
        }
        /* 0x886E */
        if (g_dest_room == 29) {
            /* 0x8880 */
            if (ST(V_LORY_STAGE) == 0 || ST(V_LORY_STAGE) == 3) {
                CB->change_piece(29, 0);
            }
            /* 0x88B1 */
            if (ST(V_LORY_STAGE) == 1 || ST(V_LORY_STAGE) == 2) {
                CB->change_piece(29, 1);
            }
        }
    }
    /* 0x88E5 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->change_piece(20, 0);
    }
    /* 0x8913 */
    if (ST(V_EVENT_37) == 1) {
        ST(V_EVENT_37) = 0;
        CB->select_pile(2, 2);
        CB->play_anim(1, 0);
        ST(ST_LEVEL) = 27;
        CB->evenement(0, 1000);
        CB->select_pile(0, 1);
    }
    /* 0x8977 */
    if (ST(V_EVENT_38) == 1) {
        ST(V_EVENT_38) = 0;
        CB->select_pile(2, 2);
        CB->play_anim(2, 0);
        ST(ST_LEVEL) = 27;
        CB->evenement(0, 1000);
        CB->select_pile(0, 1);
    }
    /* 0x89DB */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        CB->change_camera(2, CAM_LOCKED);
        CB->dialogue("L27D01", P_KONKALITE4);
    }
    /* 0x8A1C */
    if (ST(ST_VIE(P_KONKALITE4)) == VIE_DEAD) {
        CB->select_pile(0, 2);
        CB->bloque(BLOQUE_ALL);
        CB->divers(DV_COMBAT_STANCE, 0, 0);
        CB->evenement(1, 500);
        ST(ST_VIE(P_KONKALITE4)) = 4;
    }
    /* 0x8A74 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        CB->divers_empile(DV_SCREEN, 5, 0);
        CB->cinematique("SC03_500");
        CB->change_piece(24, 0);
    }
    /* 0x8ABF */
    if (ST(ST_HERO) != 4 && ST(L27_PHEDORIA_STAGE) == 5) {
        /* 0x8AE3 */
        if (ST(V_EVENT_34) == 1) {
            ST(V_EVENT_34) = 0;
            CB->divers(DV_HP, ST(ST_HERO), 0);
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            CB->animation(lieu_anim_name(P_ARKHAN2, 0x1A));
            CB->modify_var(&ST(ST_HERO), 4);
        }
        /* 0x8B65 */
        if (ST(V_EVENT_35) == 1) {
            ST(V_EVENT_35) = 0;
            CB->animation(lieu_anim_name(P_ARKHAN2, 0x18));
            CB->animation(lieu_anim_name(P_ARKHAN2, 6));
            CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP26GAR17");
            CB->animation("P26GAR17");
            CB->evenement(0x22, 250);
            CB->animation("G26FRA01");
            CB->animation("P26GAR03");
        }
        /* 0x8C10 */
        if (ST(ST_COMBAT_STANCE) == 1 && ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 9, 2) && (CB->direction_perso(P_LORY) || CB->direction_perso(P_PHEDORIA)) && CB->divers(DV_PERSONA_PRESENT, P_PHEDORIA, 0) && CB->divers(DV_PERSONA_PRESENT, P_LORY, 0)) {
            CB->bloque(BLOQUE_ALL);
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            ST(L27_LORY_PATH_STEP) = 100;
            ST(L27_PHEDORIA_PATH_STEP) = 100;
            CB->select_pile(1, 2);
            CB->joindre(-P_LORY, 0, 0, 0, ST(ST_HERO));
            CB->select_pile(2, 2);
            CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP26GAR03");
            CB->dialogue("L20D38", P_PHEDORIA);
            CB->animation("SG26FRA01");
            CB->evenement(0x22, 750);
            CB->animation("P26GAR03");
            CB->select_pile(0, 1);
        }
        /* 0x8D82 */
        if (ST(L27_GATE_SCENE_DONE) == 0 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -450) {
            i32 t1 = CB->distance(1, 3);
            /* 0x8DCF */
            if (t1 < CB->distance(8, 3)) {
                ST(L27_GATE_SCENE_DONE) = 1;
                CB->bloque(BLOQUE_ALL);
                i32 t2 = CB->distance(1, 16);
                /* 0x8E13 */
                if (t2 < CB->distance(1, 17)) {
                    CB->joindre(-ST(ST_HERO), 7414, -4722, 0xF8AF, -1);
                } else {
                    CB->joindre(-ST(ST_HERO), 7411, -4279, 0x73AB, -1);
                }
                CB->animation("Sar");
                CB->joindre_perm(-ST(ST_HERO), 15);
            }
        }
        /* 0x8E9B */
        if (ST(L27_GATE_SCENE_DONE) == 0 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > -600) {
            i32 t3 = CB->distance(1, 3);
            /* 0x8EE8 */
            if (t3 > CB->distance(8, 3)) {
                ST(L27_GATE_SCENE_DONE) = 1;
                CB->bloque(BLOQUE_ALL);
                CB->animation("Sar");
                CB->joindre_perm(-ST(ST_HERO), 15);
            }
        }
        /* 0x8F36 */
        if (ST(L27_LORY_PATH_STEP) == 0 && CB->distance(8, 10) < 20) {
            ST(L27_LORY_PATH_STEP) = 1;
            CB->select_pile(1, 1);
            CB->joindre(-P_LORY, 6680, -4480, 0xC000, -1);
            CB->select_pile(0, 1);
        }
        /* 0x8FA3 */
        if (ST(L27_PHEDORIA_PATH_STEP) == 0 && CB->distance(9, 10) < 20) {
            ST(L27_PHEDORIA_PATH_STEP) = 1;
            CB->select_pile(2, 1);
            CB->joindre(-P_PHEDORIA, 6680, -4824, 0xC000, -1);
            CB->select_pile(0, 1);
        }
        /* 0x9010 */
        if (ST(L27_LORY_PATH_STEP) == 1 && CB->distance(8, 12) < 20) {
            ST(L27_LORY_PATH_STEP) = 2;
            CB->select_pile(1, 1);
            CB->joindre(-P_LORY, 7437, -4500, 0xBD8A, -1);
            CB->select_pile(0, 1);
        }
        /* 0x907D */
        if (ST(L27_PHEDORIA_PATH_STEP) == 1 && CB->distance(9, 11) < 20) {
            ST(L27_PHEDORIA_PATH_STEP) = 2;
            CB->select_pile(2, 1);
            CB->joindre(-P_PHEDORIA, 7423, -4625, 0x2D5E, -1);
            CB->select_pile(0, 1);
        }
        /* 0x90EA */
        if (ST(L27_LORY_PATH_STEP) == 2 && CB->distance(8, 13) < 20) {
            ST(L27_LORY_PATH_STEP) = 3;
            CB->select_pile(1, 1);
            /* 0x9131 */
            if (ST(ST_LOCKED_CAMERA) == 1) {
                CB->vire_col(0x28, 0);
                CB->animation("SP15OUV01");
                CB->play_anim(2, 0);
                CB->animation("P15ARR01");
                CB->joindre(-P_LORY, 7901, -4274, 0xD427, -1);
            } else {
                CB->retire_zone(3);
                CB->ajoute_zone(3, 7513, -4475, 80, 150, 0);
                CB->pos_perso(P_LORY, -1, -1, -1, 4, "", -1, 0);
                CB->pos_perso(P_PHEDORIA, -1, -1, -1, 4, "", -1, 0);
                ST(L27_PHEDORIA_STAGE) = 10;
            }
            CB->select_pile(0, 1);
        }
        /* 0x9214 */
        if (ST(L27_LORY_PATH_STEP) == 3 && CB->distance(8, 14) < 100) {
            ST(L27_LORY_PATH_STEP) = 4;
            CB->select_pile(1, 1);
            CB->pos_perso(P_LORY, -1, -1, -1, 4, "", -1, 0);
            CB->select_pile(0, 1);
        }
        /* 0x9284 */
        if (ST(L27_LORY_PATH_STEP) == 4) {
            ST(L27_LORY_PATH_STEP) = 5;
            CB->select_pile(2, 1);
            CB->joindre(-P_PHEDORIA, 7901, -4284, 0xD427, -1);
            CB->select_pile(0, 1);
        }
        /* 0x92D8 */
        if (ST(L27_LORY_PATH_STEP) == 5 && CB->distance(9, 14) < 100) {
            ST(L27_LORY_PATH_STEP) = 10;
            CB->select_pile(2, 1);
            CB->pos_perso(P_PHEDORIA, -1, -1, -1, 4, "", -1, 0);
            ST(L27_PHEDORIA_STAGE) = 10;
            CB->select_pile(0, 1);
            /* 0x9361 */
            if (CB->on_joindre(ST(ST_HERO))) {
                CB->stop_joindre(-ST(ST_HERO));
                i32 pz = CB->divers(DV_DESC_POS, ST(ST_HERO), 2);   /* arguments are evaluated last to first */
                i32 pa = CB->divers(DV_DESC_POS, ST(ST_HERO), 1);
                i32 px = CB->divers(DV_DESC_POS, ST(ST_HERO), 0);
                CB->pos_perso(ST(ST_HERO), px, pa, pz, 1, "ar", 1, 0);
            }
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0x93F9 */
    if (ST(ST_LOCKED_CAMERA) == 3) {
        /* 0x9414 */
        if (CB->distance(1, 18) > 300 && CB->distance(1, 19) > 500) {
            CB->change_camera(99, CAM_AUTO);
        }
        /* 0x945A */
        if (CB->distance(1, 18) > 600 && CB->distance(1, 19) > 240) {
            CB->change_camera(99, CAM_AUTO);
        }
    }
    /* 0x9497 */
    if (ST(ST_LOCKED_CAMERA) == 4 && CB->distance(1, 17) > 280) {
        CB->change_camera(99, CAM_AUTO);
    }
    /* 0x94D4 */
    if (ST(ST_LOCKED_CAMERA) == 6 && CB->distance(1, 16) > 200) {
        CB->change_camera(99, CAM_AUTO);
    }
    /* 0x9511 */
    if (ST(ST_LOCKED_CAMERA) == 5 && CB->distance(1, 15) > 300) {
        CB->change_camera(99, CAM_AUTO);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000D669).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001DB0C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu27_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000D995);
 * every other path returns state[0].
 * ---------------------------------------------------------------------- */
static i32 init(i32 reload_flag) {
    l27_reset_dll_globals();

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
        l27_trig00();   /* 0x1000D899: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000D77A - the hero is dead: play the death ending */
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
 * TICK (opcode -1): drain pending sound, run body.
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000D2D0).
 * Jump table @ 0x1000D969, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu27_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu27: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l27_trigger(arg);             break;  /* dword_1001D850[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
