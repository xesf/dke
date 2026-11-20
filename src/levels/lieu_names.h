#pragma once

/*
 * lieu_names.h — names for the numbers the LIEUnn.DLL scripts pass to the
 * engine callbacks. Shared by every level port.
 *
 * Persona ids are the indices of the persos table (persos>Pnn.OME); the
 * names come from the speaker comments in the dialogue files. Where one
 * character has several ids (costume or story variants) the id is part of
 * the name.
 */

/* Persona ids (PosPerso, Joindre, Combat, Fouille, Dialogue speaker, ...) */
enum {
    P_ARKHAN0      = 0,     /* nightclothes */
    P_ARKHAN1      = 1,     /* guard uniform (heroes 1..3: ST_HERO) */
    P_ARKHAN2      = 2,
    P_ARKHAN3      = 3,
    P_ARCHIVIST    = 5,
    P_ARMAL        = 6,
    P_BANDOR       = 7,
    P_BOGDARAN     = 8,
    P_DANRYS       = 9,
    P_DELIA        = 10,
    P_DHORKAN11    = 11,
    P_KALHI12      = 12,
    P_KALHI13      = 13,
    P_LEONA        = 14,
    P_LORY         = 15,
    P_PROVIDER     = 16,
    P_MAOR         = 17,
    P_SUNSEER1     = 18,
    P_SUNSEER2     = 19,
    P_RYLSADHAR20  = 20,
    P_RYLSADHAR21  = 21,
    P_SORDOS       = 23,
    P_THANANDAR24  = 24,
    P_THANANDAR25  = 25,
    P_PHEDORIA     = 26,
    P_ZED27        = 27,
    P_BORK         = 30,
    P_GORK         = 31,
    P_GUARD        = 32,
    P_KALHI33      = 33,
    P_GUARDIAN1    = 41,
    P_GUARDIAN2    = 42,
    P_GUARDIAN3    = 43,
    P_GUARDIAN4    = 44,
    P_GUARDIAN5    = 45,
    P_GUARDIAN6    = 46,
    P_GUARDIAN7    = 47,
    P_GUARDIAN8    = 48,
    P_GUARDIAN9    = 49,
    P_GUARDIAN10   = 50,
    P_GUARDIAN11   = 51,
    P_GUARDIAN13   = 53,
    P_GUARDIAN14   = 54,
    P_GUARDIAN15   = 55,
    P_GUARDIAN16   = 56,
    P_GUARDIAN17   = 57,
    P_GUARDIAN18   = 58,
    P_GUARDIAN19   = 59,
    P_GUARDIAN20   = 60,
    P_MALROT1      = 61,
    P_MALROT2      = 62,
    P_MALROT3      = 63,
    P_MALROT4      = 64,
    P_MALROT5      = 65,
    P_MALROT6      = 66,
    P_MALROT7      = 67,
    P_MALROT8      = 68,
    P_KONKALITE1   = 71,
    P_KONKALITE2   = 72,
    P_KONKALITE3   = 73,
    P_KONKALITE4   = 74,
    P_KONKALITE5   = 75,
    P_KONKALITE6   = 76,
    P_KONKALITE7   = 77,
    P_KONKALITE8   = 78,
    P_KONKALITE9   = 79,
    P_KONKALITE10  = 80,
    P_KONKALITE11  = 81,
    P_KONKALITE12  = 82,
    P_KHORAD       = 83,
    P_ZED84        = 84,
    P_LODAR        = 86,
    P_KABAK        = 87,
    P_GUARDIAN23   = 88,
    P_GUARDIAN24   = 89,
    P_GUARDIAN25   = 93,
    P_DHORKAN95    = 95,
};

/* Item ids (OBnn objects, inventory, the argument of event5): the names
 * of datas/dialogue/us/OBJNAME.TXT, descriptions from OBJDESCR.TXT. */
enum {
    OB_UNIFORM      =  1,   /* Old Uniform of Bogdaran's */
    OB_BOW_TO_KEY   =  2,   /* Bow of Rylsadhar's Key */
    OB_METAL_PLATE  =  3,   /* Punched Metal Plate */
    OB_DISH         =  4,   /* Dish */
    OB_METAL_POLE   =  5,   /* Metal Pole */
    OB_F_THROW_AMMO =  6,   /* Refill for Flame-thrower */
    OB_BULLETS      =  7,   /* Lead Bullets for Spitrod */
    OB_BOILER       =  8,   /* Boiler for Spitrod */
    OB_HANDLE       =  9,   /* Metal Handle */
    OB_PLATE_BEFORE = 10,   /* Plate from Before */
    OB_RED_KEY      = 11,   /* Key made of Red Gemstone */
    OB_CRYSTAL_2    = 12,   /* Pointed Cut-crystal */
    OB_VEGETABLE    = 13,   /* Large Vegetable */
    OB_KONK_KEY     = 14,   /* Konkalite Key */
    OB_SORDOS_KEY   = 15,   /* Sordos' Key */
    OB_BOGD_KEY     = 16,   /* Bogdaran's key */
    OB_HAMMER       = 17,   /* Hammer */
    OB_CRYSTAL_1    = 18,   /* Key Crystal (bevelled on left) */
    OB_CRYSTAL_3    = 19,   /* Key Crystal (bevelled on right) */
    OB_MIRROR       = 20,   /* Mirror from Before */
    OB_VIAL         = 21,   /* Round Vial */
    OB_FLAGON22     = 22,   /* Flagon */
    OB_THBOLT_BEF   = 23,   /* Thunderbolt from Before */
    OB_FULL_GOURD   = 24,   /* Gourd of Stohll (full) */
    OB_CLOTHING     = 25,   /* Clothing */
    OB_AX26         = 26,   /* Battle-ax */
    OB_F_THROWER    = 27,   /* Flame-thrower */
    OB_LONG_KNIFE   = 28,   /* Long Knife */
    OB_SHAFT        = 29,   /* Pulley Shaft */
    OB_CLEAVER      = 30,   /* Provider Woman's Cleaver */
    OB_POTION       = 31,   /* Thanandar's Potion */
    OB_SM_THBOLT    = 32,   /* Thunderbolt from Kalhi */
    OB_LEAD_PLAQUE  = 33,   /* Lead Plaque */
    OB_POTATO       = 34,   /* Potato */
    OB_ZED_THBOLT35 = 35,   /* Zed's Thunderbolt */
    OB_ROOT         = 36,   /* Large Root */
    OB_HALBERD37    = 37,   /* Short Halberd */
    OB_SABER38      = 38,   /* Guardian of Fire Saber */
    OB_SM_TH_AMMO   = 39,   /* Ammo for Thunderbolt from Kalhi */
    OB_OBJ_BEFORE40 = 40,   /* Unknown Object from Before (soaked in oil) */
    OB_OBJ_BEFORE41 = 41,   /* Unknown Object from Before */
    OB_FLAGON42     = 42,   /* Flagon */
    OB_FISH         = 43,   /* Fish */
    OB_MEAT         = 44,   /* Smoked Meat */
    OB_CRYST_SLIV   = 45,   /* Sliver of Rock Crystal */
    OB_RYLS_NOTES   = 46,   /* Rylsadhar's Notes */
    OB_KONK_AX      = 47,   /* Heavy Konkalite Ax */
    OB_BROKEN_SHAFT = 48,   /* Pulley Shaft (broken) */
    OB_SORDOS_DOC   = 49,   /* Sordos' Document */
    OB_DIAG_WEAPON  = 50,   /* Diagram of Weapon */
    OB_PULLEY_BLADE = 51,   /* Pulley Blade from Well */
    OB_DRAWINGS     = 52,   /* Rylsadhar's Drawings */
    OB_SPEAR        = 53,   /* Konkalite Spear */
    OB_FIREBALL     = 54,   /* Fireball */
    OB_SPITROD      = 55,   /* Spitrod */
    OB_RAT          = 56,   /* Crushed Rat */
    OB_SORDOS_WPN   = 57,   /* Sordos' Weapon */
    OB_ZED_SABER    = 58,   /* Zed's Saber */
    OB_WANDR_SABER  = 59,   /* Wanderer's Saber */
    OB_COIN         = 60,   /* Dhorkan's Coin */
    OB_FLAIL        = 61,   /* Phedoria's Flail (zzz) */
    OB_HALBERD62    = 62,   /* Halberd (zzz) */
    OB_DAGGER63     = 63,   /* Kalhi's Dagger */
    OB_ROAST_BIRD   = 64,   /* Piece of Roast Bird */
    OB_BOWL         = 65,   /* Terra-cotta Bowl */
    OB_THUNDERBOLT  = 66,   /* Lory's Thunderbolt */
    OB_AX67         = 67,   /* Battle-ax */
    OB_ZED_THBOLT68 = 68,   /* Zed's Thunderbolt (on Zed) */
    OB_SAVE_ICON    = 69,   /* "Save" icon */
    OB_BANDOR_DIAG  = 70,   /* Master Builder's Diagram */
    OB_METAL_SUIT   = 71,   /* Metal suit (very heavy) */
    OB_THBOLT_AMMO  = 72,   /* Ammunition for Thunderbolts from Before */
    OB_ZED_AMMO     = 73,   /* Ammunition for Zed's Thunderbolt */
    OB_PICK_AX      = 74,   /* Danrys' Pick-ax */
    OB_CRYSTAL_KEY  = 75,   /* Rylsadhar's Crystal Key */
    OB_MACE         = 76,   /* Maor's Mace */
    OB_FLASK77      = 77,   /* Flask (empty) */
    OB_DANRYS_WPN   = 78,   /* Danrys' Small Weapon */
    OB_AX79         = 79,   /* Guardian of Fire Ax */
    OB_DHOR_SWORD   = 80,   /* Dhorkan's Sword */
    OB_PARCHMENT    = 81,   /* Rylsadhar's Parchment */
    OB_BOOK         = 82,   /* Archivist's Book */
    OB_SABER83      = 83,   /* Guardian of Fire Saber */
    OB_HELMET       = 84,   /* Metal Helmet */
    OB_BREASTPLATE  = 85,   /* Metal Breastplate (very heavy) */
    OB_FLASK86      = 86,   /* Flask from Before */
    OB_YONG_COUNTER = 87,   /* Counter for Game of Yong */
    OB_TUBE         = 88,   /* Bogdaran's Tube */
    OB_STOCK_KEY    = 89,   /* Stockade Key */
    OB_BULLET       = 90,   /* Lead Bullet */
    OB_DAGGER91     = 91,   /* Dagger */
    OB_LONG_DAGGER  = 92,   /* Long Dagger */
    OB_STILETTO     = 93,   /* Dhorkan's Stiletto */
    OB_PESTLE       = 94,   /* Kabak's Pestle */
    OB_MORTAR       = 95,   /* Kabak's Mortar */
    OB_LARGE_KEY    = 96,   /* Large Steel Key */
    OB_JERRYCAN     = 97,   /* Jerrycan */
    OB_GRINDSTONE   = 98,   /* Grindstone */
    OB_CAN          = 99,   /* Sealed Metal Can */
};

/* Timer events: CB->evenement(id, ms) arms id, event4(id) runs when it
 * expires (ms = -1 cancels). Ids 0..41 and 51..53 only set the flag
 * V_EVENT_<id>; the ones below do more. */
enum {
    EV_HUD_TIMER       = 18,  /* V_HUD_TIMER_MS -= 5000, HUD refresh, re-arm */
    EV_TO_ROOM_25      = 28,  /* leave for room 25 (from 23) */
    EV_MUSIC_LOOP_A    = 42,  /* music V_MUSIC_TRACK mode 4, arms B in 5 s */
    EV_MUSIC_LOOP_B    = 43,  /* music V_MUSIC_TRACK mode 1, arms A in 45 s */
    EV_END_GAME        = 44,  /* V_END_GAME_REQ = 1 */
    EV_GOURD_TO_FLASK  = 50,  /* SOB24 (full gourd) out, SOB77 (flask) in */
    EV_FLOAT_GROW_0    = 90,
    EV_FLOAT_GROW_1    = 91,
    EV_FLOAT_GROW_2    = 92,
    EV_FLOAT_DECAY     = 93,
    EV_PIPE_SCENE      = 94,  /* SP02PIPI1/2 */
    EV_RESPAWN_TOGGLE  = 98,
    EV_PENDING_BASE    = 89,  /* tick: evenement(V_PENDING_EVENT + 89) */
};

/* event6(id): LCOD zone sub-triggers (hints and the final ending). */
enum {
    EV6_LCOD34         = 0,   /* Arkhan2 hint, once */
    EV6_LCOD35         = 1,   /* Arkhan2 hint, once */
    EV6_LCOD36         = 2,   /* Arkhan3 hint, once */
    EV6_LCOD37         = 3,   /* Arkhan3 hint, once */
    EV6_SC21_ENDING    = 4,   /* Arkhan3: ending cinematic SC21 */
};

/* Divers(op, a, b) / DiversEmpile(op, a, b) — sub_446E19 cases. */
enum {
    DV_DESC_POS        = 1,   /* persona a: position/heading from descriptor */
    DV_CAN_FIGHT       = 2,   /* player ability a (7/8/9) */
    DV_RUN             = 3,   /* persona a: run flag */
    DV_EVENT_SOUND     = 4,   /* LnnEVn.WAV */
    DV_ITEM_CHARGE     = 5,
    DV_HP              = 6,   /* persona a: b < 0 reads, else writes */
    DV_INVENTORY       = 7,
    DV_ANGLE_TO        = 8,   /* persona a */
    DV_IN_ZONE         = 9,   /* persona a */
    DV_TRANSITION      = 10,  /* dissolve, hold, restore */
    DV_IDLE_TIME       = 11,
    DV_LOOK_AT_PLAYER  = 12,  /* persona a */
    DV_PRELOAD_ANIMS   = 13,
    DV_NIGHT           = 14,
    DV_FLAG_4B2B98     = 15,
    DV_SCREEN          = 16,
    DV_BIG_ANIM        = 17,
    DV_WAS_HIT         = 18,  /* persona a */
    DV_INN_MENU        = 19,
    DV_DARKNESS        = 20,  /* score_counter */
    DV_CINEMATIC       = 21,
    DV_SAVE_SCREEN     = 22,
    DV_COMBAT_STANCE   = 23,
    DV_MUSIC           = 24,
    DV_GIVE_ITEM       = 25,
    DV_PERSONA_ITEMS   = 26,  /* persona a */
    DV_END_GAME        = 27,
    DV_BLOOD_AT_BONE     = 28,
    DV_PERSONA_PRESENT = 29,  /* persona a */
    DV_P25_SOUND       = 30,
    DV_STRUCT_DIST     = 31,
    DV_PLAYER_ZONE     = 32,  /* collision zone under the player */
    DV_CREDITS         = 33,
    DV_PLAYER_LOOK_AT  = 34,  /* persona a */
    DV_CUT_SPEECH      = 35,
    DV_COMBAT_NEAR     = 36,
    DV_SCALE_BODY      = 37,
    DV_SCALE_BONE      = 38,
    DV_P40_TIMER       = 39,
    DV_ANIM_NAME       = 40,  /* persona a */
    DV_HUD_REFRESH     = 41,
    DV_WORLD_TICK      = 42,
    DV_SPECIAL_MODE    = 43,
    DV_USE_ITEM        = 44,
    DV_WAIT_KEY        = 45,
    DV_ACTION_HELD     = 46,
    DV_SOLID_TILE      = 47,
    DV_RANDOM          = 48,
};

/* ChangeCamera(camera, mode) */
enum {
    CAM_AUTO   = 1,
    CAM_LOCKED = 2,
    CAM_ANY    = 99,          /* camera: keep the current one */
};

/* Bloque(mode): 3 opens a cutscene bracket (player + camera), 0 closes it. */
enum {
    BLOQUE_NONE = 0,
    BLOQUE_ALL  = 3,
};

/* Script state slots the engine reads or writes (byte offsets for ST()). */
#define ST_UPDOWN_KEY     0x004
#define ST_ACTION_KEY     0x00C   /* action-key edge */
#define ST_COMBAT_STANCE  0x010   /* dword_505C50: player +0xD60 */
#define ST_LEVEL          0x018   /* dword_505C58: set after INIT, so INIT
                                     still sees the room it came from */
#define ST_SCRIPT_BUSY    0x030
#define ST_USED_VARIANT   0x128   /* dword_505D68: held item letter - 'A' */
/* dword_505D80[v]: per-variant charge counter of the used item (OB24 alcohol,
 * OB77 ...); the engine reads the held variant's as ST(ST_VARIANT_COUNT(ST(ST_USED_VARIANT))). */
#define ST_VARIANT_COUNT(v) (0x140 + 4 * (v))
#define ST_VIE(id)        (0x190 + (id) * 4)   /* persona state, id >= 4 */
#define ST_HERO           0x194   /* current hero persona; 4 = dead */
#define ST_SPECIAL_MODE   0x198   /* dword_505DD8: Tab walk-set toggle */
#define ST_PLAYER_WEAPON  0x19C   /* dword_505DDC: player desc[0x97] */
#define ST_LOCKED_CAMERA  0xF90   /* dword_506BD0 */
#define ST_GAME_TIME      0x960   /* dword_5065A0: game time in ms, set every frame (sub_449F9F) */
/* The designers' start point of each level: trigger 0 seeds the story state
 * for start n (1..) and clears it. A new game sets level 1's to 1. */
#define ST_START_POINT(level)  (0x960 + (level) * 4)

#include "lieu_vars.h"

/* ST_VIE values: the engine writes VIE_DEAD when the persona is killed. */
#define VIE_DEAD          2
