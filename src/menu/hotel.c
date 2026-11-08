/*
 * hotel.c — DIVERS op 19 (DV_INN_MENU), sub_4272B0 @0x4272B0, native rebuild
 * of HOTEL.DLL (64 KB x86 PE, export __LIB@8 @0x10001023 -> main @0x1000333B).
 *
 * Despite the name it is not a menu: it is an Othello (Reversi) game against
 * the computer, played at the inn of level 18 against Armal. sub_4272B0
 * clears the screen, set_display_mode(26), runs the DLL, frees the sound
 * slots 0 and 1 (sub_44E46A) and returns dword_4B33F4, which the DLL sets:
 *
 *   1  the player (white, BLANC) has more discs when the game ends
 *   2  the computer (black, NOIR) has more discs, or the player pressed Esc
 *   3  a draw (level 18 treats 1 and 3 alike)
 *
 * There is no money / inventory / HP effect in the DLL; the level script
 * (lieu18.c) branches on the value only.
 *
 * Assets (DATAS>GLOBAL>IMAGES>, .GIF names in the DLL, .CIF here):
 *   OTHELLO   640x480 board backdrop; its palette is the one shown
 *   OTHPIECE  87x170, the pointer is the 39x35 rect at (19,63)
 *   ANIBLANC  350x50, 7 disc frames of 50 px; frame 0 = white, 6 = black,
 *             the frames in between are the flip animation
 *   OTHELLO.GRD  shade table, 64 levels x 256 (level 0x12 darkens the
 *             strip behind a message)
 *   DATAS>FNT>FONT9  20 x 5 glyphs of 9 x 9, from '!' (0x21)
 *
 * Board: an 8x8 grid inside a 10x10 array (the ring is -1), cell (col,row)
 * is drawn at x = 106 + 54 (col-1), y = 42 + 47 (row-1). The click maps to
 * col = (mx-0x69)/0x36+1, row = (my-0x36)/0x2F+1 (C division, as the DLL's
 * idiv: a click a little left of / above the grid still hits column / row 1).
 *
 * Flow (main @0x1000333B): the computer (2) moves first (0x1000264E), then
 * every left click on the grid is one human turn (3E8F): a legal move is
 * placed and flipped, the flips animate (2679), and the computer answers in
 * the same click (48A0). An illegal click plays the voice L18D999 (when
 * player +0xB0 == 2) or L18D000 and nothing else happens. With no legal move
 * the click passes the turn. The game ends (3FFC) when the board is full or
 * both sides had to pass; Esc ends it at once with 2. The DLL's two-player
 * mode (X-FILES.000, "QUELLE COULEUR ? [B/N]") is not reachable from main and
 * is not ported.
 *
 * Computer move (48A0): a corner if one is legal (the last in scan order),
 * else the lookahead pick (4B22): every candidate is played and scored by 4476
 * (alternating greedy-by-position-weight replies, up to three rounds where
 * a human corner costs 100, else the sum of the computer's weights), then
 * -150 for the four X-squares (2,2) (7,2) (2,7) (7,7); the highest score
 * wins, the first on a tie.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu/screens.h"
#include "state.h"
#include "game_init.h"
#include "sfx.h"
#include "pause.h"
#include "actor.h"
#include "entity.h"
#include "libsys/common.h"
#include "libsys/system.h"
#include "libsys/file_system.h"
#include "libsys/graphics/graphic.h"

#define SCR_W 640
#define SCR_H 480

/* dword_4B33F4: the value sub_4272B0 returns. */
static i32 s_result;

/* ------------------------------------------------------------------ */
/* Game core (no I/O)                                                  */

static const i32 K_DX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };      /* 0x1000DBF8 */
static const i32 K_DY[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };      /* 0x1000DBFC */

/* Position weights, cell +8 of the board array (0x1000DA68), [row][col];
 * -1 marks the border ring. */
static const i32 K_W[10][10] = {
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    { -1, 30,  3, 25, 16, 16, 25,  3, 30, -1 },
    { -1,  3,  0,  9, 12, 12,  9,  0,  3, -1 },
    { -1, 25,  9, 30, 15, 15, 30,  9, 25, -1 },
    { -1, 16, 12, 15, 18, 18, 15, 12, 16, -1 },
    { -1, 16, 12, 15, 18, 18, 15, 12, 16, -1 },
    { -1, 25,  9, 30, 15, 15, 30,  9, 25, -1 },
    { -1,  3,  0,  9, 12, 12,  9,  0,  3, -1 },
    { -1, 30,  3, 25, 16, 16, 25,  3, 30, -1 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
};

typedef struct { i32 col, row, w; } mv_t;
#define MAXMV 64

static i32  P[10][10];            /* 0x100103FC + 4: 0 empty, 1 white, 2 black, -1 ring */
static i32  SH[10][10];           /* 0x1000F624: what the screen last showed */
static mv_t g_cm[MAXMV]; static i32 g_ncm;     /* 0x10011818 / dword_1000DA30: moves of 2 */
static mv_t g_hm[MAXMV]; static i32 g_nhm;     /* 0x10010B08 / dword_1000DA34: moves of 1 */
static i32  g_best_c[2], g_best_h[2];          /* da4c/da50 and da44/da48 */
static i32  g_score[MAXMV];                    /* 0x10011BA8 */
static i32  g_last_col, g_last_row;            /* 0x100102A4 / A8: the player's last move */
static i32  g_turn;                            /* 0x1000DA38: 1 player, 2 computer, 0 over */
static i32  g_pass;                            /* 0x1000DA3C: bit 1 computer passed, 2 player passed */

/* 0x100023FF: the start position, the computer to move. */
static void board_reset(void) {
    memset(P, 0, sizeof(P));
    for (i32 i = 0; i < 10; i++) { P[i][0] = P[i][9] = P[0][i] = P[9][i] = -1; }
    P[4][4] = 1; P[4][5] = 2; P[5][4] = 2; P[5][5] = 1;
    memcpy(SH, P, sizeof(SH));
    g_ncm = g_nhm = 0;
    g_last_col = g_last_row = 0;
    g_turn = 2;
    g_pass = 0;
}

/* 0x10003D73: can `mover` play (col,row) along direction d? */
static i32 ray_ok(i32 mover, i32 opp, i32 col, i32 row, i32 d) {
    if (P[row][col] != 0) return 0;
    if (P[row + K_DY[d]][col + K_DX[d]] != opp) return 0;
    i32 c = col, r = row;
    for (;;) {
        c += K_DX[d]; r += K_DY[d];
        i32 p = P[r][c];
        if (p == mover) return 1;
        if (p == 0 || K_W[r][c] == -1) return 0;
    }
}

/* 0x10003B54: the legal moves of `mover`, one entry per valid direction
 * (a move valid in two directions is listed twice), rows outer. */
static void gen_moves(i32 mover) {
    i32 opp = 3 - mover;
    mv_t *L = mover == 1 ? g_hm : g_cm;
    i32 n = 0;
    for (i32 row = 1; row < 9; row++)
        for (i32 col = 1; col < 9; col++) {
            if (P[row][col] != 0) continue;
            for (i32 d = 0; d < 8; d++)
                if (P[row + K_DY[d]][col + K_DX[d]] == opp &&
                    ray_ok(mover, opp, col, row, d) && n < MAXMV) {
                    L[n].col = col; L[n].row = row; L[n].w = K_W[row][col];
                    n++;
                }
        }
    if (mover == 1) g_nhm = n; else g_ncm = n;
}

/* 0x10004676 / the placing half of 0x10003898: put `mover` on (col,row) and
 * flip every bracketed line. Returns the number of valid directions (0 =
 * nothing changed). */
static i32 place(i32 mover, i32 opp, i32 col, i32 row) {
    if (col < 1 || col > 8 || row < 1 || row > 8 || P[row][col] != 0) return 0;
    i32 dirs = 0;
    for (i32 d = 0; d < 8; d++) {
        if (!ray_ok(mover, opp, col, row, d)) continue;
        dirs++;
        i32 c = col, r = row;
        P[r][c] = mover;
        for (;;) {
            c += K_DX[d]; r += K_DY[d];
            if (P[r][c] == mover) break;
            P[r][c] = mover;
        }
    }
    return dirs;
}

/* 0x10004822: empty cells left. */
static i32 empties(void) {
    i32 n = 0;
    for (i32 r = 1; r < 9; r++) for (i32 c = 1; c < 9; c++) if (P[r][c] != 0) n++;
    return 0x40 - n;
}

/* 0x100042CF: the computer's (2) weights. */
static i32 weight_sum2(void) {
    i32 s = 0;
    for (i32 r = 1; r < 9; r++) for (i32 c = 1; c < 9; c++) if (P[r][c] == 2) s += K_W[r][c];
    return s;
}

/* 0x10004133 / 0x100043ED: the highest-weight move of a list, the first on a tie. */
static void pick_best(const mv_t *L, i32 n, i32 out[2]) {
    i32 best = -1000;
    for (i32 i = 0; i < n; i++)
        if (L[i].w > best) { best = L[i].w; out[0] = L[i].col; out[1] = L[i].row; }
}

static i32 is_corner(i32 col, i32 row) {
    return (col == 1 || col == 8) && (row == 1 || row == 8);
}

/* 0x10004476: after the computer played candidate `idx`, play on greedily
 * (weights) for both sides and score it. */
static void eval_line(i32 idx) {
    i32 rounds = 0, found = 0;
    for (;;) {
        gen_moves(1);
        if (g_nhm) {
            i32 t = rounds++;
            if (t < 3) {
                for (i32 k = 0; k < g_nhm; k++)
                    if (is_corner(g_hm[k].col, g_hm[k].row)) { found = 1; g_score[idx] -= 100; break; }
            }
            pick_best(g_hm, g_nhm, g_best_h);
            place(1, 2, g_best_h[0], g_best_h[1]);
            if (empties() <= 0) break;
        }
        gen_moves(2);
        if (g_ncm) {
            pick_best(g_cm, g_ncm, g_best_c);
            place(2, 1, g_best_c[0], g_best_c[1]);
            if (empties() <= 0) break;
        }
        if (g_nhm == 0 && g_ncm == 0) break;
    }
    if (!found) g_score[idx] = weight_sum2();
}

/* 0x10004B22: the lookahead pick, into g_best_c. */
static void ai_lookahead(void) {
    i32 backup[10][10];
    mv_t cand[MAXMV];
    memset(g_score, 0, sizeof(g_score));
    memcpy(backup, P, sizeof(P));
    gen_moves(2);
    i32 n = g_ncm;
    memcpy(cand, g_cm, sizeof(cand));
    for (i32 i = 0; i < n; i++) {
        memcpy(P, backup, sizeof(P));
        place(2, 1, cand[i].col, cand[i].row);
        eval_line(i);
    }
    /* The bonuses read the board the last simulation left behind, so they
     * add the same amount to every candidate (kept as the DLL has it). */
    i32 best = -10000, bi = 0;
    for (i32 i = 0; i < n; i++) {
        i32 x = g_last_col, y = g_last_row;
        #define AT(r, c) (((r) >= 0 && (r) <= 9 && (c) >= 0 && (c) <= 9) ? P[r][c] : -2)
        if (x == 1 || x == 8) {
            if (AT(y - 2, x) == 0 && AT(y - 1, x) == 1) g_score[i] += 150;
            if (AT(y + 2, x) == 0 && AT(y + 1, x) == 1) g_score[i] += 150;
        }
        if (y == 1 || y == 8) {
            if (AT(y, x - 2) == 0 && AT(y, x - 1) == 1) g_score[i] += 150;
            if (AT(y, x + 2) == 0 && AT(y, x + 1) == 1) g_score[i] += 150;
        }
        #undef AT
        if (cand[i].col == 2 && cand[i].row == 2) g_score[i] -= 150;
        if (cand[i].col == 7 && cand[i].row == 2) g_score[i] -= 150;
        if (cand[i].col == 2 && cand[i].row == 7) g_score[i] -= 150;
        if (cand[i].col == 7 && cand[i].row == 7) g_score[i] -= 150;
        if (g_score[i] > best) { best = g_score[i]; bi = i; }
    }
    memcpy(P, backup, sizeof(P));
    if (n) { g_best_c[0] = cand[bi].col; g_best_c[1] = cand[bi].row; }
}

/* ------------------------------------------------------------------ */
/* Screen                                                              */

typedef struct { i32 state, frame; u32 t0; i32 nframes; } hanim_t;

typedef struct {
    u8     ds[SCR_W * SCR_H];     /* debug_surface, 8-bit */
    u8     pal[768];
    u8     grd[0x4000];
    bmap_t *bg, *piece, *ani, *font;
    /* 0x10010D78: per cell { state, frame, t_start, now, frames, col, row } */
    hanim_t an[10][10];
    c8     msg[64];               /* 0x100102D8 */
    i32    msg_y;
    u32    msg_t;                 /* 0x100116DC */
    i32    msg_on;                /* 0x10011AF8 */
} hscreen_t;

static hscreen_t *H;

static void free_bmap(bmap_t *b) {
    if (!b) return;
    if (b->owns_buffer && b->pixels) free(b->pixels);
    if (b->owns_palette && b->palette) free(b->palette);
    free(b);
}

/* ls_bit_move on the 8-bit surface: inclusive source rect, index 0 skipped
 * when `key`. */
static void blit(const bmap_t *src, i32 x1, i32 y1, i32 x2, i32 y2, i32 dx, i32 dy, i32 key) {
    if (!src || !src->pixels) return;
    for (i32 r = 0; r <= y2 - y1; r++) {
        i32 y = dy + r, ys = y1 + r;
        if (y < 0 || y >= SCR_H || ys < 0 || ys >= src->height) continue;
        for (i32 c = 0; c <= x2 - x1; c++) {
            i32 x = dx + c, xs = x1 + c;
            if (x < 0 || x >= SCR_W || xs < 0 || xs >= (i32)src->width) continue;
            u8 v = src->pixels[ys * src->width + xs];
            if (key && v == 0) continue;
            H->ds[y * SCR_W + x] = v;
        }
    }
}

static void present(void) {                    /* ds -> screen */
    u32 *fb = state->graphic.front_buffer;
    for (i32 i = 0; i < SCR_W * SCR_H; i++) {
        const u8 *p = &H->pal[H->ds[i] * 3];
        fb[i] = 0xFF000000u | ((u32)p[2] << 16) | ((u32)p[1] << 8) | p[0];
    }
    ls_blit(&state->system, fb);
    ls_flip(&state->system);
}

/* 0x10002C33: a message centred at y; the strip behind it is darkened with
 * the GRD level 0x12 (0x1000212C). Shown for one second (clear at 2C33
 * with an empty text, called every frame). */
static void say(const c8 *text, i32 y) {
    snprintf(H->msg, sizeof(H->msg), "%s", text);
    H->msg_y = y;
    H->msg_t = ls_tick();
    H->msg_on = 1;
}

static void draw_message(void) {
    if (!H->msg_on) return;
    if (ls_tick() - H->msg_t > 1000) { H->msg_on = 0; return; }
    i32 len = (i32)strlen(H->msg);
    i32 x = (SCR_W - 9 * len) / 2, y = H->msg_y;
    for (i32 r = 0; r <= 12; r++)
        for (i32 c = 0; c <= 9 * len + 4; c++) {
            i32 px = x - 2 + c, py = y - 2 + r;
            if (px < 0 || px >= SCR_W || py < 0 || py >= SCR_H) continue;
            u8 *d = &H->ds[py * SCR_W + px];
            *d = H->grd[0x12 * 256 + *d];
        }
    for (i32 i = 0; i < len; i++) {
        u8 ch = (u8)H->msg[i];
        if (ch <= 0x20 || ch >= 0x84) ch = 0x84;
        i32 idx = ch - 0x21;
        i32 sx = (idx % 20) * 9, sy = (idx / 20) * 9;
        blit(H->font, sx, sy, sx + 8, sy + 8, x + 9 * i, y, 1);
    }
}

#define CELL_X(col) (106 + 54 * ((col) - 1))       /* 0x1000256C.. */
#define CELL_Y(row) (42 + 47 * ((row) - 1))

static void draw_static(i32 piece, i32 x, i32 y) {
    if (piece == 1) blit(H->ani, 0, 0, 0x32, 0x32, x, y, 1);
    else if (piece == 2) blit(H->ani, 0x12C, 0, 0x15E, 0x32, x, y, 1);
}

/* 0x10001110: the backdrop and every disc; 1 when all 64 cells are idle. */
static i32 render_board(void) {
    blit(H->bg, 0, 0, 0x219, 0x1AE, 0, 0, 0);
    i32 idle = 0;
    for (i32 row = 1; row < 9; row++)
        for (i32 col = 1; col < 9; col++) {
            u32 now = ls_tick();
            i32 x = CELL_X(col), y = CELL_Y(row);
            hanim_t *a = &H->an[row][col];
            if (a->state == 2) {
                if ((i32)(now - a->t0) > 0x53) { a->state = 1; a->t0 = ls_tick(); }
                draw_static(P[row][col], x, y);
            } else if (a->state == 1) {
                i32 sx = a->frame * 50;
                blit(H->ani, sx, 0, sx + 49, 49, x, y, 1);
                if ((i32)(now - a->t0) > 0x42) {
                    if (P[row][col] == 2) {
                        if (++a->frame == a->nframes) { a->frame = 0; a->state = 0; }
                    }
                    if (P[row][col] == 1) {
                        if (--a->frame == -1) { a->frame = 6; a->state = 0; }
                    }
                    a->t0 = now;
                }
            } else {
                idle++;
                draw_static(P[row][col], x, y);
            }
        }
    return idle >= 0x40;
}

/* 0x1000201E: start the flip of (row,col) to `to`; later ones start sooner
 * (t0 backdated by 100 ms per cell already waiting). */
static void flip_start(i32 row, i32 col, i32 to) {
    i32 n = 0;
    for (i32 r = 1; r < 9; r++) for (i32 c = 1; c < 9; c++) if (H->an[r][c].state == 2) n++;
    hanim_t *a = &H->an[row][col];
    a->t0 = ls_tick() - 100u * (u32)n;
    a->nframes = 7;
    a->state = 2;
    a->frame = to == 1 ? 6 : 0;
}

/* 0x10001EBB: compare the cell with what the screen showed. */
static void flip_check(i32 row, i32 col, i32 mcol, i32 mrow) {
    if (mrow == row && mcol == col) {
        if (P[row][col] == 1) SH[row][col] = 1;
        if (P[row][col] == 2) SH[row][col] = 2;
        return;
    }
    if (P[row][col] == 1) {
        if (SH[row][col] != 1) flip_start(row, col, 1);
        SH[row][col] = 1;
    }
    if (P[row][col] == 2) {
        if (SH[row][col] != 2) flip_start(row, col, 2);
        SH[row][col] = 2;
    }
}

static void draw_cursor(void) {
    system_t *sys = &state->system;
    blit(H->piece, 19, 63, 57, 97, sys->motion_x, sys->motion_y, 1);
}

/* 0x10002679: animate the discs that changed until all are idle. */
static void animate(i32 mcol, i32 mrow) {
    system_t *sys = &state->system;
    for (i32 row = 8; row > 0; row--)
        for (i32 col = 8; col > 0; col--) flip_check(row, col, mcol, mrow);
    while (!sys->quit) {
        ls_update_events(sys);
        sfx_pump();
        i32 done = render_board();
        draw_message();
        present();
        if (done) break;
        ls_delay(5);
    }
}

/* 0x10003898: the human / the engine's checked move. `mover` plays. */
static i32 play_checked(i32 mover, i32 col, i32 row) {
    i32 opp = 3 - mover;
    if (P[row][col] != 0) {
        sfx_voice_play(NULL, E32(g_player_entity, 0xB0) == 2 ? "L18D999" : "L18D000", 0, 0);
        return 0;
    }
    i32 dirs = place(mover, opp, col, row);
    if (dirs) {
        g_turn = mover == 1 ? 2 : 1;
    } else {
        sfx_voice_play(NULL, E32(g_player_entity, 0xB0) == 2 ? "L18D999" : "L18D000", 0, 0);
    }
    return dirs;
}

/* 0x100048A0: the computer's move. */
static void computer_move(void) {
    g_best_c[0] = g_best_c[1] = 0;
    gen_moves(2);
    if (g_ncm) {
        for (i32 i = 0; i < g_ncm; i++)
            if (is_corner(g_cm[i].col, g_cm[i].row)) { g_best_c[0] = g_cm[i].col; g_best_c[1] = g_cm[i].row; }
        if (g_best_c[0] == 0 && g_best_c[1] == 0) ai_lookahead();
        if (g_best_c[0] && g_best_c[1]) {
            if (!play_checked(2, g_best_c[0], g_best_c[1])) say("attention Testevalide bugge!!!", 0x1D6);
        } else {
            say("attention EvalP bugge !!! ", 0x1D6);
        }
    } else {
        g_turn = 1;                                    /* 0x10004AC9 */
        g_pass |= 1;
        say("Je ne peux pas jouer !!! ", 0x1D6);
    }
}

/* 0x10003FFC: the end of the game. Returns the disc total. */
static i32 end_check(void) {
    i32 w = 0, b = 0;
    for (i32 r = 1; r < 9; r++) for (i32 c = 1; c < 9; c++) { if (P[r][c] == 1) w++; if (P[r][c] == 2) b++; }
    if (w + b >= 0x40 || g_pass == 3) {
        if (w > b) s_result = 1;
        if (w < b) s_result = 2;
        if (w == b) s_result = 3;
        g_turn = 0;
    }
    return w + b;
}

/* 0x10003E8F: one left click at (mx,my). */
static void on_click(i32 mx, i32 my) {
    g_pass = 0;
    i32 col = (mx - 0x69) / 0x36 + 1;
    i32 row = (my - 0x36) / 0x2F + 1;
    if (col < 1 || col > 8 || row < 1 || row > 8) return;
    if (g_turn == 1) {
        gen_moves(1);
        if (g_nhm == 0) {
            g_pass |= 2;
            g_turn = 2;
        } else {
            if (play_checked(1, col, row)) { g_last_col = col; g_last_row = row; }
            animate(col, row);
        }
    }
    if (g_turn == 2) {
        computer_move();
        animate(g_best_c[0], g_best_c[1]);
    }
    end_check();
}

static i32 load_grd(void) {
    c8 host[512];
    const c8 *base = (state && state->game_path) ? state->game_path : ".";
    if (!ls_resolve_path(base, "DATAS>GLOBAL>IMAGES>OTHELLO.GRD", host, (i32)sizeof(host))) return 0;
    FILE *f = ls_open(host, "rb");
    if (!f) return 0;
    i32 ok = fread(H->grd, 1, sizeof(H->grd), f) == sizeof(H->grd);
    ls_close(f);
    return ok;
}

static void screen_free(void) {
    if (!H) return;
    free_bmap(H->bg); free_bmap(H->piece); free_bmap(H->ani); free_bmap(H->font);
    free(H);
    H = NULL;
}

/* sub_4272B0 body: the DLL's main (0x1000333B). */
static void hotel_main(void) {
    system_t *sys = &state->system;
    H = (hscreen_t *)calloc(1, sizeof(*H));
    if (!H) { s_result = 2; return; }
    H->bg    = game_init_load_cursor("DATAS>GLOBAL>IMAGES>OTHELLO.CIF");
    H->piece = game_init_load_cursor("DATAS>GLOBAL>IMAGES>OTHPIECE.CIF");
    H->ani   = game_init_load_cursor("DATAS>GLOBAL>IMAGES>ANIBLANC.CIF");
    H->font  = game_init_load_cursor("DATAS>FNT>FONT9.CIF");
    if (!H->bg || !H->piece || !H->ani || !H->bg->palette || !load_grd()) {
        print("hotel: assets missing (OTHELLO / OTHPIECE / ANIBLANC / OTHELLO.GRD)\n");
        s_result = 2;                       /* the choice the level reads as a loss */
        screen_free();
        return;
    }
    memcpy(H->pal, H->bg->palette, 768);
    H->pal[0] = H->pal[1] = H->pal[2] = 0;

    sys->motion_x = 0; sys->motion_y = 0;   /* ls_set_mouse(0,0) */
    board_reset();
    /* 0x1000263C: draw the start position, then the computer opens. */
    animate(0, 0);
    computer_move();
    g_turn = 1;                             /* 0x10002653 */
    animate(g_best_c[0], g_best_c[1]);

    i32 prev[256];
    memcpy(prev, sys->actions, sizeof(prev));
    i32 quit = 0;
    u32 opened = ls_tick(), last_auto = opened;
    const c8 *autoplay = getenv("DKE_HOTELAUTO");   /* headless: the player clicks its best cell */
    while (!sys->quit && !quit) {
        ls_update_events(sys);
        sfx_pump();
        i32 esc = sys->actions[27] && !prev[27];
        memcpy(prev, sys->actions, sizeof(prev));
        i32 click = sys->mouse_left == 2;
        i32 cx = sys->motion_x, cy = sys->motion_y;
        if (autoplay && ls_tick() - last_auto > 250) {
            last_auto = ls_tick();
            gen_moves(1);
            i32 o[2] = { 1, 1 };
            if (g_nhm) pick_best(g_hm, g_nhm, o);
            cx = 0x69 + 0x36 * (o[0] - 1) + 5; cy = 0x36 + 0x2F * (o[1] - 1) + 5;
            click = 1;
        } else if (getenv("DKE_FRAMES") && !autoplay && ls_tick() - opened > 1500) {
            esc = 1;
        }
        if (esc) { s_result = 2; break; }      /* 0x100035A4 */
        if (click) on_click(cx, cy);
        if (g_turn == 0) break;                /* 0x1000356D */
        render_board();
        draw_message();
        draw_cursor();
        present();
        ls_delay(10);
    }
    screen_free();
}

/* sub_4272B0 / DIVERS op 19. */
i32 menu_hotel_run(void) {
    if (!state || !state->graphic.front_buffer) return s_result;
    size_t bytes = (size_t)state->graphic.width * state->graphic.height * sizeof(u32);
    u32 *view = (u32 *)malloc(bytes);
    if (view) memcpy(view, state->graphic.front_buffer, bytes);
    game_time_freeze(26);                      /* set_display_mode(26) */
    hotel_main();
    sfx_free_slots();                          /* sub_44E46A(0), sub_44E46A(1) */
    game_time_thaw(1, 26);                     /* restore_display_mode(1, 26) */
    if (view) {                                /* the game view comes back */
        memcpy(state->graphic.front_buffer, view, bytes);
        free(view);
    }
    while (!state->system.quit && (state->system.mouse_left || state->system.mouse_right)) {
        ls_update_events(&state->system);
        ls_delay(10);
    }
    return s_result;
}
