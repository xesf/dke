/*
 * lieu15.c — Native hand-port of LIEU15.DLL (level-15 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu15_trig.c (reached through
 * l15_trigger, see lieu15_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x10019E50, the state pointer global is dword_1002EBAC and the
 * trigger table is dword_1002EAC0[42].
 *
 * event4 (sub_1000124A), event5 (sub_100022C4), event6 (sub_10005BFC),
 * lieu_action_register (sub_1001A51C), lieu_pre_scene (sub_1001A6CC) and
 * sub_10001210 (float reset) are the shared lieu_common.c; the entry and
 * INIT are the same as LIEU03. tick_body (sub_1000959A) and sub_10007400
 * are level-15 specific.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu15_int.h"

/* Global sprintf buffer of the debug overlay (dword_1002E8C0, starts ""). */
c8 g_l15_txt[256];

/* .data globals of the DLL (all start at 0 on every visit). */
i32 g_dat_26B34;
i32 g_dat_26B38;
i32 g_dat_26B3C;

/* The DLL is loaded afresh for every room: its globals restart. Called by
 * INIT, and by tools/lieu_difftest_harness.c before every case. */
void l15_reset_dll_globals(void) {
    g_dat_26B34 = g_dat_26B38 = g_dat_26B3C = 0;
    g_l15_txt[0] = '\0';
}

/* -------------------------------------------------------------------------
 * sub_10007400 (thunk 0x1000109B) — Danrys' pre-scene variant, called
 * as (hero, 9, item, 6, 2); only hero (ecx) and persona (edx) are used,
 * the three stack arguments are ignored.
 * ---------------------------------------------------------------------- */
void l15_pre_scene2(i32 hero, i32 n, i32 a1, i32 a2, i32 a3) {
    c8 buf[256];
    (void)a1; (void)a2; (void)a3;
    /* 0x741D */
    if (ST(ST_PLAYER_WEAPON) != 0 && ST(V_PRE90_OBJECT) != 0) {
        CB->animation("SP09PRE73");
    }
    snprintf(buf, sizeof(buf), "SP%02dPRE60", n);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "P%02d_65535", n);
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "SG%02dPRE82", hero);
    CB->animation(buf);
    /* 0x74BC */
    if (ST(ST_PLAYER_WEAPON) != 0) {
        snprintf(buf, sizeof(buf), "G%02darr02_10", hero);
    } else {
        snprintf(buf, sizeof(buf), "G%02darr01_10", hero);
    }
    CB->animation(buf);
    snprintf(buf, sizeof(buf), "SP%02darr01_10", n);
    CB->animation(buf);
}

/* -------------------------------------------------------------------------
 * Per-frame body — sub_1000959A (thunk 0x100010BE), level-15 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x95AC */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_DEATH_EVENT_SENT) = 1;
        /* 0x95EF */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x9618 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x9644 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x9670 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x9694 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x96B1 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x96DD */
    if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) == P_DANRYS || ST(V_ENGAGED_PERSONA) == 0x391) && ST(L15_DANRYS_DEAD_SEEN) == 0) {
        ST(L15_DANRYS_DEAD_SEEN) = 1;
        ST(V_EVENT_30) = 1;
        CB->evenement(30, 300000);
    }
    /* 0x9750 */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) == P_KALHI13 || ST(V_ENGAGED_PERSONA) == 0x391) && ST(L15_KALHI_DEAD_HANDLED) == 0) {
        ST(L15_PROGRESS_STEP) = 10;
        ST(L15_KALHI_DEAD_HANDLED) = 1;
    }
    /* 0x97B3 */
    if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0x391) {
        ST(V_ENGAGED_PERSONA) = P_KALHI13;
    }
    /* 0x97E3 */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0x391) {
        ST(V_ENGAGED_PERSONA) = P_DANRYS;
        ST(L15_PROGRESS_STEP) = 10;
    }
    /* 0x9822 */
    if ((ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_KALHI13) || (ST(ST_VIE(P_DANRYS)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_DANRYS)) {
        ST(ST_SCRIPT_BUSY) = 0;
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x9885 */
    if (CB->distance(6, 10) < -200 && ST(L15_KONKALITE9_STATE) == 1) {
        ST(L15_KONKALITE9_STATE) = 2;
        ST(V_ENGAGED_PERSONA) = P_KONKALITE9;
        CB->combat(P_KONKALITE9, 0);
    }
    /* 0x98D3 */
    if (CB->divers(DV_WAS_HIT, P_DANRYS, 0) && ST(ST_VIE(P_DANRYS)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        /* 0x9907 */
        if (ST(L15_DANRYS_INTERRUPTIBLE) == 1) {
            /* 0x9919 */
            if (ST(ST_LOCKED_CAMERA) == 0) {
                CB->pause_anim(3, 5, -1);
                CB->divers_empile(DV_SCREEN, 2, 0);
                CB->load_anim("215MEU01", 0);
                CB->play_anim(0, 1);
                ST(L15_ANIM_PHASE) = 2;
                ST(L15_TRADE_ANIM_CAM) = 0;
            }
            /* 0x9987 */
            if (ST(ST_LOCKED_CAMERA) == 2) {
                CB->divers_empile(DV_SCREEN, 2, 0);
                ST(L15_ANIM_PHASE) = 2;
                ST(L15_TRADE_ANIM_CAM) = 0;
            }
            /* 0x99C9 */
            if (ST(ST_LOCKED_CAMERA) == 3) {
                CB->pause_anim(5, 5, -1);
                CB->divers_empile(DV_SCREEN, 2, 0);
                CB->load_anim("215MEU03", 2);
                CB->play_anim(2, 1);
                ST(L15_ANIM_PHASE) = 2;
                ST(L15_TRADE_ANIM_CAM) = 0;
            }
        }
        ST(ST_SCRIPT_BUSY) = 0;
        ST(L15_DANRYS_DEAD_SEEN) = 1;
        ST(L15_DANRYS_PLOT_A84) = 3;
        ST(ST_VIE(P_DANRYS)) = VIE_DEAD;
        CB->select_pile(2, 2);
        CB->combat(P_DANRYS, -1);
        CB->stop_joindre(-P_DANRYS);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_DANRYS, 0);
        CB->animation("SP09IMP53");
        CB->animation("P09CAD");
        ST(V_EVENT_30) = 1;
        CB->evenement(30, 300000);
        CB->pos_perso(P_DANRYS, 0, 0, 0, 2, "P09CAD", 2, 0);
        /* 0x9B04 */
        if (ST(L15_PROGRESS_STEP) != 2 && ST(L15_PROGRESS_STEP) != 5 && ST(L15_PROGRESS_STEP) != 8 && ST(L15_PROGRESS_STEP) != 20 && ST(L15_DANRYS_KALHI_FOLLOW) == 1 && ST(ST_VIE(P_KALHI13)) == 1 && !CB->divers(DV_WAS_HIT, P_KALHI13, 0)) {
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_KALHI13);
            ST(V_ENGAGED_PERSONA) = P_KALHI13;
            ST(V_KALHI13_TALK_C) = 3;
            lieu_action_register("L15D456", P_KALHI13, "SP13GAR26", "SP13GAR02_10");
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_KALHI13, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x9BFE */
    if (CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
        CB->select_pile(2, 2);
        ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
        ST(L15_KALHI_DEAD_HANDLED) = 1;
        ST(L15_PROGRESS_STEP) = 10;
        CB->combat(P_KALHI13, -1);
        CB->stop_joindre(-P_KALHI13);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_KALHI13, 0);
        CB->animation("SP13IMP52");
        CB->animation("P13CAD");
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13CAD", 0xC, 0);
        /* 0x9CBE */
        if (ST(ST_VIE(P_DANRYS)) == 1) {
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_DANRYS);
            /* 0x9CE6 */
            if (ST(L15_DANRYS_INTERRUPTIBLE) == 1) {
                ST(L15_ANIM_PHASE) = 2;
                ST(L15_TRADE_ANIM_CAM) = 0;
                CB->animation("SP09ATT08");
                CB->link_objet(9, "OB98", 0x1C, 0);
                /* 0x9D35 */
                if (ST(ST_LOCKED_CAMERA) == 0) {
                    CB->pause_anim(3, 5, -1);
                    CB->divers_empile(DV_SCREEN, 2, 0);
                    CB->load_anim("215MEU01", 0);
                    CB->play_anim(0, 1);
                    ST(L15_ANIM_PHASE) = 2;
                    ST(L15_TRADE_ANIM_CAM) = 0;
                }
                /* 0x9DA3 */
                if (ST(ST_LOCKED_CAMERA) == 2) {
                    CB->divers_empile(DV_SCREEN, 2, 0);
                    ST(L15_ANIM_PHASE) = 2;
                    ST(L15_TRADE_ANIM_CAM) = 0;
                }
                /* 0x9DE5 */
                if (ST(ST_LOCKED_CAMERA) == 3) {
                    CB->pause_anim(5, 5, -1);
                    CB->divers_empile(DV_SCREEN, 2, 0);
                    CB->load_anim("215MEU03", 2);
                    CB->play_anim(2, 1);
                    ST(L15_ANIM_PHASE) = 2;
                    ST(L15_TRADE_ANIM_CAM) = 0;
                }
            }
            /* 0x9E56 */
            if (ST(L15_DANRYS_SCENE_STATE) == 0) {
                ST(L15_DANRYS_SCENE_STATE) = 1;
            }
            ST(L15_DANRYS_PLOT_A84) = 3;
            ST(V_ENGAGED_PERSONA) = P_DANRYS;
            CB->dialogue("L15D76C", P_DANRYS);
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_DANRYS, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x9EC9 */
    if (CB->divers(DV_WAS_HIT, P_KONKALITE9, 0) && ST(ST_VIE(P_KONKALITE9)) != VIE_DEAD && ST(L15_KONKALITE9_ENGAGED) == 0) {
        CB->divers(DV_WAS_HIT, P_KONKALITE9, 1);
    }
    /* 0x9F19 */
    if (CB->divers(DV_WAS_HIT, P_KONKALITE9, 0) && ST(ST_VIE(P_KONKALITE9)) != VIE_DEAD && ST(L15_KONKALITE9_ENGAGED) == 1 && ST(L15_KONKALITE9_STATE) == 2) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_KONKALITE9, 1);
        CB->combat(P_KONKALITE9, -1);
        ST(V_DLG_RANDOM) = CB->divers(DV_HP, P_KONKALITE9, -1);
        /* 0x9FBB */
        if ((ST(V_DLG_RANDOM) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_KONKALITE9, (ST(V_DLG_RANDOM) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_KONKALITE9, 1);
        }
        /* 0xA015 */
        if (CB->divers(DV_HP, P_KONKALITE9, -1) <= 1) {
            ST(ST_VIE(P_KONKALITE9)) = VIE_DEAD;
            CB->stop_joindre(-P_KONKALITE9);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_KONKALITE9, 0);
            CB->animation("SP79IMP53");
            CB->animation("P79CAD");
            CB->pos_perso(P_KONKALITE9, 0, 0, 0, 2, "P79CAD", 0xA, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_KONKALITE9);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_KONKALITE9, 0);
            CB->animation("SP79IMP13");
            CB->animation("P79IMP48");
            CB->animation("P79GAR03");
            CB->combat(P_KONKALITE9, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0xA0F1 */
    if (ST(V_ENGAGED_PERSONA) == P_KONKALITE9 && ST(ST_VIE(P_KONKALITE9)) == VIE_DEAD) {
        ST(L15_KONKALITE9_ENGAGED) = 2;
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0xA130 */
    if (ST(L15_KALHI_FOLLOW_STEP) == 2 && !CB->on_joindre(P_KALHI13)) {
        ST(L15_KALHI_FOLLOW_STEP) = 3;
        CB->stop_joindre(-P_KALHI13);
        ST(V_DLG_RANDOM) = CB->random(2);
        CB->bloque(BLOQUE_ALL);
        /* 0xA191 */
        if (ST(V_DLG_RANDOM) == 0) {
            lieu_action_register("L15D135A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
        }
        /* 0xA1BC */
        if (ST(V_DLG_RANDOM) == 1) {
            lieu_action_register("L15D135B", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        }
        CB->bloque(BLOQUE_NONE);
        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
    }
    /* 0xA205 */
    if (ST(ST_VIE(P_DANRYS)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(L15_DANRYS_SCENE_STATE) == 0) {
        /* 0xA235 */
        if (ST(L15_ROOM_PART) == 0 && ST(L15_DANRYS_CALL_DONE) == 0 && ST(L15_DANRYS_APPROACHED) == 0 && CB->distance(2, 6) < 220) {
            ST(L15_DANRYS_SCENE_STATE) = 1;
        }
        /* 0xA292 */
        if (ST(L15_ROOM_PART) == 1 && ST(L15_DANRYS_CALL_DONE) == 0 && ST(L15_ANIM_PHASE) != 0 && ST(ST_LOCKED_CAMERA) == 1) {
            ST(L15_DANRYS_SCENE_STATE) = 1;
        }
    }
    /* 0xA2E6 */
    if (ST(ST_VIE(P_DANRYS)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(L15_DANRYS_SCENE_STATE) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L15_DANRYS_INTRO_STAGE) == 1 && ST(L15_PROGRESS_STEP) == 0) {
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_DANRYS_SCENE_STATE) = 2;
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xA378 */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            ST(L15_STANCE_LOWERED) = 1;
        }
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        CB->animation("SP09ATT02");
        CB->animation("P09_65535");
        CB->joindre(P_DANRYS, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L15D02A", P_DANRYS, "P09DIA01", "SP09ARR01_10");
        /* 0xA40E */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D137A", P_ARKHAN2, "SG00DIA04", NULL);
        }
        /* 0xA436 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D137B", P_ARKHAN3, "SG00DIA04", NULL);
        }
        lieu_action_register("L15D02Z", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
        /* 0xA477 */
        if (ST(V_DLG_RANDOM) == 1 && ST(L15_GREETING_RANDOM_OK) == 1 && ST(L15_STANCE_LOWERED) == 0) {
            CB->animation("P09ATT09");
            CB->animation("P09ARR01_10");
        }
        CB->animation("Sar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        /* 0xA4DE */
        if (ST(L15_STANCE_LOWERED) == 1) {
            CB->animation("Sar_10");
            CB->divers_empile(DV_COMBAT_STANCE, 1, 0);
            ST(L15_STANCE_LOWERED) = 2;
        }
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
        CB->evenement(33, 1000);
        return;
    }
    /* 0xA544 */
    if (ST(ST_VIE(P_DANRYS)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(L15_DANRYS_SCENE_STATE) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L15_DANRYS_INTRO_STAGE) == 2) {
        ST(L15_DANRYS_SCENE_STATE) = 2;
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0xA5B5 */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            ST(L15_STANCE_LOWERED) = 1;
        }
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        /* 0xA5FC */
        if (ST(L15_STANCE_LOWERED) == 1) {
            CB->animation("Sar_10");
            CB->divers_empile(DV_COMBAT_STANCE, 1, 0);
            ST(L15_STANCE_LOWERED) = 2;
        }
        /* 0xA637 */
        if (ST(L15_ROOM_PART) == 0 && ST(L15_DANRYS_WARNED) == 0) {
            /* 0xA655 */
            if (ST(L15_DANRYS_FIRST_TALK) == 0) {
                /* 0xA667 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->dialogue("L15D01A", P_ARKHAN2);
                }
                /* 0xA689 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->dialogue("L15D01B", P_ARKHAN3);
                }
                /* 0xA6A8 */
                if (ST(L15_DANRYS_GREET_PICK) == 0) {
                    CB->animation("SP09ATT02");
                    CB->animation("P09_65535");
                }
                CB->animation("SP09ARR01_10");
                /* 0xA6DB */
                if (ST(V_DLG_RANDOM) == 1 && ST(L15_GREETING_RANDOM_OK) == 1 && ST(L15_STANCE_LOWERED) == 0) {
                    CB->animation("P09ATT09");
                }
            }
            /* 0xA713 */
            if (ST(L15_DANRYS_FIRST_TALK) == 0) {
                ST(L15_DANRYS_FIRST_TALK) = 1;
                CB->dialogue("L15D03A", P_DANRYS);
                CB->animation("Sar_10");
                CB->modify_var(&ST(L15_DANRYS_SCENE_STATE), 3);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("SP09ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                CB->evenement(33, 1000);
                return;
            }
            /* 0xA7AC */
            if (ST(L15_DANRYS_FIRST_TALK) == 1) {
                lieu_action_register("L15D03B", P_DANRYS, "P09DIA56", "SP09ARR01_10");
                /* 0xA7D4 */
                if (ST(ST_COMBAT_STANCE) == 0 && ST(L15_STANCE_LOWERED) == 0) {
                    CB->animation("Sar_10");
                }
                CB->modify_var(&ST(L15_DANRYS_SCENE_STATE), 3);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                CB->evenement(33, 1000);
                return;
            }
        }
        /* 0xA858 */
        if (ST(L15_ROOM_PART) == 1) {
            CB->play_anim(4, 1);
            /* 0xA87A */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D01C", P_ARKHAN2, "SG00DIA08", NULL);
            }
            /* 0xA8A2 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D01D", P_ARKHAN3, "SG00DIA08", NULL);
            }
            CB->evenement(29, 500);
        }
    }
    /* 0xA8DA */
    if (ST(V_EVENT_29) == 1) {
        ST(V_EVENT_29) = 2;
        CB->pause_anim(4, 5, -1);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->load_anim("215MEU02", 1);
        CB->play_anim(1, 1);
        CB->animation("SP09ATT08_1000");
        CB->link_objet(9, "OB98", 0x1C, 0);
        CB->evenement(29, 500);
    }
    /* 0xA96E */
    if (ST(V_EVENT_29) == 3) {
        ST(V_EVENT_29) = 4;
        /* 0xA98C */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->bloque(BLOQUE_ALL);
        }
        CB->vire_col(8, 0);
        CB->vire_col(10, 0);
        CB->joindre(-P_DANRYS, 0, 0, 0, ST(ST_HERO));
        CB->animation("P09DIA02");
        lieu_action_register("L15D97A", P_DANRYS, "P09DIA03", "P09ARR01_10");
        lieu_action_register("L15D97B", P_DANRYS, "P09DIA03", "P09ARR01_10");
        CB->modify_var(&ST(V_EVENT15_STEP), 0);
        CB->modify_var(&ST(L15_ANIM_PHASE), 0);
        /* 0xAA42 */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            ST(L15_STANCE_LOWERED) = 1;
        }
        /* 0xAA80 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < 505) {
            CB->joindre(P_DANRYS, 5240, 590, 0x7918, -1);
            CB->modify_var(&ST(L15_DANRYS_ESCORT_STEP), 1);
        }
        /* 0xAAD6 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > 505) {
            CB->joindre(P_DANRYS, 5194, 150, 0x2904, -1);
            CB->modify_var(&ST(V_EVENT15_STEP), 3);
        }
    }
    /* 0xAB1B */
    if (ST(L15_DANRYS_ESCORT_STEP) == 1 && !CB->on_joindre(P_DANRYS)) {
        ST(L15_DANRYS_ESCORT_STEP) = 2;
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        CB->stop_joindre(-P_DANRYS);
        CB->animation("SP09ARR01_10");
        CB->joindre(-P_DANRYS, 5194, 150, 0x2904, -1);
        CB->modify_var(&ST(V_EVENT15_STEP), 3);
    }
    /* 0xABB5 */
    if (ST(V_EVENT15_STEP) == 3 && !CB->on_joindre(P_DANRYS)) {
        ST(V_EVENT15_STEP) = 4;
        ST(L15_DANRYS_INTERRUPTIBLE) = 0;
        CB->stop_joindre(-P_DANRYS);
        CB->animation("SP09ARR01_10");
        CB->bruitage("L15B01", 5043, 250);
        CB->bruitage("L15AMB1", -1, -1);
        /* 0xAC35 */
        if (ST(ST_LOCKED_CAMERA) == 0) {
            CB->free_anim(0);
        }
        /* 0xAC4F */
        if (ST(ST_LOCKED_CAMERA) == 1) {
            CB->free_anim(1);
        }
        /* 0xAC7D */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < 505) {
            CB->joindre(-ST(ST_HERO), 0, 0, 0x23AA, -1);
        }
        /* 0xACC0 */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > 505) {
            CB->joindre(-ST(ST_HERO), 0, 0, 0x528A, -1);
        }
        /* 0xACEF */
        if (ST(ST_COMBAT_STANCE) == 0 && ST(L15_STANCE_LOWERED) == 0) {
            CB->animation("Sar_10");
        }
        CB->animation("SP09ARR01_10");
        CB->joindre(-P_DANRYS, 5204, 400, 0xBB8, -1);
        CB->modify_var(&ST(V_EVENT15_STEP), 5);
        CB->modify_var(&ST(L15_ANIM_PHASE), 0);
    }
    /* 0xAD6D */
    if (ST(V_EVENT15_STEP) == 5 && !CB->on_joindre(P_DANRYS)) {
        ST(V_EVENT15_STEP) = 6;
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        CB->animation("SP09ARR01_10");
        CB->vire_col(10, 1);
        CB->vire_col(8, 1);
        CB->stop_joindre(-P_DANRYS);
        /* 0xADF1 */
        if (ST(L15_STANCE_LOWERED) == 1) {
            CB->animation("Sar_10");
            CB->divers_empile(DV_COMBAT_STANCE, 1, 0);
            ST(L15_STANCE_LOWERED) = 2;
        }
        /* 0xAE29 */
        if (ST(ST_COMBAT_STANCE) == 0 && ST(L15_STANCE_LOWERED) == 0) {
            /* 0xAE4A */
            if (ST(L15_DANRYS_PLOT_A84) != 3) {
                lieu_action_register("L15D03C", P_DANRYS, "SP09DIA57", "P09ARR01_10");
            }
            /* 0xAE75 */
            if (ST(L15_DANRYS_PLOT_A84) >= 3) {
                ST(L15_DANRYS_PLOT_A84) = 4;
                lieu_action_register("L15D02B", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
            }
            CB->animation("Sar_10");
        }
        CB->bloque(BLOQUE_NONE);
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
        CB->modify_var(&ST(V_EVENT15_STEP), 7);
        CB->modify_var(&ST(L15_DANRYS_SCENE_STATE), 3);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->evenement(33, 1000);
        return;
    }
    /* 0xAF29 */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        ST(L15_STANCE_LOWERED) = 0;
        /* 0xAF53 */
        if (ST(ST_COMBAT_STANCE) == 0) {
            CB->stop_joindre(-P_DANRYS);
        }
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
    }
    /* 0xAF86 */
    if (ST(V_EVENT_1) == 1 && ST(ST_VIE(P_DANRYS)) == 1) {
        ST(V_EVENT_1) = 0;
        ST(L15_DANRYS_SCENE_STATE) = 2;
        ST(L15_ANIM_PHASE) = 0;
        CB->change_camera(1, CAM_LOCKED);
        CB->animation("P09ATT08");
        CB->link_objet(9, "OB98", 0x1C, 0);
        lieu_action_register("L15D98", P_DANRYS, "P09DIA03", "SP09ARR01_10");
        CB->joindre(-P_DANRYS, 5218, 443, 0x6400, -1);
        CB->modify_var(&ST(L15_DANRYS_WALK_STEP), 1);
        return;
    }
    /* 0xB04E */
    if (ST(L15_DANRYS_WALK_STEP) == 1 && !CB->on_joindre(P_DANRYS) && ST(ST_VIE(P_DANRYS)) == 1) {
        ST(L15_DANRYS_WALK_STEP) = 0;
        ST(L15_DANRYS_CALL_DONE) = 1;
        CB->stop_joindre(-P_DANRYS);
        lieu_action_register("L15D99", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
        CB->change_camera(2, CAM_AUTO);
        CB->pause_anim(7, 0, 0);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
        return;
    }
    /* 0xB117 */
    if (ST(L15_TRADE_SCENE_STEP) == 2) {
        ST(L15_TRADE_SCENE_STEP) = 3;
        CB->load_anim("215MEU02", 1);
        CB->play_anim(1, 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 1);
        CB->stop_joindre(-P_DANRYS);
        /* 0xB174 */
        if (ST(L15_TRADE_TYPE) == 1) {
            /* 0xB186 */
            if (ST(V_PRE90_OBJECT) == OB_AX26 || ST(V_PRE90_OBJECT) == OB_AX79 || ST(V_PRE90_OBJECT) == OB_KONK_AX) {
                CB->joindre(-P_DANRYS, 5140, 600, 0x7530, -1);
            } else {
                CB->joindre(-P_DANRYS, 5120, 600, 0x7530, -1);
            }
        } else {
            CB->joindre(-P_DANRYS, 5055, 620, 0x7530, -1);
        }
        CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 4);
    }
    /* 0xB231 */
    if (ST(L15_TRADE_SCENE_STEP) == 4 && !CB->on_joindre(P_DANRYS)) {
        ST(L15_TRADE_SCENE_STEP) = 5;
        CB->stop_joindre(-P_DANRYS);
        /* 0xB26D */
        if (ST(L15_TRADE_TYPE) == 1) {
            CB->animation("SP09PRE73");
            CB->animation("P09ARR01_10");
            snprintf(g_l15_txt, sizeof(g_l15_txt), "OB%d", ST(V_PRE90_OBJECT));
            CB->link_objet(9, g_l15_txt, 0x1B, 1);
        }
        CB->animation("P09ATT07");
        CB->free_anim(1);
        CB->play_anim(4, 1);
        CB->evenement(0, 9000);
    }
    /* 0xB2FA */
    if (ST(V_EVENT_0) == 1 && ST(ST_VIE(P_DANRYS)) == 1) {
        ST(V_EVENT_0) = 0;
        CB->animation("SP09ARR01_10");
        /* 0xB335 */
        if (ST(L15_TRADE_TYPE) == 0) {
            ST(V_CRYSTAL_PROGRESS) = 5;
            /* 0xB353 */
            if (ST(L15_CRYSTAL_GIVEN_A) != 18 && ST(L15_CRYSTAL_GIVEN_B) != 18) {
                CB->link_objet(9, "OB45", 0x1C, 0);
                CB->link_objet(9, "OB18", 0x1B, 1);
            }
            /* 0xB399 */
            if (ST(L15_CRYSTAL_GIVEN_A) != 19 && ST(L15_CRYSTAL_GIVEN_B) != 19) {
                CB->link_objet(9, "OB45", 0x1C, 0);
                CB->link_objet(9, "OB19", 0x1B, 1);
            }
            /* 0xB3DF */
            if (ST(L15_CRYSTAL_GIVEN_A) != 12 && ST(L15_CRYSTAL_GIVEN_B) != 12) {
                CB->link_objet(9, "OB45", 0x1C, 0);
                CB->link_objet(9, "OB12", 0x1B, 1);
            }
        }
        /* 0xB425 */
        if (ST(L15_TRADE_TYPE) == 1) {
            ST(L15_TRADE_TYPE) = 2;
            /* 0xB443 */
            if (ST(L15_TRADE_FINAL) == 1) {
                CB->divers(DV_END_GAME, 9, 0);
            }
        }
        CB->change_camera(3, CAM_AUTO);
        CB->pause_anim(4, 0, -1);
        CB->divers_empile(DV_SCREEN, 2, 0);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 1);
        CB->load_anim("215MEU03", 2);
        CB->pause_anim(7, 5, 0);
        CB->play_anim(2, 1);
        lieu_action_register("L15D101", P_DANRYS, "SP09DIA03", "SP09ARR01_10");
        CB->joindre(-P_DANRYS, 5200, 590, 0x7530, -1);
        CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 6);
    }
    /* 0xB52A */
    if (ST(L15_TRADE_SCENE_STEP) == 6 && !CB->on_joindre(P_DANRYS)) {
        ST(L15_TRADE_SCENE_STEP) = 7;
        ST(L15_DANRYS_INTERRUPTIBLE) = 0;
        CB->stop_joindre(-P_DANRYS);
        /* 0xB578 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SP02ACT20");
        }
        /* 0xB595 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SP03ACT20");
        }
        /* 0xB5B2 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("P02_65535");
        }
        /* 0xB5CF */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("P03_65535");
        }
        CB->free_anim(2);
        CB->pause_anim(8, 5, 0);
        CB->bruitage("L15B01", 5020, 235);
        /* 0xB61E */
        if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
            CB->bruitage("L15AMB1", -1, -1);
        }
        /* 0xB642 */
        if (ST(L15_DANRYS_ENDING_FLAG) != 0) {
            CB->bruitage("L15AMB3", -1, -1);
        }
        /* 0xB663 */
        if (ST(L15_TRADE_TYPE) == 2) {
            CB->animation("SP09PRE73");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(L15_ANIM_PHASE), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB6B8 */
    if (ST(L15_TRADE_SCENE_STEP) == 7 && CB->distance(2, 6) < 150 && ST(ST_COMBAT_STANCE) == 0) {
        /* 0xB6F1 */
        if (ST(L15_TRADE_TYPE) == 2) {
            ST(L15_TRADE_TYPE) = 3;
        }
        ST(L15_TRADE_SCENE_STEP) = 8;
        ST(V_WEAPON_WARNED) = 1;
        CB->stop_joindre(-P_DANRYS);
        CB->bloque(BLOQUE_ALL);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        CB->joindre(-P_DANRYS, 0, 0, 0, ST(ST_HERO));
        CB->divers_empile(DV_PERSONA_ITEMS, P_DANRYS, -OB_CRYST_SLIV);
        /* 0xB78C */
        if (ST(L15_TRADE_TYPE) == 0) {
            ST(V_CRYSTAL_PROGRESS) = 5;
            lieu_action_register("L15D900A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
            lieu_action_register("L15D900B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
            /* 0xB7DC */
            if (ST(L15_CRYSTAL_GIVEN_A) != 18 && ST(L15_CRYSTAL_GIVEN_B) != 18) {
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYSTAL_1, 6, 2);
            }
            /* 0xB815 */
            if (ST(L15_CRYSTAL_GIVEN_A) != 19 && ST(L15_CRYSTAL_GIVEN_B) != 19) {
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYSTAL_3, 6, 2);
            }
            /* 0xB84E */
            if (ST(L15_CRYSTAL_GIVEN_A) != 12 && ST(L15_CRYSTAL_GIVEN_B) != 12) {
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYSTAL_2, 6, 2);
            }
            /* 0xB88A */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("P02PRE73");
            }
            /* 0xB8A7 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("P03PRE73");
            }
            CB->etat_objet("OB45", -1, -1, -1, -1, -1, -1, -2, 0);
            ST(L15_TRADE_REWARD_PENDING) = 0;
        }
        /* 0xB8EE */
        if (ST(L15_TRADE_TYPE) == 3) {
            /* 0xB900 */
            if (ST(ST_PLAYER_WEAPON) != 0) {
                /* 0xB912 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02PRE73");
                }
                /* 0xB92F */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03PRE73");
                }
            }
            CB->animation("SP09PRE73");
            CB->animation("P09ARR01_10");
            snprintf(g_l15_txt, sizeof(g_l15_txt), "OB%d", ST(V_PRE90_OBJECT));
            /* 0xB980 */
            if (ST(L15_TRADE_FINAL) == 0) {
                CB->print_msg(g_l15_txt, 0, 0xF0);
                CB->link_objet(9, g_l15_txt, 0x1B, 1);
                lieu_action_register("L15D900C", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                CB->divers_empile(DV_ITEM_CHARGE, ST(V_LAST_WEAPON), 100);
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, ST(V_LAST_WEAPON), 6, 2);
            }
            /* 0xBA08 */
            if (ST(L15_TRADE_FINAL) == 1) {
                CB->link_objet(9, g_l15_txt, 0x1B, 1);
                lieu_action_register("L15D900D", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, ST(V_LAST_WEAPON), 6, 2);
            }
        }
        CB->bloque(BLOQUE_NONE);
        /* 0xBA71 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SG02ARR02_10");
        }
        /* 0xBA8E */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SG03ARR02_10");
        }
        ST(V_PRE90_OBJECT) = 0;
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 0);
        CB->modify_var(&ST(L15_TRADE_TYPE), 0);
        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
    }
    snprintf(g_l15_txt, sizeof(g_l15_txt), "L15D76:%d", ST(L15_DANRYS_PLOT_A84));
    CB->print_msg(g_l15_txt, 0, 0xF0);
    /* 0xBB47 */
    if ((ST(L15_TRADE_TYPE) == 1 || ST(L15_TRADE_SCENE_STEP) == 7) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 20) && ST(L15_DANRYS_PLOT_A84) != 3) {
        /* 0xBB9A */
        if (ST(L15_TRADE_TYPE) == 1) {
            ST(L15_TRADE_TYPE) = 10;
        }
        /* 0xBBB8 */
        if (ST(L15_TRADE_SCENE_STEP) == 7) {
            ST(L15_TRADE_SCENE_STEP) = 8;
        }
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_DANRYS);
        CB->dialogue("L15D505", P_DANRYS);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 9);
    }
    /* 0xBC33 */
    if (ST(L15_TRADE_SCENE_STEP) == 9) {
        ST(L15_TRADE_SCENE_STEP) = 10;
        CB->joindre(-P_DANRYS, 5273, 740, 0xF230, -10);
        CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 11);
    }
    /* 0xBC87 */
    if (ST(L15_TRADE_SCENE_STEP) == 11 && !CB->on_joindre(P_DANRYS)) {
        /* 0xBCA9 */
        if (ST(L15_TRADE_TYPE) == 2) {
            ST(L15_TRADE_TYPE) = 3;
        }
        ST(L15_TRADE_SCENE_STEP) = 12;
        ST(V_WEAPON_WARNED) = 1;
        CB->stop_joindre(-P_DANRYS);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        CB->joindre(-P_DANRYS, 0, 0, 0, ST(ST_HERO));
        CB->divers_empile(DV_PERSONA_ITEMS, P_DANRYS, -OB_CRYST_SLIV);
        /* 0xBD39 */
        if (ST(L15_TRADE_TYPE) == 0) {
            ST(V_CRYSTAL_PROGRESS) = 5;
            lieu_action_register("L15D900A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
            lieu_action_register("L15D900B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
            /* 0xBD89 */
            if (ST(L15_CRYSTAL_GIVEN_A) != 18 && ST(L15_CRYSTAL_GIVEN_B) != 18) {
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYSTAL_1, 6, 2);
                /* 0xBDC5 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("P02PRE73");
                }
                /* 0xBDE2 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("P03PRE73");
                }
                CB->animation("ar");
            }
            /* 0xBE07 */
            if (ST(L15_CRYSTAL_GIVEN_A) != 19 && ST(L15_CRYSTAL_GIVEN_B) != 19) {
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYSTAL_3, 6, 2);
                /* 0xBE43 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("P02PRE73");
                }
                /* 0xBE60 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("P03PRE73");
                }
                CB->animation("ar");
            }
            /* 0xBE85 */
            if (ST(L15_CRYSTAL_GIVEN_A) != 12 && ST(L15_CRYSTAL_GIVEN_B) != 12) {
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYSTAL_2, 6, 2);
                /* 0xBEC1 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("P02PRE73");
                }
                /* 0xBEDE */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("P03PRE73");
                }
                CB->animation("ar");
            }
        }
        /* 0xBF03 */
        if (ST(L15_TRADE_TYPE) == 10) {
            lieu_action_register("L15D504", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
            /* 0xBF2E */
            if (ST(ST_PLAYER_WEAPON) != 0) {
                /* 0xBF40 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02PRE73");
                }
                /* 0xBF5D */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03PRE73");
                }
            }
            CB->animation("SP09PRE73");
            CB->animation("P09ARR01_10");
            snprintf(g_l15_txt, sizeof(g_l15_txt), "OB%d", ST(V_PRE90_OBJECT));
            CB->link_objet(9, g_l15_txt, 0x1B, 1);
            lieu_pre_scene(ST(ST_HERO), P_DANRYS, ST(V_LAST_WEAPON), 6, 2);
        }
        /* 0xBFE1 */
        if (ST(L15_TRADE_TYPE) == 3) {
            /* 0xBFF3 */
            if (ST(L15_TRADE_FINAL) == 0) {
                CB->divers(DV_ITEM_CHARGE, ST(V_LAST_WEAPON), 100);
                lieu_action_register("L15D900C", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, ST(V_LAST_WEAPON), 6, 2);
            }
            /* 0xC055 */
            if (ST(L15_TRADE_FINAL) == 1) {
                lieu_action_register("L15D900D", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                lieu_pre_scene(ST(ST_HERO), P_DANRYS, ST(V_LAST_WEAPON), 6, 2);
            }
        }
        CB->animation("ar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
        CB->modify_var(&ST(V_PRE90_OBJECT), 0);
        CB->divers_empile(DV_MUSIC, 1, 4);
        /* 0xC0F8 */
        if (ST(L15_ROOM_ENTERED) != 0 && ST(L15_PROGRESS_STEP) == 0) {
            ST(L15_PROGRESS_STEP) = 1;
        }
        ST(ST_LEVEL) = 15;
        CB->change_piece(14, -1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xC154 */
    if (ST(L15_ANIM_PHASE) == 1 && ST(L15_TRADE_ANIM_CAM) == 0) {
        CB->select_pile(2, 1);
        ST(L15_TRADE_ANIM_CAM) = 1;
        ST(L15_SAVED_CAMERA) = ST(ST_LOCKED_CAMERA);
        /* 0xC1AE */
        if (ST(ST_LOCKED_CAMERA) == 0) {
            CB->play_anim(3, 1);
            CB->pause_anim(4, 8, -1);
            CB->pause_anim(5, 8, -1);
        }
        /* 0xC1F4 */
        if (ST(ST_LOCKED_CAMERA) == 1) {
            CB->play_anim(4, 1);
            CB->pause_anim(3, 8, -1);
            CB->pause_anim(5, 8, -1);
        }
        /* 0xC23A */
        if (ST(ST_LOCKED_CAMERA) == 2) {
            CB->pause_anim(3, 8, -1);
            CB->pause_anim(4, 8, -1);
            CB->pause_anim(5, 8, -1);
        }
        /* 0xC282 */
        if (ST(ST_LOCKED_CAMERA) == 3) {
            CB->play_anim(5, 1);
            CB->pause_anim(3, 8, -1);
            CB->pause_anim(4, 8, -1);
        }
        CB->select_pile(0, 1);
    }
    /* 0xC2D5 */
    if (ST(L15_ANIM_PHASE) == 2 && ST(L15_TRADE_ANIM_CAM) == 0) {
        CB->select_pile(2, 1);
        ST(L15_TRADE_ANIM_CAM) = 1;
        ST(L15_SAVED_CAMERA) = ST(ST_LOCKED_CAMERA);
        /* 0xC32F */
        if (ST(ST_LOCKED_CAMERA) == 0) {
            CB->load_anim("215MEU01", 0);
            CB->play_anim(0, 1);
            CB->free_anim(1);
            CB->free_anim(2);
        }
        /* 0xC371 */
        if (ST(ST_LOCKED_CAMERA) == 1) {
            CB->load_anim("215MEU02", 1);
            CB->play_anim(1, 1);
            CB->free_anim(0);
            CB->free_anim(2);
        }
        /* 0xC3B6 */
        if (ST(ST_LOCKED_CAMERA) == 2) {
            CB->free_anim(0);
            CB->free_anim(1);
            CB->free_anim(2);
        }
        /* 0xC3E6 */
        if (ST(ST_LOCKED_CAMERA) == 3) {
            CB->load_anim("215MEU03", 2);
            CB->play_anim(2, 1);
            CB->free_anim(0);
            CB->free_anim(1);
        }
        CB->select_pile(0, 1);
    }
    /* 0xC443 */
    if (ST(L15_SAVED_CAMERA) != ST(ST_LOCKED_CAMERA) && ST(L15_TRADE_ANIM_CAM) == 1) {
        ST(L15_TRADE_ANIM_CAM) = 0;
    }
    /* 0xC476 */
    if (ST(ST_LOCKED_CAMERA) == 3 && ST(L15_CAMERA3_ENTERED) == 0 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->select_pile(2, 1);
        ST(L15_CAMERA3_ENTERED) = 1;
        /* 0xC4C8 */
        if (ST(L15_DANRYS_INTERRUPTIBLE) == 1) {
            CB->pause_anim(7, 5, 0);
        }
        /* 0xC4EC */
        if (ST(L15_DANRYS_INTERRUPTIBLE) == 0) {
            CB->pause_anim(7, 0, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0xC51A */
    if (ST(ST_LOCKED_CAMERA) != 3 && ST(L15_CAMERA3_ENTERED) != 0 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->select_pile(2, 1);
        ST(L15_CAMERA3_ENTERED) = 0;
        CB->pause_anim(8, 0, -1);
        CB->pause_anim(7, 0, -1);
        CB->select_pile(0, 1);
    }
    /* 0xC594 */
    if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_DANRYS_SCENE_STATE) >= 2) {
        /* 0xC5C4 */
        if (ST(L15_DANRYS_WARNED) == 0 && (ST(L15_ROOM_PART) == 0 || ST(V_EVENT15_STEP) == 7) && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_DANRYS) && ST(ST_LOCKED_CAMERA) != 2 && ST(ST_LOCKED_CAMERA) != 3) {
            /* 0xC640 */
            if (ST(L15_ROOM_PART) == 1 && ST(V_EVENT_29) == 0) {
                return;
            }
            /* 0xC669 */
            if (ST(L15_DANRYS_FIGHT_WARN) == 0 && ST(ST_VIE(P_DANRYS)) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
                ST(L15_DANRYS_WARNED) = 1;
                ST(L15_STANCE_LOWERED) = 0;
                ST(L15_DANRYS_FIGHT_WARN) = 1;
                /* 0xC6C3 */
                if (ST(L15_CRYSTAL_ROUND) == 3) {
                    g_dat_26B38 = 1;
                }
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_DANRYS);
                /* 0xC6F5 */
                if (ST(V_PRE90_OBJECT) != 0) {
                    CB->animation("SP09PRE73");
                    CB->animation("P09_65535");
                }
                CB->animation("SP09GAR26");
                CB->animation("P09GAR03_10");
                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                CB->dialogue("L15D04", P_DANRYS);
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L15_DANRYS_WARNED), 2);
                return;
            }
            /* 0xC77A */
            if (ST(L15_DANRYS_FIGHT_WARN) >= 1 && ST(ST_VIE(P_DANRYS)) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
                CB->bloque(BLOQUE_ALL);
                ST(L15_DANRYS_WARNED) = 1;
                CB->stop_joindre(-P_DANRYS);
                /* 0xC7CF */
                if (ST(V_PRE90_OBJECT) != 0) {
                    CB->animation("SP09PRE73");
                    CB->animation("P09_65535");
                }
                CB->animation("SP09GAR26");
                CB->animation("P09GAR03_10");
                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(L15_DANRYS_WARNED), 2);
                return;
            }
        }
        /* 0xC841 */
        if (ST(L15_DANRYS_WARNED) == 2 && ST(ST_COMBAT_STANCE) == 0 && ST(L15_STANCE_LOWERED) == 0) {
            /* 0xC871 */
            if (ST(L15_DANRYS_FIGHT_WARN) == 1) {
                ST(L15_DANRYS_WARNED) = 0;
                ST(L15_DANRYS_FIGHT_WARN) = 2;
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_DANRYS);
                lieu_action_register("L15D05", P_DANRYS, "SP09ATT09", "SP09ARR01_10");
                /* 0xC8CD */
                if (ST(V_PRE90_OBJECT) != 0 && ST(L15_CRYSTAL_ROUND) != 3) {
                    CB->animation("SP09PRE73");
                    CB->animation("P09ARR01");
                    snprintf(g_l15_txt, sizeof(g_l15_txt), "OB%d", ST(V_PRE90_OBJECT));
                    CB->link_objet(9, g_l15_txt, 0x1B, 1);
                }
                CB->bloque(BLOQUE_NONE);
                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                return;
            }
            /* 0xC95C */
            if (ST(L15_DANRYS_FIGHT_WARN) > 1) {
                CB->bloque(BLOQUE_ALL);
                ST(L15_DANRYS_WARNED) = 0;
                CB->stop_joindre(-P_DANRYS);
                CB->animation("SP09ATT09");
                CB->animation("P09ARR01_10");
                /* 0xC9A6 */
                if (ST(V_PRE90_OBJECT) != 0) {
                    CB->animation("SP09PRE73");
                    CB->animation("P09ARR01");
                    snprintf(g_l15_txt, sizeof(g_l15_txt), "OB%d", ST(V_PRE90_OBJECT));
                    CB->link_objet(9, g_l15_txt, 0x1B, 1);
                }
                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0xCA20 */
    if (ST(L15_KONKALITE9_STATE) == 1) {
        ST(L15_KONKALITE9_STATE) = 2;
        ST(L15_KONKALITE9_ENGAGED) = 1;
        ST(V_ENGAGED_PERSONA) = P_KONKALITE9;
        CB->bloque(BLOQUE_NONE);
        CB->combat(P_KONKALITE9, 0);
    }
    /* 0xCA6E */
    if (ST(L15_LEVER_CAM_STEP) == 1 && ST(ST_LOCKED_CAMERA) != 3) {
        ST(L15_LEVER_CAM_STEP) = 2;
        CB->free_anim(6);
    }
    /* 0xCAAC */
    if (ST(L15_DANRYS_ENDING_FLAG) == 1 && ST(L15_LEVER_CAM_STEP) == 2 && ST(ST_LOCKED_CAMERA) == 3) {
        ST(L15_LEVER_CAM_STEP) = 1;
        CB->load_anim("215MOT01", 6);
        CB->play_anim(6, 1);
    }
    /* 0xCB11 */
    if (ST(V_EVENT_0) == 1 && ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
        ST(V_EVENT_0) = 0;
        ST(V_CRYSTAL_PROGRESS) = 5;
        CB->etat_objet("SOB45", -1, -1, -1, -1, -1, -1, -2, 0);
        /* 0xCB71 */
        if (ST(L15_CRYSTAL18_GIVEN) == 0) {
            CB->etat_objet("SOB18", -1, -1, -1, -1, -1, -1, -3, 0);
        }
        /* 0xCBA1 */
        if (ST(L15_CRYSTAL19_GIVEN) == 0) {
            CB->etat_objet("SOB19", -1, -1, -1, -1, -1, -1, -3, 0);
        }
        /* 0xCBD1 */
        if (ST(L15_CRYSTAL12_GIVEN) == 0) {
            CB->etat_objet("SOB12", -1, -1, -1, -1, -1, -1, -3, 0);
        }
        CB->pause_anim(4, 0, -1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 1);
        CB->load_anim("215MEU02", 1);
        CB->play_anim(1, 1);
        CB->divers_empile(DV_SCREEN, 2, 0);
        /* 0xCC56 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D84A", P_ARKHAN2, "SG00DIA01", NULL);
        }
        /* 0xCC7E */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D84B", P_ARKHAN3, "SG00DIA01", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xCCC4 */
    if (ST(L15_KALHI_STAGE_D40) == 1 && !CB->on_joindre(ST(ST_HERO)) && ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
        ST(L15_KALHI_STAGE_D40) = 2;
        CB->stop_joindre(-ST(ST_HERO));
        CB->animation("SG00PRE09");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 1);
        /* 0xCD44 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D113A", P_ARKHAN2, "SG00DIA02", NULL);
        }
        /* 0xCD6C */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D113B", P_ARKHAN3, "SG00DIA02", NULL);
        }
        /* 0xCD94 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D114A", P_ARKHAN2, "SG00DIA08_2000", NULL);
        }
        /* 0xCDBC */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D114B", P_ARKHAN3, "SG00DIA08_2000", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xCDFD */
    if (ST(V_EVENT_2) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(V_EVENT_2) = 0;
        ST(V_DLG_RANDOM) = CB->random(3);
        /* 0xCE53 */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->bruitage("L30B01", 5040, 1050);
        }
        /* 0xCE7A */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->bruitage("L11B01", 5040, 1050);
        }
        /* 0xCEA1 */
        if (ST(V_DLG_RANDOM) == 2) {
            CB->bruitage("L12B08", 5040, 1050);
        }
        ST(V_DLG_RANDOM) = CB->random(3);
        /* 0xCEDF */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->evenement(2, 4000);
        }
        /* 0xCF01 */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->evenement(2, 6000);
        }
        /* 0xCF23 */
        if (ST(V_DLG_RANDOM) == 2) {
            CB->evenement(2, 8000);
        }
    }
    /* 0xCF45 */
    if (ST(V_EVENT_34) == 1 && ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
        ST(V_EVENT_34) = 0;
        CB->change_camera(3, CAM_AUTO);
        CB->free_anim(0);
        CB->modify_var(&ST(L15_TRADE_ANIM_CAM), 0);
        CB->load_anim("215MEU03", 2);
        CB->play_anim(2, 1);
        CB->modify_var(&ST(L15_ANIM_PHASE), 2);
        CB->animation("Sar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xD004 */
    if (ST(V_CRI_KEY) == 1 && ST(ST_COMBAT_STANCE) == 1 && ST(V_CRI_USED) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_DANRYS) && ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_DANRYS_SCENE_STATE) != 0 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
        CB->select_pile(0, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L15_DANRYS_APPROACHED) = 1;
        CB->stop_joindre(-P_DANRYS);
        ST(V_CRI_USED) = 9;
        CB->animation("SP09ESA21");
        CB->animation("P09_65535");
        CB->dialogue("L15D76A", P_DANRYS);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
        CB->select_pile(0, 1);
        return;
    }
    /* 0xD15F */
    if (ST(V_CRI_KEY) == 1 && ST(ST_COMBAT_STANCE) == 1 && ST(V_CRI_USED) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_KALHI13) && ST(ST_VIE(P_KALHI13)) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12)) {
        CB->select_pile(0, 2);
        CB->bloque(BLOQUE_ALL);
        ST(V_CRI_USED) = 13;
        CB->stop_joindre(-P_KALHI13);
        CB->animation("SP13ESA15");
        CB->animation("P13_65535");
        lieu_action_register("L04D41", P_KALHI13, "P13DIA59_10", "SP13ARR01_10");
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
        CB->select_pile(0, 1);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1001A1E9).
 * Callback ingestion (0x10019E6A..0x1001A1E4), the trigger-table fill and
 * the unused dword_1002EC2C = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu15_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1001A412);
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
        l15_trig00();   /* 0x1001A419: call sub_10001073 -> trig00_10007543 */
        return 0;
    }
    /* 0x1001A2FE — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1001A46C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x10019E50).
 * Jump table @ 0x1001A4E9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu15_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu15: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        l15_reset_dll_globals();
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l15_trigger(arg);             break;  /* dword_1002EAC0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
