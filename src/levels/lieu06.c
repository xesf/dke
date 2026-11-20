/*
 * lieu06.c — Native hand-port of LIEU06.DLL (level-6 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu06_trig.c (reached through
 * l06_trigger, see lieu06_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..03; the
 * entry is at 0x1000FC20, the state pointer global is dword_1002192C and the
 * trigger table is dword_10021840[42].
 *
 * event4 (sub_1000123A), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_100102EC), lieu_pre_scene (sub_1001049C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT follow LIEU03 instruction for instruction. The tick body
 * (sub_1000836C) is level-6 specific.
 */

#include <stdlib.h>

#include "lieu06_int.h"

/* -------------------------------------------------------------------------
 * Dhorkan's taunt: the dialogue line to play cycles with ST(L06_TAUNT_INDEX) (0..5,
 * then back to 0). The DLL repeats the whole chain inline at every site
 * (tick body and triggers 2, 13 and 15); one copy here with the same code.
 * Variant A is the usual order, variant B the one played when ST(L06_TAUNT_VARIANT) > 0.
 * ---------------------------------------------------------------------- */
void l06_taunt_a(void) {
    if (ST(L06_TAUNT_INDEX) == 0) {
        lieu_action_register("L06D77B", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 1) {
        lieu_action_register("L06D214B", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 2) {
        lieu_action_register("L06D16A", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 3) {
        lieu_action_register("L06D53B", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 4) {
        lieu_action_register("L06D16F", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 5) {
        lieu_action_register("L06D80C", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) > 5) {
        ST(L06_TAUNT_INDEX) = 0;
    } else {
        ST(L06_TAUNT_INDEX)++;
    }
}

void l06_taunt_b(void) {
    if (ST(L06_TAUNT_INDEX) == 0) {
        lieu_action_register("L06D18C", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 1) {
        lieu_action_register("L06D20", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 2) {
        lieu_action_register("L06D26A", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 3) {
        lieu_action_register("L06D26H", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 4) {
        lieu_action_register("L06D80C", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) == 5) {
        lieu_action_register("L06D53B", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
    }
    if (ST(L06_TAUNT_INDEX) > 5) {
        ST(L06_TAUNT_INDEX) = 0;
    } else {
        ST(L06_TAUNT_INDEX)++;
    }
}

/* -------------------------------------------------------------------------
 * Per-frame body — sub_1000836C (thunk 0x100010B9), level-6 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x837E */
    if (ST(ST_HERO) == 4 && ST(V_DEATH_EVENT_SENT) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(V_DEATH_EVENT_SENT) = 1;
        /* 0x83C1 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x83EA */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x8416 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x8442 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8466 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x8483 */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x84AF */
    if (ST(V_EVENT_1) == 1) {
        ST(ST_VIE(P_DHORKAN11)) = VIE_DEAD;
        ST(V_EVENT_1) = 0;
    }
    /* 0x84E7 */
    if (CB->divers(DV_WAS_HIT, P_DHORKAN11, 0) && ST(ST_VIE(P_DHORKAN11)) != VIE_DEAD) {
        CB->select_pile(1, 2);
        CB->divers_empile(DV_WAS_HIT, P_DHORKAN11, 1);
        /* 0x852A */
        if (ST(V_ENGAGED_PERSONA) == 0) {
            CB->bloque(BLOQUE_ALL);
        }
        CB->combat(P_DHORKAN11, -1);
        CB->vire_col(33, 0);
        ST(L06_DHORKAN_HP_TMP) = CB->divers(DV_HP, P_DHORKAN11, -1);
        /* 0x858F */
        if ((ST(L06_DHORKAN_HP_TMP) - ST(V_HIT_DAMAGE)) > 0) {
            CB->divers(DV_HP, P_DHORKAN11, (ST(L06_DHORKAN_HP_TMP) - ST(V_HIT_DAMAGE)));
        } else {
            CB->divers(DV_HP, P_DHORKAN11, 1);
        }
        /* 0x85E9 */
        if (CB->divers(DV_HP, P_DHORKAN11, -1) <= 1 && ST(ST_VIE(P_DHORKAN11)) != VIE_DEAD) {
            CB->stop_joindre(-P_DHORKAN11);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_DHORKAN11, 0);
            CB->animation("SP11IMP52");
            CB->pos_perso(P_DHORKAN11, 0, 0, 0, 2, "P11CAD", 4, 0);
            CB->divers_empile(DV_SCREEN, 2, 0);
            CB->evenement(1, 2000);
        } else {
            CB->stop_joindre(-P_DHORKAN11);
            CB->divers_empile(DV_BLOOD_AT_BONE, P_DHORKAN11, 0);
            CB->animation("SP11IMP12");
            CB->animation("P11IMP47");
            /* 0x86A8 */
            if (ST(V_ENGAGED_PERSONA) != 0) {
                CB->animation("P11GAR02");
            }
            /* 0x86C2 */
            if (ST(V_ENGAGED_PERSONA) == 0) {
                ST(V_ENGAGED_PERSONA) = P_DHORKAN11;
                ST(L06_DHORKAN_ENGAGED) = 1;
                CB->evenement(7, -1);
                CB->retire_zone(12);
                lieu_action_register("L06D32", P_DHORKAN11, "P11DIA60", "SP11GAR26_10");
                CB->animation("P11_65535");
                CB->bloque(BLOQUE_NONE);
            }
            CB->combat(P_DHORKAN11, 0);
        }
        CB->select_pile(0, 1);
    }
    /* 0x8750 */
    if (ST(ST_VIE(P_DHORKAN11)) != VIE_DEAD && ST(V_EVENT_14) < 2 && ST(ST_HERO) == P_ARKHAN1 && ST(L06_DHORKAN_INTRO_STEP) == 1) {
        ST(L06_DHORKAN_INTRO_STEP) = 2;
        ST(V_EVENT_14) = 2;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP11ATT04_10");
        CB->animation("sar_10");
        CB->joindre(ST(ST_HERO), 0, 0, 0, P_DHORKAN11);
        /* 0x87FB */
        if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 2100) {
            CB->change_camera(2, CAM_LOCKED);
        } else {
            CB->change_camera(1, CAM_LOCKED);
        }
        /* 0x8832 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            lieu_action_register("L06D42A", P_ARKHAN1, "SP01ATT74", NULL);
            CB->animation("SP11ATT02");
            CB->dialogue("L06D05A", P_DHORKAN11);
            CB->animation("P11_65535");
            CB->joindre(P_DHORKAN11, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L06D42B", P_ARKHAN1, "G00DIA06", NULL);
        } else {
            lieu_action_register("L06D42C", P_ARKHAN1, "SP01ATT75", NULL);
            CB->animation("SP11ATT02");
            CB->dialogue("L06D05A", P_DHORKAN11);
            CB->animation("P11_65535");
            CB->joindre(P_DHORKAN11, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L06D42D", P_ARKHAN1, "G00DIA06", NULL);
        }
        lieu_action_register("L06D05B", P_DHORKAN11, "SP11DIA62", "SP11ARR02_10");
        lieu_action_register("L06D05C", P_DHORKAN11, "SP11DIA65", "SP11ARR02_10");
        lieu_action_register("L06D05D", P_DHORKAN11, "SP11DIA57", "SP11ARR02_10");
        lieu_action_register("L06D05E", P_DHORKAN11, "SP11DIA58", "SP11ARR02_10");
        lieu_action_register("L06D05F", P_DHORKAN11, "SP11DIA64", "SP11ARR02_10");
        CB->modify_var(&ST(L06_DHORKAN_TALK_STEP), 1);
        CB->modify_var(&ST(V_EVENT_7), 1);
        CB->change_camera(0, CAM_LOCKED);
        CB->change_camera(0, CAM_AUTO);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x89F1 */
    if (ST(L06_CAMERA_HOLD) == 1 && ST(ST_UPDOWN_KEY) == 1) {
        ST(L06_CAMERA_HOLD) = 0;
        CB->change_camera(1, CAM_AUTO);
    }
    /* 0x8A31 */
    if (ST(V_EVENT_2) == 1) {
        ST(V_EVENT_2) = 0;
        CB->bloque(BLOQUE_NONE);
        ST(ST_LEVEL) = 6;
        CB->divers_empile(DV_MUSIC, 1, 4);
        CB->change_piece(2, -1);
    }
    /* 0x8A88 */
    if (ST(ST_VIE(P_DHORKAN11)) != VIE_DEAD && ST(V_EVENT_14) >= 2 && ST(L06_DHORKAN_INTRO_STEP) == 1 && ST(ST_HERO) == P_ARKHAN1 && ST(L06_D03_DONE) == 0) {
        ST(V_EVENT_7) = 0;
        ST(L06_DHORKAN_INTRO_STEP) = 2;
        CB->bloque(BLOQUE_ALL);
        CB->animation("SP11ATT02");
        CB->animation("P11_65535");
        lieu_action_register("L06D04", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
        CB->animation("sar_100");
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L06_DHORKAN_TALK_STEP), 1);
        CB->modify_var(&ST(V_EVENT_7), 1);
    }
    /* 0x8B6F */
    if (ST(V_EVENT_7) == 1 && ST(L06_DHORKAN_EVENT7_STEP) == 0) {
        ST(L06_DHORKAN_EVENT7_STEP) = 1;
        CB->evenement(7, 15000);
    }
    /* 0x8BAC */
    if (ST(V_EVENT_7) == 2 && ST(L06_DHORKAN_EVENT7_STEP) == 1) {
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(P_DHORKAN11);
        ST(L06_DHORKAN_TALK_STEP) = 3;
        lieu_action_register("L06D06A", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
        CB->evenement(7, 8000);
        ST(L06_DHORKAN_EVENT7_STEP) = 2;
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L06_DHORKAN_TALK_STEP), 1);
    }
    /* 0x8C40 */
    if (ST(V_EVENT_7) == 3 && ST(L06_DHORKAN_EVENT7_STEP) == 2) {
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(P_DHORKAN11);
        ST(L06_DHORKAN_TALK_STEP) = 3;
        lieu_action_register("L06D06B", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
        lieu_action_register("L06D06C", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
        CB->evenement(7, 8000);
        ST(L06_DHORKAN_EVENT7_STEP) = 3;
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L06_DHORKAN_TALK_STEP), 1);
    }
    /* 0x8CED */
    if (ST(V_EVENT_7) == 4 && ST(ST_HERO) == P_ARKHAN1 && ST(L06_DHORKAN_EVENT7_STEP) == 3) {
        CB->bloque(BLOQUE_ALL);
        CB->stop_joindre(-P_DHORKAN11);
        ST(L06_DHORKAN_TALK_STEP) = 3;
        lieu_action_register("L06D06D", P_DHORKAN11, "SP11DIA59", "SP11ARR02_10");
        lieu_action_register("L06D06E", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
        CB->evenement(7, 10000);
        ST(L06_DHORKAN_EVENT7_STEP) = 4;
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(L06_DHORKAN_TALK_STEP), 1);
    }
    /* 0x8DAC */
    if (ST(V_EVENT_7) == 4 && ST(ST_HERO) == P_ARKHAN2) {
        ST(V_EVENT_7) = 5;
        ST(L06_DHORKAN_EVENT7_STEP) = 4;
    }
    /* 0x8DEB */
    if (ST(V_EVENT_7) == 5 && ST(L06_PHEDORIA_FIGHT) < 2) {
        /* 0x8E0C */
        if (ST(L06_DHORKAN_EVENT7_STEP) == 4) {
            CB->change_camera(-2, CAM_AUTO);
            CB->bloque(BLOQUE_ALL);
            ST(L06_CAMERA_HOLD) = 1;
            ST(V_ENGAGED_PERSONA) = P_DHORKAN11;
            ST(ST_SCRIPT_BUSY) = 1;
            ST(L06_DHORKAN_ENGAGED) = 1;
            ST(L06_DHORKAN_EVENT7_STEP) = 5;
            ST(L06_DHORKAN_TALK_STEP) = 3;
            CB->stop_joindre(-P_DHORKAN11);
            lieu_action_register("L06D06F", P_DHORKAN11, "SP11DIA60", "SP11ARR02_10");
            CB->animation("P11_65535");
            /* 0x8EC1 */
            if (CB->divers(DV_DESC_POS, P_ARKHAN1, 1) <= -1285 && ST(L06_DHORKAN_CHASE_STEP) == 0) {
                /* 0x8EED */
                if (CB->divers(DV_DESC_POS, P_ARKHAN1, 0) <= 2065) {
                    CB->joindre(-P_DHORKAN11, 1852, -1210, 0x7A8A, -1);
                }
                /* 0x8F26 */
                if (CB->divers(DV_DESC_POS, P_ARKHAN1, 0) > 2065) {
                    CB->pos_perso(P_DHORKAN11, 2170, -1210, 0xC8DE, 1, "P11ATT01", 4, 0);
                    ST(L06_DHORKAN_POS_X) = CB->divers(DV_DESC_POS, P_DHORKAN11, 0);
                    ST(L06_DHORKAN_POS_Y) = CB->divers(DV_DESC_POS, P_DHORKAN11, 1);
                    CB->joindre(-P_DHORKAN11, 2300, -1300, 0x6B6C, -1);
                }
                CB->modify_var(&ST(L06_DHORKAN_CHASE_STEP), 1);
            }
            /* 0x8FD4 */
            if (CB->divers(DV_DESC_POS, P_ARKHAN1, 1) > -1285) {
                CB->animation("SP11GAR26");
                CB->animation("P11GAR02_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->combat(P_DHORKAN11, 0);
                CB->bloque(BLOQUE_NONE);
            }
        }
        /* 0x901F */
        if (ST(L06_DHORKAN_CHASE_STEP) == 1 && !CB->on_joindre(P_DHORKAN11)) {
            ST(L06_DHORKAN_CHASE_STEP) = 2;
            CB->animation("SP11GAR26");
            CB->animation("P11GAR02_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->combat(P_DHORKAN11, 0);
            CB->bloque(BLOQUE_NONE);
        }
    }
    /* 0x908C */
    if (ST(ST_HERO) >= 2 && ST(L06_DHORKAN_EVENT7_STEP) == 1) {
        ST(L06_DHORKAN_EVENT7_STEP) = 2;
        ST(L06_DHORKAN_TALK_STEP) = 3;
        CB->stop_joindre(-P_DHORKAN11);
        CB->change_camera(-2, CAM_AUTO);
        /* 0x90EB */
        if (CB->divers(DV_DESC_POS, P_ARKHAN1, 1) <= -1285 && ST(L06_DHORKAN_CHASE_STEP) == 0) {
            /* 0x9117 */
            if (CB->divers(DV_DESC_POS, P_ARKHAN1, 0) <= 2065) {
                CB->joindre(-P_DHORKAN11, 1852, -1210, 0x7A8A, -1);
            }
            /* 0x9150 */
            if (CB->divers(DV_DESC_POS, P_ARKHAN1, 0) > 2065) {
                CB->joindre(-P_DHORKAN11, 2283, -1300, 0x6B6C, -1);
            }
            CB->modify_var(&ST(L06_DHORKAN_CHASE_STEP), 1);
        }
        /* 0x919D */
        if (CB->divers(DV_DESC_POS, P_ARKHAN1, 1) > -1285) {
            ST(V_PLAYER_STRIKE) = -1;
            ST(V_ENGAGED_PERSONA) = P_DHORKAN11;
            ST(L06_CAMERA_HOLD) = 0;
            lieu_action_register("L06D28Z", P_DHORKAN11, "SP11DIA60", "SP11GAR26_10");
            CB->animation("P11_65535");
            CB->modify_var(&ST(L06_CAMERA_HOLD), 1);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->change_camera(1, CAM_AUTO);
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_DHORKAN11, 0);
        }
    }
    /* 0x9247 */
    if (ST(ST_HERO) >= 2 && ST(L06_DHORKAN_CHASE_STEP) == 1 && !CB->on_joindre(P_DHORKAN11)) {
        ST(L06_DHORKAN_CHASE_STEP) = 2;
        ST(L06_CAMERA_HOLD) = 0;
        ST(V_PLAYER_STRIKE) = -1;
        ST(V_ENGAGED_PERSONA) = P_DHORKAN11;
        lieu_action_register("L06D28Z", P_DHORKAN11, "SP11DIA60", "SP11GAR26_10");
        CB->animation("P11_65535");
        CB->modify_var(&ST(L06_CAMERA_HOLD), 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->change_camera(1, CAM_AUTO);
        CB->bloque(BLOQUE_NONE);
        CB->combat(P_DHORKAN11, 0);
    }
    /* 0x931C */
    if (ST(L06_DHORKAN_TALK_STEP) == 1) {
        ST(L06_DHORKAN_TALK_STEP) = 2;
        CB->joindre_perm(P_DHORKAN11, ST(ST_HERO));
    }
    /* 0x9350 */
    if (ST(ST_VIE(P_DHORKAN11)) == VIE_DEAD && ST(L06_PHEDORIA_FIGHT) < 2 && ST(V_GUARD_FIGHT_DONE) < 2) {
        /* 0x9386 */
        if (ST(L06_D07_STEP) == 0) {
            CB->bloque(BLOQUE_ALL);
            CB->print_msg("phedoria alert\351e", 0, 240);
            ST(L06_D07_STEP) = 1;
            CB->combat(P_DHORKAN11, -1);
            CB->stop_joindre(-P_PHEDORIA);
            CB->change_camera(-1, CAM_LOCKED);
            CB->retire_zone(1);
            /* 0x940B */
            if (CB->divers(DV_DESC_POS, ST(ST_HERO), 0) < 1960 && CB->divers(DV_DESC_POS, ST(ST_HERO), 1) < -1760) {
                ST(L06_D84_SAID) = 1;
                CB->dialogue("L06D84", P_PHEDORIA);
                CB->joindre(-ST(ST_HERO), 2050, -1620, 0xD2F0, -1);
            }
            CB->bruitage("porte2r", 1834, -1875);
            CB->anim_objet("IM70", -3, 90, 700, 400);
            CB->joindre(-P_PHEDORIA, 1834, -1825, 0xCF35, -1);
            CB->modify_var(&ST(L06_D07_STEP), 2);
        }
        /* 0x94E9 */
        if (ST(L06_D07_STEP) == 2 && !CB->on_joindre(P_PHEDORIA)) {
            ST(L06_D07_STEP) = 3;
            CB->stop_joindre(-ST(ST_HERO));
            CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->joindre(-ST(ST_HERO), 0, 0, 0, P_PHEDORIA);
            CB->animation("SP26GAR03");
            CB->dialogue("L06D07", P_PHEDORIA);
            CB->animation("SG26FRA01");
            CB->evenement(0, 1000);
        }
        /* 0x959F */
        if (ST(V_EVENT_0) == 1) {
            ST(V_EVENT_0) = 0;
            /* 0x95C0 */
            if (ST(ST_HERO) == P_ARKHAN1) {
                CB->animation("SG01ATT07");
            }
            /* 0x95DD */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SG02ATT07");
            }
            /* 0x95FA */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SG03ATT07");
            }
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            CB->animation("P26GAR03");
            CB->divers_empile(DV_HP, ST(ST_HERO), 0);
            CB->modify_var(&ST(ST_HERO), 4);
            return;
        }
    }
    /* 0x966E */
    if (ST(V_EVENT15_STEP) == 1) {
        ST(V_EVENT15_STEP) = 2;
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        lieu_action_register("L06D62", P_PHEDORIA, "SP26ATT50", "SP26ARR03");
        CB->change_camera(3, CAM_LOCKED);
        CB->pos_perso(P_DHORKAN11, 2027, -777, 0x3A98, 1, "P11ARR02", 4, 0);
        CB->animation("SP11ARR02_10");
        CB->evenement(15, 200);
    }
    /* 0x970F */
    if (ST(V_EVENT15_STEP) == 3) {
        ST(V_EVENT15_STEP) = 4;
        CB->joindre(-P_DHORKAN11, 0, 0, 0x7530, -1);
        CB->play_anim(0, 0);
        CB->bruitage("L06B01", 1863, -543);
        CB->evenement(15, 200);
    }
    /* 0x9775 */
    if (ST(V_EVENT15_STEP) == 5) {
        ST(V_EVENT15_STEP) = 6;
        CB->evenement(15, 200);
        CB->change_piece(6, 0);
    }
    /* 0x97B3 */
    if (ST(V_EVENT15_STEP) == 7) {
        ST(V_EVENT15_STEP) = 8;
        lieu_action_register("L06D39", P_DHORKAN11, "SP11ATT50", "SP11ARR02_10");
        CB->change_camera(0, CAM_LOCKED);
        CB->change_camera(0, CAM_AUTO);
        CB->vire_col(18, 0);
        CB->divers(DV_RUN, P_DHORKAN11, 1);
        CB->joindre(-P_DHORKAN11, 1913, -606, -1, -1);
        CB->evenement(15, 200);
    }
    /* 0x984F */
    if (ST(V_EVENT15_STEP) == 9) {
        ST(V_EVENT15_STEP) = 10;
        CB->animation("SG26FRA01");
        lieu_action_register("L06D40", P_PHEDORIA, "P26GAR27", "SP26ARR03");
        CB->animation("Sar_10");
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->evenement(15, 200);
    }
    /* 0x98C0 */
    if (ST(V_EVENT15_STEP) == 11) {
        ST(V_EVENT15_STEP) = 12;
        CB->play_anim(2, 0);
        CB->bruitage("L06B02", 1863, -543);
        CB->ajoute_zone(10, 1491, -1860, 80, 80, 1);
        CB->evenement(15, 200);
    }
    /* 0x992E */
    if (ST(V_EVENT15_STEP) == 13) {
        ST(V_EVENT15_STEP) = 14;
        CB->change_piece(6, 1);
    }
    /* 0x995F */
    if (ST(L06_PHEDORIA_FIGHT) == 2 && ST(ST_VIE(P_DHORKAN11)) == 1 && CB->distance(4, 18) < 150 && ST(V_EVENT15_STEP) == 14) {
        ST(ST_VIE(P_DHORKAN11)) = 4;
        ST(V_EVENT15_STEP) = 15;
        CB->vire_col(18, 1);
        CB->divers_empile(DV_MUSIC, 1, 4);
        CB->pos_perso(P_DHORKAN11, 1863, -543, 0x9C40, 4, "P11ARR02", 4, 0);
        /* 0x9A15 */
        if (ST(ST_SPECIAL_MODE) == 1) {
            /* 0x9A27 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L06D35A", P_ARKHAN2, "SG00DIA01", NULL);
            }
            /* 0x9A4F */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L06D35B", P_ARKHAN3, "SG00DIA01", NULL);
            }
            lieu_action_register("L06D41", P_PHEDORIA, "SP26DIA60", "SP26ARR03_10");
        }
        /* 0x9A90 */
        if (ST(ST_SPECIAL_MODE) == 0) {
            /* 0x9AA2 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L06D33B", P_ARKHAN2, "SG00DIA02", NULL);
            }
            /* 0x9ACA */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L06D33D", P_ARKHAN3, "SG00DIA02", NULL);
            }
            lieu_action_register("L06D41", P_PHEDORIA, "SP26DIA60", "SP26ARR03_10");
        }
        CB->divers(DV_RUN, P_PHEDORIA, 1);
        CB->joindre(P_PHEDORIA, 1491, -1860, -1, -1);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x9B43 */
    if (ST(V_EVENT_1) == 1) {
        ST(V_EVENT_1) = 0;
        CB->change_camera(3, CAM_LOCKED);
        CB->change_camera(3, CAM_AUTO);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9B8C */
    if (ST(L06_ARRIVE_DONE) == 0 && ST(L06_PHEDORIA_FIGHT) == 2 && ST(V_GUARD_FIGHT_DONE) < 2 && !CB->on_joindre(P_PHEDORIA) && CB->distance(10, 5) <= 100) {
        ST(L06_ARRIVE_DONE) = 1;
        ST(L06_D61_SAID) = 1;
        CB->pos_perso(P_PHEDORIA, 1491, -1860, -1, 4, "P26ARR01", 5, 0);
    }
    /* 0x9C2E */
    if (ST(L06_D670_STAGE) == 1) {
        ST(L06_D670_STAGE) = 3;
        CB->change_piece(6, 0);
    }
    /* 0x9C5C */
    if (ST(L06_D670_STAGE) == 2) {
        ST(L06_D670_STAGE) = 3;
        CB->change_piece(6, 1);
    }
    /* 0x9C8D */
    if (ST(V_EVENT_53) == 1 && !CB->on_joindre(ST(ST_HERO))) {
        ST(V_EVENT_53) = 2;
        CB->stop_joindre(-ST(ST_HERO));
        CB->animation("Sar_10");
        CB->pos_perso(ST(ST_HERO), 1800, -421, 0x5596, 1, "ar", 8, 0);
        CB->evenement(53, 10);
    }
    /* 0x9D22 */
    if (ST(V_EVENT_53) == 3) {
        ST(V_EVENT_53) = 4;
        /* 0x9D43 */
        if (ST(ST_PLAYER_WEAPON) == 0) {
            /* 0x9D55 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02MAR90");
            }
            /* 0x9D72 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03MAR90");
            }
        }
        /* 0x9D8F */
        if (ST(ST_PLAYER_WEAPON) != 0) {
            /* 0x9DA1 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("SP02MAR92");
            }
            /* 0x9DBE */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("SP03MAR92");
            }
        }
        CB->evenement(53, 2000);
    }
    /* 0x9DEB */
    if (ST(V_EVENT_53) == 5) {
        ST(V_EVENT_53) = 0;
        ST(ST_LEVEL) = 6;
        CB->divers_empile(DV_MUSIC, 1, 4);
        CB->evenement(53, -1);
        /* 0x9E3A */
        if (ST(V_ROUTE_STAGE) >= 3) {
            CB->change_piece(46, 1);
        } else {
            CB->change_piece(46, -1);
        }
    }
    /* 0x9E71 */
    if (ST(V_GUARD_FIGHT_DONE) >= 2 && ST(L06_PHEDORIA_FIGHT) < 2 && ST(L06_D82_FLAG) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(L06_FLAG_F24) < 2 && ST(L06_FLAG_CA0) != 5) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(L06_D82_FLAG) = 1;
        /* 0x9F00 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L06D82", P_ARKHAN2, "SG00DIA01", NULL);
        }
        /* 0x9F28 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L06D83", P_ARKHAN3, "SG00DIA01", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9F69 */
    if (ST(V_CRI_KEY) == 1 && ST(V_CRI_USED) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 1 && ST(ST_VIE(P_DHORKAN11)) == 1 && ST(L06_PHEDORIA_FIGHT) == 0 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 4)) {
        ST(V_CRI_USED) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0xA009 */
        if (ST(ST_HERO) == P_ARKHAN1) {
            l06_taunt_a();
        }
        /* 0xA14E */
        if (ST(ST_HERO) == P_ARKHAN2 || ST(ST_HERO) == P_ARKHAN3) {
            /* 0xA172 */
            if (ST(L06_TAUNT_VARIANT) == 0) {
                l06_taunt_a();
            }
            /* 0xA2B7 */
            if (ST(L06_TAUNT_VARIANT) > 0) {
                l06_taunt_b();
            }
        }
        CB->animation("Sar_10");
        CB->modify_var(&ST(V_CRI_USED), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0xA428 */
    if (ST(V_EVENT_0) == 1) {
        ST(V_EVENT_0) = 0;
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000FFB9).
 * Callback ingestion (0x1000FC3A..0x1000FDFB), the trigger-table fill and
 * the unused dword_100219AC = state+0xC8 are skipped: g_lieu_cb is provided
 * by the bridge, the trigger table is static in lieu06_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x100101E4);
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
        l06_trig00();   /* 0x100101E9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x100100CE — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1001023C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000FC20).
 * Jump table @ 0x100102B9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu06_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu06: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l06_trigger(arg);             break;  /* dword_10021840[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
