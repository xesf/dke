/*
 * lieu36.c - Native hand-port of LIEU36.DLL (level-36 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick, the tick body and the
 * level helper sub_10007400. The 42 zone trigger handlers live in
 * lieu36_trig.c (reached through l36_trigger, see lieu36_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000FB40, the state
 * pointer global is dword_10020C5C and the trigger table is
 * dword_10020B70[42]. event4 / event5 / event6, lieu_action_register and
 * lieu_pre_scene are identical to the shared ones in lieu_common.c.
 * The DLL .data the level code touches: dword_10020C90 (the last seen value
 * of ST(L36_PLAYER_HIT_COUNT)) and the sprintf buffer dword_10020970; the buffer is a
 * static in each function here. dword_100204D8/DC/E0 hold the hero's
 * position and are written only (never read back).
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu36_int.h"

/* dword_10020C90 - ST(L36_PLAYER_HIT_COUNT) as of the last tick (initially 0). */
i32 l36_prev_8d4;

/* The DLL is loaded afresh for every room: its globals restart
 * (dword_10020C90). Called by INIT, and by tools/lieu_difftest_harness.c
 * before every case. */
void l36_reset_dll_globals(void) {
    l36_prev_8d4 = 0;
}

/* dword_100204E0 / 100204D8 / 100204DC - hero position, written by tick only. */
static i32 g_hero_pos[3];

/* -------------------------------------------------------------------------
 * sub_10007400 (thunk sub_100010C3): __fastcall(ecx = who). Send Lodar (86)
 * or Konkalite (82) toward the hero's side of the room, unless that persona
 * is dead or already engaged; arms the EV_0 timer.
 * ---------------------------------------------------------------------- */
void l36_sub_7400(i32 who) {
    i32 d_14;
    i32 d_19;

    if (who == P_LODAR && ST(ST_VIE(P_LODAR)) >= 2)
        return;
    if (who == P_KONKALITE12 && ST(ST_VIE(P_KONKALITE12)) >= 2)
        return;
    if (who == P_LODAR && ST(V_ENGAGED_PERSONA) == P_LODAR)
        return;
    if (who == P_KONKALITE12 && ST(V_ENGAGED_PERSONA) == P_KONKALITE12)
        return;

    d_14 = CB->distance(1, 14);
    d_19 = CB->distance(1, 19);
    CB->combat(who, -1);
    CB->stop_joindre(who);
    ST(L36_CHASE_TARGET) = who;
    if (d_14 > 200 && d_19 < 200)
        CB->joindre(who, 6204, 6084, 0xFFC9, -1);
    else
        CB->joindre(who, 6193, 6486, 0x8AEF, -1);
    ST(V_EVENT_0) = 0;
    CB->evenement(0, 4000);
}

/* -------------------------------------------------------------------------
 * TICK BODY - sub_10007E7D (thunk sub_100010B9). Every `return` below is a
 * `jmp 0x10009BFA` (the function epilogue).
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    static c8 txt[256];     /* dword_10020970 */

    g_hero_pos[0] = CB->divers(DV_DESC_POS, ST(ST_HERO), 0);
    g_hero_pos[1] = CB->divers(DV_DESC_POS, ST(ST_HERO), 1);
    g_hero_pos[2] = CB->divers(DV_DESC_POS, ST(ST_HERO), 2);

    /* 0x7EDA - ST(L36_PLAYER_HIT_COUNT) changed: the hero is hurt */
    if (ST(L36_PLAYER_HIT_COUNT) != l36_prev_8d4) {
        l36_prev_8d4 = ST(L36_PLAYER_HIT_COUNT);
        CB->divers(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
        if (CB->divers(DV_HP, ST(ST_HERO), -1) - 50 > 0) {
            CB->divers(DV_HP, ST(ST_HERO), CB->divers(DV_HP, ST(ST_HERO), -1) - 50);
        } else {
            CB->select_pile(0, 2);
            CB->divers(DV_HP, ST(ST_HERO), 0);
            if (ST(ST_HERO) == P_ARKHAN2)
                CB->animation(lieu_anim_name(P_ARKHAN2, 26));
            if (ST(ST_HERO) == P_ARKHAN3)
                CB->animation(lieu_anim_name(P_ARKHAN3, 26));
            CB->modify_var(&ST(ST_HERO), 4);
            CB->select_pile(0, 1);
        }
    }

    /* 0x8014 - send the guards after the hero */
    if (ST(V_EVENT_0) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0) {
        if (CB->on_joindre(ST(L36_CHASE_TARGET))) {
            l36_sub_7400(ST(L36_CHASE_TARGET));
        } else if (!CB->on_joindre(ST(L36_CHASE_TARGET))) {
            if (ST(L36_CHASE_TARGET) == P_LODAR && ST(ST_VIE(P_LODAR)) >= 2)
                return;
            if (ST(L36_CHASE_TARGET) == P_KONKALITE12 && ST(ST_VIE(P_KONKALITE12)) >= 2)
                return;
            if (ST(L36_CHASE_TARGET) == P_LODAR && ST(V_ENGAGED_PERSONA) == P_LODAR)
                return;
            if (ST(L36_CHASE_TARGET) == P_KONKALITE12 && ST(V_ENGAGED_PERSONA) == P_KONKALITE12)
                return;
            CB->joindre(ST(L36_CHASE_TARGET), 0, 0, 0, ST(ST_HERO));
            if (ST(L36_CHASE_TARGET) == P_KONKALITE12)
                CB->animation("SP82GAR03");
            if (ST(L36_CHASE_TARGET) == P_LODAR)
                CB->animation("SP86GAR02");
            CB->joindre_perm(ST(L36_CHASE_TARGET), ST(ST_HERO));
            ST(V_EVENT_0) = 0;
        }
    }

    /* 0x8191 - debug overlay */
    snprintf(txt, sizeof(txt), "Dist %d ", CB->distance(1, 14));
    CB->print_msg(txt, 0, 200);
    snprintf(txt, sizeof(txt), "combat_on=%d act_on=%d", ST(V_ENGAGED_PERSONA), ST(ST_SCRIPT_BUSY));
    CB->print_msg(txt, 0, 180);

    /* 0x81FC - shout (cri) at Guardian 20 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && ST(V_CRI_KEY) == 1 &&
        ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 &&
        ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_GUARDIAN20) &&
        ST(ST_VIE(P_GUARDIAN20)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        ST(V_PLAYER_STRIKE) = 2;
        l36_trig04();
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x82CF - shout at Lodar */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(V_CRI_KEY) == 1 &&
        ST(V_CRI_USED) == 0 &&
        (ST(V_ENGAGED_PERSONA) == 0 || ST(V_ENGAGED_PERSONA) == P_KONKALITE12) &&
        ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_LODAR) &&
        ST(ST_VIE(P_LODAR)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        ST(V_PLAYER_STRIKE) = 2;
        l36_trig02();
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x83B1 - shout at Konkalite */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 &&
        ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 &&
        ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_KONKALITE12) &&
        ST(ST_VIE(P_KONKALITE12)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->dialogue("L36D200", P_KONKALITE12);
        if (ST(ST_VIE(P_LODAR)) == 1 && ST(L36_KILL_SEQ_ACTIVE) == 0) {
            CB->divers(DV_CINEMATIC, 0, 0);
            CB->combat(P_LODAR, -1);
            l36_sub_7400(P_LODAR);
            CB->modify_var(&ST(L36_LODAR_ALERT_STEP), 1);
        }
        ST(V_ENGAGED_PERSONA) = P_KONKALITE12;
        CB->combat(P_KONKALITE12, 0);
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }

    /* 0x850D - hero dead */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        ST(V_END_GAME_REQ) = 0;
        CB->evenement(EV_END_GAME, 2500);
    }
    /* 0x8566 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->bloque(BLOQUE_ALL);
        if (ST(V_GUARD_FIGHT_DONE) == 0)
            CB->cinematique("SC22_500");
        if (ST(V_GUARD_FIGHT_DONE) != 0)
            CB->cinematique("SC23_500");
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x85EA */
    if (ST(ST_LOCKED_CAMERA) == 4 && ST(L36_CAM4_CINEMATIC_STAGE) == 0 && ST(L36_GUARD_PAIR_PHASE) == 1 &&
        ST(ST_VIE(P_LODAR)) == 1 && ST(ST_VIE(P_KONKALITE12)) == 1) {
        ST(L36_CAM4_CINEMATIC_STAGE) = 1;
        CB->divers_empile(DV_CINEMATIC, 1, 2);
    }
    /* 0x865F */
    if (ST(ST_LOCKED_CAMERA) == 6 && ST(L36_CAM4_CINEMATIC_STAGE) == 1 && ST(L36_GUARD_PAIR_PHASE) == 1 &&
        ST(ST_VIE(P_LODAR)) == 1) {
        ST(L36_CAM4_CINEMATIC_STAGE) = 2;
        CB->dialogue("L36D146", P_LODAR);
    }
    /* 0x86C0 */
    if (ST(ST_VIE(P_LODAR)) == 1 && ST(L36_LODAR_ALERT_STEP) == 1 && ST(L36_KILL_SEQ_ACTIVE) == 0) {
        ST(L36_LODAR_ALERT_STEP) = 2;
        CB->animation("P86GAR02");
    }
    /* 0x870D */
    if (ST(ST_VIE(P_KONKALITE12)) == 1 && ST(L36_KONK_ALERT_STEP) == 1 && ST(L36_KILL_SEQ_ACTIVE) == 0) {
        ST(L36_KONK_ALERT_STEP) = 2;
        CB->animation("P82GAR03");
    }
    /* 0x875A - Lodar dead */
    if (ST(ST_VIE(P_LODAR)) == VIE_DEAD && ST(L36_KILL_SEQ_ACTIVE) == 0) {
        /* 0x877B */
        if (ST(L36_GUARD_PAIR_PHASE) == 1 && ST(V_ENGAGED_PERSONA) == P_LODAR && ST(L36_KONK_STRUCK_FIRST) == 0 &&
            ST(ST_VIE(P_KONKALITE12)) != VIE_DEAD) {
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            ST(V_EVENT_0) = 0;
            CB->evenement(0, -1);
            CB->animation("Sar_10");
            CB->animation("SP82GAR03_10");
            CB->stop_joindre(P_KONKALITE12);
            CB->stop_joindre(P_LODAR);
            CB->combat(P_KONKALITE12, -1);
            ST(L36_GUARD_PAIR_PHASE) = 2;
            ST(V_ENGAGED_PERSONA) = 1000;
            CB->divers_empile(DV_CINEMATIC, 0, 0);
            CB->combat(P_KONKALITE12, -1);
            CB->animation("SP82GAR03_10");
            CB->joindre_perm(P_KONKALITE12, ST(ST_HERO));
            CB->animation("SP82GAR03_10");
            CB->stop_joindre(P_KONKALITE12);
            CB->animation("SP82GAR27");
            CB->animation("P82_65535");
            lieu_action_register("L36D01A", P_KONKALITE12, "SP82DIA50", "SP82ARR01_10");
            lieu_action_register("L36D01B", P_KONKALITE12, "SP82DIA60", "SP82ARR01_10");
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            ST(L36_FIGHT_TARGET_STATE) = 0;
        }
        /* 0x892B */
        if (ST(L36_GUARD_PAIR_PHASE) == 1 && ST(V_ENGAGED_PERSONA) == P_LODAR && ST(L36_KONK_STRUCK_FIRST) == 1 &&
            ST(ST_VIE(P_KONKALITE12)) != VIE_DEAD) {
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L36_GUARD_PAIR_PHASE) = 2;
            ST(L36_FIGHT_TARGET_STATE) = 0;
            ST(V_ENGAGED_PERSONA) = 1000;
            CB->divers_empile(DV_CINEMATIC, 0, 0);
            CB->combat(P_KONKALITE12, -1);
            CB->animation("SP82GAR03_10");
            CB->joindre_perm(P_KONKALITE12, ST(ST_HERO));
            CB->animation("SP82GAR03_10");
            CB->stop_joindre(P_KONKALITE12);
            CB->animation("SP82GAR27");
            CB->animation("P82_65535");
            if (CB->distance(3, 1) < 1500)
                lieu_action_register("L36D02", P_KONKALITE12, "SP82DIA50", "SP82ARR01_10");
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x8A79 */
        if (ST(L36_GUARD_PAIR_PHASE) == 5) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L36_GUARD_PAIR_PHASE) = 6;
            ST(L36_FIGHT_TARGET_STATE) = 0;
        }
    }
    /* 0x8AB5 - Konkalite dead */
    if (ST(ST_VIE(P_KONKALITE12)) == VIE_DEAD && ST(L36_KILL_SEQ_ACTIVE) == 0) {
        ST(L36_KONK_ALERT_STEP) = 3;
        /* 0x8AE5 */
        if (ST(L36_GUARD_PAIR_PHASE) == 1 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_LODAR)) == 1) {
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            ST(L36_FIGHT_TARGET_STATE) = 0;
            ST(L36_GUARD_PAIR_PHASE) = 5;
            ST(V_ENGAGED_PERSONA) = 1000;
            ST(L36_LODAR_PANIC_ANIM_DONE) = 1;
            ST(L36_LODAR_FIGHT_TALK) = 0;
            ST(V_EVENT_0) = 0;
            CB->animation("SP86GAR02_10");
            CB->animation("Sar_10");
            CB->stop_joindre(P_KONKALITE12);
            CB->stop_joindre(P_LODAR);
            CB->evenement(0, -1);
            CB->combat(P_LODAR, -1);
            CB->divers_empile(DV_CINEMATIC, 0, 0);
            CB->joindre_perm(P_LODAR, ST(ST_HERO));
            CB->animation("SP86GAR02_10");
            CB->stop_joindre(P_LODAR);
            CB->animation("SP86GAR27");
            CB->animation("P86_65535");
            if (CB->distance(2, 1) < 1500)
                lieu_action_register("L36D04", P_LODAR, "SP86DIA50", "SP86ARR01_10");
            CB->stop_joindre(P_LODAR);
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x8C87 */
        if (ST(L36_GUARD_PAIR_PHASE) == 2 && ST(V_ENGAGED_PERSONA) == P_KONKALITE12) {
            ST(L36_FIGHT_TARGET_STATE) = 0;
            ST(L36_GUARD_PAIR_PHASE) = 4;
            ST(V_ENGAGED_PERSONA) = 0;
        }
        /* 0x8CD2 */
        if (ST(L36_GUARD_PAIR_PHASE) == 3 && ST(V_ENGAGED_PERSONA) == P_KONKALITE12) {
            ST(L36_FIGHT_TARGET_STATE) = 0;
            ST(L36_GUARD_PAIR_PHASE) = 7;
            ST(V_ENGAGED_PERSONA) = 0;
        }
    }
    /* 0x8D1D */
    if (ST(ST_VIE(P_GUARDIAN20)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_GUARDIAN20) {
        ST(L36_FIGHT_TARGET_STATE) = 0;
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x8D61 - Guardian 20 was hit */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN20, 0) && ST(ST_VIE(P_GUARDIAN20)) != VIE_DEAD) {
        CB->select_pile(0, 2);
        CB->select_pile(1, 2);
        ST(L36_FIGHT_TARGET_STATE) = 1;
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN20;
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN20, 1);
        CB->divers_empile(DV_CUT_SPEECH, 0, 0);
        CB->combat(P_GUARDIAN20, -1);
        CB->bloque(BLOQUE_ALL);
        ST(V_EVENT15_STEP) = CB->divers(DV_HP, P_GUARDIAN20, -1);
        if (ST(V_EVENT15_STEP) - 250 > 0)
            CB->divers(DV_HP, P_GUARDIAN20, ST(V_EVENT15_STEP) - 250);
        else
            CB->divers(DV_HP, P_GUARDIAN20, 1);
        if (CB->divers(DV_HP, P_GUARDIAN20, -1) <= 1 || ST(ST_PLAYER_WEAPON) == 27) {
            ST(ST_VIE(P_GUARDIAN20)) = VIE_DEAD;
            CB->stop_joindre(-P_GUARDIAN20);
            CB->animation("SP60IMP53");
            CB->animation("P60_65535");
            CB->animation("P60CAD");
            CB->pos_perso(P_GUARDIAN20, 0, 0, 0, 2, "P60CAD", 4, 0);
            CB->divers(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_GUARDIAN20);
            CB->animation("SP60IMP13");
            CB->animation("P60IMP48");
            CB->animation("P60GAR03");
            CB->combat(P_GUARDIAN20, 0);
            ST(L36_G20_SPOKE) = 1;
        }
        CB->animation("Sar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x8F93 - Konkalite was hit */
    if (CB->divers(DV_WAS_HIT, P_KONKALITE12, 0) && ST(ST_VIE(P_KONKALITE12)) != VIE_DEAD) {
        CB->select_pile(0, 2);
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L36_KILL_SEQ_ACTIVE) = 1;
        ST(V_ENGAGED_PERSONA) = P_KONKALITE12;
        CB->combat(P_KONKALITE12, -1);
        CB->divers_empile(DV_CINEMATIC, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_KONKALITE12, 1);
        CB->divers_empile(DV_WAS_HIT, P_LODAR, 1);
        CB->divers_empile(DV_CUT_SPEECH, 0, 0);
        ST(ST_VIE(P_KONKALITE12)) = VIE_DEAD;
        CB->stop_joindre(-P_KONKALITE12);
        CB->divers_empile(DV_BLOOD_AT_BONE, 82, 0);
        CB->animation("SP82IMP53");
        CB->animation("P82_65535");
        CB->animation("P82CAD");
        CB->pos_perso(P_KONKALITE12, 0, 0, 0, 2, "P82CAD", 3, 0);
        CB->modify_var(&ST(L36_KILL_SEQ_ACTIVE), 0);
        CB->animation("Sar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0x910D - Lodar was hit */
    if (CB->divers(DV_WAS_HIT, P_LODAR, 0) && ST(ST_VIE(P_LODAR)) != VIE_DEAD) {
        CB->select_pile(0, 2);
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L36_KILL_SEQ_ACTIVE) = 1;
        ST(ST_VIE(P_LODAR)) = VIE_DEAD;
        ST(V_ENGAGED_PERSONA) = P_LODAR;
        CB->combat(P_LODAR, -1);
        CB->stop_joindre(-P_LODAR);
        CB->divers_empile(DV_WAS_HIT, P_LODAR, 1);
        CB->divers_empile(DV_WAS_HIT, P_KONKALITE12, 1);
        CB->divers_empile(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_CINEMATIC, 0, 0);
        CB->animation("SP86IMP53");
        CB->animation("P86_65535");
        CB->animation("P86CAD");
        CB->pos_perso(P_LODAR, 0, 0, 0, 2, "P86CAD", 2, 0);
        CB->modify_var(&ST(L36_KILL_SEQ_ACTIVE), 0);
        CB->animation("Sar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }

    /* 0x926A - Guardian 20 notices the hero (first time) */
    if (ST(ST_LOCKED_CAMERA) == 1 && ST(L36_FIGHT_TARGET_STATE) == 0 && ST(L36_G20_POST_STATE) == 0 &&
        ST(ST_VIE(P_GUARDIAN20)) == 1 && ST(V_GUARD_FIGHT_DONE) < 2) {
        ST(L36_G20_POST_STATE) = 1;
        ST(L36_G20_SPOKE) = 1;
        CB->joindre(-P_GUARDIAN20, 0, 0, 0, ST(ST_HERO));
        if (ST(ST_HERO) == P_ARKHAN2 && ST(V_ENGAGED_PERSONA) == 0) {
            lieu_action_register("L36D147", P_GUARDIAN20, "SP60GAR26", "SP60GAR03_10");
            if (ST(V_ENGAGED_PERSONA) == 0)
                CB->joindre_perm(P_GUARDIAN20, ST(ST_HERO));
        }
        if (ST(ST_HERO) == P_ARKHAN3) {
            ST(L36_FIGHT_TARGET_STATE) = 1;
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN20;
            lieu_action_register("L36D148", P_GUARDIAN20, "SP60GAR26", "SP60GAR03_10");
            CB->combat(P_GUARDIAN20, 0);
            ST(L36_G20_SPOKE) = 1;
            return;
        }
    }
    /* 0x93B9 - Guardian 20 notices the hero again */
    if (ST(ST_LOCKED_CAMERA) == 1 && ST(L36_FIGHT_TARGET_STATE) == 0 && ST(L36_G20_POST_STATE) == 2 &&
        ST(ST_VIE(P_GUARDIAN20)) == 1 && ST(V_GUARD_FIGHT_DONE) < 2) {
        ST(L36_G20_POST_STATE) = 1;
        if (ST(ST_HERO) == P_ARKHAN2 && ST(V_ENGAGED_PERSONA) == 0) {
            CB->animation("SP60GAR26");
            CB->animation("SP60GAR03_10");
            if (ST(V_ENGAGED_PERSONA) == 0)
                CB->joindre_perm(P_GUARDIAN20, ST(ST_HERO));
        }
        if (ST(ST_HERO) == P_ARKHAN3 && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L36_FIGHT_TARGET_STATE) = 1;
            ST(V_ENGAGED_PERSONA) = P_GUARDIAN20;
            lieu_action_register("L36D148", P_GUARDIAN20, "SP60GAR26", "SP60GAR03_10");
            CB->combat(P_GUARDIAN20, 0);
            ST(L36_G20_SPOKE) = 1;
            return;
        }
    }
    /* 0x94F4 - Guardian 20 returns to its post */
    if (ST(ST_LOCKED_CAMERA) != 1 && ST(L36_FIGHT_TARGET_STATE) == 0 && ST(L36_G20_POST_STATE) == 1 &&
        ST(ST_VIE(P_GUARDIAN20)) == 1) {
        ST(L36_G20_POST_STATE) = 2;
        CB->stop_joindre(-P_GUARDIAN20);
        CB->pos_perso(P_GUARDIAN20, 3720, 6154, 0x4E20, 1, "P60ARR03", 4, 0);
    }
    /* 0x957B - Guardian 20 attacks */
    if (ST(ST_LOCKED_CAMERA) == 1 && ST(L36_FIGHT_TARGET_STATE) == 0 && ST(L36_G20_POST_STATE) == 1 &&
        ST(ST_VIE(P_GUARDIAN20)) == 1 &&
        CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 3720 &&
        ST(V_GUARD_FIGHT_DONE) < 2) {
        CB->select_pile(0, 2);
        ST(L36_FIGHT_TARGET_STATE) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_GUARDIAN20);
        ST(V_PLAYER_STRIKE) = -1;
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN20;
        if (ST(L36_G20_SPOKE) == 0)
            lieu_action_register("L36D08", P_GUARDIAN20, "SP60GAR26", "SP60GAR03_10");
        if (ST(L36_G20_SPOKE) == 1) {
            CB->dialogue("L36D08", P_GUARDIAN20);
            CB->animation("SP60GAR03_10");
        }
        CB->combat(P_GUARDIAN20, 0);
        ST(L36_G20_SPOKE) = 1;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x96D0 */
    if (ST(ST_LOCKED_CAMERA) == 1 && ST(L36_FIGHT_TARGET_STATE) == 1 && ST(V_ENGAGED_PERSONA) == 0 &&
        ST(V_GUARD_FIGHT_DONE) < 2) {
        ST(V_ENGAGED_PERSONA) = P_GUARDIAN20;
        CB->dialogue("L36D144", P_GUARDIAN20);
        CB->combat(P_GUARDIAN20, 0);
        ST(L36_G20_SPOKE) = 1;
    }
    /* 0x9759 */
    if (ST(L36_GUARD_PAIR_PHASE) == 3 && ST(ST_LOCKED_CAMERA) == 1 && ST(L36_PAIR_SEEN_AT_EXIT) == 0)
        ST(L36_PAIR_SEEN_AT_EXIT) = 1;
    /* 0x9798 */
    if (ST(L36_GUARD_PAIR_PHASE) == 2 && ST(ST_LOCKED_CAMERA) == 6 && ST(L36_PAIR_SEEN_AT_EXIT) == 0)
        ST(L36_PAIR_SEEN_AT_EXIT) = 1;
    /* 0x97D7 */
    if (ST(L36_GUARD_PAIR_PHASE) == 1 && ST(ST_LOCKED_CAMERA) == 6 && ST(L36_PAIR_SEEN_AT_EXIT) == 0)
        ST(L36_PAIR_SEEN_AT_EXIT) = 1;
    /* 0x9816 - the passing guard (Guardian 1) */
    if (ST(V_GUARD_FIGHT_DONE) < 2 && ST(L36_GUARD_PASS_STAGE) == 0 && ST(ST_LOCKED_CAMERA) == 2 &&
        ST(L36_GUARD_VARIANT) == 0) {
        ST(L36_GUARD_PASS_STAGE) = 1;
        CB->select_pile(1, 1);
        CB->animation("SP41ARR02_10");
        CB->joindre(-P_GUARDIAN1, 3910, 5734, 0x9470, -1);
        CB->dialogue("L36D05A", -1);
        CB->dialogue("L36D05B", -1);
        CB->dialogue("L36D05C", -1);
        CB->dialogue("L36D05D", -1);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x98E4 */
    if (ST(L36_GUARD_PASS_STAGE) == 1 && !CB->on_joindre(P_GUARDIAN1) &&
        CB->divers(DV_DESC_POS, P_GUARDIAN1, 0) > 3880 && ST(L36_GUARD_VARIANT) == 0) {
        ST(L36_GUARD_PASS_STAGE) = 2;
        CB->stop_joindre(-P_GUARDIAN1);
        CB->animation("SP41ARR02_10");
        CB->joindre(-P_GUARDIAN1, 3840, 5876, 0, -1);
        CB->modify_var(&ST(L36_GUARD_PASS_STAGE), 3);
    }
    /* 0x9984 */
    if (ST(L36_GUARD_PASS_STAGE) == 3 && !CB->on_joindre(P_GUARDIAN1) && ST(L36_GUARD_VARIANT) == 0) {
        ST(L36_GUARD_PASS_STAGE) = 4;
        CB->stop_joindre(-P_GUARDIAN1);
        CB->animation("SP41ARR03_10");
        CB->joindre(-P_GUARDIAN1, 3950, 5734, 0x9470, -1);
        CB->modify_var(&ST(L36_GUARD_PASS_STAGE), 5);
    }
    /* 0x9A0A */
    if (ST(L36_GUARD_PASS_STAGE) == 5 && !CB->on_joindre(P_GUARDIAN1) && ST(L36_GUARD_VARIANT) == 0) {
        ST(L36_GUARD_PASS_STAGE) = 6;
        CB->stop_joindre(-P_GUARDIAN1);
        CB->pos_perso(P_GUARDIAN1, 0, 0, 0, 4, "P41ARR03", 18, 0);
    }
    /* 0x9A71 */
    if (ST(V_GUARD_FIGHT_DONE) < 2 && ST(L36_GUARD_PASS_STAGE) == 0 && ST(ST_LOCKED_CAMERA) == 2 &&
        ST(L36_GUARD_VARIANT) == 1) {
        ST(L36_GUARD_PASS_STAGE) = 1;
        ST(V_DLG_RANDOM) = CB->random(3);
        if (ST(V_DLG_RANDOM) == 0) {
            CB->dialogue("L36D06A", -1);
            CB->dialogue("L36D06B", -1);
            CB->dialogue("L36D06C", -1);
        }
        if (ST(V_DLG_RANDOM) == 2) {
            CB->dialogue("L36D07A", -1);
            CB->dialogue("L36D07B", -1);
        }
        CB->joindre(-85, 3950, 5734, 0x9470, -1);
        CB->select_pile(0, 1);
    }
    /* 0x9B76 */
    if (ST(L36_GUARD_PASS_STAGE) == 1 && !CB->on_joindre(85) &&
        CB->divers(DV_DESC_POS, 85, 0) > 3880 && ST(L36_GUARD_VARIANT) == 1) {
        ST(L36_GUARD_PASS_STAGE) = 2;
        CB->stop_joindre(-85);
        CB->pos_perso(85, 0, 0, 0, 4, "P85ARR03", 18, 0);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1, 0x1000FEDD): arm events, then trigger 0 (or the death
 * ending). Returns 1 where the original returns 0 directly (the death
 * ending, 0x10010102); every other path returns state[0].
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
        l36_trig00();   /* 0x10010109: call sub_10001073 -> sub_10007547 */
        return 0;
    }
    /* 0x1000FFFA - the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1001015C): drain pending sound, run body.
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000FB40).
 * Jump table @ 0x100101D9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu36_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu36: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:
        tick();                                        break;  /* op -1 */
    case LIEU_OP_INIT:
        l36_reset_dll_globals();
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l36_trigger(arg);             break;  /* dword_10020B70[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);             break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);       break;
    case LIEU_OP_CALL6:  lieu_event6(arg);             break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
