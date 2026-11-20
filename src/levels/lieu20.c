/*
 * lieu20.c — Native hand-port of LIEU20.DLL (level-20 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu20_trig.c (reached through
 * l20_trigger, see lieu20_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x10012D00, the state
 * pointer global is dword_1002409C and the trigger table is
 * dword_10023FB0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_100133CC), lieu_pre_scene and sub_10001200
 * (float reset) are the shared lieu_common.c; the entry and INIT are
 * instruction-for-instruction identical to LIEU16's. tick_body
 * (sub_100088BD) is level-20 specific.
 */

#include <stdlib.h>

#include "lieu20_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body — sub_100088BD (thunk 0x100010B9), level-20 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue (0x1000B5FF).
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x88CF */
    if (ST(V_DEATH_EVENT_SENT) == 1) {
        ST(V_DEATH_EVENT_SENT) = 0;
        /* 0x88F5 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x891E */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x894A */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x8976 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x899A */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x89B7 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }

    /* 0x89E3 */
    if (ST(V_GUARD_FIGHT_DONE) == 1 && ST(V_EVENT_39) == 1) {
        ST(V_EVENT_39) = 0;
        /* 0x8A13 */
        if (ST(L20_THANANDAR_EXIT_STEP) == 0) {
            ST(L20_THANANDAR_EXIT_STEP) = 1;
            CB->animation("Sar_100");
            /* 0x8A3C */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation(lieu_anim_name(P_ARKHAN2, 0x18));
            }
            /* 0x8A68 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation(lieu_anim_name(P_ARKHAN3, 0x18));
            }
            CB->joindre(P_THANANDAR25, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L20D95", P_THANANDAR25, "SP25DIA50", "SP25ARR01");
            CB->divers_empile(DV_SCREEN, 5, 0);
            CB->pos_perso(P_THANANDAR25, -1, -1, -1, 4, "", -1, 0);
            CB->evenement(39, 2000);
            return;
        }
        /* 0x8B0D */
        if (ST(L20_THANANDAR_EXIT_STEP) == 1) {
            CB->divers_empile(DV_SCREEN, 3, 0);
            /* 0x8B31 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation(lieu_anim_name(P_ARKHAN2, 6));
            }
            /* 0x8B5D */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation(lieu_anim_name(P_ARKHAN3, 6));
            }
            CB->animation("ar");
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    if (ST(V_RETURN_PART) == 0 && ST(V_GUARD_FIGHT_DONE) == 0) {
        /* 0x8BC5 */
        if (ST(V_EVENT_2) == 1) {
            ST(V_EVENT_2) = 0;
            CB->select_pile(0, 2);
            CB->divers(DV_HP, ST(ST_HERO), 1);
            CB->divers(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            /* 0x8C23 */
            if (ST(ST_PLAYER_WEAPON) == 0) {
                CB->animation("SP01IMP51");
            } else {
                CB->animation("SP01IMP52");
            }
            CB->modify_var(&ST(V_DEATH_EVENT_SENT), 1);
        }

        /* 0x8C67 */
        if (ST(L20_LORY_CHAIN_STAGE) == 3 && ST(L20_LORY_GATE_STEP) == 0) {
            CB->select_pile(1, 1);
            ST(L20_LORY_GATE_STEP) = 10;
            CB->bloque(BLOQUE_ALL);
            CB->joindre(-P_GUARDIAN1, 0, 0, 0, ST(ST_HERO));
            CB->change_camera(2, CAM_LOCKED);
            lieu_action_register("L20D121", P_GUARDIAN1, "SP41DIA51", "SP41ARR02");
            CB->joindre_perm(P_GUARDIAN1, ST(ST_HERO));
            CB->animation("Sar_50");
            CB->change_camera(99, CAM_AUTO);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }

        /* 0x8D3B */
        if (ST(V_EVENT_17) < 3) {
            /* 0x8D4D */
            if (ST(L20_GUARDIAN1_FACING) == 1 && CB->distance(1, 2) > 220 && ST(V_ENGAGED_PERSONA) == 0) {
                ST(L20_GUARDIAN1_FACING) = 0;
            }
            /* 0x8D95 */
            if (ST(V_ENGAGED_PERSONA) == P_GUARDIAN1 && ST(L20_SCENE_STATE) == 0 && ST(ST_HERO) == P_ARKHAN1) {
                CB->select_pile(2, 1);
                ST(L20_SCENE_STATE) = 1;
                ST(ST_VIE(P_GUARDIAN1)) = 5;
                CB->divers(DV_RUN, P_ZED84, 1);
                CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
                CB->dialogue("L20D04", P_ZED84);
                CB->joindre(-P_ZED84, -1, -1, -1, ST(ST_HERO));
                /* 0x8E4A */
                if (ST(ST_LOCKED_CAMERA) != 0) {
                    CB->change_camera(0, CAM_LOCKED);
                }
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_ZED84);
                CB->select_pile(0, 1);
            }
            /* 0x8E93 */
            if (ST(L20_SCENE_STATE) == 1 && CB->distance(1, 5) <= 600) {
                CB->select_pile(1, 1);
                CB->animation("SP41GAR02");
                CB->stop_joindre(P_ZED84);
                CB->joindre(-P_ZED84, 0, 0, 0, ST(ST_HERO));
                ST(L20_SCENE_STATE) = 2;
                CB->change_camera(2, CAM_LOCKED);
                lieu_action_register("L20D05", P_ARKHAN1, "SG01DIA02", NULL);
                CB->dialogue("L20D06", P_ZED84);
                CB->animation("SP84ARR02_20");
                CB->animation("P84ACT01");
                CB->evenement(2, 1600);
                CB->animation("P84ARR02");
                CB->select_pile(0, 1);
            }
        }

        /* 0x8F86 */
        if (ST(V_EVENT_17) < 3 && ST(L20_GUARDIAN1_STRIKE_STAGE) != 2) {
            /* 0x8FB8 */
            if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 20) && ST(ST_SCRIPT_BUSY) == 0) {
                /* 0x8FD9 */
                if (ST(L20_ZONE20_TIMING) == 0) {
                    ST(L20_ZONE20_START_TIME) = ST(ST_GAME_TIME);
                    ST(L20_ZONE20_TIMING) = 1;
                }
                ST(L20_ZONE20_ELAPSED_MS) = (ST(ST_GAME_TIME) - ST(L20_ZONE20_START_TIME));
            }
            /* 0x9042 */
            if ((!CB->divers(DV_IN_ZONE, ST(ST_HERO), 20) || ST(ST_SCRIPT_BUSY) == 1) && ST(L20_ZONE20_TIMING) == 1) {
                ST(L20_ZONE20_ELAPSED_MS) = 0;
                ST(L20_ZONE20_TIMING) = 0;
            }
            /* 0x9090 */
            if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L20_SWORD_CHALLENGE_DONE) == 0 && (ST(L20_GUARDIAN1_FACING) == 0 || (ST(L20_GUARDIAN1_FACING) == 1 && ST(ST_SCRIPT_BUSY) == 0)) && ST(V_PLAYER_STRIKE) != 1) {
                /* 0x9108 */
                if (ST(L20_ZONE20_ELAPSED_MS) > 4000 && ST(ST_PLAYER_WEAPON) == 38 && ST(V_EVENT_17) != 1) {
                    CB->select_pile(2, 2);
                    CB->bloque(BLOQUE_ALL);
                    CB->divers(DV_COMBAT_STANCE, 0, 0);
                    CB->stop_joindre(-P_GUARDIAN1);
                    ST(L20_SWORD_CHALLENGE_DONE) = 1;
                    ST(V_EVENT_17) = 1;
                    ST(V_DLG_RANDOM) = CB->random(2);
                    lieu_action_register("L20D07A", P_GUARDIAN1, "SP41ARR02", "SP41ARR02");
                    lieu_action_register("L20D07B", P_GUARDIAN1, "SP41ARR02", "SP41ARR02");
                    ST(L20_ZONE20_START_TIME) = ST(ST_GAME_TIME);
                    CB->evenement(1, 5000);
                    CB->joindre(-ST(ST_HERO), 5536, -4406, 0x7D7A, -1);
                    CB->animation("Sar_50");
                    CB->select_pile(0, 1);
                }
                /* 0x9242 */
                if (ST(L20_ZONE20_ELAPSED_MS) > 7500 && ST(ST_PLAYER_WEAPON) != 38 && ST(V_WEAPON_WARNED) == 0) {
                    CB->select_pile(2, 2);
                    CB->bloque(BLOQUE_ALL);
                    CB->stop_joindre(-P_GUARDIAN1);
                    /* 0x929B */
                    if (ST(L20_WEAPON_WARNINGS) == 0) {
                        ST(L20_WEAPON_WARNINGS) = 1;
                        ST(V_WEAPON_WARNED) = 1;
                        lieu_action_register("L20D08A", P_GUARDIAN1, "SP41DIA56", "SP41DIA53");
                        /* 0x92E1 */
                        if (ST(ST_PLAYER_WEAPON) != 26) {
                            lieu_action_register("L20D08B", P_GUARDIAN1, "SP41DIA53", "SP41ARR02");
                        } else {
                            lieu_action_register("L20D08C", P_GUARDIAN1, "SP41DIA53", "SP41ARR02");
                        }
                        CB->animation("SP41ARR02_25");
                        CB->modify_var(&ST(L20_ZONE20_TIMING), 0);
                        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
                        CB->modify_var(&ST(L20_ZONE20_ELAPSED_MS), 0);
                        CB->joindre_perm(P_GUARDIAN1, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                    /* 0x939E */
                    if (ST(L20_WEAPON_WARNINGS) == 1) {
                        ST(L20_WEAPON_WARNINGS) = 2;
                        ST(V_WEAPON_WARNED) = 1;
                        /* 0x93CB */
                        if (ST(ST_PLAYER_WEAPON) != 26) {
                            lieu_action_register("L20D09A", P_GUARDIAN1, "SP41DIA53", "SP41ARR02");
                        } else {
                            lieu_action_register("L20D09B", P_GUARDIAN1, "SP41DIA53", "SP41ARR02");
                        }
                        CB->animation("SP41ARR02_25");
                        CB->modify_var(&ST(L20_ZONE20_TIMING), 0);
                        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
                        CB->modify_var(&ST(L20_ZONE20_ELAPSED_MS), 0);
                        CB->joindre_perm(P_GUARDIAN1, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                    /* 0x9488 */
                    if (ST(L20_WEAPON_WARNINGS) == 2) {
                        ST(V_WEAPON_WARNED) = 1;
                        ST(V_DLG_RANDOM) = CB->random(2);
                        /* 0x94C0 */
                        if (ST(V_DLG_RANDOM) == 0) {
                            lieu_action_register("L20D10A", P_GUARDIAN1, "SP41DIA54", "SP41DIA54");
                            lieu_action_register("L20D10B", P_GUARDIAN1, "SP41DIA56", "SP41ARR02");
                        }
                        /* 0x9504 */
                        if (ST(V_DLG_RANDOM) == 1) {
                            lieu_action_register("L20D11A", P_GUARDIAN1, "SP41DIA56", "SP41DIA56");
                            lieu_action_register("L20D11B", P_GUARDIAN1, "SP41DIA54", "SP41ARR02");
                        }
                        CB->animation("SP41ARR02_25");
                        CB->modify_var(&ST(L20_ZONE20_TIMING), 0);
                        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
                        CB->modify_var(&ST(L20_ZONE20_ELAPSED_MS), 0);
                        CB->joindre_perm(P_GUARDIAN1, ST(ST_HERO));
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                }
            }
            /* 0x95BF */
            if ((ST(V_EVENT_1) == 1 && ST(L20_GUARDIAN1_FACING) == 0 && ST(V_EVENT_17) == 1 && ST(V_ENGAGED_PERSONA) != P_GUARDIAN1) || (ST(V_EVENT_1) == 1 && ST(L20_GATE_OPEN_STEP) == 1)) {
                /* 0x9622 */
                if (ST(L20_GATE_OPEN_STEP) == 0) {
                    CB->select_pile(1, 1);
                    CB->bloque(BLOQUE_ALL);
                    CB->bruitage("L20AMB3", -1, -1);
                    CB->select_pile(2, 2);
                    CB->vire_col(0x17, 0);
                    /* 0x9685 */
                    if (CB->on_joindre(ST(ST_HERO))) {
                        CB->stop_joindre(-ST(ST_HERO));
                        CB->pos_perso(ST(ST_HERO), 5536, -4406, 0x7D7A, 1, "ar", 1, 0);
                    }
                    CB->animation("SP41GAR26");
                    lieu_action_register("L20D12", P_GUARDIAN1, "P41ARR02", "SP41GAR02");
                    CB->bruitage("L20AMB1", -1, -1);
                    CB->joindre(P_GUARDIAN1, 5556, -4630, 0xDA27, -1);
                    ST(L20_ZONE20_TIMING) = 2;
                    ST(V_EVENT_1) = 0;
                    ST(L20_GATE_OPEN_STEP) = 1;
                    CB->joindre(-ST(ST_HERO), 5590, -4520, 0xC000, -1);
                    CB->animation("ar");
                    /* 0x9784 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        CB->evenement(34, 400);
                    }
                    /* 0x97A6 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        CB->evenement(34, 700);
                    }
                    CB->animation("SG01OUV01");
                    CB->animation("ar");
                    CB->dialogue("L20D13", P_ARKHAN1);
                    CB->evenement(1, 1500);
                    return;
                }
                /* 0x9800 */
                if (ST(L20_GATE_OPEN_STEP) == 1) {
                    ST(V_EVENT_1) = 0;
                    ST(V_EVENT_17) = 2;
                    ST(ST_LEVEL) = 20;
                    CB->divers_empile(DV_SCREEN, 5, 0);
                    CB->change_piece(27, 1);
                    return;
                }
            }
            /* 0x9863 */
            if (ST(L20_LORY_CHAIN_STAGE) != 3 && ST(V_RETURN_PART) == 0 && ST(V_EVENT_17) != 3) {
                /* 0x9896 */
                if (ST(L20_LORY_GATE_STEP) == 0 && CB->distance(3, 16) < 250) {
                    CB->select_pile(1, 1);
                    ST(L20_LORY_GATE_STEP) = 1;
                    CB->bloque(BLOQUE_ALL);
                    /* 0x98EA */
                    if (ST(ST_LOCKED_CAMERA) != 2) {
                        CB->change_camera(2, CAM_LOCKED);
                        lieu_action_register("L20D14", P_GUARDIAN1, "SP41ATT72", "P41ARR02");
                    }
                    CB->select_pile(0, 1);
                }
                /* 0x992F */
                if (ST(L20_LORY_GATE_STEP) == 1 && !CB->on_joindre(P_LORY)) {
                    ST(L20_LORY_GATE_STEP) = 222;
                    CB->select_pile(2, 2);
                    CB->evenement(41, 200);
                    CB->select_pile(0, 1);
                    CB->animation("SP15OUV01");
                    CB->animation("P15ARR01");
                    CB->joindre(-P_LORY, 5745, -4582, 0xA212, -1);
                    CB->animation("SP15ARR01_50");
                }
                /* 0x99C7 */
                if (ST(L20_LORY_GATE_STEP) == 2 && CB->distance(3, 15) < 100 && !CB->on_joindre(P_LORY)) {
                    ST(L20_LORY_GATE_STEP) = 3;
                    CB->select_pile(2, 2);
                    CB->evenement(41, 250);
                    CB->select_pile(0, 1);
                    CB->animation("SP15OUV02");
                    CB->animation("P15ARR01");
                    CB->joindre(-P_LORY, 5855, -4686, -1, -1);
                    CB->retire_zone(3);
                    CB->retire_zone(15);
                    CB->retire_zone(16);
                    CB->vire_col(0x17, 1);
                    CB->evenement(0, 1000);
                }
            }
            /* 0x9AAB */
            if (ST(V_EVENT_41) == 1) {
                ST(V_EVENT_41) = 0;
                /* 0x9ACC */
                if (ST(L20_LORY_GATE_STEP) == 222) {
                    ST(L20_LORY_GATE_STEP) = 2;
                    CB->select_pile(2, 2);
                    CB->play_anim(0, 0);
                    CB->select_pile(0, 1);
                }
                /* 0x9B0E */
                if (ST(L20_LORY_GATE_STEP) == 3) {
                    CB->select_pile(2, 2);
                    CB->play_anim(1, 0);
                    CB->select_pile(0, 1);
                }
            }
            /* 0x9B4A */
            if (ST(V_EVENT_0) == 1) {
                ST(V_EVENT_0) = 0;
                ST(L20_LORY_CHAIN_STAGE) = 3;
                CB->change_camera(99, CAM_AUTO);
                CB->select_pile(2, 1);
                CB->pos_perso(P_LORY, -1, -1, -1, 4, "", -1, 0);
                CB->bloque(BLOQUE_NONE);
                CB->pos_perso(P_ZED84, 3730, 10000, 0x7EE8, 1, "P84ARR02", 5, 0);
                CB->link_objet(P_ZED84, "OB68", 0x1D, 1);
                CB->select_pile(0, 1);
            }
        }
    } else {
        /* 0x9C0C */
        if (ST(L20_ESCORT_STAGE) != 0 && ST(L20_ESCORT_STAGE) != 10) {
            /* 0x9C30 */
            if (ST(ST_LOCKED_CAMERA) == 2 && ST(L20_SCENE_STATE) == 1 && !CB->divers(DV_PERSONA_PRESENT, P_PHEDORIA, 0) && !CB->divers(DV_PERSONA_PRESENT, P_LORY, 0)) {
                CB->select_pile(2, 2);
                ST(L20_SCENE_STATE) = 2;
                CB->pause_anim(0, 0xF, 0);
                CB->select_pile(0, 1);
            }
            /* 0x9CC3 */
            if (ST(ST_LOCKED_CAMERA) != 2 && ST(L20_SCENE_STATE) == 2) {
                ST(L20_SCENE_STATE) = 1;
            }
            /* 0x9CF6 */
            if (ST(L20_ESCORT_STAGE) == 2) {
                /* 0x9D18 */
                if ((ST(L20_INIT_TIME) + 4000) < ST(ST_GAME_TIME) && ST(ST_LOCKED_CAMERA) == 0 && ST(L20_CAMERA_PAN_STATE) == 0 && CB->distance(1, 11) > 750) {
                    ST(L20_CAMERA_PAN_STATE) = 1;
                    CB->bloque(BLOQUE_ALL);
                    CB->change_camera(6, CAM_LOCKED);
                }
                /* 0x9D9A */
                if (CB->distance(4, 17) < 50 && (ST(L20_INIT_TIME) + 4000) < ST(ST_GAME_TIME)) {
                    ST(L20_ESCORT_STAGE) = 3;
                    CB->select_pile(2, 1);
                    CB->change_camera(2, CAM_LOCKED);
                    ST(L20_CAMERA_PAN_STATE) = 99;
                    CB->select_pile(1, 1);
                    CB->bloque(BLOQUE_ALL);
                    CB->vire_col(0x17, 0);
                    CB->retire_zone(12);
                    CB->stop_joindre(-P_PHEDORIA);
                    CB->joindre(P_PHEDORIA, 5614, -4501, 0xBA95, -1);
                    CB->animation("SP26OUV01");
                    CB->evenement(33, 200);
                    CB->animation("P26ARR01");
                    CB->select_pile(0, 1);
                }
            }
            /* 0x9E99 */
            if (ST(L20_ESCORT_STAGE) == 3) {
                /* 0x9EB2 */
                if (CB->distance(4, 17) > 50 && !CB->on_joindre(P_PHEDORIA) && ST(L20_PHEDORIA_SENT) == 0) {
                    ST(L20_PHEDORIA_SENT) = 1;
                    CB->select_pile(1, 1);
                    CB->joindre(-P_PHEDORIA, 5500, -4460, 0x7FD1, -1);
                    CB->animation("SP26ARR01");
                    CB->joindre_perm(P_PHEDORIA, 0xF);
                    CB->change_piece(-20, 1);
                    CB->select_pile(0, 1);
                }
                /* 0x9F63 */
                if (CB->distance(3, 17) < 75) {
                    ST(L20_ESCORT_STAGE) = 4;
                    ST(L20_SCENE_TIME) = ST(ST_GAME_TIME);
                    CB->select_pile(2, 1);
                    CB->stop_joindre(-P_LORY);
                    CB->joindre(-P_LORY, 5747, -4560, -1, -1);
                    CB->select_pile(0, 1);
                }
            }
            /* 0x9FDC */
            if (ST(L20_ESCORT_STAGE) == 4 && CB->distance(3, 18) < 20) {
                ST(L20_ESCORT_STAGE) = 5;
                ST(L20_SCENE_TIME) = ST(ST_GAME_TIME);
                CB->select_pile(2, 1);
                CB->bloque(BLOQUE_NONE);
                /* 0xA04E */
                if (CB->distance(1, 19) > 1200) {
                    CB->change_camera(99, CAM_AUTO);
                }
                CB->stop_joindre(-P_LORY);
                CB->joindre(-P_LORY, 5769, -4720, -1, -1);
                CB->animation("SP15ARR01_20");
                CB->modify_var(&ST(L20_ESCORT_WALK_SYNC), 2);
                CB->select_pile(0, 1);
            }
            /* 0xA0C3 */
            if (ST(L20_ESCORT_WALK_SYNC) == 2 && !CB->on_joindre(P_LORY)) {
                ST(L20_ESCORT_WALK_SYNC) = 0;
                CB->select_pile(1, 1);
                CB->pos_perso(P_LORY, -1, -1, -1, 4, "", -1, 0);
                CB->change_piece(-20, 1);
                CB->select_pile(2, 1);
                CB->joindre(-P_PHEDORIA, 5806, -4614, -1, -1);
                CB->modify_var(&ST(L20_ESCORT_WALK_SYNC), 4);
                CB->select_pile(0, 1);
            }
            /* 0xA183 */
            if (ST(L20_ESCORT_WALK_SYNC) == 4 && !CB->on_joindre(P_PHEDORIA)) {
                ST(L20_ESCORT_WALK_SYNC) = 0;
                CB->select_pile(2, 1);
                ST(L20_SCENE_STATE) = 1;
                CB->joindre(P_PHEDORIA, 5769, -4720, -1, -1);
                CB->animation("SP26ARR01_20");
                CB->change_camera(99, CAM_AUTO);
                CB->pos_perso(P_PHEDORIA, -1, -1, -1, 4, "", -1, 0);
                ST(L20_SCENE_TIME) = ST(ST_GAME_TIME);
                CB->retire_zone(17);
                CB->retire_zone(18);
                CB->select_pile(0, 1);
            }
            /* 0xA263 */
            if (ST(V_EVENT_35) == 1) {
                CB->select_pile(1, 1);
                ST(V_EVENT_35) = 0;
                CB->animation("SG01ATT07");
                CB->modify_var(&ST(V_DEATH_EVENT_SENT), 1);
                CB->select_pile(0, 1);
            }
            /* 0xA2C3 */
            if (ST(V_EVENT_36) == 1) {
                ST(V_EVENT_36) = 0;
                CB->animation(lieu_anim_name(P_ARKHAN2, 0x18));
                CB->animation(lieu_anim_name(P_ARKHAN2, 6));
                CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP26GAR17");
                CB->animation("P26GAR17");
                CB->evenement(35, 1000);
                CB->animation("G26FRA01");
                CB->animation("P26GAR03");
            }
            /* 0xA36E */
            if (ST(ST_COMBAT_STANCE) == 1 && ST(ST_PLAYER_WEAPON) != 0 && CB->divers(DV_CAN_FIGHT, 9, 2) && (CB->direction_perso(P_LORY) || CB->direction_perso(P_PHEDORIA))) {
                CB->bloque(BLOQUE_ALL);
                CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
                ST(L20_ESCORT_STAGE) = 100;
                CB->select_pile(1, 2);
                CB->joindre(-P_LORY, 0, 0, 0, ST(ST_HERO));
                CB->select_pile(2, 2);
                CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP26GAR03");
                CB->dialogue("L20D38", P_PHEDORIA);
                CB->animation("SG26FRA01");
                CB->evenement(35, 750);
                CB->animation("P26GAR03");
                CB->select_pile(0, 1);
            }
        }

        if (ST(L20_GUARD_SQUAD_STAGE) != 0) {
            /* 0xA4A3 */
            if (ST(L20_GUARD_SQUAD_STAGE) == 1 && ST(ST_VIE(P_GUARDIAN9)) != VIE_DEAD && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
                /* 0xA4F7 */
                if (ST(L20_SQUAD_SCENE_STEP) == 0 && ST(ST_LOCKED_CAMERA) == 2) {
                    ST(L20_SQUAD_SCENE_STEP) = 1;
                    CB->select_pile(1, 1);
                    CB->bloque(BLOQUE_ALL);
                    CB->joindre(-P_GUARDIAN9, 0, 0, 0, ST(ST_HERO));
                    CB->animation("SP49GAR26");
                    lieu_action_register("L20D96", P_GUARDIAN9, "P49GAR02", "SP49GAR02");
                    CB->joindre_perm(P_GUARDIAN9, ST(ST_HERO));
                    CB->evenement(37, 5000);
                    CB->bloque(BLOQUE_NONE);
                    CB->select_pile(2, 1);
                    CB->joindre(-P_GUARDIAN10, 0, 0, 0, ST(ST_HERO));
                    CB->animation("SP50GAR26");
                    CB->animation("P50GAR03");
                    CB->joindre_perm(P_GUARDIAN10, ST(ST_HERO));
                    CB->select_pile(0, 1);
                }
                /* 0xA617 */
                if (ST(ST_LOCKED_CAMERA) == 2 && ST(V_EVENT_37) == 1) {
                    ST(V_EVENT_37) = 0;
                    /* 0xA647 */
                    if (ST(L20_SQUAD_SCENE_STEP) == 1) {
                        ST(L20_SQUAD_SCENE_STEP) = 2;
                        CB->select_pile(2, 1);
                        CB->bloque(BLOQUE_ALL);
                        CB->dialogue("L20D97", P_GUARDIAN10);
                        CB->evenement(37, 10000);
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                    /* 0xA6B7 */
                    if (ST(L20_SQUAD_SCENE_STEP) == 2) {
                        ST(L20_SQUAD_SCENE_STEP) = 3;
                        CB->select_pile(1, 1);
                        CB->bloque(BLOQUE_ALL);
                        CB->dialogue("L20D112", P_GUARDIAN9);
                        CB->evenement(37, 1000);
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                    /* 0xA727 */
                    if (ST(L20_SQUAD_SCENE_STEP) == 3) {
                        ST(L20_SQUAD_SCENE_STEP) = 10;
                        ST(V_ENGAGED_PERSONA) = 4950;
                        CB->select_pile(2, 1);
                        CB->stop_joindre(-P_GUARDIAN9);
                        CB->combat(P_GUARDIAN10, 0);
                        CB->select_pile(1, 1);
                        CB->stop_joindre(-P_GUARDIAN10);
                        CB->combat(P_GUARDIAN9, 0);
                        CB->select_pile(0, 1);
                    }
                }
            }

            /* 0xA7AE */
            if (ST(L20_GUARD_SQUAD_STAGE) == 111 && ST(ST_VIE(P_GUARDIAN9)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
                /* 0xA7DE */
                if (ST(L20_SQUAD_SCENE_STEP) == 0 && ST(ST_LOCKED_CAMERA) == 2) {
                    ST(L20_SQUAD_SCENE_STEP) = 1;
                    CB->select_pile(1, 1);
                    CB->bloque(BLOQUE_ALL);
                    CB->joindre(-P_GUARDIAN9, 0, 0, 0, ST(ST_HERO));
                    CB->animation("SP49GAR26");
                    lieu_action_register("L20D96", P_GUARDIAN9, "P49GAR02", "SP49GAR02");
                    CB->joindre_perm(P_GUARDIAN9, ST(ST_HERO));
                    CB->evenement(37, 5000);
                    CB->bloque(BLOQUE_NONE);
                    CB->select_pile(0, 1);
                }
                /* 0xA8A5 */
                if (ST(ST_LOCKED_CAMERA) == 2 && ST(V_EVENT_37) == 1) {
                    ST(V_EVENT_37) = 0;
                    /* 0xA8D5 */
                    if (ST(L20_SQUAD_SCENE_STEP) == 1) {
                        ST(L20_SQUAD_SCENE_STEP) = 2;
                        CB->select_pile(1, 1);
                        CB->bloque(BLOQUE_ALL);
                        CB->dialogue("L20D112", P_GUARDIAN9);
                        CB->evenement(37, 1000);
                        CB->bloque(BLOQUE_NONE);
                        CB->select_pile(0, 1);
                        return;
                    }
                    /* 0xA945 */
                    if (ST(L20_SQUAD_SCENE_STEP) == 2) {
                        ST(L20_SQUAD_SCENE_STEP) = 10;
                        ST(V_ENGAGED_PERSONA) = P_GUARDIAN9;
                        CB->select_pile(2, 1);
                        CB->stop_joindre(-P_GUARDIAN9);
                        CB->combat(P_GUARDIAN9, 0);
                        CB->select_pile(0, 1);
                    }
                }
            }

            /* 0xA9A4 */
            if (ST(L20_GUARD_SQUAD_STAGE) == 10 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN9)) != VIE_DEAD && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD) {
                /* 0xA9F2 */
                if (CB->distance(1, 6) > 500 && CB->distance(1, 7) > 500 && ST(L20_SQUAD_FAR) == 0) {
                    ST(L20_SQUAD_FAR) = 1;
                    CB->select_pile(1, 1);
                    CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN9, 0);
                    CB->stop_joindre(-P_GUARDIAN9);
                    CB->joindre(P_GUARDIAN9, 0, 0, 0, P_GUARDIAN10);
                    CB->animation("SP49DIA54");
                    CB->select_pile(2, 1);
                    CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN10, 0);
                    CB->stop_joindre(-P_GUARDIAN10);
                    CB->joindre(P_GUARDIAN10, 0, 0, 0, P_GUARDIAN9);
                    CB->animation("SP50DIA54");
                    CB->select_pile(0, 1);
                }
                /* 0xAAE6 */
                if (CB->distance(1, 6) < 350 && CB->distance(1, 7) < 350 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 6) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 7) && ST(L20_SQUAD_FAR) == 1) {
                    ST(L20_SQUAD_FAR) = 0;
                    CB->select_pile(1, 1);
                    CB->animation("SP49ARR02");
                    CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN9, 1);
                    CB->select_pile(2, 1);
                    CB->animation("SP50ARR03");
                    CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN10, 1);
                    CB->select_pile(0, 1);
                }
            }

            /* 0xABD5 */
            if (ST(L20_GUARD_SQUAD_STAGE) == 112 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_GUARDIAN9)) != VIE_DEAD) {
                /* 0xAC11 */
                if (CB->distance(1, 26) > 500 && ST(L20_SQUAD_FAR) == 0) {
                    ST(L20_SQUAD_FAR) = 1;
                    CB->select_pile(1, 1);
                    CB->stop_joindre(-P_GUARDIAN9);
                    CB->joindre(P_GUARDIAN9, 5484, -4521, 0x3DEB, -1);
                    CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN9, 0);
                    CB->select_pile(0, 1);
                }
                /* 0xAC9D */
                if (CB->distance(1, 26) < 350 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 26) && ST(L20_SQUAD_FAR) == 1) {
                    ST(L20_SQUAD_FAR) = 0;
                    CB->select_pile(1, 1);
                    CB->divers(DV_LOOK_AT_PLAYER, P_GUARDIAN9, 1);
                    CB->select_pile(0, 1);
                }
            }

            /* 0xAD19 */
            if (ST(L20_GUARD_SQUAD_STAGE) == 2 && ST(ST_VIE(P_GUARDIAN11)) != VIE_DEAD && ST(ST_VIE(52)) != VIE_DEAD) {
                /* 0xAD4C */
                if (ST(V_ENGAGED_PERSONA) == 0 && ST(L20_SQUAD_SCENE_STEP) == 0 && (ST(ST_LOCKED_CAMERA) == 2 || ST(ST_LOCKED_CAMERA) == 1)) {
                    ST(L20_SQUAD_SCENE_STEP) = 1;
                    CB->select_pile(1, 1);
                    CB->bloque(BLOQUE_ALL);
                    CB->joindre(-P_GUARDIAN11, 0, 0, 0, ST(ST_HERO));
                    CB->animation("SP51GAR26");
                    lieu_action_register("L20D120", P_GUARDIAN11, "P51GAR03", "SP51GAR03");
                    CB->evenement(37, 200);
                    CB->bloque(BLOQUE_NONE);
                    CB->select_pile(2, 1);
                    CB->joindre(-52, 0, 0, 0, ST(ST_HERO));
                    CB->animation("P52GAR05");
                    CB->select_pile(0, 1);
                }
                /* 0xAE56 */
                if (ST(V_EVENT_37) == 1) {
                    ST(V_EVENT_37) = 0;
                    CB->select_pile(1, 1);
                    CB->combat(P_GUARDIAN11, 0);
                    CB->select_pile(2, 1);
                    CB->combat(52, 0);
                    CB->select_pile(0, 1);
                }
            }

            /* 0xAEBE */
            if ((ST(L20_GUARD_SQUAD_STAGE) == 1 || ST(L20_GUARD_SQUAD_STAGE) == 10) && ST(ST_VIE(P_GUARDIAN9)) == VIE_DEAD && ST(ST_VIE(P_GUARDIAN10)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
                ST(L20_GUARD_SQUAD_STAGE) = 11;
                ST(V_ENGAGED_PERSONA) = 0;
            }

            /* 0xAF30 */
            if ((ST(L20_GUARD_SQUAD_STAGE) == 111 || ST(L20_GUARD_SQUAD_STAGE) == 112) && ST(ST_VIE(P_GUARDIAN9)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
                ST(L20_GUARD_SQUAD_STAGE) = 115;
                ST(V_ENGAGED_PERSONA) = 0;
            }

            /* 0xAF90 */
            if (ST(L20_GUARD_SQUAD_STAGE) == 2 && ST(ST_VIE(P_GUARDIAN11)) == VIE_DEAD && ST(ST_VIE(52)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
                ST(L20_GUARD_SQUAD_STAGE) = 21;
                ST(V_ENGAGED_PERSONA) = 0;
            }

            /* 0xAFF8 */
            if (CB->divers(DV_WAS_HIT, P_GUARDIAN9, 0) && ST(ST_VIE(P_GUARDIAN9)) != VIE_DEAD) {
                CB->select_pile(1, 2);
                CB->divers_empile(DV_WAS_HIT, P_GUARDIAN9, 1);
                CB->combat(P_GUARDIAN9, -1);
                /* 0xB062 */
                if ((CB->divers(DV_HP, P_GUARDIAN9, -1) - ST(V_HIT_DAMAGE)) > 0) {
                    CB->divers(DV_HP, P_GUARDIAN9, (CB->divers(DV_HP, P_GUARDIAN9, -1) - ST(V_HIT_DAMAGE)));
                } else {
                    CB->divers(DV_HP, P_GUARDIAN9, 1);
                }
                /* 0xB0C3 */
                if (CB->divers(DV_HP, P_GUARDIAN9, -1) <= 1) {
                    ST(ST_VIE(P_GUARDIAN9)) = VIE_DEAD;
                    CB->stop_joindre(-P_GUARDIAN9);
                    CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN9, 0);
                    CB->animation("SP49IMP52");
                    CB->animation("P49CAD");
                    CB->pos_perso(P_GUARDIAN9, 0, 0, 0, 2, "P49CAD", 6, 0);
                    CB->divers_empile(DV_SCREEN, 2, 0);
                } else {
                    CB->stop_joindre(-P_GUARDIAN9);
                    CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARDIAN9, 0);
                    CB->animation("SP49IMP12");
                    CB->animation("P49IMP47");
                    CB->animation("P49GAR02");
                    /* 0xB190 */
                    if (CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN10, 0)) {
                        /* 0xB1A2 */
                        if (ST(V_ENGAGED_PERSONA) != 4950) {
                            CB->combat(P_GUARDIAN9, 0);
                            CB->select_pile(2, 2);
                            CB->combat(P_GUARDIAN10, 0);
                            CB->select_pile(0, 1);
                            ST(V_ENGAGED_PERSONA) = 4950;
                        } else {
                            CB->combat(P_GUARDIAN9, 0);
                        }
                    } else {
                        CB->combat(P_GUARDIAN9, 0);
                    }
                }
                CB->select_pile(0, 1);
            }

            /* 0xB230 */
            if (CB->divers(DV_WAS_HIT, P_GUARDIAN10, 0) && ST(ST_VIE(P_GUARDIAN10)) != VIE_DEAD && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN10, 0)) {
                CB->select_pile(2, 2);
                CB->divers_empile(DV_WAS_HIT, P_GUARDIAN10, 1);
                CB->combat(P_GUARDIAN10, -1);
                /* 0xB2B4 */
                if ((CB->divers(DV_HP, P_GUARDIAN10, -1) - ST(V_HIT_DAMAGE)) > 0) {
                    CB->divers(DV_HP, P_GUARDIAN10, (CB->divers(DV_HP, P_GUARDIAN10, -1) - ST(V_HIT_DAMAGE)));
                } else {
                    CB->divers(DV_HP, P_GUARDIAN10, 1);
                }
                /* 0xB315 */
                if (CB->divers(DV_HP, P_GUARDIAN10, -1) <= 1) {
                    ST(ST_VIE(P_GUARDIAN10)) = VIE_DEAD;
                    CB->stop_joindre(-P_GUARDIAN10);
                    CB->animation("SP50IMP53");
                    CB->animation("P50CAD");
                    CB->pos_perso(P_GUARDIAN10, 0, 0, 0, 2, "P50CAD", 7, 0);
                    CB->divers_empile(DV_SCREEN, 2, 0);
                } else {
                    CB->stop_joindre(-P_GUARDIAN10);
                    CB->animation("SP50IMP13");
                    CB->animation("P50IMP48");
                    CB->animation("P50GAR03");
                    /* 0xB3B6 */
                    if (ST(V_ENGAGED_PERSONA) != 4950) {
                        CB->combat(P_GUARDIAN10, 0);
                        CB->select_pile(1, 2);
                        CB->combat(P_GUARDIAN9, 0);
                        CB->select_pile(0, 1);
                        ST(V_ENGAGED_PERSONA) = 4950;
                    } else {
                        CB->combat(P_GUARDIAN10, 0);
                    }
                }
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0xB42A */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0xB44B */
        if (ST(V_RETURN_PART) != 0) {
            CB->evenement(38, 500);
        }
        CB->select_pile(1, 1);
        CB->play_anim(0, 0);
        CB->select_pile(0, 1);
    }

    /* 0xB494 */
    if (ST(V_EVENT_38) == 1) {
        ST(V_EVENT_38) = 0;
        /* 0xB4B5 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->change_piece(27, 1);
        } else {
            CB->change_piece(27, 0);
        }
    }

    /* 0xB4E9 */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        /* 0xB50A */
        if (ST(L20_SCENE_STATE) == 0) {
            ST(L20_SCENE_STATE) = 1;
            CB->select_pile(0, 1);
            /* 0xB538 */
            if (ST(L20_ESCORT_STAGE) != 0) {
                CB->bruitage("porte1", 5690, -4513);
            }
            CB->select_pile(2, 1);
            /* 0xB56F */
            if (ST(ST_LOCKED_CAMERA) == 2) {
                CB->play_anim(0, 0);
            }
            CB->ajoute_zone(19, 5690, -4513, 80, 170, 1);
            CB->select_pile(0, 1);
        } else if (ST(L20_SCENE_STATE) == 1) {
            /* 0xB5BB */
            ST(L20_SCENE_STATE) = 0;
            CB->select_pile(1, 1);
            CB->play_anim(1, 0);
            CB->select_pile(0, 1);
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x10013099).
 * Callback ingestion (0x10012D1A..0x10013094), the trigger-table fill and
 * the unused dword_10024E1C = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu20_trig.c.
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
        l20_trig00();   /* 0x100132C9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x100131AE — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1001331C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x10012D00).
 * Jump table @ 0x10013399, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu20_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu20: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l20_trigger(arg);             break;  /* dword_10024CB0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
