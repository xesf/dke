/*
 * lieu44.c — Native hand-port of LIEU44.DLL (level-44 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu44_trig.c (reached through
 * l44_trigger, see lieu44_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. The entry is at 0x10008140, the state
 * pointer global is dword_100180CC and the trigger table is a static array.
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000880C), lieu_pre_scene and sub_10001200
 * (float reset) are the shared lieu_common.c. The tick body (sub_10007618)
 * is level-44 specific. The DLL's only writable global is the sprintf
 * buffer (0x10017DE0), which holds no state.
 */

#include <stdio.h>

#include "lieu44_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body — sub_10007618 (thunk 0x100010B9), level-44 specific.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    static c8 buf[128];   /* 0x10017DE0 */

    /* 0x7626 */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        ST(V_END_GAME_REQ) = 0;
        CB->evenement(EV_END_GAME, 1500);
    }
    /* 0x7673 */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->cinematique("SC23_500");
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x76BD */
    if (ST(L44_HERO_ACTING) == 0 && ST(L44_CAUGHT_STATE) == 0) {
        for (i32 i = 0; i < 6; i++) {
            i32 d = CB->divers(DV_STRUCT_DIST, i, 0);
            /* 0x7707 */
            if (d < 100) {
                snprintf(buf, sizeof buf, "%d %d %d", CB->divers(DV_STRUCT_DIST, i, 0), ST(L44_HERO_ACTING), ST(L44_CAUGHT_STATE));
                CB->print_msg(buf, 0, 210);
                ST(L44_CAUGHT_STATE) = 1;
                break;
            }
        }
    }
    /* 0x776E */
    snprintf(buf, sizeof buf, "test1 %d test2 %d ALLER %d", ST(L44_HERO_ACTING), ST(L44_CAUGHT_STATE), ST(ST_UPDOWN_KEY));
    CB->print_msg(buf, 0, 460);
    /* 0x77AD */
    if (ST(L44_CAUGHT_STATE) == 1) {
        ST(L44_CAUGHT_STATE) = 2;
        CB->bloque(BLOQUE_ALL);
        if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
            CB->animation("SP03ATT80");
        } else {
            CB->animation("SP03ATT81");
        }
        CB->animation("P03_65535");
        CB->divers_empile(DV_HP, ST(ST_HERO), 0);
        CB->modify_var(&ST(ST_HERO), 4);
    }
    /* 0x7842 */
    if (ST(L44_CAMERA_RELEASE_PENDING) == 1 && ST(ST_UPDOWN_KEY) == 1) {
        ST(L44_CAMERA_RELEASE_PENDING) = 0;
        CB->change_camera(1, CAM_AUTO);
    }
    /* 0x787C */
    if (CB->divers(DV_ACTION_HELD, 0, 0) == 1 && ST(L44_HERO_ACTING) == 1 && ST(L44_ACTION_ANIM_DONE) == 1) {
        CB->bloque(BLOQUE_ALL);
        ST(L44_HERO_ACTING) = 0;
        ST(L44_ACTION_ANIM_DONE) = 0;
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000815A).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001814C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu44_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x10008702);
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
        l44_trig00();   /* 0x10008709: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x100085EE — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000875C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x10008140).
 * Jump table @ 0x100087D9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu44_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu44: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l44_trigger(arg);             break;  /* trigger table[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
