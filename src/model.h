#pragma once

/*
 * model.h — Placeholder 3D model + software renderer (Slice F pass 1)
 *
 * We can't parse .OME (Object Mesh Editor, "3DITOR v1.3" format) yet — the
 * file layout is nested/chunked with material tags (CUIR, ACIER, ROUIL, ...)
 * and needs its own reverse-engineering slice. This module ships a hard-coded
 * cube instead so the render pipeline (rotation → projection → per-pixel Z
 * depth-test against the scene's Z bmap → L light-map shading) can land and
 * be tested independently of the .OME loader.
 *
 * Once model_load_ome exists, it will populate the same model_t struct, so
 * the renderer stays unchanged.
 */

#include "libsys/common.h"
#include "libsys/system.h"
#include "libsys/graphics/graphic.h"

typedef struct {
    i32 x, y, z;   /* Q16.16 fixed point in model space (-1..1 ≈ ±0x10000) */
} model_vec3_t;

typedef struct {
    /* Face colour as pre-multiplied XRGB (already 0xFFRRGGBB packed to
     * match blit_bmap_to_framebuffer byte order: byte0=R, byte1=G,
     * byte2=B, byte3=alpha). Renderer modulates by L before writing. */
    u32 colour;
    /* Indices into vertices[]. */
    u8  i0, i1, i2;
    /* Material base palette index (MATR field[0]); indexes the GRD shade
     * ramp for palette-correct character coloring. 0 = none. */
    u8  mat_base;
    /* Textured faces (ATM flags & 0xC == 0xC): 256-wide 8bpp page of game
     * palette indices, and each corner's texel (u, row). NULL = untextured. */
    const u8 *tex;
    i32       tex_rows;
    u8        ramp;      /* untextured: ramp base = nearest palette colour */
    u8        key_on;    /* material flag 0x200: texels equal to key are skipped */
    u8        key;
    u8        two_sided; /* ATM +0x38 bit 0x20 clear: never back-face culled */
    u8        tu[3];
    u16       tv[3];
} model_face_t;

typedef struct {
    const model_vec3_t *vertices;
    i32                 vertex_count;
    const model_face_t *faces;
    i32                 face_count;
    const model_vec3_t *normals;   /* per-vertex Q14 normals, or NULL */
} model_t;

/*
 * model_test_cube — a unit cube centered at origin, 8 vertices + 12 faces.
 * Face colours picked to make orientation obvious (red/green/blue axes).
 */
const model_t *model_test_cube(void);

/*
 * model_from_ome_atom — build a render-ready model_t from a decoded ome_atom_t.
 *
 * Quad faces (vcnt==4) are split into 2 triangles.
 * Vertex coords are scaled from raw .OME units to Q16.16 by OME_VERT_SCALE
 * (shift left 1 bit: max ~30000 → ~60000 ≈ ONE), keeping them compatible
 * with model_render's perspective divide.
 * Face colour is looked up via matr (may be NULL; falls back to built-in
 * placeholder palette when NULL or mat_id is out of range).
 *
 * Returns a heap-allocated model_t; call model_free() to release.
 * Returns NULL on alloc failure.
 */
#include "model_ome.h"
model_t *model_from_ome_atom(const ome_atom_t *atom, const ome_matr_t *matr);

/* model_free — release a model_t built by model_from_ome_atom. */
void model_free(model_t *m);

/*
 * model_render — draw a model into the front buffer.
 *
 * Rotation is applied on the Y axis using tick-driven angle (spins the cube
 * so the render is visibly alive).  Vertices are projected orthographically
 * for this pass: (mx, my) is the screen-space centre where the model lands,
 * scale converts model units (Q16.16) into pixels.
 *
 * For each visible pixel, samples the scene depth (z_bmap, 640×480 u8) and
 * only writes when the model's projected depth is closer (smaller u8) than
 * the background depth — that gives the 2.5D "characters go behind the
 * console" effect Dark Earth uses.  Samples the light map (l_bmap, 320×240
 * u8 grayscale) and modulates the face colour so the model picks up scene
 * lighting.
 *
 * Wireframe pass 1: draws each face as three Bresenham line segments.
 * Triangle-fill rasteriser lands in pass 2.
 *
 * When rx_override is non-zero the rx/ry/rz values are used as explicit
 * rotation angles (cosine_table index 0..4095 = 0..360°) instead of the
 * tick-driven yaw+fixed-pitch.  Pass rx_override=0 to use tick spin.
 *
 * atom_tx/ty/tz — character-relative translation of this atom's pivot
 * (raw model units, same scale as vertices).  The camera rotation is
 * applied to this translation so the atom is offset correctly in world
 * space.  Pass (0,0,0) to render at the screen centre (e.g. test cube).
 */
/* Camera pitch for rendering (cosine_table index, 0..4095 = 0..360°).
 * Default 300 ≈ 26°. Set by scene.c from L%02d.INF projection matrix. */
extern i32 model_default_pitch;

void model_render(const model_t   *m,
                  system_t        *sys,
                  i32              mx,        /* screen centre X */
                  i32              my,        /* screen centre Y */
                  i32              scale,     /* focal length in pixels */
                  u32              tick,      /* animation clock ms */
                  i32              world_z,   /* model centre world depth (units) */
                  i32              world_size,/* model half-extent world units */
                  i32              z_far,     /* scene camera zfar (units) */
                  const bmap_t    *l_bmap,    /* optional light map, may be NULL */
                  const bmap_t    *z_bmap,    /* optional depth bmap, may be NULL */
                  i32              rx_override, /* 0 = use tick, else explicit rx */
                  i32              ry_override, /* explicit ry (cosine_table index) */
                  i32              rz_override, /* explicit rz */
                  i32              atom_tx,   /* character-relative translation X */
                  i32              atom_ty,   /* character-relative translation Y */
                  i32              atom_tz);  /* character-relative translation Z */

/*
 * model_render_world — rasterize a mesh atom through the REAL in-game camera
 * (dkev sub_420D80 pipeline). Each OME vertex is rotated by the object facing,
 * translated to its world position, transformed to camera space by the camera
 * matrix, and perspective-projected (1/z). Faces are painter-sorted back-to-
 * front (engine uses summed vertex depth, no per-pixel z-buffer) and flat-
 * filled; scene occlusion via optional z_bmap.
 *
 *   cam_r[9]  camera rotation ints, R(row,col)=cam_r[row+col*3] (Q14)
 *   cam_t[3]  camera translation, added after >>14
 *   gscale    globalScale = round(10240/tan(fov/2))
 *   obj_wx/wy/wz  object world position (world units)
 *   facing    yaw angle (cosine_table index 0..4095)
 *   vshift    right-shift applied to the model_t Q16.16 vertex -> world units
 *   atom_tx/ty/tz  per-atom (bone) world translation, added after facing rotate
 */
void model_render_world(const model_t *m,
                        const i32 *cam_r, const i32 *cam_t, i32 gscale,
                        i32 obj_wx, i32 obj_wy, i32 obj_wz, i32 facing,
                        i32 vshift, i32 flip_y,
                        i32 atom_tx, i32 atom_ty, i32 atom_tz,
                        const i32 *bone_rot,
                        const bmap_t *l_bmap, const bmap_t *z_bmap,
                        const u8 *pal, const u8 *grd,
                        float lx, float ly, float lz);

/* One mesh of a batch: vertices go through v' = transpose(m3) * v + t
 * (Q14 rotation, camera space) before the perspective divide. */
typedef struct {
    const model_t *m;
    i32 m3[9];
    i32 t[3];
    i32 zval;      /* depth vs the backdrop Z image (0xFE near .. 0 far); -1 = none */
    /* Engine lighting (sub_4358D3 / 0x421710): camera-space Q14 light
     * direction and strength S (light-map byte * 0.75). S < 0 = legacy. */
    i32 light[3];
    i32 light_s;
} model_draw_t;

/* Screen point the batch projects the camera axis to (default 320, 240). */
void model_set_center(i32 cx, i32 cy);

/* While value >= 0, every pixel drawn in depth mode also writes `value`
 * into zb (object Z stamp). Pass -1 to stop. */
void model_set_zstamp(bmap_t *zb, i32 value);

void model_render_batch(const model_draw_t *items, i32 n_items, i32 gscale,
                        const bmap_t *l_bmap, const bmap_t *z_bmap,
                        const u8 *pal, const u8 *grd,
                        float lx, float ly, float lz);

#include "obc.h"
/* Build (once) and return the render mesh of an object-tree atom. Vertices
 * keep raw world units; faces take their colour from `matr` like
 * model_from_ome_atom. Cached on atom->render. */
const model_t *model_from_atm_atom(atm_atom_t *atom, const ome_matr_t *matr);
