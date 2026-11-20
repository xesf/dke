/*
 * lieu11.c — Native hand-port of LIEU11.DLL (level-11 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu11_trig.c (reached through
 * l11_trigger, see lieu11_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. LIEU11 was built with a different layout
 * than LIEU01..LIEU09: the entry is at 0x1000AF60 (thunk 0x1000104B), the
 * state pointer global is dword_1001AAFC and the trigger table is
 * dword_1001AA10[42].
 *
 * event4 (sub_1000124A), event5 (sub_100022C4), event6 (sub_10005BFC),
 * lieu_action_register (sub_1000B62C), lieu_pre_scene (sub_1000B7DC) and
 * sub_10001210 (float reset) are the shared lieu_common.c. The entry and
 * INIT are the usual ones; tick_body (sub_100079BE) is level-11 specific.
 */

#include <stdio.h>

#include "lieu11_int.h"

/* Debug text buffers of the DLL (sprintf target in the original). */
c8 g_txt[256];    /* dword_1001A810 */
c8 g_txt2[256];   /* dword_1001A610 */

/* -------------------------------------------------------------------------
 * Per-frame body — sub_100079BE (thunk 0x100010BE), level-11 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x79D0 */
    if (ST(ST_HERO) == P_ARKHAN3 && ST(L11_GUARD_WARN_FAR_DONE) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 500 && ST(L11_GUARD_HOSTILE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L11_GUARD_WARN_FAR_DONE) = 1;
        CB->animation("SP32GAR05");
        CB->dialogue("L11D01", P_GUARD);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7A9B */
    if (ST(ST_HERO) == P_ARKHAN3 && ST(L11_GUARD_ATTACK_DONE) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 200 && ST(L11_GUARD_HOSTILE) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L11_GUARD_ATTACK_DONE) = 1;
        CB->vire_col(7, 0);
        CB->dialogue("L11D02", P_GUARD);
        ST(V_ENGAGED_PERSONA) = P_GUARD;
        CB->combat(P_GUARD, 0);
        ST(L11_GUARD_HOSTILE) = 1;
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7B81 */
    if (ST(ST_HERO) == P_ARKHAN2 && ST(L11_GUARD_WARN2_FAR_DONE) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 400 && ST(L11_GUARD_HOSTILE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L11_GUARD_WARN2_FAR_DONE) = 1;
        CB->vire_col(7, 0);
        CB->animation("SP32GAR05");
        CB->dialogue("L11D03", P_GUARD);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7C59 */
    if (ST(ST_HERO) == P_ARKHAN2 && ST(L11_GUARD_WARN2_NEAR_DONE) == 0 && ST(ST_VIE(P_GUARD)) != VIE_DEAD && CB->distance(6, 12) < 250 && ST(L11_GUARD_HOSTILE) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->select_pile(1, 1);
        CB->bloque(BLOQUE_ALL);
        ST(L11_GUARD_WARN2_NEAR_DONE) = 1;
        CB->vire_col(7, 0);
        CB->animation("SP32ARR04");
        CB->dialogue("L11D04", P_GUARD);
        CB->bloque(BLOQUE_NONE);
        CB->select_pile(0, 1);
        return;
    }
    /* 0x7D31 */
    if (ST(ST_VIE(P_GUARD)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) == P_GUARD) {
        ST(V_ENGAGED_PERSONA) = 0;
    }
    /* 0x7D66 */
    if (CB->divers(DV_WAS_HIT, P_GUARD, 0) && ST(ST_VIE(P_GUARD)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_GUARD, 1);
        CB->combat(P_GUARD, -1);
        /* 0x7DB9 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = P_GUARD;
        }
        ST(L11_GUARD_HOSTILE) = 1;
        /* 0x7DFA */
        if ((CB->divers(DV_HP, P_GUARD, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_GUARD, (CB->divers(DV_HP, P_GUARD, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_GUARD, 1);
        }
        /* 0x7E5B */
        if (CB->divers(DV_HP, P_GUARD, -1) <= 1) {
            ST(ST_VIE(P_GUARD)) = VIE_DEAD;
            CB->select_pile(2, 2);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARD, 0);
            CB->select_pile(1, 1);
            CB->animation("SP32IMP55");
            CB->animation("P32CAD");
            CB->pos_perso(P_GUARD, 0, 0, 0, 2, "P32CAD", 0xC, 0);
        } else {
            CB->divers_empile(DV_BLOOD_AT_BONE, P_GUARD, 0);
            CB->animation("SP32IMP15");
            CB->animation("P32IMP50");
            CB->animation("P32GAR05");
            CB->combat(P_GUARD, 0);
        }
        CB->select_pile(0, 1);
    }
    snprintf(g_txt, sizeof(g_txt), "Garde:%d", ST(ST_VIE(P_GUARD)));
    CB->print_msg(g_txt, 0, 0xF0);
    snprintf(g_txt, sizeof(g_txt), "F37:%d", ST(L11_GUARD_BRIBE_STAGE));
    CB->print_msg(g_txt, 0, 0x104);
    /* 0x7F92 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        /* 0x7FB3 */
        if (ST(L11_CRI_STATE) == 32) {
            ST(L11_CRI_STATE) = 99;
            ST(V_ENGAGED_PERSONA) = P_GUARD;
            CB->combat(P_GUARD, 0);
            ST(L11_GUARD_HOSTILE) = 1;
            lieu_action_register("L11D05", P_GUARD, "SP32ARR04", "SP32GAR05");
            CB->animation("Sar_50");
            CB->modify_var(&ST(V_CRI_USED), 0);
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
    /* 0x8041 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        CB->play_anim(0, 0);
        CB->animation("Sar_20");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->evenement(2, 1260);
    }
    /* 0x8098 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->divers_empile(DV_SCREEN, 1, 0);
        /* 0x80CB */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->cinematique("SC28_500");
        }
        /* 0x80E8 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->cinematique("SC29_500");
        }
        CB->bloque(BLOQUE_NONE);
        ST(ST_LEVEL) = 11;
        CB->change_piece(12, -1);
    }
    /* 0x8129 */
    if (ST(L11_GUARD_HOSTILE) == 1 && ST(ST_LEVEL) == 10) {
        CB->pos_perso(P_GUARD, 6087, -1362, 0x449F, 1, "P32GAR05", 0xC, 0);
    }
    /* 0x816F */
    if (ST(ST_HERO) == 4 && ST(L11_DEATH_EVENT_SENT) == 0) {
        ST(L11_DEATH_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x81AC */
    if (ST(V_END_GAME_REQ) == 1) {
        ST(V_END_GAME_REQ) = 0;
        /* 0x81D2 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x81FB */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x8227 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x8253 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8277 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x8294 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
        return;
    }
    /* 0x82C5 */
    if (ST(V_GUARD_FIGHT_DONE) == 2 && ST(ST_LOCKED_CAMERA) == 3 && ST(L11_CAMERA_PAUSED) == 0) {
        CB->pause_anim(0, 1, 0);
        ST(L11_CAMERA_PAUSED) = 1;
    }
    /* 0x8319 */
    if (ST(ST_LOCKED_CAMERA) != 3 && ST(L11_CAMERA_PAUSED) == 1) {
        ST(L11_CAMERA_PAUSED) = 0;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000B2F9).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001AB7C = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu11_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000B522);
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
        l11_trig00();   /* 0x1000B529: call sub_10001078 -> sub_10007463 */
        return 0;
    }
    /* 0x1000B3FC — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000B57C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x1000104B -> 0x1000AF60).
 * Jump table @ 0x1000B5F9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu11_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu11: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l11_trigger(arg);             break;  /* dword_1001AA10[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
