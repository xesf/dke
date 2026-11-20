/*
 * lieu38.c - Native hand-port of LIEU38.DLL (level-38 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu38_trig.c (reached through
 * l38_trigger, see lieu38_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x1000BF90, the state pointer global is dword_1001BE4C and the
 * trigger table is dword_1001BD60[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register, lieu_pre_scene and sub_10001200 (float reset) are
 * the shared lieu_common.c; the entry and INIT follow LIEU35. tick_body
 * (sub_10007E0C) is level-38 specific. The level code touches no DLL .data
 * global, so INIT resets nothing.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu38_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10007E0C (thunk 0x100010B9), level-38 specific.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x7E1E */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x7E61 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        CB->bloque(BLOQUE_ALL);
        /* 0x7E92 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x7EBB */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x7EE7 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x7F13 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x7F37 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x7F54 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x7F80 - the skeleton guard (persona 91) notices the hero */
    if (ST(L38_GUARD91_STATE) == 0 && CB->distance(1, 20) < 220) {
        ST(L38_GUARD91_STATE) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP91GAR26");
        CB->animation("P91GAR01");
        ST(V_ENGAGED_PERSONA) = 91;
        CB->combat(91, 1);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x8001 */
    if (ST(L38_GUARD91_STATE) == 1 && CB->distance(1, 20) < 220 && ST(L38_GUARD91_REARM) == 99) {
        ST(L38_GUARD91_REARM) = 0;
        CB->combat(91, 1);
    }
    /* 0x805F */
    if (ST(L38_GUARD91_STATE) == 1) {
        /* 0x8071 */
        if (ST(ST_VIE(91)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
            ST(V_ENGAGED_PERSONA) = 0;
            ST(L38_GUARD91_STATE) = 2;
        }
        /* 0x80B5 */
        if (CB->divers(DV_WAS_HIT, 91, 0) && ST(ST_VIE(91)) != VIE_DEAD) {
            CB->select_pile(1, 2);
            CB->divers_empile(DV_WAS_HIT, 91, 1);
            CB->combat(91, -1);
            /* 0x811F */
            if (CB->divers(DV_HP, 91, -1) - ST(V_HIT_DAMAGE) > 0) {
                i32 hp = CB->divers(DV_HP, 91, -1) - ST(V_HIT_DAMAGE);
                CB->divers(DV_HP, 91, hp);
            } else {
                CB->divers(DV_HP, 91, 1);
            }
            /* 0x816B */
            if (CB->divers(DV_HP, 91, -1) <= 1) {
                ST(ST_VIE(91)) = VIE_DEAD;
                ST(V_ENGAGED_PERSONA) = 0;
                ST(L38_GUARD91_STATE) = 2;
                CB->divers_empile(DV_BLOOD_AT_BONE, 91, 0);
                CB->animation("SP91IMP51");
                CB->animation("P91ARR01");
                CB->pos_perso(91, -1, -1, -1, 4, "", -1, 0);
                CB->divers_empile(DV_SCREEN, 2, 0);
            } else {
                CB->divers_empile(DV_BLOOD_AT_BONE, 91, 0);
                CB->animation("SP91IMP01");
                CB->animation("P91GAR01");
                CB->combat(91, 1);
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0x8258 - the hero is close to the descriptor of the key spot */
    if (ST(L38_KEYSPOT_NEAR) == 0 && ST(ST_HERO) != 4 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) < 90 && (ST(ST_LOCKED_CAMERA) == 5 || ST(ST_LOCKED_CAMERA) == 6)) {
        ST(L38_KEYSPOT_NEAR) = 1;
    }
    /* 0x82D0 */
    if (ST(L38_KEYSPOT_NEAR) == 1) {
        /* 0x82E2 */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
            /* 0x8319: pass unless (in zone 24 and facing 0x24B8); zone 24 is
             * queried a second time on the not-passed paths */
            i32 skip = 0;
            if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 24)) {
                if (CB->direction_spec(0x24B8)) {
                    /* 0x834C */
                    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 24)) skip = 1;
                }
            } else {
                /* 0x834C */
                if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 24)) skip = 1;
            }
            /* 0x836C: same for zone 25 and direction_zone(25) */
            if (!skip) {
                if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 25)) {
                    if (CB->direction_zone(25)) {
                        /* 0x839F */
                        if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 25)) skip = 1;
                    }
                } else {
                    /* 0x839F */
                    if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 25)) skip = 1;
                }
            }
            if (!skip) {
                /* 0x83BF */
                ST(ST_SCRIPT_BUSY) = 1;
                CB->bloque(BLOQUE_ALL);
                /* 0x83D6 */
                if (ST(L38_KEYSPOT_USES) < 2) {
                    /* 0x83E8 */
                    if (CB->divers(DV_CAN_FIGHT, 9, 2)) {
                        CB->animation("SP02ACT52");
                    } else {
                        CB->animation("SP02ACT51");
                    }
                    CB->evenement(0, 100);
                    /* 0x842A */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L38D03A", P_ARKHAN2, "G02DIA01", NULL);
                        lieu_action_register("L38D03B", P_ARKHAN2, "SG00CON01", NULL);
                    }
                    /* 0x8468 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L38D04A", P_ARKHAN3, "G03DIA01", NULL);
                        lieu_action_register("L38D04B", P_ARKHAN3, "SG00CON01", NULL);
                    }
                    ST(L38_DRAIN_LAST_TIME) = ST(ST_GAME_TIME);
                    ST(L38_KEYSPOT_USES)++;
                    CB->animation("ar");
                    return;
                }
                /* 0x84D5 */
                if (ST(L38_KEYSPOT_USES) == 2) {
                    /* 0x84EC */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L38D05", P_ARKHAN2, "SG00DIA02", NULL);
                    }
                    /* 0x850F */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L38D06", P_ARKHAN3, "SG00DIA02", NULL);
                    }
                    CB->animation("Sar_50");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0x8560 */
        if (ST(ST_HERO) != 4 && CB->divers(DV_DESC_POS, ST(ST_HERO), 3) > 100) {
            ST(L38_KEYSPOT_NEAR) = 0;
            ST(L38_KEYSPOT_USES) = 0;
        }
    }
    /* 0x85B1 - drain timer while event 0 is armed */
    if (ST(V_EVENT_0) == 1 && (ST(ST_GAME_TIME) - ST(L38_DRAIN_LAST_TIME)) / 25 > 1) {
        ST(L38_DRAIN_STEP_COUNT)++;
        ST(L38_DRAIN_LAST_TIME) = ST(ST_GAME_TIME);
        /* 0x8604 */
        if (CB->divers(DV_HP, ST(ST_HERO), -1) - ST(L38_DRAIN_HP_STEP) > 0) {
            i32 hp = CB->divers(DV_HP, ST(ST_HERO), -1) - ST(L38_DRAIN_HP_STEP);
            CB->divers(DV_HP, ST(ST_HERO), hp);
        } else {
            ST(L38_DRAIN_STEP_COUNT) = 31;
        }
        /* 0x8676 */
        if (CB->divers(DV_DARKNESS, -1, 0) - ST(L38_DRAIN_DARKNESS_STEP) > 0) {
            i32 dark = CB->divers(DV_DARKNESS, -1, 0) - ST(L38_DRAIN_DARKNESS_STEP);
            CB->divers(DV_DARKNESS, dark, 0);
        } else {
            ST(L38_DRAIN_STEP_COUNT) = 31;
        }
        /* 0x86D4 */
        if (ST(L38_DRAIN_STEP_COUNT) > 30) {
            ST(V_EVENT_0) = 0;
            ST(L38_DRAIN_STEP_COUNT) = 0;
            ST(ST_SCRIPT_BUSY) = 0;
            CB->bloque(BLOQUE_NONE);
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000C329).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001BECC = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu38_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000C552);
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
        l38_trig00();   /* 0x1000C559: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000C43E - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000BF90).
 * Jump table @ 0x1000C629, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu38_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu38: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l38_trigger(arg);             break;  /* dword_1001BD60[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
