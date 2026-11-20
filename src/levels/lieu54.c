/*
 * lieu54.c - Native hand-port of LIEU54.DLL (level-54 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick, the tick body and the
 * event4 / event5 / event6 dispatch
 * (the handlers are the shared older-version ones, lieu_common_v1.c). The 42 zone trigger handlers live in
 * lieu54_trig.c (reached through l54_trigger, see lieu54_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. LIEU54 is another build of the room-14
 * script (L14 dialogue codes, the Malrot / Provider fight), linked in a
 * non-standard layout: the shared library code (lieu_action_register,
 * pre90_scene, the pre-scene sub_10001856, event4/5/6) sits before the
 * level code. The entry is at 0x10001200, the state pointer global is
 * dword_10029CAC and the trigger table is dword_10029BC0[42].
 *
 * Its event4 (sub_10011A80), event5 (sub_1001281F) and event6
 * (sub_10015AF3) are instruction for instruction the LIEU47 ones (checked
 * by a normalised listing diff), not the lieu_common.c ones, so they are
 * the lieu_common_v1.c ones (newer=1). The
 * pre-scene sub_10001856 lacks the `n != 6` test of lieu_pre_scene
 * (lieu_v1_pre_scene in lieu_common_v1.c). lieu_action_register
 * is the shared one. The DLL .data the level code touches: the four event4
 * float accumulators (initially 1.0f), the two event5 sprintf buffers and
 * the tick print_msg buffer (dword_100299C0); all are reset at INIT.
 *
 * The start point read by trigger 0 is dword 0x998 = ST_START_POINT(14),
 * the room-14 slot, not ST_START_POINT(54).
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu54_int.h"

/* tick body print_msg buffer (dword_100299C0). */
static c8 g_txt[32];

/* The DLL is loaded afresh for every room: its globals restart. Called by
 * INIT, and by tools/lieu_difftest_harness.c before every case. */
void l54_reset_dll_globals(void) {
    lieu_v1_reset();
    g_txt[0] = '\0';
}

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10003F67 (thunk 0x100010B4), level-14 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    snprintf(g_txt, sizeof(g_txt), "comb:%d", ST(V_ENGAGED_PERSONA));
    CB->print_msg(g_txt, 0, 240);
    snprintf(g_txt, sizeof(g_txt), "Malfrat4:%d", ST(ST_VIE(P_MALROT4)));
    CB->print_msg(g_txt, 0, 248);
    /* 0x3FD6 */
    if (ST(V_EVENT_0) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(V_ENGAGED_PERSONA) = P_MALROT1;
        CB->combat(P_MALROT1, 0);
        CB->select_pile(2, 1);
        CB->evenement(2, -1);
        CB->stop_joindre(P_MALROT1);
        ST(V_EVENT_2) = 0;
        ST(V_EVENT_0) = 0;
        CB->select_pile(0, 1);
        /* 0x4066 */
        if (ST(ST_VIE(P_MALROT4)) == 1 && ST(L54_DELIA_DEAD) != 1 && ST(V_DELIA_PHASE) != 3 && ST(V_DELIA_PHASE) != 4) {
            CB->select_pile(1, 1);
            CB->evenement(1, -1);
            CB->animation("Sar_10");
            ST(V_EVENT_1) = 0;
            CB->stop_joindre(-P_MALROT4);
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
            CB->joindre(-P_MALROT4, 0, 0, 0, ST(ST_HERO));
            CB->modify_var(&ST(L54_MALROT4_APPROACH), 1);
            ST(V_ENGAGED_PERSONA) = 0x1814;
            CB->combat(P_MALROT4, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x415F */
    if (ST(V_EVENT_1) == 1 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && ST(L54_MALROT4_CHASE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(V_EVENT_1) = 0;
        /* 0x41B3 */
        if (ST(L54_MALROT4_WAYPOINT) == 1) {
            ST(L54_MALROT4_WAYPOINT) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP64ARR01_10");
            ST(L54_RANDOM_PICK) = CB->random(3);
            /* 0x4200 */
            if (ST(L54_RANDOM_PICK) == 0) {
                CB->joindre(-P_MALROT4, 6245, 1272, 0xBC4D, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 2);
            }
            /* 0x4242 */
            if (ST(L54_RANDOM_PICK) == 1) {
                CB->joindre(-P_MALROT4, 5737, 1624, 0x62CC, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 3);
            }
            /* 0x4284 */
            if (ST(L54_RANDOM_PICK) == 2) {
                CB->joindre(-P_MALROT4, 4332, 1306, 0x4260, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 4);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x42EB */
        if (ST(L54_MALROT4_WAYPOINT) == 2) {
            ST(L54_MALROT4_WAYPOINT) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP64ARR01_10");
            ST(L54_RANDOM_PICK) = CB->random(2);
            /* 0x4338 */
            if (ST(L54_RANDOM_PICK) == 0) {
                CB->joindre(-P_MALROT4, 5737, 1624, 0x62CC, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 3);
            }
            /* 0x437A */
            if (ST(L54_RANDOM_PICK) == 1) {
                CB->joindre(-P_MALROT4, 4332, 1306, 0x4260, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 4);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x43E1 */
        if (ST(L54_MALROT4_WAYPOINT) == 3) {
            ST(L54_MALROT4_WAYPOINT) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP64ARR01_10");
            ST(L54_RANDOM_PICK) = CB->random(2);
            /* 0x442E */
            if (ST(L54_RANDOM_PICK) == 0) {
                CB->joindre(-P_MALROT4, 6245, 1272, 0xBC4D, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 2);
            }
            /* 0x4470 */
            if (ST(L54_RANDOM_PICK) == 1) {
                CB->joindre(-P_MALROT4, 4332, 1306, 0x4260, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 4);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x44D7 */
        if (ST(L54_MALROT4_WAYPOINT) == 4) {
            ST(L54_MALROT4_WAYPOINT) = 0;
            CB->select_pile(1, 1);
            CB->animation("SP64ARR01_10");
            ST(L54_RANDOM_PICK) = CB->random(2);
            /* 0x4524 */
            if (ST(L54_RANDOM_PICK) == 0) {
                CB->joindre(-P_MALROT4, 6245, 1272, 0xBC4D, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 2);
            }
            /* 0x4566 */
            if (ST(L54_RANDOM_PICK) == 1) {
                CB->joindre(-P_MALROT4, 5737, 1624, 0x62CC, -1);
                CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 3);
            }
            CB->evenement(1, 1000);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x45CD */
    if (ST(L54_MALROT4_CHASE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && CB->distance(1, 2) > 200) {
        CB->select_pile(1, 1);
        ST(L54_MALROT4_TAUNTED) = 0;
        ST(L54_MALROT4_CHASE) = 0;
        ST(L54_MALROT4_WAYPOINT) = 10;
        ST(L54_MALROT4_APPROACH) = 0;
        CB->stop_joindre(-P_MALROT4);
        CB->joindre(P_MALROT4, 4787, 1423, 0xB798, -1);
        CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 1);
        CB->evenement(1, 7500);
        CB->select_pile(0, 1);
    }
    /* 0x46C2 */
    if (ST(V_EVENT_2) == 1 && ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && ST(V_DELIA_PHASE) == 0 && ST(V_DELIA_GONE) == 0) {
        CB->select_pile(2, 1);
        CB->divers(DV_SCREEN, 3, 0);
        ST(V_EVENT_2) = 0;
        /* 0x4767 */
        if (CB->divers(DV_DESC_POS, P_MALROT1, 2) > 55000 || CB->divers(DV_DESC_POS, P_MALROT1, 2) < 10000) {
            CB->stop_joindre(-P_MALROT1);
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT1, 0);
            CB->animation("SP61ATT05");
            CB->animation("P61ATT04");
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_MALROT1, 1);
        }
        CB->evenement(2, 5000);
        CB->select_pile(0, 1);
    }
    /* 0x47F8 */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        /* 0x4819 */
        if (ST(L54_MALROT4_SHOUT) == 64) {
            ST(L54_MALROT4_SHOUT) = 99;
            CB->dialogue("L14D235", P_MALROT4);
            ST(V_ENGAGED_PERSONA) = P_MALROT4;
            CB->combat(P_MALROT4, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x489D */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && CB->direction_perso(P_PROVIDER)) {
        ST(V_CRI_USED) = 1;
        /* 0x4925 */
        if (ST(L54_PROVIDER_SHOUT) != 99) {
            ST(L54_PROVIDER_SHOUT) = 16;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(34, 1000);
        return;
    }
    /* 0x4966 */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        /* 0x4987 */
        if (ST(L54_PROVIDER_SHOUT) == 16) {
            ST(L54_PROVIDER_SHOUT) = 99;
            lieu_action_register("L14D236", P_PROVIDER, "SP16GAR03", "SP16GAR03");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x49ED */
    if (ST(L54_PROVIDER_SHOUT) == 99) {
        ST(L54_PROVIDER_SHOUT) = 16;
    }
    /* 0x4A0E */
    if (ST(V_EVENT_35) == 1) {
        ST(V_EVENT_35) = 0;
        /* 0x4A2F */
        if (ST(L54_MALROT1_SHOUT) == 61) {
            ST(L54_MALROT1_SHOUT) = 99;
            lieu_action_register("L14D237", P_MALROT1, "SP61GAR02", "SP61GAR02");
            ST(V_ENGAGED_PERSONA) = P_MALROT1;
            CB->combat(P_MALROT1, 0);
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x4ABC */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 24) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && CB->direction_perso(P_MALROT1)) {
        ST(V_CRI_USED) = 1;
        ST(L54_GUARDS_HOSTILE) = 1;
        /* 0x4B53 */
        if (ST(L54_MALROT1_SHOUT) != 99) {
            ST(L54_MALROT1_SHOUT) = 61;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(35, 1000);
        return;
    }
    /* 0x4B94 */
    if (ST(L54_MALROT1_SHOUT) == 99) {
        ST(L54_MALROT1_SHOUT) = 16;
    }
    /* 0x4BC3 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 33) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_DELIA)) == 1 && CB->direction_perso(P_DELIA)) {
        ST(V_CRI_USED) = 1;
        /* 0x4C4B */
        if (ST(V_WEAPON_WARNED) != 99) {
            ST(V_WEAPON_WARNED) = 10;
        }
        CB->bloque(BLOQUE_ALL);
        CB->evenement(37, 1000);
        return;
    }
    /* 0x4C8C */
    if (ST(V_WEAPON_WARNED) == 99) {
        ST(V_WEAPON_WARNED) = 10;
    }
    /* 0x4CAD */
    if (ST(V_EVENT_37) == 1) {
        ST(V_EVENT_37) = 0;
        /* 0x4CCE */
        if (ST(V_WEAPON_WARNED) == 10) {
            ST(V_WEAPON_WARNED) = 99;
            lieu_action_register("L16D122", P_DELIA, "SP10DIA50", "SP10ARR01_10");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x4D34 */
    if (ST(V_EVENT_36) == 1) {
        CB->select_pile(1, 2);
        ST(V_EVENT_36) = 0;
        /* 0x4D65 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->play_anim(0, 1);
            CB->play_anim(1, 1);
        }
        /* 0x4D94 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->play_anim(2, 1);
            CB->play_anim(3, 1);
        }
        ST(L54_CAM_ANIM_ON) = 1;
        CB->select_pile(0, 2);
    }
    /* 0x4DE2 */
    if (ST(ST_VIE(P_MALROT4)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x4E00 */
        if (ST(V_ENGAGED_PERSONA) == P_MALROT4) {
            ST(V_ENGAGED_PERSONA) = 0;
            return;
        }
        /* 0x4E23 */
        if (ST(V_ENGAGED_PERSONA) == 0x1814) {
            ST(V_ENGAGED_PERSONA) = P_MALROT1;
            return;
        }
        /* 0x4E46 */
        if (ST(V_ENGAGED_PERSONA) == 0x1910) {
            ST(V_ENGAGED_PERSONA) = P_PROVIDER;
            return;
        }
    }
    /* 0x4E69 */
    if (ST(ST_VIE(P_PROVIDER)) == VIE_DEAD) {
        /* 0x4E78 */
        if (ST(V_ENGAGED_PERSONA) == P_PROVIDER) {
            ST(V_ENGAGED_PERSONA) = 0;
        }
        /* 0x4E96 */
        if (ST(V_ENGAGED_PERSONA) == 0x1910) {
            ST(V_ENGAGED_PERSONA) = P_MALROT4;
        }
        CB->ajoute_zone(38, 3315, 945, 275, 280, 0);
        CB->ajoute_zone(39, 3318, 957, 50, 50, 0);
    }
    /* 0x4EF0 */
    if (ST(ST_VIE(P_MALROT1)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x4F0E */
        if (ST(V_ENGAGED_PERSONA) == P_MALROT1) {
            ST(V_ENGAGED_PERSONA) = 0;
            return;
        } else {
            ST(V_ENGAGED_PERSONA) = P_MALROT4;
        }
        CB->ajoute_zone(9, 3096, 1157, 470, 135, 0);
    }
    /* 0x4F63 */
    if (ST(ST_VIE(35)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 35) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x4F90 */
    if (ST(L54_MALROT4_AMBUSH) >= 1) {
        /* 0x4F9F */
        if (ST(L54_PROVIDER_FOLLOWING) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && CB->distance(1, 3) > 480 && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L54_PROVIDER_FOLLOWING) = 0;
            CB->joindre(P_PROVIDER, 3267, 915, 0xCE17, -1);
            ST(L54_PROVIDER_WARN_ARMED) = 0;
        }
        /* 0x501E */
        if (ST(L54_PROVIDER_FOLLOWING) == 0 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && CB->distance(1, 3) < 200 && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L54_PROVIDER_FOLLOWING) = 1;
            CB->joindre(P_PROVIDER, 0, 0, 0, ST(ST_HERO));
        }
    }
    /* 0x50A4 */
    if (ST(L54_PROVIDER_WARN_ARMED) == 1 && ST(L54_PROVIDER_TALK_A2) >= 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && CB->distance(1, 3) < 250 && ST(ST_HERO) == P_ARKHAN2) {
        ST(L54_PROVIDER_WARN_ARMED) = 0;
        CB->dialogue("L14D263", P_PROVIDER);
    }
    /* 0x5132 */
    if (ST(L54_PROVIDER_WARN_ARMED) == 0 && ST(L54_PROVIDER_TALK_A2) >= 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD && ST(V_GUARD_FIGHT_DONE) != 2 && CB->distance(1, 3) > 400) {
        ST(L54_PROVIDER_WARN_ARMED) = 1;
    }
    /* 0x51A1 */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(V_DELIA_GONE) == 0 && ST(L54_MALROT1_INTRO_DONE) == 0 && CB->distance(1, 24) < 200 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD) {
        /* 0x5213 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L54_MALROT1_INTRO_DONE) = 1;
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT1, 1);
            CB->joindre(P_MALROT1, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP61GAR26");
            CB->animation("P61GAR02");
            CB->dialogue("L14D171", P_MALROT1);
            /* 0x52A4 */
            if (ST(ST_SPECIAL_MODE) == 1) {
                lieu_action_register("L14D139A", P_ARKHAN2, "SG00DIA02", NULL);
            }
            /* 0x52CC */
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
        /* 0x5356 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L54_MALROT1_INTRO_DONE) = 1;
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT1, 1);
            CB->joindre(P_MALROT1, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP61GAR26");
            CB->animation("P61GAR02");
            CB->dialogue("L14D172", P_MALROT1);
            CB->bloque(BLOQUE_NONE);
            ST(L54_GUARDS_HOSTILE) = 1;
            ST(V_ENGAGED_PERSONA) = P_MALROT1;
            CB->combat(P_MALROT1, 0);
            /* 0x5417 */
            if (ST(ST_VIE(P_MALROT4)) == 1) {
                CB->select_pile(1, 1);
                CB->evenement(1, -1);
                CB->animation("Sar_10");
                ST(V_EVENT_1) = 0;
                CB->stop_joindre(-P_MALROT4);
                CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
                CB->joindre(-P_MALROT4, 0, 0, 0, ST(ST_HERO));
                CB->modify_var(&ST(L54_MALROT4_APPROACH), 1);
                ST(V_ENGAGED_PERSONA) = 0x1814;
                CB->combat(P_MALROT4, 0);
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0x54D7 */
    if (ST(L54_KALHI13_ENTRY_STAGE) == 1) {
        CB->select_pile(2, 1);
        ST(L54_KALHI13_ENTRY_STAGE) = 40;
        /* 0x5505 */
        if (ST(ST_VIE(P_KALHI13)) == 1) {
            CB->joindre(-P_KALHI13, 4781, 1328, -1, -1);
        }
        CB->modify_var(&ST(L54_KALHI13_ENTRY_STAGE), 2);
        CB->select_pile(0, 1);
    }
    /* 0x554E */
    if (ST(L54_KALHI13_ENTRY_STAGE) == 2) {
        CB->select_pile(2, 1);
        /* 0x5570 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && !CB->on_joindre(P_KALHI13)) {
            ST(L54_KALHI13_ENTRY_STAGE) = 3;
            CB->stop_joindre(-P_KALHI13);
            CB->joindre(-P_KALHI13, 5181, 979, -1, -1);
            CB->animation("Sar_10");
            CB->modify_var(&ST(L54_KALHI13_ENTRY_STAGE), 4);
        }
        CB->select_pile(0, 1);
    }
    /* 0x55EE */
    if (ST(L54_KALHI13_ENTRY_STAGE) == 4) {
        CB->select_pile(2, 1);
        /* 0x5610 */
        if (ST(ST_VIE(P_KALHI13)) == 1 && !CB->on_joindre(P_KALHI13)) {
            ST(L54_KALHI13_ENTRY_STAGE) = 5;
            CB->pos_perso(P_KALHI13, 0, 0, 0, 4, "", -1, 0);
            CB->modify_var(&ST(ST_VIE(P_KALHI13)), 1);
            CB->vire_col(26, 1);
            CB->pos_perso(P_MALROT4, 5592, 1439, 0x2D9D, 1, "P64ARR01", 2, 601);
            CB->select_pile(1, 1);
            CB->animation("Sar_10");
            CB->modify_var(&ST(L54_MALROT4_WAYPOINT), 1);
            CB->evenement(1, 2000);
            CB->select_pile(0, 1);
            ST(L54_MALROT4_AMBUSH) = 1;
            CB->retire_zone(9);
        }
        CB->select_pile(0, 1);
    }
    /* 0x572B */
    if (CB->distance(1, 4) > 700 && ST(L54_ZONE4_UNBLOCKED) == 0 && ST(V_DELIA_GONE) == 0) {
        CB->select_pile(2, 1);
        ST(L54_ZONE4_UNBLOCKED) = 1;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x579D */
    if (CB->divers(DV_WAS_HIT, P_MALROT4, 0) && ST(ST_VIE(P_MALROT4)) != VIE_DEAD) {
        ST(ST_VIE(P_MALROT4)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->stop_joindre(-P_MALROT4);
        CB->combat(P_MALROT4, -1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_MALROT4, 0);
        CB->animation("SP64IMP51");
        CB->animation("P64CAD");
        CB->pos_perso(P_MALROT4, 0, 0, 0, 2, "P64CAD", 2, 0);
        ST(L54_GUARDS_HOSTILE) = 1;
        CB->select_pile(0, 1);
        /* 0x585B */
        if (ST(ST_VIE(P_MALROT1)) == 1) {
            CB->select_pile(2, 2);
            CB->evenement(2, -1);
            ST(V_EVENT_1) = 0;
            ST(V_ENGAGED_PERSONA) = 0x1814;
            CB->combat(P_MALROT1, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x58CA */
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
    /* 0x5970 */
    if (CB->divers(DV_WAS_HIT, P_MALROT1, 0) && ST(ST_VIE(P_MALROT1)) != VIE_DEAD) {
        CB->select_pile(2, 2);
        ST(ST_VIE(P_MALROT1)) = VIE_DEAD;
        CB->stop_joindre(-P_MALROT1);
        CB->combat(P_MALROT1, -1);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_MALROT1, 0);
        CB->animation("SP61IMP52");
        CB->animation("P61CAD");
        CB->pos_perso(P_MALROT1, 0, 0, 0, 2, "P61CAD", 0x18, 0);
        ST(L54_GUARDS_HOSTILE) = 1;
        CB->select_pile(0, 1);
        /* 0x5A2E */
        if (ST(ST_VIE(P_MALROT4)) == 1) {
            CB->select_pile(1, 1);
            CB->evenement(1, -1);
            CB->animation("Sar_10");
            ST(V_EVENT_1) = 0;
            CB->stop_joindre(-P_MALROT4);
            CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
            CB->joindre(-P_MALROT4, 0, 0, 0, ST(ST_HERO));
            CB->modify_var(&ST(L54_MALROT4_APPROACH), 1);
            ST(V_ENGAGED_PERSONA) = 0x1814;
            CB->combat(P_MALROT4, 0);
            CB->select_pile(0, 1);
        }
    }
    /* 0x5AF9 */
    if (CB->divers(DV_WAS_HIT, 35, 0) && ST(ST_VIE(35)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, 35, 1);
        CB->combat(35, -1);
        /* 0x5B63 */
        if ((CB->divers(DV_HP, 35, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, 35, (CB->divers(DV_HP, 35, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, 35, 1);
        }
        /* 0x5BC4 */
        if (CB->divers(DV_HP, 35, -1) <= 1) {
            ST(ST_VIE(35)) = VIE_DEAD;
            CB->stop_joindre(-35);
            CB->divers_empile(DV_BLOOD_AT_BONE, 35, 0);
            CB->animation("SP35IMP51");
            CB->animation("P35CAD");
            CB->pos_perso(0x23, 0, 0, 0, 2, "P35CAD", 4, 0);
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
    /* 0x5C95 */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && ST(V_EVENT15_STEP) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 3) < 200 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 1;
        CB->divers(DV_LOOK_AT_PLAYER, P_PROVIDER, 1);
        CB->dialogue("L14D167", P_PROVIDER);
        CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP16GAR03");
        CB->joindre_perm(P_PROVIDER, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x5D8E */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(V_EVENT15_STEP) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(1, 3) < 250 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_PROVIDER, 0);
        CB->dialogue("L14D168", P_PHEDORIA);
        CB->stop_joindre(-P_PROVIDER);
        CB->joindre(P_PROVIDER, 3667, 1075, 0x8A83, -1);
        CB->animation("SP16ARR01");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x5E61 */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->distance(1, 3) > 300 && ST(V_EVENT15_STEP) == 1 && ST(ST_VIE(P_PROVIDER)) != VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = 0;
        CB->divers(DV_LOOK_AT_PLAYER, P_PROVIDER, 0);
        CB->stop_joindre(-P_PROVIDER);
        CB->joindre(P_PROVIDER, 3667, 1075, 0x8A83, -1);
        CB->animation("SP16ARR01");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x5F15 */
    if (ST(ST_COMBAT_STANCE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2) && ST(L54_MALROT4_APPROACH) == 1 && ST(L54_MALROT4_TAUNTED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && CB->distance(1, 2) < 200) {
        CB->select_pile(1, 1);
        CB->evenement(1, -1);
        ST(V_EVENT_1) = 1;
        ST(L54_MALROT4_TAUNTED) = 1;
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_MALROT4);
        CB->divers(DV_LOOK_AT_PLAYER, P_MALROT4, 1);
        CB->joindre(-P_MALROT4, 0, 0, 0, ST(ST_HERO));
        CB->dialogue("L14D169", P_MALROT4);
        CB->modify_var(&ST(L54_MALROT4_APPROACH), 1);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x6068 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(L54_MALROT4_TAUNTED) == 1 && ST(ST_VIE(P_MALROT4)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0 && ST(L54_MALROT4_APPROACH) == 1 && CB->distance(1, 2) < 200) {
        CB->bloque(BLOQUE_ALL);
        ST(L54_MALROT4_TAUNTED) = 0;
        CB->dialogue("L14D170A", P_MALROT4);
        CB->dialogue("L14D170B", P_MALROT4);
        CB->stop_joindre(-P_MALROT4);
        CB->animation("SP64ARR01_10");
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x6132 */
    if (ST(L54_MALROT1_CALLED) != 0 && CB->distance(1, 24) > 150) {
        ST(L54_MALROT1_CALLED) = 0;
        CB->select_pile(2, 1);
        CB->joindre(P_MALROT1, 3649, 1268, 0xE98, -1);
        CB->animation("SP61ATT04");
        CB->evenement(2, 5000);
        CB->select_pile(0, 1);
    }
    /* 0x61BF */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L54_MALROT1_GUARDING) == 0 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && CB->distance(1, 24) < 150) {
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L54_MALROT1_GUARDING) = 1;
        CB->evenement(2, -1);
        ST(V_EVENT_2) = 0;
        CB->joindre_perm(P_MALROT1, ST(ST_HERO));
        CB->stop_joindre(P_MALROT1);
        CB->animation("SP61GAR26");
        CB->animation("P61GAR02");
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x62C0 */
    if (ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L54_MALROT1_GUARDING) == 1 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD) {
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L54_MALROT1_GUARDING) = 0;
        /* 0x6347 */
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
    /* 0x63BD */
    if (ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L54_MALROT1_GUARDING) == 1 && ST(ST_VIE(P_MALROT1)) != VIE_DEAD && CB->distance(1, 24) > 300) {
        CB->select_pile(2, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L54_MALROT1_GUARDING) = 0;
        CB->animation("SP61GAR27");
        CB->animation("P61ARR01");
        CB->joindre(P_MALROT1, 3649, 1268, 0xE98, -1);
        CB->bloque(BLOQUE_NONE);
        CB->evenement(2, 100);
        CB->select_pile(0, 1);
    }
    /* 0x64A7 */
    if (CB->distance(1, 2) > 400 && ST(L54_MALROT4_TALK_ARMED) >= 1) {
        ST(L54_MALROT4_TALK_ARMED) = 0;
    }
    /* 0x64E3 */
    if (CB->distance(1, 25) > 400 && ST(L54_MALROT1_ARMED_TALK) >= 1) {
        ST(L54_MALROT1_ARMED_TALK) = 0;
    }
    /* 0x6516 */
    if (ST(V_DELIA_PHASE) == 4 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->select_pile(2, 1);
        /* 0x6560 */
        if (CB->distance(33, 1) < 100 && ST(L54_DELIA_NEAR) == 0) {
            ST(L54_DELIA_NEAR) = 1;
            CB->joindre_perm(-P_DELIA, ST(ST_HERO));
            CB->stop_joindre(P_DELIA);
            CB->pos_perso(P_DELIA, CB->divers(DV_DESC_POS, P_DELIA, 0), CB->divers(DV_DESC_POS, P_DELIA, 1), CB->divers(DV_DESC_POS, P_DELIA, 2), 1, "P10ARR01", 0x21, 0);
        }
        /* 0x6604 */
        if (ST(L54_DELIA_ESCORT_ON) == 1 && (ST(V_FRAME_COUNTER) % 20) == 0) {
            /* 0x6637 */
            if (CB->distance(33, 1) >= 180) {
                CB->divers(DV_RUN, P_DELIA, 0);
                ST(L54_DELIA_NEAR) = 0;
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0x6693 */
            if (CB->distance(33, 1) >= 300) {
                ST(L54_DELIA_NEAR) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0x66FB */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 33)) {
        ST(V_DELIA_PHASE) = 20;
        ST(V_DELIA_KILLED) = 14;
        ST(L54_DELIA_DEAD) = 1;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        CB->select_pile(1, 2);
        CB->combat(P_DELIA, -1);
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0x21, 0);
        CB->select_pile(0, 1);
    }
    /* 0x67E5 */
    if (ST(ST_LOCKED_CAMERA) == 2 && ST(L54_CAM_ANIM_ON) == 0 && ST(L54_MOTOR_STOPPED) == 0) {
        /* 0x681B */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->play_anim(0, 1);
            CB->play_anim(1, 1);
            CB->bruitage("L14AMB4", -1, -1);
            ST(L54_CAM_ANIM_ON) = 1;
        }
        /* 0x686B */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->play_anim(2, 1);
            CB->play_anim(3, 1);
            CB->bruitage("L14AMB3", -1, -1);
            ST(L54_CAM_ANIM_ON) = 1;
        }
    }
    /* 0x68BE */
    if (ST(ST_LOCKED_CAMERA) != 2 && ST(L54_CAM_ANIM_ON) == 1) {
        ST(L54_CAM_ANIM_ON) = 0;
        /* 0x68F1 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->bruitage("L14AMB2", -1, -1);
        }
        /* 0x6915 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->bruitage("L14AMB1", -1, -1);
        }
    }
    /* 0x6939 */
    if (ST(ST_LOCKED_CAMERA) != 2 && ST(L54_MOTOR_SND_DONE) == 1) {
        ST(L54_MOTOR_SND_DONE) = 0;
    }
    /* 0x696C */
    if (ST(L54_MOTOR_STOPPED) == 1 && ST(L54_MOTOR_SND_DONE) == 0 && ST(ST_LOCKED_CAMERA) == 2) {
        /* 0x69A2 */
        if (ST(V_GUARD_FIGHT_DONE) == 2) {
            CB->bruitage("L14AMB2", -1, -1);
        }
        /* 0x69C6 */
        if (ST(V_GUARD_FIGHT_DONE) != 2) {
            CB->bruitage("L14AMB1", -1, -1);
        }
        CB->pause_anim(0, 0, 0);
        CB->pause_anim(1, 0, 0);
        ST(L54_MOTOR_SND_DONE) = 1;
        ST(L54_MOTOR_REFUELED) = 0;
    }
    /* 0x6A23 */
    if (ST(ST_HERO) == 4 && ST(L54_GAMEOVER_QUEUED) == 0) {
        ST(L54_GAMEOVER_QUEUED) = 1;
        CB->evenement(44, 3000);
    }
    /* 0x6A60 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x6A86 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x6AAF */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x6ADB */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x6B07 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x6B2B */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x6B48 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000158A).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1002967C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu54_trig.c. There is no death
 * branch in this version.
 *   arg = reload flag (non-zero => skip arming).
 * ---------------------------------------------------------------------- */
static void init(i32 reload_flag) {
    if (reload_flag == 0)
        l54_trig00();   /* 0x100015A3: call sub_1000106E -> sub_10001D60 */
}

/* -------------------------------------------------------------------------
 * TICK (opcode -1, 0x100015F6): drain pending sound, run body.
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x10001200).
 * Jump table @ 0x10001673, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu54_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu54: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        l54_reset_dll_globals();
        init(arg);
        break;
    case LIEU_OP_EVENT:  l54_trigger(arg);             break;  /* dword_10029BC0[arg]() */
    case LIEU_OP_CALL4:  lieu_v1_event4(arg, 1);            break;
    case LIEU_OP_CALL5:  lieu_v1_event5(arg, arg2, 1);      break;
    case LIEU_OP_CALL6:  lieu_v1_event6(arg);                   break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
