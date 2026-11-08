/*
 * model.c — placeholder 3D model + wireframe renderer.
 * See model.h for the module contract.
 *
 * All math is Q16.16 fixed-point using the existing cosine_table
 * (Q14, 4096 entries) from libsys/graphics/shading.h.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libsys/common.h"
#include "libsys/graphics/graphic.h"
#include "libsys/graphics/shading.h"
#include "state.h"
#include "model_ome.h"
#include "model.h"


/* -------------------------------------------------------------------------
 * Test cube: 8 verts, 12 triangles.
 *
 * Model space is a unit cube: coords are Q16.16 in ±0x10000 (±1.0).
 * Coordinate frame: +X right, +Y up, +Z out of the screen.
 * Face indices are wound counter-clockwise when viewed from outside.
 * ------------------------------------------------------------------------- */

#define ONE  0x10000   /* 1.0 in Q16.16 */

i32 model_default_pitch = 300;

static const model_vec3_t CUBE_VERTS[8] = {
    { -ONE, -ONE, -ONE },   /* 0: back-bottom-left  */
    {  ONE, -ONE, -ONE },   /* 1: back-bottom-right */
    {  ONE,  ONE, -ONE },   /* 2: back-top-right    */
    { -ONE,  ONE, -ONE },   /* 3: back-top-left     */
    { -ONE, -ONE,  ONE },   /* 4: front-bottom-left */
    {  ONE, -ONE,  ONE },   /* 5: front-bottom-right*/
    {  ONE,  ONE,  ONE },   /* 6: front-top-right   */
    { -ONE,  ONE,  ONE },   /* 7: front-top-left    */
};

/* Colours packed 0xFF000000 | (B<<16) | (G<<8) | R to match blit byte order. */
#define RGB_TO_XRGB(r, g, b) \
    ((u32)0xFF000000u | ((u32)(b) << 16) | ((u32)(g) << 8) | (u32)(r))

static const model_face_t CUBE_FACES[12] = {
    /* Front (+Z)  — bright red */
    { RGB_TO_XRGB(220,  60,  60), 4, 5, 6, 0 },
    { RGB_TO_XRGB(220,  60,  60), 4, 6, 7, 0 },
    /* Back  (-Z)  — dark red */
    { RGB_TO_XRGB(140,  30,  30), 0, 2, 1, 0 },
    { RGB_TO_XRGB(140,  30,  30), 0, 3, 2, 0 },
    /* Right (+X)  — bright green */
    { RGB_TO_XRGB( 60, 220,  60), 1, 2, 6, 0 },
    { RGB_TO_XRGB( 60, 220,  60), 1, 6, 5, 0 },
    /* Left  (-X)  — dark green */
    { RGB_TO_XRGB( 30, 140,  30), 0, 4, 7, 0 },
    { RGB_TO_XRGB( 30, 140,  30), 0, 7, 3, 0 },
    /* Top   (+Y)  — bright blue */
    { RGB_TO_XRGB( 60,  60, 220), 3, 7, 6, 0 },
    { RGB_TO_XRGB( 60,  60, 220), 3, 6, 2, 0 },
    /* Bot   (-Y)  — dark blue */
    { RGB_TO_XRGB( 30,  30, 140), 0, 1, 5, 0 },
    { RGB_TO_XRGB( 30,  30, 140), 0, 5, 4, 0 },
};

static const model_t CUBE = {
    CUBE_VERTS, 8,
    CUBE_FACES, 12,
};

const model_t *model_test_cube(void) { return &CUBE; }

/* -------------------------------------------------------------------------
 * Renderer
 * ------------------------------------------------------------------------- */

/*
 * Projected vertex — screen (x, y) plus depth for the Z-test.
 * depth is packed to fit the Z bmap's u8 space (0=near, 255=far) so it can
 * be compared directly against z_bmap pixels.
 */
typedef struct {
    i32 sx, sy;   /* pixel coords in the front buffer */
    i32 depth;   /* clamped to 0..255 */
} proj_vertex_t;

/*
 * Sample the light map at (fb_x, fb_y). Returns 128 (neutral) when l_bmap
 * is NULL or coords are out of bounds — no shading applied.
 * L is grayscale palette (index == brightness), quarter-res upscale ×2.
 */
static inline u32 sample_light(const bmap_t *l_bmap, i32 fb_x, i32 fb_y,
                                i32 fb_w, i32 fb_h) {
    if (!l_bmap || !l_bmap->pixels) return 128;
    i32 lw = (i32)l_bmap->width;
    i32 lh = l_bmap->height;
    i32 lx = (fb_x * lw) / fb_w;
    i32 ly = (fb_y * lh) / fb_h;
    if ((u32)lx >= (u32)lw || (u32)ly >= (u32)lh) return 128;
    u32 l = l_bmap->pixels[ly * lw + lx];
    /* Floor at 48 so the model doesn't vanish in unlit corners.
     * Removed once we understand the L semantics per real .OME mesh. */
    if (l < 48) l = 48;
    return l;
}

/*
 * Sample scene depth. Returns 255 (far) when no Z bmap, so the model
 * always draws in front — that keeps the wireframe visible on backdrops
 * that ship without a Z layer.
 */
static inline u32 sample_depth(const bmap_t *z_bmap, i32 fb_x, i32 fb_y) {
    if (!z_bmap || !z_bmap->pixels) return 255;
    i32 zw = (i32)z_bmap->width;
    i32 zh = z_bmap->height;
    if ((u32)fb_x >= (u32)zw || (u32)fb_y >= (u32)zh) return 255;
    return z_bmap->pixels[fb_y * zw + fb_x];
}

/*
 * Modulate an XRGB colour by an L (0..255) sample. Formula matches the
 * era-standard 2× lighting convention: L=128 → identity, L=255 → 2× (clamped).
 */
static inline u32 apply_light(u32 col, u32 l) {
    u32 r = (col >>  0) & 0xFF;
    u32 g = (col >>  8) & 0xFF;
    u32 b = (col >> 16) & 0xFF;
    r = (r * l) >> 7;   if (r > 255) r = 255;
    g = (g * l) >> 7;   if (g > 255) g = 255;
    b = (b * l) >> 7;   if (b > 255) b = 255;
    return 0xFF000000u | (b << 16) | (g << 8) | r;
}

/*
 * Write one pixel with depth + light. depth is the model pixel's projected
 * depth (0..255). Only draws when depth < scene depth (model in front).
 */
/* Engine depth mode (render_entity_with_ui): the backdrop Z image stores
 * 0xFE (near) .. 0 (far); an actor carries one value of the same scale and
 * is visible where it is not behind the backdrop. -1 = off. */
static i32 g_actor_z = -1;

/* Object bake (sub_4762BF): pixels the object covers also write its depth
 * into a Z image, so actors drawn later are hidden behind it. */
static bmap_t *g_zstamp = NULL;
static i32     g_zstamp_val = 0;

/* Projection centre of model_render_batch (the 640x480 screen centre; the
 * inventory windows move it). */
static i32 g_model_cx = 320, g_model_cy = 240;
void model_set_center(i32 cx, i32 cy) { g_model_cx = cx; g_model_cy = cy; }

void model_set_zstamp(bmap_t *zb, i32 value) {
    g_zstamp = (value >= 0) ? zb : NULL;
    g_zstamp_val = value;
}

static void plot_pixel(u32 *fb, i32 fb_w, i32 fb_h,
                       i32 x, i32 y, i32 depth,
                       u32 colour,
                       const bmap_t *l_bmap, const bmap_t *z_bmap) {
    if ((u32)x >= (u32)fb_w || (u32)y >= (u32)fb_h) return;
    if (g_actor_z >= 0) {
        if (z_bmap && (u32)g_actor_z < sample_depth(z_bmap, x, y)) return;
        fb[y * fb_w + x] = colour;
        if (g_zstamp && g_zstamp->pixels && x < g_zstamp->width && y < g_zstamp->height)
            g_zstamp->pixels[y * g_zstamp->width + x] = (u8)g_zstamp_val;
        return;
    }
    u32 scene_z = sample_depth(z_bmap, x, y);
    if ((u32)depth > scene_z) return;   /* behind scene → skip */
    u32 l = sample_light(l_bmap, x, y, fb_w, fb_h);
    fb[y * fb_w + x] = apply_light(colour, l);
}

/*
 * Draw a horizontal scanline with linear depth interpolation.
 * xL/xR are inclusive left/right edges; zL/zR are depths at those edges.
 */
static void draw_scanline(u32 *fb, i32 fb_w, i32 fb_h,
                          i32 y, i32 xL, i32 xR, i32 zL, i32 zR,
                          u32 colour,
                          const bmap_t *l_bmap, const bmap_t *z_bmap) {
    if (y < 0 || y >= fb_h) return;
    if (xL > xR) {
        i32 t = xL; xL = xR; xR = t;
        i32 tz = zL; zL = zR; zR = tz;
    }
    i32 span = xR - xL;
    if (span < 0) return;
    i32 dz = zR - zL;

    /* Clip to fb bounds while keeping z interpolation correct. */
    i32 start = xL;
    i32 end   = xR;
    if (start < 0)       start = 0;
    if (end   >= fb_w)   end   = fb_w - 1;
    if (start > end)     return;

    for (i32 x = start; x <= end; x++) {
        i32 depth = span > 0 ? zL + (dz * (x - xL)) / span : zL;
        plot_pixel(fb, fb_w, fb_h, x, y, depth, colour, l_bmap, z_bmap);
    }
}

/*
 * Solid triangle fill with linear depth interpolation.  Splits at the
 * middle vertex into a flat-bottom + flat-top pair and walks scanlines
 * between the interpolated left/right edges.  Depth is interpolated
 * along each edge in Q16.16 so per-pixel Z-tests get sub-scanline
 * accuracy on tall triangles.
 */
static void fill_triangle(u32 *fb, i32 fb_w, i32 fb_h,
                          i32 x0, i32 y0, i32 z0,
                          i32 x1, i32 y1, i32 z1,
                          i32 x2, i32 y2, i32 z2,
                          u32 colour,
                          const bmap_t *l_bmap, const bmap_t *z_bmap) {
    /* Sort by y ascending: (x0,y0,z0) top, (x1,y1,z1) middle, (x2,y2,z2) bottom. */
    #define SWAP_VERT(A, B) do {                              \
        i32 _t; _t = x##A; x##A = x##B; x##B = _t;             \
                _t = y##A; y##A = y##B; y##B = _t;             \
                _t = z##A; z##A = z##B; z##B = _t;             \
    } while (0)
    if (y0 > y1) SWAP_VERT(0, 1);
    if (y0 > y2) SWAP_VERT(0, 2);
    if (y1 > y2) SWAP_VERT(1, 2);
    #undef SWAP_VERT

    if (y2 == y0) return;   /* degenerate — collapsed triangle */

    /* Edge (0→2) is the long edge that spans the whole triangle.
     * Compute the x + z on that edge at the middle vertex's y. */
    i32 total_dy = y2 - y0;
    i32 mid_dy   = y1 - y0;
    i32 mid_edge_x = x0 + ((x2 - x0) * mid_dy) / total_dy;
    i32 mid_edge_z = z0 + ((z2 - z0) * mid_dy) / total_dy;

    /* Upper half: y0..y1. */
    if (y1 != y0) {
        i32 dy_up = y1 - y0;
        for (i32 y = y0; y <= y1; y++) {
            i32 t = y - y0;
            i32 xL = x0 + ((x1 - x0)     * t) / dy_up;
            i32 xR = x0 + ((mid_edge_x - x0) * t) / dy_up;
            i32 zL = z0 + ((z1 - z0)     * t) / dy_up;
            i32 zR = z0 + ((mid_edge_z - z0) * t) / dy_up;
            draw_scanline(fb, fb_w, fb_h, y, xL, xR, zL, zR,
                          colour, l_bmap, z_bmap);
        }
    }

    /* Lower half: y1..y2. */
    if (y2 != y1) {
        i32 dy_lo = y2 - y1;
        for (i32 y = y1; y <= y2; y++) {
            i32 t = y - y1;
            i32 xL = x1        + ((x2 - x1)        * t) / dy_lo;
            i32 xR = mid_edge_x + ((x2 - mid_edge_x) * t) / dy_lo;
            i32 zL = z1        + ((z2 - z1)        * t) / dy_lo;
            i32 zR = mid_edge_z + ((z2 - mid_edge_z) * t) / dy_lo;
            draw_scanline(fb, fb_w, fb_h, y, xL, xR, zL, zR,
                          colour, l_bmap, z_bmap);
        }
    }
}

void model_render(const model_t *m, system_t *sys,
                  i32 mx, i32 my, i32 scale, u32 tick,
                  i32 world_z, i32 world_size, i32 z_far,
                  const bmap_t *l_bmap, const bmap_t *z_bmap,
                  i32 rx_override, i32 ry_override, i32 rz_override,
                  i32 atom_tx, i32 atom_ty, i32 atom_tz) {
    (void)sys;
    if (!m || !state) return;

    u32 *fb   = state->graphic.front_buffer;
    i32  fb_w = state->graphic.width;
    i32  fb_h = state->graphic.height;
    if (!fb) return;

    /* Rotation angles in cosine_table index units (0..4095 = 360°).
     * When rx_override != 0, use explicit per-atom angles from CAN keyframe.
     * Otherwise use tick-driven spin + fixed pitch camera tilt. */
    i32 yaw, pitch, roll;
    if (rx_override) {
        yaw   = ry_override & 0xFFF;   /* Y rotation = yaw */
        pitch = rx_override & 0xFFF;   /* X rotation = pitch */
        roll  = rz_override & 0xFFF;   /* Z rotation = roll */
    } else {
        yaw   = (i32)((tick / 4u) & 0xFFF);
        pitch = model_default_pitch;    /* from L%02d.INF camera projection */
        roll  = 0;
    }
    (void)roll; /* roll not yet applied in projection */

    i32 cy = cosine_table[yaw];
    i32 sy = cosine_table[(yaw   + 1024) & 0xFFF];
    i32 cp = cosine_table[pitch];
    i32 sp = cosine_table[(pitch + 1024) & 0xFFF];

    /* Apply camera rotation to the atom translation, then project to
     * a screen offset and adjust the render centre.
     * Matches the vertex-transform path: rotY → rotX → perspective.
     * All values are Q14 fixed-point from cosine_table. */
    if (atom_tx | atom_ty | atom_tz) {
        i64 rx = ((i64)cy * atom_tx + (i64)sy * atom_tz) >> 14;
        i64 rz = (-(i64)sy * atom_tx + (i64)cy * atom_tz) >> 14;
        i64 ry = atom_ty;
        i64 py = ( (i64)cp * ry - (i64)sp * rz) >> 14;
        i64 pz = ( (i64)sp * ry + (i64)cp * rz) >> 14;
        const i32 CAM_D_local = 4 * ONE;
        i32 denom = CAM_D_local - (i32)pz;
        if (denom < ONE / 8) denom = ONE / 8;
        mx += (i32)(((i64)rx * scale) / denom);
        my -= (i32)(((i64)py * scale) / denom);
    }

    /* Perspective projection.
     * Camera sits at (0, 0, +CAM_D) in Q16.16 model space, looking down -Z.
     * screen_x = mx + (vx * scale) / (CAM_D - vz)
     * screen_y = my - (vy * scale) / (CAM_D - vz)
     * CAM_D chosen so a unit cube fills a comfortable centre area. Larger
     * CAM_D → flatter perspective; smaller → more foreshortening. */
    const i32 CAM_D = 4 * ONE;   /* 4 units back from origin */

    proj_vertex_t projected[64];
    if (m->vertex_count > (i32)(sizeof(projected)/sizeof(projected[0]))) return;

    for (i32 i = 0; i < m->vertex_count; i++) {
        i32 vx = m->vertices[i].x;
        i32 vy = m->vertices[i].y;
        i32 vz = m->vertices[i].z;

        /* rotY (yaw): x' =  cy*x + sy*z ; z' = -sy*x + cy*z */
        i64 rx = ((i64)cy * vx + (i64)sy * vz) >> 14;
        i64 rz = (-(i64)sy * vx + (i64)cy * vz) >> 14;
        i64 ry = vy;

        /* rotX (pitch): y' = cp*y - sp*z ; z' =  sp*y + cp*z */
        i64 py = ( (i64)cp * ry - (i64)sp * rz) >> 14;
        i64 pz = ( (i64)sp * ry + (i64)cp * rz) >> 14;

        i32 mx_v = (i32)rx;
        i32 my_v = (i32)py;
        i32 mz_v = (i32)pz;

        /* Perspective divide. Keep everything Q16.16 while dividing so the
         * projected pixel stays integer at the end. Guard against
         * degenerate depth (should not happen for a unit cube with
         * CAM_D >= 3).  Nearer verts (larger mz_v) → bigger denom
         * reciprocal → bigger projected offset. */
        i32 denom = CAM_D - mz_v;
        if (denom < ONE / 8) denom = ONE / 8;   /* clamp very-near */
        i64 proj_x = ((i64)mx_v * scale) / denom;
        i64 proj_y = ((i64)my_v * scale) / denom;

        projected[i].sx = mx + (i32)proj_x;
        projected[i].sy = my - (i32)proj_y;
        /* Depth: map the vertex into world units, then normalise against
         * the scene's zfar so we can compare directly against the 8-bit
         * Z bmap. World depth = world_z - vz_world (positive mz_v =
         * closer to camera → smaller depth). world_size is the world
         * half-extent that model space's ±ONE covers.
         *
         *   vz_world     = (mz_v * world_size) / ONE
         *   world_depth  = world_z - vz_world
         *   pixel_depth  = clamp(world_depth * 255 / zfar, 0, 255)
         *
         * When z_far <= 0 or world_size <= 0 we degrade to a fixed
         * 128-mid depth so the model still renders (behaves like the
         * previous "Z-off" path). */
        i32 d;
        if (z_far > 0 && world_size > 0) {
            i64 vz_world    = ((i64)mz_v * world_size) >> 16;
            i64 world_depth = (i64)world_z - vz_world;
            i64 pix         = (world_depth * 255) / (i64)z_far;
            if (pix < 0)   pix = 0;
            if (pix > 255) pix = 255;
            d = (i32)pix;
        } else {
            d = 128 - ((i32)((i64)mz_v * 127 >> 16));
            if (d < 0)   d = 0;
            if (d > 255) d = 255;
        }
        projected[i].depth = d;
    }

    /* Solid triangle fill with backface culling. Winding: model faces are
     * CCW in model space. Our projection flips Y (screen down), which
     * inverts winding, so a front face is CCW in screen space and its
     * signed area (cross_z / 2) is NEGATIVE. Trace: face {4,5,6} at
     * angle=0, scale=80, centre=(320,240) →
     *   a=(240,320) b=(400,320) c=(400,160)
     *   cross_z = (400-240)(160-320) - (320-320)(400-240) = -25600
     * so keep cross_z < 0 and cull cross_z >= 0 as back faces. */
    for (i32 f = 0; f < m->face_count; f++) {
        const model_face_t *face = &m->faces[f];
        const proj_vertex_t *a = &projected[face->i0];
        const proj_vertex_t *b = &projected[face->i1];
        const proj_vertex_t *c = &projected[face->i2];

        i64 cross_z = (i64)(b->sx - a->sx) * (i64)(c->sy - a->sy)
                    - (i64)(b->sy - a->sy) * (i64)(c->sx - a->sx);
        if (cross_z >= 0) continue;   /* back face — skip */

        fill_triangle(fb, fb_w, fb_h,
                      a->sx, a->sy, a->depth,
                      b->sx, b->sy, b->depth,
                      c->sx, c->sy, c->depth,
                      face->colour, l_bmap, z_bmap);
    }
}

/* -------------------------------------------------------------------------
 * model_render_world — real in-game camera pipeline (dkev sub_420D80).
 * ------------------------------------------------------------------------- */
#define MRW_MAX_VERTS 2048
#define MRW_MAX_FACES 2048
#define MRW_SHADE_MID 32   /* GRD shade band (0..63) used until per-face lighting */

typedef struct { i32 sx, sy; i64 cx, cy, cz; i32 vis; i32 shade; } mrw_vert_t;

/* Shade and fill one projected face (back-face cull, key-light shade, GRD
 * ramp lookup). Shared by model_render_world and model_render_batch. */
static void mrw_draw_face(const model_face_t *face, const mrw_vert_t *a,
                          const mrw_vert_t *b, const mrw_vert_t *c,
                          u32 *fb, i32 fb_w, i32 fb_h,
                          const bmap_t *l_bmap, const bmap_t *z_bmap,
                          const u8 *pal, const u8 *grd,
                          double Lx, double Ly, double Lz) {
        if (!a->vis && !b->vis && !c->vis) return;    /* all behind near plane */

        /* Backface cull via screen winding (Y flipped -> front faces have
         * negative signed area, matching model_render). */
        i64 cross_z = (i64)(b->sx - a->sx) * (i64)(c->sy - a->sy)
                    - (i64)(b->sy - a->sy) * (i64)(c->sx - a->sx);
        if (cross_z >= 0) return;

        /* Shade level 0..63 from the camera-space face normal dotted with a
         * fixed key light (engine does normal.L). 32 = neutral (base color). */
        i32 shade = MRW_SHADE_MID;
        {
            double ux = (double)(b->cx - a->cx), uy = (double)(b->cy - a->cy), uz = (double)(b->cz - a->cz);
            double vx = (double)(c->cx - a->cx), vy = (double)(c->cy - a->cy), vz = (double)(c->cz - a->cz);
            double nx = uy*vz - uz*vy, ny = uz*vx - ux*vz, nz = ux*vy - uy*vx;
            double nl = nx*nx + ny*ny + nz*nz;
            if (nl > 1.0) {
                nl = 1.0 / sqrt(nl);
                double dot = (nx*Lx + ny*Ly + nz*Lz) * nl;        /* scene key light */
                /* Map dot (-1..1) into the ramp's lit-but-not-blown range.
                 * The GRD ramp darkens below ~16 and blows to pink/white past
                 * ~34 (indices 189/125/130), so keep shade in ~10..33. */
                shade = 22 + (i32)(dot * 11);
                if (shade < 10) shade = 10;
                if (shade > 33) shade = 33;
            }
        }

        /* Face color: material base index (MATR field[0]). With the GRD shade
         * ramp (grd[base*64 + shade] = shaded palette index) this reproduces
         * the engine's baked lighting; else palette[base] with a brightness
         * modulate; else the raw material RGB. */
        u32 col = face->colour;
        if (!face->mat_base) {
            /* Direct-RGB face: light it with the same shade band. */
            double fct = 0.45 + 0.55 * (shade / 63.0) * 2.0;
            if (fct > 1.6) fct = 1.6;
            u32 r = (u32)(( col        & 0xFF) * fct); if (r > 255) r = 255;
            u32 g = (u32)(((col >> 8)  & 0xFF) * fct); if (g > 255) g = 255;
            u32 b = (u32)(((col >> 16) & 0xFF) * fct); if (b > 255) b = 255;
            col = 0xFF000000u | (b << 16) | (g << 8) | r;
        }
        if (pal && face->mat_base) {
            i32 idx = face->mat_base & 0xFF;
            if (grd) {
                idx = grd[(idx * 64 + shade) & 0x3FFF];
                col = 0xFF000000u | ((u32)pal[3*idx+2] << 16)
                                  | ((u32)pal[3*idx+1] << 8)
                                  |  (u32)pal[3*idx+0];
            } else {
                double f = 0.45 + 0.55 * (shade / 63.0) * 2.0;
                if (f > 1.6) f = 1.6;
                u32 r  = (u32)(pal[3*idx+0] * f); if (r  > 255) r  = 255;
                u32 g  = (u32)(pal[3*idx+1] * f); if (g  > 255) g  = 255;
                u32 bl = (u32)(pal[3*idx+2] * f); if (bl > 255) bl = 255;
                col = 0xFF000000u | (bl << 16) | (g << 8) | r;
            }
        }

        fill_triangle(fb, fb_w, fb_h,
                      a->sx, a->sy, 128,
                      b->sx, b->sy, 128,
                      c->sx, c->sy, 128,
                      col, l_bmap, z_bmap);
}

/* process_color_palette @ 0x40C2A0: an untextured face's ramp base is
 * color_lookup[R>>4 << 8 | G>>3 << 3 | B>>5] of its RGB (+0x5C), i.e. the
 * nearest colour of the current palette. Cached per palette. */
static i32 palette_nearest(const u8 *pal, u32 colour) {
    static const u8 *s_pal = NULL;
    static i16 s_lut[4096];
    if (pal != s_pal) {
        s_pal = pal;
        for (i32 k = 0; k < 4096; k++) s_lut[k] = -1;
    }
    i32 r = colour & 0xFF, g = (colour >> 8) & 0xFF, b = (colour >> 16) & 0xFF;
    i32 key = ((r >> 4) << 8) | ((g >> 3) << 3) | (b >> 5);
    if (s_lut[key] < 0) {
        i32 qr = (r & 0xF0) + 8, qg = (g & 0xF8) + 4, qb = (b & 0xE0) + 16;
        i32 best = 1, bd = 0x7FFFFFFF;
        for (i32 i = 1; i < 255; i++) {
            i32 dr = pal[3 * i] - qr, dg = pal[3 * i + 1] - qg, db = pal[3 * i + 2] - qb;
            i32 dd = dr * dr + dg * dg + db * db;
            if (dd < bd) { bd = dd; best = i; }
        }
        s_lut[key] = (i16)best;
    }
    return s_lut[key];
}

/* Textured / ramp gouraud triangle: sub_495BFA (texel) and sub_497CD9
 * (colour ramp) in the 8-bit table. u, v and the shade row are interpolated
 * affinely across the screen triangle (no perspective, no z); each pixel is
 * palette[grd[colour * 64 + shade]], colour being the texel or the face's
 * ramp base. */
static void fill_tri_shaded(const model_face_t *face, const mrw_vert_t *A,
                            const mrw_vert_t *B, const mrw_vert_t *C,
                            u32 *fb, i32 fb_w, i32 fb_h, const bmap_t *z_bmap,
                            const u8 *pal, const u8 *grd) {
    const mrw_vert_t *V[3] = { A, B, C };
    f64 u[3], v[3], I[3];
    for (i32 k = 0; k < 3; k++) {
        u[k] = face->tex ? face->tu[k] : 0;
        v[k] = face->tex ? face->tv[k] : 0;
        I[k] = V[k]->shade;
    }
    f64 x0 = A->sx, y0 = A->sy, x1 = B->sx, y1 = B->sy, x2 = C->sx, y2 = C->sy;
    f64 area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
    if (area == 0) return;
    i32 minx = A->sx, maxx = A->sx, miny = A->sy, maxy = A->sy;
    for (i32 k = 1; k < 3; k++) {
        if (V[k]->sx < minx) minx = V[k]->sx;
        if (V[k]->sx > maxx) maxx = V[k]->sx;
        if (V[k]->sy < miny) miny = V[k]->sy;
        if (V[k]->sy > maxy) maxy = V[k]->sy;
    }
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx >= fb_w) maxx = fb_w - 1;
    if (maxy >= fb_h) maxy = fb_h - 1;
    f64 inv = 1.0 / area;
    i32 rows = face->tex_rows > 0 ? face->tex_rows : 256;
    i32 base = face->ramp ? palette_nearest(pal, face->colour) : face->mat_base;
    for (i32 y = miny; y <= maxy; y++) {
        f64 py = y + 0.5;
        for (i32 x = minx; x <= maxx; x++) {
            f64 px = x + 0.5;
            f64 w0 = ((x1 - px) * (y2 - py) - (x2 - px) * (y1 - py)) * inv;
            f64 w1 = ((x2 - px) * (y0 - py) - (x0 - px) * (y2 - py)) * inv;
            f64 w2 = 1.0 - w0 - w1;
            if (w0 < -1e-9 || w1 < -1e-9 || w2 < -1e-9) continue;
            i32 sh = (i32)(w0 * I[0] + w1 * I[1] + w2 * I[2]);
            if (sh < 0) sh = 0;
            if (sh > 63) sh = 63;
            i32 colour;
            if (face->tex) {
                i32 tu = (i32)(w0 * u[0] + w1 * u[1] + w2 * u[2]) & 0xFF;
                i32 tv = (i32)(w0 * v[0] + w1 * v[1] + w2 * v[2]);
                if (tv >= rows) tv = rows - 1;
                if (tv < 0) tv = 0;
                colour = face->tex[tv * 256 + tu];
                if (face->key_on && colour == face->key) continue;   /* 0x495B56 */
            } else {
                colour = base;
            }
            i32 idx = grd[(colour * 64 + sh) & 0x3FFF];
            u32 rgb = 0xFF000000u | ((u32)pal[3 * idx + 2] << 16)
                                  | ((u32)pal[3 * idx + 1] << 8) | (u32)pal[3 * idx];
            plot_pixel(fb, fb_w, fb_h, x, y, 0, rgb, NULL, z_bmap);
        }
    }
}

/* Near plane (0x49A790 / 0x498330): a face with a corner in front of the plane
 * and one behind is cut at cz = -32768 instead of being projected from clamped
 * depths. Returns the clipped polygon (up to 4 corners) in camera space, with
 * the shade row and texel (u, v) interpolated along the cut edges. */
static i32 mrw_clip_near(const model_face_t *face, const mrw_vert_t *in[3],
                         mrw_vert_t out[4], f64 ou[4], f64 ov[4], i32 gscale) {
    const i64 NEAR_Z = -32768;
    i32 n = 0;
    for (i32 k = 0; k < 3; k++) {
        const mrw_vert_t *a = in[k], *b = in[(k + 1) % 3];
        f64 au = face->tex ? face->tu[k] : 0, av = face->tex ? face->tv[k] : 0;
        f64 bu = face->tex ? face->tu[(k + 1) % 3] : 0, bv = face->tex ? face->tv[(k + 1) % 3] : 0;
        if (a->vis) { out[n] = *a; ou[n] = au; ov[n] = av; n++; }
        if (a->vis != b->vis) {
            f64 t = (f64)(NEAR_Z - a->cz) / (f64)(b->cz - a->cz);
            mrw_vert_t *o = &out[n];
            o->cx = a->cx + (i64)((f64)(b->cx - a->cx) * t);
            o->cy = a->cy + (i64)((f64)(b->cy - a->cy) * t);
            o->cz = NEAR_Z - 1;
            o->shade = (a->shade < 0 || b->shade < 0) ? -1 : (i32)(a->shade + (b->shade - a->shade) * t);
            o->vis = 1;
            ou[n] = au + (bu - au) * t;
            ov[n] = av + (bv - av) * t;
            i32 d = (i32)((-o->cz) >> 9); if (d < 1) d = 1;
            f64 sc = ((f64)gscale / 16384.0) / (f64)d;
            o->sx = g_model_cx + (i32)(o->cx * sc + (o->cx < 0 ? -0.5 : 0.5));
            o->sy = g_model_cy - (i32)(o->cy * sc + (o->cy < 0 ? -0.5 : 0.5));
            n++;
        }
    }
    return n;
}

void model_render_world(const model_t *m,
                        const i32 *cam_r, const i32 *cam_t, i32 gscale,
                        i32 obj_wx, i32 obj_wy, i32 obj_wz, i32 facing,
                        i32 vshift, i32 flip_y,
                        i32 atom_tx, i32 atom_ty, i32 atom_tz,
                        const i32 *bone_rot,
                        const bmap_t *l_bmap, const bmap_t *z_bmap,
                        const u8 *pal, const u8 *grd,
                        float lx, float ly, float lz) {
    if (!m || !state) return;
    /* Key light direction (camera space). Fall back to a fixed upper-front
     * light when the caller passes none. */
    double Lx = lx, Ly = ly, Lz = lz;
    { double ln = Lx*Lx + Ly*Ly + Lz*Lz;
      if (ln < 1e-6) { Lx = 0.30; Ly = 0.55; Lz = -0.78; }
      else { ln = 1.0/sqrt(ln); Lx*=ln; Ly*=ln; Lz*=ln; } }
    u32 *fb   = state->graphic.front_buffer;
    i32  fb_w = state->graphic.width;
    i32  fb_h = state->graphic.height;
    if (!fb) return;
    if (m->vertex_count <= 0 || m->vertex_count > MRW_MAX_VERTS) return;
    if (m->face_count   <= 0 || m->face_count   > MRW_MAX_FACES) return;

    i32 cyf = cosine_table[facing & 0xFFF];
    i32 syf = cosine_table[(facing + 1024) & 0xFFF];

    /* Scale bone translation to the same world units as the vertices. OBJC
     * bone coords are raw OME; model_t vertices are OME<<1, so lift the bone
     * to model units (<<1) then apply the same vshift. */
    #define MRW_SCALE(v)  (vshift >= 0 ? ((v) >> vshift) : ((v) << (-vshift)))
    i32 sax = MRW_SCALE(atom_tx << 1);
    i32 say = MRW_SCALE(atom_ty << 1);
    i32 saz = MRW_SCALE(atom_tz << 1);

    static mrw_vert_t P[MRW_MAX_VERTS];
    for (i32 i = 0; i < m->vertex_count; i++) {
        /* model_t vertices are OME_raw<<1 (Q16.16-ish). vshift brings them to
         * world units (vshift=1 => OME units == world units 1:1). */
        i32 ox, oy, oz;
        if (vshift >= 0) {
            ox = m->vertices[i].x >> vshift;
            oy = m->vertices[i].y >> vshift;
            oz = m->vertices[i].z >> vshift;
        } else {
            ox = m->vertices[i].x << (-vshift);
            oy = m->vertices[i].y << (-vshift);
            oz = m->vertices[i].z << (-vshift);
        }

        /* Skeletal pose: rotate the atom's vertices by the bone's accumulated
         * world rotation (Q14 row-major), so the limb geometry follows the
         * animation, not just its pivot. Then offset to the bone world pos. */
        if (bone_rot) {
            i32 bx = (i32)(( (i64)bone_rot[0]*ox + (i64)bone_rot[1]*oy + (i64)bone_rot[2]*oz) >> 14);
            i32 by = (i32)(( (i64)bone_rot[3]*ox + (i64)bone_rot[4]*oy + (i64)bone_rot[5]*oz) >> 14);
            i32 bz = (i32)(( (i64)bone_rot[6]*ox + (i64)bone_rot[7]*oy + (i64)bone_rot[8]*oz) >> 14);
            ox = bx; oy = by; oz = bz;
        }
        /* Character-local posed vertex (bone pivot in the same local frame). */
        i32 lx0 = ox + sax, ly0 = oy + say, lz0 = oz + saz;
        if (flip_y) ly0 = -ly0;

        /* Character facing (yaw about world Y). */
        i32 rx = (i32)(( (i64)lx0 * cyf + (i64)lz0 * syf) >> 14);
        i32 rz = (i32)((-(i64)lx0 * syf + (i64)lz0 * cyf) >> 14);
        i32 ry = ly0;

        /* Object -> world. */
        i32 wx = obj_wx + rx, wy = obj_wy + ry, wz = obj_wz + rz;

        /* World -> camera (Q14), R(row,col)=cam_r[row+col*3]; T added直接. */
        i64 cx = (( (i64)wx*cam_r[0] + (i64)wy*cam_r[3] + (i64)wz*cam_r[6] ) >> 14) + cam_t[0];
        i64 cy = (( (i64)wx*cam_r[1] + (i64)wy*cam_r[4] + (i64)wz*cam_r[7] ) >> 14) + cam_t[1];
        i64 cz = (( (i64)wx*cam_r[2] + (i64)wy*cam_r[5] + (i64)wz*cam_r[8] ) >> 14) + cam_t[2];

        i32 vis = (cz < -32768) ? 1 : 0;
        i64 czc = (cz < -32768) ? cz : -32768;
        i32 d = (i32)((-czc) >> 9); if (d < 1) d = 1;
        double s = ((double)gscale / 16384.0) / (double)d;
        P[i].sx = 320 + (i32)(cx * s + (cx < 0 ? -0.5 : 0.5));
        P[i].sy = 240 - (i32)(cy * s + (cy < 0 ? -0.5 : 0.5));
        P[i].cx = cx; P[i].cy = cy; P[i].cz = cz;
        P[i].vis = vis;
    }

    /* Painter's order: face key = sum of vertex camera depths (cz, negative =
     * far). Draw most-negative first (back-to-front). Simple insertion sort. */
    static i32 order[MRW_MAX_FACES];
    static i64 key[MRW_MAX_FACES];
    i32 nf = 0;
    for (i32 f = 0; f < m->face_count; f++) {
        const model_face_t *face = &m->faces[f];
        key[nf]   = P[face->i0].cz + P[face->i1].cz + P[face->i2].cz;
        order[nf] = f;
        i32 j = nf - 1;
        while (j >= 0 && key[j] > key[j+1]) {           /* ascending: far first */
            i64 tk = key[j]; key[j] = key[j+1]; key[j+1] = tk;
            i32 to = order[j]; order[j] = order[j+1]; order[j+1] = to;
            j--;
        }
        nf++;
    }

    if (getenv("DKE_MESHDBG")) {
        static i32 shown = 0;
        if (shown < 8) {
            shown++;
            i32 minx = 99999, maxx = -99999, miny = 99999, maxy = -99999, nv = 0;
            for (i32 i = 0; i < m->vertex_count; i++) {
                if (P[i].sx < minx) minx = P[i].sx;
                if (P[i].sx > maxx) maxx = P[i].sx;
                if (P[i].sy < miny) miny = P[i].sy;
                if (P[i].sy > maxy) maxy = P[i].sy;
                if (P[i].vis) nv++;
            }
            print("mesh: verts=%d vis=%d faces=%d bbox x[%d..%d] y[%d..%d] (%dx%d px)\n",
                  m->vertex_count, nv, m->face_count, minx, maxx, miny, maxy,
                  maxx - minx, maxy - miny);
        }
    }

    for (i32 oi = 0; oi < nf; oi++) {
        const model_face_t *face = &m->faces[order[oi]];
        mrw_draw_face(face, &P[face->i0], &P[face->i1], &P[face->i2],
                      fb, fb_w, fb_h, l_bmap, z_bmap, pal, grd, Lx, Ly, Lz);
    }
}

/* -------------------------------------------------------------------------
 * model_render_batch — draw several meshes, each with its own camera-space
 * transform, as ONE painter-sorted face list so overlapping limbs of a
 * character order correctly against each other.
 * ------------------------------------------------------------------------- */
#define MRB_MAX_VERTS 8192
#define MRB_MAX_FACES 8192

void model_render_batch(const model_draw_t *items, i32 n_items, i32 gscale,
                        const bmap_t *l_bmap, const bmap_t *z_bmap,
                        const u8 *pal, const u8 *grd,
                        float lx, float ly, float lz) {
    if (!items || n_items <= 0 || !state || !state->graphic.front_buffer) return;
    u32 *fb   = state->graphic.front_buffer;
    i32  fb_w = state->graphic.width;
    i32  fb_h = state->graphic.height;
    double Lx = lx, Ly = ly, Lz = lz;
    { double ln = Lx*Lx + Ly*Ly + Lz*Lz;
      if (ln < 1e-6) { Lx = 0.30; Ly = 0.55; Lz = -0.78; }
      else { ln = 1.0/sqrt(ln); Lx*=ln; Ly*=ln; Lz*=ln; } }

    static mrw_vert_t P[MRB_MAX_VERTS];
    static struct { const model_face_t *f; i32 base; i64 key; i32 z; } F[MRB_MAX_FACES];
    i32 nv = 0, nf = 0;

    for (i32 it = 0; it < n_items; it++) {
        const model_t *m = items[it].m;
        const i32 *M = items[it].m3, *T = items[it].t;
        if (!m || nv + m->vertex_count > MRB_MAX_VERTS) continue;
        i32 base = nv;
        for (i32 i = 0; i < m->vertex_count; i++, nv++) {
            /* v' = transpose(M) * v + T  (transform_vector @ 0x48F424) */
            i64 vx = m->vertices[i].x, vy = m->vertices[i].y, vz = m->vertices[i].z;
            i64 cx = ((vx*M[0] + vy*M[3] + vz*M[6]) >> 14) + T[0];
            i64 cy = ((vx*M[1] + vy*M[4] + vz*M[7]) >> 14) + T[1];
            i64 cz = ((vx*M[2] + vy*M[5] + vz*M[8]) >> 14) + T[2];
            i32 vis = (cz < -32768) ? 1 : 0;
            i64 czc = (cz < -32768) ? cz : -32768;
            i32 d = (i32)((-czc) >> 9); if (d < 1) d = 1;
            double sc = ((double)gscale / 16384.0) / (double)d;
            P[nv].sx = g_model_cx + (i32)(cx * sc + (cx < 0 ? -0.5 : 0.5));
            P[nv].sy = g_model_cy - (i32)(cy * sc + (cy < 0 ? -0.5 : 0.5));
            P[nv].cx = cx; P[nv].cy = cy; P[nv].cz = cz;
            P[nv].vis = vis;
            P[nv].shade = -1;
            /* 0x421710: d = n.L (Q14 x Q14); a normal facing the light gets
             * full strength, one facing away half; row = (I >> 16) & 0xFF. */
            if (items[it].light_s >= 0 && m->normals) {
                i64 nx = m->normals[i].x, ny = m->normals[i].y, nz = m->normals[i].z;
                i64 ncx = (nx*M[0] + ny*M[3] + nz*M[6]) >> 14;
                i64 ncy = (nx*M[1] + ny*M[4] + nz*M[7]) >> 14;
                i64 ncz = (nx*M[2] + ny*M[5] + nz*M[8]) >> 14;
                const i32 *L = items[it].light;
                i64 d = ncx * L[0] + ncy * L[1] + ncz * L[2];
                i64 S = items[it].light_s;
                i64 I = d > 0 ? ((d >> 6) * S) >> 9 : (((-d) >> 6) * S) >> 8;
                i32 row = (i32)((I >> 16) & 0xFF);
                P[nv].shade = row > 63 ? 63 : row;
            }
        }
        for (i32 f = 0; f < m->face_count && nf < MRB_MAX_FACES; f++, nf++) {
            const model_face_t *face = &m->faces[f];
            F[nf].f = face;
            F[nf].base = base;
            F[nf].z = items[it].zval;
            F[nf].key = P[base + face->i0].cz + P[base + face->i1].cz + P[base + face->i2].cz;
        }
    }

    /* Far first (most negative camera z). */
    for (i32 i = 1; i < nf; i++) {
        __typeof__(F[0]) t = F[i];
        i32 j = i - 1;
        while (j >= 0 && F[j].key > t.key) { F[j + 1] = F[j]; j--; }
        F[j + 1] = t;
    }
    for (i32 i = 0; i < nf; i++) {
        const model_face_t *face = F[i].f;
        i32 b = F[i].base;
        g_actor_z = F[i].z;
        const mrw_vert_t *va = &P[b + face->i0], *vb = &P[b + face->i1], *vc = &P[b + face->i2];
        if (va->vis + vb->vis + vc->vis == 1 || va->vis + vb->vis + vc->vis == 2) {
            /* Crosses the near plane: cut it, then draw the pieces. */
            const mrw_vert_t *in[3] = { va, vb, vc };
            mrw_vert_t cp[4];
            f64 cu[4], cvv[4];
            i32 nc = mrw_clip_near(face, in, cp, cu, cvv, gscale);
            for (i32 k = 1; k + 1 < nc; k++) {
                const mrw_vert_t *tri[3] = { &cp[0], &cp[k], &cp[k + 1] };
                model_face_t ff = *face;
                if (face->tex)
                    for (i32 q = 0; q < 3; q++) {
                        i32 idx = q == 0 ? 0 : k + q - 1;
                        f64 uu = cu[idx], vv = cvv[idx];
                        ff.tu[q] = (u8)((i32)uu & 0xFF);
                        ff.tv[q] = (u16)(vv < 0 ? 0 : (i32)vv);
                    }
                i64 cross_z = (i64)(tri[1]->sx - tri[0]->sx) * (i64)(tri[2]->sy - tri[0]->sy)
                            - (i64)(tri[1]->sy - tri[0]->sy) * (i64)(tri[2]->sx - tri[0]->sx);
                if (pal && grd && (face->tex || face->ramp || face->mat_base) &&
                    tri[0]->shade >= 0 && tri[1]->shade >= 0 && tri[2]->shade >= 0) {
                    if (cross_z >= 0 && !face->two_sided) continue;
                    fill_tri_shaded(&ff, tri[0], tri[1], tri[2], fb, fb_w, fb_h, z_bmap, pal, grd);
                } else {
                    mrw_draw_face(&ff, tri[0], tri[1], tri[2], fb, fb_w, fb_h, l_bmap, z_bmap,
                                  pal, grd, Lx, Ly, Lz);
                }
            }
            continue;
        }
        if (pal && grd && (face->tex || face->ramp || face->mat_base) &&
            va->shade >= 0 && vb->shade >= 0 && vc->shade >= 0) {
            if (!va->vis && !vb->vis && !vc->vis) continue;
            i64 cross_z = (i64)(vb->sx - va->sx) * (i64)(vc->sy - va->sy)
                        - (i64)(vb->sy - va->sy) * (i64)(vc->sx - va->sx);
            if (cross_z >= 0 && !face->two_sided) continue;
            fill_tri_shaded(face, va, vb, vc, fb, fb_w, fb_h, z_bmap, pal, grd);
            continue;
        }
        mrw_draw_face(face, va, vb, vc,
                      fb, fb_w, fb_h, l_bmap, z_bmap, pal, grd, Lx, Ly, Lz);
    }
    g_actor_z = -1;
}

/* -------------------------------------------------------------------------
 * model_from_ome_atom — build a render-ready model_t from ome_atom_t.
 * See model.h for contract.
 * ------------------------------------------------------------------------- */

/* Scale raw .OME vertex coords to Q16.16: shift left 1 bit.
 * Max raw value ~30000 → ~60000 ≈ ONE (65536), keeping proportions intact. */
#define OME_VERT_SCALE(v) ((v) << 1)

/* Placeholder colour palette indexed by mat_id & 0x0F. */
static const u32 MAT_COLOURS[16] = {
    RGB_TO_XRGB(180, 160, 140), /* 0  default tan    */
    RGB_TO_XRGB(200, 180, 160), /* 1  skin / light   */
    RGB_TO_XRGB(100,  80,  60), /* 2  dark leather   */
    RGB_TO_XRGB(140, 120, 100), /* 3  leather (CUIR) */
    RGB_TO_XRGB(160, 160, 170), /* 4  steel (ACIER)  */
    RGB_TO_XRGB(110,  90,  70), /* 5  rust  (ROUIL)  */
    RGB_TO_XRGB( 80, 100,  60), /* 6  green cloth    */
    RGB_TO_XRGB(200, 200, 210), /* 7  silver         */
    RGB_TO_XRGB( 60,  60,  80), /* 8  dark metal     */
    RGB_TO_XRGB(220, 200, 180), /* 9  pale skin      */
    RGB_TO_XRGB(120,  80,  40), /* 10 dark leather   */
    RGB_TO_XRGB(170, 140, 110), /* 11 mid leather    */
    RGB_TO_XRGB(190, 170, 150), /* 12 light leather  */
    RGB_TO_XRGB(140, 130, 120), /* 13 grey cloth     */
    RGB_TO_XRGB(100, 110, 130), /* 14 blue-grey      */
    RGB_TO_XRGB(160,  50,  50), /* 15 red accent     */
};

model_t *model_from_ome_atom(const ome_atom_t *atom, const ome_matr_t *matr) {
    if (!atom || atom->vert_count <= 0 || atom->face_count <= 0) return NULL;

    /* Quads become 2 triangles; pre-compute max triangle count. */
    i32 max_tris = 0;
    for (i32 i = 0; i < atom->face_count; i++) {
        max_tris += (atom->faces[i].vcnt == 4) ? 2 : 1;
    }

    model_vec3_t *verts = (model_vec3_t *)malloc(
        (size_t)atom->vert_count * sizeof(model_vec3_t));
    model_face_t *faces = (model_face_t *)calloc(
        (size_t)max_tris, sizeof(model_face_t));
    model_t      *m     = (model_t      *)calloc(1, sizeof(model_t));

    if (!verts || !faces || !m) {
        free(verts); free(faces); free(m);
        return NULL;
    }

    for (i32 i = 0; i < atom->vert_count; i++) {
        verts[i].x = OME_VERT_SCALE(atom->verts[i].x);
        verts[i].y = OME_VERT_SCALE(atom->verts[i].y);
        verts[i].z = OME_VERT_SCALE(atom->verts[i].z);
    }

    if (getenv("DKE_MATDBG")) {
        static i32 shown = 0;
        if (shown++ < 3) {
            print("matdbg: mat_count=%d\n", matr ? matr->mat_count : -1);
            for (i32 k = 0; matr && k < matr->mat_count && k < 8; k++)
                print("  mat[%d] rgb=(%d,%d,%d) '%s'\n", k,
                      matr->mats[k].r, matr->mats[k].g, matr->mats[k].b,
                      matr->mats[k].name);
            i32 bad = 0, zero = 0, hist[8] = {0};
            for (i32 k = 0; k < atom->face_count; k++) {
                i32 id = atom->faces[k].mat_id;
                if (!matr || id < 0 || id >= matr->mat_count) bad++;
                else if (id == 0) zero++;
                if (id >= 0 && id < 8) hist[id]++;
            }
            print("  faces=%d out-of-range=%d mat0=%d hist=[%d %d %d %d %d %d %d %d]\n",
                  atom->face_count, bad, zero, hist[0], hist[1], hist[2], hist[3],
                  hist[4], hist[5], hist[6], hist[7]);
        }
    }

    i32 tri = 0;
    for (i32 i = 0; i < atom->face_count; i++) {
        const ome_face_t *f = &atom->faces[i];
        u32 colour;
        u8  mat_base = 0;
        /* mat_id -1 means "no material" — those faces are not drawn. They
         * were being painted with MAT_COLOURS[0], a bright debug colour, and
         * showed up as white shards stuck to the characters. */
        if (f->mat_id < 0) continue;
        const ome_mat_t *mt = ome_matr_slot(matr, f->mat_id);
        if (mt) {
            colour = 0xFF000000u | ((u32)mt->b << 16) | ((u32)mt->g << 8) | (u32)mt->r;
        } else {
            /* No MATR (debug meshes): fall back to the placeholder palette. */
            colour = MAT_COLOURS[f->mat_id & 0x0F];
        }

        faces[tri].colour   = colour;
        faces[tri].i0       = f->idx[0];
        faces[tri].i1       = f->idx[1];
        faces[tri].i2       = f->idx[2];
        faces[tri].mat_base = mat_base;
        tri++;

        if (f->vcnt == 4) {
            faces[tri].colour   = colour;
            faces[tri].i0       = f->idx[0];
            faces[tri].i1       = f->idx[2];
            faces[tri].i2       = f->idx[3];
            faces[tri].mat_base = mat_base;
            tri++;
        }
    }

    m->vertices     = verts;
    m->vertex_count = atom->vert_count;
    m->faces        = faces;
    m->face_count   = tri;
    return m;
}

void model_free(model_t *m) {
    if (!m) return;
    free((void *)m->vertices);
    free((void *)m->faces);
    free((void *)m->normals);
    free(m);
}

/* -------------------------------------------------------------------------
 * model_from_atm_atom — render mesh for an atm_atom_t (object-tree atom).
 * ------------------------------------------------------------------------- */
static void model_free_void(void *m) { model_free((model_t *)m); }

/* Texel coordinate for one Q16.16 UV component, after sub_422E31's bias by
 * the material rect: c += base << 16; an integer part of 256 becomes 255. */
static i32 atm_texel(i32 c, i32 base) {
    c += base << 16;
    if (((u32)c & 0xFFFF0000u) == 0x01000000u) c -= 0x10000;
    return c >> 16;
}

const model_t *model_from_atm_atom(atm_atom_t *atom, const ome_matr_t *matr) {
    if (!atom) return NULL;
    if (atom->render) return (const model_t *)atom->render;
    if (atom->vert_count <= 0 || atom->face_count <= 0 || atom->vert_count > 255)
        return NULL;
    atm_render_free = model_free_void;

    i32 max_tris = 0;
    for (i32 i = 0; i < atom->face_count; i++)
        max_tris += (atom->faces[i * ATM_FACE_STRIDE + ATM_FACE_VCNT] == 4) ? 2 : 1;

    model_vec3_t *verts = (model_vec3_t *)malloc((size_t)atom->vert_count * sizeof(*verts));
    model_vec3_t *norms = (model_vec3_t *)malloc((size_t)atom->vert_count * sizeof(*norms));
    model_face_t *faces = (model_face_t *)calloc((size_t)max_tris, sizeof(*faces));
    model_t      *m     = (model_t *)calloc(1, sizeof(*m));
    if (!verts || !norms || !faces || !m) { free(verts); free(norms); free(faces); free(m); return NULL; }

    for (i32 i = 0; i < atom->vert_count; i++) {
        const i32 *v = atom->verts + (size_t)i * ATM_VERT_STRIDE;
        verts[i].x = v[ATM_VERT_X];  verts[i].y = v[ATM_VERT_Y];  verts[i].z = v[ATM_VERT_Z];
        norms[i].x = v[ATM_VERT_NX]; norms[i].y = v[ATM_VERT_NY]; norms[i].z = v[ATM_VERT_NZ];
    }
    i32 tri = 0;
    for (i32 i = 0; i < atom->face_count; i++) {
        const i32 *f = atom->faces + (size_t)i * ATM_FACE_STRIDE;
        i32 slot = f[ATM_FACE_MAT], vc = f[ATM_FACE_VCNT];
        if (vc != 3 && vc != 4) continue;
        const ome_mat_t *mt = ome_matr_slot(matr, slot);
        model_face_t fc = {0};
        fc.two_sided = (f[14] & 0x20) == 0;            /* 0x4215A9 */
        if (slot < 0 || !mt) {
            /* No material: gouraud on the colour ramp of the palette index
             * at +0x28 (sub_497CD9); +0x5C holds the same colour as RGB. */
            u32 rgb = (u32)f[23];
            fc.colour = 0xFF000000u | ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb >> 16) & 0xFF);
            fc.ramp = 1;
        } else {
            fc.colour = 0xFF000000u | ((u32)mt->b << 16) | ((u32)mt->g << 8) | (u32)mt->r;
            const u8 *page = matr->bm_pix[mt->bitmap];
            if ((f[ATM_FACE_FLAGS] & 0x0C) == 0x0C && page) {
                /* 0x423014: u += x, v += y (or 256 - h when the rect reaches
                 * the bottom; the texel base then moves down y + h - 256). */
                i32 vb    = (mt->y + mt->h >= 256) ? 256 - mt->h : mt->y;
                i32 shift = (mt->y + mt->h > 256) ? mt->y + mt->h - 256 : 0;
                fc.tex = page;
                fc.key_on = (mt->flags & 0x200) != 0;      /* 0x495C16 */
                fc.key = (u8)(mt->flags & 0xFF);
                fc.tex_rows = matr->bm_h[mt->bitmap];
                const i32 *uv = f + ATM_FACE_UV;
                i32 tu[4], tv[4];
                for (i32 k = 0; k < 4; k++) {
                    tu[k] = atm_texel(uv[k * 2], mt->x) & 0xFF;
                    tv[k] = (atm_texel(uv[k * 2 + 1], vb) & 0xFF) + shift;
                }
                fc.tu[0] = (u8)tu[0]; fc.tu[1] = (u8)tu[1]; fc.tu[2] = (u8)tu[2];
                fc.tv[0] = (u16)tv[0]; fc.tv[1] = (u16)tv[1]; fc.tv[2] = (u16)tv[2];
                if (vc == 4) {
                    /* sub_490620 draws (v0,v1,v2) + (v0,v3,v2); kept as
                     * (v0,v2,v3) here so both halves share one winding. */
                    model_face_t q = fc;
                    q.tu[1] = (u8)tu[2]; q.tv[1] = (u16)tv[2];
                    q.tu[2] = (u8)tu[3]; q.tv[2] = (u16)tv[3];
                    const i32 *ix = f + ATM_FACE_IDX;
                    fc.i0 = (u8)ix[0]; fc.i1 = (u8)ix[1]; fc.i2 = (u8)ix[2];
                    q.i0  = (u8)ix[0]; q.i1  = (u8)ix[2]; q.i2  = (u8)ix[3];
                    faces[tri++] = fc;
                    faces[tri++] = q;
                    continue;
                }
            }
        }
        const i32 *ix = f + ATM_FACE_IDX;
        fc.i0 = (u8)ix[0]; fc.i1 = (u8)ix[1]; fc.i2 = (u8)ix[2];
        faces[tri++] = fc;
        if (vc == 4) {
            model_face_t q = fc;
            q.i0 = (u8)ix[0]; q.i1 = (u8)ix[2]; q.i2 = (u8)ix[3];
            faces[tri++] = q;
        }
    }
    m->vertices = verts; m->vertex_count = atom->vert_count;
    m->faces = faces;    m->face_count = tri;
    m->normals = norms;
    atom->render = m;
    return m;
}
