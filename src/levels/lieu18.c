/*
 * lieu18.c — Native hand-port of LIEU18.DLL (level-18 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu18_trig.c (reached through
 * l18_trigger, see lieu18_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Unlike the earlier levels the entry is not
 * at RVA 0x1046: @FonctionEditor@16 is at 0x100169F0 (found through the
 * export table), the state pointer global is dword_1002AD5C and the trigger
 * table is dword_1002AC70[42].
 *
 * event4 (sub_1000124A), event5 (sub_100022C4), event6 (sub_10005BFC),
 * lieu_action_register (sub_100170BC), lieu_pre_scene (sub_1001726C),
 * pre90_scene (sub_1001715C) and sub_10001210 (float reset) are the shared
 * lieu_common.c; the entry and INIT are instruction-for-instruction
 * identical to LIEU03. tick_body (sub_100087FC) is level-18 specific.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu18_int.h"

/* Global sprintf buffer of the debug overlay (dword_1002AA70). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * sub_10013756 (thunk 0x10001019) — no caller in the DLL.
 * __fastcall(ecx = g, edx = sp): the PRE80/PRE60 clips of a weapon warning.
 * ---------------------------------------------------------------------- */
void l18_pre80_scene(i32 g, i32 sp) {
    c8 b[256];

    snprintf(b, sizeof(b), "SG%02dPRE80", g);
    CB->animation(b);
    snprintf(b, sizeof(b), "G%02dPRE81", g);
    CB->animation(b);
    snprintf(b, sizeof(b), "SP%02dPRE60", sp);
    CB->animation(b);
    snprintf(b, sizeof(b), "P%02d_65535", sp);
    CB->animation(b);
    snprintf(b, sizeof(b), "SG%02dPRE82", g);
    CB->animation(b);
    /* 0x383D */
    if (ST(ST_PLAYER_WEAPON) != 0)
        snprintf(b, sizeof(b), "G%02darr02_10", g);
    else
        snprintf(b, sizeof(b), "G%02darr01_10", g);
    CB->animation(b);
    snprintf(b, sizeof(b), "SP%02darr01_10", sp);
    CB->animation(b);
}

/* Per-frame body — sub_100087FC (thunk 0x100010BE), level-18 specific. */
static void tick_body(void) {
    i32 t2 = CB->distance(3, 16);
    i32 t1 = CB->distance(6, 16);
    snprintf(g_txt, sizeof(g_txt), "DISTANCE(6,16) %d DISTANCE(3,16) %d : %d   ", t1, t2, ST(ST_SCRIPT_BUSY));
    CB->print_msg(g_txt, 0, 440);
    /* 0x885A */
    if (ST(ST_COMBAT_STANCE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->divers(DV_CAN_FIGHT, 0, 0)) {
        /* 0x8892 */
        if (ST(ST_LOCKED_CAMERA) != 0 && ST(V_EVENT_51) == 0 && ST(ST_VIE(P_MALROT2)) != VIE_DEAD && ST(L18_MALROT2_FIGHT_STARTED) == 0) {
            CB->select_pile(0, 2);
            CB->select_pile(1, 2);
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            CB->evenement(0, -1);
            CB->evenement(1, -1);
            ST(V_EVENT_0) = 0;
            ST(V_EVENT_1) = 0;
            ST(L18_MALROT2_FIGHT_STARTED) = 1;
            ST(V_ENGAGED_PERSONA) |= 0x662;   /* engaged: P_ARMAL << 8 | P_MALROT2 */
            CB->stop_joindre(-P_MALROT2);
            CB->animation("Sar_10");
            CB->animation("SP62GAR02_10");
            CB->animation("SP06GAR02_10");
            CB->modify_var(&ST(V_EVENT_51), 1);
            CB->modify_var(&ST(L18_ARMAL_FIGHT_REQ), 1);
            CB->modify_var(&ST(L18_FIGHT_PHASE), 3);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x89E5 */
        if (ST(ST_LOCKED_CAMERA) == 0 && ST(L18_MALROT2_FIGHT_STARTED) == 0 && ST(ST_VIE(P_MALROT2)) == 1) {
            CB->select_pile(0, 2);
            CB->select_pile(1, 2);
            CB->select_pile(2, 2);
            CB->bloque(BLOQUE_ALL);
            CB->evenement(0, -1);
            CB->evenement(1, -1);
            ST(V_EVENT_0) = 0;
            ST(V_EVENT_1) = 0;
            ST(L18_MALROT2_FIGHT_STARTED) = 1;
            ST(V_ENGAGED_PERSONA) |= 0x662;   /* engaged: P_ARMAL << 8 | P_MALROT2 */
            CB->stop_joindre(-P_MALROT2);
            CB->animation("Sar_10");
            CB->animation("SP62GAR02_10");
            CB->modify_var(&ST(L18_ARMAL_ATTACKED), 1);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x8AFB */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && CB->direction_zone(4) && ST(ST_ACTION_KEY) == 1 && ST(ST_VIE(P_ARMAL)) == 1 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_SCRIPT_BUSY) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x8B76 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x8B88 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD20", P_ARKHAN2);
            }
            /* 0x8BAA */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD21", P_ARKHAN3);
            }
        }
        /* 0x8BCC */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0x8BDE */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD61", P_ARKHAN2);
            }
            /* 0x8C00 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD63", P_ARKHAN3);
            }
        }
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
    /* 0x8C49 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && CB->direction_zone(3) && ST(ST_ACTION_KEY) == 1 && ST(ST_VIE(P_MALROT2)) == 1 && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_SCRIPT_BUSY) == 0) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x8CC4 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x8CD6 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD20", P_ARKHAN2);
            }
            /* 0x8CF8 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD21", P_ARKHAN3);
            }
        }
        /* 0x8D1A */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0x8D2C */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("LCOD61", P_ARKHAN2);
            }
            /* 0x8D4E */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("LCOD63", P_ARKHAN3);
            }
        }
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
    /* 0x8D97 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 4) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_ARMAL) && ST(ST_VIE(P_ARMAL)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP06ARR01_300");
        lieu_action_register("L18D265A", P_ARMAL, "SP06DIA64", "SP06ARR01_10");
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8E88 */
    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 3) && ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_MALROT2) && ST(ST_VIE(P_MALROT2)) == 1) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->animation("P62GAR02_300");
        lieu_action_register("L18D134", P_MALROT2, "SP62DIA63", "P62GAR02_10");
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8F6B */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        ST(V_END_GAME_REQ) = 0;
        CB->evenement(44, 2500);
    }
    /* 0x8FBD */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        ST(V_DEATH_EVENT_SENT) = 1;
        /* 0x8FFD */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x9026 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x9052 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x907E */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x90A2 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x90BF */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x90EB */
    if (ST(ST_VIE(P_ARMAL)) >= 2 && ST(ST_VIE(P_MALROT2)) >= 2 && ST(L18_BOTH_KNOCKED_OUT) == 0) {
        ST(L18_BOTH_KNOCKED_OUT) = 1;
        ST(ST_SCRIPT_BUSY) = 0;
        CB->change_camera(0, CAM_AUTO);
        CB->modify_var(&ST(L18_DOOR_RETURN_STATE), 4);
        CB->ajoute_zone(7, 2804, 1983, 120, 100, 0);
        CB->ajoute_zone(8, 2117, 2100, 120, 100, 0);
    }
    /* 0x919F */
    if (CB->distance(4, 6) < 600) {
        /* 0x91B9 */
        if (CB->divers(DV_WAS_HIT, P_ARMAL, 0) && ST(ST_VIE(P_ARMAL)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            ST(ST_SCRIPT_BUSY) = 1;
            CB->divers(DV_CUT_SPEECH, 0, 0);
            CB->divers_empile(DV_WAS_HIT, P_ARMAL, 1);
            CB->divers_empile(DV_BLOOD_AT_BONE, 6, 0);
            CB->bloque(BLOQUE_ALL);
            /* 0x9242 */
            if (CB->divers(DV_HP, P_ARMAL, -1) <= 150 || ST(ST_PLAYER_WEAPON) == 27) {
                CB->combat(P_ARMAL, -1);
                ST(V_ENGAGED_PERSONA) &= 0xff;   /* drop P_ARMAL (high byte) */
                ST(ST_VIE(P_ARMAL)) = VIE_DEAD;
                CB->stop_joindre(-P_ARMAL);
                CB->animation("SP06IMP52");
                CB->animation("P06_65535");
                CB->animation("P06CAD");
                CB->pos_perso(P_ARMAL, 0, 0, 0, 2, "P06CAD", 4, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->select_pile(0, 1);
            } else {
                /* 0x932A */
                if ((CB->divers(DV_HP, P_ARMAL, -1) - ST(V_HIT_DAMAGE)) > 0) {
                    CB->divers(DV_HP, P_ARMAL, (CB->divers(DV_HP, P_ARMAL, -1) - ST(V_HIT_DAMAGE)));
                } else {
                    CB->divers(DV_HP, P_ARMAL, 1);
                }
                CB->stop_joindre(-P_ARMAL);
                CB->animation("SP06IMP12");
                CB->animation("P06IMP47");
                CB->animation("P06_65535");
                CB->animation("SP06GAR02_10");
                /* 0x93B9 */
                if (ST(V_EVENT_51) == 0 && (ST(V_ENGAGED_PERSONA) & 0xFF) == 0 && ST(ST_VIE(P_MALROT2)) != VIE_DEAD) {
                    CB->select_pile(0, 2);
                    CB->select_pile(1, 2);
                    CB->select_pile(2, 2);
                    CB->bloque(BLOQUE_ALL);
                    CB->evenement(0, -1);
                    CB->evenement(1, -1);
                    ST(V_EVENT_0) = 0;
                    ST(V_EVENT_1) = 0;
                    CB->modify_var(&ST(V_EVENT_51), 1);
                    CB->modify_var(&ST(L18_ARMAL_FIGHT_REQ), 1);
                    CB->modify_var(&ST(L18_FIGHT_PHASE), 3);
                } else {
                    CB->bloque(BLOQUE_NONE);
                }
                CB->combat(P_ARMAL, 0);
                ST(V_ENGAGED_PERSONA) |= 0x600;   /* engaged: P_ARMAL (high byte) */
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9501 */
            if ((CB->divers(DV_HP, P_ARMAL, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_ARMAL, (CB->divers(DV_HP, P_ARMAL, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_ARMAL, 1);
            }
            CB->animation("SP06GAR02_10");
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->select_pile(0, 1);
        }
    } else if (CB->divers(DV_WAS_HIT, P_ARMAL, 0) && ST(ST_VIE(P_ARMAL)) != VIE_DEAD) {
        CB->divers_empile(DV_WAS_HIT, P_ARMAL, 1);
    }
    /* 0x95D6 */
    if (CB->distance(6, 3) < 600) {
        /* 0x95F0 */
        if (CB->divers(DV_WAS_HIT, P_MALROT2, 0) && ST(ST_VIE(P_MALROT2)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            CB->divers(DV_CUT_SPEECH, 0, 0);
            CB->divers_empile(DV_WAS_HIT, P_MALROT2, 1);
            CB->divers_empile(DV_BLOOD_AT_BONE, 62, 0);
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x9679 */
            if (CB->divers(DV_HP, P_MALROT2, -1) <= 150 || ST(ST_PLAYER_WEAPON) == 27) {
                CB->combat(P_MALROT2, -1);
                ST(V_ENGAGED_PERSONA) &= 0xff00;   /* drop P_MALROT2 (low byte) */
                ST(ST_VIE(P_MALROT2)) = VIE_DEAD;
                /* 0x96C8 */
                if (ST(ST_VIE(P_ARMAL)) != VIE_DEAD) {
                    CB->combat(P_ARMAL, 0);
                    ST(V_ENGAGED_PERSONA) |= 0x600;   /* engaged: P_ARMAL (high byte) */
                }
                CB->stop_joindre(-P_MALROT2);
                CB->animation("SP62IMP52");
                CB->animation("P62_65535");
                CB->animation("P62CAD");
                CB->pos_perso(P_MALROT2, 0, 0, 0, 2, "P62CAD", 3, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->select_pile(0, 1);
            } else {
                CB->stop_joindre(-P_MALROT2);
                CB->animation("SP62IMP12");
                CB->animation("P62IMP47");
                CB->animation("P62_65535");
                CB->animation("SP62GAR02_1");
                /* 0x97AF */
                if (ST(ST_VIE(P_ARMAL)) != VIE_DEAD && ST(L18_ARMAL_ATTACKED) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
                    CB->modify_var(&ST(L18_EVENT52_ARMED), 1);
                    CB->modify_var(&ST(L18_ARMAL_ATTACKED), 1);
                }
                CB->combat(P_MALROT2, 0);
                ST(V_ENGAGED_PERSONA) |= 0x62;   /* engaged: P_MALROT2 (low byte) */
            }
            /* 0x983A */
            if ((CB->divers(DV_HP, P_MALROT2, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_MALROT2, (CB->divers(DV_HP, P_MALROT2, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_MALROT2, 1);
            }
            CB->animation("SP62GAR02_1");
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->select_pile(0, 1);
        }
    } else if (CB->divers(DV_WAS_HIT, P_MALROT2, 0) && ST(ST_VIE(P_MALROT2)) != VIE_DEAD) {
        CB->divers_empile(DV_WAS_HIT, P_MALROT2, 1);
    }
    /* 0x9906 */
    if ((ST(L18_MALROT_CHASE_STAGE) == 0 || ST(L18_MALROT_CHASE_STAGE) == 10) && ST(ST_VIE(P_ARMAL)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(L18_MALROT2_ENCOUNTER_STEP) != 1000) {
        /* 0x995D */
        if (ST(V_EVENT_1) == 1 && ST(L18_ARMAL_PATROL_STEP) == 0 && ST(ST_VIE(P_ARMAL)) == 1) {
            CB->pos_perso(P_ARMAL, 2117, 2000, 0x4650, 1, "P06ATT02", 4, 0);
            ST(V_EVENT_1) = 0;
            CB->select_pile(1, 1);
            CB->joindre(-P_ARMAL, 0, 0, 0x4650, -1);
            ST(V_DLG_RANDOM) = CB->random(2);
            /* 0x9A01 */
            if (ST(V_DLG_RANDOM) == 0) {
                CB->animation("SP06ATT02");
            }
            /* 0x9A1E */
            if (ST(V_DLG_RANDOM) == 1) {
                CB->animation("SP06ATT03");
            }
            CB->animation("P06ATT01_10");
            CB->evenement(1, 5000);
            CB->select_pile(0, 1);
        }
        /* 0x9A60 */
        if (ST(L18_ARMAL_ARRIVAL_LOCK) == 0 && ST(V_ARMAL_STAGE) >= 2 && ST(ST_VIE(P_ARMAL)) == 1 && ST(ST_LOCKED_CAMERA) == 1 && ST(L18_ARMAL_PATROL_STEP) == 0 && ST(L18_ARMAL_ATTACKED) == 0) {
            ST(L18_ARMAL_PATROL_STEP) = 10;
            CB->bloque(BLOQUE_ALL);
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_ARMAL);
            ST(V_EVENT_1) = 0;
            CB->evenement(1, -1);
            CB->vire_col(8, 0);
            CB->joindre(-P_ARMAL, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP06ARR01_10");
            CB->vire_col(8, 1);
            CB->animation("Sar_10");
            CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x9B8D */
        if (ST(V_ARMAL_STAGE) >= 2 && ST(ST_VIE(P_ARMAL)) == 1 && ST(ST_LOCKED_CAMERA) != 1 && ST(L18_ARMAL_PATROL_STEP) != 0 && ST(L18_ARMAL_STANCE_STATE) == 0) {
            ST(L18_ARMAL_PATROL_STEP) = 0;
            ST(L18_ARMAL_ARRIVAL_LOCK) = 0;
            CB->select_pile(1, 1);
            CB->stop_joindre(P_ARMAL);
            CB->pos_perso(P_ARMAL, 2117, 2010, 0x61A8, 1, "P06ARR01", 4, 0);
            CB->joindre(P_ARMAL, 0, 0, 0x4650, -1);
            CB->evenement(1, 2000);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x9C71 */
    if ((ST(L18_MALROT_CHASE_STAGE) == 0 || ST(L18_MALROT_CHASE_STAGE) == 10) && ST(ST_VIE(P_ARMAL)) == 1) {
        /* 0x9CA7 */
        if (ST(V_EVENT_0) == 1 && ST(L18_MALROT2_SEQ_STEP) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT2)) == 1) {
            ST(L18_MALROT2_SEQ_STEP) = 1;
        }
        /* 0x9CFB */
        if (ST(L18_MALROT2_NEAR) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_MALROT2)) == 1) {
            /* 0x9D2E */
            if (ST(V_EVENT_0) != 0 && ST(L18_MALROT2_SEQ_STEP) <= 1) {
                CB->select_pile(2, 1);
                ST(V_EVENT_0) = 0;
                ST(L18_MALROT2_SEQ_STEP) = 1;
                CB->joindre(-P_MALROT2, 2912, 1682, 0xA028, -1);
                CB->modify_var(&ST(L18_MALROT2_SEQ_STEP), 2);
                CB->select_pile(0, 1);
            }
            /* 0x9DC0 */
            if (ST(L18_MALROT2_SEQ_STEP) == 2 && !CB->on_joindre(P_MALROT2)) {
                CB->select_pile(2, 1);
                ST(L18_MALROT2_SEQ_STEP) = 3;
                CB->stop_joindre(-P_MALROT2);
                CB->joindre(-P_MALROT2, 0, 0, 0xA028, -1);
                /* 0x9E25 */
                if (ST(V_EVENT15_STEP) == 0) {
                    ST(V_DLG_RANDOM) = CB->random(2);
                    /* 0x9E4E */
                    if (ST(V_DLG_RANDOM) == 0) {
                        CB->animation("SP62ATT02");
                    }
                    /* 0x9E6B */
                    if (ST(V_DLG_RANDOM) == 1) {
                        CB->animation("SP62ATT04");
                    }
                }
                /* 0x9E88 */
                if (ST(V_EVENT15_STEP) == 1) {
                    ST(V_EVENT15_STEP) = 0;
                    CB->animation("SP62GAR27");
                    ST(V_DLG_RANDOM) = CB->random(2);
                    /* 0x9ECB */
                    if (ST(V_DLG_RANDOM) == 0) {
                        CB->animation("P62ATT02");
                    }
                    /* 0x9EE8 */
                    if (ST(V_DLG_RANDOM) == 1) {
                        CB->animation("P62ATT04");
                    }
                }
                ST(V_DLG_RANDOM) = CB->random(3);
                /* 0x9F1C */
                if (ST(V_DLG_RANDOM) == 0) {
                    CB->evenement(0, 15000);
                }
                /* 0x9F3B */
                if (ST(V_DLG_RANDOM) == 1) {
                    CB->evenement(0, 20000);
                }
                /* 0x9F5A */
                if (ST(V_DLG_RANDOM) == 2) {
                    CB->evenement(0, 30000);
                }
                CB->select_pile(0, 1);
            }
            /* 0x9F86 */
            if (ST(V_EVENT_0) != 0 && ST(L18_MALROT2_SEQ_STEP) == 3) {
                CB->select_pile(2, 1);
                ST(V_EVENT_0) = 0;
                ST(L18_MALROT2_SEQ_STEP) = 4;
                CB->joindre(-P_MALROT2, 2825, 1880, 0xFDE8, -1);
                CB->modify_var(&ST(L18_MALROT2_SEQ_STEP), 5);
                CB->select_pile(0, 1);
            }
            /* 0xA018 */
            if (ST(L18_MALROT2_SEQ_STEP) == 5 && !CB->on_joindre(P_MALROT2)) {
                CB->select_pile(2, 1);
                ST(L18_MALROT2_SEQ_STEP) = 0;
                CB->stop_joindre(-P_MALROT2);
                CB->joindre(-P_MALROT2, 0, 0, 0xFDE8, -1);
                /* 0xA07D */
                if (ST(V_EVENT15_STEP) == 0) {
                    CB->animation("SP62ACT02");
                }
                /* 0xA09A */
                if (ST(V_EVENT15_STEP) == 1) {
                    ST(V_EVENT15_STEP) = 0;
                    CB->animation("SP62GAR27");
                    CB->animation("P62ACT02");
                }
                CB->animation("P62_65535");
                CB->evenement(0, 200);
                CB->select_pile(0, 1);
            }
        }
        /* 0xA0F6 */
        if (ST(ST_VIE(P_MALROT2)) == 1 && CB->distance(3, 6) < 240 && ST(V_ENGAGED_PERSONA) == 0 && ST(L18_MALROT2_NEAR) == 0) {
            /* 0xA144 */
            if (ST(L18_MALROT2_SEQ_STEP) == 2) {
                ST(L18_MALROT2_SEQ_STEP) = 1;
            }
            /* 0xA165 */
            if (ST(L18_MALROT2_SEQ_STEP) == 4) {
                ST(L18_MALROT2_SEQ_STEP) = 3;
            }
            /* 0xA186 */
            if (ST(L18_MALROT2_SEQ_STEP) == 0) {
                ST(L18_MALROT2_SEQ_STEP) = 5;
            }
            /* 0xA1A7 */
            if (ST(L18_MALROT2_SEQ_STEP) == 3) {
                ST(L18_MALROT2_SEQ_STEP) = 2;
            }
            ST(L18_MALROT2_NEAR) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->evenement(0, -1);
            CB->animation("SP62ARR01_10");
            CB->stop_joindre(-P_MALROT2);
            CB->joindre(-P_MALROT2, 2923, 1682, 0x4E20, -1);
            CB->animation("SP62ARR01_10");
            CB->joindre(P_MALROT2, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP62ARR01_10");
            CB->joindre_perm(P_MALROT2, ST(ST_HERO));
            CB->animation("SP62ARR01_10");
            /* 0xA275 */
            if (ST(V_ARMAL_STAGE) == 2 && ST(L18_ARMAL_DIALOGUE_SEEN) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0) {
                ST(L18_ARMAL_DIALOGUE_SEEN) = 1;
                lieu_action_register("L18D190A", P_MALROT2, "SP62DIA62", "SP62ARR01_10");
                lieu_action_register("L18D190B", P_MALROT2, "SP62DIA63", "SP62ARR01_10");
            }
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(L18_MALROT2_ENCOUNTER_STEP), 1);
            return;
        }
        /* 0xA319 */
        if (ST(ST_VIE(P_MALROT2)) == 1 && CB->distance(3, 6) > 400 && ST(V_ENGAGED_PERSONA) == 0 && ST(L18_MALROT2_NEAR) == 1) {
            ST(L18_MALROT2_NEAR) = 0;
            ST(L18_MALROT2_ENCOUNTER_STEP) = 0;
            CB->stop_joindre(-P_MALROT2);
            /* 0xA38D */
            if (ST(L18_MALROT2_SEQ_STEP) <= 1 || ST(L18_MALROT2_SEQ_STEP) == 3) {
                CB->evenement(0, 200);
            }
        }
    }
    /* 0xA3BB */
    if (ST(L18_ARMAL_PATROL_STEP) == 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_ARMAL)) == 1) {
        /* 0xA3EB */
        if (ST(L18_ARMAL_STANCE_STATE) == 0 && ST(ST_COMBAT_STANCE) >= 1 && CB->direction_perso(P_ARMAL) && ST(ST_LOCKED_CAMERA) == 1) {
            ST(L18_ARMAL_STANCE_STATE) = 1;
            ST(L18_ARMAL_PATROL_STEP) = 40;
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_ARMAL);
            CB->animation("SP06GAR26");
            CB->animation("P06GAR02_10");
            CB->dialogue("L18D10", P_ARMAL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xA4C0 */
        if (ST(L18_ARMAL_STANCE_STATE) == 1 && ST(ST_COMBAT_STANCE) == 0) {
            CB->bloque(BLOQUE_ALL);
            ST(L18_ARMAL_STANCE_STATE) = 0;
            ST(L18_ARMAL_PATROL_STEP) = 40;
            CB->stop_joindre(-P_ARMAL);
            CB->animation("SP06GAR27");
            CB->animation("P06ARR01_100");
            CB->dialogue("L18D11", P_ARMAL);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("Sar_10");
            CB->bloque(BLOQUE_NONE);
            /* 0xA559 */
            if (ST(ST_LOCKED_CAMERA) != 1) {
                ST(L18_ARMAL_PATROL_STEP) = 0;
                CB->joindre(-P_ARMAL, 2117, 2000, 0x4650, -1);
                CB->evenement(1, 1000);
            }
            /* 0xA5A3 */
            if (ST(ST_LOCKED_CAMERA) == 1) {
                CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
            }
            return;
        }
        /* 0xA5CB */
        if (ST(L18_ARMAL_STANCE_STATE) == 1 && ST(ST_COMBAT_STANCE) >= 1) {
            ST(L18_ARMAL_STANCE_STATE) = 2;
            ST(V_EVENT_51) = 1;
            ST(L18_ARMAL_FIGHT_REQ) = 1;
        }
    }
    /* 0xA613 */
    if (ST(L18_MALROT2_ENCOUNTER_STEP) == 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_ARMAL)) < 2) {
        /* 0xA646 */
        if (ST(V_EVENT15_STEP) == 0 && ST(ST_COMBAT_STANCE) == 1 && CB->direction_perso(P_MALROT2)) {
            ST(V_EVENT15_STEP) = 1;
            ST(L18_MALROT2_ENCOUNTER_STEP) = 0;
            ST(ST_SCRIPT_BUSY) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_MALROT2);
            CB->animation("SP62GAR26");
            CB->animation("P62GAR02_10");
            CB->dialogue("L18D134", P_MALROT2);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(L18_MALROT2_ENCOUNTER_STEP), 1);
            return;
        }
        /* 0xA70F */
        if (ST(V_EVENT15_STEP) == 1 && ST(ST_COMBAT_STANCE) == 0) {
            ST(V_EVENT15_STEP) = 0;
            ST(L18_MALROT2_ENCOUNTER_STEP) = 0;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_MALROT2);
            CB->animation("SP62GAR27");
            CB->animation("P62ARR01_100");
            CB->dialogue("L18D135", P_MALROT2);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(L18_MALROT2_ENCOUNTER_STEP), 1);
            return;
        }
    }
    /* 0xA7B6 */
    if (ST(L18_ARMAL_PATROL_STEP) == 1) {
        ST(L18_ARMAL_PATROL_STEP) = 2;
        ST(L18_MALROT2_ENCOUNTER_STEP) = 1000;
        CB->animation("SP06ARR01_10");
        CB->joindre_perm(-P_ARMAL, ST(ST_HERO));
    }
    /* 0xA7FE */
    if (ST(L18_MALROT2_ENCOUNTER_STEP) == 1) {
        ST(L18_MALROT2_ENCOUNTER_STEP) = 2;
        CB->joindre_perm(-P_MALROT2, ST(ST_HERO));
    }
    /* 0xA832 */
    if (ST(ST_VIE(P_ARMAL)) == 1 && ST(V_EVENT_51) == 1 && ST(L18_FIGHT_REQ_TRIG) == 1 && ST(L18_FIGHT_PHASE) == 0) {
        ST(V_EVENT_51) = 0;
        ST(L18_FIGHT_PHASE) = 1;
        ST(L18_ARMAL_STANCE_STATE) = 1;
        CB->animation("Sar_10");
        lieu_action_register("L18D118", P_ARMAL, "SP06DIA60", "SP06GAR02_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
        return;
    }
    /* 0xA8FA */
    if (ST(ST_VIE(P_ARMAL)) == 1 && ST(V_EVENT_51) == 1 && (ST(L18_ARMAL_FIGHT_REQ) == 1 || ST(L18_FIGHT_REQ_ZONE) == 1 || (ST(L18_FIGHT_REQ_TRIG) == 1 && ST(L18_FIGHT_PHASE) == 1))) {
        CB->bloque(BLOQUE_ALL);
        CB->select_pile(0, 2);
        ST(L18_ARMAL_FIGHT_REQ) = 2;
        ST(V_ENGAGED_PERSONA) |= 0x662;   /* engaged: P_ARMAL << 8 | P_MALROT2 */
        ST(V_EVENT_51) = 2;
        ST(L18_MALROT2_SEQ_STEP) = 1000;
        ST(V_EVENT_0) = 100;
        ST(V_EVENT_1) = 100;
        ST(L18_MALROT2_SEQ_STEP) = 0;
        CB->stop_joindre(-P_MALROT2);
        CB->stop_joindre(-P_ARMAL);
        CB->joindre(-P_ARMAL, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP06GAR02_10");
        ST(L18_FIGHT_PHASE) = 3;
        CB->evenement(1, -1);
        CB->evenement(0, -1);
        CB->animation("Sar_10");
        lieu_action_register("L18D117", P_ARMAL, "SP06DIA60", "SP06GAR02_10");
        CB->change_camera(-1, CAM_LOCKED);
        CB->retire_zone(1);
        CB->vire_col(11, 1);
        CB->divers(DV_RUN, P_MALROT2, 1);
        /* 0xAAAF */
        if (ST(L18_ARMAL_FIGHT_REQ) == 0) {
            CB->joindre(-ST(ST_HERO), 0, 0, 0xAFC8, -1);
        }
        CB->joindre(-P_MALROT2, 2804, 1983, 0x2890, -1);
        lieu_action_register("L18D06", P_ARMAL, "SP06DIA60", "SP06GAR02_10");
        CB->evenement(51, 500);
    }
    /* 0xAB26 */
    if (ST(V_EVENT_51) == 3 && !CB->on_joindre(P_MALROT2)) {
        ST(V_EVENT_51) = 4;
        CB->stop_joindre(-P_MALROT2);
        CB->animation("SP62ATT01");
        CB->bruitage("L18B01", -1, -1);
        CB->animation("P62ARR01_10");
        CB->change_camera(1, CAM_LOCKED);
        CB->bruitage("L18B02", 2400, 2115);
        CB->play_anim(0, 0);
        CB->play_anim(1, 0);
        CB->play_anim(2, 0);
        CB->evenement(51, 500);
    }
    /* 0xABE6 */
    if (ST(V_EVENT_51) == 5 && ST(L18_FIGHT_PHASE) == 3) {
        ST(V_EVENT_51) = 6;
        CB->divers(DV_RUN, P_MALROT2, 0);
        CB->stop_joindre(-P_MALROT2);
        /* 0xAC36 */
        if (ST(ST_LOCKED_CAMERA) == 0) {
            CB->pos_perso(P_MALROT2, 2825, 1600, 0x4A38, 1, "P62GAR02", 3, 0);
        }
        CB->change_camera(0, CAM_LOCKED);
        CB->play_anim(3, 0);
        CB->pause_anim(0, 1, -1);
        CB->pause_anim(1, 1, -1);
        CB->pause_anim(2, 1, -1);
        CB->modify_var(&ST(L18_DOOR_RETURN_STATE), 318);
        CB->ajoute_zone(9, 2400, 2115, 140, 100, 1);
        CB->change_camera(0, CAM_AUTO);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        CB->combat(P_ARMAL, 0);
        CB->combat(P_MALROT2, 0);
        ST(V_ENGAGED_PERSONA) |= 0x662;   /* engaged: P_ARMAL << 8 | P_MALROT2 */
        return;
    }
    /* 0xAD3A */
    if (ST(ST_VIE(P_ARMAL)) == 1 && ST(V_EVENT_52) == 1 && ST(L18_EVENT52_DONE) == 0 && ST(L18_EVENT52_ARMED) == 1) {
        ST(L18_MALROT2_ENCOUNTER_STEP) = 1000;
        ST(V_EVENT_52) = 0;
        ST(L18_EVENT52_ARMED) = 0;
        ST(V_EVENT15_STEP) = 1;
        ST(L18_EVENT52_DONE) = 1;
        CB->animation("Sar_10");
        CB->animation("SP62GAR02_10");
        lieu_action_register("L18D160", P_MALROT2, "SP62DIA54", "SP62GAR02_10");
        CB->change_camera(1, CAM_LOCKED);
        CB->dialogue("L18D161", P_ARMAL);
        CB->change_camera(0, CAM_AUTO);
        lieu_action_register("L18D136", P_MALROT2, "SP62DIA60", "SP62GAR02_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L18_MALROT2_ENCOUNTER_STEP), 1);
        return;
    }
    /* 0xAE71 */
    if (ST(ST_VIE(P_ARMAL)) == 1 && (ST(L18_ARMAL_ATTACKED) == 1 || (ST(V_EVENT_52) == 1 && (ST(L18_EVENT52_DONE) == 1 || ST(L18_EVENT52_ARMED) == 0)))) {
        ST(V_EVENT_52) = 2;
        ST(L18_MALROT2_ENCOUNTER_STEP) = 1000;
        ST(V_EVENT15_STEP) = 1;
        ST(L18_ARMAL_ATTACKED) = 2;
        ST(V_EVENT_0) = 0;
        ST(V_EVENT_1) = 0;
        CB->bloque(BLOQUE_ALL);
        CB->evenement(0, -1);
        CB->evenement(1, -1);
        ST(L18_EVENT52_DONE) = 1;
        CB->retire_zone(1);
        CB->vire_col(11, 1);
        CB->animation("Sar_10");
        /* 0xAF7F */
        if (ST(L18_EVENT52_ARMED) == 1) {
            CB->animation("Sar_10");
            CB->animation("SP62GAR02_10");
            lieu_action_register("L18D137", P_MALROT2, "SP62DIA54", "SP62GAR02_10");
        }
        CB->change_camera(1, CAM_LOCKED);
        /* 0xAFCD */
        if (ST(L18_ARMAL_PATROL_STEP) != 0) {
            ST(L18_ARMAL_PATROL_STEP) = 5;
            CB->stop_joindre(-P_ARMAL);
        }
        CB->vire_col(8, 0);
        CB->animation("SP06ARR01_10");
        CB->dialogue("L18D10", P_ARMAL);
        CB->joindre_perm(-P_ARMAL, ST(ST_HERO));
        CB->joindre(-P_ARMAL, 2132, 2021, 0x35C, -1);
        CB->animation("P06ATT07");
        /* 0xB05B */
        if (ST(L18_EVENT52_ARMED) == 0) {
            CB->pos_perso(P_MALROT2, 2825, 1600, 0x4A38, 1, "P62GAR02", 3, 0);
        }
        CB->bruitage("L18B01", -1, -1);
        CB->animation("P06ARR01_10");
        CB->bruitage("L18B02", 2400, 2115);
        CB->play_anim(0, 0);
        CB->play_anim(1, 0);
        CB->play_anim(2, 0);
        CB->evenement(52, 500);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0xB100 */
    if (ST(V_EVENT_52) == 3) {
        ST(V_EVENT_52) = 4;
        CB->change_camera(-1, CAM_LOCKED);
        CB->play_anim(3, 0);
        CB->pause_anim(0, 1, -1);
        CB->pause_anim(1, 1, -1);
        CB->pause_anim(2, 1, -1);
        CB->modify_var(&ST(L18_DOOR_RETURN_STATE), 3);
        CB->ajoute_zone(9, 2400, 2115, 140, 100, 1);
        CB->change_camera(0, CAM_AUTO);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->vire_col(8, 1);
        CB->combat(P_ARMAL, 0);
        CB->combat(P_MALROT2, 0);
        ST(V_ENGAGED_PERSONA) |= 0x662;   /* engaged: P_ARMAL << 8 | P_MALROT2 */
        return;
    }
    /* 0xB207 */
    if (ST(V_EVENT_36) == 1) {
        ST(V_EVENT_36) = 0;
        CB->play_anim(7, 0);
        /* 0xB235 */
        if (ST(ST_VIE(P_ARMAL)) == 1) {
            CB->evenement(52, 800);
        }
        return;
    }
    /* 0xB25C */
    if (ST(V_EVENT_33) == 1) {
        ST(L18_ARMAL_PATROL_STEP) = 60;
        CB->stop_joindre(-P_ARMAL);
        CB->vire_col(8, 1);
        ST(V_EVENT_33) = 0;
        ST(L18_INN_MENU_CHOICE) = CB->divers(DV_INN_MENU, 0, 0);
        CB->bruitage("L18AMB1", -1, -1);
    }
    /* 0xB2CB */
    if (ST(L18_INN_MENU_CHOICE) == 2) {
        ST(L18_INN_MENU_CHOICE) = 0;
        CB->divers(DV_SCREEN, 1, 0);
        CB->pos_perso(P_ARMAL, 2117, 2000, 0x4650, 1, "P06ARR01", 4, 0);
        CB->pos_perso(ST(ST_HERO), 1950, 1980, 0xBB80, 1, "ar", 6, 0);
        CB->divers_empile(DV_SCREEN, 3, 0);
        ST(L18_ARMAL_TALKED) = 1;
        ST(L18_INN_RESULT) = 2;
        ST(L18_INN_ACTION) = 2;
        /* 0xB384 */
        if (ST(L18_INN_STATE) == 1) {
            ST(L18_INN_STATE) = 2;
        }
        /* 0xB3A5 */
        if (ST(L18_ARMAL_MOOD) == 0) {
            ST(L18_ARMAL_TALK_MODE) = 4;
            lieu_action_register("L18D120A", P_ARMAL, "SP06DIA60", "SP06ARR01_10");
            lieu_action_register("L18D120B", P_ARMAL, "SP06DIA60", "SP06ARR01_10");
            lieu_action_register("L18D121", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
        }
        /* 0xB411 */
        if (ST(L18_ARMAL_MOOD) == 2) {
            ST(L18_ARMAL_TALK_MODE) = 0;
            ST(L18_ARMAL_MOOD) = 0;
            lieu_action_register("L18D132", P_ARMAL, "SP06DIA53", "SP06ARR01_10");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
        return;
    }
    /* 0xB489 */
    if (ST(L18_INN_MENU_CHOICE) == 1 || ST(L18_INN_MENU_CHOICE) == 3) {
        ST(L18_INN_MENU_CHOICE) = 0;
        CB->divers(DV_SCREEN, 1, 0);
        CB->pos_perso(P_ARMAL, 2117, 2000, 0x4650, 1, "P06ARR01", 4, 0);
        CB->pos_perso(ST(ST_HERO), 1950, 1980, 0xBB80, 1, "ar", 6, 0);
        CB->animation("Sar_100");
        ST(L18_ARMAL_TALK_MODE) = 3;
        ST(L18_INN_RESULT) = 1;
        ST(L18_INN_ACTION) = 1;
        CB->select_pile(1, 1);
        CB->divers_empile(DV_SCREEN, 3, 0);
        /* 0xB56F */
        if (ST(L18_ARMAL_MOOD) == 0) {
            /* 0xB581 */
            if (ST(L18_MAIN_QUEST_STAGE) == 1) {
                lieu_action_register("L18D122A", P_ARMAL, "SP06DIA62", "SP06ARR01_10");
                /* 0xB5AC */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L18D250", P_ARKHAN2, "SG00DIA07", NULL);
                }
                /* 0xB5D4 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L18D278", P_ARKHAN3, "SG00DIA07", NULL);
                }
                lieu_action_register("L18D122B", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
                lieu_action_register("L18D122C", P_ARMAL, "SP06DIA53", "SP06ARR01_10");
                lieu_action_register("L18D122D", P_ARMAL, "SP06DIA59", "SP06ARR01_10");
                lieu_action_register("L18D122E", P_ARMAL, "SP06DIA55", "SP06ARR01_10");
                lieu_action_register("L18D122F", P_ARMAL, "SP06DIA60", "SP06ARR01_10");
                lieu_action_register("L18D122G", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
                CB->animation("P06ARR01_800");
                lieu_action_register("L18D122H", P_ARMAL, "P06DIA57", "SP06ARR01_10");
                lieu_action_register("L18D122I", P_ARMAL, "SP06DIA59", "SP06ARR01_10");
                lieu_action_register("L18D122J", P_ARMAL, "SP06DIA60", "SP06ARR01_10");
                lieu_action_register("L18D122K", P_ARMAL, "SP06DIA57", "SP06ARR01_10");
                lieu_action_register("L18D122L", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
                lieu_action_register("L18D122M", P_ARMAL, "SP06DIA59", "SP06ARR01_10");
                lieu_action_register("L18D122N", P_ARMAL, "SP06DIA55", "SP06ARR01_10");
            }
            /* 0xB74C */
            if (ST(L18_MAIN_QUEST_STAGE) == 5) {
                ST(L18_ARMAL_L18D123_SEEN) = 1;
                lieu_action_register("L18D123A", P_ARMAL, "SP06ATT60", "SP06ARR01_10");
                lieu_action_register("L18D123B", P_ARMAL, "SP06DIA53", "SP06ARR01_10");
                lieu_action_register("L18D123C", P_ARMAL, "SP06DIA59", "SP06ARR01_10");
                lieu_action_register("L18D123D", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
                lieu_action_register("L18D123E", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
                lieu_action_register("L18D123F", P_ARMAL, "SP06DIA64", "SP06ARR01_10");
                /* 0xB803 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L18D251A", P_ARKHAN2, "SG00DIA07", NULL);
                    lieu_action_register("L18D251B", P_ARKHAN2, "SG00DIA07", NULL);
                }
                /* 0xB841 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L18D252", P_ARKHAN3, "SG00DIA07", NULL);
                }
                lieu_action_register("L18D253A", P_ARMAL, "SP06ATT60", "SP06ARR01_10");
                lieu_action_register("L18D253B", P_ARMAL, "SP06DIA53", "SP06ARR01_10");
                lieu_action_register("L18D253C", P_ARMAL, "SP06DIA53", "SP06ARR01_10");
                ST(L18_MAIN_QUEST_STAGE) = 6;
                /* 0xB8C3 */
                if (ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(L18_SORDOS_LOOTED) == 0) {
                    ST(L18_SORDOS_LOOTED) = 1;
                    /* 0xB8F6 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L18D279A", P_ARKHAN2, "SG00DIA07", NULL);
                        lieu_action_register("L18D279B", P_ARKHAN2, "SG00DIA01", NULL);
                        lieu_action_register("L18D279C", P_ARKHAN2, "SG00DIA08", NULL);
                    }
                    /* 0xB94A */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L18D280A", P_ARKHAN3, "SG00DIA04", NULL);
                        lieu_action_register("L18D280B", P_ARKHAN3, "SG00DIA08", NULL);
                    }
                    lieu_action_register("L18D281A", P_ARMAL, "SP06DIA58", "SP06ARR01_10");
                    lieu_action_register("L18D281B", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
                    lieu_action_register("L18D281C", P_ARMAL, "SP06DIA57", "SP06ARR01_10");
                    lieu_action_register("L18D281D", P_ARMAL, "SP06DIA56", "SP06ARR01_10");
                    /* 0xB9EC */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L18D282", P_ARKHAN2, "SG00DIA01", NULL);
                    }
                    /* 0xBA14 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L18D283", P_ARKHAN3, "SG00DIA01", NULL);
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
                    return;
                }
            }
        }
        /* 0xBA6E */
        if (ST(L18_ARMAL_MOOD) == 2) {
            ST(L18_ARMAL_TALK_MODE) = 0;
            ST(L18_ARMAL_MOOD) = 0;
            ST(L18_ARMAL_ANGER_COUNT)++;
            lieu_action_register("L18D254A", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
            lieu_action_register("L18D254B", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
            lieu_action_register("L18D254C", P_ARMAL, "SP06DIA60", "SP06ARR01_10");
            CB->animation("P06_65535");
            ST(V_DLG_RANDOM) = CB->random(3);
            /* 0xBB16 */
            if (ST(V_DLG_RANDOM) == 0) {
                CB->etat_objet("SOB72", -1, -1, -1, -1, -1, -1, -3, 0);
            }
            /* 0xBB46 */
            if (ST(V_DLG_RANDOM) > 0) {
                CB->etat_objet("SOB73", -1, -1, -1, -1, -1, -1, -3, 0);
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
        CB->select_pile(0, 1);
        return;
    }
    /* 0xBBB5 */
    if (ST(ST_LOCKED_CAMERA) == 1 && ST(L18_DOOR_RETURN_STATE) != 2 && ST(L18_DOOR_RETURN_STATE) != 0 && ST(L18_DOOR_RETURN_STATE) != 10) {
        ST(L18_DOOR_RETURN_STATE) = 2;
        ST(L18_CAMERA_ZONE_STATE) = 1;
        CB->pause_anim(3, 1, -1);
        CB->play_anim(9, 0);
    }
    /* 0xBC3A */
    if (ST(ST_LOCKED_CAMERA) == 0 && ST(L18_DOOR_RETURN_STATE) != 3 && ST(L18_DOOR_RETURN_STATE) != 0 && ST(L18_DOOR_RETURN_STATE) != 10) {
        ST(L18_DOOR_RETURN_STATE) = 3;
        ST(L18_CAMERA_ZONE_STATE) = 2;
        CB->pause_anim(9, 1, -1);
        CB->play_anim(3, 0);
    }
    /* 0xBCBF */
    if (ST(ST_LOCKED_CAMERA) == 2 && ST(L18_DOOR_RETURN_STATE) != 1 && ST(L18_DOOR_RETURN_STATE) != 0 && ST(L18_DOOR_RETURN_STATE) != 10) {
        ST(L18_DOOR_RETURN_STATE) = 1;
        /* 0xBD16 */
        if (ST(L18_CAMERA_ZONE_STATE) == 1) {
            CB->pause_anim(9, 1, -1);
        }
        /* 0xBD3A */
        if (ST(L18_CAMERA_ZONE_STATE) == 2) {
            CB->pause_anim(3, 1, -1);
        }
    }
    /* 0xBD5E */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        CB->retire_zone(9);
        CB->pause_anim(5, 1, -1);
        CB->pause_anim(6, 1, -1);
        CB->pause_anim(4, 1, -1);
        CB->change_camera(1, CAM_AUTO);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->retire_zone(9);
        CB->vire_col(11, 0);
        CB->ajoute_zone(1, 2400, 2115, 140, 80, 1);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xBE2F */
    if (ST(V_EVENT_35) == 1) {
        ST(V_EVENT_35) = 0;
        CB->pause_anim(0, 1, -1);
        CB->pause_anim(1, 1, -1);
        CB->pause_anim(2, 1, -1);
        CB->vire_col(11, 1);
        CB->retire_zone(2);
        CB->ajoute_zone(9, 2400, 2115, 140, 100, 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 100);
        CB->change_camera(1, CAM_AUTO);
        /* 0xBEDD */
        if (ST(L18_DOOR_ZONE_PRESSED) == 1) {
            CB->play_anim(3, 0);
        }
        /* 0xBEF9 */
        if (ST(L18_DOOR_ZONE_PRESSED) == 2) {
            CB->play_anim(9, 0);
        }
        CB->animation("Sar_10");
        CB->retire_zone(1);
        /* 0xBF2B */
        if (ST(L18_DOOR_ZONE_PRESSED) == 1) {
            CB->modify_var(&ST(L18_DOOR_RETURN_STATE), 3);
        }
        /* 0xBF51 */
        if (ST(L18_DOOR_ZONE_PRESSED) == 2) {
            CB->modify_var(&ST(L18_DOOR_RETURN_STATE), 2);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xBF98 */
    if (ST(L18_CHEST_STAGE) == 0 && ST(ST_LOCKED_CAMERA) == 2 && ST(L18_CHEST_ANIM_ON) == 0) {
        ST(L18_CHEST_ANIM_ON) = 1;
        CB->play_anim(8, 1);
    }
    /* 0xBFED */
    if (ST(L18_CHEST_STAGE) == 0 && ST(ST_LOCKED_CAMERA) != 2 && ST(L18_CHEST_ANIM_ON) == 1) {
        ST(L18_CHEST_ANIM_ON) = 0;
        CB->pause_anim(8, 1, -1);
    }
    /* 0xC044 */
    if (ST(L18_CHEST_STAGE) == 1 && ST(ST_LOCKED_CAMERA) != 2) {
        ST(L18_CHEST_STAGE) = 2;
        CB->pause_anim(7, 1, -1);
    }
    /* 0xC089 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        /* 0xC0AA */
        if (ST(ST_VIE(P_MALROT2)) == 1) {
            CB->select_pile(2, 1);
            ST(L18_MALROT2_ENCOUNTER_STEP) = 0;
            CB->stop_joindre(-P_MALROT2);
            lieu_action_register("L18D175", P_MALROT2, "SP62DIA59", "SP62ARR01_10");
        }
        CB->select_pile(0, 1);
        /* 0xC109 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            /* 0xC123 */
            if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0xC135 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP02ACT07");
                }
                /* 0xC152 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP02ACT36");
                }
            }
            /* 0xC177 */
            if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0xC189 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP02ACT35");
                }
                /* 0xC1A6 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP02ACT34");
                }
            }
        }
        /* 0xC1C3 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            /* 0xC1DD */
            if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0xC1EF */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP03ACT07");
                }
                /* 0xC20C */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP03ACT36");
                }
            }
            /* 0xC231 */
            if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0xC243 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->animation("SP03ACT35");
                }
                /* 0xC260 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->animation("SP03ACT34");
                }
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bruitage("L08B05", 2740, 1983);
        CB->animation("ar_10");
        CB->bloque(BLOQUE_NONE);
        /* 0xC2B6 */
        if (ST(ST_VIE(P_MALROT2)) == 1) {
            CB->modify_var(&ST(L18_MALROT2_ENCOUNTER_STEP), 1);
        }
        return;
    }
    /* 0xC2E1 */
    if (ST(V_EVENT_37) == 1 && !CB->on_joindre(ST(ST_HERO))) {
        ST(V_WEAPON_WARNED) = 1;
        ST(V_EVENT_37) = 0;
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-ST(ST_HERO));
        CB->animation("SG00PRE80");
        /* 0xC355 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->animation("P02_65535");
        }
        /* 0xC372 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->animation("P03_65535");
        }
        CB->joindre(-P_MALROT6, 0, 0, 0, ST(ST_HERO));
        CB->etat_objet("SOB73", -1, -1, -1, -1, -1, -1, -3, 0);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_MALROT6);
        lieu_action_register("L18D195A", P_MALROT6, "SP66DIA54", "SP66ARR01_10");
        CB->joindre(-P_MALROT6, 2261, 1835, 0, -1);
        CB->evenement(38, 200);
    }
    /* 0xC429 */
    if (ST(V_EVENT_38) == 1 && !CB->on_joindre(P_MALROT6)) {
        ST(V_EVENT_38) = 0;
        CB->joindre(-P_MALROT6, 0, 0, 0, ST(ST_HERO));
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_MALROT6);
        CB->joindre(-P_ARMAL, 0, 0, 0, P_MALROT6);
        lieu_action_register("L18D195B", P_MALROT6, "SP66DIA59", "SP66ARR01_10");
        CB->divers(DV_RUN, P_MALROT6, 1);
        CB->joindre(-P_MALROT6, 2380, 2500, 0, -1);
        CB->modify_var(&ST(L18_MALROT_CHASE_STAGE), 6);
    }
    /* 0xC505 */
    if (ST(L18_MALROT_CHASE_STAGE) == 6 && !CB->on_joindre(P_MALROT6)) {
        ST(L18_MALROT_CHASE_STAGE) = 7;
        CB->joindre(-P_ARMAL, 0, 0, 0, P_MALROT6);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_ARMAL);
        lieu_action_register("L18D196", P_ARMAL, "SP06DIA54", "SP06ARR01_10");
        CB->joindre_perm(P_ARMAL, ST(ST_HERO));
        CB->joindre_perm(ST(ST_HERO), P_ARMAL);
        lieu_action_register("L18D197", P_ARMAL, "SP06DIA62", "SP06ARR01_10");
        CB->pos_perso(P_MALROT6, 2380, 2500, 0, 4, "P66ARR01", 0x11, 0);
        CB->pos_perso(P_MALROT2, 2825, 1880, 0x4A38, 1, "P62ARR01", 3, 0);
        CB->evenement(0, 2000);
        CB->change_camera(1, CAM_AUTO);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
        CB->modify_var(&ST(L18_MALROT_CHASE_STAGE), 10);
        CB->modify_var(&ST(L18_ARMAL_PATROL_STEP), 1);
    }
    /* 0xC69F */
    if ((CB->divers(DV_DESC_POS, ST(ST_HERO), 0) > 2040 || CB->distance(35, 6) > 100) && ST(L18_ZONE2_REMOVED) == 1) {
        ST(L18_ZONE2_REMOVED) = 0;
        CB->ajoute_zone(2, 1975, 1975, 100, 100, 1);
    }
    /* 0xC706 */
    if (ST(ST_VIE(P_ARMAL)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) & 0xFF00) != 0) {
        ST(V_ENGAGED_PERSONA) &= 0xff;   /* drop P_ARMAL (high byte) */
    }
    /* 0xC733 */
    if (ST(ST_VIE(P_MALROT2)) == VIE_DEAD && (ST(V_ENGAGED_PERSONA) & 0xFF) != 0) {
        ST(V_ENGAGED_PERSONA) &= 0xff00;   /* drop P_MALROT2 (low byte) */
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x10016D7A).
 * Callback ingestion (0x10016A0A..0x10016D79), the trigger-table fill and
 * the unused dword_1002AD?? = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu18_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x10016FB2);
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
        l18_trig00();   /* 0x10016FB9: call sub_10001078 -> sub_10007400 */
        return 0;
    }
    /* 0x10016E9E — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1001700C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x1000104B -> 0x100169F0).
 * Jump table @ 0x10017089, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu18_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu18: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l18_trigger(arg);             break;  /* dword_1002AC70[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
