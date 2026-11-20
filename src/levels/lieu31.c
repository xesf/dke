/*
 * lieu31.c - Native hand-port of LIEU31.DLL (level-31 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu31_trig.c (reached through
 * l31_trigger, see lieu31_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU30; the
 * entry is at 0x10010C70, the state pointer global is dword_10021C8C and the
 * trigger table is dword_10021BA0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1001133C), lieu_pre_scene (sub_100010AF thunk)
 * and sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are the same as LIEU30. tick_body (sub_10008861) is level-31
 * specific. The only DLL .data global the tick body touches is the debug
 * sprintf buffer, which is reset at INIT.
 *
 * V_ENGAGED_PERSONA holds two packed persona ids as a*100+b in this level:
 * 0xC09 = 3081 = Bork(30) + Konkalite11(81), 0x1EC8 = 7880 = Konkalite8(78)
 * + Konkalite10(80).
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu31_int.h"

static c8 g_txt[512];   /* debug sprintf buffer, dword_100219A0 */

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10008861 (thunk 0x100010B9), level-31 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    snprintf(g_txt, sizeof(g_txt), "Alcool:%d", ST(ST_VARIANT_COUNT(ST(ST_USED_VARIANT))));
    CB->print_msg(g_txt, 0, 240);
    /* 0x88B8 */
    if (CB->divers(DV_PERSONA_PRESENT, P_KONKALITE8, 0) && CB->divers(DV_WAS_HIT, P_KONKALITE8, 0) && ST(ST_VIE(P_KONKALITE8)) != VIE_DEAD) {
        CB->evenement(2, -1);
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_KONKALITE8);
        /* 0x891E */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_KONKALITE8;
        }
        CB->divers_empile(DV_WAS_HIT, P_KONKALITE8, 1);
        CB->combat(P_KONKALITE8, -1);
        /* 0x8972 */
        if ((CB->divers(DV_HP, P_KONKALITE8, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_KONKALITE8, (CB->divers(DV_HP, P_KONKALITE8, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_KONKALITE8, 1);
        }
        /* 0x89D3 */
        if (CB->divers(DV_HP, P_KONKALITE8, -1) <= 1) {
            ST(ST_VIE(P_KONKALITE8)) = VIE_DEAD;
            CB->stop_joindre(-P_KONKALITE8);
            CB->divers_empile(DV_BLOOD_AT_BONE, 78, 0);
            CB->animation("SP78IMP53");
            CB->animation("P78CAD");
            CB->pos_perso(P_KONKALITE8, 0, 0, 0, 2, "P78CAD", 2, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            ST(V_ENGAGED_PERSONA) = P_KONKALITE8;
            CB->stop_joindre(-P_KONKALITE8);
            CB->divers_empile(DV_BLOOD_AT_BONE, 78, 0);
            CB->animation("SP78IMP13");
            CB->animation("P78IMP48");
            CB->animation("P78GAR03");
            CB->combat(P_KONKALITE8, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8AC6 */
    if (CB->divers(DV_PERSONA_PRESENT, P_KONKALITE10, 0) && CB->divers(DV_WAS_HIT, P_KONKALITE10, 0) && ST(ST_VIE(P_KONKALITE10)) != VIE_DEAD) {
        CB->evenement(2, -1);
        CB->select_pile(2, 2);
        CB->stop_joindre(-P_KONKALITE10);
        /* 0x8B2C */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_KONKALITE10;
        }
        CB->divers_empile(DV_WAS_HIT, P_KONKALITE10, 1);
        CB->combat(P_KONKALITE10, -1);
        /* 0x8B80 */
        if ((CB->divers(DV_HP, P_KONKALITE10, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_KONKALITE10, (CB->divers(DV_HP, P_KONKALITE10, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_KONKALITE10, 1);
        }
        /* 0x8BE1 */
        if (CB->divers(DV_HP, P_KONKALITE10, -1) <= 1) {
            ST(ST_VIE(P_KONKALITE10)) = VIE_DEAD;
            CB->stop_joindre(-P_KONKALITE10);
            CB->divers_empile(DV_BLOOD_AT_BONE, 80, 0);
            CB->animation("SP80IMP53");
            CB->animation("P80CAD");
            CB->pos_perso(P_KONKALITE10, 0, 0, 0, 2, "P80CAD", 3, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            ST(V_ENGAGED_PERSONA) = P_KONKALITE10;
            CB->stop_joindre(-P_KONKALITE10);
            CB->divers_empile(DV_BLOOD_AT_BONE, 80, 0);
            CB->animation("SP80IMP13");
            CB->animation("P80IMP48");
            CB->animation("P80GAR03");
            CB->combat(P_KONKALITE10, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8CD4 */
    if (CB->divers(DV_PERSONA_PRESENT, P_BORK, 0) && CB->divers(DV_WAS_HIT, P_BORK, 0) && ST(ST_VIE(P_BORK)) != VIE_DEAD) {
        CB->select_pile(2, 1);
        /* 0x8D1F */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = 0xC09;
            CB->divers(DV_LOOK_AT_PLAYER, P_BORK, 0);
            CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE11, 0);
            CB->combat(P_KONKALITE11, 0);
        }
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_BORK, 1);
        CB->combat(P_BORK, -1);
        /* 0x8DB4 */
        if ((CB->divers(DV_HP, P_BORK, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_BORK, (CB->divers(DV_HP, P_BORK, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_BORK, 1);
        }
        /* 0x8E15 */
        if (CB->divers(DV_HP, P_BORK, -1) <= 1) {
            ST(ST_VIE(P_BORK)) = VIE_DEAD;
            CB->stop_joindre(-P_BORK);
            CB->divers_empile(DV_BLOOD_AT_BONE, 30, 0);
            CB->animation("SP30IMP53");
            CB->animation("P30CAD");
            CB->pos_perso(P_BORK, 0, 0, 0, 2, "P30CAD", 4, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_BORK);
            CB->divers_empile(DV_BLOOD_AT_BONE, 30, 0);
            CB->animation("SP30IMP13");
            CB->animation("P30IMP48");
            CB->animation("P30GAR03");
            CB->combat(P_BORK, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8EFC */
    if (CB->divers(DV_PERSONA_PRESENT, P_KONKALITE11, 0) && CB->divers(DV_WAS_HIT, P_KONKALITE11, 0) && ST(ST_VIE(P_KONKALITE11)) != VIE_DEAD) {
        CB->select_pile(1, 1);
        /* 0x8F47 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = 0xC09;
            CB->divers(DV_LOOK_AT_PLAYER, P_BORK, 0);
            CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE11, 0);
            CB->combat(P_BORK, 0);
        }
        CB->select_pile(2, 2);
        CB->divers_empile(DV_WAS_HIT, P_KONKALITE11, 1);
        CB->combat(P_KONKALITE11, -1);
        /* 0x8FDC */
        if ((CB->divers(DV_HP, P_KONKALITE11, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_KONKALITE11, (CB->divers(DV_HP, P_KONKALITE11, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_KONKALITE11, 1);
        }
        /* 0x903D */
        if (CB->divers(DV_HP, P_KONKALITE11, -1) <= 1) {
            ST(ST_VIE(P_KONKALITE11)) = VIE_DEAD;
            CB->stop_joindre(-P_KONKALITE11);
            CB->divers_empile(DV_BLOOD_AT_BONE, 81, 0);
            CB->animation("SP81IMP53");
            CB->animation("P81CAD");
            CB->pos_perso(P_KONKALITE11, 0, 0, 0, 2, "P81CAD", 5, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_KONKALITE11);
            CB->divers_empile(DV_BLOOD_AT_BONE, 81, 0);
            CB->animation("SP81IMP13");
            CB->animation("P81IMP48");
            CB->animation("P81GAR03");
            CB->combat(P_KONKALITE11, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x911C */
    if ((ST(V_CHASE_ROUTE) == 1 || ST(V_CHASE_ROUTE) == 11) && ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        CB->divers(DV_HP, ST(ST_HERO), 1);
        /* 0x9179 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation(lieu_anim_name(P_ARKHAN2, 24));
            CB->modify_var(&ST(ST_HERO), 4);
            return;
        }
        /* 0x91C1 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation(lieu_anim_name(P_ARKHAN3, 24));
            CB->modify_var(&ST(ST_HERO), 4);
            return;
        }
    }
    /* 0x9209 */
    if (ST(ST_HERO) != 4) {
        ST(L31_LAST_LIVING_HERO) = ST(ST_HERO);
    }
    /* 0x9232 */
    if (ST(ST_HERO) == 4 && ST(L31_DEATH_HANDLED) == 0) {
        ST(L31_DEATH_HANDLED) = 1;
        CB->evenement(1, 3000);
    }
    /* 0x9275 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x9296 */
        if (ST(L31_ARRIVAL_KIND) == 1) {
            /* 0x92A8 */
            if (ST(V_CHASE_STATE) == 0 && ST(V_SORDOS_DUEL) < 2 && ST(L31_STORY_PART) == 10 && (ST(ST_VIE(P_GORK)) < 2 || ST(ST_VIE(P_KONKALITE11)) < 2)) {
                ST(V_CHASE_STATE) = 1;
                ST(L31_BORK_ROUTE_STAGE) = 10;
                /* 0x9368 */
                if (ST(V_CHASE_ROUTE) != 1 && ST(V_CHASE_ROUTE) != 11) {
                    ST(V_CHASE_ROUTE) = 1;
                }
                ST(ST_VIE(69)) = 0;
                ST(ST_LEVEL) = 31;
                CB->change_camera(99, CAM_AUTO);
                CB->change_piece(32, 0);
            } else {
                /* 0x93DD */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
                    CB->divers(DV_HP, P_ARKHAN0, 0);
                }
                /* 0x9406 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
                    CB->divers(DV_HP, P_ARKHAN1, 0);
                }
                /* 0x9432 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
                    CB->divers(DV_HP, P_ARKHAN2, 0);
                }
                /* 0x945E */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
                    CB->divers(DV_HP, P_ARKHAN3, 0);
                }
                /* 0x9482 */
                if (ST(V_GUARD_FIGHT_DONE) == 0) {
                    CB->cinematique("SC22_500");
                }
                /* 0x949F */
                if (ST(V_GUARD_FIGHT_DONE) != 0) {
                    CB->cinematique("SC23_500");
                }
                CB->divers_empile(DV_END_GAME, 0, 0);
            }
        }
        /* 0x94CB */
        if (ST(L31_ARRIVAL_KIND) == 0 || ST(L31_ARRIVAL_KIND) == 2) {
            /* 0x94EF */
            if (ST(V_CHASE_STATE) == 0 && ST(V_SORDOS_DUEL) < 2 && ST(L31_STORY_PART) == 2 && (ST(ST_VIE(P_GORK)) < 2 || ST(ST_VIE(P_KONKALITE11)) < 2)) {
                ST(V_CHASE_STATE) = 1;
                ST(L31_BORK_ROUTE_STAGE) = 30;
                /* 0x95AF */
                if (ST(V_CHASE_ROUTE) != 1 && ST(V_CHASE_ROUTE) != 11) {
                    ST(V_CHASE_ROUTE) = 1;
                }
                ST(ST_VIE(69)) = 1;
                ST(ST_LEVEL) = 31;
                CB->change_camera(99, CAM_AUTO);
                CB->change_piece(32, 0);
            } else {
                /* 0x9624 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
                    CB->divers(DV_HP, P_ARKHAN0, 0);
                }
                /* 0x964D */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
                    CB->divers(DV_HP, P_ARKHAN1, 0);
                }
                /* 0x9679 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
                    CB->divers(DV_HP, P_ARKHAN2, 0);
                }
                /* 0x96A5 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
                    CB->divers(DV_HP, P_ARKHAN3, 0);
                }
                /* 0x96C9 */
                if (ST(V_GUARD_FIGHT_DONE) == 0) {
                    CB->cinematique("SC22_500");
                }
                /* 0x96E6 */
                if (ST(V_GUARD_FIGHT_DONE) != 0) {
                    CB->cinematique("SC23_500");
                }
                CB->divers_empile(DV_END_GAME, 0, 0);
            }
        }
    }
    /* 0x9712 */
    if (ST(V_CHASE_ROUTE) != 1 && ST(L31_BORK_ROUTE_STAGE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KONKALITE8)) != VIE_DEAD && ST(ST_VIE(P_KONKALITE10)) != VIE_DEAD) {
        /* 0x9766 */
        if (ST(L31_KONKALITE8_AMBUSH) == 0 && ST(ST_VIE(P_KONKALITE8)) != VIE_DEAD && ST(L31_KONKALITE8_HITS) < 5 && ST(L31_KONKALITE10_HITS) < 5 && ST(ST_LOCKED_CAMERA) == 0 && CB->distance(1, 2) < 700) {
            ST(L31_KONKALITE8_AMBUSH) = 1;
            ST(L31_KONKALITE8_HITS) = 1;
            ST(L31_KONKALITE10_HITS) = 1;
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            /* 0x9820 */
            if (ST(ST_LOCKED_CAMERA) != 4) {
                CB->change_camera(4, CAM_LOCKED);
            }
            lieu_action_register("L31D500", P_KONKALITE8, "SP78DIA53", "SP78ARR01_10");
            CB->select_pile(2, 1);
            CB->animation("SP80ARR01_10");
            CB->joindre(-P_KONKALITE10, 0, 0, 0, ST(ST_HERO));
            CB->animation("P80GAR26");
            CB->animation("P80GAR03");
            CB->joindre_perm(P_KONKALITE10, ST(ST_HERO));
            CB->select_pile(1, 1);
            CB->joindre(-P_KONKALITE8, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L31D35", P_KONKALITE8, "SP78ATT50", "SP78ARR01_10");
            CB->joindre_perm(P_KONKALITE8, ST(ST_HERO));
            CB->change_camera(99, CAM_AUTO);
            CB->evenement(2, 3000);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
        }
        /* 0x9950 */
        if (ST(V_EVENT_2) == 1 && ST(ST_VIE(P_KONKALITE8)) != VIE_DEAD) {
            ST(V_EVENT_2) = 0;
            /* 0x9983 */
            if (ST(L31_KONKALITE8_HITS) == 1) {
                /* 0x999E */
                if (CB->distance(1, 2) > 300) {
                    ST(L31_KONKALITE8_HITS) = 2;
                    CB->select_pile(1, 2);
                    CB->bloque(BLOQUE_ALL);
                    CB->stop_joindre(-P_KONKALITE8);
                    lieu_action_register("L31D36", P_KONKALITE8, "SP78GAR26", "SP78GAR03_100");
                    CB->joindre_perm(P_KONKALITE8, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                    CB->evenement(2, 4000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9A47 */
                if (CB->distance(1, 2) <= 300 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
                    ST(L31_KONKALITE8_HITS) = 2;
                    CB->select_pile(1, 2);
                    CB->bloque(BLOQUE_ALL);
                    CB->stop_joindre(-P_KONKALITE8);
                    lieu_action_register("L31D37", P_KONKALITE8, "SP78GAR26", "SP78GAR03_100");
                    CB->joindre_perm(P_KONKALITE8, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                    CB->evenement(2, 4000);
                    CB->select_pile(0, 1);
                    return;
                }
            }
            /* 0x9B07 */
            if (ST(L31_KONKALITE8_HITS) == 2) {
                ST(L31_KONKALITE8_HITS) = 3;
                CB->select_pile(1, 2);
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_KONKALITE8);
                CB->dialogue("L31D38", P_KONKALITE8);
                CB->combat(P_KONKALITE8, 0);
                ST(V_ENGAGED_PERSONA) = P_KONKALITE8;
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x9B8E */
        if ((ST(L31_KONKALITE8_AMBUSH) == 2 || ST(L31_KONKALITE10_AMBUSH) == 1) && ST(ST_VIE(P_KONKALITE8)) != VIE_DEAD && (CB->distance(1, 2) > 300 || CB->distance(1, 7) < 250) && ST(V_ENGAGED_PERSONA) == 0 && ST(L31_KONKALITE8_HITS) < 6 && ST(L31_KONKALITE10_HITS) < 6 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 3)) {
            ST(L31_KONKALITE8_AMBUSH) = 3;
            ST(L31_KONKALITE10_AMBUSH) = 2;
            /* 0x9C62 */
            if (ST(ST_LOCKED_CAMERA) == 4) {
                CB->change_camera(0, CAM_AUTO);
            }
            CB->select_pile(1, 2);
            CB->stop_joindre(-P_KONKALITE8);
            CB->combat(P_KONKALITE8, 0);
            ST(V_ENGAGED_PERSONA) = P_KONKALITE8;
            CB->dialogue("L31D39", P_KONKALITE8);
            CB->select_pile(0, 1);
        }
        /* 0x9CCF */
        if (ST(L31_KONKALITE_RETREAT_WHO) == 0 && ST(L31_KONKALITE_RETREAT_DONE) == 0) {
            /* 0x9CF3 */
            if (ST(L31_KONKALITE10_HITS) == 6) {
                ST(L31_KONKALITE8_HITS) = 99;
                ST(L31_KONKALITE_RETREAT_DONE) = 1;
                ST(L31_KONKALITE_RETREAT_WHO) = 78;
                CB->select_pile(1, 1);
                CB->stop_joindre(-P_KONKALITE8);
                CB->animation("SP78GAR27");
                CB->animation("P78ARR01");
                CB->joindre(P_KONKALITE8, 3101, 6559, -1, -1);
                CB->animation("P78ARR01");
                CB->joindre(P_KONKALITE8, 3380, 6138, 996, -1);
                CB->animation("P78ATT04");
            }
            /* 0x9DAB */
            if (ST(L31_KONKALITE8_HITS) == 6) {
                ST(L31_KONKALITE10_HITS) = 99;
                ST(L31_KONKALITE_RETREAT_DONE) = 1;
                ST(L31_KONKALITE_RETREAT_WHO) = 80;
                CB->select_pile(2, 1);
                CB->stop_joindre(-P_KONKALITE10);
                CB->animation("SP80GAR27");
                CB->animation("P80ARR01");
                CB->joindre(P_KONKALITE10, 3380, 6138, 1828, -1);
                CB->animation("P80ATT04");
            }
        }
    }
    /* 0x9E3F */
    if (ST(L31_BORK_ROUTE_STAGE) == 2) {
        CB->select_pile(1, 1);
        /* 0x9E5E */
        if (ST(L31_BORK_APPROACH_STEP) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 4) < 1000) {
            ST(L31_BORK_APPROACH_STEP) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->change_camera(4, CAM_LOCKED);
            CB->divers(DV_LOOK_AT_PLAYER, P_BORK, 1);
            CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE11, 1);
            CB->joindre_perm(P_KONKALITE11, ST(ST_HERO));
            CB->stop_joindre(P_KONKALITE11);
            CB->animation("SP81GAR03");
            CB->joindre_perm(P_KONKALITE11, ST(ST_HERO));
            lieu_action_register("L31D501", P_BORK, "SP30GAR03", "SP30GAR03_10");
            CB->joindre_perm(P_BORK, ST(ST_HERO));
            CB->dialogue("L31D502", P_KONKALITE11);
            CB->dialogue("L31D503", P_BORK);
            CB->joindre_perm(P_KONKALITE11, ST(ST_HERO));
            CB->change_camera(99, CAM_AUTO);
            CB->evenement(35, 4000);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x9FB9 */
        if (ST(V_EVENT_35) == 1) {
            ST(V_EVENT_35) = 0;
            ST(V_ENGAGED_PERSONA) = 0xC09;
            CB->bloque(BLOQUE_ALL);
            CB->divers(DV_LOOK_AT_PLAYER, P_BORK, 0);
            CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE11, 0);
            CB->stop_joindre(P_BORK);
            CB->stop_joindre(P_KONKALITE11);
            CB->dialogue("L31D507", P_KONKALITE11);
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_BORK, 0);
            CB->combat(P_KONKALITE11, 0);
            return;
        }
        /* 0xA05F */
        if (ST(L31_BORK_APPROACH_STEP) == 1) {
            /* 0xA07A */
            if (CB->distance(1, 4) < 300 && ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = 0xC09;
                ST(L31_BORK_APPROACH_STEP) = 2;
                CB->bloque(BLOQUE_ALL);
                CB->divers(DV_LOOK_AT_PLAYER, P_BORK, 0);
                CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE11, 0);
                CB->stop_joindre(P_BORK);
                CB->stop_joindre(P_KONKALITE11);
                CB->dialogue("L31D504", P_BORK);
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_BORK, 0);
                CB->combat(P_KONKALITE11, 0);
                return;
            }
            /* 0xA138 */
            if (CB->distance(1, 5) < 250 && ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = 0xC09;
                ST(L31_BORK_APPROACH_STEP) = 2;
                CB->bloque(BLOQUE_ALL);
                CB->divers(DV_LOOK_AT_PLAYER, P_BORK, 0);
                CB->divers(DV_LOOK_AT_PLAYER, P_KONKALITE11, 0);
                CB->stop_joindre(P_BORK);
                CB->stop_joindre(P_KONKALITE11);
                lieu_action_register("L31D89", P_KONKALITE11, "SP81GAR03", "SP81GAR03");
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_BORK, 0);
                CB->combat(P_KONKALITE11, 0);
                return;
            }
        }
    }
    /* 0xA1F6 */
    if (ST(L31_BORK_ROUTE_STAGE) == 1 && ST(V_CHASE_ROUTE) != 1 && ST(L31_AMBUSH_CAMERA_RELEASED) == 0 && ST(ST_LOCKED_CAMERA) == 4 && CB->distance(1, 7) > 900 && (ST(L31_KONKALITE8_HITS) >= 5 || ST(L31_KONKALITE10_HITS) >= 5)) {
        CB->change_camera(99, CAM_AUTO);
        ST(L31_AMBUSH_CAMERA_RELEASED) = 1;
    }
    /* 0xA29C */
    if (ST(L31_AMBUSH_CAMERA_RELEASED) == 1 && ST(ST_LOCKED_CAMERA) != 4) {
        ST(L31_AMBUSH_CAMERA_RELEASED) = 0;
    }
    /* 0xA2CF */
    if (ST(L31_BORK_ROUTE_STAGE) == 1 || ST(L31_BORK_ROUTE_STAGE) == 10) {
        /* 0xA2F0 */
        if (ST(V_ENGAGED_PERSONA) == P_KONKALITE8 && ST(ST_VIE(P_KONKALITE10)) != VIE_DEAD) {
            CB->select_pile(2, 1);
            CB->change_camera(99, CAM_AUTO);
            /* 0xA335 */
            if (CB->on_joindre(P_KONKALITE8)) {
                CB->stop_joindre(-P_KONKALITE8);
            }
            /* 0xA353 */
            if (CB->on_joindre(P_KONKALITE10)) {
                CB->stop_joindre(-P_KONKALITE10);
            }
            CB->combat(P_KONKALITE10, 0);
            ST(V_ENGAGED_PERSONA) = 0x1EC8;
            CB->select_pile(0, 1);
        }
        /* 0xA393 */
        if (ST(V_ENGAGED_PERSONA) == P_KONKALITE10 && ST(ST_VIE(P_KONKALITE8)) != VIE_DEAD) {
            CB->select_pile(1, 1);
            CB->change_camera(99, CAM_AUTO);
            /* 0xA3D8 */
            if (CB->on_joindre(P_KONKALITE8)) {
                CB->stop_joindre(-P_KONKALITE8);
            }
            /* 0xA3F6 */
            if (CB->on_joindre(P_KONKALITE10)) {
                CB->stop_joindre(-P_KONKALITE10);
            }
            CB->combat(P_KONKALITE8, 0);
            ST(V_ENGAGED_PERSONA) = 0x1EC8;
            CB->select_pile(0, 1);
        }
        /* 0xA439 */
        if (ST(ST_VIE(P_KONKALITE8)) == VIE_DEAD && ST(ST_VIE(P_KONKALITE10)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            /* 0xA46C */
            if (ST(ST_LOCKED_CAMERA) == 3) {
                CB->change_camera(99, CAM_AUTO);
            }
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L31_BORK_ROUTE_STAGE) = 20;
        }
        /* 0xA4A9 */
        if (ST(ST_VIE(P_KONKALITE8)) == VIE_DEAD && ST(ST_VIE(P_KONKALITE10)) == VIE_DEAD && ST(L31_KONKALITE10_HITS) == 8 && ST(L31_KONKALITE8_HITS) == 8 && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0xA500 */
            if (ST(ST_LOCKED_CAMERA) == 3) {
                CB->change_camera(99, CAM_AUTO);
            }
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L31_BORK_ROUTE_STAGE) = 20;
        }
    }
    /* 0xA53D */
    if (ST(L31_BORK_ROUTE_STAGE) == 2 || ST(L31_BORK_ROUTE_STAGE) == 30) {
        /* 0xA561 */
        if (ST(V_ENGAGED_PERSONA) == 0xC09 && ST(ST_VIE(P_KONKALITE11)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = P_BORK;
        }
        /* 0xA591 */
        if (ST(V_ENGAGED_PERSONA) == 0xC09 && ST(ST_VIE(P_BORK)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = P_KONKALITE11;
        }
        /* 0xA5BE */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_BORK)) == VIE_DEAD && ST(ST_VIE(P_KONKALITE11)) == VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L31_BORK_ROUTE_STAGE) = 35;
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x10010E09).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001CA2C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu31_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x10011232);
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
        l31_trig00();   /* 0x10011239: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1001111E - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x10010C70).
 * Jump table @ 0x10011309, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu31_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu31: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        g_txt[0] = '\0';
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l31_trigger(arg);             break;  /* dword_10021BA0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
