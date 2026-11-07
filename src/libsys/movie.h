#pragma once

#include "common.h"

/*
 * LIBSYS movie playback subsystem (SEB codec).
 *
 * This header exposes only init / shut. Playback (ls_play_movie) is a
 * partial skeleton — decoder chain (init_decompress, read_and_init_sbt,
 * decompress_anim) is not ported yet.
 *
 * Original chain:
 *   ls_init_movie -> init_my_memory + tables + seb_get_memory * N + cp_init
 *   ls_play_movie -> init_decompress -> read_and_init_sbt -> decompress_anim
 *   ls_shut_movie -> cp_uninit + ls_release_heap + seb_release_all
 *
 * seb_get_memory / seb_release_all are a movie-local sub-allocator backed
 * by the LIBSYS memory pool. The port uses malloc/free directly (matches
 * the semantics of ls_get_memory in the original).
 */

/* 00431EA0 — a=flip_mode, b=audio_callback_slot (0 -> traite_zik_flip, 1 -> traite_zik) */
void ls_init_movie(i32 a, i32 b);

/* 00432120 — teardown, releases all seb slots + heap slot + cp state */
void ls_shut_movie(void);

/* skeleton — logs and returns without decoding. */
void ls_play_movie(const c8 *path);

/*
 * Per-movie options for the next ls_play_movie call (reset after it).
 *   ls_set_movie_subtitles  0x432799: text callback, called every frame with the
 *                           line due (sub_415CC0 in the game).
 *   subtitle file           DATAS>SBT>SC<nn>_<lang>.TXT, "mm:ss:ff mm:ss:ff {text}".
 *   start frame             the "_500" suffix of the CINEMA name.
 *   end card                the callback of ls_play_movie (sub_444B10, OVER card).
 */
typedef void (*ls_movie_sbt_fn)(const c8 *text, u32 *fb, i32 w, i32 h);
void ls_set_movie_subtitles(ls_movie_sbt_fn cb, const c8 *lang);
void ls_set_movie_start(i32 frame);
void ls_set_movie_end_card(void (*cb)(void));
