/*
 * lieu32.c - Native hand-port of LIEU32.DLL (level-32 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu32_trig.c (reached through
 * l32_trigger, see lieu32_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU30; the
 * entry is at 0x10010B10, the state pointer global is dword_100219FC and the
 * trigger table is dword_10021910[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_100111DC), lieu_pre_scene (sub_1001138C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are the same as LIEU30. tick_body (sub_10008749) is level-32
 * specific. The level code touches no DLL .data global, so INIT has
 * nothing to reset.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu32_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10008749 (thunk 0x100010B9), level-32 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x8769 */
    if (ST(L32_HIT_COUNT_SEEN) < ST(L32_PLAYER_HIT_COUNT) && ST(ST_COMBAT_STANCE) == 0) {
        CB->select_pile(0, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        ST(ST_SCRIPT_BUSY) = 0;
        CB->bloque(BLOQUE_NONE);
        ST(L32_HIT_COUNT_SEEN) = ST(L32_PLAYER_HIT_COUNT);
    }
    /* 0x87D1 */
    if (ST(L32_CAMERA1_PAUSED) == 0 && ST(ST_LOCKED_CAMERA) == 1) {
        ST(L32_CAMERA1_PAUSED) = 1;
        /* 0x8804 */
        if (ST(L32_DOORS_OPENED) == 0) {
            CB->pause_anim(1, 0, 0);
        }
        /* 0x8825 */
        if (ST(L32_DOORS_OPENED) == 1) {
            CB->pause_anim(1, 20, 0);
        }
    }
    /* 0x8849 */
    if (ST(L32_CAMERA1_PAUSED) == 1 && ST(ST_LOCKED_CAMERA) != 1) {
        ST(L32_CAMERA1_PAUSED) = 0;
    }
    /* 0x887C */
    if (ST(L32_CAMERA_ANIM_PAUSED) == 0 && ST(ST_LOCKED_CAMERA) == 0) {
        /* 0x88A0 */
        if (ST(L32_ROOM_ANIM_KIND) == 1) {
            ST(L32_CAMERA_ANIM_PAUSED) = 1;
            CB->pause_anim(0, 0, 0);
        }
        /* 0x88CD */
        if (ST(L32_ROOM_ANIM_KIND) == 2) {
            ST(L32_CAMERA_ANIM_PAUSED) = 1;
            CB->pause_anim(0, 15, 0);
        }
    }
    /* 0x88FD */
    if (ST(L32_CAMERA_ANIM_PAUSED) == 1 && ST(ST_LOCKED_CAMERA) != 0) {
        ST(L32_CAMERA_ANIM_PAUSED) = 0;
    }
    /* 0x8930 */
    if (ST(V_EVENT15_STEP) == 0 && CB->distance(12, 14) < 100) {
        ST(V_EVENT15_STEP) = 1;
        ST(L32_KONKALITE3_STATE) = 3;
        CB->pos_perso(P_MALROT6, -1, -1, -1, 4, "", -1, 0);
        CB->animation("Sar_50");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x89C1 */
    if (ST(V_EVENT_36) == 1) {
        ST(V_EVENT_36) = 0;
        CB->divers_empile(DV_SCREEN, 1, 0);
        /* 0x89F4 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->cinematique("SC04_500");
        }
        /* 0x8A11 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->cinematique("SC05_500");
        }
        /* 0x8A36 */
        if (CB->divers(DV_PERSONA_PRESENT, P_KONKALITE7, 0)) {
            CB->pos_perso(P_KONKALITE7, 0, 0, 0, 4, "", -1, 0);
        }
        CB->pos_perso(29, 6483, 6435, 0x2077, 1, "P29GAR01", 7, 0);
        CB->pos_perso(ST(ST_HERO), 5996, 6518, 0xC411, 1, "ar", 1, 0);
        CB->divers_empile(DV_SCREEN, 3, 0);
        CB->animation("Sar_50");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->combat(29, 0);
        return;
    }
    /* 0x8AFC */
    if (ST(ST_HERO) != 4) {
        ST(L32_LAST_LIVING_HERO) = ST(ST_HERO);
    }
    /* 0x8B25 */
    if (ST(ST_HERO) == 4 && ST(L32_DEATH_HANDLED) == 0) {
        ST(L32_DEATH_HANDLED) = 1;
        CB->evenement(1, 2000);
    }
    /* 0x8B68 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x8B89 */
        if (ST(V_CHASE_STATE) == 0 && ST(V_SORDOS_DUEL) < 2 && (ST(L32_STORY_PART) == 2 || ST(L32_STORY_PART) == 10) &&
            (ST(ST_VIE(P_GORK)) < 2 || ST(ST_VIE(P_KONKALITE11)) < 2) &&
            !CB->divers(DV_PERSONA_PRESENT, 29, 0)) {
            ST(V_CHASE_STATE) = 1;
            ST(ST_VIE(69)) = VIE_DEAD;
            /* 0x8C75 */
            if (ST(V_CHASE_ROUTE) != 1 && ST(V_CHASE_ROUTE) != 11 && ST(L32_STORY_PART) == 10) {
                ST(V_CHASE_ROUTE) = 1;
            }
            CB->bloque(BLOQUE_ALL);
            CB->divers_empile(DV_SCREEN, 5, 0);
            CB->divers_empile(DV_SCREEN, 1, 0);
            /* 0x8CE9 */
            if (ST(ST_VIE(P_KONKALITE7)) != VIE_DEAD) {
                CB->pos_perso(P_KONKALITE7, -1, -1, -1, 4, "", -1, 0);
            }
            CB->pos_perso(P_GORK, -1, -1, -1, 4, "", -1, 0);
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            CB->evenement(34, 100);
        } else {
            /* 0x8D62 */
            if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
                CB->divers(DV_HP, P_ARKHAN0, 0);
            }
            /* 0x8D8B */
            if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
                CB->divers(DV_HP, P_ARKHAN1, 0);
            }
            /* 0x8DB7 */
            if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
                CB->divers(DV_HP, P_ARKHAN2, 0);
            }
            /* 0x8DE3 */
            if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
                CB->divers(DV_HP, P_ARKHAN3, 0);
            }
            /* 0x8E07 */
            if (ST(V_GUARD_FIGHT_DONE) == 0) {
                CB->cinematique("SC22_500");
            }
            /* 0x8E24 */
            if (ST(V_GUARD_FIGHT_DONE) != 0) {
                CB->cinematique("SC23_500");
            }
            CB->divers_empile(DV_END_GAME, 0, 0);
        }
    }
    /* 0x8E50 */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        CB->ajoute_zone(11, 5023, 6975, 314, 160, 0);
        /* 0x8E92 */
        if (ST(L32_GORK_DEAD) == 0) {
            CB->ajoute_zone(7, 5226, 7139, 20, 20, 0);
            CB->ajoute_zone(8, 5646, 7309, 20, 20, 0);
            CB->ajoute_zone(9, 5352, 7582, 20, 20, 0);
        } else {
            CB->ajoute_zone(7, 4824, 7217, 20, 20, 0);
            CB->ajoute_zone(8, 5738, 7038, 20, 20, 0);
            CB->ajoute_zone(9, 5494, 7592, 20, 20, 0);
        }
        ST(ST_HERO) = ST(L32_LAST_LIVING_HERO);
        CB->divers(DV_HP, ST(ST_HERO), 256);
        CB->divers(DV_INVENTORY, 1, 0);
        /* 0x8F8C */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
        }
        CB->pos_perso(ST(ST_HERO), 5313, 7762, 0x89A5, 1, "P02ARR01", 1, 0);
        CB->pos_perso(P_SORDOS, 5177, 7173, 0xB2FD, 1, "P23ARR03", 2, 0);
        /* 0x8FFD */
        if (ST(L32_GORK_DEAD) == 0) {
            CB->pos_perso(P_GORK, 5809, 6920, 0x2384, 1, "P31ARR01", 4, 1);
        } else {
            CB->pos_perso(P_KONKALITE11, 5809, 6920, 0x2384, 1, "P81ARR01", 0x10, 1);
        }
        CB->divers_empile(DV_SCREEN, 3, 0);
        /* 0x9070 */
        if (ST(V_CHASE_STATE) == 1) {
            ST(V_CHASE_STATE) = 2;
            ST(L32_GORK_KONKALITE7_STATE) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->bloque(BLOQUE_ALL);
            /* 0x90B7 */
            if (ST(L32_GORK_DEAD) == 0) {
                ST(L32_SORDOS_SCENE_STEP) = 20;
                CB->joindre(-P_GORK, 5292, 7089, 0x2A66, -1);
                CB->joindre(-P_SORDOS, 0, 0, 0, P_GORK);
                lieu_action_register("L32D08", P_SORDOS, "SP23DIA54", "SP23ARR03");
                CB->evenement(0, 2000);
            } else {
                CB->joindre(-P_KONKALITE11, 5292, 7089, 0x2A66, -1);
                CB->joindre(-P_SORDOS, 0, 0, 0, P_KONKALITE11);
                lieu_action_register("L32D70", P_SORDOS, "SP23DIA53", "SP23ARR03");
                CB->evenement(0, 2000);
            }
        }
    }
    /* 0x9184 */
    if (ST(V_CHASE_STATE) != 0 && (ST(L32_GORK_KONKALITE7_STATE) == 0 || ST(L32_GORK_KONKALITE7_STATE) >= 10) && ST(L32_ROOM_PLOT_STAGE) != 3 && ST(L32_ROOM_PLOT_STAGE) != 4) {
        /* 0x91DE */
        if (ST(L32_GORK_DEAD) == 1) {
            /* 0x91F0 */
            if (ST(V_EVENT_0) == 1) {
                ST(V_EVENT_0) = 0;
                /* 0x920E */
                if (ST(L32_SORDOS_SCENE_STEP) == 0) {
                    ST(L32_ROOM_ANIM_KIND) = 1;
                    ST(L32_SORDOS_SCENE_STEP) = 2;
                    CB->select_pile(1, 1);
                    CB->joindre(-P_KONKALITE11, 0, 0, 0, P_SORDOS);
                    lieu_action_register("L32D71", P_KONKALITE11, "P81DIA56", "SP81ARR01_10");
                    /* 0x9277 */
                    if (ST(ST_VIE(69)) != 1) {
                        lieu_action_register("L32D09A", P_SORDOS, "SP23DIA56", "SP23DIA51");
                    }
                    lieu_action_register("L32D09B", P_SORDOS, "SP23DIA51", "SP23ARR03");
                    lieu_action_register("L32D72", P_KONKALITE11, "SP81DIA56", "SP81ARR01_10");
                    CB->joindre_perm(-P_KONKALITE11, P_SORDOS);
                    CB->evenement(0, 100);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9300 */
                if (ST(L32_SORDOS_SCENE_STEP) == 2) {
                    ST(L32_SORDOS_SCENE_STEP) = 3;
                    CB->select_pile(2, 1);
                    CB->evenement(0, 2500);
                    CB->divers(DV_LOOK_AT_PLAYER, P_SORDOS, 1);
                    CB->joindre(-P_SORDOS, 5192, 7533, 0xE136, -1);
                    /* 0x9369 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02LEV02");
                    }
                    /* 0x9386 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03LEV02");
                    }
                    CB->animation("ar_50");
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x93BD */
                if (ST(L32_SORDOS_SCENE_STEP) == 3) {
                    ST(L32_SORDOS_SCENE_STEP) = 51;
                    CB->joindre(-ST(ST_HERO), 0, 0, 0, P_SORDOS);
                    CB->joindre(P_KONKALITE11, 5226, 7139, 0x1518, -1);
                    CB->select_pile(1, 1);
                    CB->joindre(-P_SORDOS, 0, 0, 0, ST(ST_HERO));
                    lieu_action_register("L32D05A", P_SORDOS, "SP23DIA59", "SP23DIA56");
                    lieu_action_register("L32D05B", P_SORDOS, "SP23DIA56", "SP23DIA53");
                    lieu_action_register("L32D05C", P_SORDOS, "SP23DIA53", "SP23ARR03");
                    CB->vire_col(14, 1);
                    CB->animation("SP23ARR03_50");
                    CB->joindre(-P_SORDOS, 4837, 7219, -1, -1);
                    CB->evenement(0, 150);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x94DC */
                if (ST(L32_SORDOS_SCENE_STEP) == 51) {
                    ST(L32_SORDOS_SCENE_STEP) = 100;
                    CB->select_pile(1, 1);
                    /* 0x950A */
                    if (ST(ST_VIE(P_GORK)) != VIE_DEAD) {
                        CB->stop_joindre(-P_KONKALITE11);
                        CB->joindre(-P_KONKALITE11, 5192, 7533, 0xE136, -1);
                        lieu_action_register("L32D73A", P_KONKALITE11, "SP81DIA50", "SP81ARR01_10");
                        CB->pos_perso(P_SORDOS, -1, -1, -1, 4, "", -1, 0);
                        lieu_action_register("L32D73B", P_KONKALITE11, "SP81DIA50", "SP81ARR01_10");
                        lieu_action_register("L32D73C", P_KONKALITE11, "SP81DIA50", "SP81ARR01_10");
                        CB->change_camera(0, CAM_LOCKED);
                        CB->bloque(BLOQUE_NONE);
                    } else {
                        CB->stop_joindre(-P_KONKALITE11);
                        CB->joindre(-P_KONKALITE11, 5192, 7533, 0xE136, -1);
                        lieu_action_register("L32D73B", P_KONKALITE11, "SP81DIA50", "SP81ARR01_10");
                        CB->pos_perso(P_SORDOS, -1, -1, -1, 4, "", -1, 0);
                        lieu_action_register("L32D73C", P_KONKALITE11, "SP81DIA50", "SP81ARR01_10");
                        CB->change_camera(0, CAM_LOCKED);
                        CB->bloque(BLOQUE_NONE);
                    }
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9663 */
                if (ST(L32_SORDOS_SCENE_STEP) == 100 && !CB->on_joindre(P_SORDOS)) {
                    ST(L32_SORDOS_SCENE_STEP) = 250;
                    CB->select_pile(2, 1);
                    CB->pos_perso(P_SORDOS, -1, -1, -1, 4, "", -1, 0);
                    CB->select_pile(0, 1);
                    return;
                }
            }
        } else if (ST(V_EVENT_0) == 1) {
            ST(V_EVENT_0) = 0;
            /* 0x96F8 */
            if (ST(L32_SORDOS_SCENE_STEP) == 20) {
                ST(L32_ROOM_ANIM_KIND) = 1;
                ST(L32_SORDOS_SCENE_STEP) = 21;
                CB->select_pile(2, 1);
                CB->joindre(-P_GORK, 0, 0, 0, P_SORDOS);
                CB->select_pile(1, 1);
                CB->joindre(-P_SORDOS, 0, 0, 0, P_GORK);
                /* 0x976B */
                if (ST(ST_VIE(69)) != VIE_DEAD) {
                    lieu_action_register("L32D09A", P_SORDOS, "SP23DIA56", "SP23DIA51");
                }
                lieu_action_register("L32D09B", P_SORDOS, "SP23DIA51", "SP23ARR03");
                lieu_action_register("L32D10", P_GORK, "SP31GAR01", "SP31ARR01_10");
                CB->divers_empile(DV_LOOK_AT_PLAYER, P_GORK, 1);
                CB->evenement(0, 2500);
                CB->joindre(-P_SORDOS, 5192, 7533, 0xE136, -1);
                /* 0x9803 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02LEV02");
                }
                /* 0x9820 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03LEV02");
                }
                CB->animation("ar_50");
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9857 */
            if (ST(L32_SORDOS_SCENE_STEP) == 21) {
                ST(L32_SORDOS_SCENE_STEP) = 22;
                CB->select_pile(1, 1);
                CB->joindre(P_GORK, 5226, 7139, 0x1518, -1);
                CB->animation("SP23ARR03");
                CB->joindre(-P_SORDOS, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L32D05A", P_SORDOS, "SP23DIA59", "SP23DIA56");
                lieu_action_register("L32D05B", P_SORDOS, "SP23DIA56", "SP23DIA53");
                lieu_action_register("L32D05C", P_SORDOS, "SP23DIA53", "SP23ARR03");
                CB->vire_col(14, 1);
                CB->animation("SP23ARR03_50");
                CB->joindre(-P_SORDOS, 4837, 7219, -1, -1);
                CB->evenement(0, 150);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9964 */
            if (ST(L32_SORDOS_SCENE_STEP) == 22) {
                ST(L32_SORDOS_SCENE_STEP) = 23;
                CB->select_pile(1, 1);
                CB->joindre(-P_GORK, 5349, 7573, 0x3C0, -1);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x99BD */
        if (ST(L32_SORDOS_SCENE_STEP) == 23 && CB->distance(2, 4) > 400) {
            ST(L32_SORDOS_SCENE_STEP) = 24;
            CB->select_pile(2, 1);
            CB->animation("SP31GAR01");
            CB->dialogue("L32D12A", P_GORK);
            CB->dialogue("L32D12B", P_GORK);
            CB->dialogue("L32D12C", P_GORK);
            CB->change_camera(0, CAM_LOCKED);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x9A60 */
        if (ST(L32_SORDOS_SCENE_STEP) == 24 && !CB->on_joindre(P_SORDOS)) {
            ST(L32_SORDOS_SCENE_STEP) = 25;
            CB->select_pile(1, 1);
            CB->evenement(2, -1);
            CB->pos_perso(P_SORDOS, -1, -1, -1, 4, "", -1, 0);
            CB->evenement(2, 4000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9AF2 */
        if (ST(V_EVENT_33) == 1) {
            ST(V_EVENT_33) = 0;
            /* 0x9B10 */
            if (ST(L32_GORK_ENCOUNTER_STEP) == 2) {
                ST(L32_GORK_ENCOUNTER_STEP) = 99;
                CB->select_pile(2, 2);
                /* 0x9B46 */
                if (CB->divers(DV_PERSONA_PRESENT, P_KONKALITE11, 0)) {
                    CB->animation("SP81GAR26");
                    CB->animation("P81GAR03");
                }
                CB->select_pile(1, 2);
                ST(V_ENGAGED_PERSONA) = P_KONKALITE11;
                CB->play_anim(0, 0);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9BB7 */
            if ((CB->divers(DV_HP, ST(ST_HERO), -1) - 50) > 0) {
                CB->select_pile(0, 1);
                CB->evenement(2, 2500);
                CB->divers(DV_HP, ST(ST_HERO), (CB->divers(DV_HP, ST(ST_HERO), -1) - 20));
                CB->animation(lieu_anim_name(P_ARKHAN2, 22));
                CB->animation("ar");
                return;
            } else {
                CB->select_pile(0, 1);
                ST(ST_HERO) = 4;
                CB->animation(lieu_anim_name(P_ARKHAN2, 26));
                return;
            }
        }
        /* 0x9C7F */
        if (ST(L32_GORK_TAUNT_ACTIVE) == 1 && !CB->on_joindre(P_GORK) && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L32_GORK_TAUNT_ACTIVE) = 0;
            CB->select_pile(1, 1);
            CB->evenement(2, 2500);
            CB->select_pile(0, 1);
        }
        /* 0x9CEC */
        if (ST(V_EVENT_2) == 1 && ST(ST_VIE(P_GORK)) != VIE_DEAD && ST(L32_GORK_ENCOUNTER_STEP) < 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(V_CRI_USED) == 0 && !CB->on_joindre(P_GORK)) {
            CB->select_pile(1, 1);
            ST(V_EVENT_2) = 0;
            ST(L32_TAUNT_VARIANT) = CB->random(3) + 1;
            switch (ST(L32_TAUNT_VARIANT)) {
            case 1:   /* 0x9D8B */
                if (CB->distance(4, 7) > 100) {
                    CB->joindre(P_GORK, 5226, 7139, 0x1518, -1);
                    CB->modify_var(&ST(L32_GORK_TAUNT_ACTIVE), 1);
                } else {
                    ST(V_EVENT_2) = 1;
                }
                CB->select_pile(0, 1);
                break;
            case 2:   /* 0x9DFA */
                if (CB->distance(4, 8) > 100) {
                    CB->joindre(P_GORK, 5646, 7309, 0x1CE8, -1);
                    CB->modify_var(&ST(L32_GORK_TAUNT_ACTIVE), 1);
                } else {
                    ST(V_EVENT_2) = 1;
                }
                CB->select_pile(0, 1);
                break;
            case 3:   /* 0x9E69 */
                if (CB->distance(4, 9) > 100) {
                    CB->joindre(P_GORK, 5352, 7582, 0x3A6, -1);
                    CB->bloque(BLOQUE_ALL);
                    CB->animation("SP31ARR01_10");
                    CB->evenement(33, 300);
                    CB->animation("SP31ATA06");
                    CB->animation("P31ARR01");
                    CB->bloque(BLOQUE_NONE);
                } else {
                    ST(V_EVENT_2) = 1;
                }
                CB->select_pile(0, 1);
                break;
            default:
                break;
            }
        }
        /* 0x9F3C */
        if (ST(V_EVENT_37) == 1 && ST(ST_VIE(P_GORK)) != VIE_DEAD) {
            ST(V_EVENT_37) = 0;
            CB->select_pile(1, 1);
            ST(V_DLG_RANDOM) = CB->random(3);
            CB->animation("SP31ATT01");
            CB->animation("P31ARR01_10");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            CB->evenement(2, 500);
            return;
        }
        /* 0x9FE5 */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_GORK)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
        /* 0xA012 */
        if (ST(L32_SORDOS_SCENE_STEP) == 100) {
            ST(L32_SORDOS_SCENE_STEP) = 0;
            CB->select_pile(1, 1);
            CB->evenement(35, 2500);
            CB->select_pile(0, 1);
        }
        /* 0xA05D */
        if (ST(V_EVENT_35) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
            CB->select_pile(1, 1);
            ST(V_EVENT_35) = 0;
            ST(L32_TAUNT_VARIANT) = CB->random(3) + 1;
            switch (ST(L32_TAUNT_VARIANT)) {
            case 1:   /* 0xA0B6 */
                if (CB->distance(16, 7) > 100) {
                    CB->joindre(P_KONKALITE11, 4824, 7217, 0x5522, -1);
                    CB->animation("SP81ARR01_500");
                    CB->modify_var(&ST(L32_SORDOS_SCENE_STEP), 100);
                } else {
                    ST(V_EVENT_35) = 1;
                }
                CB->select_pile(0, 1);
                break;
            case 2:   /* 0xA130 */
                if (CB->distance(16, 8) > 100) {
                    CB->joindre(P_KONKALITE11, 5738, 7038, 0xA78C, -1);
                    CB->animation("SP81ARR01_500");
                    CB->modify_var(&ST(L32_SORDOS_SCENE_STEP), 100);
                } else {
                    ST(V_EVENT_35) = 1;
                }
                CB->select_pile(0, 1);
                break;
            case 3:   /* 0xA1AA */
                if (CB->distance(16, 9) > 100) {
                    CB->joindre(P_KONKALITE11, 5494, 7592, 0x24CD, -1);
                    CB->animation("SP81ARR01_500");
                    CB->modify_var(&ST(L32_SORDOS_SCENE_STEP), 100);
                } else {
                    ST(V_EVENT_35) = 1;
                }
                CB->select_pile(0, 1);
                break;
            default:
                break;
            }
        }
        /* 0xA255 */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_KONKALITE11)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
    }
    /* 0xA285 */
    if ((ST(L32_GORK_KONKALITE7_STATE) == 1 || ST(L32_GORK_KONKALITE7_STATE) == 2) && (ST(V_CHASE_STATE) == 0 || ST(V_CHASE_STATE) == 10) && ST(L32_ROOM_PLOT_STAGE) == 0) {
        /* 0xA2DC */
        if (ST(L32_KONKALITE7_AMBUSH_ONCE) == 0 && ST(ST_VIE(P_KONKALITE7)) != VIE_DEAD && ST(ST_VIE(P_GORK)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 12) > 400) {
            ST(L32_KONKALITE7_AMBUSH_ONCE) = 1;
            ST(L32_GORK_KONKALITE7_STATE) = 2;
            CB->select_pile(2, 1);
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE7, 1);
            CB->dialogue("L32D13", P_KONKALITE7);
            CB->joindre(P_KONKALITE7, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP77GAR26");
            CB->modify_var(&ST(L32_KONKALITE7_FOLLOWUP), 1);
            CB->animation("P77GAR03");
            CB->dialogue("L32D14", P_KONKALITE7);
            CB->vire_col(14, 1);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xA40A */
        if (ST(L32_KONKALITE7_FOLLOWUP) == 1) {
            ST(L32_KONKALITE7_FOLLOWUP) = 2;
            CB->select_pile(1, 1);
            CB->joindre(P_GORK, -1, -1, -1, ST(ST_HERO));
            CB->select_pile(0, 1);
            return;
        }
        /* 0xA467 */
        if (ST(L32_KONKALITE7_FOLLOWUP) == 2 && CB->distance(1, 4) < 350) {
            ST(L32_KONKALITE7_FOLLOWUP) = 0;
            CB->bloque(BLOQUE_NONE);
            ST(V_ENGAGED_PERSONA) = 3177;   /* P_GORK * 100 + P_KONKALITE7 */
            CB->select_pile(1, 1);
            CB->combat(P_GORK, 0);
            CB->select_pile(2, 1);
            CB->combat(P_KONKALITE7, 0);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xA500 */
        if (ST(ST_VIE(P_KONKALITE7)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && (!CB->divers(DV_CAN_FIGHT, 9, 2) || ST(ST_PLAYER_WEAPON) == 0)) {
            /* 0xA556 */
            if (CB->distance(1, 5) >= 300) {
                CB->select_pile(2, 1);
                /* 0xA575 */
                if (ST(L32_TIMER_OR_FLAG) == 0) {
                    ST(L32_TIMER_OR_FLAG) = 1;
                    CB->combat(P_KONKALITE7, -1);
                }
                CB->divers(DV_RUN, P_KONKALITE7, 1);
                /* 0xA5BB */
                if ((ST(V_FRAME_COUNTER) % 20) == 0) {
                    CB->joindre(-P_KONKALITE7, -1, -1, -1, ST(ST_HERO));
                }
                CB->select_pile(0, 1);
                return;
            }
            /* 0xA608 */
            if (CB->distance(1, 5) < 250) {
                CB->select_pile(2, 1);
                ST(L32_TIMER_OR_FLAG) = 0;
                CB->combat(P_KONKALITE7, 0);
                CB->select_pile(0, 1);
                return;
            }
        }
    }
    /* 0xA655 */
    if ((ST(L32_GORK_KONKALITE7_STATE) == 2 || ST(L32_GORK_KONKALITE7_STATE) == 3) && ST(L32_ROOM_PLOT_STAGE) != 3 && ST(L32_ROOM_PLOT_STAGE) != 4 && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0xA6AC */
        if (ST(ST_VIE(P_GORK)) == VIE_DEAD && ST(L32_GORK_DEAD) == 0) {
            ST(L32_GORK_DEAD) = 1;
        }
        /* 0xA6DF */
        if (ST(ST_VIE(P_KONKALITE7)) == VIE_DEAD && ST(ST_VIE(P_GORK)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L32_GORK_KONKALITE7_STATE) = 10;
            ST(L32_GORK_DEAD) = 1;
        }
    }
    /* 0xA72D */
    if (ST(L32_GORK_KONKALITE7_STATE) != 0 || ST(L32_ROOM_PLOT_STAGE) == 5) {
        /* 0xA759 */
        if (CB->divers(DV_WAS_HIT, P_KONKALITE7, 0) && ST(ST_VIE(P_KONKALITE7)) != VIE_DEAD && ST(L32_ROOM_PLOT_STAGE) != 5) {
            CB->retire_zone(7);
            CB->select_pile(2, 2);
            CB->divers_empile(DV_WAS_HIT, P_KONKALITE7, 1);
            CB->combat(P_KONKALITE7, -1);
            /* 0xA7CC */
            if (ST(L32_GORK_KONKALITE7_STATE) == 1) {
                ST(L32_GORK_KONKALITE7_STATE) = 2;
            }
            /* 0xA801 */
            if ((CB->divers(DV_HP, P_KONKALITE7, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_KONKALITE7, (CB->divers(DV_HP, P_KONKALITE7, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_KONKALITE7, 1);
            }
            /* 0xA862 */
            if (CB->divers(DV_HP, P_KONKALITE7, -1) <= 1) {
                ST(ST_VIE(P_KONKALITE7)) = VIE_DEAD;
                CB->stop_joindre(-P_KONKALITE7);
                CB->divers_empile(DV_BLOOD_AT_BONE, 77, 0);
                CB->animation("SP77IMP53");
                CB->animation("P77CAD");
                CB->pos_perso(P_KONKALITE7, 0, 0, 0, 2, "P77CAD", 5, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_KONKALITE7);
                CB->divers_empile(DV_BLOOD_AT_BONE, 77, 0);
                CB->animation("SP77IMP13");
                CB->animation("P77IMP48");
                CB->animation("P77GAR03");
                CB->combat(P_KONKALITE7, 0);
                ST(V_ENGAGED_PERSONA) = 3177;   /* P_GORK * 100 + P_KONKALITE7 */
                CB->select_pile(1, 1);
                /* 0xA950 */
                if (ST(ST_VIE(P_GORK)) != VIE_DEAD) {
                    CB->combat(P_GORK, 0);
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0xA984 */
        if (CB->divers(DV_WAS_HIT, P_GORK, 0) && ST(ST_VIE(P_GORK)) != VIE_DEAD) {
            CB->retire_zone(7);
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, P_GORK, 1);
            CB->combat(P_GORK, -1);
            CB->divers(DV_CUT_SPEECH, 0, 0);
            /* 0xA9F4 */
            if (ST(L32_GORK_KONKALITE7_STATE) == 1 && ST(L32_ROOM_PLOT_STAGE) != 5) {
                ST(L32_GORK_KONKALITE7_STATE) = 2;
            }
            /* 0xAA3B */
            if ((CB->divers(DV_HP, P_GORK, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_GORK, (CB->divers(DV_HP, P_GORK, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_GORK, 1);
            }
            /* 0xAA9C */
            if (CB->divers(DV_HP, P_GORK, -1) <= 1) {
                ST(ST_VIE(P_GORK)) = VIE_DEAD;
                CB->stop_joindre(-P_GORK);
                CB->divers_empile(DV_BLOOD_AT_BONE, 31, 0);
                CB->animation("SP31IMP51");
                CB->modify_var(&ST(ST_VIE(P_GORK)), 2);
                CB->animation("P31CAD");
                CB->pos_perso(P_GORK, 0, 0, 0, 2, "P31CAD", 4, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_GORK);
                CB->divers_empile(DV_BLOOD_AT_BONE, 31, 0);
                CB->animation("SP31IMP11");
                CB->animation("P31IMP46");
                CB->animation("P31GAR01");
                CB->combat(P_GORK, 0);
                /* 0xAB82 */
                if (ST(V_ENGAGED_PERSONA) == 0) {
                    /* 0xAB94 */
                    if (ST(L32_ROOM_PLOT_STAGE) != 5) {
                        ST(V_ENGAGED_PERSONA) = 3177;   /* P_GORK * 100 + P_KONKALITE7 */
                    } else {
                        ST(V_ENGAGED_PERSONA) = P_GORK;
                    }
                }
                CB->select_pile(2, 1);
                /* 0xABD3 */
                if (ST(ST_VIE(P_KONKALITE7)) != VIE_DEAD && ST(L32_ROOM_PLOT_STAGE) != 5) {
                    CB->combat(P_KONKALITE7, 0);
                }
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0xAC11 */
    if (ST(L32_ROOM_PLOT_STAGE) == 6) {
        /* 0xAC2B */
        if (CB->divers(DV_WAS_HIT, 29, 0) && ST(ST_VIE(29)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, 29, 1);
            CB->combat(29, -1);
            /* 0xAC95 */
            if ((CB->divers(DV_HP, 29, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, 29, (CB->divers(DV_HP, 29, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, 29, 1);
            }
            /* 0xACF6 */
            if (CB->divers(DV_HP, 29, -1) <= 1) {
                ST(ST_VIE(29)) = VIE_DEAD;
                CB->stop_joindre(-29);
                CB->divers_empile(DV_BLOOD_AT_BONE, 29, 0);
                CB->animation("SP29IMP51");
                CB->animation("P29CAD");
                CB->pos_perso(29, 0, 0, 0, 2, "P29CAD", 7, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-29);
                CB->divers_empile(DV_BLOOD_AT_BONE, 29, 0);
                CB->animation("SP29IMP06");
                CB->animation("P29GAR01");
                CB->combat(29, 0);
                ST(V_ENGAGED_PERSONA) = 29;
            }
            CB->select_pile(0, 1);
        }
        /* 0xADD3 */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(29)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
    }
    /* 0xAE03 */
    if (ST(L32_ROOM_PLOT_STAGE) == 1 && ST(ST_LOCKED_CAMERA) >= 2) {
        ST(L32_CAMERA_CUT_DONE) = 1;
        ST(L32_ROOM_PLOT_STAGE) = 2;
    }
    /* 0xAE45 */
    if (ST(L32_ROOM_PLOT_STAGE) == 3 || ST(L32_ROOM_PLOT_STAGE) == 4) {
        /* 0xAE71 */
        if (CB->divers(DV_WAS_HIT, P_KONKALITE3, 0) && ST(ST_VIE(P_KONKALITE3)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, P_KONKALITE3, 1);
            CB->combat(P_KONKALITE3, -1);
            ST(ST_VIE(P_KONKALITE3)) = VIE_DEAD;
            /* 0xAED6 */
            if (ST(L32_KONKALITE3_STATE) == 0) {
                ST(L32_KONKALITE3_STATE) = 1;
            }
            /* 0xAEF4 */
            if (ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = P_KONKALITE3;
            }
            CB->stop_joindre(-P_KONKALITE3);
            CB->divers_empile(DV_BLOOD_AT_BONE, 73, 0);
            CB->animation("SP73IMP53");
            CB->animation("P73CAD");
            CB->pos_perso(P_KONKALITE3, 0, 0, 0, 2, "P73CAD", 0xF, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xAF8D */
        if (CB->divers(DV_WAS_HIT, P_MALROT6, 0) && ST(ST_VIE(P_MALROT6)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            CB->divers_empile(DV_WAS_HIT, P_MALROT6, 1);
            ST(ST_VIE(P_MALROT6)) = VIE_DEAD;
            ST(L32_MALROT6_DEAD) = 10;
            CB->stop_joindre(-P_MALROT6);
            CB->divers_empile(DV_BLOOD_AT_BONE, 66, 0);
            CB->animation("SP66IMP52");
            CB->animation("P66CAD");
            CB->pos_perso(P_MALROT6, 0, 0, 0, 2, "P66CAD", 0xE, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
            CB->select_pile(0, 1);
        }
        /* 0xB05F */
        if (ST(V_EVENT_33) == 1) {
            ST(V_EVENT_33) = 0;
            CB->select_pile(1, 2);
            CB->play_anim(0, 0);
            CB->select_pile(0, 1);
            return;
        }
        /* 0xB0AC */
        if (ST(L32_KONKALITE3_STATE) == 0 && ST(ST_VIE(P_KONKALITE3)) != VIE_DEAD) {
            /* 0xB0CD */
            if (ST(L32_TIMER_OR_FLAG) == 0) {
                ST(L32_KONKALITE3_TAUNT) = 0;
                ST(L32_TIMER_OR_FLAG) = ST(ST_GAME_TIME);
            }
            /* 0xB102 */
            if (ST(L32_KONKALITE3_TAUNT) == 0 && (ST(ST_GAME_TIME) - ST(L32_TIMER_OR_FLAG)) > 5000) {
                ST(L32_KONKALITE3_TAUNT) = CB->random(3) + 1;
                CB->bloque(BLOQUE_ALL);
                CB->select_pile(1, 1);
                ST(V_DLG_RANDOM) = CB->random(3);
                /* 0xB17D */
                if (ST(V_DLG_RANDOM) == 0) {
                    CB->dialogue("L32D226", P_MALROT6);
                }
                /* 0xB19F */
                if (ST(V_DLG_RANDOM) == 1) {
                    CB->dialogue("L32D227", P_MALROT6);
                }
                /* 0xB1C1 */
                if (ST(V_DLG_RANDOM) == 2) {
                    CB->dialogue("L32D228", P_MALROT6);
                }
                CB->animation("SP66ATT03");
                CB->animation("P66ATT02");
                /* 0xB1F9 */
                if (ST(L32_KONKALITE3_TAUNT) == 1) {
                    lieu_action_register("L32D666", P_KONKALITE3, "SP73ATT02", "SP73ATT01");
                }
                /* 0xB224 */
                if (ST(L32_KONKALITE3_TAUNT) == 2) {
                    lieu_action_register("L32D667", P_KONKALITE3, "SP73ATT02", "SP73ATT01");
                }
                /* 0xB24F */
                if (ST(L32_KONKALITE3_TAUNT) == 3) {
                    lieu_action_register("L32D668", P_KONKALITE3, "SP73ATT02", "SP73ATT01");
                }
                CB->animation("Sar_50");
                CB->modify_var(&ST(L32_TIMER_OR_FLAG), 0);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0xB2B0 */
            if (ST(L32_CHEER_PLAYED) == 0 && (CB->distance(1, 12) > 400 || ST(L32_MALROT6_DEAD) == 10 || (ST(V_PLAYER_STRIKE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && !CB->divers(DV_WAS_HIT, P_MALROT6, 0) && !CB->divers(DV_WAS_HIT, P_KONKALITE3, 0)))) {
                ST(L32_CHEER_PLAYED) = 1;
                ST(L32_KONKALITE3_STATE) = 1;
                CB->select_pile(1, 2);
                CB->bloque(BLOQUE_ALL);
                CB->divers(DV_LOOK_AT_PLAYER, P_MALROT6, 1);
                CB->animation("SP73ATT03");
                CB->animation("P73GAR03");
                CB->pos_perso(P_KONKALITE3, 5158, 7032, 0x1B50, 1, "P73GAR03", 0xF, 0);
                CB->animation("SP73GAR03");
                CB->joindre(-P_KONKALITE3, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L32D100", P_KONKALITE3, "SP73GAR03", "SP73GAR03");
                CB->animation("Sar_50");
                CB->combat(P_KONKALITE3, 0);
                ST(V_ENGAGED_PERSONA) = P_KONKALITE3;
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0xB451 */
        if (ST(L32_KONKALITE3_STATE) == 1 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_KONKALITE3)) == VIE_DEAD) {
            CB->select_pile(1, 1);
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L32_KONKALITE3_STATE) = 2;
            /* 0xB4AF */
            if (ST(ST_VIE(P_MALROT6)) != VIE_DEAD) {
                CB->dialogue("L32D225", P_MALROT6);
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0xB4DE */
    if (ST(L32_ROOM_PLOT_STAGE) == 5 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_GORK)) == VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0;
        ST(L32_GORK_DEAD) = 1;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x10010EA9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_10021A7C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu32_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x100110D2);
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
        l32_trig00();   /* 0x100110D9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x10010FBE - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x10010B10).
 * Jump table @ 0x100111A9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu32_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu32: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l32_trigger(arg);             break;  /* dword_10021910[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
