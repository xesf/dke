/*
 * lieu02.c — Native hand-port of LIEU02.DLL (level-2 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the DLL helpers. The 42 zone trigger handlers live in lieu02_trig.c
 * (reached through l02_trigger, see lieu02_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01.DLL; every
 * DLL global sits 0x20 bytes higher than in LIEU01 (engine slots C4xx..C9xx,
 * state pointer dword_1002C98C, trigger table dword_1002C8A0[42]), so the
 * lieu_cb_t members map 1:1 (LIEU02 slot = LIEU01 slot + 0x20).
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are instruction-for-instruction identical to LIEU01. tick_body (sub_10009DC8) is
 * level-2 specific.
 */

#include <stdio.h>

#include "lieu02_int.h"


/* Global sprintf buffer of the debug overlay (dword_1002C6A0). */
static c8 g_txt[256];



/* -------------------------------------------------------------------------
 * Per-frame body — sub_10009DC8 (thunk 0x100010B9), level-2 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp loc_1000D33C` (epilogue).
 * ---------------------------------------------------------------------- */
/* engine slot C988 (lieu_cb_t.cb_968, offset 0x7C, sub_446B54): OnJoindre,
 * nonzero while the actor is still walking. */
static i32 cb968(i32 id) { return CB->on_joindre(id); }

/* Hit drain shared by the guard blocks of tick_body (inlined 3x in the
 * original, e.g. 0xA5C1): life = layer_op(6,a,-1) stored in [0x84];
 * life -= damage [0x130], floored at 1. */
static void e_hit_drain(i32 a) {
    ST(L02_HIT_LIFE_TMP) = CB->divers(DV_HP, a, -1);
    if (ST(L02_HIT_LIFE_TMP) - ST(V_HIT_DAMAGE) > 0)
        CB->divers(DV_HP, a, ST(L02_HIT_LIFE_TMP) - ST(V_HIT_DAMAGE));
    else
        CB->divers(DV_HP, a, 1);
}

/* Actor 33 walks back to its post (inlined 2x in the original:
 * 0xBB0C..0xBDE2 and 0xC227..0xC4FD). Returns 1 where the original
 * jumps to the tick_body epilogue. Ends with state_toggle(0,1) on every
 * path. */
static i32 e_a33_walk_back(void) {
    if (ST(L02_KALHI33_WALK_STEP) != 3 && ST(L02_KALHI33_ROUTE) == 1) {
        if (ST(L02_KALHI33_WALK_STEP) == 0 || ST(L02_KALHI33_WALK_STEP) == 1) {
            CB->joindre(-P_KALHI33, -510, -2120, -1, -1);
            CB->animation("SP33ARR01_10");
            CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 2);
            CB->joindre(-P_KALHI33, -510, -1950, -1, -1);
            CB->animation("SP33ARR01_10");
            CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 3);
            CB->joindre(-P_KALHI33, -1090, -1380, 0xC350, -1);
            CB->select_pile(0, 1);
            return 1;
        }
        if (ST(L02_KALHI33_WALK_STEP) == 2) {
            CB->joindre(-P_KALHI33, -510, -1950, -1, -1);
            CB->animation("SP33ARR01_10");
            CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 3);
            CB->joindre(-P_KALHI33, -1090, -1380, 0xC350, -1);
            CB->select_pile(0, 1);
            return 1;
        }
    }
    if (ST(L02_KALHI33_WALK_STEP) != 3 && ST(L02_KALHI33_ROUTE) == 2) {
        if (ST(L02_KALHI33_WALK_STEP) == 0 || ST(L02_KALHI33_WALK_STEP) == 1) {
            CB->joindre(-P_KALHI33, -510, -571, -1, -1);
            CB->animation("SP33ARR01_10");
            CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 2);
            CB->joindre(-P_KALHI33, -510, -740, -1, -1);
            CB->animation("SP33ARR01_10");
            CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 3);
            CB->joindre(-P_KALHI33, -1090, -1380, 0xC350, -1);
            CB->select_pile(0, 1);
            return 1;
        }
        if (ST(L02_KALHI33_WALK_STEP) == 2) {
            CB->joindre(-P_KALHI33, -510, -740, -1, -1);
            CB->animation("SP33ARR01_10");
            CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 3);
            CB->joindre(-P_KALHI33, -1090, -1380, 0xC350, -1);
            CB->select_pile(0, 1);
            return 1;
        }
    }
    if (ST(L02_KALHI33_WALK_STEP) == 3)
        CB->joindre(-P_KALHI33, -1090, -1380, 0xC350, -1);
    CB->select_pile(0, 1);
    return 0;
}

static void tick_body(void) {
    /* 0x9DCE */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x9E11 — intro cinematic replay */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        if (CB->divers(DV_PERSONA_PRESENT, 0, 0)) CB->divers(DV_HP, 0, 0);
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) CB->divers(DV_HP, P_ARKHAN1, 0);
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) CB->divers(DV_HP, P_ARKHAN2, 0);
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) CB->divers(DV_HP, P_ARKHAN3, 0);
        if (ST(V_GUARD_FIGHT_DONE) == 0) CB->cinematique("SC22_500");
        if (ST(V_GUARD_FIGHT_DONE) != 0) CB->cinematique("SC23_500");
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x9F30 — door 2 / SP26 arrival */
    if (ST(V_EVENT_41) == 1) {
        ST(V_EVENT_41) = 0;
        CB->stop_joindre(-ST(ST_HERO));
        CB->joindre(-ST(ST_HERO), 0, 0, 0, 0x1A);
        CB->vire_col(0x21, 0);
        if (ST(ST_HERO) == P_ARKHAN2) lieu_action_register("L02D209", 2, "SG00DIA03", "Sar_10");
        if (ST(ST_HERO) == P_ARKHAN3) lieu_action_register("L02D210", 3, "SG00DIA03", "Sar_10");
        if (CB->distance(20, 31) < 0x50) {
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02MAR30");
                CB->animation("P02_65535");
            }
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03MAR30");
                CB->animation("P03_65535");
            }
        }
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("SG02PRE80");
            CB->animation("G02PRE81");
            CB->animation("SP26PRE60");
            CB->animation("P26_65535");
            CB->animation("SG02PRE82");
            if (ST(ST_PLAYER_WEAPON) != 0) CB->animation("G02ARR02_10");
            else                CB->animation("G02ARR01_10");
            CB->animation("SP26ARR01_10");
        }
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("SG03PRE80");
            CB->animation("G03PRE81");
            CB->animation("SP26PRE60");
            CB->animation("P26_65535");
            CB->animation("SG03PRE82");
            if (ST(ST_PLAYER_WEAPON) != 0) CB->animation("G03ARR02_10");
            else                CB->animation("G03ARR01_10");
            CB->animation("SP26ARR01_10");
        }
        CB->animation("SP26PRE91");
        CB->animation("P26ARR01_10");
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_PHEDORIA, 1);
        lieu_action_register("L02D211", 0x1A, "P26DIA60", "SP26ARR03_10");
        CB->divers_empile(DV_LOOK_AT_PLAYER, P_PHEDORIA, 0);
        CB->joindre(-P_PHEDORIA, 0, 0, 0xC350, -1);
        CB->animation("SP26OUV01");
        CB->bruitage("porte2", 1662, -1886);
        CB->anim_objet("IM03", 3, 0x3C, 0x5DC, 0);
        CB->animation("P00_65535");
        ST(ST_LEVEL) = 2;
        CB->divers_empile(DV_MUSIC, 1, 4);
        CB->change_piece(6, 1);
        return;
    }
    /* 0xA231 */
    if (ST(L02_PHEDORIA_FIGHT) == 1 && ST(V_GUARD_FIGHT_DONE) == 0
        && (ST(ST_LOCKED_CAMERA) == 2 || ST(ST_LOCKED_CAMERA) == 1)
        && ST(ST_COMBAT_STANCE) == 1
        && CB->divers(DV_CAN_FIGHT, 9, 2)
        && CB->direction_perso(P_PHEDORIA)
        && ST(V_PLAYER_STRIKE) == 0 && ST(ST_HERO) != 4) {
        ST(V_PLAYER_STRIKE) = 1;
        ST(V_PHEDORIA_TALK_B) = 1;
        l02_trig31();   /* call 0x10001000 -> sub_100149FB = trigger[31] */
        return;
    }
    /* 0xA2FB — corpse 13 */
    if (CB->divers(DV_WAS_HIT, P_KALHI13, 0) && ST(ST_VIE(P_KALHI13)) != VIE_DEAD) {
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_VIE(P_KALHI13)) = VIE_DEAD;
        ST(V_KALHI_KILLED) = 1;
        CB->select_pile(1, 2);
        CB->combat(P_KALHI13, -1);
        CB->stop_joindre(-P_KALHI13);
        CB->animation("SP13IMP52");
        CB->animation("P13CAD");
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13CAD", 0x1A, 0);
        CB->select_pile(0, 1);
    }
    /* 0xA3BB — corpse 33 */
    if (CB->divers(DV_WAS_HIT, P_KALHI33, 0) && ST(ST_VIE(P_KALHI33)) != VIE_DEAD) {
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_VIE(P_KALHI33)) = VIE_DEAD;
        ST(V_KALHI_KILLED) = 1;
        CB->select_pile(1, 2);
        CB->combat(P_KALHI33, -1);
        CB->stop_joindre(-P_KALHI33);
        CB->animation("SP33IMP52");
        CB->animation("P33CAD");
        CB->pos_perso(P_KALHI33, 0, 0, 0, 2, "P33CAD", 0x1B, 0);
        CB->select_pile(0, 1);
    }
    /* 0xA47B — corpse 10 */
    if (CB->divers(DV_WAS_HIT, P_DELIA, 0) && ST(ST_VIE(P_DELIA)) != VIE_DEAD) {
        ST(ST_SCRIPT_BUSY) = 0;
        ST(ST_VIE(P_DELIA)) = VIE_DEAD;
        ST(V_DELIA_KILLED) = 2;
        ST(V_DELIA_PHASE) = 0x14;
        CB->select_pile(1, 2);
        CB->combat(P_DELIA, -1);
        CB->stop_joindre(-P_DELIA);
        CB->animation("SP10IMP51");
        CB->animation("P10CAD");
        CB->pos_perso(P_DELIA, 0, 0, 0, 2, "P10CAD", 0x10, 0);
        CB->select_pile(0, 1);
    }
    /* 0xA54A — actor 57 hit */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN17, 0) && ST(ST_VIE(P_GUARDIAN17)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        ST(V_ENGAGED_PERSONA) = 0x39;
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN17, 1);
        CB->combat(P_GUARDIAN17, -1);
        CB->vire_col(0x21, 0);
        e_hit_drain(0x39);
        if (CB->divers(DV_HP, P_GUARDIAN17, -1) <= 1) {
            ST(ST_VIE(P_GUARDIAN17)) = VIE_DEAD;
            CB->stop_joindre(-P_GUARDIAN17);
            CB->animation("SP57IMP52");
            CB->animation("P57CAD");
            CB->pos_perso(P_GUARDIAN17, 0, 0, 0, 2, "P57CAD", 7, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_GUARDIAN17);
            CB->animation("SP57IMP12");
            CB->animation("P57IMP47");
            CB->animation("P57GAR02");
            CB->combat(P_GUARDIAN17, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0xA701 — actor 58 hit */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN18, 0) && ST(ST_VIE(P_GUARDIAN18)) != VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0x3A;
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN18, 1);
        CB->combat(P_GUARDIAN18, -1);
        CB->vire_col(0x21, 0);
        e_hit_drain(0x3A);
        if (CB->divers(DV_HP, P_GUARDIAN18, -1) <= 1) {
            ST(ST_VIE(P_GUARDIAN18)) = VIE_DEAD;
            CB->stop_joindre(-P_GUARDIAN18);
            CB->animation("SP58IMP55");
            CB->animation("P58CAD");
            CB->pos_perso(P_GUARDIAN18, 0, 0, 0, 2, "P58CAD", 0x0B, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_GUARDIAN18);
            CB->animation("SP58IMP15");
            CB->animation("P58IMP50");
            CB->animation("P58GAR05");
            CB->combat(P_GUARDIAN18, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0xA8B8 — actor 59 hit */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN19, 0) && ST(ST_VIE(P_GUARDIAN19)) != VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0x3B;
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN19, 1);
        CB->combat(P_GUARDIAN19, -1);
        CB->vire_col(0x20, 0);
        e_hit_drain(0x3B);
        if (CB->divers(DV_HP, P_GUARDIAN19, -1) <= 1) {
            ST(ST_VIE(P_GUARDIAN19)) = VIE_DEAD;
            CB->stop_joindre(-P_GUARDIAN19);
            CB->animation("SP59IMP53");
            CB->animation("P59CAD");
            CB->pos_perso(P_GUARDIAN19, 0, 0, 0, 2, "P59CAD", 2, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-P_GUARDIAN19);
            CB->animation("SP59IMP13");
            CB->animation("P59IMP48");
            CB->animation("P59GAR03");
            CB->combat(P_GUARDIAN19, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0xAA6F */
    if (ST(V_EVENT_34) == 1 && ST(ST_HERO) == P_ARKHAN1) {
        ST(V_EVENT_34) = 0;
        CB->animation("SP01IMP51");
        CB->evenement(0x26, 2000);
        CB->animation("P84ACT02");
        CB->animation("P84ARR01_1000");
    }
    /* 0xAAD3 */
    if (ST(V_EVENT_38) == 1) {
        ST(V_EVENT_38) = 0;
        CB->select_pile(1, 1);
        CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
        CB->select_pile(0, 1);
    }
    /* 0xAB58 */
    if ((i32)ST(V_ARKHAN1_PROGRESS) < 3 && ST(V_EVENT_2) == 1 && ST(V_ZED84_TALK_STAGE) == 1) {
        ST(V_EVENT_2) = 0;
        CB->animation("SG01ATT07");
        CB->animation("P84ARR02_10");
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
    }
    /* 0xABE2 — guard 57 alerted (hero 1) */
    if (ST(ST_HERO) == P_ARKHAN1 && ST(V_ENGAGED_PERSONA) == 0x39 && ST(L02_GUARD17_ALERT_STAGE) == 0) {
        ST(L02_GUARD17_ALERT_STAGE) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->dialogue("L02D107", P_GUARDIAN17);
        if (CB->divers(DV_PERSONA_PRESENT, P_DHORKAN11, 0))
            CB->pos_perso(P_DHORKAN11, -1, -1, -1, 4, "P11ARR02", -1, 1);
        if (CB->divers(DV_PERSONA_PRESENT, P_ZED84, 0))
            CB->pos_perso(P_ZED84, -1, -1, -1, 4, "P84ARR01", -1, 1);
        CB->pos_perso(P_PHEDORIA, 1030, -1600, 0xAB38, 1, "P26ARR03_50", 0x1F, 0);
        CB->change_camera(2, CAM_LOCKED);
        CB->animation("SP26GAR26");
        CB->animation("P26GAR03_50");
        CB->modify_var(&ST(L02_GUARD17_ALERT_STAGE), 2);
        CB->combat(P_GUARDIAN17, -1);
    }
    /* 0xAD23 — guard 57 alerted (heroes 2/3) */
    if ((i32)ST(ST_HERO) > 1 && ST(V_ENGAGED_PERSONA) == 0x39 && ST(L02_GUARD17_ALERT_STAGE) == 0
        && CB->distance(22, 7) > 0x12C) {
        CB->select_pile(1, 2);
        ST(V_GUARD17_AWAY) = 1;
        ST(L02_GUARD17_ALERT_STAGE) = 1;
        if (ST(ST_LOCKED_CAMERA) == 1 || ST(ST_LOCKED_CAMERA) == 2)
            CB->bloque(BLOQUE_ALL);
        CB->divers(DV_RUN, P_GUARDIAN17, 1);
        CB->combat(P_GUARDIAN17, -1);
        CB->animation("SP57GAR27");
        CB->animation("P57ARR02_500");
        CB->joindre(P_GUARDIAN17, 1565, -1836, 0x437A, -1);
        CB->animation("SP57ARR02_500");
        CB->modify_var(&ST(L02_GUARD17_ALERT_STAGE), 2);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xAE59 — guard 57 back on guard */
    if ((i32)ST(ST_HERO) > 1
        && (ST(V_ENGAGED_PERSONA) == 0x39 || (ST(V_ENGAGED_PERSONA) == 0 && ST(V_GUARD17_AWAY) == 1))
        && ST(L02_GUARD17_ALERT_STAGE) == 2
        && (CB->distance(7, 20) < 0x96
            || (ST(ST_LOCKED_CAMERA) == 1 && ST(V_ENGAGED_PERSONA) == 0x39))) {
        ST(L02_GUARD17_ALERT_STAGE) = 0;
        CB->animation("SP57GAR26");
        CB->animation("P57GAR02");
        if (ST(V_GUARD17_AWAY) == 1) {
            ST(V_GUARD17_AWAY) = 0;
            ST(V_ENGAGED_PERSONA) = 0x39;
        }
        CB->vire_col(0x21, 0);
        CB->combat(P_GUARDIAN17, 0);
    }
    /* 0xAF55 */
    if (ST(L02_GUARD17_ALERT_STAGE) == 2 && !cb968(0x1A)) {
        ST(L02_GUARD17_ALERT_STAGE) = 3;
        CB->joindre(-ST(ST_HERO), 0, 0, 0, 0x1A);
        lieu_action_register("L02D217", 0x1A, "SP26GAR03", "SP26GAR03_10");
        CB->modify_var(&ST(L02_GUARD17_ALERT_STAGE), 4);
    }
    /* 0xAFD6 */
    if (ST(L02_GUARD17_ALERT_STAGE) == 4 && !cb968(ST(ST_HERO))) {
        ST(L02_GUARD17_ALERT_STAGE) = 5;
        CB->select_pile(1, 2);
        CB->evenement(0x27, 850);
        CB->animation("SG26FRA01");
        CB->animation("P26GAR03");
        CB->select_pile(0, 1);
    }
    /* 0xB053 */
    if (ST(V_EVENT_39) == 1) {
        ST(V_EVENT_39) = 0;
        CB->select_pile(2, 2);
        CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
        CB->animation("SG01ATT07");
        CB->animation("P26GAR03");
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
        CB->select_pile(0, 1);
    }
    /* 0xB0EE */
    if (ST(V_EVENT_33) == 1) {
        ST(V_EVENT_33) = 0;
        CB->animation("SP00ACT01");
        CB->evenement(1, 800);
    }
    /* 0xB12A */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        ST(ST_START_POINT(1)) = 0;
        ST(ST_LEVEL) = 2;
        CB->divers_empile(DV_MUSIC, 1, 4);
        CB->change_piece(1, 0);
    }
    /* 0xB185 — actor 11 (P11) dialogue */
    if (ST(L02_DHORKAN11_STEP) == 1 && !cb968(0x0B)) {
        ST(V_EVENT_14) = 2;
        ST(L02_DHORKAN11_STEP) = 2;
        CB->stop_joindre(-P_DHORKAN11);
        if (ST(ST_SPECIAL_MODE) == 1) lieu_action_register("L02D105", 1, "SP01ATT74", NULL);
        if (ST(ST_SPECIAL_MODE) == 0) lieu_action_register("L02D106", 1, "SP01ATT75", NULL);
        CB->animation("sar_10");
        CB->joindre(P_DHORKAN11, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L06D05C", 0x0B, "SP11DIA65", "SP11ARR02_10");
        lieu_action_register("L06D05D", 0x0B, "SP11DIA57", "SP11ARR02_10");
        lieu_action_register("L06D05E", 0x0B, "SP11DIA58", "SP11ARR02_10");
        lieu_action_register("L06D05F", 0x0B, "SP11DIA64", "SP11ARR02_500");
        CB->animation("P11_65535");
        CB->joindre(P_DHORKAN11, 0, 0, 0xCB20, -1);
        CB->animation("P11ARR02");
        CB->joindre(-P_DHORKAN11, 186, -1662, 0xCB20, -1);
        CB->joindre(-ST(ST_HERO), 0, 0, ST(L02_GATE_HERO_FACING), -1);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB323 */
    if (ST(L02_DHORKAN11_STEP) == 2 && CB->distance(14, 6) <= 0x212) {
        ST(L02_DHORKAN11_STEP) = 3;
        CB->pos_perso(P_DHORKAN11, 186, -1662, 0xCB20, 4, "", 0x0E, 1);
        CB->modify_var(&ST(ST_VIE(P_DHORKAN11)), 1);
        CB->retire_zone(14);
    }
    /* 0xB3A6 — actor 84 follows hero 1 */
    if (ST(ST_HERO) == P_ARKHAN1 && ST(V_GUARD_FIGHT_DONE) == 0 && ST(V_RETURN_PART) == 0
        && (i32)ST(V_ARKHAN1_PROGRESS) < 3 && CB->divers(DV_PERSONA_PRESENT, P_ZED84, 0)) {
        if (ST(V_ZED84_TALK_A) == 0) {
            /* 0xB41A */
            if ((ST(L02_ZED84_FOLLOW) == 1 || ST(L02_ZED84_FOLLOW) == 2)
                && CB->distance(3, 20) < 0x96
                && CB->divers(DV_ANGLE_TO, P_ZED84, ST(ST_HERO)) < 0xFA0) {
                ST(L02_ZED84_FOLLOW) = 2;
                CB->stop_joindre(-P_ZED84);
                return;
            }
            /* 0xB496 */
            if (ST(L02_ZED84_FOLLOW) == 2) {
                i32 near_ = CB->distance(3, 20) < 0xC8
                         && CB->divers(DV_ANGLE_TO, P_ZED84, ST(ST_HERO)) < 0x1770;
                if ((!near_ && ST(ST_COMBAT_STANCE) == 0) || ST(ST_COMBAT_STANCE) == 1) {
                    /* 0xB505 */
                    ST(L02_ZED84_FOLLOW) = 1;
                    CB->stop_joindre(-P_ZED84);
                    CB->joindre(-P_ZED84, 1400, -695, 0xEA60, -1);
                    return;
                }
            }
        }
        /* 0xB53D */
        if (ST(L02_ZED84_FOLLOW) != 0x2C && !cb968(0x54)
            && CB->distance(3, 21) <= 0x64) {
            ST(L02_ZED84_GATE_STAGE) = 2;
            ST(L02_ZED84_FOLLOW) = 0x2C;
            ST(L02_ZED84_BRIEFING) = 2;
            CB->pos_perso(P_ZED84, 1400, -695, -1, 4, "", 3, 0);
            CB->retire_zone(3);
        }
    }
    /* 0xB5CC */
    if (ST(V_KALHI_RECRUIT) == 3 && ST(L02_ZED84_FOLLOW) == 1) {
        i32 a0, a1, a2;
        ST(L02_ZED84_FOLLOW) = 2;
        CB->select_pile(1, 1);
        CB->ajoute_zone(39, -1090, -1380, 70, 70, 0);
        CB->ajoute_zone(40, -506, -1740, 70, 70, 0);
        CB->divers_empile(DV_SCREEN, 5, 0);
        CB->pos_perso(P_KALHI13, -1, -1, -1, 4, "", 0x1A, 0);
        ST(ST_VIE(P_KALHI13)) = 4;
        CB->retire_zone(26);
        /* coordinates queried axis 2,1,0 (original order) */
        a2 = CB->divers(DV_DESC_POS, P_KALHI13, 2);
        a1 = CB->divers(DV_DESC_POS, P_KALHI13, 1);
        a0 = CB->divers(DV_DESC_POS, P_KALHI13, 0);
        CB->pos_perso(P_KALHI33, a0, a1, a2, 1, "P33ARR01", 0x1B, 1);
        CB->divers_empile(DV_SCREEN, 3, 0);
        CB->animation("SP33ATT01");
        lieu_action_register("L04D04", 0x21, "P33DIA54", "SP33ARR01");
        lieu_action_register("L04D05", 0x21, "SP33DIA59", "SP33ARR01_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("SP33ARR01_10");
        CB->modify_var(&ST(L02_KALHI33_WALK_STEP), 3);
        CB->modify_var(&ST(L02_KALHI33_FOLLOW), 1);
        CB->joindre(P_KALHI33, -1090, -1380, 0xC350, -1);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
    }
    /* 0xB7A1 — actor 13 follows */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(V_KALHI_PHASE) == 3) {
        if (ST(L02_KALHI13_FOLLOW) == 1 && CB->distance(26, 20) < 0x78 && ST(L02_DHORKAN11_STEP) == 0) {
            CB->select_pile(1, 1);
            if (CB->divers(DV_RUN, P_KALHI13, 1) == 1)
                CB->divers(DV_RUN, P_KALHI13, 0);
            ST(L02_DHORKAN11_STEP) = 1;
            ST(L02_KALHI13_CHATTER) = 1;
            CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->select_pile(0, 1);
        }
        /* 0xB887 */
        if (ST(L02_KALHI13_FOLLOW) == 1 && ST(V_FRAME_COUNTER) % 0x14 == 0) {
            if (CB->distance(26, 20) >= 0xB4
                && CB->distance(26, 20) < 0x12C) {
                CB->select_pile(1, 1);
                ST(L02_DHORKAN11_STEP) = 0;
                CB->divers(DV_RUN, P_KALHI13, 0);
                CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
                CB->select_pile(0, 1);
            }
            /* 0xB945 */
            if (CB->distance(26, 20) >= 0x12C) {
                CB->select_pile(1, 1);
                ST(L02_DHORKAN11_STEP) = 0;
                CB->divers(DV_RUN, P_KALHI13, 1);
                CB->joindre(-P_KALHI13, -1, -1, -1, ST(ST_HERO));
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0xB9BE — actor 33 follows */
    if (ST(V_KALHI_PHASE) == 3 && ST(ST_VIE(P_KALHI33)) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->select_pile(1, 1);
        if (ST(L02_KALHI33_FOLLOW) == 1) {
            if (CB->distance(27, 20) < 0x96
                && CB->divers(DV_ANGLE_TO, P_KALHI33, ST(ST_HERO)) < 0xFA0) {
                ST(L02_KALHI33_FOLLOW) = 2;
                CB->stop_joindre(-P_KALHI33);
                CB->select_pile(0, 1);
                return;
            }
            /* 0xBA81 */
            CB->select_pile(0, 1);
        }
        /* 0xBA8E */
        if (ST(L02_KALHI33_FOLLOW) == 2) {
            if (CB->distance(27, 20) < 0xC8
                && CB->divers(DV_ANGLE_TO, P_KALHI33, ST(ST_HERO)) < 0x1770) {
                /* 0xBDE8 */
                CB->select_pile(0, 0);
            } else {
                /* 0xBAE2 */
                CB->select_pile(1, 2);
                ST(L02_KALHI33_FOLLOW) = 1;
                CB->animation("SP33ARR01_10");
                if (e_a33_walk_back())
                    return;
                /* falls through to 0xBDE8 */
                CB->select_pile(0, 0);
            }
        }
    }
    /* 0xBDF2 — actor 33 notices the hero */
    if (ST(V_KALHI_PHASE) == 3 && ST(ST_VIE(P_KALHI33)) == 1 && !cb968(0x21)
        && CB->distance(27, 39) <= 0x64
        && ST(ST_LOCKED_CAMERA) != 6 && ST(ST_LOCKED_CAMERA) != 5
        && ST(L02_ZED84_GATE_STAGE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L02_ZED84_GATE_STAGE) = 1;
        ST(V_SAVED_HERO) = ST(ST_HERO);
        CB->select_pile(0, 1);
        CB->bloque(BLOQUE_ALL);
        CB->change_camera(5, CAM_LOCKED);
        CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
        lieu_action_register("L02D65A", 0x21, "SP33DIA59", "SP33ARR01");
        lieu_action_register("L02D65C", 0x21, "SP33DIA56", "SP33ARR01_10");
        CB->animation("SP33ARR01_10");
        CB->change_camera(5, CAM_AUTO);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xBF41 — actor 13 joins */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(V_KALHI_PHASE) == 4 && ST(V_EVENT15_STEP) == 1
        && CB->distance(26, 20) < 0x78) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(V_EVENT15_STEP) = 2;
        ST(V_KALHI_PHASE) = 3;
        ST(L02_COMPANION_JOINED) = 1;
        CB->animation("SP13LEV01");
        CB->animation("P13_65535");
        CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP13ARR01");
        lieu_action_register("L08D335A", 0x0D, "SP13DIA60", "SP13ARR01_10");
        lieu_action_register("L08D335B", 0x0D, "SP13DIA60", "SP13ARR01_10");
        lieu_action_register("L08D335C", 0x0D, "SP13DIA60", "SP13ARR01_10");
        ST(V_KALHI13_TALK_C) = 2;
        CB->modify_var(&ST(L02_KALHI13_FOLLOW), 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->divers(DV_LOOK_AT_PLAYER, P_KALHI13, 1);
    }
    /* 0xC0AB — actor 33 joins */
    if (ST(ST_VIE(P_KALHI33)) == 1 && ST(V_KALHI_PHASE) == 4 && ST(V_EVENT15_STEP) == 1
        && CB->distance(20, 27) < 0x78) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L02_KALHI33_FOLLOW) = 1;
        ST(V_EVENT15_STEP) = 2;
        ST(V_KALHI_PHASE) = 3;
        ST(L02_COMPANION_JOINED) = 1;
        ST(V_KALHI13_TALK_C) = 2;
        CB->animation("SP33LEV01");
        CB->animation("P33_65535");
        CB->joindre(-P_KALHI33, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP33ARR01");
        lieu_action_register("L08D335A", 0x21, "SP33DIA60", "SP33ARR01_10");
        lieu_action_register("L08D335B", 0x21, "SP33DIA60", "SP33ARR01_10");
        lieu_action_register("L08D335C", 0x21, "SP33DIA60", "SP33ARR01_10");
        CB->joindre(-P_KALHI33, -1090, -1380, 0xC350, -1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        if (e_a33_walk_back())   /* 0xC227 */
            return;
    }
    /* 0xC503 — actor 13 chatter */
    if (ST(L02_KALHI13_CHATTER) == 1 && CB->divers(DV_IDLE_TIME, 0, 0) >= 0x2710 && ST(ST_VIE(P_KALHI13)) == 1) {
        ST(L02_KALHI13_CHATTER) = 0;
        if (ST(L02_KALHI_CHAT_STAGE) == 2)
            lieu_action_register("L04D07", 0x0D, "SP13DIA60", "SP13ARR01_10");
        if (ST(L02_KALHI_CHAT_STAGE) == 1) {
            ST(L02_KALHI_CHAT_STAGE) = 2;
            lieu_action_register("L04D08", 0x0D, "SP13DIA53", "SP13ARR01_10");
        }
        if (ST(L02_KALHI_CHAT_STAGE) == 0) {
            ST(L02_KALHI_CHAT_STAGE) = 1;
            lieu_action_register("L04D09", 0x0D, "SP13DIA54", "SP13ARR01_10");
        }
        CB->divers(DV_IDLE_TIME, 1, 0);
        CB->modify_var(&ST(L02_KALHI13_CHATTER), 1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xC625 — actor 10 follows */
    if (ST(V_DELIA_PHASE) == 8 && ST(ST_VIE(P_DELIA)) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
        if (CB->distance(20, 16) < 0x8C && ST(L02_DELIA_RUN) == 0) {
            if (CB->divers(DV_RUN, P_DELIA, 1) == 1)
                CB->divers(DV_RUN, P_DELIA, 0);
            ST(L02_DELIA_RUN) = 1;
            CB->joindre(-P_DELIA, 0, 0, 0, ST(ST_HERO));
        }
        /* 0xC6DE */
        if (ST(L02_DELIA_FOLLOW) == 1 && ST(V_FRAME_COUNTER) % 0x14 == 0) {
            if (CB->distance(20, 16) >= 0xB4) {
                CB->divers(DV_RUN, P_DELIA, 0);
                ST(L02_DELIA_RUN) = 0;
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
            /* 0xC764 */
            if (CB->distance(20, 16) >= 0x12C) {
                ST(L02_DELIA_RUN) = 0;
                CB->divers(DV_RUN, P_DELIA, 1);
                CB->joindre(-P_DELIA, -1, -1, -1, ST(ST_HERO));
            }
        }
    }
    /* 0xC7C0 — hero crosses the bridge line */
    if ((ST(V_DELIA_PHASE) == 8 || ST(V_KALHI_PHASE) == 3) && ST(ST_HERO) != 4
        && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < -1290) {
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 0x3B6 && ST(L02_BRIDGE_CROSS) == 0)
            ST(L02_BRIDGE_CROSS) = 1;
        /* 0xC85D */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 0x320 && ST(L02_BRIDGE_CROSS) == 2)
            ST(L02_BRIDGE_CROSS) = 3;
    }
    /* 0xC8A1 */
    if (ST(L02_BRIDGE_CROSS) == 1) {
        ST(L02_BRIDGE_CROSS) = 2;
        if (ST(V_KALHI_PHASE) == 3 && ST(ST_VIE(P_KALHI13)) == 1) {
            CB->stop_joindre(-P_KALHI13);
            ST(L02_KALHI13_FOLLOW) = 0x14;
            ST(L02_KALHI13_CHATTER) = 0x0A;
            if (ST(V_FROM_LEVEL) != 7 && ST(V_FROM_LEVEL) != 6)
                lieu_action_register("L02D500", 0x0D, "SP13ARR01", "SP13ARR01_50");
        }
        /* 0xC94C */
        if (ST(V_DELIA_PHASE) == 8 && ST(ST_VIE(P_DELIA)) == 1) {
            CB->stop_joindre(-P_DELIA);
            ST(L02_DELIA_FOLLOW) = 0x14;
            if (ST(V_FROM_LEVEL) != 7 && ST(V_FROM_LEVEL) != 6) {
                if (ST(ST_HERO) == P_ARKHAN2) CB->dialogue("L02D501", P_ARKHAN2);
                if (ST(ST_HERO) == P_ARKHAN3) CB->dialogue("L02D502", P_ARKHAN3);
            }
        }
    }
    /* 0xC9F2 */
    if (ST(L02_BRIDGE_CROSS) == 3) {
        ST(L02_BRIDGE_CROSS) = 0;
        if (ST(V_KALHI_PHASE) == 3 && ST(ST_VIE(P_KALHI13)) == 1) {
            ST(L02_KALHI13_FOLLOW) = 1;
            ST(L02_KALHI13_CHATTER) = 1;
        }
        if (ST(V_DELIA_PHASE) == 8 && ST(ST_VIE(P_DELIA)) == 1)
            ST(L02_DELIA_FOLLOW) = 1;
    }
    /* 0xCA88 — guard 57 */
    if (ST(V_GUARD_FIGHT_DONE) != 2 && ST(L02_PHEDORIA_FIGHT) == 0) {
        if (ST(ST_VIE(P_GUARDIAN17)) == 1 && ST(V_ENGAGED_PERSONA) != 0x39 && ST(V_GUARD17_AWAY) == 0) {
            if (!cb968(0x39) && ST(L02_GUARD17_WALKING) == 1) {
                ST(L02_GUARD17_WALKING) = 0;
                CB->stop_joindre(-P_GUARDIAN17);
                return;
            }
            /* 0xCB23 */
            if ((i32)ST(ST_HERO) >= 2 && CB->distance(20, 22) < 0x12C
                && ST(L02_GUARD_LEAVING) == 0 && ST(L02_GUARD17_CHASING) == 0) {
                ST(L02_GUARD17_CHASING) = 1;
                CB->animation("SP57GAR26");
                CB->animation("P57GAR02_10");
                CB->joindre_perm(-0x39, ST(ST_HERO));
                return;
            }
            /* 0xCBB1 */
            if (CB->distance(20, 22) >= 0x1F4 && ST(L02_GUARD_LEAVING) == 0 && ST(L02_GUARD17_CHASING) == 1) {
                ST(L02_GUARD17_CHASE_ARMED) = 0;
                ST(L02_GUARD17_CHASING) = 0;
                CB->stop_joindre(-P_GUARDIAN17);
                CB->animation("SP57GAR27");
                CB->animation("P57ARR02_10");
                return;
            }
            /* 0xCC31 */
            if (CB->distance(20, 22) >= 0x1F4 && ST(L02_GUARD_LEAVING) == 1) {
                ST(L02_GUARD17_CHASE_ARMED) = 0;
                ST(L02_GUARD_LEAVING) = 0;
                ST(L02_GUARD_RETURN) = 1;
                CB->stop_joindre(-P_GUARDIAN17);
                CB->joindre(-P_GUARDIAN17, 1565, -1836, 0x437A, -1);
                return;
            }
        }
        /* 0xCCB1 */
        if (!cb968(0x39) && ST(L02_GUARD_RETURN) == 1) {
            ST(L02_GUARD_RETURN) = 2;
            CB->vire_col(0x21, 1);
        }
    }
    /* 0xCCF5 — guard 59 back to post */
    if (ST(ST_VIE(P_GUARDIAN19)) == 1 && CB->distance(20, 2) >= 0x190
        && ST(L02_GUARD_LEAVING) == 1 && ST(V_ENGAGED_PERSONA) != 0x3B) {
        ST(L02_GUARD_LEAVING) = 0;
        ST(L02_GUARD19_STATE) = 0;
        ST(L02_GUARD_RETURN) = 1;
        CB->joindre(P_GUARDIAN19, 490, -700, 0x7D00, -1);
    }
    /* 0xCD83 — guard 59 */
    if (ST(L02_GUARD19_STATE) == 0 && ST(ST_VIE(P_GUARDIAN19)) == 1 && ST(V_ENGAGED_PERSONA) != 0x3B) {
        if (ST(V_G19_TALK) == 0 && CB->distance(2, 20) < 0x190) {
            ST(L02_GUARD19_STATE) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->joindre(P_GUARDIAN19, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L02D67", 0x3B, "SP59GAR26", "SP59GAR03_10");
            CB->vire_col(0x21, 1);
            CB->retire_zone(9);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0xCE50 */
        if (ST(V_G19_TALK) == 1 && CB->distance(2, 20) < 0x12C) {
            CB->vire_col(0x20, 0);
            CB->bloque(BLOQUE_ALL);
            ST(L02_GUARD19_STATE) = 1;
            ST(L02_GUARD_LEAVING) = 1;
            CB->dialogue("L02D68", P_GUARDIAN19);
            CB->joindre(P_GUARDIAN19, 623, -700, 0x59D8, -1);
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0xCEE1 — guard 58 */
    if (ST(L02_GUARD18_STAGE) == 0 && ST(ST_VIE(P_GUARDIAN18)) == 1 && ST(V_ENGAGED_PERSONA) != 0x3A
        && CB->distance(11, 20) < 0x320) {
        ST(L02_GUARD18_STAGE) = 1;
        lieu_action_register("L02D110", 0x3A, "SP58ARR04", "SP58GAR05_10");
    }
    /* 0xCF51 */
    if (ST(L02_GUARD18_STAGE) == 1 && ST(ST_VIE(P_GUARDIAN18)) == 1 && ST(V_ENGAGED_PERSONA) != 0x3A
        && CB->distance(11, 20) < 0x190) {
        ST(L02_GUARD18_STAGE) = 2;
        CB->vire_col(0x21, 0);
        ST(V_ENGAGED_PERSONA) = 0x3A;
        CB->combat(P_GUARDIAN18, 0);
    }
    /* 0xCFCE */
    if ((ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0x0D)
        || (ST(ST_VIE(P_KALHI33)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0x21)) {
        ST(V_KALHI_KILLED) = 1;
        ST(ST_SCRIPT_BUSY) = 0;
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0xD037 — re-arm zones once the fighting guard is dead */
    if ((i32)ST(ST_HERO) > 1 && ST(V_ENGAGED_PERSONA) == 0x39 && ST(ST_VIE(P_GUARDIAN17)) == VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0;
        CB->ajoute_zone(1, 1600, -1830, 80, 90, 0);
    }
    /* 0xD091 */
    if (ST(V_ENGAGED_PERSONA) == 0x3A && ST(ST_VIE(P_GUARDIAN18)) == VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0;
        CB->ajoute_zone(1, 1600, -1830, 80, 90, 0);
    }
    /* 0xD0D9 */
    if (ST(V_ENGAGED_PERSONA) == 0x3B && ST(ST_VIE(P_GUARDIAN19)) == VIE_DEAD) {
        ST(V_ENGAGED_PERSONA) = 0;
        CB->ajoute_zone(9, 486, -610, 120, 90, 0);
    }
    /* 0xD121 — hero 1 at the gate */
    if (ST(V_EVENT_14) == 0 && ST(ST_HERO) == P_ARKHAN1
        && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < 0x0A && ST(ST_LOCKED_CAMERA) == 5) {
        if (ST(L02_ZED84_GATE_STAGE) == 1) {
            ST(L02_ZED84_GATE_STAGE) = 2;
            CB->pos_perso(P_ZED84, 1400, -695, -1, 4, "", 3, 0);
            ST(L02_ZED84_FOLLOW) = 0x2C;
            ST(L02_ZED84_BRIEFING) = 2;
            CB->pos_perso(P_DHORKAN11, 2000, -1662, 0x3A98, 1, "P11ARR02", 0x0E, 0);
            return;
        }
        /* 0xD1FA */
        if (ST(L02_DHORKAN11_SCENE_ON) == 0) {
            ST(L02_DHORKAN11_SCENE_ON) = 1;
            CB->bloque(BLOQUE_ALL);
            ST(L02_GATE_HERO_FACING) = CB->divers(DV_DESC_POS, ST(ST_HERO), 2);
            CB->pos_perso(P_DHORKAN11, 186, -1662, 0x3A98, 1, "P11ARR02", 0x0E, 0);
            CB->dialogue("L02D50", P_DHORKAN11);
            CB->joindre(-ST(ST_HERO), 0, 0, 0, 0x0B);
            CB->retire_zone(24);
            CB->retire_zone(25);
            CB->joindre(P_DHORKAN11, -202, -1693, 0x3A98, -1);
            CB->modify_var(&ST(L02_DHORKAN11_STEP), 1);
        }
    }
    /* 0xD2DC — debug overlay (sprintf into the global dword_1002C6A0) */
    snprintf(g_txt, sizeof(g_txt), "CRI:%d", ST(V_CRI_KEY));
    CB->print_msg(g_txt, 0, 0xF0);
    snprintf(g_txt, sizeof(g_txt), "cri_on:%d", ST(V_CRI_USED));
    CB->print_msg(g_txt, 0, 0xF8);
    /* 0xD33C */
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x10019AFA).
 * Callback ingestion (0x10019AFA..0x10019CC1), the trigger-table fill
 * (0x10019CC6..0x10019E60) and the unused dword_1002CA0C = state+0xC8 are
 * skipped: g_lieu_cb is provided by the bridge, the trigger table is
 * static in lieu02_trig.c.
 *   arg = reload flag (non-zero => skip arming).
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
        l02_trig00();   /* 0x1001A0A9: call sub_10001073 -> sub_100073F0 */
    } else {
        CB->divers_empile(DV_SCREEN, 1, 0);
        ST(V_DEATH_EVENT_SENT) = 1;
        for (i32 layer = 0; layer < 4; layer++) {
            if (CB->divers(DV_PERSONA_PRESENT, layer, 0))
                CB->divers(DV_HP, layer, 0);
        }
        if (!ST(V_GUARD_FIGHT_DONE))
            CB->cinematique("SC22_500");
        else
            CB->cinematique("SC23_500");
        CB->divers_empile(DV_END_GAME, 0, 0);
        return 1;   /* the death ending returns 0, not state[0] */
    }
    return 0;
}

/* -------------------------------------------------------------------------
 * TICK (opcode -1, 0x1001A0FC): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x10019AE0).
 * Jump table @ 0x1001A179, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu02_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu02: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l02_trigger(arg);             break;  /* dword_1002C8A0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
