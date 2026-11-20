/*
 * lieu34.c - Native hand-port of LIEU34.DLL (level-34 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu34_trig.c (reached through
 * l34_trigger, see lieu34_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU30; the
 * entry is at 0x1000BDA0, the state pointer global is 0x1001BA2C and the
 * trigger table is dword_1001B940[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000C46C), lieu_pre_scene (sub_1000C61C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT are the same as LIEU30. tick_body (sub_10007DC4) is level-34
 * specific. The only DLL .data global the level code touches is the debug
 * sprintf buffer (0x1001B740), which is reset at INIT.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lieu34_int.h"

static c8 g_txt[256];   /* debug sprintf buffer, 0x1001B740 */

/* -------------------------------------------------------------------------
 * Per-frame body - sub_10007DC4 (thunk 0x100010B9), level-34 specific.
 * `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    snprintf(g_txt, sizeof(g_txt), "F58:%d", ST(V_CHASE_ROUTE));
    CB->print_msg(g_txt, 0, 240);
    snprintf(g_txt, sizeof(g_txt), "combat_on:%d", ST(V_ENGAGED_PERSONA));
    CB->print_msg(g_txt, 0, 248);
    snprintf(g_txt, sizeof(g_txt), "G93:%d", ST(L34_SORDOS_SCENE_KIND));
    CB->print_msg(g_txt, 0, 256);
    /* 0x7E63 */
    if (ST(ST_HERO) != 4) {
        ST(L34_LAST_LIVING_HERO) = ST(ST_HERO);
    }
    /* 0x7E8C */
    if (ST(ST_HERO) == 4 && ST(L34_DEATH_HANDLED) == 0) {
        ST(L34_DEATH_HANDLED) = 1;
        /* 0x7EB9 */
        if (ST(ST_VIE(P_THANANDAR24)) < 2) {
            CB->select_pile(1, 1);
            CB->animation("P24ARR01");
            CB->select_pile(0, 1);
        }
        CB->evenement(1, 500);
    }
    /* 0x7F03 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        /* 0x7F24 */
        if (ST(L34_SORDOS_SCENE_KIND) == 1) {
            /* 0x7F36: the Gork / Konkalite11 compare chain (0x7F65..0x7FC6)
             * reaches the body unless both have VIE >= 2 */
            if (ST(V_CHASE_STATE) == 0 && ST(V_SORDOS_DUEL) < 2 && ST(L34_STORY_PART) == 10 &&
                !(ST(ST_VIE(P_GORK)) >= 2 && ST(ST_VIE(P_KONKALITE11)) >= 2)) {
                ST(V_CHASE_STATE) = 1;
                ST(V_CHASE_ROUTE) = 1;
                CB->bloque(BLOQUE_ALL);
                lieu_action_register("L34D04A", P_SORDOS, "SP23DIA01", "SP23GAR03");
                CB->dialogue("L34D04B", P_SORDOS);
                CB->dialogue("L34D04C", P_SORDOS);
                CB->dialogue("L34D04D", P_SORDOS);
                ST(ST_LEVEL) = 34;
                CB->change_piece(32, 0);
                CB->bloque(BLOQUE_NONE);
            } else {
                /* 0x8064 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
                    CB->divers(DV_HP, P_ARKHAN0, 0);
                }
                /* 0x808A */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
                    CB->divers(DV_HP, P_ARKHAN1, 0);
                }
                /* 0x80B6 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
                    CB->divers(DV_HP, P_ARKHAN2, 0);
                }
                /* 0x80E2 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
                    CB->divers(DV_HP, P_ARKHAN3, 0);
                }
                /* 0x810E */
                if (ST(V_GUARD_FIGHT_DONE) == 0) {
                    CB->cinematique("SC22_500");
                }
                /* 0x8130 */
                if (ST(V_GUARD_FIGHT_DONE) != 0) {
                    CB->cinematique("SC23_500");
                }
                CB->divers_empile(DV_END_GAME, 0, 0);
            }
        }
        /* 0x8157 */
        if (ST(L34_SORDOS_SCENE_KIND) == 2) {
            /* 0x816E: same Gork / Konkalite11 chain (0x8192..0x81F3) */
            if (ST(V_CHASE_STATE) == 0 && ST(L34_STORY_PART) == 2 &&
                !(ST(ST_VIE(P_GORK)) >= 2 && ST(ST_VIE(P_KONKALITE11)) >= 2)) {
                ST(V_CHASE_STATE) = 1;
                ST(V_CHASE_ROUTE) = 1;
                CB->bloque(BLOQUE_ALL);
                lieu_action_register("L34D04A", P_SORDOS, "SP23DIA01", "SP23GAR03");
                CB->dialogue("L34D04B", P_SORDOS);
                CB->dialogue("L34D04C", P_SORDOS);
                CB->dialogue("L34D04D", P_SORDOS);
                ST(ST_LEVEL) = 34;
                CB->change_piece(32, 0);
                CB->bloque(BLOQUE_NONE);
            } else {
                /* 0x8291 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
                    CB->divers(DV_HP, P_ARKHAN0, 0);
                }
                /* 0x82B7 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
                    CB->divers(DV_HP, P_ARKHAN1, 0);
                }
                /* 0x82E3 */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
                    CB->divers(DV_HP, P_ARKHAN2, 0);
                }
                /* 0x830F */
                if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
                    CB->divers(DV_HP, P_ARKHAN3, 0);
                }
                /* 0x833B */
                if (ST(V_GUARD_FIGHT_DONE) == 0) {
                    CB->cinematique("SC22_500");
                }
                /* 0x835D */
                if (ST(V_GUARD_FIGHT_DONE) != 0) {
                    CB->cinematique("SC23_500");
                }
                CB->divers_empile(DV_END_GAME, 0, 0);
            }
        }
    }
    /* 0x8384 */
    if (ST(V_CHASE_ROUTE) == 0 && ST(L34_SORDOS_ENGAGED) == 0 && ST(L34_STORY_PART) == 2 && CB->divers(DV_PERSONA_PRESENT, P_SORDOS, 0)) {
        /* 0x83D6 */
        if (ST(L34_SORDOS_ATTACK_STEP) == 0 && (CB->distance(1, 3) <= 600 || (ST(V_PLAYER_STRIKE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2)))) {
            ST(L34_SORDOS_ATTACK_STEP) = 1;
            CB->select_pile(2, 1);
            CB->animation("SP23ATT02");
            CB->evenement(0, 400);
            CB->select_pile(0, 1);
        }
        /* 0x8465 */
        if (ST(V_EVENT_0) == 1) {
            ST(V_EVENT_0) = 0;
            /* 0x848B */
            if (ST(V_ENGAGED_PERSONA) == 0) {
                /* 0x849A */
                if (ST(L34_SORDOS_ATTACK_STEP) == 1) {
                    CB->select_pile(2, 1);
                    ST(L34_SORDOS_ATTACK_STEP) = 3;
                    CB->bloque(BLOQUE_ALL);
                    CB->link_objet(23, "OB50", 28, 0);
                    CB->etat_objet("OB50", 1340, -2519, 5770, 0, 0xB600, 0xDE00, 0, 6);
                    lieu_action_register("L34D38", P_SORDOS, "SP23ATT03", "SP23GAR03");
                    CB->pos_perso(P_SORDOS, 1525, 5810, CB->divers(DV_DESC_POS, P_SORDOS, 2), 1, "P23GAR03", 3, 0);
                    CB->joindre_perm(P_SORDOS, ST(ST_HERO));
                    CB->evenement(0, 5000);
                    CB->vire_col(6, 1);
                    CB->bloque(BLOQUE_NONE);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x85A2 */
                if (ST(L34_SORDOS_ATTACK_STEP) == 3) {
                    CB->select_pile(2, 1);
                    ST(L34_SORDOS_ATTACK_STEP) = 4;
                    CB->stop_joindre(P_SORDOS);
                    lieu_action_register("L34D39", P_SORDOS, "SP23GAR03", "SP23GAR03");
                    CB->joindre_perm(-P_SORDOS, ST(ST_HERO));
                    CB->evenement(0, 2500);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x8626 */
                if (ST(L34_SORDOS_ATTACK_STEP) == 4) {
                    CB->select_pile(2, 1);
                    CB->dialogue("L34D40A", P_SORDOS);
                    CB->dialogue("L34D40B", P_SORDOS);
                    CB->stop_joindre(-P_SORDOS);
                    ST(V_ENGAGED_PERSONA) = P_SORDOS;
                    CB->combat(P_SORDOS, 4);
                    CB->select_pile(0, 1);
                    return;
                }
            }
        }
    }
    /* 0x869E */
    if (ST(V_CHASE_ROUTE) == 0 && ST(V_ENGAGED_PERSONA) != 0) {
        /* 0x86C0 */
        if (ST(L34_SORDOS_SCENE_KIND) == 1) {
            /* 0x86D2 */
            if (ST(V_ENGAGED_PERSONA) == P_SORDOS) {
                /* 0x86E1 */
                if (ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(ST_VIE(P_THANANDAR24)) != VIE_DEAD) {
                    ST(V_SORDOS_DUEL) = 2;
                    ST(V_ENGAGED_PERSONA) = P_THANANDAR24;
                    CB->stop_joindre(-P_THANANDAR24);
                    CB->animation("SP24GAR01");
                    CB->combat(P_THANANDAR24, 0);
                }
                /* 0x8742 */
                if (ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(ST_VIE(P_THANANDAR24)) == VIE_DEAD) {
                    ST(V_SORDOS_DUEL) = 2;
                    ST(V_CHASE_ROUTE) = 2;
                    ST(V_ENGAGED_PERSONA) = 0;
                    CB->change_camera(CAM_ANY, CAM_AUTO);
                }
            }
            /* 0x87A0 */
            if (ST(V_ENGAGED_PERSONA) == P_THANANDAR24 && ST(ST_VIE(P_THANANDAR24)) == VIE_DEAD) {
                ST(L34_STORY_PART) = 10;
                ST(V_CHASE_ROUTE) = 2;
                ST(V_ENGAGED_PERSONA) = 0;
                CB->change_camera(CAM_ANY, CAM_AUTO);
            }
            /* 0x87FB */
            if (ST(V_ENGAGED_PERSONA) == 0x914) {
                /* 0x880D */
                if (ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(ST_VIE(P_THANANDAR24)) == VIE_DEAD) {
                    ST(V_SORDOS_DUEL) = 2;
                    ST(L34_STORY_PART) = 10;
                    ST(V_CHASE_ROUTE) = 2;
                    ST(V_ENGAGED_PERSONA) = 0;
                    CB->change_camera(CAM_ANY, CAM_AUTO);
                }
                /* 0x887A */
                if (ST(ST_VIE(P_SORDOS)) != VIE_DEAD && ST(ST_VIE(P_THANANDAR24)) == VIE_DEAD) {
                    ST(L34_SORDOS_ENGAGED) = 10;
                    ST(V_ENGAGED_PERSONA) = P_SORDOS;
                    ST(L34_STORY_PART) = 10;
                }
                /* 0x88C5 */
                if (ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(ST_VIE(P_THANANDAR24)) != VIE_DEAD) {
                    ST(L34_SORDOS_ENGAGED) = 10;
                    ST(V_SORDOS_DUEL) = 2;
                    ST(V_ENGAGED_PERSONA) = P_THANANDAR24;
                }
            }
        }
        /* 0x8910 */
        if (ST(L34_SORDOS_SCENE_KIND) == 2 && ST(ST_VIE(P_SORDOS)) == VIE_DEAD) {
            ST(V_SORDOS_DUEL) = 2;
            ST(V_CHASE_ROUTE) = 3;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->animation("Sar_50");
            /* 0x896E */
            if (ST(L34_SORDOS_ATTACK_STEP) != 0) {
                CB->ajoute_zone(6, 1405, 5768, 120, 120, 0);
            }
            CB->change_camera(CAM_ANY, CAM_AUTO);
        }
    }
    /* 0x89A3 */
    if (ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(ST_VIE(P_THANANDAR24)) == VIE_DEAD && ST(L34_SORDOS_SCENE_KIND) == 1 && CB->divers(DV_PERSONA_PRESENT, P_THANANDAR24, 0)) {
        ST(V_SORDOS_DUEL) = 2;
        ST(L34_STORY_PART) = 10;
        ST(V_CHASE_ROUTE) = 2;
        ST(V_ENGAGED_PERSONA) = 0;
        CB->animation("Sar_50");
        CB->change_camera(CAM_ANY, CAM_AUTO);
    }
    /* 0x8A56 */
    if ((ST(V_CHASE_ROUTE) == 1 || ST(V_CHASE_ROUTE) == 11) && ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0) {
        ST(V_SORDOS_DUEL) = 2;
        ST(V_CHASE_ROUTE) = 2;
        ST(V_ENGAGED_PERSONA) = 0;
        CB->animation("Sar_50");
        /* 0x8AD5 */
        if (ST(L34_SORDOS_ATTACK_STEP) != 0) {
            CB->ajoute_zone(6, 1405, 5768, 120, 120, 0);
        }
        CB->change_camera(CAM_ANY, CAM_AUTO);
    }
    /* 0x8B0A */
    if (ST(V_CHASE_ROUTE) == 2 && ST(ST_VIE(P_SORDOS)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(L34_SORDOS_SCENE_KIND) == 2) {
        ST(V_SORDOS_DUEL) = 2;
        ST(V_CHASE_ROUTE) = 3;
        ST(V_ENGAGED_PERSONA) = 0;
        CB->animation("Sar_50");
        /* 0x8B89 */
        if (ST(L34_SORDOS_ATTACK_STEP) != 0) {
            CB->ajoute_zone(6, 1405, 5768, 120, 120, 0);
        }
        CB->change_camera(CAM_ANY, CAM_AUTO);
    }
    /* 0x8BBE */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->select_pile(1, 1);
        CB->anim_objet("IM23", 1, 50, 0x1DB, 0x400);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->select_pile(0, 1);
    }
    /* 0x8C29 */
    if (CB->divers(DV_WAS_HIT, P_SORDOS, 0) && ST(ST_VIE(P_SORDOS)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_SORDOS, 1);
        CB->combat(P_SORDOS, -1);
        /* 0x8C95 */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            /* 0x8CA6 */
            if (ST(L34_STORY_PART) != 2) {
                ST(V_ENGAGED_PERSONA) = 0x914;
            } else {
                ST(V_ENGAGED_PERSONA) = P_SORDOS;
            }
        }
        /* 0x8CD4 */
        if ((CB->divers(DV_HP, P_SORDOS, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_SORDOS, (CB->divers(DV_HP, P_SORDOS, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_SORDOS, 1);
        }
        /* 0x8D40 */
        if (CB->divers(DV_HP, P_SORDOS, -1) <= 1) {
            ST(ST_VIE(P_SORDOS)) = VIE_DEAD;
            CB->divers_empile(DV_BLOOD_AT_BONE, P_SORDOS, 0);
            CB->animation("SP23IMP53");
            CB->animation("P23CAD");
            CB->pos_perso(P_SORDOS, 0, 0, 0, 2, "P23CAD", 3, 0);
        } else {
            CB->divers_empile(DV_BLOOD_AT_BONE, P_SORDOS, 0);
            CB->animation("SP23IMP13");
            CB->animation("P23IMP48");
            CB->combat(P_SORDOS, 4);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8DF8 */
    if (CB->divers(DV_WAS_HIT, P_THANANDAR24, 0) && ST(ST_VIE(P_THANANDAR24)) != VIE_DEAD) {
        CB->select_pile(2, 2);
        CB->divers_empile(DV_WAS_HIT, P_THANANDAR24, 1);
        CB->combat(P_THANANDAR24, -1);
        /* 0x8E5B */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_ENGAGED_PERSONA) = 0x914;
        }
        /* 0x8E71 */
        if ((CB->divers(DV_HP, P_THANANDAR24, -1) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_THANANDAR24, (CB->divers(DV_HP, P_THANANDAR24, -1) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_THANANDAR24, 1);
        }
        /* 0x8EDD */
        if (CB->divers(DV_HP, P_THANANDAR24, -1) <= 1) {
            ST(ST_VIE(P_THANANDAR24)) = VIE_DEAD;
            ST(L34_STORY_PART) = 10;
            CB->stop_joindre(-P_THANANDAR24);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_THANANDAR24, 0);
            CB->animation("SP24IMP51");
            CB->animation("P24CAD");
            CB->pos_perso(P_THANANDAR24, 0, 0, 0, 2, "P24CAD", 2, 0);
        } else {
            CB->stop_joindre(-P_THANANDAR24);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_THANANDAR24, 0);
            CB->animation("SP24IMP11");
            CB->animation("P24IMP46");
            CB->combat(P_THANANDAR24, 4);
        }
        CB->select_pile(0, 1);
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) - arming tail of FonctionEditor case 1 (0x1000C139).
 * Callback ingestion, the trigger-table fill and the unused
 * dword_1001BAAC = state+0xC8 are skipped: g_lieu_cb is provided by the
 * bridge, the trigger table is static in lieu34_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000C364);
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
        l34_trig00();   /* 0x1000C369: call sub_10001073 -> trig00_100073F0 */
        return 0;
    }
    /* 0x1000C24E - the hero is dead: play the death ending */
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
 * Entry point - @FonctionEditor@16 (thunk 0x10001046 -> 0x1000BDA0).
 * Jump table @ 0x1000C439, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu34_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu34: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        g_txt[0] = '\0';
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l34_trigger(arg);             break;  /* dword_1001B940[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
