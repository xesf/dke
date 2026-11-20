/*
 * lieu49.c — Native hand-port of LIEU49.DLL (level-49 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu49_trig.c (reached through
 * l49_trigger, see lieu49_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000C010, the state
 * pointer global is dword_1001BC3C and the trigger table is
 * dword_1001BB50[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000C6DC), lieu_pre_scene (sub_1000C88C) and
 * sub_10001200 (float reset) are the shared lieu_common.c. The tick body
 * (sub_10007F43) is level-49 specific.
 */

#include <stdio.h>

#include "lieu49_int.h"

/* Debug sprintf buffer of the tick body (DLL global dword_1001B950, starts ""). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * Per-frame body — sub_10007F43 (thunk 0x100010B9), level-49 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    snprintf(g_txt, sizeof(g_txt), "test4:%d", ST(L49_KALHI_WALK_STEP));
    CB->print_msg(g_txt, 0, 0xF0);
    i32 t1 = CB->on_joindre(P_KALHI13);
    snprintf(g_txt, sizeof(g_txt), "onjoin:%d", t1);
    CB->print_msg(g_txt, 0, 0xF8);
    snprintf(g_txt, sizeof(g_txt), "test11:%d", ST(V_EVENT15_STEP));
    CB->print_msg(g_txt, 0, 0x100);
    snprintf(g_txt, sizeof(g_txt), "G79:%d", ST(L49_AMBUSH_PHASE));
    CB->print_msg(g_txt, 0, 0x108);
    /* 0x8012 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        CB->play_anim(0, 0);
        CB->evenement(1, 2000);
    }
    /* 0x804D */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x806E */
        if (ST(V_BUNKER_SEEN) == 1) {
            /* 0x8080 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("Sar_50");
                CB->dialogue("L49D38", P_ARKHAN2);
            }
            /* 0x80AD */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("Sar_50");
                CB->dialogue("L49D39", P_ARKHAN2);
            }
        }
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_LEVEL) = 49;
        CB->change_piece(13, 0);
    }
    /* 0x80FF */
    if (ST(V_EVENT_34) == 1 && ST(ST_VIE(P_MALROT8)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        ST(V_EVENT_34) = 0;
        CB->dialogue("L49D02B", P_MALROT8);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x8170 */
    if (CB->distance(1, 2) < 500 && ST(L49_KALHI_STORY_STAGE) == 2 && ST(L49_MALROT8_FIGHT) == 0) {
        CB->select_pile(1, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers(DV_CINEMATIC, 0, 0);
        CB->animation("SP13ARR01_10");
        ST(L49_MALROT8_FIGHT) = 1;
        /* 0x81F0 */
        if (!CB->divers(DV_WAS_HIT, P_MALROT8, 0)) {
            CB->dialogue("L49D40", P_MALROT8);
            CB->combat(P_MALROT8, 0);
            ST(V_ENGAGED_PERSONA) = P_MALROT8;
        }
        /* 0x822B */
        if (ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            CB->animation("SP13GAR02_10");
            CB->divers(DV_CINEMATIC, 0, 0);
            CB->joindre(-P_KALHI13, 2779, 4202, 0x279F, -1);
            CB->animation("SP13ARR01_10");
            CB->modify_var(&ST(L49_KALHI_JOINED), 1);
            CB->animation("P13GAR02");
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->animation("SP13GAR02_10");
            CB->modify_var(&ST(V_EVENT15_STEP), 1);
            CB->select_pile(0, 1);
        }
        CB->select_pile(0, 1);
    }
    /* 0x830A */
    if (CB->divers(DV_WAS_HIT, P_DHORKAN11, 0) && ST(ST_VIE(P_DHORKAN11)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_DHORKAN11, 1);
        CB->combat(P_DHORKAN11, -1);
        /* 0x835D */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_DHORKAN11;
        }
        ST(L49_DHORKAN_STAGE) = 1;
        /* 0x839E */
        if ((CB->divers(DV_HP, P_DHORKAN11, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_DHORKAN11, (CB->divers(DV_HP, P_DHORKAN11, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_DHORKAN11, 1);
        }
        /* 0x83FF */
        if (CB->divers(DV_HP, P_DHORKAN11, -1) <= 1) {
            CB->stop_joindre(-P_DHORKAN11);
            ST(ST_VIE(P_DHORKAN11)) = VIE_DEAD;
            CB->select_pile(2, 2);
            CB->divers_empile(DV_BLOOD_AT_BONE, 0xB, 0);
            CB->select_pile(1, 1);
            CB->animation("SP11IMP52");
            CB->animation("P11CAD");
            CB->pos_perso(P_DHORKAN11, 0, 0, 0, 2, "P11CAD", 4, 0);
        } else {
            CB->stop_joindre(-P_DHORKAN11);
            CB->divers_empile(DV_BLOOD_AT_BONE, 0xB, 0);
            CB->animation("SP11IMP12");
            CB->animation("P11IMP47");
            CB->animation("P11GAR02");
            CB->combat(P_DHORKAN11, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x84F4 */
    if (CB->divers(DV_WAS_HIT, P_MALROT8, 0) && ST(ST_VIE(P_MALROT8)) != VIE_DEAD && ST(L49_MALROT8_FIGHT) == 1) {
        CB->select_pile(1, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_MALROT8, 1);
        CB->combat(P_MALROT8, -1);
        /* 0x857C */
        if ((CB->divers(DV_HP, P_MALROT8, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_MALROT8, (CB->divers(DV_HP, P_MALROT8, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_MALROT8, 1);
        }
        /* 0x85DD */
        if (CB->divers(DV_HP, P_MALROT8, -1) <= 1) {
            ST(ST_VIE(P_MALROT8)) = VIE_DEAD;
            /* 0x85FF */
            if (CB->on_joindre(P_MALROT8)) {
                CB->stop_joindre(-P_MALROT8);
            }
            CB->animation("SP68IMP52");
            CB->animation("P68CAD");
            CB->pos_perso(P_MALROT8, 0, 0, 0, 2, "P68CAD", 2, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            /* 0x8666 */
            if (CB->on_joindre(P_MALROT8)) {
                CB->stop_joindre(-P_MALROT8);
            }
            CB->divers_empile(DV_BLOOD_AT_BONE, 0x44, 0);
            CB->animation("SP68IMP12");
            CB->animation("P68IMP47");
            CB->animation("P68GAR02");
            CB->combat(P_MALROT8, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x86D8 */
    if (CB->divers(DV_WAS_HIT, P_MALROT8, 0) && ST(ST_VIE(P_MALROT8)) != VIE_DEAD && ST(L49_MALROT8_FIGHT) == 0) {
        CB->select_pile(1, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        ST(L49_MALROT8_FIGHT) = 1;
        CB->dialogue("L49D40", P_MALROT8);
        CB->combat(P_MALROT8, 0);
        ST(V_ENGAGED_PERSONA) = P_MALROT8;
        /* 0x875F */
        if (ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            /* 0x8781 */
            if (ST(ST_VIE(P_MALROT8)) == VIE_DEAD) {
                CB->bloque(BLOQUE_ALL);
            }
            CB->animation("SP13GAR02_10");
            CB->divers(DV_CINEMATIC, 0, 0);
            CB->joindre(-P_KALHI13, 2779, 4202, 0x279F, -1);
            CB->animation("SP13ARR01_10");
            CB->modify_var(&ST(L49_KALHI_JOINED), 1);
            CB->animation("P13GAR02");
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->animation("SP13GAR02_10");
            CB->modify_var(&ST(V_EVENT15_STEP), 1);
            CB->select_pile(0, 1);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8853 */
    if (ST(V_EVENT15_STEP) == 1 && CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_KALHI13);
        CB->divers_empile(DV_BLOOD_AT_BONE, 0xD, 0);
        CB->animation("SP13IMP52");
        CB->animation("P13CAD");
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13CAD", 3, 0);
        ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
        ST(L49_KALHI_HIT_STAGE) = 3;
        CB->bloque(BLOQUE_NONE);
    } else if (ST(V_EVENT15_STEP) != 1 && CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
        CB->divers_empile(DV_WAS_HIT, P_KALHI13, 1);
    }
    /* 0x8966 */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(L49_KALHI_STORY_STAGE) == 2) {
        ST(L49_KALHI_STORY_STAGE) = 10;
        /* 0x8999 */
        if (ST(L49_AMBUSH_PHASE) == 0 && ST(V_ENGAGED_PERSONA) != P_MALROT8 && ST(ST_VIE(P_MALROT8)) != VIE_DEAD) {
            ST(V_ENGAGED_PERSONA) = P_MALROT8;
            CB->animation("SP68GAR02_10");
            ST(L49_AMBUSH_PHASE) = 1;
            CB->combat(P_MALROT8, 0);
        }
    }
    /* 0x89FF */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_VIE(P_DHORKAN11)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(L49_DHORKAN_STAGE) == 0 && CB->distance(1, 4) < 600 && ST(L49_DHORKAN_INTRO_DONE) == 0) {
        CB->change_camera(5, CAM_LOCKED);
        CB->bloque(BLOQUE_ALL);
        /* 0x8A86 */
        if (ST(ST_COMBAT_STANCE) == 1 && ST(L49_STANCE_SAVED) == 0) {
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            ST(L49_STANCE_SAVED) = 1;
        }
        CB->joindre(ST(ST_HERO), 0, 0, 0, 0xB);
        ST(L49_DHORKAN_INTRO_DONE) = 1;
        CB->animation("SP11ATT05");
        CB->animation("P11ARR02");
        CB->joindre(P_DHORKAN11, 0, 0, 0, ST(ST_HERO));
        /* 0x8B20 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L49D07", P_ARKHAN2, "SG02DIA05", NULL);
        }
        /* 0x8B48 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L49D08", P_ARKHAN3, "SG03DIA05", NULL);
        }
        lieu_action_register("L49D09", P_DHORKAN11, "SP11DIA54", "SP11ARR02_10");
        CB->divers_empile(DV_RANDOM, 0, 0);
        CB->change_camera(0, CAM_LOCKED);
        /* 0x8BA5 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L49D10", P_ARKHAN2, "SG02DIA08", NULL);
        }
        /* 0x8BCD */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L49D11", P_ARKHAN3, "SG03DIA08", NULL);
        }
        CB->divers_empile(DV_RANDOM, 0, 0);
        CB->change_camera(5, CAM_LOCKED);
        lieu_action_register("L49D12A", P_DHORKAN11, "SP11DIA54", "SP11ARR02_10");
        CB->animation("Sar_10");
        lieu_action_register("L49D12B", P_DHORKAN11, "SP11DIA54", "SP11GAR26");
        CB->animation("P11GAR02_10");
        CB->modify_var(&ST(L49_DHORKAN_STAGE), 1);
        CB->combat(P_DHORKAN11, 0);
        ST(V_ENGAGED_PERSONA) = P_DHORKAN11;
        /* 0x8C8C */
        if (ST(L49_STANCE_SAVED) == 1) {
            CB->divers_empile(DV_COMBAT_STANCE, 1, 0);
        }
        CB->bloque(BLOQUE_NONE);
        CB->animation("Sar");
        CB->change_camera(0, CAM_LOCKED);
        CB->change_camera(0, CAM_AUTO);
    }
    /* 0x8CDD */
    if (ST(ST_VIE(P_MALROT8)) == VIE_DEAD && ST(L49_KALHI_LEFT) == 0 && ST(V_GUARD_FIGHT_DONE) != 2) {
        ST(V_ENGAGED_PERSONA) = 0;
        /* 0x8D1F */
        if (ST(L49_MALROT8_DEAD_HOLD) == 0 && ST(ST_VIE(P_MALROT8)) == VIE_DEAD && ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
            ST(L49_MALROT8_DEAD_HOLD) = 1;
            CB->bloque(BLOQUE_ALL);
        }
        /* 0x8D6F */
        if (ST(ST_VIE(P_KALHI13)) != VIE_DEAD && ST(L49_KALHI_JOINED) == 1) {
            /* 0x8D90 */
            if (ST(L49_KALHI_GIFT_STEP) == 0 && ST(L49_KALHI_REGROUP_STARTED) == 0) {
                CB->divers(DV_COMBAT_STANCE, 0, 0);
                CB->bloque(BLOQUE_ALL);
                ST(L49_KALHI_REGROUP_STARTED) = 1;
                CB->joindre(ST(ST_HERO), 2762, 4295, 0x85B1, -1);
                CB->stop_joindre(-P_KALHI13);
                CB->animation("SP13GAR27");
                CB->animation("P13ARR01_50");
                CB->modify_var(&ST(L49_KALHI_GIFT_STEP), 1);
                CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                CB->select_pile(0, 1);
                return;
            }
            /* 0x8E59 */
            if (ST(L49_KALHI_GIFT_STEP) == 1 && CB->distance(1, 3) < 200 && !CB->on_joindre(ST(ST_HERO))) {
                ST(L49_KALHI_GIFT_STEP) = 1000;
                CB->stop_joindre(-P_KALHI13);
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                lieu_action_register("L49D14", P_KALHI13, "SP13DIA57", "SP13ARR01");
                ST(V_EVENT15_STEP) = 2;
                /* 0x8EFB */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->joindre(-ST(ST_HERO), 0, 0, 0, 0xD);
                    CB->animation("Sar_10");
                    /* 0x8F35 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L49D15", P_ARKHAN2, "SG02DIA05", NULL);
                    }
                    /* 0x8F5D */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L49D16", P_ARKHAN2, "SG02DIA05", NULL);
                    }
                    /* 0x8F85 */
                    if (ST(ST_LOCKED_CAMERA) == 2) {
                        CB->change_camera(0, CAM_LOCKED);
                    }
                }
                /* 0x8FA4 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->joindre(-ST(ST_HERO), 0, 0, 0, 0xD);
                    CB->animation("Sar_10");
                    /* 0x8FDE */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L49D17", P_ARKHAN3, "SG03DIA05", NULL);
                    }
                    /* 0x9006 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L49D18", P_ARKHAN3, "SG03DIA05", NULL);
                    }
                    /* 0x902E */
                    if (ST(ST_LOCKED_CAMERA) == 2) {
                        CB->change_camera(0, CAM_LOCKED);
                    }
                }
                lieu_action_register("L49D19A", P_KALHI13, "SP13DIA57", "SP13ARR01");
                lieu_action_register("L49D19B", P_KALHI13, "SP13DIA57", "SP13ARR01");
                /* 0x907F */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L49D20", P_ARKHAN2, "SG02DIA05", NULL);
                }
                /* 0x90A7 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L49D21", P_ARKHAN3, "SG03DIA05", NULL);
                }
                lieu_action_register("L49D22A", P_KALHI13, "SP13DIA57", "SP13ARR01");
                lieu_action_register("L49D22B", P_KALHI13, "SP13DIA57", "SP13ARR01");
                /* 0x9101 */
                if (ST(ST_PLAYER_WEAPON) != 0) {
                    /* 0x9113 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02PRE73");
                        CB->animation("ar_50");
                    }
                    /* 0x913B */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03PRE73");
                        CB->animation("ar_50");
                    }
                }
                CB->animation("Sar_50");
                CB->link_objet(0xD, "OB32", 0x1B, 1);
                CB->animation("Sar_50");
                /* 0x918D */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SG02PRE80");
                    CB->animation("G02PRE81");
                    CB->animation("SP13PRE60");
                    CB->animation("P13_65535");
                    CB->animation("SG02PRE82");
                }
                /* 0x91D6 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SG03PRE80");
                    CB->animation("G03PRE81");
                    CB->animation("SP13PRE60");
                    CB->animation("P13_65535");
                    CB->animation("SG03PRE82");
                }
                CB->animation("ar_10");
                CB->animation("SP13arr01_10");
                /* 0x9235 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02PRE73");
                    CB->animation("ar_50");
                    CB->divers_empile(DV_GIVE_ITEM, OB_SM_THBOLT, -1);
                }
                /* 0x926F */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03PRE73");
                    CB->divers(DV_GIVE_ITEM, OB_SM_THBOLT, -1);
                    CB->animation("ar_50");
                }
                /* 0x92A9 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    /* 0x92BB */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L49D23A", P_ARKHAN2, "SG02DIA05", NULL);
                        lieu_action_register("L49D23B", P_ARKHAN2, "SG02DIA05", NULL);
                    }
                    /* 0x92F9 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L49D24A", P_ARKHAN2, "SG02DIA05", NULL);
                        lieu_action_register("L49D24B", P_ARKHAN2, "SG02DIA05", NULL);
                    }
                }
                /* 0x9337 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    /* 0x9349 */
                    if (ST(ST_SPECIAL_MODE) == 1) {
                        lieu_action_register("L49D25", P_ARKHAN3, "SG03DIA05", NULL);
                    }
                    /* 0x9371 */
                    if (ST(ST_SPECIAL_MODE) == 0) {
                        lieu_action_register("L49D26", P_ARKHAN3, "SG03DIA05", NULL);
                    }
                }
                /* 0x9399 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    lieu_action_register("L49D27", P_KALHI13, "SP13DIA57", "SP13ARR01_10");
                }
                /* 0x93C4 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    lieu_action_register("L49D28", P_KALHI13, "SP13DIA57", "SP13ARR01_10");
                }
                CB->animation("Sar_10");
                CB->modify_var(&ST(L49_KALHI_GIFT_STEP), 2);
                CB->change_camera(0, CAM_AUTO);
                return;
            }
            /* 0x941D */
            if (ST(L49_KALHI_GIFT_STEP) == 2) {
                ST(L49_KALHI_GIFT_STEP) = 1000;
                CB->select_pile(1, 1);
                CB->evenement(0x21, 6500);
                CB->joindre(P_KALHI13, 2477, 3734, -1, -1);
                CB->modify_var(&ST(L49_KALHI_WALK_STEP), 1);
                CB->select_pile(0, 1);
            }
        }
        /* 0x9492 */
        if (ST(L49_KALHI_WALK_STEP) == 1 && !CB->on_joindre(P_KALHI13)) {
            CB->select_pile(1, 1);
            ST(L49_KALHI_STORY_STAGE) = 3;
            CB->animation("Sar_10");
            CB->joindre(P_KALHI13, 2703, 3336, 0xB27B, -1);
            CB->modify_var(&ST(L49_KALHI_WALK_STEP), 2);
            CB->select_pile(0, 1);
        }
        /* 0x951B */
        if (ST(L49_KALHI_WALK_STEP) == 2 && !CB->on_joindre(P_KALHI13)) {
            CB->select_pile(1, 1);
            CB->joindre(P_KALHI13, 2819, 3340, -1, -1);
            CB->modify_var(&ST(L49_KALHI_WALK_STEP), 3);
            CB->select_pile(0, 1);
        }
        /* 0x9587 */
        if (ST(L49_KALHI_WALK_STEP) == 3 && !CB->on_joindre(P_KALHI13)) {
            CB->select_pile(1, 1);
            CB->pos_perso(P_KALHI13, 0, 0, 0, 4, "", -1, 0);
            CB->modify_var(&ST(ST_VIE(P_KALHI13)), 1);
            ST(L49_KALHI_LEFT) = 1;
            ST(L49_KALHI_STORY_STAGE) = 3;
            CB->select_pile(0, 1);
        }
        /* 0x961A */
        if (ST(V_EVENT_33) == 1) {
            ST(V_EVENT_33) = 0;
            CB->select_pile(0, 1);
            CB->animation("Sar_10");
            CB->etat_objet("OB32", -1, -1, -1, -1, -1, -1, -3, -1);
            CB->animation("Sar_10");
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0x9684 */
    if (ST(ST_VIE(P_DHORKAN11)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_DHORKAN11) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(V_ENGAGED_PERSONA) = 0;
        CB->evenement(0x24, 2000);
    }
    /* 0x96DC */
    if (ST(V_EVENT_36) == 1) {
        ST(V_EVENT_36) = 0;
        CB->combat(P_DHORKAN11, -1);
        /* 0x970D */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L49D42", P_ARKHAN2);
        }
        /* 0x972F */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L49D43", P_ARKHAN3);
        }
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        ST(L49_DHORKAN_STAGE) = 2;
    }
    /* 0x9775 */
    if (ST(L49_KALHI_HIT_STAGE) == 3 && ST(ST_VIE(P_MALROT8)) != VIE_DEAD && ST(L49_MALROT8_TAUNT_SAID) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(L49_MALROT8_TAUNT_SAID) = 1;
        ST(L49_AMBUSH_PHASE) = 2;
        ST(L49_KALHI_STORY_STAGE) = 20;
        CB->dialogue("L49D41A", P_MALROT8);
        CB->dialogue("L49D41B", P_MALROT8);
        ST(L49_KALHI_HIT_STAGE) = 4;
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x9819 */
    if (ST(ST_HERO) == 4 && ST(L49_GAMEOVER_SENT) == 0) {
        ST(L49_GAMEOVER_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x9856 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x987C */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x98A5 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x98D1 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x98FD */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x9921 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x993E */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000C3A9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001BCBC = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu49_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000C5D2);
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
        l49_trig00();   /* 0x1000C5D9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000C4A6 — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000C62C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000C010).
 * Jump table @ 0x1000C6A9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu49_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu49: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        g_txt[0] = '\0';   /* .data globals restart on every room load */
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l49_trigger(arg);             break;  /* dword_1001BB50[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
