/*
 * lieu15_trig.c — Native hand-port of the LIEU15.DLL zone trigger
 * handlers (opcode 3). The entry point, INIT, tick and events live in
 * lieu15.c; this file holds the 42-slot trigger table and its handlers.
 *
 * Ported from disassembly (ImageBase 0x10000000). Each function carries
 * its original address. Handlers call the engine only through g_lieu_cb.
 */

#include <stdio.h>
#include <stdlib.h>

#include "libsys/common.h"
#include "lieu_cb.h"
#include "levels/lieu15_int.h"

/* -------------------------------------------------------------------------
 * Forward decls for the trigger table.
 * ---------------------------------------------------------------------- */
static void trig01(void); static void trig02(void); static void trig03(void);
static void trig04(void); static void trig05(void); static void trig07(void);
static void trig08(void); static void trig09(void); static void trig10(void);
static void trig11(void); static void trig12(void); static void trig13(void);
static void trig14(void); static void trig41(void);
static void trig_noop(void);   /* the 27 stub slots */

/* Trigger table, filled at INIT with the 5-byte jmp thunks at
 * 0x10001000+5*i; static here.
 *   idx handler        idx handler        idx handler
 *    0  10007543       14  100160B3       28  10016212 stub
 *    1  1000D293       15  10016183 stub  29  1001621D stub
 *    2  1000D35C       16  1001618E stub  30  10016228 stub
 *    3  10012FA5       17  10016199 stub  31  10016233 stub
 *    4  1001393A       18  100161A4 stub  32  1001623E stub
 *    5  10014039       19  100161AF stub  33  10016249 stub
 *    6  100146A7 stub  20  100161BA stub  34  10016254 stub
 *    7  100146B2       21  100161C5 stub  35  1001625F stub
 *    8  10014833       22  100161D0 stub  36  1001626A stub
 *    9  1001497C       23  100161DB stub  37  10016275 stub
 *   10  10014B04       24  100161E6 stub  38  10016280 stub
 *   11  10014CFF       25  100161F1 stub  39  1001628B stub
 *   12  10014DA4       26  100161FC stub  40  10016296 stub
 *   13  10015FE3       27  10016207 stub  41  10008D73
 */
static void (*const g_trigger[42])(void) = {
    l15_trig00, trig01, trig02, trig03, trig04, trig05, trig_noop, trig07,
    trig08, trig09, trig10, trig11, trig12, trig13, trig14, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop, trig_noop,
    trig_noop, trig_noop, trig_noop, trig_noop, trig41,
};

/* Op 3: run trigger handler idx (call [4*arg + 0x1002EAC0]). */
void l15_trigger(i32 idx) {
    if (idx >= 0 && idx < 42)
        g_trigger[idx]();
}

/* Empty slots: the stub bodies are `push ebp; mov ebp,esp; ... ret`. */
static void trig_noop(void) {
}

/* trigger 00 — trig00_10007543 — level setup: start-point presets, state reset, zones, objects, actor placement */
void l15_trig00(void) {
    /* 0x7555 */
    if (ST(ST_START_POINT(15)) == 1) {
        ST(L15_PROGRESS_STEP) = 0;
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(ST_VIE(P_KONKALITE9)) = 0;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(L15_DANRYS_INTRO_STAGE) = 0;
        ST(L15_ROOM_ENTERED) = 0;
        ST(ST_LEVEL) = 14;
        ST(L15_ROOM_SEEN_E40) = 1;
    }
    /* 0x7609 */
    if (ST(ST_START_POINT(15)) == 2) {
        ST(L15_PROGRESS_STEP) = 1;
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x769F */
    if (ST(ST_START_POINT(15)) == 3) {
        ST(L15_PROGRESS_STEP) = 2;
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(L15_CRYSTAL_ROUND) = 0;
        ST(V_GUARD_FIGHT_DONE) = 0;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x7753 */
    if (ST(ST_START_POINT(15)) == 4) {
        ST(L15_PROGRESS_STEP) = 3;
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(L15_CRYSTAL_ROUND) = 1;
        ST(V_CRYSTAL_PROGRESS) = 0;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x7825 */
    if (ST(ST_START_POINT(15)) == 5) {
        ST(L15_PROGRESS_STEP) = 3;
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(V_CRYSTAL_PROGRESS) = 0;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x78E8 */
    if (ST(ST_START_POINT(15)) == 6) {
        ST(L15_PROGRESS_STEP) = 4;
        ST(ST_HERO) = P_ARKHAN3;
        ST(ST_VIE(P_DANRYS)) = VIE_DEAD;
        ST(L15_DANRYS_PLOT_A84) = 3;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(V_CRYSTAL_PROGRESS) = 0;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x79BA */
    if (ST(ST_START_POINT(15)) == 7) {
        ST(L15_PROGRESS_STEP) = 7;
        ST(ST_HERO) = P_ARKHAN3;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(ST_VIE(P_DANRYS)) = VIE_DEAD;
        ST(L15_DANRYS_DEAD_SEEN) = 0;
        ST(L15_DANRYS_PLOT_A84) = 3;
        ST(V_CRYSTAL_PROGRESS) = 0;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x7A8C */
    if (ST(ST_START_POINT(15)) == 8) {
        ST(L15_PROGRESS_STEP) = 7;
        ST(ST_HERO) = P_ARKHAN3;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(ST_VIE(P_KALHI13)) = 1;
        ST(L15_DANRYS_DEAD_SEEN) = 0;
        ST(L15_DANRYS_PLOT_A84) = 3;
        ST(V_CRYSTAL_PROGRESS) = 2;
        ST(L15_CRYSTAL18_GIVEN) = 1;
        ST(L15_CRYSTAL19_GIVEN) = 1;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
    }
    /* 0x7B7C */
    if (ST(ST_START_POINT(15)) == 9) {
        ST(L15_PROGRESS_STEP) = 9;
        ST(ST_HERO) = P_ARKHAN2;
        ST(L15_CRYSTAL18_GIVEN) = 1;
        ST(L15_CRYSTAL19_GIVEN) = 1;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 1;
        ST(V_CRYSTAL_PROGRESS) = 2;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_PLOT_A84) = 0;
        ST(ST_LEVEL) = 14;
        ST(ST_VIE(P_DANRYS)) = VIE_DEAD;
        ST(ST_VIE(P_KALHI13)) = 1;
    }
    /* 0x7C3F */
    if (ST(ST_START_POINT(15)) == 10) {
        ST(L15_PROGRESS_STEP) = 9;
        ST(ST_HERO) = P_ARKHAN2;
        ST(V_CRYSTAL_PROGRESS) = 2;
        ST(L15_CRYSTAL18_GIVEN) = 1;
        ST(L15_CRYSTAL19_GIVEN) = 1;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 1;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_PLOT_A84) = 0;
        ST(ST_LEVEL) = 14;
        ST(V_EVENT_31) = 1;
        ST(ST_VIE(P_DANRYS)) = VIE_DEAD;
    }
    /* 0x7D02 */
    if (ST(ST_START_POINT(15)) == 11) {
        ST(L15_PROGRESS_STEP) = 4;
        ST(ST_HERO) = P_ARKHAN2;
        ST(ST_VIE(P_DANRYS)) = 1;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(L15_DANRYS_STATE_BE8) = 2;
        ST(V_CRYSTAL_PROGRESS) = 0;
        ST(V_CRYSTAL_KEY_STAGE) = 1;
        ST(L15_FLAG_BE4) = 0;
        ST(V_GUARD_FIGHT_DONE) = 1;
        ST(L15_DANRYS_INTRO_STAGE) = 2;
        ST(L15_ROOM_ENTERED) = 1;
        ST(ST_LEVEL) = 14;
        ST(V_EVENT_31) = 1;
        CB->divers(DV_PERSONA_ITEMS, P_DANRYS, OB_HANDLE);
    }
    ST(L15_DANRYS_SCENE_STATE) = 0;
    ST(L15_UNUSED_68) = 0;
    ST(L15_DANRYS_WARNED) = 0;
    ST(L15_DANRYS_FIRST_TALK) = 0;
    ST(L15_DANRYS_WALK_STEP) = 0;
    ST(L15_TRADE_TYPE) = 0;
    ST(L15_KONKALITE9_STATE) = 0;
    ST(L15_DANRYS_KALHI_FOLLOW) = 0;
    ST(L15_UNUSED_84) = 0;
    ST(L15_DANRYS_PRE_TALK_DONE) = 0;
    ST(V_EVENT15_STEP) = 0;
    ST(L15_ARKHAN2_TOPIC_90) = 0;
    ST(L15_DANRYS_CALL_DONE) = 0;
    ST(L15_KALHI_DANRYS_DEAD_NOTE) = 0;
    ST(L15_TRADE_SCENE_STEP) = 0;
    ST(L15_TRADE_ANIM_CAM) = 0;
    ST(L15_DANRYS_ESCORT_STEP) = 0;
    ST(L15_ANIM_PHASE) = 0;
    ST(L15_ROOM_PART) = 0;
    ST(L15_CAMERA3_ENTERED) = 0;
    ST(L15_TRADE_REWARD_PENDING) = 0;
    ST(L15_LEVER_CAM_STEP) = 0;
    ST(L15_DANRYS_APPROACHED) = 0;
    ST(L15_TRADE_FINAL) = 0;
    ST(L15_KALHI_FOLLOW_STEP) = 0;
    ST(L15_DANRYS_INTRO_114) = 0;
    ST(V_WEAPON_WARNED) = 0;
    ST(L15_STANCE_LOWERED) = 0;
    ST(L15_DANRYS_GREET_PICK) = 0;
    ST(L15_GREETING_RANDOM_OK) = 0;
    ST(L15_DANRYS_TOPIC_58) = 0;
    ST(V_EVENT_0) = 0;
    ST(V_EVENT_1) = 0;
    ST(V_EVENT_2) = 0;
    ST(V_EVENT_29) = 0;
    ST(V_EVENT_34) = 0;
    ST(V_EVENT_33) = 0;
    CB->evenement(29, -1);
    ST(ST_SCRIPT_BUSY) = 0;
    ST(V_ENGAGED_PERSONA) = 0;
    ST(V_CRI_USED) = 0;
    ST(V_PRE90_OBJECT) = 0;
    ST(V_LAST_WEAPON) = 0;
    CB->ajoute_zone(1, 5250, 950, 250, 130, 1);
    CB->ajoute_zone(3, 5040, 215, 100, 100, 0);
    CB->ajoute_zone(4, 5030, 590, 115, 115, 0);
    CB->ajoute_zone(5, 5090, 110, 120, 100, 0);
    CB->ajoute_zone(7, 4922, 634, 85, 190, 0);
    CB->ajoute_zone(8, 5224, 80, 105, 100, 0);
    CB->ajoute_zone(9, 5320, 410, 80, 300, 0);
    CB->ajoute_zone(13, 5080, 500, 80, 100, 0);
    CB->ajoute_zone(14, 4920, 540, 85, 95, 0);
    CB->ajoute_zone(15, 5004, 115, 30, 30, 0);
    CB->ajoute_zone(20, 5272, 833, 205, 65, 0);
    /* 0x8171 */
    if (ST(L15_TRIG9_SEEN) == 0) {
        CB->etat_objet("OB74", 5361, -2208, 368, 0xBE00, 0x85FC, 0xFA00, -1, 0);
    }
    /* 0x81B0 */
    if (ST(V_CRYSTAL_PROGRESS) == 4) {
        ST(L15_TRADE_SCENE_STEP) = 1;
        ST(L15_TRADE_REWARD_PENDING) = 1;
    }
    /* 0x81E0 */
    if (ST(V_CRYSTAL_PROGRESS) == 3) {
        ST(V_CRYSTAL_PROGRESS) = 5;
    }
    /* 0x8201 */
    if (ST(V_EVENT_31) == 1 && ST(L15_DANRYS_DEAD_SEEN) == 0) {
        CB->divers(DV_PERSONA_ITEMS, P_DANRYS, -OB_HANDLE);
        ST(L15_CRYSTAL_ROUND) = 3;
    }
    /* 0x8246 */
    if (ST(L15_FLAG_BE4) == 1) {
        /* 0x8258 */
        if (ST(ST_VIE(P_DANRYS)) == 1) {
            ST(ST_VIE(P_DANRYS)) = VIE_DEAD;
            ST(V_EVENT_30) = 1;
            CB->evenement(30, 300000);
        }
        /* 0x8298 */
        if (ST(ST_VIE(P_KONKALITE9)) == 0) {
            /* 0x82AA */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(L15_KALHI_DEAD_HANDLED) == 1) {
                ST(ST_VIE(P_KALHI13)) = 4;
                ST(L15_PROGRESS_STEP) = 10;
            }
            ST(ST_VIE(P_KONKALITE9)) = 1;
        }
    }
    /* 0x82FB */
    if (ST(ST_VIE(P_DANRYS)) < 2 && ST(L15_DANRYS_INTRO_STAGE) < 2 && ST(V_EVENT_31) == 0) {
        ST(L15_ROOM_PART) = 0;
    }
    /* 0x8340 */
    if (ST(ST_VIE(P_DANRYS)) < 2 && ST(L15_DANRYS_INTRO_STAGE) >= 2 && ST(V_EVENT_31) == 0 && ST(L15_DANRYS_PLOT_A84) != 3) {
        ST(V_DLG_RANDOM) = CB->random(2);
        /* 0x839F */
        if (ST(V_DLG_RANDOM) == 0) {
            ST(L15_ROOM_PART) = 0;
        }
        /* 0x83C0 */
        if (ST(V_DLG_RANDOM) == 1) {
            ST(L15_ROOM_PART) = 1;
        }
    }
    /* 0x83E1 */
    if (ST(ST_VIE(P_DANRYS)) < 2 && ST(L15_DANRYS_PLOT_A84) >= 3) {
        /* 0x8405 */
        if (ST(L15_DANRYS_PLOT_A84) == 4) {
            ST(L15_DANRYS_PLOT_A84) = 3;
        }
        ST(L15_ROOM_PART) = 0;
    }
    CB->pos_perso(ST(ST_HERO), 5295, 788, CB->divers(DV_DESC_POS, ST(ST_HERO), 2), 1, "ar", 6, 0);
    /* 0x8474 */
    if (ST(ST_VIE(P_DANRYS)) < 2) {
        ST(ST_VIE(P_DANRYS)) = 1;
        /* 0x8495 */
        if (ST(L15_PROGRESS_STEP) == 0) {
            CB->pos_perso(P_DANRYS, 5183, 370, 0x4268, 1, "P09ARR03", 2, 0);
        }
        /* 0x84CC */
        if (ST(L15_DANRYS_PLOT_A84) == 3) {
            ST(L15_DANRYS_INTERRUPTIBLE) = 0;
            ST(L15_DANRYS_GREET_PICK) = 1;
            CB->pos_perso(P_DANRYS, 5183, 370, 0xF230, 1, "P09ARR01", 2, 0);
            /* 0x851E */
            if (ST(V_EVENT_31) == 1) {
                CB->link_objet(9, "OB27", 0x1B, 1);
            }
        } else {
            /* 0x8549 */
            if (ST(V_EVENT_31) == 0 && ST(L15_ROOM_PART) == 0) {
                ST(L15_DANRYS_INTERRUPTIBLE) = 0;
                ST(V_DLG_RANDOM) = CB->random(2);
                /* 0x8593 */
                if (ST(V_DLG_RANDOM) == 0) {
                    CB->pos_perso(P_DANRYS, 5183, 370, 0x4268, 1, "P09ARR02", 2, 0);
                }
                /* 0x85CA */
                if (ST(V_DLG_RANDOM) == 1) {
                    CB->pos_perso(P_DANRYS, 5183, 370, 0x4268, 1, "P09ATT01", 2, 0);
                    ST(L15_GREETING_RANDOM_OK) = 1;
                }
            }
            /* 0x860D */
            if (ST(V_EVENT_31) == 0 && ST(L15_ROOM_PART) == 1) {
                ST(L15_DANRYS_INTERRUPTIBLE) = 1;
                CB->pos_perso(P_DANRYS, 5053, 600, 0x7530, 1, "P09ATT06", 2, 0);
                CB->link_objet(9, "OB98", 0x1C, 1);
                ST(L15_SAVED_CAMERA) = 0;
            }
            /* 0x8688 */
            if (ST(V_EVENT_31) == 1) {
                CB->pos_perso(P_DANRYS, 5183, 370, 0xAFC8, 1, "P09ATT05", 2, 0);
                CB->animation("P09ARR01");
                CB->link_objet(9, "OB27", 0x1B, 1);
                ST(L15_DANRYS_GREET_PICK) = 1;
            }
        }
    }
    /* 0x86EA */
    if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
        CB->pos_perso(P_DANRYS, 5192, 370, 0xA, 2, "P09cad", 2, 0);
    }
    /* 0x871E */
    if (ST(ST_VIE(P_KALHI13)) == 1 && ST(L15_PROGRESS_STEP) != 2 && ST(L15_PROGRESS_STEP) != 5 && ST(L15_PROGRESS_STEP) != 8 && ST(L15_PROGRESS_STEP) != 20) {
        ST(L15_DANRYS_KALHI_FOLLOW) = 1;
        /* 0x8787 */
        if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD && ST(L15_DANRYS_DEAD_SEEN) == 0 && ST(L15_KALHI_DEAD_TALK_DONE) == 0) {
            ST(L15_DANRYS_INTERRUPTIBLE) = 0;
            CB->pos_perso(P_KALHI13, 5240, 500, 0x4650, 1, "P13ARR03", 0xC, 0);
            ST(L15_KALHI_DANRYS_DEAD_NOTE) = 1;
        } else if (ST(V_KALHI13_TALK_C) == 3 || (ST(V_KALHI13_TALK_C) != 3 && ST(L15_DANRYS_DEAD_SEEN) == 1 && ST(L15_KALHI_DEAD_TALK_DONE) == 0) || ST(L15_DANRYS_PLOT_A84) == 3) {
            ST(L15_DANRYS_INTERRUPTIBLE) = 0;
            /* 0x886E */
            if (ST(L15_DANRYS_PLOT_A84) == 3) {
                ST(L15_DANRYS_PLOT_A84) = 4;
            }
            /* 0x888F */
            if (ST(V_KALHI13_TALK_C) != 3 && ST(L15_DANRYS_DEAD_SEEN) == 1 && ST(L15_KALHI_DEAD_TALK_DONE) == 0) {
                ST(L15_KALHI_DANRYS_DEAD_NOTE) = 1;
                CB->pos_perso(P_KALHI13, 5188, 481, 0x858F, 1, "P13ARR01", 0xC, 0);
            } else {
                CB->pos_perso(P_KALHI13, 4924, 850, 0x8E94, 1, "P13ARR03", 0xC, 0);
            }
        } else if (ST(L15_PROGRESS_STEP) <= 1 || ST(L15_PROGRESS_STEP) == 4 || ST(L15_PROGRESS_STEP) == 7) {
            CB->pos_perso(P_KALHI13, 4920, 820, 0x9C40, 1, "P13ARR01", 0xC, 0);
        } else {
            ST(L15_KALHI_FOLLOW_STEP) = 1;
            CB->pos_perso(P_KALHI13, 4924, 850, 0x8E94, 1, "P13ARR01", 0xC, 0);
        }
    }
    /* 0x89BC */
    if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD && ST(L15_KALHI_DEAD_HANDLED) == 1) {
        CB->pos_perso(P_KALHI13, 0, 0, 0, 2, "P13cad", 0xC, 0);
        ST(L15_PROGRESS_STEP) = 20;
    }
    CB->load_anim("215MEU05", 3);
    CB->load_anim("215MEU04", 4);
    CB->load_anim("215MEU06", 5);
    CB->load_anim("215LEV01", 7);
    CB->load_anim("215LEV02", 8);
    /* 0x8A5B */
    if (ST(L15_DANRYS_INTERRUPTIBLE) == 0 && ST(L15_DANRYS_ENDING_FLAG) == 0) {
        ST(L15_ANIM_PHASE) = 0;
        ST(L15_LEVER_CAM_STEP) = 0;
        CB->etat_objet("OBCOMP", 5050, -2253, 110, 0, 0, 0x4000, 0, 4);
    }
    /* 0x8AC1 */
    if (ST(L15_DANRYS_INTERRUPTIBLE) == 0 && ST(L15_DANRYS_ENDING_FLAG) == 1) {
        ST(L15_ANIM_PHASE) = 0;
        ST(L15_LEVER_CAM_STEP) = 1;
        CB->etat_objet("OBCOMP", 5050, -2253, 110, 0xF600, 0, 0x4000, 0, 4);
    }
    /* 0x8B2A */
    if (ST(L15_DANRYS_INTERRUPTIBLE) == 1 && ST(L15_DANRYS_ENDING_FLAG) == 0) {
        /* 0x8B4E */
        if (ST(ST_VIE(P_DANRYS)) == 1) {
            ST(L15_ANIM_PHASE) = 1;
        }
        /* 0x8B6F */
        if (ST(ST_VIE(P_DANRYS)) >= 2) {
            ST(L15_ANIM_PHASE) = 2;
        }
        ST(L15_LEVER_CAM_STEP) = 0;
        CB->etat_objet("OBCOMP", 5050, -2253, 110, 0, 0, 0x4000, 0, 4);
    }
    /* 0x8BC3 */
    if (ST(L15_DANRYS_INTERRUPTIBLE) == 1 && ST(L15_DANRYS_ENDING_FLAG) == 1) {
        /* 0x8BE7 */
        if (ST(ST_VIE(P_DANRYS)) == 1) {
            ST(L15_ANIM_PHASE) = 1;
        }
        /* 0x8C08 */
        if (ST(ST_VIE(P_DANRYS)) >= 2) {
            ST(L15_ANIM_PHASE) = 2;
        }
        ST(L15_LEVER_CAM_STEP) = 1;
        CB->etat_objet("OBCOMP", 5050, -2253, 110, 0xF600, 0, 0x4000, 0, 4);
        CB->load_anim("215MOT01", 6);
    }
    /* 0x8C6F */
    if (ST(L15_ANIM_PHASE) == 0) {
        /* 0x8C81 */
        if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
            CB->bruitage("L15AMB1", -1, -1);
        }
        /* 0x8CA5 */
        if (ST(L15_DANRYS_ENDING_FLAG) == 1) {
            CB->bruitage("L15AMB3", -1, -1);
        }
    }
    /* 0x8CC9 */
    if (ST(L15_ANIM_PHASE) != 0) {
        /* 0x8CDB */
        if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
            CB->bruitage("L15AMB2", -1, -1);
        }
        /* 0x8CFF */
        if (ST(L15_DANRYS_ENDING_FLAG) == 1) {
            CB->bruitage("L15AMB4", -1, -1);
        }
    }
    /* 0x8D23 */
    if (ST(ST_VIE(P_DANRYS)) == 1) {
        CB->divers(DV_MUSIC, 1, 0);
    }
    /* 0x8D47 */
    if (ST(ST_VIE(P_DANRYS)) != 1) {
        CB->divers(DV_MUSIC, 2, 0);
    }
    ST(ST_START_POINT(15)) = 0;
}

/* trigger 01 — trig01_1000D293 — zone 1: leave for room 14 */
static void trig01(void) {
    /* 0xD2A5 */
    if (ST(L15_ROOM_ENTERED) != 0 && ST(L15_PROGRESS_STEP) == 0) {
        ST(L15_PROGRESS_STEP) = 1;
    }
    /* 0xD2D8 */
    if (ST(L15_DANRYS_INTERRUPTIBLE) == 1 && ST(L15_ROOM_PART) == 1 && ST(V_EVENT15_STEP) == 0) {
        ST(L15_DANRYS_INTERRUPTIBLE) = 0;
    }
    CB->select_pile(0, 2);
    CB->retire_zone(1);
    ST(ST_LEVEL) = 15;
    CB->divers(DV_MUSIC, 1, 4);
    CB->change_piece(14, -1);
}

/* trigger 02 — trig02_1000D35C — chained zone triggers, the Danrys scenes and weapon reactions */
static void trig02(void) {
    /* 0xD36B */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xD6D8) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 3)) {
        trig03();
    }
    /* 0xD3D0 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xC738) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 9)) {
        trig09();
    }
    /* 0xD435 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x2EE0) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13)) {
        trig13();
    }
    /* 0xD49A */
    if (ST(ST_ACTION_KEY) == 1 && CB->direction_perso(P_KALHI13) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12)) {
        trig12();
    }
    /* 0xD4E1 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DANRYS)) != VIE_DEAD && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_perso(P_DANRYS) && ST(L15_TRADE_SCENE_STEP) == 0 && ST(L15_DANRYS_SCENE_STATE) != 0) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->stop_joindre(-P_DANRYS);
        CB->joindre(ST(ST_HERO), 0, 0, 0, P_DANRYS);
        /* 0xD5A1 */
        if (ST(L15_ROOM_ENTERED) == 0) {
            ST(L15_PROGRESS_STEP) = 1;
            ST(L15_ARKHAN2_TOPIC_90) = 1;
            ST(L15_ROOM_ENTERED) = 1;
            /* 0xD5E0 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0xD5F2 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    lieu_action_register("L15D300A", P_ARKHAN2, "SG00DIA01", NULL);
                    lieu_action_register("L15D300B", P_ARKHAN2, "SG00DIA01", NULL);
                }
                /* 0xD630 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    lieu_action_register("L15D301A", P_ARKHAN2, "SG00DIA01", NULL);
                    lieu_action_register("L15D301B", P_ARKHAN2, "SG00DIA01", NULL);
                    lieu_action_register("L15D301C", P_ARKHAN2, "SG00DIA01", NULL);
                }
            }
            /* 0xD684 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0xD696 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    lieu_action_register("L15D302A", P_ARKHAN3, "SG00DIA01", NULL);
                    lieu_action_register("L15D302B", P_ARKHAN3, "SG00DIA01", NULL);
                    lieu_action_register("L15D302C", P_ARKHAN3, "SG00DIA01", NULL);
                }
                /* 0xD6EA */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    lieu_action_register("L15D303A", P_ARKHAN3, "SG00DIA01", NULL);
                    lieu_action_register("L15D303B", P_ARKHAN3, "SG00DIA01", NULL);
                    lieu_action_register("L15D303C", P_ARKHAN3, "SG00DIA01", NULL);
                }
            }
            CB->stop_joindre(-P_KALHI13);
            CB->joindre(-P_KALHI13, 0, 0, 0, P_DANRYS);
            lieu_action_register("L15D304A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            lieu_action_register("L15D304B", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            lieu_action_register("L15D304C", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            CB->joindre(-P_DANRYS, 0, 0, 0, P_KALHI13);
            lieu_action_register("L15D305", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
            CB->joindre(-P_DANRYS, 0, 0, 0, ST(ST_HERO));
            lieu_action_register("L15D306A", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
            lieu_action_register("L15D306B", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
            return;
        }
        /* 0xD86C */
        if (ST(L15_ROOM_ENTERED) == 1) {
            /* 0xD883 */
            if (CB->divers(DV_CAN_FIGHT, 0, 2) && ST(L15_CRYSTAL_ROUND) != 3 && (ST(L15_DANRYS_PRE_TALK_DONE) == 0 || (ST(L15_DANRYS_PRE_TALK_DONE) == 1 && (ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON) || (g_dat_26B3C == 0 && CB->divers(DV_CAN_FIGHT, 0, 1)) || (g_dat_26B3C == 1 && CB->divers(DV_CAN_FIGHT, 0, 0)))))) {
                ST(L15_ARKHAN2_TOPIC_90) = 1;
                /* 0xD941 */
                if (CB->divers(DV_CAN_FIGHT, 0, 0)) {
                    /* 0xD95B */
                    if (CB->divers(DV_CAN_FIGHT, 6, 2) || CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        g_dat_26B3C = 0;
                        /* 0xD9B4 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D08A", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xD9DF */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D08B", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        lieu_action_register("L15D901A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        lieu_action_register("L15D901B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        CB->animation("SG00PRE82_10");
                        CB->animation("ar_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0xDA8C */
                    if (CB->divers(DV_ITEM_CHARGE, 0, -1) >= 80) {
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        g_dat_26B3C = 0;
                        /* 0xDACB */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D08A", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xDAF6 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D08B", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        CB->animation("SP09PRE90");
                        /* 0xDB34 */
                        if (CB->divers(DV_CAN_FIGHT, 9, 0)) {
                            lieu_action_register("L15D11A", P_DANRYS, "P09DIA55", "SP09ARR01_10");
                            lieu_action_register("L15D11B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        }
                        /* 0xDB80 */
                        if (CB->divers(DV_CAN_FIGHT, 8, 0)) {
                            lieu_action_register("L15D11C", P_DANRYS, "P09DIA55", "SP09ARR01_10");
                        }
                        CB->animation("SG00PRE82_10");
                        CB->animation("ar_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0xDBFB */
                    if (CB->divers(DV_ITEM_CHARGE, 0, -1) < 80) {
                        /* 0xDC14 */
                        if (CB->distance(6, 2) < 80) {
                            /* 0xDC26 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                CB->animation("P02MAR30");
                            }
                            /* 0xDC43 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                CB->animation("P03MAR30");
                            }
                            CB->animation("P00_65535");
                        }
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        g_dat_26B3C = 0;
                        /* 0xDCA0 */
                        if (CB->divers(DV_CAN_FIGHT, 9, 0)) {
                            ST(L15_TRADE_TYPE) = 0;
                        }
                        /* 0xDCC6 */
                        if (CB->divers(DV_CAN_FIGHT, 8, 0)) {
                            ST(L15_TRADE_TYPE) = 1;
                        }
                        /* 0xDCE4 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D08A", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xDD0F */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D08B", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        CB->animation("SP09PRE90");
                        lieu_action_register("L15D09", P_DANRYS, "P09DIA53", "SP09ARR01_10");
                        l15_pre_scene2(ST(ST_HERO), P_DANRYS, ST(ST_PLAYER_WEAPON), 6, 2);
                        CB->modify_var(&ST(V_PRE90_OBJECT), ST(ST_PLAYER_WEAPON));
                        /* 0xDD9D */
                        if (ST(L15_TRADE_TYPE) == 0) {
                            CB->animation("SP09ATT04_4000");
                            CB->animation("P09_65535");
                            lieu_pre_scene(ST(ST_HERO), P_DANRYS, ST(ST_PLAYER_WEAPON), 6, 2);
                            CB->modify_var(&ST(V_PRE90_OBJECT), 0);
                        }
                        /* 0xDDFB */
                        if (ST(L15_TRADE_TYPE) == 1) {
                            lieu_action_register("L15D21C", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                            CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 1);
                        }
                        CB->animation("SP09PRE73");
                        CB->animation("P09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                }
                /* 0xDE8C */
                if (CB->divers(DV_CAN_FIGHT, 0, 1) && (ST(L15_DANRYS_PRE_TALK_DONE) == 0 || (ST(L15_DANRYS_PRE_TALK_DONE) == 1 && (ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON) || (g_dat_26B3C == 0 && CB->divers(DV_CAN_FIGHT, 0, 1)) || (g_dat_26B3C == 1 && CB->divers(DV_CAN_FIGHT, 0, 0)))))) {
                    ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    g_dat_26B3C = 1;
                    /* 0xDF51 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D08A", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                    }
                    /* 0xDF7C */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D08B", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                    }
                    lieu_action_register("L15D131A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                    lieu_action_register("L15D131B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                    CB->animation("SG00PRE82_10");
                    CB->animation("ar_10");
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
            }
            /* 0xE023 */
            if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(L15_CRYSTAL_ROUND) != 3) {
                ST(L15_ARKHAN2_TOPIC_90) = 1;
                /* 0xE06D */
                if (ST(ST_PLAYER_WEAPON) == OB_BOW_TO_KEY || (ST(ST_PLAYER_WEAPON) == OB_HANDLE && ST(L15_CRYSTAL_ROUND) == 1) ||
                    ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_2 || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_1 || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_3 ||
                    ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE40 || ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE41 || ST(ST_PLAYER_WEAPON) == OB_CRYST_SLIV ||
                    ST(ST_PLAYER_WEAPON) == OB_RYLS_NOTES || ST(ST_PLAYER_WEAPON) == OB_DRAWINGS || ST(ST_PLAYER_WEAPON) == OB_DIAG_WEAPON ||
                    ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_KEY || ST(ST_PLAYER_WEAPON) == OB_FLASK86 || ST(ST_PLAYER_WEAPON) == OB_CAN) {
                    /* 0xE17B */
                    if (ST(ST_PLAYER_WEAPON) == OB_HANDLE && ST(L15_CRYSTAL_ROUND) == 1 && (ST(L15_DANRYS_PRE_TALK_DONE) == 0 || (ST(L15_DANRYS_PRE_TALK_DONE) == 1 && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)))) {
                        ST(L15_DANRYS_APPROACHED) = 1;
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        g_dat_26B34 = 1;
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        ST(L15_CRYSTAL_ROUND) = 2;
                        /* 0xE22F */
                        if (CB->distance(6, 2) < 80) {
                            /* 0xE241 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                CB->animation("P02MAR30");
                            }
                            /* 0xE25E */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                CB->animation("P03MAR30");
                            }
                            CB->animation("P00_65535");
                        }
                        /* 0xE286 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D12C", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xE2B1 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D12D", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        lieu_action_register("L15D14A", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        CB->animation("SP09PRE90_1000");
                        lieu_action_register("L15D14B", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                        lieu_action_register("L15D14C", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        l15_pre_scene2(ST(ST_HERO), P_DANRYS, ST(ST_PLAYER_WEAPON), 6, 2);
                        CB->evenement(31, 300000);
                        lieu_action_register("L15D14D", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                        lieu_action_register("L15D14E", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                        CB->animation("SP09PRE73");
                        CB->animation("P09ARR01_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(V_PRE90_OBJECT), 0);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0xE3F7 */
                    if (ST(ST_PLAYER_WEAPON) == OB_BOW_TO_KEY) {
                        /* 0xE409 */
                        if (ST(L15_CONVO_ABC) == 0) {
                            ST(L15_CONVO_ABC) = 1;
                            /* 0xE42A */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L15D322", P_ARKHAN2, "SG00DIA04", NULL);
                            }
                            /* 0xE452 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D323", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            CB->animation("SP09PRE90_1500");
                            lieu_action_register("L15D324", P_DANRYS, "P09DIA51", "SP09ARR01_10");
                            /* 0xE49E */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                /* 0xE4B0 */
                                if (ST(ST_SPECIAL_MODE) == 1) {
                                    lieu_action_register("L15D325A", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D325B", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D325C", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D325D", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                                /* 0xE51A */
                                if (ST(ST_SPECIAL_MODE) == 0) {
                                    lieu_action_register("L15D326A", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D326B", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D326C", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D326D", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                            }
                            /* 0xE584 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D327A", P_ARKHAN3, "SG00DIA04", NULL);
                                lieu_action_register("L15D327B", P_ARKHAN3, "SG00DIA04", NULL);
                                lieu_action_register("L15D327C", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            lieu_action_register("L15D324Y", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                            lieu_action_register("L15D324Z", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                            return;
                        }
                        /* 0xE63E */
                        if (ST(L15_CONVO_ABC) == 1) {
                            ST(L15_CONVO_ABC) = 2;
                            /* 0xE65F */
                            if (ST(L15_CONVO_C18) == 0) {
                                /* 0xE671 */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    lieu_action_register("L15D328", P_ARKHAN2, "SG00DIA01", NULL);
                                }
                                /* 0xE699 */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    lieu_action_register("L15D329", P_ARKHAN3, "SG00DIA01", NULL);
                                }
                                lieu_action_register("L15D330A", P_DANRYS, "SP09DIA50", "SP09ARR01_10");
                                lieu_action_register("L15D330B", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                                lieu_action_register("L15D330C", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                                lieu_action_register("L15D330D", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                                lieu_action_register("L15D330E", P_DANRYS, "SP09DIA62", "SP09ARR01_10");
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                return;
                            }
                        }
                        /* 0xE772 */
                        if (ST(L15_CONVO_ABC) == 2) {
                            ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                            /* 0xE798 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L15D331", P_ARKHAN2, "SG00DIA01", NULL);
                            }
                            /* 0xE7C0 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D332", P_ARKHAN3, "SG00DIA01", NULL);
                            }
                            /* 0xE7E8 */
                            if (ST(L15_DANRYS_INTRO_114) == 0) {
                                lieu_action_register("L15D333", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                                ST(L15_DANRYS_INTRO_114) = 1;
                            } else {
                                lieu_action_register("L15D334", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                            }
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->bloque(BLOQUE_NONE);
                            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                            return;
                        }
                    }
                    /* 0xE874 */
                    if (ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_1 || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_3 || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_2) {
                        /* 0xE8AA */
                        if (ST(L15_DANRYS_TALK_STAGE_A94) == 0 && ST(L15_CONVO_ABC) == 0) {
                            ST(L15_DANRYS_TALK_STAGE_A94) = 1;
                            ST(L15_CONVO_ABC) = 1;
                            ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                            ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                            ST(L15_CRYSTAL_GIVEN_A) = ST(ST_PLAYER_WEAPON);
                            /* 0xE923 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L15D335", P_ARKHAN2, "SG00DIA04", NULL);
                            }
                            /* 0xE94B */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D336", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            lieu_action_register("L15D337", P_DANRYS, "SP09DIA51", "SP09ARR01_10");
                            /* 0xE98C */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                /* 0xE99E */
                                if (ST(ST_SPECIAL_MODE) == 1) {
                                    lieu_action_register("L15D338A", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D338B", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D338C", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                                /* 0xE9F2 */
                                if (ST(ST_SPECIAL_MODE) == 0) {
                                    lieu_action_register("L15D339A", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D339B", P_ARKHAN2, "SG00DIA04", NULL);
                                    lieu_action_register("L15D339C", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                            }
                            /* 0xEA46 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D327A", P_ARKHAN3, "SG00DIA04", NULL);
                                lieu_action_register("L15D327B", P_ARKHAN3, "SG00DIA04", NULL);
                                lieu_action_register("L15D327C", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            CB->animation("SP09PRE90");
                            lieu_action_register("L15D340A", P_DANRYS, "P09DIA54", "SP09ARR01_10");
                            lieu_action_register("L15D340B", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                            lieu_action_register("L15D340C", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                            CB->animation("SG00PRE82_10");
                            CB->animation("ar_10");
                            CB->bloque(BLOQUE_NONE);
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                            return;
                        }
                        /* 0xEB3A */
                        if (ST(L15_DANRYS_TALK_STAGE_A94) == 0 && ST(L15_CONVO_ABC) >= 1) {
                            ST(L15_DANRYS_TALK_STAGE_A94) = 1;
                            ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                            ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                            ST(L15_CRYSTAL_GIVEN_A) = ST(ST_PLAYER_WEAPON);
                            /* 0xEBA4 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L15D341", P_ARKHAN2, "SG00DIA04", NULL);
                                lieu_action_register("L15D338C", P_ARKHAN2, "SG00DIA04", NULL);
                            }
                            /* 0xEBE2 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D327A", P_ARKHAN3, "SG00DIA04", NULL);
                                lieu_action_register("L15D327B", P_ARKHAN3, "SG00DIA04", NULL);
                                lieu_action_register("L15D327C", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            CB->animation("SP09PRE90_1500");
                            lieu_action_register("L15D340A", P_DANRYS, "P09DIA53", "SP09ARR01_10");
                            lieu_action_register("L15D340B", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                            lieu_action_register("L15D340C", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                            CB->animation("SG00PRE82_10");
                            CB->animation("ar_10");
                            CB->bloque(BLOQUE_NONE);
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                            return;
                        }
                        /* 0xECD6 */
                        if (ST(L15_DANRYS_TALK_STAGE_A94) == 1) {
                            /* 0xECF0 */
                            if (ST(ST_PLAYER_WEAPON) == ST(L15_CRYSTAL_GIVEN_A)) {
                                ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                                ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                                /* 0xED34 */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    lieu_action_register("L15D342", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                }
                                /* 0xED5F */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    lieu_action_register("L15D343", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                                }
                                lieu_action_register("L15D344", P_DANRYS, "SP09DIA58", "SP09ARR01_10");
                                CB->animation("SG00PRE82_10");
                                CB->animation("ar_10");
                                CB->bloque(BLOQUE_NONE);
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                return;
                            }
                            /* 0xEDF5 */
                            if (ST(ST_PLAYER_WEAPON) != ST(L15_CRYSTAL_GIVEN_A)) {
                                ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                                ST(L15_CRYSTAL_GIVEN_B) = ST(ST_PLAYER_WEAPON);
                                /* 0xEE3E */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    /* 0xEE50 */
                                    if (ST(ST_SPECIAL_MODE) == 1) {
                                        lieu_action_register("L15D345", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                    }
                                    /* 0xEE7B */
                                    if (ST(ST_SPECIAL_MODE) == 0) {
                                        lieu_action_register("L15D346", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                    }
                                    CB->animation("P02_65535");
                                }
                                /* 0xEEB1 */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    /* 0xEEC3 */
                                    if (ST(ST_SPECIAL_MODE) == 1) {
                                        lieu_action_register("L15D347", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                                    }
                                    /* 0xEEEE */
                                    if (ST(ST_SPECIAL_MODE) == 0) {
                                        lieu_action_register("L15D348", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                                    }
                                    CB->animation("P03_65535");
                                }
                                /* 0xEF24 */
                                if (ST(L15_DANRYS_TALK_STAGE_E80) < 2) {
                                    /* 0xEF36 */
                                    if (ST(L15_DANRYS_TALK_STAGE_A80) == 0) {
                                        ST(L15_DANRYS_TALK_STAGE_A80) = 1;
                                        /* 0xEF57 */
                                        if (ST(L15_DANRYS_TALK_STAGE_E80) == 0) {
                                            ST(L15_DANRYS_TALK_STAGE_E80) = 1;
                                        }
                                        CB->animation("SP09PRE90_1000");
                                        lieu_action_register("L15D23A", P_DANRYS, "P09DIA01", "SP09ARR01_10");
                                        lieu_action_register("L15D23B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                                        lieu_action_register("L15D23C", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                                        CB->animation("SG00PRE82_200");
                                        CB->animation("ar_10");
                                        CB->animation("SP09DIA01");
                                        lieu_action_register("L15D23D", P_DANRYS, "P09DIA50", "SP09ARR01_10");
                                        lieu_action_register("L15D23E", P_DANRYS, "P09DIA53", "SP09ARR01_10");
                                        CB->animation("SP09ARR01_10");
                                        CB->bloque(BLOQUE_NONE);
                                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                        return;
                                    }
                                    /* 0xF060 */
                                    if (ST(L15_DANRYS_TALK_STAGE_A80) >= 1) {
                                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                                        /* 0xF081 */
                                        if (ST(L15_DANRYS_TALK_STAGE_A80) == 1) {
                                            lieu_action_register("L15D24B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                                        }
                                        /* 0xF0AC */
                                        if (ST(L15_DANRYS_TALK_STAGE_A80) == 2) {
                                            ST(L15_DANRYS_TALK_STAGE_A80) = 2;
                                            lieu_action_register("L15D24A", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                                        }
                                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                        CB->bloque(BLOQUE_NONE);
                                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                        return;
                                    }
                                }
                                /* 0xF11A */
                                if (ST(L15_DANRYS_TALK_STAGE_E80) == 2) {
                                    ST(V_CRYSTAL_PROGRESS) = 4;
                                    ST(L15_DANRYS_CALL_DONE) = 1;
                                    ST(L15_TRADE_REWARD_PENDING) = 1;
                                    CB->animation("SP09PRE90_1500");
                                    lieu_action_register("L15D23A", P_DANRYS, "P09DIA01", "SP09ARR01_10");
                                    lieu_action_register("L15D23B", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                                    lieu_action_register("L15D21A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                                    lieu_action_register("L15D21B", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                                    CB->animation("SG00PRE82_200");
                                    CB->animation("ar_10");
                                    CB->bloque(BLOQUE_NONE);
                                    CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 1);
                                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                    return;
                                }
                            }
                        }
                    }
                    /* 0xF229 */
                    if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE40 && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        /* 0xF278 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D608", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xF2A3 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D609", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        lieu_action_register("L15D610", P_DANRYS, "SP09PRE90", "SP09ARR01_10");
                        /* 0xF2E7 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D611", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xF312 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D612", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        CB->animation("Sar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xF366 */
                    if (ST(ST_PLAYER_WEAPON) == OB_OBJ_BEFORE41 && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        /* 0xF3B5 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D604", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xF3E0 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D605", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xF40B */
                        if (ST(L15_TRIG_564) == 1) {
                            lieu_action_register("L15D607A", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                            lieu_action_register("L15D607B", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        }
                        /* 0xF44F */
                        if (ST(L15_TRIG_564) == 0) {
                            ST(L15_TRIG_564) = 1;
                            lieu_action_register("L15D606A", P_DANRYS, "SP09PRE90", "SP09ARR01_10");
                            lieu_action_register("L15D606B", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        }
                        CB->animation("Sar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        return;
                    }
                    /* 0xF4CB */
                    if (ST(ST_PLAYER_WEAPON) == OB_CRYST_SLIV && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        /* 0xF4F7 */
                        if (ST(L15_DANRYS_TALK_STAGE_E80) == 0) {
                            ST(L15_DANRYS_TALK_STAGE_E80) = 2;
                            ST(L15_DANRYS_APPROACHED) = 1;
                            /* 0xF52E */
                            if (CB->distance(6, 2) < 80) {
                                /* 0xF540 */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    CB->animation("P02MAR30");
                                }
                                /* 0xF55D */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    CB->animation("P03MAR30");
                                }
                                CB->animation("P00_65535");
                            }
                            /* 0xF585 */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                /* 0xF597 */
                                if (ST(ST_SPECIAL_MODE) == 1) {
                                    lieu_action_register("L15D468", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                }
                                /* 0xF5C2 */
                                if (ST(ST_SPECIAL_MODE) == 0) {
                                    lieu_action_register("L15D469", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                }
                            }
                            /* 0xF5ED */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D470", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                            }
                            l15_pre_scene2(ST(ST_HERO), P_DANRYS, OB_CRYST_SLIV, 6, 2);
                            CB->animation("SP09PRE91");
                            lieu_action_register("L15D471", P_DANRYS, "P09DIA54", "SP09ARR01_10");
                            CB->animation("SP09PRE73");
                            CB->animation("P09ARR01_10");
                            CB->bloque(BLOQUE_NONE);
                            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                            return;
                        }
                        /* 0xF6A1 */
                        if (ST(L15_DANRYS_TALK_STAGE_E80) == 1) {
                            ST(L15_DANRYS_APPROACHED) = 1;
                            ST(L15_DANRYS_CALL_DONE) = 1;
                            ST(L15_DANRYS_TALK_STAGE_E80) = 2;
                            /* 0xF6E7 */
                            if (CB->distance(6, 2) < 80) {
                                /* 0xF6F9 */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    CB->animation("P02MAR30");
                                }
                                /* 0xF716 */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    CB->animation("P03MAR30");
                                }
                                CB->animation("P00_65535");
                            }
                            /* 0xF73E */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                /* 0xF750 */
                                if (ST(ST_SPECIAL_MODE) == 1) {
                                    lieu_action_register("L15D350", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                }
                                /* 0xF77B */
                                if (ST(ST_SPECIAL_MODE) == 0) {
                                    lieu_action_register("L15D351", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                }
                            }
                            /* 0xF7A6 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                /* 0xF7B8 */
                                if (ST(ST_SPECIAL_MODE) == 1) {
                                    lieu_action_register("L15D352", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                                }
                                /* 0xF7E3 */
                                if (ST(ST_SPECIAL_MODE) == 0) {
                                    lieu_action_register("L15D353", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                                }
                            }
                            /* 0xF80E */
                            if (ST(V_CRYSTAL_PROGRESS) == 2) {
                                ST(V_CRYSTAL_PROGRESS) = 4;
                                l15_pre_scene2(ST(ST_HERO), P_DANRYS, OB_CRYST_SLIV, 6, 2);
                                CB->animation("SP09PRE91");
                                lieu_action_register("L15D354", P_DANRYS, "P09DIA54", "SP09ARR01_10");
                                lieu_action_register("L15D21C", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                                CB->animation("SG00PRE82_200");
                                CB->animation("ar_10");
                                CB->bloque(BLOQUE_NONE);
                                CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 1);
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->modify_var(&ST(V_PRE90_OBJECT), 1);
                                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                return;
                            }
                            /* 0xF8FF */
                            if (ST(V_CRYSTAL_PROGRESS) == 5) {
                                ST(L15_DANRYS_APPROACHED) = 1;
                                lieu_action_register("L15D354", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                                CB->animation("SG00PRE82");
                                /* 0xF944 */
                                if (ST(ST_HERO) == P_ARKHAN2) {
                                    lieu_action_register("L15D356A", P_ARKHAN2, "SG00DIA01", NULL);
                                    lieu_action_register("L15D356B", P_ARKHAN2, "SG00DIA01", NULL);
                                }
                                /* 0xF982 */
                                if (ST(ST_HERO) == P_ARKHAN3) {
                                    lieu_action_register("L15D357A", P_ARKHAN3, "SG00DIA01", NULL);
                                    lieu_action_register("L15D357B", P_ARKHAN3, "SG00DIA01", NULL);
                                }
                                lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_CRYST_SLIV, 6, 2);
                                CB->animation("SP09PRE73");
                                CB->animation("P09ARR01");
                                CB->modify_var(&ST(V_PRE90_OBJECT), 1);
                                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                                CB->bloque(BLOQUE_NONE);
                                CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                                return;
                            }
                        }
                    }
                    /* 0xFA3C */
                    if ((ST(ST_PLAYER_WEAPON) == OB_RYLS_NOTES || ST(ST_PLAYER_WEAPON) == OB_SORDOS_DOC || ST(ST_PLAYER_WEAPON) == OB_DRAWINGS) && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        /* 0xFAAF */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0xFAC1 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D600", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                            }
                            /* 0xFAEC */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D601", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                            }
                        }
                        /* 0xFB17 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D602", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        CB->animation("SP09PRE90_1300");
                        lieu_action_register("L15D603A", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D603B", P_DANRYS, "SP09DIA58", "SP09ARR01_10");
                        CB->animation("SG00PRE82_10");
                        CB->animation("ar_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0xFBC9 */
                    if (ST(ST_PLAYER_WEAPON) == OB_DIAG_WEAPON && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(L15_CRYSTAL_ROUND) = 1;
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(L15_DANRYS_APPROACHED) = 1;
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        /* 0xFC36 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0xFC48 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                CB->animation("SG00PRE80");
                                CB->dialogue("L15D358A", P_ARKHAN2);
                                CB->dialogue("L15D358B", P_ARKHAN2);
                                CB->animation("SG00PRE81_10");
                            }
                            /* 0xFC90 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                CB->animation("SG00PRE80");
                                CB->dialogue("L15D359A", P_ARKHAN2);
                                CB->dialogue("L15D359B", P_ARKHAN2);
                                CB->animation("SG00PRE81_10");
                            }
                        }
                        /* 0xFCD8 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0xFCEA */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D360", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                            }
                            /* 0xFD15 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D361", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                            }
                        }
                        lieu_action_register("L15D362A", P_DANRYS, "SP09PRE90", "SP09ARR01_10");
                        lieu_action_register("L15D362B", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        lieu_action_register("L15D362C", P_DANRYS, "SP09DIA62", "SP09ARR01_10");
                        lieu_action_register("L15D362D", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                        lieu_action_register("L15D362E", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        /* 0xFDBD */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D363", P_ARKHAN2, "SG00DIA01", NULL);
                        }
                        /* 0xFDE5 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D364", P_ARKHAN3, "SG00DIA01", NULL);
                        }
                        lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_DIAG_WEAPON, 6, 2);
                        lieu_action_register("L15D365A", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                        lieu_action_register("L15D365B", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                        lieu_action_register("L15D365C", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D365D", P_DANRYS, "SP09DIA58", "SP09ARR01_10");
                        lieu_action_register("L15D365E", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                        lieu_action_register("L15D365F", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        /* 0xFEBE */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D463", P_ARKHAN2, "SG00DIA01", NULL);
                        }
                        /* 0xFEE6 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D464", P_ARKHAN3, "SG00DIA01", NULL);
                        }
                        CB->animation("SP09PRE73");
                        CB->animation("P09ARR01_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(V_PRE90_OBJECT), 1);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0xFF6F */
                    if (ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_KEY && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        /* 0xFFAF */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D604", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0xFFDA */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D605", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        lieu_action_register("L15D23B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                        CB->animation("ar_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x1005D */
                    if (ST(ST_PLAYER_WEAPON) == OB_FLASK86 && ST(L15_FLASK86_STAGE) == 0) {
                        ST(L15_FLASK86_STAGE) = 1;
                        /* 0x10090 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D366A", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                            CB->dialogue("L15D366B", P_ARKHAN2);
                        }
                        /* 0x100CB */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D367A", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                            CB->dialogue("L15D367B", P_ARKHAN3);
                        }
                        CB->animation("SP09PRE90_1000");
                        lieu_action_register("L15D368A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        CB->animation("SG00PRE82_10");
                        CB->joindre(-P_DANRYS, 0, 0, 0, -6);
                        CB->animation("Sar_10");
                        lieu_action_register("L15D368B", P_DANRYS, "SP09DIA55", "SP09ATT50_10");
                        CB->joindre(-P_DANRYS, 0, 0, 0, ST(ST_HERO));
                        lieu_action_register("L15D368C", P_DANRYS, "SP09DIA55", "SP09ATT50_10");
                        CB->animation("SP09ARR01_10");
                        CB->animation("Sar_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x101EC */
                    if (ST(ST_PLAYER_WEAPON) == OB_FLASK86 && ST(L15_FLASK86_STAGE) >= 1) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        /* 0x10224 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D369", P_ARKHAN2, "SG00DIA04", NULL);
                        }
                        /* 0x1024C */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D370", P_ARKHAN3, "SG00DIA04", NULL);
                        }
                        lieu_action_register("L15D371A", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D371B", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D371C", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D371D", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        /* 0x102D8 */
                        if (ST(L15_FLASK86_STAGE) == 1) {
                            ST(L15_FLASK86_STAGE) = 2;
                            lieu_action_register("L15D371E", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                        }
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x10346 */
                    if (ST(ST_PLAYER_WEAPON) == OB_CAN && ST(ST_PLAYER_WEAPON) != ST(V_LAST_WEAPON)) {
                        ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                        ST(L15_CONVO_C00) = 1;
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        /* 0x103A4 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x103B6 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D372", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                            }
                            /* 0x103E1 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D373A", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                lieu_action_register("L15D373B", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                                lieu_action_register("L15D375A", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                            }
                        }
                        /* 0x1043E */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D374", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                        lieu_action_register("L15D371A", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D371B", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        lieu_action_register("L15D375B", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        CB->animation("SG00PRE82_10");
                        CB->animation("ar_10");
                        CB->bloque(BLOQUE_NONE);
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                } else {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    ST(V_DLG_RANDOM) = CB->random(2);
                    /* 0x1052E */
                    if (ST(V_DLG_RANDOM) == 0) {
                        /* 0x10540 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D376", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0x1056B */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D377", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                    }
                    /* 0x10596 */
                    if (ST(V_DLG_RANDOM) == 1) {
                        /* 0x105A8 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                        }
                        /* 0x105D3 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("LCOD19", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                        }
                    }
                    ST(V_DLG_RANDOM) = CB->random(2);
                    /* 0x10615 */
                    if (ST(V_DLG_RANDOM) == 0) {
                        lieu_action_register("L15D378", P_DANRYS, "SP09PRE90", "SP09ARR01_10");
                    }
                    /* 0x10640 */
                    if (ST(V_DLG_RANDOM) == 1) {
                        lieu_action_register("L15D379", P_DANRYS, "SP09PRE90", "SP09ARR01_10");
                    }
                    CB->animation("SG00PRE82_200");
                    CB->animation("ar_10");
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
            }
            /* 0x106B5 */
            if (ST(L15_DANRYS_PRE_TALK_DONE) == 1 || ST(ST_PLAYER_WEAPON) == 0 || ST(L15_CRYSTAL_ROUND) == 3) {
                ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                /* 0x106FF */
                if (ST(L15_CRYSTAL_ROUND) == 1 && ST(L15_CONVO_AA8) < 2) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    /* 0x10732 */
                    if (ST(L15_CONVO_AA8) == 0) {
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(L15_CONVO_AA8) = 1;
                        /* 0x10762 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x10774 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D380A", P_ARKHAN2, "SG00DIA07", "Sar");
                                lieu_action_register("L15D380B", P_ARKHAN2, "SG00DIA07", "Sar");
                            }
                            /* 0x107B8 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D381", P_ARKHAN2, "SG00DIA07", "Sar");
                            }
                        }
                        /* 0x107E3 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D382", P_ARKHAN3, "SG00DIA07", "Sar");
                        }
                        lieu_action_register("L15D383A", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                        lieu_action_register("L15D383B", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                        lieu_action_register("L15D383C", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                        lieu_action_register("L15D383D", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x108A6 */
                    if (ST(L15_CONVO_AA8) == 1) {
                        ST(L15_CONVO_AA8) = 2;
                        /* 0x108C7 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D384", P_ARKHAN2, "SG00DIA05", NULL);
                        }
                        /* 0x108EF */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D385", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L15D386A", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                        lieu_action_register("L15D386B", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                        lieu_action_register("L15D386C", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                }
                /* 0x10996 */
                if (ST(L15_CRYSTAL_ROUND) > 1 && ST(L15_CRYSTAL_ROUND) < 4 && g_dat_26B34 == 0) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    /* 0x109D6 */
                    if (ST(ST_HERO) == P_ARKHAN2 && ST(L15_CONVO_AAC) < 2 && ST(L15_DANRYS_TOPIC_58) != 3) {
                        /* 0x10A09 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D387", P_ARKHAN2, "SG00DIA01", NULL);
                        }
                        /* 0x10A31 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D388", P_ARKHAN2, "SG00DIA01", NULL);
                        }
                    }
                    /* 0x10A59 */
                    if (ST(ST_HERO) == P_ARKHAN3 && ST(L15_CONVO_AAC) < 2 && ST(L15_DANRYS_TOPIC_58) != 3) {
                        /* 0x10A8C */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D389", P_ARKHAN3, "SG00DIA01", NULL);
                        }
                        /* 0x10AB4 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D390", P_ARKHAN3, "SG00DIA01", NULL);
                        }
                    }
                    /* 0x10ADC */
                    if (ST(L15_CRYSTAL_ROUND) == 2 && ST(L15_CONVO_AAC) == 0 && ST(L15_DANRYS_TOPIC_58) != 3) {
                        ST(L15_CONVO_AAC) = 1;
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(L15_DANRYS_TOPIC_58) = 3;
                        lieu_action_register("L15D391A", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                        lieu_action_register("L15D391B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x10B9F */
                    if (ST(L15_CRYSTAL_ROUND) == 2 && ST(L15_CONVO_AAC) == 1 && ST(L15_DANRYS_TOPIC_58) != 3) {
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(L15_CONVO_AAC) = 2;
                        ST(L15_DANRYS_TOPIC_58) = 3;
                        lieu_action_register("L15D392A", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                        lieu_action_register("L15D392B", P_DANRYS, "SP09DIA60", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x10C62 */
                    if (ST(L15_CRYSTAL_ROUND) == 3) {
                        ST(V_WEAPON_WARNED) = 1;
                        CB->joindre(P_DANRYS, 0, 0, 0, ST(ST_HERO));
                        ST(V_EVENT_31) = 0;
                        ST(L15_CRYSTAL_ROUND) = 4;
                        ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                        ST(V_PRE90_OBJECT) = 0;
                        /* 0x10CDC */
                        if (ST(L15_CONVO_AAC) != 1) {
                            lieu_action_register("L15D393A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                            lieu_action_register("L15D393B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        }
                        /* 0x10D20 */
                        if (ST(L15_CONVO_AAC) == 1) {
                            lieu_action_register("L15D394A", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                            lieu_action_register("L15D394B", P_DANRYS, "SP09DIA55", "SP09ARR01_10");
                        }
                        /* 0x10D5F */
                        if (g_dat_26B38 == 1) {
                            CB->animation("SP09PRE73");
                            CB->animation("P09_65535");
                            CB->link_objet(9, "OB27", 0x1B, 1);
                        }
                        lieu_pre_scene(ST(ST_HERO), P_DANRYS, OB_F_THROWER, 6, 2);
                        /* 0x10DB6 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            CB->animation("P02PRE73");
                        }
                        /* 0x10DD3 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            CB->animation("P03PRE73");
                        }
                        CB->animation("ar_10");
                        CB->divers_empile(DV_GIVE_ITEM, 27, -1);
                        CB->divers_empile(DV_GIVE_ITEM, 6, 3);
                        CB->etat_objet("OB27", -1, -1, -1, -1, -1, -1, -3, -1);
                        CB->divers_empile(DV_PERSONA_ITEMS, P_DANRYS, -OB_HANDLE);
                        ST(L15_DANRYS_GREET_PICK) = 0;
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->modify_var(&ST(V_WEAPON_WARNED), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                }
                /* 0x10EA3 */
                if (ST(L15_PROGRESS_STEP) == 2 && ST(L15_DANRYS_TOPIC_58) == 0) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                    ST(L15_DANRYS_TOPIC_58) = 1;
                    /* 0x10EEE */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0x10F00 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D395", P_ARKHAN2, "SG00DIA06", NULL);
                        }
                        /* 0x10F28 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D396", P_ARKHAN2, "SG00DIA06", NULL);
                        }
                    }
                    /* 0x10F50 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        /* 0x10F62 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D397", P_ARKHAN3, "SG00DIA06", NULL);
                        }
                        /* 0x10F8A */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D398", P_ARKHAN3, "SG00DIA06", NULL);
                        }
                    }
                    lieu_action_register("L15D399A", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    lieu_action_register("L15D399B", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    lieu_action_register("L15D399C", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x11031 */
                if (ST(L15_PROGRESS_STEP) == 5 && ST(L15_DANRYS_TOPIC_58) == 0) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                    ST(L15_DANRYS_TOPIC_58) = 1;
                    /* 0x1107C */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0x1108E */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D400", P_ARKHAN2, "SG00DIA06", NULL);
                        }
                        /* 0x110B6 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D401", P_ARKHAN2, "SG00DIA06", NULL);
                        }
                    }
                    /* 0x110DE */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        /* 0x110F0 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D402", P_ARKHAN3, "SG00DIA06", NULL);
                        }
                        /* 0x11118 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D398", P_ARKHAN3, "SG00DIA06", NULL);
                        }
                    }
                    lieu_action_register("L15D403A", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    lieu_action_register("L15D403B", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x111A6 */
                if (ST(L15_PROGRESS_STEP) == 8 && ST(L15_DANRYS_TOPIC_58) == 0) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                    ST(L15_DANRYS_TOPIC_58) = 1;
                    /* 0x111F1 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D404", P_ARKHAN2, "SG00DIA06", NULL);
                    }
                    /* 0x11219 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D405", P_ARKHAN3, "SG00DIA06", NULL);
                    }
                    lieu_action_register("L15D406A", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    lieu_action_register("L15D406B", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x112A7 */
                if (ST(L15_PROGRESS_STEP) == 10 && ST(L15_DANRYS_TOPIC_58) != 1) {
                    ST(L15_PROGRESS_STEP) = 20;
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_DANRYS_PRE_TALK_DONE) = 1;
                    ST(L15_DANRYS_TOPIC_58) = 1;
                    /* 0x11301 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0x11313 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D902", P_ARKHAN2, "SG00DIA01", NULL);
                        }
                        /* 0x1133B */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D903", P_ARKHAN2, "SG00DIA01", NULL);
                        }
                    }
                    /* 0x11363 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        /* 0x11375 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D904", P_ARKHAN3, "SG00DIA01", NULL);
                        }
                        /* 0x1139D */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D905", P_ARKHAN3, "SG00DIA01", NULL);
                        }
                    }
                    lieu_action_register("L15D906A", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    lieu_action_register("L15D906B", P_DANRYS, "SP09DIA53", "SP09ARR01_10");
                    CB->animation("SP09ARR01_500");
                    lieu_action_register("L15D906C", P_DANRYS, "P09DIA60", "SP09ARR01_10");
                    lieu_action_register("L15D906D", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
                    lieu_action_register("L15D906E", P_DANRYS, "SP09DIA58", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x11481 */
                if (ST(L15_CONVO_AB8) == 0) {
                    ST(L15_CONVO_AB8) = 1;
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    /* 0x114B1 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D55A", P_ARKHAN2, "SG00DIA02", NULL);
                    }
                    /* 0x114D9 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D55B", P_ARKHAN3, "SG00DIA02", NULL);
                    }
                    lieu_action_register("L15D56A", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                    lieu_action_register("L15D56B", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                    lieu_action_register("L15D56C", P_DANRYS, "SP09ATT50", "SP09ARR01_10");
                    /* 0x1154C */
                    if (ST(V_ARMAL_STAGE) == 2) {
                        /* 0x1155E */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D457", P_ARKHAN2, "SG00DIA02", NULL);
                        }
                        /* 0x11586 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D458", P_ARKHAN3, "SG00DIA02", NULL);
                        }
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x115E2 */
                if (ST(V_ARMAL_STAGE) == 0 && ST(ST_VIE(P_ARMAL)) == 1 && ST(L15_ARMAL_STAGE) == 0 && ST(L15_DANRYS_TOPIC_58) != 2) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_DANRYS_TOPIC_58) = 2;
                    /* 0x11642 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D57A", P_ARKHAN2, "SG00DIA07", NULL);
                    }
                    /* 0x1166A */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D57B", P_ARKHAN3, "SG00DIA07", NULL);
                    }
                    lieu_action_register("L15D58A", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D58B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D58C", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D58D", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    /* 0x116F6 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D500", P_ARKHAN2, "SG00DIA07", NULL);
                    }
                    /* 0x1171E */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D501", P_ARKHAN3, "SG00DIA07", NULL);
                    }
                    lieu_action_register("L15D502A", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D502B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D502C", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D502D", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x117DE */
                if (ST(V_ARMAL_STAGE) == 0 && ST(ST_VIE(P_ARMAL)) == 1 && ST(L15_ARMAL_STAGE) == 1) {
                    ST(V_ARMAL_STAGE) = 1;
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    /* 0x11832 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D550A", P_ARKHAN2, "SG00DIA07", NULL);
                        lieu_action_register("L15D550B", P_ARKHAN2, "SG00DIA06", NULL);
                    }
                    /* 0x11870 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D551A", P_ARKHAN3, "SG00DIA07", NULL);
                        lieu_action_register("L15D551B", P_ARKHAN3, "SG00DIA06", NULL);
                    }
                    lieu_action_register("L15D552A", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D552B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D552C", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D552D", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    /* 0x11912 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D553", P_ARKHAN2, "SG00DIA12", NULL);
                    }
                    /* 0x1193A */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D554", P_ARKHAN3, "SG00DIA12", NULL);
                    }
                    lieu_action_register("L15D555A", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    lieu_action_register("L15D555B", P_DANRYS, "SP09DIA01", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x119C8 */
                if (ST(V_GUARD_FIGHT_DONE) != 2) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    /* 0x119E9 */
                    if (ST(L15_CONVO_AA0) == 0 && ST(L15_ROOM_SEEN_E40) == 0) {
                        ST(L15_CONVO_AA0) = 1;
                        /* 0x11A1C */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x11A2E */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D312A", P_ARKHAN2, "SG00DIA05", NULL);
                                lieu_action_register("L15D312B", P_ARKHAN2, "SG00DIA05", NULL);
                            }
                            /* 0x11A6C */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D313A", P_ARKHAN2, "SG00DIA05", NULL);
                                lieu_action_register("L15D313B", P_ARKHAN2, "SG00DIA05", NULL);
                            }
                        }
                        /* 0x11AAA */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0x11ABC */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D314A", P_ARKHAN3, "SG00DIA05", NULL);
                                lieu_action_register("L15D314B", P_ARKHAN3, "SG00DIA05", NULL);
                            }
                            /* 0x11AFA */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D315A", P_ARKHAN3, "SG00DIA05", NULL);
                                lieu_action_register("L15D315B", P_ARKHAN3, "SG00DIA05", NULL);
                            }
                        }
                        lieu_action_register("L15D316A", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        lieu_action_register("L15D316B", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        lieu_action_register("L15D316C", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x11BB7 */
                    if (ST(L15_CONVO_AA0) == 0 && ST(L15_ROOM_SEEN_E40) >= 1) {
                        ST(L15_CONVO_AA0) = 1;
                        ST(L15_CONVO_AB0) = 1;
                        /* 0x11BF9 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D317A", P_ARKHAN2, "SG00DIA05", NULL);
                            lieu_action_register("L15D317B", P_ARKHAN2, "SG00DIA05", NULL);
                        }
                        /* 0x11C37 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D318A", P_ARKHAN3, "SG00DIA05", NULL);
                            lieu_action_register("L15D318B", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L15D319A", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        lieu_action_register("L15D319B", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x11CDB */
                    if (ST(L15_CONVO_AA0) == 1 && ST(L15_ROOM_SEEN_E40) >= 1 && ST(L15_CONVO_AB0) == 0) {
                        ST(L15_CONVO_AB0) = 1;
                        /* 0x11D20 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D320A", P_ARKHAN2, "SG00DIA05", NULL);
                            lieu_action_register("L15D320B", P_ARKHAN2, "SG00DIA05", NULL);
                        }
                        /* 0x11D5E */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D321A", P_ARKHAN3, "SG00DIA05", NULL);
                            lieu_action_register("L15D321B", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L15D319A", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        lieu_action_register("L15D319B", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x11E02 */
                    if (ST(L15_CONVO_AA0) == 1 && ST(L15_STORY_EB0) >= 2 && ST(L15_CONVO_AA4) == 0 && ST(V_EVENT_3) == 0 && ST(V_EVENT_4) == 0) {
                        ST(L15_CONVO_AA4) = 1;
                        /* 0x11E6B */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x11E7D */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D307A", P_ARKHAN2, "SG00DIA05", NULL);
                                lieu_action_register("L15D307B", P_ARKHAN2, "SG00DIA05", NULL);
                                lieu_action_register("L15D307C", P_ARKHAN2, "SG00DIA05", NULL);
                            }
                            /* 0x11ED1 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D308A", P_ARKHAN2, "SG00DIA05", NULL);
                                lieu_action_register("L15D308B", P_ARKHAN2, "SG00DIA05", NULL);
                                lieu_action_register("L15D308C", P_ARKHAN2, "SG00DIA05", NULL);
                            }
                        }
                        /* 0x11F25 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D309A", P_ARKHAN3, "SG00DIA05", NULL);
                            lieu_action_register("L15D309B", P_ARKHAN3, "SG00DIA05", NULL);
                            lieu_action_register("L15D309C", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L15D311A", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                        lieu_action_register("L15D311B", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                        lieu_action_register("L15D311C", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                }
                /* 0x11FF8 */
                if (ST(V_GUARD_FIGHT_DONE) != 0 && ST(L15_FIGHT_DONE_LINES) == 0) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_FIGHT_DONE_LINES) = 1;
                    /* 0x1203A */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D407A", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D407B", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D407C", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D407D", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D407E", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D407F", P_ARKHAN2, "SG00DIA03", NULL);
                    }
                    /* 0x120D0 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D408A", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D408B", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D408C", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D408D", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D408E", P_ARKHAN2, "SG00DIA03", NULL);
                        lieu_action_register("L15D408F", P_ARKHAN2, "SG00DIA03", NULL);
                    }
                    lieu_action_register("L15D409A", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    lieu_action_register("L15D409B", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    lieu_action_register("L15D409C", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    lieu_action_register("L15D409D", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x121FE */
                if (ST(L15_STORY_BCC) != 0 && ST(L15_CONVO_AC0) == 0) {
                    ST(L15_ARKHAN2_TOPIC_90) = 1;
                    ST(L15_CONVO_AC0) = 1;
                    /* 0x12240 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0x12252 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D410", P_ARKHAN2, "SG00DIA03", NULL);
                        }
                        /* 0x1227A */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D411", P_ARKHAN2, "SG00DIA03", NULL);
                        }
                    }
                    /* 0x122A2 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D412", P_ARKHAN3, "SG00DIA03", NULL);
                    }
                    lieu_action_register("L15D413", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    /* 0x122E3 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        /* 0x122F5 */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D414A", P_ARKHAN2, "SG00DIA03", NULL);
                            lieu_action_register("L15D414B", P_ARKHAN2, "SG00DIA03", NULL);
                            lieu_action_register("L15D414C", P_ARKHAN2, "SG00DIA03", NULL);
                            lieu_action_register("L15D414D", P_ARKHAN2, "SG00DIA03", NULL);
                        }
                        /* 0x1235F */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D415A", P_ARKHAN2, "SG00DIA03", NULL);
                            lieu_action_register("L15D415B", P_ARKHAN2, "SG00DIA03", NULL);
                            lieu_action_register("L15D415C", P_ARKHAN2, "SG00DIA03", NULL);
                            lieu_action_register("L15D415D", P_ARKHAN2, "SG00DIA03", NULL);
                        }
                    }
                    /* 0x123C9 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        /* 0x123DB */
                        if (ST(ST_SPECIAL_MODE) == 1) {
                            lieu_action_register("L15D416A", P_ARKHAN3, "SG00DIA03", NULL);
                            lieu_action_register("L15D416B", P_ARKHAN3, "SG00DIA03", NULL);
                            lieu_action_register("L15D416C", P_ARKHAN3, "SG00DIA03", NULL);
                            lieu_action_register("L15D416D", P_ARKHAN3, "SG00DIA03", NULL);
                        }
                        /* 0x12445 */
                        if (ST(ST_SPECIAL_MODE) == 0) {
                            lieu_action_register("L15D417A", P_ARKHAN3, "SG00DIA03", NULL);
                            lieu_action_register("L15D417B", P_ARKHAN3, "SG00DIA03", NULL);
                            lieu_action_register("L15D417C", P_ARKHAN3, "SG00DIA03", NULL);
                            lieu_action_register("L15D417D", P_ARKHAN3, "SG00DIA03", NULL);
                        }
                    }
                    lieu_action_register("L15D418A", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    lieu_action_register("L15D418B", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    lieu_action_register("L15D418C", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    lieu_action_register("L15D418D", P_DANRYS, "SP09DIA61", "SP09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                } else {
                    /* 0x1254C */
                    if (ST(L15_ARKHAN2_TOPIC_90) == 0) {
                        ST(L15_ARKHAN2_TOPIC_90) = 1;
                        /* 0x1256D */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D86A", P_ARKHAN2, "SG00DIA07", NULL);
                        }
                        /* 0x12595 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D86B", P_ARKHAN3, "SG00DIA07", NULL);
                        }
                        lieu_action_register("L15D87", P_DANRYS, "SP09DIA57", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                    /* 0x1260A */
                    if (ST(L15_ARKHAN2_TOPIC_90) == 1) {
                        /* 0x1261C */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D59A", P_ARKHAN2, "SG00DIA04", NULL);
                        }
                        /* 0x12644 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D59B", P_ARKHAN3, "SG00DIA04", NULL);
                        }
                        lieu_action_register("L15D60", P_DANRYS, "SP09DIA56", "SP09ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                        return;
                    }
                }
            }
        }
    }
    /* 0x126B6 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_DANRYS)) != VIE_DEAD && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_perso(P_DANRYS) && ST(L15_TRADE_SCENE_STEP) == 1 && ST(L15_DANRYS_SCENE_STATE) != 0) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->stop_joindre(-P_DANRYS);
        CB->joindre(-ST(ST_HERO), 0, 0, 0, P_DANRYS);
        /* 0x12768 */
        if (ST(L15_TRADE_TYPE) == 1) {
            /* 0x1277A */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D130A", P_ARKHAN2, "SG00DIA08", NULL);
            }
            /* 0x127A2 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D130B", P_ARKHAN3, "SG00DIA08", NULL);
            }
            lieu_action_register("L15D103A", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
        }
        /* 0x127E0 */
        if (ST(L15_TRADE_TYPE) == 0) {
            /* 0x127F2 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D102A", P_ARKHAN2, "SG00DIA08", NULL);
            }
            /* 0x1281A */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D102B", P_ARKHAN3, "SG00DIA08", NULL);
            }
            lieu_action_register("L15D103A", P_DANRYS, "SP09DIA54", "SP09ARR01_10");
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
        return;
    }
    /* 0x1288C */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_DANRYS)) == VIE_DEAD && ST(ST_SCRIPT_BUSY) == 0) {
        /* 0x128CE */
        if (ST(L15_KALHI_STAGE_D40) == 3 && ST(L15_DANRYS_DEAD_SEEN) == 0) {
            ST(ST_SCRIPT_BUSY) = 1;
            ST(L15_KALHI_STAGE_D40) = 2;
            CB->animation("SG00PRE09");
            /* 0x12918 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D113A", P_ARKHAN2, "SG00DIA02", NULL);
            }
            /* 0x12940 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D113B", P_ARKHAN3, "SG00DIA02", NULL);
            }
            /* 0x12968 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D114A", P_ARKHAN2, "SG00DIA08_2000", NULL);
            }
            /* 0x12990 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D114B", P_ARKHAN3, "SG00DIA08_2000", NULL);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->bloque(BLOQUE_NONE);
        } else {
            /* 0x129D6 */
            if (ST(L15_CRYSTAL_ROUND) == 3 && ST(L15_DANRYS_DEAD_SEEN) == 0) {
                CB->divers(DV_GIVE_ITEM, 6, 3);
                CB->divers(DV_PERSONA_ITEMS, P_DANRYS, OB_F_THROWER);
                CB->divers(DV_PERSONA_ITEMS, P_DANRYS, -OB_HANDLE);
            }
            CB->fouille(P_DANRYS);
        }
    }
    /* 0x12A38 */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_COMBAT_STANCE) == 0 && (ST(V_ENGAGED_PERSONA) == P_DANRYS || ST(V_ENGAGED_PERSONA) == 0x391) && ST(ST_VIE(P_DANRYS)) == 1 && CB->direction_perso(P_DANRYS)) {
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x12ABA */
        if (ST(ST_HERO) == P_ARKHAN2) {
            CB->dialogue("L15D95", P_ARKHAN2);
        }
        /* 0x12ADC */
        if (ST(ST_HERO) == P_ARKHAN3) {
            CB->dialogue("L15D96", P_ARKHAN2);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
    /* 0x12B0C */
    if (ST(V_PLAYER_STRIKE) == 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_DANRYS) && ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_DANRYS_SCENE_STATE) != 0) {
        CB->select_pile(0, 2);
        CB->bloque(BLOQUE_ALL);
        ST(L15_DANRYS_APPROACHED) = 1;
        ST(V_PLAYER_STRIKE) = -1;
        CB->stop_joindre(-P_DANRYS);
        ST(V_ENGAGED_PERSONA) = P_DANRYS;
        CB->animation("SP09ESA21");
        CB->animation("P09_65535");
        /* 0x12BD0 */
        if (ST(L15_DANRYS_PLOT_A84) == 0) {
            ST(L15_DANRYS_PLOT_A84) = 1;
            CB->animation("SP09ARR01_10");
            CB->dialogue("L15D76A", P_DANRYS);
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
            CB->select_pile(0, 1);
            return;
        }
        /* 0x12C4D */
        if (ST(L15_DANRYS_PLOT_A84) == 1) {
            ST(L15_DANRYS_PLOT_A84) = 2;
            CB->animation("SP09ARR01_10");
            lieu_action_register("L15D76B", P_DANRYS, "P09DIA60", "SP09GAR03_10");
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
            CB->joindre_perm(P_DANRYS, ST(ST_HERO));
            CB->select_pile(0, 1);
            return;
        }
        /* 0x12CD3 */
        if (ST(L15_DANRYS_PLOT_A84) == 2) {
            ST(L15_DANRYS_PLOT_A84) = 3;
            CB->dialogue("L15D76C", P_DANRYS);
            ST(V_ENGAGED_PERSONA) = P_DANRYS;
            /* 0x12D10 */
            if (ST(L15_PROGRESS_STEP) != 2 && ST(L15_PROGRESS_STEP) != 5 && ST(L15_PROGRESS_STEP) != 8 && ST(L15_PROGRESS_STEP) != 20 && ST(L15_DANRYS_KALHI_FOLLOW) == 1 && ST(ST_VIE(P_KALHI13)) == 1) {
                CB->stop_joindre(-P_KALHI13);
                ST(V_KALHI13_TALK_C) = 3;
                ST(V_ENGAGED_PERSONA) = 0x391;
                lieu_action_register("L15D456", P_KALHI13, "SP13GAR26", "SP13GAR02_10");
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_KALHI13, 0);
            }
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_DANRYS, 0);
            CB->select_pile(0, 1);
            return;
        }
    }
    /* 0x12DF4 */
    if (ST(V_PLAYER_STRIKE) == 2 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && CB->direction_perso(P_DANRYS) && ST(ST_VIE(P_DANRYS)) == 1) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->select_pile(1, 2);
        ST(V_ENGAGED_PERSONA) = P_DANRYS;
        ST(V_PLAYER_STRIKE) = -1;
        CB->stop_joindre(-P_DANRYS);
        CB->combat(P_DANRYS, 0);
        ST(L15_DANRYS_PLOT_A84) = 3;
        ST(V_EVENT_30) = 1;
        CB->evenement(30, 300000);
        /* 0x12EC3 */
        if (ST(L15_PROGRESS_STEP) != 2 && ST(L15_PROGRESS_STEP) != 5 && ST(L15_PROGRESS_STEP) != 8 && ST(L15_PROGRESS_STEP) != 20 && ST(L15_DANRYS_KALHI_FOLLOW) == 1 && ST(ST_VIE(P_KALHI13)) == 1) {
            CB->bloque(BLOQUE_ALL);
            CB->stop_joindre(-P_KALHI13);
            ST(V_KALHI13_TALK_C) = 3;
            ST(V_ENGAGED_PERSONA) = 0x391;
            lieu_action_register("L15D456", P_KALHI13, "SP13GAR26", "SP13GAR02_10");
            CB->bloque(BLOQUE_NONE);
            CB->combat(P_KALHI13, 0);
        }
        CB->select_pile(0, 1);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
    }
}

/* trigger 03 — trig03_10012FA5 — zone 3 */
static void trig03(void) {
    /* 0x12FB4 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xD6D8)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->pos_perso(ST(ST_HERO), 5028, 237, 0xD203, 1, "ar", 6, 0);
        /* 0x13039 */
        if (ST(L15_FLAG_BF4) == 0) {
            /* 0x1304B */
            if (ST(L15_DANRYS_INTERRUPTIBLE) == 1) {
                ST(L15_DANRYS_INTERRUPTIBLE) = 0;
                ST(L15_CAMERA3_ENTERED) = 1;
                /* 0x13083 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0x13095 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT39");
                    }
                    /* 0x130B2 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT39");
                    }
                }
                /* 0x130D7 */
                if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0x130E9 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT20");
                    }
                    /* 0x13106 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT20");
                    }
                }
                /* 0x13123 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("P02_65535");
                }
                /* 0x13140 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("P03_65535");
                }
                CB->pause_anim(8, 0, -1);
                CB->pause_anim(5, 0, -1);
                CB->bruitage("L15B01", 5020, 235);
                /* 0x13190 */
                if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
                    CB->bruitage("L15AMB1", -1, -1);
                }
                /* 0x131B4 */
                if (ST(L15_DANRYS_ENDING_FLAG) != 0) {
                    CB->bruitage("L15AMB3", -1, -1);
                }
                /* 0x131D8 */
                if (ST(L15_ANIM_PHASE) == 1 && ST(ST_VIE(P_DANRYS)) == 1) {
                    ST(L15_ANIM_PHASE) = 0;
                    CB->animation("SP09DIA01_10");
                    CB->free_anim(2);
                    CB->evenement(1, 2000);
                    return;
                }
                /* 0x13236 */
                if (ST(L15_ANIM_PHASE) == 2) {
                    ST(L15_ANIM_PHASE) = 0;
                }
                CB->free_anim(2);
                CB->pause_anim(7, 0, 0);
                CB->animation("ar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x1329A */
            if (ST(L15_DANRYS_INTERRUPTIBLE) == 0) {
                ST(L15_DANRYS_INTERRUPTIBLE) = 1;
                ST(L15_CAMERA3_ENTERED) = 1;
                /* 0x132CA */
                if (ST(ST_PLAYER_WEAPON) != 0 && ST(L15_TRADE_TYPE) == 1) {
                    /* 0x132EB */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02PRE73");
                        CB->animation("P02_65535");
                    }
                    /* 0x13313 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03PRE73");
                        CB->animation("P03_65535");
                    }
                }
                /* 0x13343 */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0x13355 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT40");
                    }
                    /* 0x13372 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT40");
                    }
                }
                /* 0x13397 */
                if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0x133A9 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT21");
                    }
                    /* 0x133C6 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT21");
                    }
                }
                /* 0x133E3 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("P02_65535");
                }
                /* 0x13400 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("P03_65535");
                }
                CB->pause_anim(7, 0, -1);
                CB->bruitage("L15B02", 5020, 235);
                /* 0x13441 */
                if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
                    CB->bruitage("L15AMB2", -1, -1);
                }
                /* 0x13465 */
                if (ST(L15_DANRYS_ENDING_FLAG) != 0) {
                    CB->bruitage("L15AMB4", -1, -1);
                }
                /* 0x13489 */
                if (ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_TRADE_SCENE_STEP) != 1) {
                    CB->stop_joindre(-P_DANRYS);
                    CB->change_camera(0, CAM_LOCKED);
                    CB->pause_anim(8, 0, -1);
                    CB->load_anim("215MEU01", 0);
                    CB->play_anim(0, 1);
                    lieu_action_register("L15D100A", P_DANRYS, "SP09DIA03", "P09ARR01_10");
                    lieu_action_register("L15D100B", P_DANRYS, "SP09DIA03", "P09ARR01_10");
                    CB->change_camera(3, CAM_AUTO);
                    CB->free_anim(0);
                    CB->modify_var(&ST(L15_TRADE_ANIM_CAM), 0);
                    CB->load_anim("215MEU03", 2);
                    CB->play_anim(2, 1);
                    ST(L15_DANRYS_INTERRUPTIBLE) = 0;
                    /* 0x13583 */
                    if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0x13595 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            CB->animation("SP02ACT39");
                        }
                        /* 0x135B2 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            CB->animation("SP03ACT39");
                        }
                    }
                    /* 0x135D7 */
                    if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                        /* 0x135E9 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            CB->animation("SP02ACT20");
                        }
                        /* 0x13606 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            CB->animation("SP03ACT20");
                        }
                    }
                    /* 0x13623 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("P02_65535");
                    }
                    /* 0x13640 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("P03_65535");
                    }
                    CB->pause_anim(8, 5, 0);
                    CB->bruitage("L15B01", 5043, 250);
                    /* 0x13684 */
                    if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
                        CB->bruitage("L15AMB1", -1, -1);
                    }
                    /* 0x136A8 */
                    if (ST(L15_DANRYS_ENDING_FLAG) != 0) {
                        CB->bruitage("L15AMB3", -1, -1);
                    }
                    CB->free_anim(2);
                    CB->animation("ar_10");
                    CB->animation("P09ARR01_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->modify_var(&ST(L15_ANIM_PHASE), 0);
                    CB->bloque(BLOQUE_NONE);
                    CB->joindre_perm(P_DANRYS, ST(ST_HERO));
                    return;
                }
                /* 0x13735 */
                if (ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_TRADE_SCENE_STEP) == 1) {
                    ST(L15_DANRYS_APPROACHED) = 1;
                    /* 0x13768 */
                    if (ST(L15_TRADE_REWARD_PENDING) == 1) {
                        CB->link_objet(9, "OB45", 0x1B, 1);
                    }
                    CB->change_camera(1, CAM_LOCKED);
                    CB->modify_var(&ST(L15_TRADE_SCENE_STEP), 2);
                }
                /* 0x137B5 */
                if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
                    ST(L15_TRADE_ANIM_CAM) = 0;
                    ST(L15_ANIM_PHASE) = 2;
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0x1380E */
        if (ST(L15_FLAG_BF4) == 1) {
            /* 0x13828 */
            if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0x1383A */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02ACT40");
                }
                /* 0x13857 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03ACT40");
                }
            }
            /* 0x1387C */
            if (!CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0x1388E */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02ACT21");
                }
                /* 0x138AB */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03ACT21");
                }
            }
            /* 0x138C8 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D461", P_ARKHAN2, "SG00ATT01", NULL);
            }
            /* 0x138F0 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D462", P_ARKHAN3, "SG00ATT01", NULL);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("Sar_10");
            CB->bloque(BLOQUE_NONE);
            return;
        }
    }
}

/* trigger 04 — trig04_1001393A — zone 4 */
static void trig04(void) {
    /* 0x13949 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x7080) && ST(ST_VIE(P_DANRYS)) == 1 && (ST(L15_ROOM_PART) == 0 || ST(V_EVENT_29) == 4)) {
        ST(ST_SCRIPT_BUSY) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x139D9 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D506", P_ARKHAN2, "SG00DIA06", NULL);
        }
        /* 0x13A01 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D507", P_ARKHAN3, "SG00DIA06", NULL);
        }
        CB->bloque(BLOQUE_NONE);
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        return;
    }
    /* 0x13A48 */
    if (CB->direction_spec(0x7530) && ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x13AB0 */
        if (ST(L15_ANIM_PHASE) == 0) {
            /* 0x13AC2 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("L15D112A", P_ARKHAN2, "SG00ATT01", NULL);
            }
            /* 0x13AEA */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("L15D112B", P_ARKHAN3, "SG00ATT01", NULL);
            }
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            CB->animation("Sar_10");
            CB->bloque(BLOQUE_NONE);
            return;
        }
        /* 0x13B3B */
        if (ST(L15_ANIM_PHASE) == 2) {
            /* 0x13B4D */
            if (ST(ST_PLAYER_WEAPON) == 0) {
                /* 0x13B5F */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    /* 0x13B71 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D80A", P_ARKHAN2, "SG00DIA04", NULL);
                    }
                    /* 0x13B99 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D80B", P_ARKHAN3, "SG00DIA04", NULL);
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->animation("Sar_10");
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0x13BEA */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    /* 0x13BFC */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT29");
                    }
                    /* 0x13C19 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT29");
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->animation("Sar_10");
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
            /* 0x13C67 */
            if (CB->divers(DV_CAN_FIGHT, 8, 2)) {
                /* 0x13C79 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->dialogue("L15D81A", P_ARKHAN2);
                    CB->dialogue("L15D81B", P_ARKHAN2);
                }
                /* 0x13CAB */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->dialogue("L15D81C", P_ARKHAN3);
                    CB->dialogue("L15D81D", P_ARKHAN3);
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->animation("Sar_10");
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x13D06 */
            if (ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_2 || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_1 || ST(ST_PLAYER_WEAPON) == OB_CRYSTAL_3) {
                /* 0x13D3C */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L15D82A", P_ARKHAN2, "SG00DIA06", NULL);
                }
                /* 0x13D64 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L15D82B", P_ARKHAN3, "SG00DIA06", NULL);
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x13DAA */
            if (ST(ST_PLAYER_WEAPON) == OB_CRYST_SLIV) {
                /* 0x13DBC */
                if (ST(V_CRYSTAL_PROGRESS) <= 1) {
                    /* 0x13DCE */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D465A", P_ARKHAN2, "SG00DIA06", NULL);
                        lieu_action_register("L15D465B", P_ARKHAN2, "SG00DIA06", NULL);
                    }
                    /* 0x13E0C */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D466A", P_ARKHAN3, "SG00DIA06", NULL);
                        lieu_action_register("L15D466B", P_ARKHAN2, "SG00DIA06", NULL);
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0x13E68 */
                if (ST(V_CRYSTAL_PROGRESS) == 2) {
                    /* 0x13E7A */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D419", P_ARKHAN2, "SG00DIA06", NULL);
                        CB->animation("SP02ACT23");
                    }
                    /* 0x13EAD */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D420", P_ARKHAN3, "SG00DIA06", NULL);
                        CB->animation("SP03ACT23");
                    }
                    CB->free_anim(1);
                    CB->play_anim(4, 1);
                    CB->evenement(0, 9000);
                    return;
                }
                /* 0x13F0D */
                if (ST(V_CRYSTAL_PROGRESS) >= 3) {
                    /* 0x13F1F */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("L15D89A", P_ARKHAN2, "SG00DIA06", NULL);
                    }
                    /* 0x13F47 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("L15D89B", P_ARKHAN3, "SG00DIA06", NULL);
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            } else {
                /* 0x13F92 */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    lieu_action_register("L15D85A", P_ARKHAN2, "SG00DIA06", NULL);
                }
                /* 0x13FBA */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    lieu_action_register("L15D85B", P_ARKHAN3, "SG00DIA06", NULL);
                }
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
        }
    }
    CB->bloque(BLOQUE_NONE);
    /* 0x14009 */
    if (CB->direction_spec(0x2EE0) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 13)) {
        trig13();
    }
}

/* trigger 05 — trig05_10014039 — zone 5 */
static void trig05(void) {
    /* 0x14048 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x36B0)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        CB->joindre(-ST(ST_HERO), 0, 0, 0, -16);
        /* 0x140BF */
        if (ST(L15_FLAG_BF4) == 0) {
            /* 0x140D1 */
            if (ST(L15_DANRYS_ENDING_FLAG) == 1) {
                CB->modify_var(&ST(L15_DANRYS_ENDING_FLAG), 0);
                /* 0x140FF */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0x14111 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT44");
                    }
                    /* 0x1412E */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT44");
                    }
                } else {
                    /* 0x14150 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT43");
                    }
                    /* 0x1416D */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT43");
                    }
                }
                CB->etat_objet("OBCOMP", 5050, -2253, 110, 0, 0, 0x4000, 0, 4);
                CB->modify_var(&ST(L15_LEVER_CAM_STEP), 0);
                CB->free_anim(6);
                /* 0x141CD */
                if (ST(L15_ANIM_PHASE) == 0) {
                    CB->bruitage("L15AMB1", -1, -1);
                }
                /* 0x141F1 */
                if (ST(L15_ANIM_PHASE) != 0) {
                    CB->bruitage("L15AMB2", -1, -1);
                }
                CB->animation("Sar_10");
                CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                CB->bloque(BLOQUE_NONE);
                return;
            }
            /* 0x1423E */
            if (ST(L15_DANRYS_ENDING_FLAG) == 0) {
                CB->modify_var(&ST(L15_DANRYS_ENDING_FLAG), 1);
                CB->load_anim("215MOT01", 6);
                /* 0x1427F */
                if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                    /* 0x14291 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT42");
                    }
                    /* 0x142AE */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT42");
                    }
                } else {
                    /* 0x142D0 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT41");
                    }
                    /* 0x142ED */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT41");
                    }
                }
                CB->etat_objet("OBCOMP", 5050, -2253, 110, 0xF600, 0, 0x4000, 0, 4);
                CB->modify_var(&ST(L15_LEVER_CAM_STEP), 1);
                CB->play_anim(6, 1);
                /* 0x14358 */
                if (ST(L15_ANIM_PHASE) == 0) {
                    CB->bruitage("L15AMB3", -1, -1);
                }
                /* 0x1437C */
                if (ST(L15_ANIM_PHASE) != 0) {
                    CB->bruitage("L15AMB4", -1, -1);
                }
                /* 0x143A0 */
                if (ST(ST_PLAYER_WEAPON) != OB_FLASK86) {
                    /* 0x143B2 */
                    if (ST(L15_CONVO_C00) == 0) {
                        /* 0x143C4 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D104A", P_ARKHAN2, "SG00ATT01", NULL);
                        }
                        /* 0x143EC */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D104B", P_ARKHAN3, "SG00ATT01", NULL);
                        }
                    }
                    /* 0x14414 */
                    if (ST(L15_CONVO_C00) != 0) {
                        /* 0x14426 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D105A", P_ARKHAN2, "SG00ATT01", NULL);
                        }
                        /* 0x1444E */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D105B", P_ARKHAN3, "SG00ATT01", NULL);
                        }
                    }
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
                /* 0x14494 */
                if (ST(ST_PLAYER_WEAPON) == OB_FLASK86) {
                    ST(V_HUD_TIMER_MS) = 1200000;
                    /* 0x144B5 */
                    if (ST(L15_ONCE_C50) == 0) {
                        CB->modify_var(&ST(L15_ONCE_C50), 1);
                    }
                    /* 0x144DE */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->animation("SP02ACT45");
                    }
                    /* 0x144FB */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->animation("SP03ACT45");
                    }
                    CB->animation("P00_65535");
                    /* 0x14523 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        CB->dialogue("L15D421", P_ARKHAN2);
                    }
                    /* 0x14545 */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        CB->dialogue("L15D422", P_ARKHAN3);
                    }
                    CB->animation("Sar_10");
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->bloque(BLOQUE_NONE);
                    return;
                }
            }
        }
        /* 0x14590 */
        if (ST(L15_FLAG_BF4) == 1) {
            /* 0x145AA */
            if (CB->divers(DV_CAN_FIGHT, 7, 2)) {
                /* 0x145BC */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02ACT44");
                }
                /* 0x145D9 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03ACT44");
                }
            } else {
                /* 0x145FB */
                if (ST(ST_HERO) == P_ARKHAN2) {
                    CB->animation("SP02ACT43");
                }
                /* 0x14618 */
                if (ST(ST_HERO) == P_ARKHAN3) {
                    CB->animation("SP03ACT43");
                }
            }
            /* 0x14635 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("LCOD39", P_ARKHAN2, "G00ATT01", NULL);
            }
            /* 0x1465D */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("LCOD40", P_ARKHAN3, "G00ATT01", NULL);
            }
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 07 — trig07_100146B2 — zone 7 */
static void trig07(void) {
    /* 0x146C5 */
    if (CB->direction_spec(0x3A98) && ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(V_DLG_RANDOM) = CB->random(3);
        /* 0x14732 */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->animation("SG00PRE01");
        }
        /* 0x1474F */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->animation("SG00PRE02");
        }
        /* 0x1476C */
        if (ST(V_DLG_RANDOM) == 2) {
            CB->animation("SG00PRE03");
        }
        /* 0x14789 */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD76", P_ARKHAN2, "G00DIA01", NULL);
        }
        /* 0x147B1 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD77", P_ARKHAN3, "G00DIA01", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
    /* 0x14803 */
    if (CB->direction_spec(0xABE0) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 14)) {
        trig14();
    }
}

/* trigger 08 — trig08_10014833 — zone 8 */
static void trig08(void) {
    /* 0x14842 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x9088)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        ST(V_DLG_RANDOM) = CB->random(3);
        /* 0x148B3 */
        if (ST(V_DLG_RANDOM) == 0) {
            CB->animation("SG00PRE01");
        }
        /* 0x148D0 */
        if (ST(V_DLG_RANDOM) == 1) {
            CB->animation("SG00PRE02");
        }
        /* 0x148ED */
        if (ST(V_DLG_RANDOM) == 2) {
            CB->animation("SG00PRE03");
        }
        /* 0x1490A */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("LCOD76", P_ARKHAN2, "G00DIA01", NULL);
        }
        /* 0x14932 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("LCOD77", P_ARKHAN3, "G00DIA01", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 09 — trig09_1001497C — zone 9 */
static void trig09(void) {
    /* 0x1498B */
    if (ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xC738) && ST(ST_COMBAT_STANCE) == 0) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x149E5 */
        if (ST(L15_TRIG9_SEEN) == 1) {
            ST(V_DLG_RANDOM) = CB->random(2);
            /* 0x14A0E */
            if (ST(V_DLG_RANDOM) == 0) {
                CB->animation("SG00PRE01");
            }
            /* 0x14A2B */
            if (ST(V_DLG_RANDOM) == 1) {
                CB->animation("SG00PRE02");
            }
            /* 0x14A48 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                lieu_action_register("LCOD76", P_ARKHAN2, "G00DIA01", NULL);
            }
            /* 0x14A70 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                lieu_action_register("LCOD77", P_ARKHAN3, "G00DIA01", NULL);
            }
        }
        /* 0x14A98 */
        if (ST(L15_TRIG9_SEEN) == 0) {
            ST(L15_TRIG9_SEEN) = 1;
            CB->animation("SG00PRE06");
            CB->etat_objet("OB74B", -1, -1, -1, -1, -1, -1, -3, 0);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 10 — trig10_10014B04 — Konkalite 9 dialogue / search */
static void trig10(void) {
    /* 0x14B16 */
    if (ST(ST_VIE(P_KONKALITE9)) < 2 && CB->direction_perso(P_KONKALITE9) && ST(ST_ACTION_KEY) == 1 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_perso(P_KONKALITE9)) {
        /* 0x14B6C */
        if (ST(L15_KONKALITE9_TALK) == 0) {
            ST(L15_KONKALITE9_TALK) = 1;
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x14B99 */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L15D91A", P_ARKHAN2);
            }
            /* 0x14BBB */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L15D91B", P_ARKHAN3);
            }
            CB->dialogue("L15D92", P_KONKALITE9);
            return;
        }
        /* 0x14BF2 */
        if (ST(L15_KONKALITE9_TALK) == 1) {
            CB->bloque(BLOQUE_ALL);
            ST(ST_SCRIPT_BUSY) = 1;
            /* 0x14C1B */
            if (ST(ST_HERO) == P_ARKHAN2) {
                CB->dialogue("L15D94A", P_ARKHAN2);
            }
            /* 0x14C3D */
            if (ST(ST_HERO) == P_ARKHAN3) {
                CB->dialogue("L15D94B", P_ARKHAN3);
            }
            CB->bloque(BLOQUE_NONE);
            CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
            return;
        }
    }
    /* 0x14C7D */
    if (ST(ST_VIE(P_KONKALITE9)) == VIE_DEAD && CB->direction_perso(P_KONKALITE9) && ST(ST_ACTION_KEY) == 1) {
        CB->fouille(P_KONKALITE9);
    }
    /* 0x14CBC */
    if (ST(ST_VIE(P_KONKALITE9)) == VIE_DEAD && CB->direction_perso(P_KALHI13) && CB->divers(DV_IN_ZONE, ST(ST_HERO), 12)) {
        trig12();
    }
}

/* trigger 11 — trig11_10014CFF — zone 11 */
static void trig11(void) {
    /* 0x14D0E */
    if (ST(ST_UPDOWN_KEY) == 1 && ST(ST_VIE(P_KONKALITE9)) == 1) {
        CB->bloque(BLOQUE_ALL);
        ST(L15_KONKALITE9_ENGAGED) = 1;
        CB->evenement(2, -1);
        CB->retire_zone(11);
        CB->animation("SP79ATT02");
        CB->pos_perso(P_KONKALITE9, 5012, 1050, 0x8CA0, 1, "P79GAR03", 0xA, 0);
        CB->modify_var(&ST(L15_KONKALITE9_STATE), 1);
    }
}

/* trigger 12 — trig12_10014DA4 — Kalhi 13 dialogues and fight start */
static void trig12(void) {
    /* 0x14DB7 */
    if (CB->direction_perso(P_KALHI13)) {
        /* 0x14DC6 */
        if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0) {
            /* 0x14DF6 */
            if (ST(ST_VIE(P_KALHI13)) == 1) {
                ST(ST_SCRIPT_BUSY) = 1;
                CB->bloque(BLOQUE_ALL);
                CB->stop_joindre(-P_KALHI13);
                CB->joindre(-ST(ST_HERO), 0, 0, 0, P_KALHI13);
                CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
                CB->animation("P13ARR01_10");
                /* 0x14E6F */
                if (ST(ST_PLAYER_WEAPON) == 0 || CB->divers(DV_CAN_FIGHT, 0, 2) || (ST(ST_PLAYER_WEAPON) != 0 && ST(ST_PLAYER_WEAPON) == ST(L15_LAST_WEAPON_KALHI))) {
                    /* 0x14EC7 */
                    if (ST(L15_KALHI_TALK_STAGE) == 0) {
                        ST(L15_KALHI_TALK_STAGE) = 1;
                        /* 0x14EE8 */
                        if (ST(ST_VIE(P_DANRYS)) == 1) {
                            /* 0x14EFA */
                            if (ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L15D425", P_ARKHAN2, "SG00DIA06", NULL);
                            }
                            /* 0x14F22 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D426", P_ARKHAN3, "SG00DIA06", NULL);
                            }
                            lieu_action_register("L15D427", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                        }
                        /* 0x14F63 */
                        if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
                            /* 0x14F75 */
                            if (ST(L15_DANRYS_DEAD_SEEN) == 0 && ST(ST_HERO) == P_ARKHAN2) {
                                /* 0x14F99 */
                                if (ST(ST_SPECIAL_MODE) == 1) {
                                    lieu_action_register("L15D428", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                                /* 0x14FC1 */
                                if (ST(ST_SPECIAL_MODE) == 0) {
                                    lieu_action_register("L15D429", P_ARKHAN2, "SG00DIA04", NULL);
                                }
                            }
                            /* 0x14FE9 */
                            if (ST(L15_DANRYS_DEAD_SEEN) == 1 && ST(ST_HERO) == P_ARKHAN2) {
                                lieu_action_register("L15D429", P_ARKHAN2, "SG00DIA04", NULL);
                            }
                            /* 0x15023 */
                            if (ST(ST_HERO) == P_ARKHAN3) {
                                lieu_action_register("L15D430", P_ARKHAN3, "SG00DIA04", NULL);
                            }
                            lieu_action_register("L15D431", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                        }
                        /* 0x15064 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D432", P_ARKHAN2, "SG00DIA03", NULL);
                        }
                        /* 0x1508C */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D433", P_ARKHAN3, "SG00DIA03", NULL);
                        }
                        /* 0x150B4 */
                        if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
                            lieu_action_register("L15D427", P_KALHI13, "SP13DIA57", "SP13ARR01_10");
                        }
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                        return;
                    }
                    /* 0x15113 */
                    if (ST(L15_KALHI_TALK_STAGE) == 1 && ST(L15_PROGRESS_STEP) != 3 && ST(L15_PROGRESS_STEP) != 6 && ST(L15_PROGRESS_STEP) != 9) {
                        ST(L15_KALHI_TALK_STAGE) = 2;
                        /* 0x1516A */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x1517C */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D434", P_ARKHAN2, "SG00DIA02", NULL);
                            }
                            /* 0x151A4 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D435", P_ARKHAN2, "SG00DIA02", NULL);
                            }
                        }
                        /* 0x151CC */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D436", P_ARKHAN3, "SG00DIA02", NULL);
                        }
                        lieu_action_register("L15D437", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                        /* 0x1520D */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D438", P_ARKHAN2, "SG00ATT01", NULL);
                        }
                        /* 0x15235 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D439", P_ARKHAN3, "SG00ATT01", NULL);
                        }
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                        return;
                    }
                    /* 0x15291 */
                    if (ST(L15_KALHI_TALK_STAGE) == 1 && (ST(L15_PROGRESS_STEP) == 3 || ST(L15_PROGRESS_STEP) == 6 || ST(L15_PROGRESS_STEP) == 9)) {
                        ST(L15_KALHI_TALK_STAGE) = 2;
                        /* 0x152E8 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x152FA */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D440", P_ARKHAN2, "SG00DIA06", NULL);
                            }
                            /* 0x15322 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D441A", P_ARKHAN2, "SG00DIA06", NULL);
                                lieu_action_register("L15D441B", P_ARKHAN2, "SG00DIA06", NULL);
                            }
                        }
                        /* 0x15360 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0x15372 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D472", P_ARKHAN3, "SG00DIA06", NULL);
                            }
                            /* 0x1539A */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D473", P_ARKHAN3, "SG00DIA06", NULL);
                            }
                        }
                        /* 0x153C2 */
                        if (ST(L15_PROGRESS_STEP) == 3) {
                            ST(L15_PROGRESS_STEP) = 4;
                            lieu_action_register("L15D467", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                        }
                        /* 0x153FC */
                        if (ST(L15_PROGRESS_STEP) == 6) {
                            ST(L15_PROGRESS_STEP) = 7;
                            lieu_action_register("L15D443A", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                            lieu_action_register("L15D443B", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                        }
                        /* 0x1544F */
                        if (ST(L15_PROGRESS_STEP) == 9) {
                            lieu_action_register("L15D444A", P_KALHI13, "SP13DIA58", "SP13ARR01_10");
                            lieu_action_register("L15D444B", P_KALHI13, "SP13DIA58", "SP13ARR01_10");
                        }
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                        return;
                    }
                    /* 0x154C7 */
                    if (ST(L15_KALHI_TALK_STAGE) == 2 && ST(L15_PROGRESS_STEP) != 3 && ST(L15_PROGRESS_STEP) != 6 && ST(L15_PROGRESS_STEP) != 9) {
                        /* 0x1550F */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x15521 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D445", P_ARKHAN2, "SG00DIA07", NULL);
                            }
                            /* 0x15549 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D446", P_ARKHAN2, "SG00DIA07", NULL);
                            }
                        }
                        /* 0x15571 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0x15583 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D447", P_ARKHAN3, "SG00DIA07", NULL);
                            }
                            /* 0x155AB */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D448", P_ARKHAN3, "SG00DIA07", NULL);
                            }
                        }
                        lieu_action_register("L15D449A", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
                        lieu_action_register("L15D449B", P_KALHI13, "SP13DIA56", "SP13ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                        return;
                    }
                    /* 0x15639 */
                    if (ST(L15_KALHI_TALK_STAGE) == 2 && (ST(L15_PROGRESS_STEP) == 3 || ST(L15_PROGRESS_STEP) == 6 || ST(L15_PROGRESS_STEP) == 9)) {
                        /* 0x15681 */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            /* 0x15693 */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D440", P_ARKHAN2, "SG00DIA06", NULL);
                            }
                            /* 0x156BB */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D441A", P_ARKHAN2, "SG00DIA06", NULL);
                                lieu_action_register("L15D441B", P_ARKHAN2, "SG00DIA06", NULL);
                            }
                        }
                        /* 0x156F9 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            /* 0x1570B */
                            if (ST(ST_SPECIAL_MODE) == 1) {
                                lieu_action_register("L15D472", P_ARKHAN3, "SG00DIA06", NULL);
                            }
                            /* 0x15733 */
                            if (ST(ST_SPECIAL_MODE) == 0) {
                                lieu_action_register("L15D473", P_ARKHAN3, "SG00DIA06", NULL);
                            }
                        }
                        /* 0x1575B */
                        if (ST(L15_PROGRESS_STEP) == 3) {
                            ST(L15_PROGRESS_STEP) = 4;
                            lieu_action_register("L15D467", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
                        }
                        /* 0x15795 */
                        if (ST(L15_PROGRESS_STEP) == 6) {
                            ST(L15_PROGRESS_STEP) = 7;
                            lieu_action_register("L15D443A", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                            lieu_action_register("L15D443B", P_KALHI13, "SP13DIA59", "SP13ARR01_10");
                        }
                        /* 0x157E8 */
                        if (ST(L15_PROGRESS_STEP) == 9) {
                            lieu_action_register("L15D444A", P_KALHI13, "SP13DIA58", "SP13ARR01_10");
                            lieu_action_register("L15D444B", P_KALHI13, "SP13DIA58", "SP13ARR01_10");
                        }
                        /* 0x1582C */
                        if (ST(ST_HERO) == P_ARKHAN2) {
                            lieu_action_register("L15D450", P_ARKHAN2, "SG00DIA05", NULL);
                        }
                        /* 0x15854 */
                        if (ST(ST_HERO) == P_ARKHAN3) {
                            lieu_action_register("L15D451", P_ARKHAN3, "SG00DIA05", NULL);
                        }
                        lieu_action_register("L15D452", P_KALHI13, "SP13DIA50", "SP13ARR01_10");
                        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                        CB->bloque(BLOQUE_NONE);
                        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                        return;
                    }
                }
                /* 0x158C9 */
                if (ST(ST_PLAYER_WEAPON) != 0 && !CB->divers(DV_CAN_FIGHT, 0, 2) && ST(ST_PLAYER_WEAPON) != ST(L15_LAST_WEAPON_KALHI)) {
                    ST(V_LAST_WEAPON) = ST(ST_PLAYER_WEAPON);
                    /* 0x15923 */
                    if (ST(ST_HERO) == P_ARKHAN2) {
                        lieu_action_register("LCOD18", P_ARKHAN2, "SG00PRE80", "SG00PRE81_10");
                    }
                    /* 0x1594E */
                    if (ST(ST_HERO) == P_ARKHAN3) {
                        lieu_action_register("LCOD19", P_ARKHAN3, "SG00PRE80", "SG00PRE81_10");
                    }
                    ST(V_DLG_RANDOM) = CB->random(3);
                    /* 0x15990 */
                    if (ST(V_DLG_RANDOM) == 0 && ST(ST_VIE(P_DANRYS)) != VIE_DEAD) {
                        lieu_action_register("L15D453", P_KALHI13, "SP13ATT60", "SP13ARR01_10");
                    }
                    /* 0x159CD */
                    if (ST(V_DLG_RANDOM) == 1 && ST(ST_VIE(P_DANRYS)) != VIE_DEAD) {
                        lieu_action_register("L15D454", P_KALHI13, "SP13ATT61", "SP13ARR01_10");
                    }
                    /* 0x15A0A */
                    if (ST(V_DLG_RANDOM) == 2 || ST(ST_VIE(P_DANRYS)) == VIE_DEAD) {
                        lieu_action_register("L15D455", P_KALHI13, "SP13ATT62", "SP13ARR01_10");
                    }
                    CB->animation("SG00PRE82_200");
                    CB->animation("ar_10");
                    CB->bloque(BLOQUE_NONE);
                    CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
                    CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                    return;
                }
            }
            /* 0x15A91 */
            if (ST(ST_VIE(P_KALHI13)) == VIE_DEAD) {
                CB->fouille(P_KALHI13);
            }
        }
        /* 0x15AAB */
        if (ST(ST_ACTION_KEY) == 1 && ST(ST_COMBAT_STANCE) == 0 && ST(ST_VIE(P_KALHI13)) == 1 && (ST(V_ENGAGED_PERSONA) == P_KALHI13 || ST(V_ENGAGED_PERSONA) == 0x391) && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_perso(P_KALHI13)) {
            CB->bloque(BLOQUE_ALL);
            /* 0x15B2C */
            if (ST(ST_HERO) == P_ARKHAN2) {
                /* 0x15B3E */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->dialogue("L04D37", P_ARKHAN2);
                }
                /* 0x15B60 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->dialogue("L04D38", P_ARKHAN2);
                }
            }
            /* 0x15B82 */
            if (ST(ST_HERO) == P_ARKHAN3) {
                /* 0x15B94 */
                if (ST(ST_SPECIAL_MODE) == 0) {
                    CB->dialogue("L04D39", P_ARKHAN3);
                }
                /* 0x15BB6 */
                if (ST(ST_SPECIAL_MODE) == 1) {
                    CB->dialogue("L04D40", P_ARKHAN3);
                }
            }
            CB->bloque(BLOQUE_NONE);
        }
        /* 0x15BDD */
        if (ST(V_PLAYER_STRIKE) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI13)) == 1) {
            CB->select_pile(0, 2);
            CB->bloque(BLOQUE_ALL);
            ST(V_ENGAGED_PERSONA) = P_KALHI13;
            CB->stop_joindre(-P_KALHI13);
            ST(V_PLAYER_STRIKE) = -1;
            CB->animation("SP13ESA15");
            CB->animation("P13_65535");
            /* 0x15C61 */
            if (ST(V_KALHI13_TALK_C) == 0) {
                ST(V_KALHI13_TALK_C) = 1;
                lieu_action_register("L04D41", P_KALHI13, "P13DIA59_10", "SP13ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
                CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                CB->select_pile(0, 1);
                return;
            }
            /* 0x15CDC */
            if (ST(V_KALHI13_TALK_C) == 1) {
                ST(V_KALHI13_TALK_C) = 2;
                lieu_action_register("L04D42", P_KALHI13, "P13DIA59_10", "SP13ARR01_10");
                CB->bloque(BLOQUE_NONE);
                CB->modify_var(&ST(V_ENGAGED_PERSONA), 0);
                CB->joindre_perm(P_KALHI13, ST(ST_HERO));
                CB->select_pile(0, 1);
                return;
            }
            /* 0x15D57 */
            if (ST(V_KALHI13_TALK_C) == 2) {
                /* 0x15D69 */
                if (ST(L15_PROGRESS_STEP) == 0) {
                    ST(L15_PROGRESS_STEP) = 1;
                    ST(L15_DANRYS_INTRO_STAGE) = 2;
                    ST(L15_ROOM_ENTERED) = 1;
                }
                lieu_action_register("L04D43", P_KALHI13, "P13GAR26_10", "SP13GAR02_10");
                ST(V_ENGAGED_PERSONA) = P_KALHI13;
                /* 0x15DCD */
                if (ST(ST_VIE(P_DANRYS)) == 1) {
                    /* 0x15DDF */
                    if (ST(L15_ROOM_PART) == 1 && ST(L15_DANRYS_SCENE_STATE) == 0) {
                        ST(L15_ANIM_PHASE) = 2;
                        ST(L15_TRADE_ANIM_CAM) = 0;
                    }
                    ST(L15_DANRYS_PLOT_A84) = 3;
                    CB->stop_joindre(-P_DANRYS);
                    ST(V_ENGAGED_PERSONA) = 0x391;
                    CB->dialogue("L15D76C", P_DANRYS);
                    CB->bloque(BLOQUE_NONE);
                    CB->combat(P_DANRYS, 0);
                }
                CB->bloque(BLOQUE_NONE);
                CB->combat(P_KALHI13, 0);
                CB->select_pile(0, 1);
                return;
            }
        }
    } else if (CB->divers(DV_IN_ZONE, ST(ST_HERO), 2)) {
        trig02();
    }
    /* 0x15EB7 */
    if (ST(V_PLAYER_STRIKE) > 1 && ST(ST_SCRIPT_BUSY) == 0 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_VIE(P_KALHI13)) == 1 && CB->direction_perso(P_KALHI13)) {
        CB->select_pile(0, 2);
        ST(V_ENGAGED_PERSONA) = P_KALHI13;
        CB->stop_joindre(-P_KALHI13);
        CB->combat(P_KALHI13, 0);
        CB->divers_empile(DV_BLOOD_AT_BONE, P_DELIA, 0);
        /* 0x15F4F */
        if (ST(ST_VIE(P_DANRYS)) == 1) {
            CB->stop_joindre(-P_DANRYS);
            ST(V_ENGAGED_PERSONA) = 0x391;
            /* 0x15F78 */
            if (ST(L15_ROOM_PART) == 1 && ST(V_EVENT_29) == 0) {
                CB->animation("SP09ATT08");
                CB->link_objet(9, "OB98", 0x1C, 0);
            }
            CB->dialogue("L15D76C", P_DANRYS);
            CB->combat(P_DANRYS, 0);
        }
        CB->select_pile(0, 1);
        return;
    }
}

/* trigger 13 — trig13_10015FE3 — zone 13 */
static void trig13(void) {
    /* 0x15FF2 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0x2EE0)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x1604C */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D506", P_ARKHAN2, "SG00DIA06", NULL);
        }
        /* 0x16074 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D507", P_ARKHAN3, "SG00DIA06", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 14 — trig14_100160B3 — zone 14 */
static void trig14(void) {
    /* 0x160C2 */
    if (ST(ST_ACTION_KEY) == 1 && ST(V_ENGAGED_PERSONA) == 0 && ST(ST_SCRIPT_BUSY) == 0 && CB->direction_spec(0xABE0)) {
        CB->bloque(BLOQUE_ALL);
        ST(ST_SCRIPT_BUSY) = 1;
        /* 0x1611C */
        if (ST(ST_HERO) == P_ARKHAN2) {
            lieu_action_register("L15D506", P_ARKHAN2, "SG00DIA06", NULL);
        }
        /* 0x16144 */
        if (ST(ST_HERO) == P_ARKHAN3) {
            lieu_action_register("L15D507", P_ARKHAN3, "SG00DIA06", NULL);
        }
        CB->modify_var(&ST(ST_SCRIPT_BUSY), 0);
        CB->bloque(BLOQUE_NONE);
        return;
    }
}

/* trigger 41 — trig41_10008D73 — room scenes: music, Kalhi 13 / Danrys / Konkalite 9 */
static void trig41(void) {
    /* 0x8D85 */
    if (ST(ST_VIE(P_DANRYS)) == 1) {
        CB->divers_empile(DV_MUSIC, 1, 1);
    }
    /* 0x8DA9 */
    if (ST(ST_VIE(P_DANRYS)) != 1) {
        CB->divers_empile(DV_MUSIC, 2, 1);
    }
    /* 0x8DCD */
    if (ST(L15_KONKALITE9_ENGAGED) != 0 && ST(ST_VIE(P_KALHI13)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0) == 1) {
        ST(L15_KONKALITE9_ENGAGED) = 0;
        ST(ST_VIE(P_KONKALITE9)) = 4;
    }
    /* 0x8E32 */
    if (!CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0)) {
        /* 0x8E44 */
        if (ST(ST_VIE(P_KONKALITE9)) == 1) {
            /* 0x8E56 */
            if (ST(L15_KONKALITE9_ENGAGED) == 0) {
                CB->pos_perso(P_KONKALITE9, 5040, 1050, 0x6978, 1, "P79ATT01", 0xA, 0);
                CB->ajoute_zone(11, 4930, 858, 305, 80, 1);
                CB->evenement(2, 2000);
            }
            /* 0x8EBB */
            if (ST(L15_KONKALITE9_ENGAGED) == 1) {
                CB->pos_perso(P_KONKALITE9, 5185, 490, 0xEA60, 1, "P79GAR03", 0xA, 0);
            }
        }
        /* 0x8EF2 */
        if (ST(ST_VIE(P_KONKALITE9)) == VIE_DEAD) {
            CB->pos_perso(P_KONKALITE9, CB->divers(DV_DESC_POS, P_KONKALITE9, 0), CB->divers(DV_DESC_POS, P_KONKALITE9, 1), CB->divers(DV_DESC_POS, P_KONKALITE9, 2), 2, "P79cad", 0xA, 0);
        }
    }
    /* 0x8F54 */
    if (ST(L15_PROGRESS_STEP) == 0 && ST(L15_DANRYS_INTRO_STAGE) == 0) {
        ST(L15_DANRYS_INTRO_STAGE) = 1;
        CB->bloque(BLOQUE_ALL);
        /* 0x8F92 */
        if (ST(V_EVENT_40) == 0) {
            CB->evenement(40, -1);
            lieu_action_register("L15D115", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            CB->dialogue("L15D116", P_DANRYS);
            lieu_action_register("L15D136", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        }
        /* 0x8FF6 */
        if (ST(V_EVENT_40) == 1) {
            lieu_action_register("L15D459", P_KALHI13, "SP13DIA53", "SP13ARR01_10");
            CB->dialogue("L15D460", P_DANRYS);
            lieu_action_register("L15D136", P_KALHI13, "SP13DIA54", "SP13ARR01_10");
        }
        CB->animation("SP13ARR01_1000");
        CB->animation("Sar_10");
        CB->bloque(BLOQUE_NONE);
        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
    }
    /* 0x907E */
    if (ST(L15_DANRYS_DEAD_SEEN) == 0 && ST(ST_VIE(P_DANRYS)) == VIE_DEAD && ST(L15_KALHI_STAGE_D40) == 0 && !CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0)) {
        ST(L15_KALHI_STAGE_D40) = 1;
        CB->bloque(BLOQUE_ALL);
        CB->joindre(-ST(ST_HERO), 5192, 470, 0x8CA0, -1);
    }
    /* 0x910E */
    if (ST(L15_KALHI_DANRYS_DEAD_NOTE) == 1 && ST(L15_KALHI_DEAD_TALK_DONE) == 0) {
        ST(L15_KALHI_DANRYS_DEAD_NOTE) = 0;
        /* 0x9141 */
        if (ST(L15_KALHI_STAGE_D40) == 0) {
            ST(L15_KALHI_STAGE_D40) = 3;
        }
        ST(L15_KALHI_DEAD_TALK_DONE) = 1;
        lieu_action_register("L15D200A", P_KALHI13, "SP13DIA58", "SP13ARR01_10");
        lieu_action_register("L15D200B", P_KALHI13, "SP13DIA57", "SP13ARR01_10");
        lieu_action_register("L15D200C", P_KALHI13, "SP13DIA61", "SP13ARR01_10");
        CB->joindre(-P_KALHI13, 0, 0, 0, ST(ST_HERO));
        CB->animation("SP13ARR01_100");
        CB->joindre_perm(P_KALHI13, ST(ST_HERO));
        CB->bloque(BLOQUE_NONE);
    }
    /* 0x9202 */
    if (ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_DANRYS_PLOT_A84) >= 3 && ST(L15_KALHI_DEAD_HANDLED) != 1) {
        ST(L15_DANRYS_SCENE_STATE) = 0;
        CB->bloque(BLOQUE_ALL);
        ST(L15_DANRYS_PLOT_A84) = 2;
        ST(V_KALHI13_TALK_C) = 2;
        ST(L15_DANRYS_GREET_PICK) = 1;
        lieu_action_register("L15D02B", P_DANRYS, "SP09DIA59", "SP09ARR01_10");
        /* 0x929A */
        if (CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0)) {
            ST(V_KALHI13_TALK_C) = 2;
            CB->animation("SP13LEV01");
            CB->animation("P13_65535");
            CB->joindre(P_KALHI13, 0, 0, 0, ST(ST_HERO));
            CB->animation("SP13ARR01");
            lieu_action_register("L03D500A", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
            lieu_action_register("L03D500B", P_KALHI13, "SP13DIA60", "SP13ARR01_10");
            CB->bloque(BLOQUE_NONE);
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
        }
        CB->bloque(BLOQUE_NONE);
        CB->joindre_perm(P_DANRYS, ST(ST_HERO));
    }
    /* 0x9367 */
    if (ST(ST_VIE(P_DANRYS)) == 1 && ST(L15_KALHI_DEAD_HANDLED) == 1) {
        ST(L15_DANRYS_FIRST_TALK) = 1;
        CB->bloque(BLOQUE_ALL);
        ST(V_ENGAGED_PERSONA) = P_DANRYS;
        CB->dialogue("L15D76C", P_DANRYS);
        CB->bloque(BLOQUE_NONE);
        CB->combat(P_DANRYS, 0);
        return;
    }
    /* 0x93D8 */
    if (ST(ST_VIE(P_DANRYS)) == VIE_DEAD && ST(V_KALHI13_TALK_C) == 3 && ST(ST_VIE(P_KALHI13)) == 1 && CB->divers(DV_PERSONA_PRESENT, P_KALHI13, 0) == 1) {
        CB->bloque(BLOQUE_ALL);
        ST(L15_DANRYS_FIRST_TALK) = 1;
        /* 0x9440 */
        if (ST(L15_DANRYS_DEAD_SEEN) == 0) {
            ST(V_KALHI13_TALK_C) = 2;
        }
        /* 0x9461 */
        if (ST(L15_DANRYS_DEAD_SEEN) == 1) {
            ST(V_KALHI13_TALK_C) = 3;
        }
        ST(L15_DANRYS_PLOT_A84) = 2;
        CB->animation("SP13LEV01");
        CB->animation("P13_65535");
        lieu_action_register("L03D500A", P_KALHI13, "P13DIA60", "SP13ARR01_10");
        lieu_action_register("L03D500B", P_KALHI13, "P13DIA60", "SP13ARR01_10");
        /* 0x94D9 */
        if (ST(L15_DANRYS_DEAD_SEEN) == 1) {
            ST(V_ENGAGED_PERSONA) = P_KALHI13;
            CB->combat(P_KALHI13, 0);
        }
        CB->bloque(BLOQUE_NONE);
        /* 0x950C */
        if (ST(L15_DANRYS_DEAD_SEEN) == 0) {
            CB->joindre_perm(P_KALHI13, ST(ST_HERO));
        }
        return;
    }
    /* 0x9539 */
    if (ST(L15_KONKALITE9_ENGAGED) == 1) {
        ST(V_ENGAGED_PERSONA) = P_KONKALITE9;
        CB->combat(P_KONKALITE9, 0);
    }
    /* 0x9564 */
    if (ST(L15_KALHI_FOLLOW_STEP) == 1) {
        ST(L15_KALHI_FOLLOW_STEP) = 2;
        CB->joindre(-P_KALHI13, 5104, 745, 0xB3B0, -1);
    }
}
