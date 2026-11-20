/*
 * lieu37.c — Native hand-port of LIEU37.DLL (level-37 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu37_trig.c (reached through
 * l37_trigger, see lieu37_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x1000B060, the state
 * pointer global is dword_1001B8EC and the trigger table is
 * dword_1001B800[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000B72C), lieu_pre_scene (sub_1000B8DC) and
 * sub_10001200 (float reset) are the shared lieu_common.c. The tick body
 * (sub_10007F21) is level-37 specific.
 *
 * V_ENGAGED_PERSONA holds two packed decimal persona ids here: 3938 =
 * 39 * 100 + 38, both guards engaged.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu37_int.h"

/* Global sprintf buffer of the debug overlay (DLL buffer at 0x1001B600). */
static c8 g_txt[256];

/* -------------------------------------------------------------------------
 * Per-frame body — sub_10007F21 (thunk 0x100010B9), level-37 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x7F33 */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(44, 1500);
    }
    /* 0x7F76 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        ST(V_DEATH_EVENT_SENT) = 1;
        /* 0x7FB1 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x7FCE */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x8002 */
    if (CB->divers(DV_WAS_HIT, 39, 0) && ST(ST_VIE(39)) != VIE_DEAD) {
        /* 0x8026 */
        if (ST(L37_FIGHT_STATE) == 1) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, 39, 1);
            CB->combat(39, -1);
            /* 0x807E */
            if ((CB->divers(DV_HP, 39, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, 39, (CB->divers(DV_HP, 39, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, 39, 1);
            }
            /* 0x80DF */
            if (CB->divers(DV_HP, 39, -1) <= 1) {
                ST(ST_VIE(39)) = VIE_DEAD;
                CB->stop_joindre(-39);
                CB->divers_empile(DV_BLOOD_AT_BONE, 39, 0);
                CB->animation("SP39IMP51");
                CB->animation("P39CAD");
                CB->pos_perso(39, 0, 0, 0, 2, "P39CAD", 8, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->stop_joindre(-39);
                CB->divers_empile(DV_BLOOD_AT_BONE, 39, 0);
                CB->animation("SP39IMP01");
                CB->animation("P39GAR01");
                /* 0x8196 */
                if (ST(L37_TRUCE_38) != 2 && ST(L37_TRUCE_39) != 1) {
                    CB->combat(39, 0);
                }
            }
            CB->select_pile(0, 1);
        } else if (ST(L37_P39_HIT_STATE) == 0) {
            CB->select_pile(1, 2);
            ST(L37_P39_HIT_STATE) = 99;
            CB->divers_empile(DV_WAS_HIT, 39, 1);
            CB->divers_empile(DV_BLOOD_AT_BONE, 39, 0);
            /* 0x823F */
            if ((CB->divers(DV_HP, 39, -1) - ST(V_HIT_DAMAGE)) > 0) {
                CB->divers(DV_HP, 39, (CB->divers(DV_HP, 39, -1) - ST(V_HIT_DAMAGE)));
            } else {
                CB->divers(DV_HP, 39, 1);
            }
            CB->animation("SP39IMP01");
            CB->modify_var(&ST(L37_P39_HIT_STATE), 1);
            CB->animation("P39GAR01");
            CB->select_pile(0, 1);
        }
    }
    snprintf(g_txt, sizeof(g_txt), "contamination:%d", (1023 - CB->divers(DV_DARKNESS, -1, 0)));
    CB->print_msg(g_txt, 0, 248);
    /* 0x8317 */
    if (CB->divers(DV_WAS_HIT, 38, 0) && ST(ST_VIE(38)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, 38, 1);
        CB->combat(38, -1);
        /* 0x8381 */
        if ((CB->divers(DV_HP, 38, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, 38, (CB->divers(DV_HP, 38, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, 38, 1);
        }
        /* 0x83E2 */
        if (CB->divers(DV_HP, 38, -1) <= 1) {
            ST(ST_VIE(38)) = VIE_DEAD;
            CB->stop_joindre(-38);
            CB->divers_empile(DV_BLOOD_AT_BONE, 38, 0);
            CB->animation("SP38IMP51");
            CB->animation("P38CAD");
            CB->pos_perso(38, 0, 0, 0, 2, "P39CAD", 5, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
        } else {
            CB->stop_joindre(-38);
            CB->divers_empile(DV_BLOOD_AT_BONE, 38, 0);
            CB->animation("SP38IMP01");
            CB->animation("P38GAR01");
            /* 0x8499 */
            if (ST(L37_TRUCE_38) != 2 && ST(L37_TRUCE_39) != 1) {
                CB->combat(38, 0);
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0x84D4 */
    if (ST(V_EVENT_0) == 1) {
        CB->bloque(BLOQUE_ALL);
        CB->vire_col(-8, 0);
        ST(V_EVENT_0) = 0;
        CB->vire_col(2, 1);
        CB->combat(39, 0);
        CB->bloque(BLOQUE_NONE);
        ST(V_ENGAGED_PERSONA) = 39;
    }
    /* 0x853E */
    if (ST(L37_P39_AGGRO) == 0 && (CB->distance(1, 6) > 390 || ST(L37_P39_HIT_STATE) == 1) && ST(ST_VIE(39)) != VIE_DEAD && ST(L37_FIGHT_STATE) != 1) {
        CB->bloque(BLOQUE_ALL);
        /* 0x85AC */
        if (ST(L37_P39_HIT_STATE) == 0) {
            /* 0x85BE */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L37D01C", P_ARKHAN2, "SG00DIA08", NULL);
            }
            /* 0x85E6 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L37D02C", P_ARKHAN3, "SG00DIA08", NULL);
            }
        }
        CB->vire_col(2, 0);
        CB->animation("SP39ATT01");
        CB->animation("P39_65535");
        CB->animation("P39GAR01_10");
        CB->bloque(BLOQUE_NONE);
        CB->evenement(0, 500);
        ST(L37_FIGHT_STATE) = 1;
        ST(L37_P39_AGGRO) = 2;
        return;
    }
    /* 0x8674 */
    if (ST(ST_VIE(38)) == 1 && (ST(V_ENGAGED_PERSONA) == 0 || ST(V_ENGAGED_PERSONA) == 39) && CB->distance(5, 6) < 600 && ST(L37_TRUCE_39) == 0 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
        /* 0x86EB */
        if (ST(V_ENGAGED_PERSONA) == 39) {
            ST(V_ENGAGED_PERSONA) = 3938;
        } else {
            ST(V_ENGAGED_PERSONA) = 38;
        }
        CB->combat(38, 0);
    }
    /* 0x8727 */
    if (ST(ST_VIE(38)) == VIE_DEAD && ST(ST_VIE(39)) != VIE_DEAD && ST(V_ENGAGED_PERSONA) == 3938) {
        ST(V_ENGAGED_PERSONA) = 39;
    }
    /* 0x8769 */
    if (ST(ST_VIE(38)) != VIE_DEAD && ST(ST_VIE(39)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 3938) {
        ST(V_ENGAGED_PERSONA) = 38;
    }
    /* 0x87AB */
    if (ST(ST_VIE(38)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 38) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x87D8 */
    if (ST(ST_VIE(39)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == 39) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x8805 */
    if (ST(ST_VIE(38)) == VIE_DEAD && ST(ST_VIE(39)) == VIE_DEAD && ST(L37_FIGHT_STATE) == 1) {
        ST(L37_FIGHT_STATE) = 2;
    }
    snprintf(g_txt, sizeof(g_txt), "comb:%d", ST(V_ENGAGED_PERSONA));
    CB->print_msg(g_txt, 0, 240);
    /* 0x8877 */
    if (ST(L37_EXIT_JOIN) == 2 && !CB->on_joindre(ST(ST_HERO))) {
        ST(L37_EXIT_STAGE) = 0;
        ST(L37_EXIT_JOIN) = 0;
        CB->stop_joindre(-ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        /* 0x88DA */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(38)) != VIE_DEAD) {
            ST(L37_EXIT_STAGE) = 2;
            CB->combat(38, 0);
        }
        /* 0x8917 */
        if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(39)) != VIE_DEAD) {
            ST(L37_EXIT_STAGE) = 2;
            CB->combat(39, 0);
        }
    }
    /* 0x8957 */
    if (ST(L37_FAR_DIALOGUE_SEEN) == 0 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) > 15940) {
        ST(L37_FAR_DIALOGUE_SEEN) = 1;
        /* 0x899B */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L37D05", P_ARKHAN2);
        }
        /* 0x89BD */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L37D06", P_ARKHAN3);
        }
    }
    /* 0x89DF */
    if (ST(L37_CAM1_HINT_SEEN) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->distance(6, 8) > 300 && CB->distance(6, 5) > 300 && ST(ST_LOCKED_CAMERA) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(V_CRI_USED) == 0 && (CB->direction_spec(0x7530) || CB->direction_spec(0x9858))) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L37_CAM1_HINT_SEEN) = 1;
        /* 0x8AB5 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L37D07", P_ARKHAN2, "SG00DIA05", NULL);
        }
        /* 0x8ADD */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L37D08", P_ARKHAN3, "SG00DIA05", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8B1E */
    if (ST(L37_CAM3_HINT_SEEN) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_LOCKED_CAMERA) == 3 && ST(V_ENGAGED_PERSONA) == 0 && ST(V_CRI_USED) == 0 && CB->direction_spec(0x88B8)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L37_CAM3_HINT_SEEN) = 1;
        /* 0x8BAB */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L37D09", P_ARKHAN2, "SG00DIA05", NULL);
        }
        /* 0x8BD3 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L37D10", P_ARKHAN3, "SG00DIA05", NULL);
        }
        CB->animation("Sar_50");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000B3F9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001B96C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu37_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000B622);
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
        l37_trig00();   /* 0x1000B629: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000B50E — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000B67C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000B060).
 * Jump table @ 0x1000B6F9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu37_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu37: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l37_trigger(arg);             break;  /* dword_1001B800[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
