#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libsys/common.h"
#include "libsys/memory.h"
#include "libsys/system.h"
#include "libsys/vclock.h"
#include "libsys/file_system.h"
#include "libsys/graphics/graphic.h"
#include "libsys/graphics/shading.h"
#include "libsys/graphics/colour_tables.h"
#include "libsys/audio/audio.h"
#include "libsys/mouse.h"
#include "libsys/sound.h"
#include "libsys/cd.h"
#include "libsys/joy.h"
#include "libsys/movie.h"
#include "libsys/lsys_startup.h"
#include "libsys/graphics/font.h"
#include "libsys/graphics/palette.h"

#include "state.h"
#include "actor.h"
#include "game_detection.h"
#include "game_init.h"
#include "menu/menu.h"
#include "menu/screens.h"
#include "gamecfg.h"
#include "gamma.h"
#include "scene.h"
#include "pause.h"
#include "libsys/graphics/graphic.h"


state_t *state;

/**
 * Load configuration from win.ini file
 * Format:
 * [LIBSYS]
 * DEBUG=TRUE
 * [DKE]
 * HICOLOR=TRUE
 * DEMO=FALSE
 * DEBUG=TRUE
 * KEY=TRUE
 */
// 0046091B
void load_configuration(state_t *state) {
    c8 *config_path[256];
    sprintf((char *)config_path, "%s/win.ini", state->game_path);
    if (ls_exists((const c8 *)config_path)) {
        FILE *file = ls_open((const c8 *)config_path, "r");
        if (file) {
            char line[256];
            c8 current_section[64] = {0};
            while (fgets(line, sizeof(line), file)) {
                c8 key[64];
                c8 value[64];
                c8 trimmed[256];

                sscanf(line, "%[^\n]", trimmed);

                if (trimmed[0] == '[' && trimmed[strlen(trimmed) - 1] == ']') {
                    sscanf(trimmed, "[%[^]]", current_section);
                } else if (sscanf(trimmed, "%[^=]=%s", key, value) == 2) {
                    if (strcmp(current_section, "LIBSYS") == 0) {
                        if (strcmp(key, "DEBUG") == 0) {
                            state->ls_debug_enabled = (strcmp(value, "TRUE") == 0);
                        } else if (strcmp(key, "Master") == 0) {
                            state->ls_master_enabled = (strcmp(value, "TRUE") == 0);
                        } else if (strcmp(key, "Info") == 0) {
                            state->ls_info_enabled = (strcmp(value, "TRUE") == 0);
                        }
                    } else if (strcmp(current_section, "DKE") == 0) {
                        if (strcmp(key, "HICOLOR") == 0) {
                            state->colour_mode = (strcmp(value, "TRUE") == 0);
                        }
                        else if (strcmp(key, "DEMO") == 0) {
                            state->is_demo = (strcmp(value, "TRUE") == 0);
                        }
                        else if (strcmp(key, "DEBUG") == 0) {
                            state->debug_enabled = (strcmp(value, "TRUE") == 0);
                        }
                        else if (strcmp(key, "KEY") == 0) {
                            state->use_key = (strcmp(value, "TRUE") == 0);
                        }
                    }
                }
            }
            print("Configuration loaded from %s\n", (char *)config_path);
            ls_close(file);
        }
    }
    else {
        print("Configuration file not found: %s\n", (char *)config_path);
    }
}

/**
 * Load DKE.CFG game-side configuration
 * Format:
 * // comments
 * Key : Value
 * Ends at CONEX_SAID_END_OF_FILE
 * Brightness, Contrast, Saturation, Shadow and Detail also seed the player's
 * settings (g_gamecfg); DKEGAME.CFG overrides them afterwards.
 */
// 0045B361
void load_dke_config(state_t *state) {
    c8 *config_path[256];
    sprintf((char *)config_path, "%s/DKE.CFG", state->game_path);
    if (ls_exists((const c8 *)config_path)) {
        FILE *file = ls_open((const c8 *)config_path, "r");
        if (file) {
            char line[256];
            while (fgets(line, sizeof(line), file)) {
                char *trimmed = line;
                // skip leading whitespace
                while (*trimmed == ' ' || *trimmed == '\t') {
                    trimmed++;
                }
                // skip blank lines and comments
                if (*trimmed == '\0' || *trimmed == '\n' || (trimmed[0] == '/' && trimmed[1] == '/')) {
                    continue;
                }
                // check for end sentinel
                if (strncmp(trimmed, "CONEX_SAID_END_OF_FILE", 22) == 0) {
                    break;
                }
                // parse Key : Value
                c8 key[64];
                i32 value;
                if (sscanf(trimmed, " %63[^: ] : %d", key, &value) == 2) {
                    if (strcmp(key, "FirstRoom") == 0) {
                        state->first_room = value;
                    }
                    else if (strcmp(key, "Lum") == 0) {
                        state->lum = value;
                    }
                    else if (strcmp(key, "FirstPerso") == 0) {
                        state->first_perso = value;
                    }
                    else if (strcmp(key, "FirstObject") == 0) {
                        state->first_object = value;
                    }
                    else if (strcmp(key, "Brightness") == 0) {
                        state->brightness = value;
                        g_gamecfg.brightness = (i8)value;     /* rgb_brightness_slider_control */
                    }
                    else if (strcmp(key, "Contrast") == 0) {
                        state->contrast = value;
                        g_gamecfg.contrast = (i8)value;       /* rgb_contrast_slider_control */
                    }
                    else if (strcmp(key, "Saturation") == 0) {
                        g_gamecfg.saturation = (i8)value;     /* rgb_saturation_slider_control */
                    }
                    else if (strcmp(key, "Shadow") == 0) {
                        state->shadow = value;
                        g_gamecfg.shadows = value;            /* dword_4B7240 */
                    }
                    else if (strcmp(key, "Detail") == 0) {
                        state->detail = value;
                        /* render_mode = dword_4B7228[value]: 0xF for 0..5 */
                        g_gamecfg.render_mode = (value >= 0 && value < 6) ? value : 5;   /* dword_4B721C */
                    }
                    else if (strcmp(key, "Language") == 0) {
                        state->language_id = value;
                    }
                    else {
                        print("Unknown DKE.CFG key: %s\n", key);
                    }
                }
            }
            print("DKE.CFG loaded from %s\n", (char *)config_path);
            ls_close(file);
        }
    }
}

// 0047CBA0
void ls_startup(system_t *system) {
    // dword_55C790 = 0;
    ls_libsys_init_flags(state);   /* sub_487E30 — apply LIBSYS flags, detect lang */
    ls_init_memory_manager();
    ls_build_virtual_root();       /* sub_485840 — build g_virtual_root */
    ls_libsys_flag_reset();        /* sub_487D40 — reset g_ls_pause_state */
    lsys_init_mouse_system();
    ls_init_graphic_system(&state->graphic, system);
    ls_init_sound_system();
    if (state->audio_enabled) {
        ls_init_audio(&state->audio, 22050, 2, 16);
    }
    lsys_init_cd_system();
    lsys_init_joy_system();

    print("System started successfully.\n");
}

// 00491E20
void init_all_systems(system_t *system) {
    ls_startup(system);
    /* Set video mode using dimensions and colour depth from system_t.
     * bit_depth: 16 when colour_mode == 1 (hicolor), 8 otherwise.
     * bpp_flags: 0x200 (original default for non-movie mode).
     * ls_set_video_mode creates g_graphics_context + g_back_screen internally. */
    i32 bit_depth = (state->colour_mode) ? 16 : 8;
    ls_set_video_mode(system->width, system->height, bit_depth, 0x200);

    preload_shading_table();

    /* ls_main+138: 16-bit colour path — build colour mapping tables */
    if (state->colour_mode) {
        initialize_color_mapping_tables();
    }

    /* Font subsystem — must follow colour table init */
    init_font_system();
}

// 00460B6C
i32 ls_main(system_t *system, c8 **argv) {
    audio_t *audio = &state->audio;

    state->game_path = ls_get_base_path();
    load_configuration(state);
    load_dke_config(state);
    b8 first_run = !gamecfg_exists();
    gamecfg_load();                            /* config_load_gamedke_cfg_45AE70 */

    // force colour mode to 16-bit
    state->colour_mode = true;

    // skip memory check

    // sub_4843F0(v13, v14); — CPU/memory check
    cpu_info_t cpu_info;
    ls_measure_cpu_speed(&cpu_info);
    print("ls_main: CPU speed ~%u MHz, perf_freq=%llu\n",
          cpu_info.cpu_speed_mhz, (unsigned long long)cpu_info.perf_counter_freq);

    // sub_4953B0(); — copy unknown buffers a→b
    ls_copy_unknown_a_to_b();

    ls_set_title(system, state->title);

    // sub_491450(); — copy alternative palette data to palette target buffer
    palette_copy_alternative();

    // sub_495390(); — install default error/warn handlers
    ls_install_default_error_handlers();

    if (state->audio_enabled) {
        // Initialize audio system
        if (ls_init_audio(audio, 22050, 2, 16)) {
            print("Audio system initialized successfully.\n");
        } else {
            print("Failed to initialize audio system.\n");
        }
    }

    init_all_systems(system);

    if (!game_init(state)) {
        print("game_init failed — cannot start game\n");
    }

    /* ls_main debug text — mirrors original ls_main reference:
     *   set_position_coordinates(0, 32); draw_text(aInitLibsysFini); */
    if (state->debug_enabled) {
        set_position_coordinates(0, 32);
        draw_text("Init libsys finished");
    }

    ls_init_movie(0, 0);

    /* Play intro cutscene chain. Original engine has multiple entry paths
     * (game_play_intro_469F32 does CD-drive detection first; sub_437DDD plays
     * EADKE.CRH from the main-menu "trailer" option). For now, play the
     * publisher/title splash (EADKE) at boot so we exercise the CRH decoder
     * + CoreAudio streaming end-to-end. ls_play_movie skips gracefully if
     * the file is missing rather than crash. */
    /* process_mainmenu_437EB7 0x43839C: no DKEGAME.CFG -> menu_process_gamma, then EADKE.
     * DKE_GAMMA_PICKER=1 shows it headless too. */
    if ((first_run && !getenv("DKE_NEWGAME")) || getenv("DKE_GAMMA_PICKER"))
        menu_gamma_run();
    if (!getenv("DKE_NEWGAME")) {  /* skip the movies in headless test mode */
        /* process_mainmenu_437EB7 0x43837C: EADKE (sub_437DDD) on every run;
         * the first time (no DKEGAME.CFG) the SC01 intro follows (the gamma
         * picker came first, above). The cfg is written right after SC01. */
        ls_play_movie("HDISK:>EARTH>sc>EADKE.CRH");
        if (first_run) {
            ls_play_movie("HDISK:>EARTH>sc>SC01.CRH");
            gamecfg_save();
        }
    }

    /* --- Slice B: main menu state machine ---
     * menu_init fetches the MENUGEN backdrop from game_init (same source as
     * Slice A) and sets up the menu_t state.  menu_render owns the per-frame
     * backdrop blit + text overlay, replacing the raw Slice A blit. */
    menu_t menu;
    menu_init(&menu);

    /* Debug autostart for headless/log-driven testing:
     *   DKE_NEWGAME=1  — skip the menu, load first room immediately.
     *   DKE_FRAMES=N   — quit after N main-loop frames. */
    i32 dbg_autonew   = (getenv("DKE_NEWGAME") != NULL);
    i32 dbg_maxframes = getenv("DKE_FRAMES") ? atoi(getenv("DKE_FRAMES")) : 0;
    i32 dbg_frame     = 0;
    if (dbg_autonew) {
        print("main: DKE_NEWGAME -> scene_load_level(%d)\n", state->first_room);
        actor_new_game();
        if (scene_load_level(state->first_room) == 0) {
            menu.state = MENU_STATE_LOADING;
            menu.game_running = 1;
        }
        else
            print("main: DKE_NEWGAME scene_load_level failed\n");
    }

    i32 prev_menu_state = menu.state;
    while(!system->quit) {
        if (dbg_maxframes && ++dbg_frame > dbg_maxframes) {
            print("main: DKE_FRAMES=%d reached — exiting\n", dbg_maxframes);
            break;
        }
        if (vclock_active()) vclock_advance(vclock_frame_ms());   /* DKE_FASTCLOCK */
        system->timer.tick = ls_tick();
        ls_update_events(system);

        /* Update hover / keyboard nav state */
        menu_update(&menu, system);

        /* Render menu vs scene depending on state. Once the user picks
         * NEW GAME, menu_dispatch calls scene_load_level and flips the
         * state to LOADING; from that frame on we blit the level's
         * camera-0 backdrop instead of the menu backdrop. */
        if (menu.state == MENU_STATE_LOADING && prev_menu_state != MENU_STATE_LOADING)
            game_time_thaw(1, 39);                   /* restore_display_mode(1, 39): back from the menu */
        prev_menu_state = menu.state;
        if (menu.state == MENU_STATE_LOADING) {
            /* scene_render owns WASD polling now (moves the placeholder
             * cube around the room). Returns 1 when Esc is pressed;
             * flip back to the main menu so the user can pick another
             * action. */
            if (scene_render(system) == 1) {
                game_time_freeze(39);                /* set_display_mode(39) before process_mainmenu_437EB7 */
                menu.state = MENU_STATE_MAIN;
                menu.hovered_item = 6;               /* var_4AEC = 6: resume */
                menu.dirty = 1;
                menu.startup_frames_remaining = 15;  /* re-arm click grace */
            }
        } else {
            menu_render(&menu, system);
        }

        /* Exit main loop when user selects QUIT */
        if (menu.state == MENU_STATE_QUIT) {
            print("ls_main: menu QUIT selected, breaking loop\n");
            break;
        }

        /* Headless capture of any screen (menu included): DKE_MSHOT=<frame>
         * writes DKE_SHOT_PATH (PPM). */
        if (getenv("DKE_MSHOT") && dbg_frame == atoi(getenv("DKE_MSHOT")) &&
            state->graphic.front_buffer) {
            const c8 *out = getenv("DKE_SHOT_PATH");
            FILE *f = fopen(out ? out : "/tmp/dke_shot.ppm", "wb");
            if (f) {
                i32 w = state->graphic.width, h = state->graphic.height;
                fprintf(f, "P6\n%d %d\n255\n", w, h);
                u32 *shot = (u32 *)malloc((size_t)w * h * sizeof(u32));
                if (shot) gamma_copy_pixels(shot, state->graphic.front_buffer, w * h);
                for (i32 i = 0; i < w * h; i++) {
                    u32 p = shot ? shot[i] : state->graphic.front_buffer[i];
                    u8 rgb[3] = { (u8)p, (u8)(p >> 8), (u8)(p >> 16) };
                    fwrite(rgb, 1, 3, f);
                }
                free(shot);
                fclose(f);
            }
        }

        ls_blit(system, state->graphic.front_buffer);
        ls_flip(system);

        system->timer.frame_count++;
        if (system->timer.tick - system->timer.last_tick_fps >= 1000) {
            snprintf(state->title, 64, "%s - fps: %d", system->title, system->timer.frame_count);
            ls_set_title(system, state->title);
            system->timer.frame_count = 0;
            system->timer.last_tick_fps = system->timer.tick;
        }

        if (system->actions[102] == true) {
            state->lock_fps = !state->lock_fps;
        }
        if (state->lock_fps) {
            system->timer.last_tick = ls_tick();
            // limit fps to 40
            if (system->timer.last_tick - system->timer.tick < 25) {
                ls_delay(25 - (system->timer.last_tick - system->timer.tick));
            }
        }
        else {
            ls_delay(1);
        }
    }
    gamecfg_save();                            /* 0x46134A */
    ls_shut_movie();
    scene_release();
    menu_release();
    return 0;
}

#ifndef _WIN32
i32 main(i32 argc, c8 **argv) {
#else
#include <windows.h>
i32 CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, i32 nCmdShow) {
#endif
    state = memory_calloc(sizeof(state_t));
    state_init(state);
    state->game_type = detect_game();
    state->system.title = (c8*)get_game_title(state->game_type);

    system_t *system = &state->system;

    if (ls_init_instance(system, 640, 480, 4)) {
        // TODO: hide cursor
#ifdef _WIN32
        ls_main(system, NULL);
#else
        ls_main(system, argv);
#endif
    }
    
    ls_release(system);
    state_release(state);
    
    return 0;
}
