/*
 * lieu30.c - Native hand-port of LIEU30.DLL (level-30 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu30_trig.c (reached through
 * l30_trigger, see lieu30_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x1000CA30, the state pointer global is dword_1001C9AC and the
 * trigger table is dword_1001C8C0[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000D0FC), lieu_pre_scene (sub_1000D2AC) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are the same as LIEU26. tick_body (sub_1000818E) is level-30
 * specific. The only DLL .data global the level code touches is the debug
 * sprintf buffer, which is reset at INIT.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu30_int.h"

static c8 g_txt[256];   /* debug sprintf buffer, dword_1001C6C0 */

/* -------------------------------------------------------------------------
 * Per-frame body - sub_1000818E (thunk 0x100010B9), level-30 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    i32 t1 = CB->divers(DV_DESC_POS, ST(ST_HERO), 3);
    snprintf(g_txt, sizeof(g_txt), "y:%d", t1);
    CB->print_msg(g_txt, 0, 240);
    /* 0x81DD */
    if (ST(ST_HERO) == 4 && ST(L30_DEATH_HANDLED) == 0) {
        ST(L30_DEATH_HANDLED) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x8220 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x8251 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x827A */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x82A6 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x82D2 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x82F6 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x8313 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x833F */
    if (ST(L30_CAMERA_SWITCH_PENDING) == 1 && ST(ST_HERO) != 4 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < -2830) {
        CB->select_pile(0, 2);
        ST(L30_CAMERA_SWITCH_PENDING) = 0;
        ST(ST_SCRIPT_BUSY) = 0;
        CB->bloque(BLOQUE_NONE);
        CB->change_piece(35, 0);
        return;
    }
    /* 0x83C8 */
    if (ST(V_KONKALITE2_FIGHT) < 60) {
        /* 0x83DA */
        if (ST(ST_VIE(P_KONKALITE2)) == VIE_DEAD && ST(L30_KONKALITE2_DEAD_HANDLED) == 0) {
            ST(L30_KONKALITE2_DEAD_HANDLED) = 1;
            CB->vire_col(39, 0);
        }
        /* 0x841A */
        if (ST(L30_CAMERA_AREA_STATE) == 1 && ST(ST_LOCKED_CAMERA) == 3 && ST(V_KONKALITE2_FIGHT) != 56) {
            ST(L30_CAMERA_AREA_STATE) = 2;
            ST(L30_KONKALITE2_SAVED_LIFE) = ST(ST_VIE(P_KONKALITE2));
            CB->pos_perso(P_KONKALITE2, -1, -1, -1, 4, "", -1, 0);
            /* 0x8495 */
            if (ST(L30_KONKALITE5_BEATEN) != 2) {
                CB->pos_perso(P_KONKALITE5, 5042, 3740, 0xE753, 1, "P75GAR03", 2, 0);
            } else {
                CB->pos_perso(P_KONKALITE5, 0, 0, 0, 2, "P75CAD", 2, 0);
            }
            /* 0x84ED */
            if (ST(L30_KONKALITE94_BEATEN) != 2) {
                CB->pos_perso(94, 4750, 3731, 0xD051, 1, "P94GAR03", 3, 0);
            } else {
                CB->pos_perso(94, 0, 0, 0, 2, "P94CAD", 3, 0);
            }
            CB->select_pile(0, 1);
        }
        /* 0x8552 */
        if (ST(L30_CAMERA_AREA_STATE) == 2 && ST(ST_LOCKED_CAMERA) == 4 && ST(V_KONKALITE2_FIGHT) != 56) {
            ST(L30_CAMERA_AREA_STATE) = 1;
            ST(L30_CAMERA_AREA_VISITED) = 1;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->pos_perso(P_KONKALITE5, -1, -1, -1, 4, "", -1, 0);
            CB->pos_perso(94, -1, -1, -1, 4, "", -1, 0);
            /* 0x85F0 */
            if (ST(L30_KONKALITE2_SAVED_LIFE) < 2) {
                CB->pos_perso(P_KONKALITE2, 4375, 5873, 0x937F, 1, "P72GAR03", 0xA, 0);
                CB->combat(P_KONKALITE2, 1);
            } else {
                CB->pos_perso(P_KONKALITE2, 0, 0, 0, 2, "P72CAD", 0xA, 0);
            }
        }
        /* 0x8658 */
        if (ST(L30_CAMERA_AREA_VISITED) == 0 && ST(L30_CAMERA_AREA_STATE) != 1 && ST(ST_LOCKED_CAMERA) == 4 && ST(V_KONKALITE2_FIGHT) != 56) {
            ST(L30_CAMERA_AREA_VISITED) = 1;
            /* 0x86AF */
            if (ST(ST_VIE(P_KONKALITE5)) != VIE_DEAD) {
                CB->select_pile(1, 1);
                CB->combat(P_KONKALITE5, -1);
                CB->stop_joindre(-P_KONKALITE5);
                CB->pos_perso(P_KONKALITE5, 5042, 3740, 0xE753, 1, "P75GAR03", 2, 0);
            }
            /* 0x8711 */
            if (ST(ST_VIE(94)) != VIE_DEAD) {
                CB->select_pile(2, 1);
                CB->combat(94, -1);
                CB->stop_joindre(-94);
                CB->pos_perso(94, 4750, 3731, 0xD051, 1, "P94GAR03", 3, 0);
            }
            ST(V_ENGAGED_PERSONA) = 0;
            CB->select_pile(0, 1);
        }
        /* 0x878C */
        if (ST(L30_CAMERA_AREA_VISITED) == 1 && ST(ST_LOCKED_CAMERA) == 2 && ST(V_KONKALITE2_FIGHT) != 56) {
            ST(L30_CAMERA_AREA_VISITED) = 0;
            /* 0x87D1 */
            if (ST(ST_VIE(P_KONKALITE5)) != VIE_DEAD) {
                CB->select_pile(1, 1);
                ST(V_ENGAGED_PERSONA) = P_KONKALITE5;
                CB->combat(P_KONKALITE5, 0);
            }
            /* 0x880C */
            if (ST(ST_VIE(94)) != VIE_DEAD) {
                CB->select_pile(2, 1);
                /* 0x882B */
                if (ST(V_ENGAGED_PERSONA) == P_KONKALITE5) {
                    ST(V_ENGAGED_PERSONA) = 0x1DAA;
                } else {
                    ST(V_ENGAGED_PERSONA) = 94;
                }
                CB->combat(94, 0);
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0x8874 */
    if (ST(V_KONKALITE2_FIGHT) == 1) {
        /* 0x8883 */
        if (ST(L30_CAMERA_NEAR_ONCE) == 0 && CB->distance(1, 2) < 1200 && ST(ST_LOCKED_CAMERA) == 0) {
            ST(L30_CAMERA_NEAR_ONCE) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->bruitage("L30B02", 2852, 2762);
            CB->evenement(1, 1500);
        }
        /* 0x88FE */
        if (ST(V_EVENT_1) == 1) {
            ST(V_EVENT_1) = 0;
            /* 0x891F */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L30D05", P_ARKHAN2);
            }
            /* 0x8941 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L30D06", P_ARKHAN3);
            }
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x8968 */
        if (ST(L30_KONKALITE5_ALERT_ONCE) == 0 && CB->distance(1, 2) <= 500 && ST(ST_LOCKED_CAMERA) == 1 && ST(ST_VIE(P_KONKALITE5)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L30_KONKALITE5_ALERT_ONCE) = 1;
            CB->select_pile(2, 1);
            CB->combat(94, 0);
            CB->combat(P_KONKALITE5, 0);
            CB->select_pile(1, 1);
            CB->dialogue("L30D01", P_KONKALITE5);
            ST(V_ENGAGED_PERSONA) = 0x1DAA;
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x8A3C */
    if (ST(V_KONKALITE2_FIGHT) < 60 && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x8A5D */
        if (ST(ST_VIE(P_KONKALITE5)) == VIE_DEAD && ST(L30_KONKALITE5_BEATEN) == 0) {
            ST(L30_KONKALITE5_BEATEN) = 2;
            ST(V_ENGAGED_PERSONA) = 94;
        }
        /* 0x8A9C */
        if (ST(ST_VIE(94)) == VIE_DEAD && ST(L30_KONKALITE94_BEATEN) == 0) {
            ST(L30_KONKALITE94_BEATEN) = 2;
            ST(V_ENGAGED_PERSONA) = P_KONKALITE5;
        }
        /* 0x8ADB */
        if (ST(ST_VIE(P_KONKALITE5)) == VIE_DEAD && ST(ST_VIE(94)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(V_KONKALITE2_FIGHT) = 56;
        }
    }
    /* 0x8B29 */
    if (ST(V_KONKALITE2_FIGHT) < 60) {
        /* 0x8B43 */
        if (CB->divers(DV_WAS_HIT, P_KONKALITE5, 0) && ST(ST_VIE(P_KONKALITE5)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, P_KONKALITE5, 1);
            CB->divers(DV_CUT_SPEECH, 0, 0);
            CB->combat(P_KONKALITE5, -1);
            /* 0x8BA5 */
            if (ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = 0x1DAA;
            }
            /* 0x8BD7 */
            if ((CB->divers(DV_HP, P_KONKALITE5, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_KONKALITE5, (CB->divers(DV_HP, P_KONKALITE5, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_KONKALITE5, 1);
            }
            /* 0x8C38 */
            if (CB->divers(DV_HP, P_KONKALITE5, -1) <= 1) {
                ST(ST_VIE(P_KONKALITE5)) = VIE_DEAD;
                CB->stop_joindre(-P_KONKALITE5);
                CB->divers_empile(DV_BLOOD_AT_BONE, 75, 0);
                CB->animation("SP75IMP53");
                CB->animation("P75CAD");
                CB->pos_perso(P_KONKALITE5, 0, 0, 0, 2, "P75CAD", 2, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_KONKALITE5);
                CB->divers_empile(DV_BLOOD_AT_BONE, 75, 0);
                CB->animation("SP75IMP13");
                CB->animation("P75IMP48");
                CB->animation("P75GAR03");
                /* 0x8CFD */
                if (ST(V_ENGAGED_PERSONA) != 0x1DAA) {
                    CB->combat(P_KONKALITE5, 0);
                    CB->select_pile(2, 1);
                    CB->combat(94, 0);
                    CB->select_pile(1, 1);
                    ST(V_ENGAGED_PERSONA) = 0x1DAA;
                } else {
                    CB->combat(P_KONKALITE5, 0);
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0x8D7C */
        if (CB->divers(DV_WAS_HIT, 94, 0) && ST(ST_VIE(94)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            CB->divers_empile(DV_WAS_HIT, 94, 1);
            CB->divers(DV_CUT_SPEECH, 0, 0);
            CB->combat(94, -1);
            /* 0x8DDE */
            if (ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = 0x1DAA;
            }
            /* 0x8E10 */
            if ((CB->divers(DV_HP, 94, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, 94, (CB->divers(DV_HP, 94, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, 94, 1);
            }
            /* 0x8E71 */
            if (CB->divers(DV_HP, 94, -1) <= 1) {
                ST(ST_VIE(94)) = VIE_DEAD;
                CB->stop_joindre(-94);
                CB->divers_empile(DV_BLOOD_AT_BONE, 94, 0);
                CB->animation("SP94IMP53");
                CB->animation("P94CAD");
                CB->pos_perso(94, 0, 0, 0, 2, "P94CAD", 3, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-94);
                CB->divers_empile(DV_BLOOD_AT_BONE, 94, 0);
                CB->animation("SP94IMP13");
                CB->animation("P94IMP48");
                CB->animation("P94GAR03");
                /* 0x8F36 */
                if (ST(V_ENGAGED_PERSONA) != 0x1DAA) {
                    CB->combat(94, 0);
                    CB->select_pile(1, 1);
                    CB->combat(P_KONKALITE5, 0);
                    CB->select_pile(0, 1);
                    ST(V_ENGAGED_PERSONA) = 0x1DAA;
                } else {
                    CB->combat(94, 0);
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0x8FB2 */
        if (CB->divers(DV_WAS_HIT, P_KONKALITE2, 0) && ST(ST_VIE(P_KONKALITE2)) != VIE_DEAD) {
            CB->select_pile(2, 2);
            CB->divers_empile(DV_WAS_HIT, P_KONKALITE2, 1);
            CB->divers(DV_CUT_SPEECH, 0, 0);
            CB->combat(P_KONKALITE2, -1);
            /* 0x902B */
            if ((CB->divers(DV_HP, P_KONKALITE2, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, P_KONKALITE2, (CB->divers(DV_HP, P_KONKALITE2, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, P_KONKALITE2, 1);
            }
            /* 0x908C */
            if (CB->divers(DV_HP, P_KONKALITE2, -1) <= 1) {
                ST(ST_VIE(P_KONKALITE2)) = VIE_DEAD;
                CB->stop_joindre(-P_KONKALITE2);
                CB->divers_empile(DV_BLOOD_AT_BONE, 72, 0);
                CB->animation("SP72IMP53");
                CB->animation("P72CAD");
                CB->pos_perso(P_KONKALITE2, 0, 0, 0, 2, "P72CAD", 0xA, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-P_KONKALITE2);
                CB->divers_empile(DV_BLOOD_AT_BONE, 72, 0);
                CB->animation("SP72IMP13");
                CB->animation("P72IMP48");
                CB->animation("P72GAR03");
                CB->combat(P_KONKALITE2, 1);
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0x916E */
    if (ST(V_KONKALITE2_FIGHT) < 60) {
        /* 0x9180 */
        if (ST(L30_CAMERA_AREA_STATE) == 0 && ST(ST_LOCKED_CAMERA) == 5) {
            ST(L30_CAMERA_AREA_STATE) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->evenement(0, 200);
        }
        /* 0x91CB */
        if (ST(V_EVENT_0) == 1) {
            ST(V_EVENT_0) = 0;
            CB->divers_empile(DV_SCREEN, 5, 0);
            CB->pos_perso(P_KONKALITE5, -1, -1, -1, 4, "", -1, 0);
            CB->pos_perso(94, -1, -1, -1, 4, "", -1, 0);
            CB->cinematique("SC12_500");
            CB->pos_perso(P_KONKALITE2, 4281, 5830, 0xB743, 1, "P72ARR01", 0xA, 0);
            CB->pos_perso(P_GORK, 3765, 6813, 0xDE85, 1, "P31ARR01", 0xB, 0);
            CB->change_camera(5, CAM_LOCKED);
            CB->change_camera(5, CAM_AUTO);
            CB->joindre(-P_GORK, 4672, 7158, -1, -1);
            CB->divers_empile(DV_SCREEN, 3, 0);
            CB->joindre(-P_KONKALITE2, 4375, 5873, -1, -1);
            CB->animation("SP72ARR01");
            CB->joindre(P_KONKALITE2, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP72GAR26");
            CB->modify_var(&ST(L30_GORK_WALK_STEP), 1);
            lieu_action_register("L30D02", P_KONKALITE2, "P72GAR03", "SP72GAR03");
            CB->combat(P_KONKALITE2, 1);
            CB->vire_col(39, 1);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x9380 */
        if (ST(L30_GORK_WALK_STEP) == 1 && !CB->on_joindre(P_GORK)) {
            ST(L30_GORK_WALK_STEP) = 0;
            CB->select_pile(1, 1);
            CB->pos_perso(P_GORK, -1, -1, -1, 4, "", -1, 0);
            CB->select_pile(0, 1);
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000CDC9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001CA2C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu30_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000CFF2);
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
        l30_trig00();   /* 0x1000CFF9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000CEDE - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000CA30).
 * Jump table @ 0x1000D0C9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu30_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu30: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        g_txt[0] = '\0';
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l30_trigger(arg);             break;  /* dword_1001C8C0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
