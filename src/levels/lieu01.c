/*
 * lieu01.c — Native hand-port of LIEU01.DLL (level-1 scripting).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Handlers call the engine only through
 * g_lieu_cb (see lieu_cb.h). See docs/functions/lieu01_handlers.md for
 * the opcode dispatch, trigger table and state-offset map.
 *
 * This file: entry / opcode dispatch, INIT, tick / tick body and the
 * helpers both halves use. Events 4-6 are the shared lieu_common.c. The 42 trigger slots (25 handlers) are
 * in lieu01_trig.c.
 */

#include <stdio.h>

#include "lieu01_int.h"

/* Engine-provided callback vtable is defined in lieu_cb.c (g_lieu_cb). */


/* Shorthands mirroring the disassembly. */


/* Spawn "ar" actor at position from 3 layer-1 coordinate queries.
 * Queries evaluated axis 2,1,0 (original order) into temps. */
void l01_spawn_ar(void) {
    i32 a2 = CB->divers(DV_DESC_POS, ST(ST_HERO), 2);
    i32 a1 = CB->divers(DV_DESC_POS, ST(ST_HERO), 1);
    i32 a0 = CB->divers(DV_DESC_POS, ST(ST_HERO), 0);
    CB->pos_perso(ST(ST_HERO), a0, a1, a2, 1, "ar", 6, 0);
}


/*
 * Per-frame body — sub_100088FE (11 KB, the level-1 state machine).
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp loc_1000B55E` (epilogue).
 * Complete: 0x88FE .. 0xB562 (all 2183 instructions).
 */
static void tick_body(void) {
    /* 0x88FE — debug subtitle */
    {
        c8 buf[64];
        snprintf(buf, sizeof(buf), "testos printos:%d", ST(ST_GAME_TIME));
        CB->print_msg(buf, 0, 0xF0);
    }
    /* 0x8934 */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        ST(V_DEATH_EVENT_SENT) = 1;
        CB->evenement(EV_END_GAME, 3000);
    }
    /* 0x8977 — intro cinematic replay */
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
    /* 0x8A96 — level-0 arrival spawn */
    if (ST(V_EVENT_2) == 1 && ST(ST_HERO) == P_ARKHAN0) {
        ST(V_EVENT_2) = 0;
        CB->animation("SP12ARR01_10");
        CB->animation("SP00LEV01");
        CB->bruitage("L01B03", 638, -3062);
        CB->animation("P00MAR25");
        CB->animation("ar");
        CB->pos_perso(0, 628, -2950, 0xFBC0, 1, "ar", 6, 0);
        CB->divers_empile(DV_MUSIC, 1, 1);
        CB->change_camera(CAM_ANY, CAM_AUTO);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8B5B */
    if (ST(V_EVENT_34) == 1) {
        ST(V_EVENT_34) = 0;
        ST(V_ZED_BRIEFING) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->dialogue("L01D01A", P_ZED27);
        CB->dialogue("L01D01B", P_ZED27);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x8BC3 — section F: guarded by [1C0]==1 && [EC0]==0 */
    if (ST(ST_VIE(P_KALHI12)) == 1 && ST(V_LEFT_HOME) == 0) {
        /* 0x8BE7 */
        if (ST(V_EVENT_0) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(L01_KALHI12_ALERT) != 1) {
            ST(V_EVENT_0) = 2;
            ST(L01_KALHI12_ALERT)  = 1;
            ST(L01_SAVED_CAMERA)  = ST(ST_LOCKED_CAMERA);
            ST(L01_CAMERA_RESTORE)  = 1;
            ST(L01_SCENE_CAMERA)  = 1;
            ST(ST_SCRIPT_BUSY)  = 1;
            CB->bloque(BLOQUE_ALL);
            if (ST(V_KALHI12_TALK_NIGHT) == 0) CB->change_camera(6, CAM_LOCKED);
            CB->animation("SP12LEV01");
            CB->animation("P12ARR03_25");
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_KALHI12, 1);
            lieu_action_register("L01D03", 0x0C, "P12ARR03", "SP12ARR03_100");
            CB->modify_var(&ST(ST_SCRIPT_BUSY),  0);
            CB->modify_var(&ST(V_KALHI12_TALK_NIGHT), 1);
            CB->modify_var(&ST(L01_KALHI12_ANIM_STATE),  1);
            if (ST(L01_SAVED_CAMERA) != 0 && ST(L01_SAVED_CAMERA) != 6)
                CB->change_camera(ST(ST_LOCKED_CAMERA), CAM_AUTO);
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x8D67 */
        if (ST(L01_KALHI12_ANIM_STATE) == 1 && ST(ST_SCRIPT_BUSY) == 0) {
            ST(L01_KALHI12_ANIM_STATE) = 2;
            CB->select_pile(1, 1);
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_KALHI12, 0);
            CB->animation("P12WAA01");
            CB->animation("P12ARR03_10");
            CB->divers_empile(DV_LOOK_AT_PLAYER, P_KALHI12, 1);
            CB->modify_var(&ST(L01_KALHI12_ANIM_STATE), 0x29A);
            CB->select_pile(0, 1);
        }
        /* 0x8E05 */
        if (CB->distance(6, 4) > 0x190 && (i32)ST(V_EVENT_0) > 0 && ST(L01_KALHI12_RESUME) != 1) {
            /* 0x8E41 */
            if (ST(ST_LOCKED_CAMERA) == 6 && ST(L01_CAMERA_RESTORE) == 1) {
                ST(L01_CAMERA_RESTORE) = 2;
                CB->change_camera(ST(ST_LOCKED_CAMERA), CAM_AUTO);
            }
            /* 0x8E8A */
            if (ST(L01_KALHI12_ALERT) == 1 && ST(V_ENGAGED_PERSONA) == 0
                && CB->random(0xC8) == 1 && ST(L01_KALHI12_ANIM_STATE) == 0x29A) {
                CB->divers(DV_LOOK_AT_PLAYER, P_KALHI12, 0);
                ST(L01_KALHI12_ALERT) = 0;
                CB->select_pile(1, 1);
                CB->animation("SP12LEV03");
                CB->animation("P12ARR01_10");
                CB->select_pile(0, 1);
            }
            /* 0x8F22 */
            if (ST(V_EVENT_1) != 0)
                ST(V_EVENT_1) = 0;
        }
        /* 0x8F43 */
        if (CB->distance(6, 4) < 0x96 && (i32)ST(V_EVENT_0) > 0
            && ST(L01_KALHI12_ALERT) == 0 && ST(V_ENGAGED_PERSONA) == 0
            && CB->random(0x64) == 1 && ST(L01_KALHI12_ANIM_STATE) == 0x29A) {
            ST(L01_KALHI12_ALERT) = 1;
            CB->select_pile(1, 1);
            CB->animation("SP12LEV01");
            CB->animation("P12ARR03_10");
            CB->modify_var(&ST(V_EVENT_1), 1);
            CB->select_pile(0, 1);
        }
        /* 0x900D */
        if (ST(V_EVENT_1) == 1) {
            ST(V_EVENT_1) = 2;
            CB->divers(DV_LOOK_AT_PLAYER, P_KALHI12, 1);
        }
    }
    /* 0x9040 — section G: guarded by [EC0]==0 && [B04]==0 */
    if (ST(V_LEFT_HOME) == 0 && ST(V_GUARD_FIGHT_DONE) == 0) {
        /* 0x9064 */
        if (ST(L01_PHEDORIA_STEP) == 1) {
            ST(L01_PHEDORIA_STEP) = -1;
            CB->pos_perso(P_PHEDORIA, 400, -2330, 0x9470, 1,
                          "P26GAR03_10", -1, 0);
            CB->change_camera(4, CAM_LOCKED);
            CB->bruitage("PORTE3", 512, -2087);
            CB->dialogue("L01D45A", P_PHEDORIA);
            if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 7))
                CB->change_camera(0, CAM_LOCKED);
            /* 0x9106 */
            CB->joindre(ST(ST_HERO), 0, 0, 0, 0x1A);
            if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 4))
                CB->change_camera(0, CAM_LOCKED);
            /* 0x914C */
            CB->animation("Sar");
            CB->joindre(-ST(ST_HERO), 512, -2677, 0x859, -1);
            CB->dialogue("L01D99", 0);
            CB->change_camera(1, CAM_LOCKED);
            lieu_action_register("L01D45B", 0x1A, "SP26GAR03", "SG26FRA01");
            CB->evenement(0x27, 750);
            CB->animation("P26GAR03_10");
        }
        /* 0x91CE */
        if (ST(V_EVENT_39) == 1) {
            ST(V_EVENT_39) = 0;
            CB->select_pile(2, 2);
            CB->stop_joindre(-ST(ST_HERO));
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            if (ST(ST_HERO) == P_ARKHAN0) CB->animation("SG00ATT07");
            if (ST(ST_HERO) == P_ARKHAN1) CB->animation("SG01ATT07");
            CB->modify_var(&ST(L01_PHEDORIA_STEP), 2);
            CB->select_pile(0, 1);
        }
        /* 0x9287 */
        if (ST(L01_PHEDORIA_STEP) == 2) {
            ST(L01_PHEDORIA_STEP)  = -1;
            ST(V_EVENT_1) = 0;
            CB->divers(DV_LOOK_AT_PLAYER, P_KALHI12, 0);
            CB->animation("SP12ATT03");
            CB->change_camera(6, CAM_LOCKED);
            CB->dialogue("L01D44", P_KALHI12);
            CB->animation("SP12ATT03_2000");
            CB->divers_empile(DV_HP, ST(ST_HERO), 0);
            CB->modify_var(&ST(ST_HERO), 4);
        }
    }
    /* 0x9328 */
    if (ST(V_ENGAGED_PERSONA) == 0x57 && ST(V_GUARD_WAVE) == 0 && ST(ST_VIE(P_KABAK)) == VIE_DEAD && ST(V_LEFT_HOME) == 1)
        ST(V_ENGAGED_PERSONA) = 0;
    /* 0x9379 — door PORTE spawn */
    if (ST(ST_VIE(P_KABAK)) == VIE_DEAD && ST(V_GUARD_WAVE) != 0 && ST(L01_KABAK_DOOR_DONE) == 0 && ST(V_KABAK_DOOR_ZONE) == 0) {
        ST(L01_KABAK_GUARD)  = 0;
        ST(L01_KABAK_DOOR_DONE)  = 1;
        ST(V_KABAK_DOOR_ZONE) = 1;
        CB->select_pile(2, 1);
        CB->ajoute_zone(16, 320, -2543, 90, 90, 0);
        CB->select_pile(0, 1);
    }
    /* 0x9426 — imp W (0x57) appearance */
    if (CB->divers(DV_WAS_HIT, P_KABAK, 0) && ST(ST_VIE(P_KABAK)) != VIE_DEAD
        && CB->divers(DV_PERSONA_PRESENT, P_KABAK, 0)) {
        CB->select_pile(1, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_KABAK, 1);
        ST(ST_SCRIPT_BUSY)  = 0;
        ST(V_ENGAGED_PERSONA)  = 0;
        ST(ST_VIE(P_KABAK)) = VIE_DEAD;
        CB->combat(P_KABAK, -1);
        CB->stop_joindre(-P_KABAK);
        CB->animation("SP87IMP52");
        CB->animation("P87CAD");
        CB->pos_perso(P_KABAK, 0, 0, 0, 2, "P87CAD", 9, 0);
        if (ST(V_GUARD_WAVE) == 1 && ST(ST_VIE(P_GUARDIAN23)) == 1) {
            if (ST(L01_G23_UP) == 1) {
                ST(V_G23_FIGHT_ON) = 1;
                ST(L01_G23_FOLLOW_STEP)  = 0x0A;
                CB->select_pile(2, 2);
                CB->stop_joindre(-P_GUARDIAN23);
                CB->dialogue("L01D147", P_GUARDIAN23);
                CB->combat(P_GUARDIAN23, 0);
                ST(V_ENGAGED_PERSONA) = 0x58;
            }
            if (ST(L01_G23_UP) == 0) {
                ST(V_G23_FIGHT_ON) = 1;
                ST(L01_G23_UP)  = 1;
                CB->select_pile(2, 2);
                CB->vire_col(1, 0);
                CB->animation("SP88LEV01");
                CB->animation("P88_65535");
                CB->dialogue("L01D147", P_GUARDIAN23);
                CB->vire_col(1, 1);
                CB->combat(P_GUARDIAN23, 0);
                ST(V_ENGAGED_PERSONA) = 0x58;
            }
        }
        CB->select_pile(0, 1);
    }
    /* 0x9652 — imp X (0x58) appearance */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN23, 0) && ST(ST_VIE(P_GUARDIAN23)) != VIE_DEAD && ST(L01_G23_UP) == 0) {
        CB->select_pile(2, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN23, 1);
        ST(L01_GUARD_HIT)  = 1;
        ST(ST_SCRIPT_BUSY)  = 0;
        ST(V_ENGAGED_PERSONA)  = 0x57;
        ST(ST_VIE(P_GUARDIAN23)) = VIE_DEAD;
        CB->animation("SP88IMP99");
        CB->animation("P88CAD");
        CB->pos_perso(P_GUARDIAN23, 0, 0, 0, 2, "P88CAD", 0x11, 0);
        if (ST(ST_VIE(P_KABAK)) == 1) {
            ST(V_KABAK_FIGHT_ON) = 1;
            CB->select_pile(1, 2);
            CB->dialogue("L01D148", P_KABAK);
            CB->combat(P_KABAK, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x9784 — imp Y (0x59) appearance */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN24, 0) && ST(ST_VIE(P_GUARDIAN24)) != VIE_DEAD
        && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN24, 0)) {
        ST(L01_GUARD_HIT)  = 1;
        ST(ST_SCRIPT_BUSY)  = 0;
        ST(V_ENGAGED_PERSONA)  = 0;
        ST(ST_VIE(P_GUARDIAN24)) = VIE_DEAD;
        CB->select_pile(2, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN24, 1);
        CB->animation("SP89IMP51");
        CB->animation("P89CAD");
        CB->pos_perso(P_GUARDIAN24, 0, 0, 0, 2, "P89CAD", 0x12, 0);
        if (ST(ST_VIE(P_KABAK)) == 1) {
            ST(V_KABAK_FIGHT_ON) = 1;
            CB->select_pile(1, 2);
            CB->dialogue("L01D148", P_KABAK);
            CB->combat(P_KABAK, 0);
            ST(V_ENGAGED_PERSONA) = 0x57;
        }
        CB->select_pile(0, 1);
    }
    /* 0x98CA — imp X (0x58) variant */
    if (CB->divers(DV_WAS_HIT, P_GUARDIAN23, 0) && ST(ST_VIE(P_GUARDIAN23)) != VIE_DEAD && ST(L01_G23_UP) == 1) {
        ST(L01_GUARD_HIT)  = 1;
        ST(ST_SCRIPT_BUSY)  = 0;
        ST(ST_VIE(P_GUARDIAN23)) = VIE_DEAD;
        CB->select_pile(2, 2);
        CB->divers(DV_CUT_SPEECH, 0, 0);
        CB->divers_empile(DV_WAS_HIT, P_GUARDIAN23, 1);
        CB->combat(P_GUARDIAN23, -1);
        CB->stop_joindre(-P_GUARDIAN23);
        CB->animation("SP88IMP51");
        CB->animation("P88CAD");
        CB->pos_perso(P_GUARDIAN23, 0, 0, 0, 2, "P88CAD", 0x11, 0);
        if (ST(ST_VIE(P_KABAK)) == 1) {
            ST(V_KABAK_FIGHT_ON) = 1;
            CB->select_pile(1, 2);
            CB->dialogue("L01D148", P_KABAK);
            CB->combat(P_KABAK, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x9A0B */
    if (ST(ST_VIE(P_KABAK)) == 1 && ST(V_GUARD_WAVE) == 0) {
        if (ST(L01_KABAK_NEAR_STATE) == 0 && CB->distance(6, 9) <= 0xFA)
            ST(L01_KABAK_NEAR_STATE) = 1;
        /* 0x9A65 */
        if (ST(L01_KABAK_NEAR_STATE) == 2 && ST(ST_LOCKED_CAMERA) != 2 && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L01_KABAK_NEAR_STATE) = 0;
            CB->select_pile(2, 1);
            CB->stop_joindre(-P_KABAK);
            if (ST(L01_KABAK_GUARD) == 1) {
                ST(L01_KABAK_GUARD) = 0;
                CB->bloque(BLOQUE_ALL);
                CB->animation("SP87GAR27");
                CB->animation("P87ARR01_10");
                CB->bloque(BLOQUE_NONE);
            }
            /* 0x9B06 */
            CB->joindre(P_KABAK, 0, 0, 0x5DC0, -1);
            CB->animation("SP87ATT04");
            CB->select_pile(0, 1);
        }
    }
    /* 0x9B34 */
    if (ST(ST_VIE(P_KABAK)) == 1 && ST(V_GUARD_WAVE) != 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(L01_KABAK_GUARD) == 0) {
        if (ST(L01_KABAK_NEAR_STATE) == 0 && CB->distance(6, 9) <= 0xFA)
            ST(L01_KABAK_NEAR_STATE) = 1;
        /* 0x9BAF */
        if (ST(L01_KABAK_NEAR_STATE) == 2
            && (ST(ST_LOCKED_CAMERA) == 0 || ST(ST_LOCKED_CAMERA) == 6 || ST(ST_LOCKED_CAMERA) == 3)
            && CB->distance(6, 9) > 0x113) {
            ST(L01_KABAK_NEAR_STATE) = 0;
            CB->select_pile(2, 1);
            CB->stop_joindre(-P_KABAK);
            CB->pos_perso(P_KABAK, 320, -2530, 0xDD4, 1,
                          "P87ATT01", 9, 1);
            if (ST(L01_OB94_LINKED) == 0) {
                ST(L01_OB94_LINKED) = 1;
                CB->link_objet(0x57, "OB94", 0x1B, 1);
            }
            CB->select_pile(0, 1);
        }
    }
    /* 0x9C9D — Sar dialogue driver (guarded) */
    if (ST(ST_VIE(P_KABAK)) < 2 && ST(L01_KABAK_NEAR_STATE) == 1 && ST(V_KABAK_FIGHT_ON) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
        ST(L01_KABAK_NEAR_STATE) = 2;
        if (ST(ST_COMBAT_STANCE) == 0) {
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_KABAK);
            CB->animation("SP87ARR01_10");
            CB->joindre(P_KABAK, 0, 0, 0, ST(ST_HERO));
            if (ST(L01_KABAK_MET) == 0) {
                ST(L01_KABAK_MET) = 1;
                if (ST(ST_HERO) == P_ARKHAN1 && ST(V_KABAK_MET_H1) == 0) {
                    CB->joindre(-ST(ST_HERO), 0, 0, 0, 0x57);
                    CB->animation("sar_10");
                    ST(V_KABAK_MET_H1) = 1;
                    CB->dialogue("L01D66A", P_ARKHAN1);
                    lieu_action_register("L01D67", 0x57, "SP87ATT70", "SP87ARR01_10");
                }
                /* 0x9DDD */
                if (ST(ST_HERO) >= 2) {
                    CB->joindre(-ST(ST_HERO), 0, 0, 0, 0x57);
                    CB->animation("sar_10");
                    if (ST(V_KABAK_TALK_H23) == 0) {
                        ST(V_KABAK_TALK_H23) = 1;
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            if (ST(ST_SPECIAL_MODE) == 1)
                                lieu_action_register("L01D60A", 2, "SG02DIA03", NULL);
                            if (ST(ST_SPECIAL_MODE) == 0)
                                lieu_action_register("L01D60B", 2, "SG02DIA03", NULL);
                        }
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            if (ST(ST_SPECIAL_MODE) == 1)
                                lieu_action_register("L01D61A", 3, "SG03DIA03", NULL);
                            if (ST(ST_SPECIAL_MODE) == 0)
                                lieu_action_register("L01D61B", 3, "SG03DIA03", NULL);
                        }
                        lieu_action_register("L01D30A", 0x57, "SP87DIA50", "SP87ARR01_10");
                        lieu_action_register("L01D30B", 0x57, "SP87DIA56", "SP87ARR01_10");
                    } else {
                        /* 0x9F33 */
                        lieu_action_register("L01D139", 0x57, "SP87DIA50", "SP87ARR01_10");
                    }
                }
                /* 0x9F4C */
                CB->animation("SP87ARR01_10");
                CB->joindre_perm(0x57, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
            } else {
                /* 0x9F7A */
                CB->animation("SP87ARR01_10");
                CB->joindre_perm(0x57, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
            }
        }
        /* 0x9FA3 */
        if (ST(ST_COMBAT_STANCE) == 1) {
            CB->bloque(BLOQUE_ALL);
            ST(L01_KABAK_GUARD) = 1;
            if (ST(L01_OB94_LINKED) == 1) {
                ST(L01_OB94_LINKED) = 0;
                CB->link_objet(0x57, "OB94", 0x1B, 0);
            }
            if (ST(L01_KABAK_MET) == 0) {
                ST(L01_KABAK_MET) = 1;
                if (ST(ST_HERO) == P_ARKHAN1) { ST(V_KABAK_MET_H1) = 1; CB->dialogue("L01D66A", P_ARKHAN1); }
                if (ST(ST_HERO) == P_ARKHAN2) { ST(V_KABAK_TALK_H23) = 1; CB->dialogue("L01D66B", P_ARKHAN2); }
                if (ST(ST_HERO) == P_ARKHAN3) { ST(V_KABAK_TALK_H23) = 1; CB->dialogue("L01D66C", P_ARKHAN3); }
                CB->animation("SP87ARR01_10");
                CB->joindre(-P_KABAK, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP87GAR26");
                CB->animation("P87GAR02_10");
                if ((i32)ST(V_PLAYER_STRIKE) >= 1) {
                    ST(V_ENGAGED_PERSONA) = 0x57;
                    CB->evenement(2, 100);
                }
                if (ST(V_PLAYER_STRIKE) == 0) {
                    if (ST(V_KABAK_D81_SAID) == 0) {
                        ST(V_KABAK_D81_SAID) = 1;
                        if (ST(ST_HERO) == P_ARKHAN1)
                            lieu_action_register("L01D81", 0x57, "SP87DIA01", "SP87GAR02_10");
                        if (ST(ST_HERO) >= 2)
                            lieu_action_register("L01D82", 0x57, "SP87DIA01", "SP87GAR02_10");
                    }
                    CB->joindre_perm(0x57, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                }
                /* [14]!=0 falls through to A223 */
            } else {
                /* 0xA1C7 */
                CB->animation("SP87ARR01_10");
                CB->joindre(-P_KABAK, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP87GAR26");
                CB->animation("P87GAR02_10");
                CB->joindre_perm(0x57, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
            }
        }
    }
    /* 0xA223 */
    if (ST(ST_VIE(P_KABAK)) < 2 && ST(L01_KABAK_NEAR_STATE) == 1 && ST(V_KABAK_FIGHT_ON) == 1 && ST(V_ENGAGED_PERSONA) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_ENGAGED_PERSONA) = 0x57;
        ST(L01_KABAK_NEAR_STATE) = 0x0A;
        ST(L01_KABAK_GUARD) = 1;
        if (ST(L01_OB94_LINKED) == 1) { ST(L01_OB94_LINKED) = 0; CB->link_objet(0x57, "OB94", 0x1B, 0); }
        CB->animation("SP87ARR01_10");
        CB->joindre(-P_KABAK, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP87GAR26");
        lieu_action_register("L01D87", 0x57, "SP87DIA01", "SP87GAR02_10");
        CB->bloque(BLOQUE_NONE);
        if (ST(V_GUARD_WAVE) == 1 && ST(ST_VIE(P_GUARDIAN23)) == 1) {
            if (ST(L01_G23_UP) != 1) {
                ST(L01_G23_UP) = 1;
                CB->animation("SP88LEV01");
                CB->animation("P88_65535");
            }
            CB->combat(P_GUARDIAN23, 0);
        }
        CB->combat(P_KABAK, 0);
    }
    /* 0xA395 */
    if (ST(ST_VIE(P_KABAK)) < 2 && ST(L01_KABAK_NEAR_STATE) == 2 && ST(V_ENGAGED_PERSONA) == 0) {
        if (ST(L01_KABAK_GUARD) == 0 && ST(ST_COMBAT_STANCE) == 1) {
            ST(L01_KABAK_GUARD) = 1;
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_KABAK);
            if (ST(L01_OB94_LINKED) == 1) { ST(L01_OB94_LINKED) = 0; CB->link_objet(0x57, "OB94", 0x1B, 0); }
            CB->joindre(-P_KABAK, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP87ARR01_10");
            CB->animation("P87GAR26");
            if (ST(V_KABAK_D81_SAID) == 0) {
                ST(V_KABAK_D81_SAID) = 1;
                if (ST(ST_HERO) == P_ARKHAN1)
                    lieu_action_register("L01D81", 0x57, "P87DIA01", "SP87GAR02_10");
                if (ST(ST_HERO) >= 2)
                    lieu_action_register("L01D82", 0x57, "P87DIA01", "SP87GAR02_10");
            }
            CB->joindre_perm(0x57, ST(ST_HERO));
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0xA50D */
        if (ST(L01_KABAK_GUARD) == 1 && ST(ST_COMBAT_STANCE) == 0) {
            ST(L01_KABAK_GUARD) = 0;
            CB->bloque(BLOQUE_ALL);
            if (ST(ST_LOCKED_CAMERA) == 0 || ST(ST_LOCKED_CAMERA) == 3 || ST(ST_LOCKED_CAMERA) == 6) {
                CB->dialogue("L01D80", P_KABAK);
                CB->bloque(BLOQUE_NONE);
                return;
            } else {
                CB->stop_joindre(-P_KABAK);
                CB->animation("SP87GAR27");
                CB->animation("P87ARR01_200");
                if (ST(V_KABAK_D80_SAID) == 0 && ST(ST_HERO) >= 2) {
                    ST(V_KABAK_D80_SAID) = 1;
                    lieu_action_register("L01D80", 0x57, "P87DIA59", "SP87ARR01_10");
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->joindre_perm(0x57, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    /* 0xA672 */
    if (ST(V_EVENT_2) == 1 && ST(V_ENGAGED_PERSONA) == 0x57) {
        ST(V_EVENT_2) = 2;
        ST(V_KABAK_INTRO) = 1;
        ST(V_ENGAGED_PERSONA)  = 0;
        CB->joindre(-P_KABAK, 0, 0, 0, ST(ST_HERO));
        if (ST(ST_HERO) == P_ARKHAN1)
            lieu_action_register("L01D137", 0x57, "SP87DIA01", "SP87GAR02_10");
        if (ST(ST_HERO) >= 2)
            lieu_action_register("L01D138A", 0x57, "SP87DIA01", "SP87GAR02_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->joindre_perm(0x57, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xA76F — imp W distance-gated dialogue/attack */
    if (ST(ST_VIE(P_GUARDIAN23)) == 1 && ST(V_ENGAGED_PERSONA) == 0 && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN23, 0)) {
        /* 0xA7AA — sub-block, else falls to B03A */
        if (ST(ST_VIE(P_KABAK)) < 2 && ST(ST_HERO) != 4
            && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) <= -2800
            && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) >= 0x1F4
            && ST(L01_SCENE_CAMERA) == 0) {
            if (ST(L01_G23_AMBUSH) == 0) {
                CB->bloque(BLOQUE_ALL);
                ST(ST_SCRIPT_BUSY)  = 1;
                ST(L01_G23_AMBUSH) = 1;
                ST(L01_SCENE_CAMERA)  = 1;
                CB->change_camera(6, CAM_LOCKED);
                ST(L01_KABAK_NEAR_STATE) = 0;
                CB->pos_perso(P_KABAK, 320, -2530, 0x8CA0, 1, "P87ARR01", 9, 1);
                if (ST(ST_VIE(P_KABAK)) == 1 && ST(V_KABAK_TALK_H23) == 0) {
                    ST(V_KABAK_TALK_H23) = 1;
                    ST(L01_G23_UP)  = 1;
                    CB->vire_col(1, 0);
                    CB->animation("SP88LEV01");
                    CB->animation("P88_65535");
                    CB->joindre(-P_GUARDIAN23, 0, 0, 0, ST(ST_HERO));
                    lieu_action_register("L01D121", 0x58, "SP88DIA59", "SP88ARR01");
                    CB->change_camera(2, CAM_LOCKED);
                    CB->joindre(-P_KABAK, 0, 0, 0, 0x58);
                    if (ST(L01_KABAK_GUARD) == 0)
                        lieu_action_register("L01D31A", 0x57, "SP87DIA55", "SP87ARR01_100");
                    if (ST(L01_KABAK_GUARD) != 0) {
                        ST(L01_KABAK_GUARD) = 0;
                        CB->stop_joindre(-P_KABAK);
                        CB->animation("SP87GAR27");
                        CB->animation("P87ARR01_200");
                        lieu_action_register("L01D31A", 0x57, "P87DIA55", "SP87ARR01_100");
                    }
                    CB->joindre(P_KABAK, 0, 0, 0, ST(ST_HERO));
                    lieu_action_register("L01D30A", 0x57, "SP87DIA50", "SP87ARR01");
                    lieu_action_register("L01D30B", 0x57, "SP87DIA56", "SP87ARR01");
                    CB->joindre(-P_KABAK, 0, 0, 0, 0x58);
                    lieu_action_register("L01D31B", 0x57, "SP87DIA55", "SP87ARR01_100");
                    lieu_action_register("L01D31C", 0x57, "SP87DIA59", "SP87ARR01_100");
                    CB->change_camera(6, CAM_LOCKED);
                    CB->pos_perso(P_KABAK, 320, -2530, 0xDD4, 1, "P87ATT01", 9, 1);
                    if (ST(L01_GUARD_HIT) == 0) {
                        CB->link_objet(0x57, "OB94", 0x1B, 1);
                        ST(L01_OB94_LINKED) = 1;
                    }
                    if (ST(ST_COMBAT_STANCE) == 0)
                        lieu_action_register("L01D122", 0x58, "SP88DIA60", "SP88ARR01_100");
                    if (ST(ST_COMBAT_STANCE) == 1)
                        lieu_action_register("L01D123", 0x58, "SP88DIA60", "SP88GAR01_500");
                    ST(L01_KABAK_MET) = 1;
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(0x58, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0xAB83 */
                if (ST(ST_VIE(P_KABAK)) == 1 && ST(V_KABAK_TALK_H23) == 1) {
                    ST(L01_G23_UP) = 1;
                    CB->vire_col(1, 0);
                    CB->animation("SP88LEV01");
                    CB->animation("P88_65535");
                    CB->joindre(-P_GUARDIAN23, 0, 0, 0, ST(ST_HERO));
                    lieu_action_register("L01D124", 0x58, "SP88DIA59", "SP88ARR01_10");
                    CB->vire_col(1, 1);
                    CB->change_camera(2, CAM_LOCKED);
                    CB->joindre(-P_KABAK, 0, 0, 0, 0x58);
                    if (ST(L01_KABAK_GUARD) == 0 && ST(ST_COMBAT_STANCE) == 0)
                        lieu_action_register("L01D31B", 0x57, "SP87DIA55", "SP87ARR01_100");
                    /* 0xAC7C — [80]/[10] gated L01D171 */
                    {
                        i32 do171;
                        if (ST(L01_KABAK_GUARD) != 0) {
                            ST(L01_KABAK_GUARD) = 0;
                            CB->stop_joindre(-P_KABAK);
                            CB->animation("SP87GAR27");
                            CB->animation("P87ARR01_200");
                            do171 = 1;
                        } else {
                            do171 = (ST(ST_COMBAT_STANCE) == 1) ? 1 : 0;
                        }
                        if (do171)
                            lieu_action_register("L01D171", 0x57, "P87DIA55", "SP87ARR01_100");
                    }
                    lieu_action_register("L01D31C", 0x57, "SP87DIA59", "SP87ARR01_100");
                    CB->change_camera(6, CAM_LOCKED);
                    if (ST(ST_COMBAT_STANCE) == 0)
                        lieu_action_register("L01D122", 0x58, "SP88DIA60", "SP88ARR01_100");
                    if (ST(ST_COMBAT_STANCE) == 1)
                        lieu_action_register("L01D123", 0x58, "SP88DIA60", "SP88GAR01_500");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(0x58, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0xADB7 */
            if (ST(L01_G23_AMBUSH) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0) {
                ST(L01_SCENE_CAMERA) = 1;
                CB->bloque(BLOQUE_ALL);
                ST(ST_SCRIPT_BUSY) = 1;
                ST(L01_KABAK_NEAR_STATE) = 0;
                ST(L01_KABAK_GUARD) = 0;
                CB->change_camera(6, CAM_LOCKED);
                CB->pos_perso(P_KABAK, 320, -2530, 0x8CA0, 1, "P87ARR01", 9, 1);
                if (ST(L01_G23_UP) == 0) {
                    ST(L01_G23_UP) = 1;
                    CB->vire_col(1, 0);
                    CB->animation("SP88LEV01");
                    CB->animation("P88_65535");
                    CB->joindre(-P_GUARDIAN23, 0, 0, 0, ST(ST_HERO));
                }
                if (ST(ST_COMBAT_STANCE) == 0)
                    lieu_action_register("L01D94", 0x58, "SP88DIA54", "SP88ARR01_10");
                if (ST(ST_COMBAT_STANCE) == 1)
                    lieu_action_register("L01D95", 0x58, "SP88DIA51", "SP88ARR01_10");
                CB->vire_col(1, 1);
                CB->change_camera(1, CAM_LOCKED);
                if (ST(L01_KABAK_GUARD) == 0 && ST(ST_COMBAT_STANCE) == 0)
                    lieu_action_register("L01D96", 0x57, "SP87DIA61", "SP87ARR01_10");
                /* 0xAF68 — [80]/[10] gated L01D172 */
                {
                    i32 do172;
                    if (ST(L01_KABAK_GUARD) != 0) {
                        ST(L01_KABAK_GUARD) = 0;
                        CB->stop_joindre(-P_KABAK);
                        CB->animation("SP87GAR27");
                        CB->animation("P87ARR01_200");
                        do172 = 1;
                    } else {
                        do172 = (ST(ST_COMBAT_STANCE) == 1) ? 1 : 0;
                    }
                    if (do172)
                        lieu_action_register("L01D172", 0x57, "SP87DIA61", "SP87ARR01_10");
                }
                CB->change_camera(6, CAM_LOCKED);
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->joindre_perm(0x58, ST(ST_HERO));
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
        /* 0xB03A */
        if (ST(ST_VIE(P_KABAK)) >= 2 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_HERO) != 4
            && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < -2500
            && CB->divers(DV_DESC_POS, ST(ST_HERO), 0) >= 0x1F4) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            ST(L01_G23_UP) = 1;
            ST(V_ENGAGED_PERSONA) = 0x57;
            CB->change_camera(6, CAM_LOCKED);
            CB->vire_col(1, 0);
            CB->animation("SP88LEV01");
            CB->animation("P88_65535");
            CB->joindre(-P_GUARDIAN23, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L01D127", 0x58, "SP88DIA60", "SP88GAR01_500");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->vire_col(1, 1);
            CB->change_camera(0, CAM_AUTO);
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_GUARDIAN23, 0);
            return;
        }
    }
    /* 0xB196 */
    if (ST(ST_VIE(P_GUARDIAN23)) == VIE_DEAD && ST(V_ENGAGED_PERSONA) != 0 && ST(ST_VIE(P_KABAK)) >= 2)
        ST(V_ENGAGED_PERSONA) = 0;
    /* 0xB1D5 */
    if (ST(ST_VIE(P_GUARDIAN23)) == VIE_DEAD && ST(V_G23_FIGHT_ON) == 1)
        ST(V_G23_FIGHT_ON) = 0;
    /* 0xB208 */
    if (ST(ST_VIE(P_GUARDIAN23)) < 2 && ST(V_ENGAGED_PERSONA) == 0 && CB->distance(6, 17) > 0x12C
        && (ST(ST_LOCKED_CAMERA) == 2 || ST(ST_LOCKED_CAMERA) == 3) && ST(L01_G23_UP) == 1) {
        CB->stop_joindre(-P_GUARDIAN23);
        ST(L01_G23_FOLLOW_STEP) = 0x0A;
        ST(L01_G23_UP) = 0;
        CB->pos_perso(P_GUARDIAN23, 578, -3010, 0x1388, 1, "P88ARR02", 0x11, 1);
    }
    /* 0xB2C8 */
    if (ST(ST_LOCKED_CAMERA) == 6 && CB->distance(6, 4) > 0xC8 && CB->distance(6, 7) > 0xC8
        && ST(L01_SCENE_CAMERA) != 0 && ST(V_LEFT_HOME) == 0 && CB->divers(DV_PERSONA_PRESENT, P_KALHI12, 0)) {
        CB->change_camera(CAM_ANY, CAM_AUTO);
        ST(L01_SCENE_CAMERA) = 0;
    }
    /* 0xB36D */
    if (ST(ST_LOCKED_CAMERA) == 6 && CB->distance(6, 7) > 0x12C && ST(L01_SCENE_CAMERA) != 0
        && ST(V_LEFT_HOME) == 1 && ST(V_GUARD_WAVE) == 0) {
        CB->change_camera(CAM_ANY, CAM_AUTO);
        ST(L01_SCENE_CAMERA) = 0;
    }
    /* 0xB3EF */
    if (ST(ST_LOCKED_CAMERA) == 6 && CB->distance(6, 17) > 0xC8 && ST(L01_SCENE_CAMERA) != 0
        && ST(V_LEFT_HOME) == 1 && ST(V_GUARD_WAVE) == 1 && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN23, 0)) {
        CB->change_camera(CAM_ANY, CAM_AUTO);
        ST(L01_SCENE_CAMERA) = 0;
    }
    /* 0xB48B */
    if (ST(ST_LOCKED_CAMERA) == 6 && CB->distance(6, 18) > 0xC8 && ST(L01_SCENE_CAMERA) != 0
        && ST(V_LEFT_HOME) == 1 && ST(V_GUARD_WAVE) == 2 && CB->divers(DV_PERSONA_PRESENT, P_GUARDIAN24, 0)) {
        CB->change_camera(CAM_ANY, CAM_AUTO);
        ST(L01_SCENE_CAMERA) = 0;
    }
    /* 0xB527 */
    if (ST(L01_G23_FOLLOW_STEP) == 1) {
        ST(L01_G23_FOLLOW_STEP) = 2;
        CB->joindre_perm(0x58, ST(ST_HERO));
    }
    /* 0xB55E */
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail from FonctionEditor case 1.
 * Callback ingestion is skipped: g_lieu_cb is provided by the bridge.
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
        l01_trig00();   /* sub_10001073 default path */
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
 * TICK (opcode 0) + DEINIT (opcode -1): drain pending sound, run body.
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
 * Entry point — mirrors @FonctionEditor@16 opcode dispatch.
 * ---------------------------------------------------------------------- */
i32 g_lieu_trace = 0;

i32 lieu01_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu01: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l01_trigger(arg); break;
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
