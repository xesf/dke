/*
 * lieu07.c — Native hand-port of LIEU07.DLL (level-7 scripting): the
 * @FonctionEditor@16 entry, INIT, the per-frame tick and the tick body. The
 * 42 zone trigger handlers live in lieu07_trig.c (reached through
 * l07_trigger, see lieu07_int.h).
 *
 * Ported directly from disassembly (ImageBase 0x10000000). Each function
 * carries its original address. Same compiler/layout as LIEU01..LIEU03; the
 * entry is at 0x1000DF20, the state pointer global is dword_1001E2AC and the
 * trigger table is dword_1001E1C0[42].
 *
 * event4 (sub_100010F5 thunk), event5 (sub_100022B4), event6 (sub_10005BEC),
 * lieu_action_register (sub_1000E5EC), lieu_pre_scene (sub_1000E79C) and
 * sub_10001200 (float reset) are the shared lieu_common.c; the entry and
 * INIT follow LIEU03. tick_body (sub_10007F20) is level-7 specific.
 */

#include <stdlib.h>

#include "lieu07_int.h"

/* -------------------------------------------------------------------------
 * Per-frame body — sub_10007F20 (thunk 0x100010B9), level-7 specific.
 * Ported faithfully from disassembly (labels in comments = original
 * addresses). `return` == original `jmp` to the epilogue.
 * ---------------------------------------------------------------------- */
static void tick_body(void) {
    /* 0x7F35 */
    if (ST(V_DEATH_EVENT_SENT) == 1) {
        ST(V_DEATH_EVENT_SENT) = 0;
        /* 0x7F5B */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN0, 0)) {
            CB->divers(DV_HP, P_ARKHAN0, 0);
        }
        /* 0x7F84 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN1, 0)) {
            CB->divers(DV_HP, P_ARKHAN1, 0);
        }
        /* 0x7FB0 */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN2, 0)) {
            CB->divers(DV_HP, P_ARKHAN2, 0);
        }
        /* 0x7FDC */
        if (CB->divers(DV_PERSONA_PRESENT, P_ARKHAN3, 0)) {
            CB->divers(DV_HP, P_ARKHAN3, 0);
        }
        /* 0x8000 */
        if (ST(V_GUARD_FIGHT_DONE) == 0) {
            CB->cinematique("SC22_500");
        }
        /* 0x801D */
        if (ST(V_GUARD_FIGHT_DONE) != 0) {
            CB->cinematique("SC23_500");
        }
        CB->divers_empile(DV_END_GAME, 0, 0);
    }
    /* 0x8049 */
    if (ST(ST_HERO) == P_ARKHAN1 && ST(V_ARKHAN1_PROGRESS) == 0 && ST(V_ZED_BRIEFING) == 3 && ST(ST_SCRIPT_BUSY) == 0 && ST(L07_ZED_BRIEFING) == 0) {
        /* 0x809D */
        if (ST(V_ENGAGED_PERSONA) == P_ZED27) {
            /* 0x80AC */
            if (ST(ST_COMBAT_STANCE) == 0 && ST(L07_ZED27_TRAINING) != 4) {
                ST(V_EVENT_1) = 0;
                ST(V_ENGAGED_PERSONA) = P_DELIA;
                CB->select_pile(1, 1);
                CB->vire_col(0x20, 0);
                CB->vire_col(0x21, 0);
                CB->combat(P_ZED27, -1);
                CB->evenement(0, -1);
                CB->evenement(0x26, 100);
                CB->select_pile(0, 1);
            }
            /* 0x814C */
            if (ST(ST_COMBAT_STANCE) == 1 && ST(L07_ZED27_TRAINING) == 4) {
                ST(L07_ZED27_TRAINING) = 5;
            }
        }
        /* 0x817F */
        if (ST(V_EVENT_38) == 1) {
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            CB->select_pile(1, 1);
            ST(V_EVENT_38) = 0;
            CB->animation("SP27GAR27");
            CB->animation("P27ARR01");
            CB->joindre_perm(P_ZED27, ST(ST_HERO));
            /* 0x8202 */
            if ((ST(L07_ZED_REF_A) + 5) >= ST(L07_ZED_REF_B) && (ST(L07_ZED_REF_A) - 5) <= ST(L07_ZED_REF_B)) {
                CB->dialogue("L07D01A", P_ZED27);
            }
            /* 0x8252 */
            if ((ST(L07_ZED_REF_A) + 5) < ST(L07_ZED_REF_B)) {
                CB->dialogue("L07D01B", P_ZED27);
            }
            /* 0x8282 */
            if ((ST(L07_ZED_REF_A) - 5) > ST(L07_ZED_REF_B)) {
                CB->dialogue("L07D01C", P_ZED27);
            }
            CB->stop_joindre(P_ZED27);
            CB->bloque(BLOQUE_NONE);
            CB->animation("SP27ARR01_250");
            CB->joindre(-P_ZED27, 400, -978, -1, -1);
            CB->animation("SP27ARR01_250");
            CB->animation("P27GAR26");
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 1);
            CB->animation("P27GAR02");
            CB->select_pile(0, 1);
        }
        /* 0x831A */
        if (ST(V_ENGAGED_PERSONA) == P_ARKHAN1 && ST(ST_COMBAT_STANCE) == 0) {
            ST(L07_ZONE_ENTER_LOCK) = 0;
            /* 0x834B */
            if (!CB->on_joindre(P_ZED27)) {
                /* 0x836B */
                if (!CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
                    ST(V_ENGAGED_PERSONA) = 0;
                    CB->evenement(1, 500);
                } else {
                    ST(V_ENGAGED_PERSONA) = 0x63;
                    CB->joindre_perm(-P_ZED27, ST(ST_HERO));
                    CB->evenement(0, -1);
                    CB->evenement(0, 10000);
                }
            }
        }
        /* 0x83D7 */
        if (ST(V_ENGAGED_PERSONA) == P_ARKHAN1 && ST(ST_COMBAT_STANCE) == 1) {
            ST(L07_ZONE_ENTER_LOCK) = 0;
            /* 0x8415 */
            if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
                ST(V_ENGAGED_PERSONA) = 0;
            }
            /* 0x8441 */
            if (!CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
                ST(V_ENGAGED_PERSONA) = 0;
                CB->evenement(1, 500);
            }
        }
        /* 0x846C */
        if (ST(V_ENGAGED_PERSONA) == 0x63) {
            /* 0x847E */
            if (ST(V_EVENT_0) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 40) && ST(ST_COMBAT_STANCE) == 0) {
                ST(V_EVENT_0) = 0;
                CB->select_pile(1, 1);
                CB->bloque(BLOQUE_ALL);
                CB->joindre(P_ZED27, 0, 0, 0, ST(ST_HERO));
                CB->animation("SP27ARR01");
                CB->dialogue("L07D05A", P_ZED27);
                CB->dialogue("L07D05B", P_ZED27);
                CB->evenement(0, -1);
                CB->evenement(0, 10000);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
            }
            /* 0x8560 */
            if (ST(V_EVENT_0) == 1 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 40)) {
                ST(V_EVENT_0) = 0;
                ST(V_ENGAGED_PERSONA) = P_ARKHAN1;
                CB->select_pile(1, 1);
                CB->stop_joindre(-P_ZED27);
                CB->select_pile(0, 1);
            }
        }
    }
    /* 0x85D5 */
    if (ST(V_GUARD_FIGHT_DONE) == 0 && ST(L07_PHEDORIA_FIGHT) == 0 && ST(ST_HERO) == P_ARKHAN1 && ST(V_ENGAGED_PERSONA) == P_PHEDORIA && ST(ST_COMBAT_STANCE) == 0 && ST(V_EVENT15_STEP) != 0 && CB->divers(DV_IN_ZONE, P_PHEDORIA, 22) && ST(L07_PHEDORIA_D2_DONE) == 0) {
        ST(V_EVENT15_STEP) = 0;
        ST(V_EVENT_2) = 0;
        ST(V_ENGAGED_PERSONA) = P_DELIA;
        CB->select_pile(2, 1);
        CB->bloque(BLOQUE_ALL);
        CB->combat(P_PHEDORIA, -1);
        CB->animation("SP26GAR27");
        CB->animation("P26ARR01");
        CB->dialogue("L07D02A", P_PHEDORIA);
        CB->dialogue("L07D02B", P_PHEDORIA);
        CB->bloque(BLOQUE_NONE);
        CB->joindre(-P_PHEDORIA, 244, -1724, 0xBE0F, -1);
        CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
        CB->evenement(2, 500);
        CB->select_pile(0, 1);
    }
    /* 0x874A */
    if ((CB->divers(DV_PLAYER_ZONE, 0, 0) == 0x22 && CB->direction_zone(26) && ST(L07_ZONE_ENTER_LOCK) == 0) || (CB->divers(DV_PLAYER_ZONE, 0, 0) == 0x21 && CB->direction_spec(0x8000) && ST(L07_ZONE_ENTER_LOCK) == 0 && ST(L07_ZED_BRIEFING) == 0)) {
        ST(L07_ZONE_ENTER_LOCK) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x87F0 */
        if (CB->divers(DV_PLAYER_ZONE, 0, 0) == 0x22) {
            CB->vire_col(0x21, 0);
        }
        /* 0x8815 */
        if (CB->divers(DV_PLAYER_ZONE, 0, 0) == 0x21) {
            CB->vire_col(0x20, 0);
        }
        CB->select_pile(1, 1);
        CB->combat(P_ZED27, -1);
        CB->animation("SP27GAR27");
        CB->animation("P27ARR01");
        ST(V_DLG_RANDOM) = CB->random(3);
        /* 0x8881 */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->dialogue("L07D01A", P_ZED27);
        }
        /* 0x88A3 */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->dialogue("L07D01B", P_ZED27);
        }
        /* 0x88C5 */
        if (ST(V_DLG_RANDOM) == 2) {
            CB->dialogue("L07D01C", P_ZED27);
        }
        CB->bloque(BLOQUE_NONE);
        CB->joindre(-P_ZED27, 300, -1107, 0xC000, -1);
        CB->animation("SP27ARR01_250");
        CB->modify_var(&ST(V_ENGAGED_PERSONA), 1);
        CB->select_pile(0, 1);
    }
    /* 0x8937 */
    if (ST(ST_VIE(P_ZED27)) != VIE_DEAD && ST(ST_HERO) == P_ARKHAN1 && ST(V_ARKHAN1_PROGRESS) == 0 && ST(V_ZED_BRIEFING) == 3 && ST(L07_ZED_BRIEFING) == 0) {
        /* 0x8991 */
        if (ST(V_EVENT_1) == 1 && CB->divers(DV_IN_ZONE, P_ZED27, 39) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && (!CB->divers(DV_IN_ZONE, ST(ST_HERO), 39) || CB->divers(DV_ANGLE_TO, P_ZED27, ST(ST_HERO)) > 0x4000) && ST(V_ENGAGED_PERSONA) == 0) {
            ST(V_EVENT_1) = 0;
            CB->select_pile(1, 1);
            switch (CB->random(3)) {
            case 0:
                CB->animation("SP27ATA09");
                CB->modify_var(&ST(L07_ZED27_TURNED), 0);
                CB->animation("P27GAR02");
                CB->evenement(1, 100);
                break;
            case 1:
                CB->animation("SP27ATA10");
                CB->modify_var(&ST(L07_ZED27_TURNED), 0);
                CB->animation("P27GAR02");
                CB->evenement(1, 100);
                break;
            case 2:
                CB->animation("SP27ATA08");
                CB->modify_var(&ST(L07_ZED27_TURNED), 0);
                CB->animation("P27GAR02");
                CB->evenement(1, 100);
                break;
            }
            CB->select_pile(0, 1);
        }
        /* 0x8B4E */
        if (ST(L07_ZED27_ZONE_LOCK) == 0 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 39) && ST(V_ENGAGED_PERSONA) == 0 && ST(L07_ZED27_TRAINING) < 4) {
            ST(L07_ZED27_ZONE_LOCK) = 1;
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            ST(V_EVENT_1) = 0;
            CB->select_pile(1, 1);
            CB->evenement(1, -1);
            CB->evenement(0, -1);
            CB->joindre(-P_ZED27, -1, -1, -1, ST(ST_HERO));
            CB->select_pile(0, 1);
        }
        /* 0x8C97 */
        if (ST(L07_ZED27_TURNED) == 0 && ST(L07_ZED27_ZONE_LOCK) != 1 && !CB->divers(DV_IN_ZONE, P_ZED27, 39) && ST(V_ENGAGED_PERSONA) == 0) {
            ST(L07_ZED27_TURNED) = 1;
            CB->select_pile(1, 2);
            CB->evenement(1, -1);
            CB->joindre(-P_ZED27, 0, 0, (CB->divers(DV_DESC_POS, P_ZED27, 2) + 0x8000) & 0xFFFF, -1);
            CB->animation("SP27MAR01");
            CB->modify_var(&ST(V_EVENT_1), 1);
            CB->animation("P27ARR01");
            CB->select_pile(0, 1);
        }
        /* 0x8D75 */
        if (ST(L07_ZED27_ZONE2_SEEN) == 0 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_COMBAT_STANCE) == 0) {
            CB->select_pile(1, 2);
            ST(L07_ZED27_ZONE2_SEEN) = 1;
            /* 0x8DE1 */
            if (ST(L07_ZED27_ZONE_LOCK) == 0 && ST(L07_ZED27_TRAINING) < 4) {
                ST(L07_ZED27_ZONE_LOCK) = 1;
                CB->bloque(BLOQUE_ALL);
                CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0), CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
                ST(V_EVENT_1) = 0;
                CB->evenement(1, -1);
                CB->evenement(0, -1);
            }
            CB->stop_joindre(-P_ZED27);
            CB->pos_perso(P_ZED27, CB->divers(DV_DESC_POS, P_ZED27, 0), CB->divers(DV_DESC_POS, P_ZED27, 1), CB->divers(DV_DESC_POS, P_ZED27, 2), 1, "P27GAR02", 2, 601);
            /* 0x8F12 */
            if (ST(L07_ZED27_TRAINING) == 0) {
                CB->animation("SP27GAR27");
                CB->animation("P27ARR01");
                CB->joindre(P_ZED27, 0, 0, 0, ST(ST_HERO));
                CB->animation("P27ARR01");
                CB->joindre_perm(P_ZED27, ST(ST_HERO));
                lieu_action_register("L07D03A", P_ZED27, "SP27DIA50", "SP27ARR01");
                lieu_action_register("L07D03B", P_ZED27, "SP27DIA51", "SP27ARR01");
                ST(L07_ZED27_TRAINING) = 1;
                ST(V_ENGAGED_PERSONA) = 0x63;
                CB->evenement(0, -1);
                CB->evenement(0, 15000);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x8FF9 */
            if (ST(L07_ZED27_TRAINING) == 1) {
                CB->animation("SP27GAR27");
                CB->animation("P27ARR01");
                CB->joindre(P_ZED27, 0, 0, 0, ST(ST_HERO));
                CB->animation("P27ARR01");
                CB->joindre_perm(P_ZED27, ST(ST_HERO));
                CB->dialogue("L07D04", P_ZED27);
                ST(L07_ZED27_TRAINING) = 2;
                ST(V_ENGAGED_PERSONA) = 0x63;
                CB->evenement(0, -1);
                CB->evenement(0, 17500);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x90BE */
            if (ST(L07_ZED27_TRAINING) == 2) {
                CB->animation("SP27GAR27");
                CB->animation("P27ARR01");
                CB->joindre(P_ZED27, 0, 0, 0, ST(ST_HERO));
                CB->animation("P27ARR01");
                CB->joindre_perm(P_ZED27, ST(ST_HERO));
                CB->dialogue("L07D05A", P_ZED27);
                CB->dialogue("L07D05B", P_ZED27);
                ST(L07_ZED27_TRAINING) = 3;
                ST(V_ENGAGED_PERSONA) = 0x63;
                CB->evenement(0, -1);
                CB->evenement(0, 20000);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9193 */
            if (ST(L07_ZED27_TRAINING) == 3) {
                CB->bloque(BLOQUE_NONE);
                CB->dialogue("L07D06", P_ZED27);
                ST(L07_ZED27_TRAINING) = 4;
                ST(L07_ZONE_ENTER_LOCK) = 0;
                CB->vire_col(0x20, 1);
                CB->vire_col(0x21, 1);
                CB->combat(P_ZED27, 2);
                ST(V_ENGAGED_PERSONA) = P_ZED27;
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9229 */
            if (ST(L07_ZED27_TRAINING) == 5) {
                CB->bloque(BLOQUE_NONE);
                CB->animation("SP27ARR01");
                CB->joindre(-P_ZED27, 0, 0, 0, ST(ST_HERO));
                ST(V_ENGAGED_PERSONA) = 0x63;
                CB->evenement(0, -1);
                CB->evenement(0, 5000);
                CB->select_pile(0, 1);
                return;
            }
        }
        /* 0x92A0 */
        if (ST(L07_ZED27_ZONE2_SEEN) == 1 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 40) && ST(V_ENGAGED_PERSONA) == 0x63 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2) && ST(L07_ZED27_TRAINING) < 4) {
            CB->select_pile(1, 1);
            CB->bloque(BLOQUE_ALL);
            CB->evenement(0, -1);
            CB->stop_joindre(-P_ZED27);
            /* 0x9346 */
            if (ST(L07_ZED27_TRAINING) == 1) {
                CB->dialogue("L07D08", P_ZED27);
            }
            /* 0x9368 */
            if (ST(L07_ZED27_TRAINING) == 2) {
                CB->dialogue("L07D09", P_ZED27);
            }
            /* 0x938A */
            if (ST(L07_ZED27_TRAINING) == 3) {
                CB->dialogue("L07D10", P_ZED27);
            }
            CB->animation("SP27GAR26");
            CB->animation("P27GAR02");
            CB->evenement(1, 2000);
            CB->bloque(BLOQUE_NONE);
            ST(L07_ZED27_ZONE2_SEEN) = 0;
            ST(V_ENGAGED_PERSONA) = 0;
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9401 */
        if (ST(L07_ZED27_ZONE2_SEEN) == 1 && ST(L07_ZED27_TRAINING) == 5 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 40) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
            ST(L07_ZED27_ZONE2_SEEN) = 0;
        }
        /* 0x9471 */
        if (ST(L07_ZED27_ZONE_LOCK) == 1 && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 40) && !CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
            ST(L07_ZED27_ZONE_LOCK) = 0;
        }
    }
    /* 0x94D2 */
    if (ST(V_GUARD_FIGHT_DONE) == 0 && ST(L07_PHEDORIA_FIGHT) == 0 && ST(ST_HERO) == P_ARKHAN1) {
        /* 0x9510 */
        if (!CB->divers(DV_IN_ZONE, P_PHEDORIA, 22) && ST(L07_PHEDORIA_D2_DONE) == 0) {
            ST(L07_PHEDORIA_D2_DONE) = 5;
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            CB->divers_empile(DV_COMBAT_STANCE, 0, 0);
            CB->animation("Sar");
            CB->joindre(-ST(ST_HERO), 383, -1586, 0x8296, -1);
            CB->animation("Sar");
            CB->divers_empile(DV_COMBAT_STANCE, 1, 0);
            CB->select_pile(2, 2);
            CB->combat(P_PHEDORIA, -1);
            CB->pos_perso(P_PHEDORIA, 611, -1738, 0x3D0A, 1, "P26GAR03", 3, 1);
            CB->animation("SP26GAR27");
            CB->animation("P26ARR03_50");
            CB->animation("P26ARR03_50");
            CB->joindre(P_PHEDORIA, 190, -1735, -1, -1);
            CB->modify_var(&ST(L07_PHEDORIA_D2_DONE), 1);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x9660 */
        if (ST(L07_PHEDORIA_D2_DONE) == 1 && !CB->on_joindre(P_PHEDORIA) && CB->divers(DV_IN_ZONE, P_PHEDORIA, 22)) {
            CB->select_pile(2, 2);
            ST(L07_PHEDORIA_D2_DONE) = 0;
            CB->combat(P_PHEDORIA, 2);
            CB->bloque(BLOQUE_NONE);
            CB->select_pile(0, 1);
            return;
        }
        /* 0x96F6 */
        if (!CB->divers(DV_IN_ZONE, ST(ST_HERO), 22)) {
            /* 0x9708 */
            if (ST(L07_FLAG_750) == 1) {
                ST(L07_FLAG_750) = 0;
            }
            /* 0x9726 */
            if (ST(V_ENGAGED_PERSONA) == P_PHEDORIA) {
                CB->bloque(BLOQUE_ALL);
                ST(V_ENGAGED_PERSONA) = 0;
                CB->combat(P_PHEDORIA, -1);
                lieu_action_register("L07D32", P_PHEDORIA, "SP26GAR03", "SP26GAR03_50");
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x9785 */
            if (ST(V_EVENT_2) == 1) {
                CB->select_pile(2, 1);
                ST(V_EVENT_2) = 0;
                switch (CB->random(3)) {
                case 0:
                    CB->animation("SP26ATA18");
                    CB->evenement(2, 1300);
                    CB->animation("P26GAR03");
                    break;
                case 1:
                    CB->animation("SP26ATA17");
                    CB->evenement(2, 1700);
                    CB->animation("P26GAR03");
                    break;
                case 2:
                    CB->animation("SP26ATA16");
                    CB->evenement(2, 1300);
                    CB->animation("P26GAR03");
                    break;
                }
                CB->select_pile(0, 1);
            }
            /* 0x987F */
            if (ST(L07_PHEDORIA_BLOCK) == 0 && CB->distance(3, 13) >= 200) {
                CB->select_pile(2, 2);
                ST(L07_PHEDORIA_BLOCK) = 1;
                CB->evenement(2, -1);
                CB->animation("SP26GAR27");
                CB->animation("P26ARR03_50");
                CB->animation("P26ARR03_50");
                CB->joindre(P_PHEDORIA, 190, -1735, 0xBE0F, -1);
                CB->animation("SP26GAR26");
                CB->animation("P26GAR03");
                CB->evenement(2, 500);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
            }
            /* 0x9953 */
            if (ST(L07_PHEDORIA_BLOCK) == 1 && CB->distance(3, 22) <= 25 && !CB->on_joindre(P_PHEDORIA)) {
                ST(L07_ZED_TIMER_PHASE) = 0;
                ST(L07_PHEDORIA_BLOCK) = 0;
            }
        }
        /* 0x99AC */
        if (ST(L07_PHEDORIA_BLOCK) == 1 && CB->divers(DV_IN_ZONE, ST(ST_HERO), 15) && ST(L07_ZED_TIMER_PHASE) == 0) {
            CB->bloque(BLOQUE_ALL);
            CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0) - 20, CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            ST(L07_ZED_TIMER_PHASE) = 1;
        }
        /* 0x9A6E */
        if (ST(L07_FLAG_750) == 0 && (ST(L07_PHEDORIA_BLOCK) == 0 || ST(L07_ZED_TIMER_PHASE) == 1) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 15) && ST(V_ENGAGED_PERSONA) == 0) {
            CB->bloque(BLOQUE_ALL);
            CB->pos_perso(ST(ST_HERO), CB->divers(DV_DESC_POS, ST(ST_HERO), 0) - 15, CB->divers(DV_DESC_POS, ST(ST_HERO), 1), CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 1, 0);
            ST(L07_FLAG_750) = 1;
            CB->select_pile(2, 1);
            /* 0x9B64 */
            if (ST(L07_PHEDORIA_TALK_STAGE) == 0) {
                ST(L07_PHEDORIA_TALK_STAGE) = 1;
                CB->animation("SP26GAR27");
                CB->animation("P26ARR03");
                CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
                CB->dialogue("L07D11", P_PHEDORIA);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9BE2 */
            if (ST(L07_PHEDORIA_TALK_STAGE) == 1) {
                ST(L07_PHEDORIA_TALK_STAGE) = 2;
                CB->animation("SP26GAR27");
                CB->animation("P26ARR03");
                CB->joindre(-P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
                CB->dialogue("L07D12", P_PHEDORIA);
                CB->bloque(BLOQUE_NONE);
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9C60 */
            if (ST(L07_PHEDORIA_TALK_STAGE) == 2) {
                ST(L07_PHEDORIA_TALK_STAGE) = 3;
                CB->dialogue("L07D13", P_PHEDORIA);
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_PHEDORIA, 2);
                ST(V_ENGAGED_PERSONA) = P_PHEDORIA;
                CB->select_pile(0, 1);
                return;
            }
            /* 0x9CC7 */
            if (ST(L07_PHEDORIA_TALK_STAGE) == 3) {
                CB->dialogue("L07D14", P_PHEDORIA);
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_PHEDORIA, 2);
                ST(V_ENGAGED_PERSONA) = P_PHEDORIA;
                CB->select_pile(0, 1);
                return;
            }
        }
    }
    /* 0x9D1F */
    if (ST(V_GUARD_FIGHT_DONE) == 0 && ST(L07_PHEDORIA_FIGHT) == 0 && ST(ST_HERO) > 1) {
        /* 0x9D55 */
        if (ST(V_EVENT_0) == 1) {
            ST(V_EVENT_0) = 0;
            CB->select_pile(2, 1);
            /* 0x9D9B */
            if (CB->divers(DV_ANGLE_TO, P_PHEDORIA, ST(ST_HERO)) > 0x1770 || ST(ST_LOCKED_CAMERA) != 6) {
                CB->animation("SG26FRA01");
                CB->animation("P26GAR03");
                CB->evenement(0, 3000);
            } else {
                /* 0x9DE4 */
                if (ST(L07_PHEDORIA_SCENE_STEP) == 0) {
                    ST(L07_PHEDORIA_SCENE_STEP) = 1;
                    CB->dialogue("L07D74", P_PHEDORIA);
                    CB->evenement(0, 8000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9E2E */
                if (ST(L07_PHEDORIA_SCENE_STEP) == 1) {
                    ST(L07_PHEDORIA_SCENE_STEP) = 2;
                    /* 0x9E4C */
                    if (ST(ST_VIE(70)) != VIE_DEAD) {
                        CB->dialogue("L07D75", P_PHEDORIA);
                    } else {
                        CB->dialogue("L07D74", P_PHEDORIA);
                    }
                    CB->evenement(0, 7000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9E9F */
                if (ST(L07_PHEDORIA_SCENE_STEP) == 2) {
                    ST(L07_PHEDORIA_SCENE_STEP) = 3;
                    CB->dialogue("L07D76", P_PHEDORIA);
                    CB->evenement(0, 6000);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0x9EE9 */
                if (ST(L07_PHEDORIA_SCENE_STEP) == 3) {
                    ST(L07_PHEDORIA_TALK_STEP) = 2;
                    CB->bloque(BLOQUE_ALL);
                    CB->joindre(P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
                    CB->dialogue("L07D77", P_PHEDORIA);
                    CB->animation("SG26FRA01");
                    CB->evenement(0x21, 500);
                    CB->select_pile(0, 1);
                    return;
                }
            }
            CB->select_pile(0, 1);
        }
        /* 0x9F79 */
        if ((ST(L07_PHEDORIA_TALK_B) == 2 || ST(L07_PHEDORIA_TALK_A) == 2) && ST(L07_PHEDORIA_APPROACH) == 0 && !CB->on_joindre(P_PHEDORIA) && CB->distance(1, 3) > 100) {
            ST(L07_PHEDORIA_TALK_A) = 3;
            ST(L07_PHEDORIA_TALK_B) = 3;
            ST(L07_PHEDORIA_APPROACH) = 1;
            CB->evenement(0, 5000);
            CB->animation("Sar_50");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        }
        /* 0xA036 */
        if (ST(L07_PHEDORIA_SEEN) == 0 && ST(L07_PHEDORIA_ARRIVED) == 0 && (ST(ST_LOCKED_CAMERA) == 6 || ST(ST_LOCKED_CAMERA) == 2)) {
            ST(L07_PHEDORIA_SEEN) = 1;
            CB->select_pile(2, 2);
            CB->joindre(P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP26GAR26");
            CB->animation("P26GAR03");
            CB->divers(DV_LOOK_AT_PLAYER, P_PHEDORIA, 1);
            CB->bloque(BLOQUE_ALL);
            CB->joindre_perm(P_PHEDORIA, ST(ST_HERO));
            CB->dialogue("L07D54", P_PHEDORIA);
            CB->bloque(BLOQUE_NONE);
            CB->evenement(0x21, 2500);
            CB->select_pile(0, 1);
        }
        /* 0xA135 */
        if (ST(L07_PHEDORIA_ARRIVED) == 1 && ST(ST_LOCKED_CAMERA) == 3 && CB->distance(1, 3) > 300 && ST(L07_SCENE_100) == 0) {
            ST(L07_SCENE_100) = 1;
            CB->change_camera(99, CAM_AUTO);
        }
        /* 0xA1A5 */
        if (ST(V_EVENT_33) == 1) {
            ST(V_EVENT_33) = 0;
            /* 0xA1C6 */
            if (ST(ST_LOCKED_CAMERA) != 0 && ST(ST_LOCKED_CAMERA) != 1) {
                /* 0xA1E7 */
                if (ST(L07_PHEDORIA_TALK_STEP) == 0) {
                    CB->select_pile(2, 1);
                    ST(L07_PHEDORIA_TALK_STEP) = 1;
                    CB->bloque(BLOQUE_ALL);
                    /* 0xA220 */
                    if (ST(L07_L2_SPAWN) == 3) {
                        CB->change_camera(3, CAM_LOCKED);
                    } else {
                        CB->change_camera(6, CAM_LOCKED);
                    }
                    CB->stop_joindre(P_PHEDORIA);
                    lieu_action_register("L07D55", P_PHEDORIA, "SP26GAR03", "SP26GAR03");
                    CB->change_camera(99, CAM_AUTO);
                    CB->evenement(0x21, 4000);
                    CB->joindre_perm(P_PHEDORIA, ST(ST_HERO));
                    CB->bloque(BLOQUE_NONE);
                    CB->select_pile(0, 1);
                    return;
                }
                /* 0xA2C8 */
                if (ST(L07_PHEDORIA_TALK_STEP) == 1) {
                    CB->select_pile(2, 2);
                    ST(L07_PHEDORIA_TALK_STEP) = 2;
                    CB->bloque(BLOQUE_ALL);
                    CB->joindre_perm(-P_PHEDORIA, ST(ST_HERO));
                    CB->stop_joindre(P_PHEDORIA);
                    CB->animation("SP26GAR03");
                    CB->dialogue("L01D45B", P_PHEDORIA);
                    CB->animation("SG26FRA01");
                    CB->evenement(0x21, 750);
                    CB->animation("P26GAR03");
                    CB->select_pile(0, 1);
                    CB->joindre(ST(ST_HERO), 0, 0, 0, 0x1A);
                    return;
                }
                /* 0xA38B */
                if (ST(L07_PHEDORIA_TALK_STEP) == 2) {
                    CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
                    CB->animation(lieu_anim_name(P_ARKHAN2, 0x1A));
                    CB->modify_var(&ST(V_DEATH_EVENT_SENT), 1);
                    return;
                }
            } else {
                /* 0xA3ED */
                if (ST(L07_PHEDORIA_TALK_STEP) == 0) {
                    CB->evenement(0x21, 4000);
                }
                /* 0xA40C */
                if (ST(L07_PHEDORIA_TALK_STEP) == 1) {
                    CB->evenement(0x21, 4000);
                }
            }
        }
        /* 0xA437 */
        if (ST(ST_COMBAT_STANCE) == 1 && ST(L07_D79_SAID) == 0 && ST(ST_LOCKED_CAMERA) > 1 && (CB->distance(1, 3) < 750 || (ST(V_PLAYER_STRIKE) == 1 && CB->divers(DV_CAN_FIGHT, 9, 2)))) {
            ST(L07_D79_SAID) = 1;
            CB->select_pile(2, 2);
            CB->evenement(0, -1);
            CB->evenement(0x21, -1);
            ST(V_EVENT_0) = 0;
            ST(V_EVENT_33) = 0;
            CB->stop_joindre(-P_PHEDORIA);
            CB->joindre(P_PHEDORIA, 0, 0, 0, ST(ST_HERO));
            CB->bloque(BLOQUE_ALL);
            CB->animation("Sar");
            lieu_action_register("L07D78", P_PHEDORIA, "SP26GAR26", "SP26GAR03");
            CB->animation("Sar");
            /* 0xA58E */
            if (CB->distance(1, 3) > 200) {
                CB->evenement(0x22, 500);
                CB->animation("SG26FRA01");
            } else {
                CB->evenement(0x23, 450);
                CB->animation("SP26ATA18");
            }
            CB->animation("P26GAR03_50");
            CB->select_pile(0, 1);
            return;
        }
        /* 0xA5F8 */
        if (ST(V_EVENT_34) == 1) {
            ST(V_EVENT_34) = 0;
            CB->select_pile(0, 2);
            CB->divers_empile(DV_BLOOD_AT_BONE, ST(ST_HERO), 0);
            CB->animation(lieu_anim_name(P_ARKHAN2, 0x1A));
            CB->modify_var(&ST(V_DEATH_EVENT_SENT), 1);
            return;
        }
        /* 0xA674 */
        if (ST(V_EVENT_35) == 1) {
            ST(V_EVENT_35) = 0;
            CB->animation(lieu_anim_name(P_ARKHAN2, 0x18));
            /* 0xA6AF */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->animation("P02_65535");
            }
            /* 0xA6CC */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->animation("P03_65535");
            }
            CB->animation(lieu_anim_name(P_ARKHAN2, 6));
            CB->animation("ar_50");
            CB->animation("SG26FRA01");
            CB->evenement(0x22, 600);
            CB->animation("P26GAR03_50");
            return;
        }
    }
}

/* -------------------------------------------------------------------------
 * INIT (opcode 1) — arming tail of FonctionEditor case 1 (0x1000E2A?).
 * Callback ingestion, the trigger-table fill are skipped: g_lieu_cb is
 * provided by the bridge, the trigger table is static in lieu07_trig.c.
 *   arg = reload flag (non-zero => skip arming).
 * Returns 1 where the original returns 0 directly (the death ending, 0x1000E4E4);
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
        l07_trig00();   /* 0x1000E4E9: call sub_10001073 -> sub_100073F0 */
        return 0;
    }
    /* 0x1000E3CE — the hero is dead: play the death ending */
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
 * TICK (opcode -1, 0x1000E53C): drain pending sound, run body.
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
 * Entry point — @FonctionEditor@16 (thunk 0x10001046 -> 0x1000DF20).
 * Jump table @ 0x1000E5B9, index = op+1 (0..7); op 0 / op 2 / out of range
 * fall through to `return state[0]`.
 * ---------------------------------------------------------------------- */
i32 lieu07_fonction_editor(i32 op, lieu_state_t *state, i32 arg, i32 arg2) {
    g_lvl_S = state;
    if (g_lieu_trace)
        print("lieu07: op=%d arg=%d arg2=%d\n", op, arg, arg2);
    switch (op) {
    case LIEU_OP_TICK:   tick();                       break;  /* op -1 */
    case LIEU_OP_INIT:
        if (init(arg))
            return 0;
        break;
    case LIEU_OP_EVENT:  l07_trigger(arg);             break;  /* dword_1001E1C0[arg]() */
    case LIEU_OP_CALL4:  lieu_event4(arg);                  break;
    case LIEU_OP_CALL5:  lieu_event5(arg, arg2);            break;
    case LIEU_OP_CALL6:  lieu_event6(arg);                  break;
    case LIEU_OP_TEARDOWN: /* engine-side cleanup; handler no-op */
    default:                                           break;
    }
    return ST(LIEU_ST_RESULT);
}
