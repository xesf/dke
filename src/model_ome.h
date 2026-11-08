#pragma once

/*
 * model_ome.h — Dark Earth .OME (Object Mesh Editor) loader.
 *
 * .OME files are the "3DITOR v1.3 / v2.0" nested chunk format used for
 * every 3D asset in the game (level objects, characters, weapons).
 * Structure (empirical, from tools/src/dump_ome.py):
 *
 *   outer MATR   — material table (CUIR, ACIER, ROUIL, ... tags)
 *   nested OBJC  — object container (holds N ATOMF descriptors)
 *   nested ATOMF — per-atom geometry (vertices + faces + material refs)
 *
 * Each chunk begins with:
 *   +0x00  16-byte signature "3DITOR v?.? TAG "  (17 for ATOMF)
 *   +0x10  16-byte dashes or per-chunk header bytes
 *   +0x20  16-byte filename padded to 16
 *   +0x30  48-byte full path (F:\DEV\DKE\...) padded to 48
 *   +0x60  chunk body starts
 *
 * Blocker (documented in PLAN.md): ATOMF bodies contain runtime-process
 * heap pointers (0x009Axxxx style) that were serialised as-is. Any
 * loader that walks the body must skip / re-derive those addresses.
 *
 * This pass exposes an inventory-only API: parse the outer structure and
 * log which chunks are present without decoding the geometry. Enough to
 * drive next-pass field-layout guessing and let scene.c prefetch mesh
 * paths without stalling on the pointer fix-up work.
 */

#include "libsys/common.h"

typedef enum {
    OME_CHUNK_MATR,   /* material table */
    OME_CHUNK_OBJC,   /* object container */
    OME_CHUNK_ATOMF,  /* atom / geometry */
    OME_CHUNK_UNKNOWN
} ome_chunk_kind_e;

typedef struct {
    ome_chunk_kind_e kind;
    i32              offset;      /* byte offset in the source file */
    i32              length;      /* bytes until next chunk / EOF   */
    c8               name[16];    /* filename from the +0x20 field   */
} ome_chunk_t;

typedef struct {
    ome_chunk_t     *chunks;      /* heap-owned; free via model_ome_free */
    i32              chunk_count;
    i32              matr_count;
    i32              objc_count;
    i32              atomf_count;
    /* Raw file kept live for later chunk-body decode passes. */
    u8              *raw;
    i32              raw_size;
} ome_file_t;

/* -------------------------------------------------------------------------
 * MATR material library + embedded texture (sub_44F454 / sub_44F92D)
 *
 * MATR chunk = file + u32[file+8] when the file starts with "CTX3", else 0:
 *   +0x000  signature "3DITOR v1.3 MATR----------------"
 *   +0x020  library header (0x160): name[16], ..., +0x144 nmaterials,
 *           +0x148 nslots (64), +0x14C nbitmaps (1)
 *   +0x180  nbitmaps x 0x114 bitmap entries (source TGA path[256], ...)
 *   then    nslots x 0x48 material records: +0x20 name[16],
 *           +0x30 x, +0x34 y, +0x38 w, +0x3C h (atlas rect), +0x40 flags
 *           (0x100 = unused slot), +0x44 bitmap index
 *   then    the texture (sub_44F58E): +0 version, +8 width, +0xC height,
 *           +0x10 palette (256 x RGB), +0x310 CIF LZW pixels (8bpp)
 * Used slots are compacted in order (sub_44F6E8); faces store the SLOT id,
 * mapped through slot_map (the engine's lib[0x10 + 0xFF - slot] table).
 * ------------------------------------------------------------------------- */

#define OME_MATR_MAX_MATS  64

typedef struct {
    u8   r, g, b;       /* mean colour of the material's texels (fallback) */
    c8   name[16];
    i32  x, y, w, h;    /* rect in the texture atlas */
    i32  bitmap;        /* +0x44: index into the library's bitmaps */
    i32  flags;         /* +0x40: bit 0x200 = colour key, the low byte is the key texel (0x495B56) */
} ome_mat_t;

#define OME_MATR_MAX_BITMAPS 4

typedef struct {
    ome_mat_t mats[OME_MATR_MAX_MATS];
    i32       mat_count;
    i8        slot_map[OME_MATR_MAX_MATS];   /* face slot -> mats[] or -1 */
    i32       tex_w, tex_h;                  /* 0 = no embedded texture   */
    u8        tex_pal[768];
    u8       *tex_pix;                       /* tex_w * tex_h, heap-owned */
    /* Per bitmap entry: the embedded image, or the shared objects atlas
     * (dword_4FB1DC, OBJETS>OBJETS.CIF) for a bitmap named "objets.gif".
     * Pages are 256 wide. */
    i32       nbitmaps;
    const u8 *bm_pix[OME_MATR_MAX_BITMAPS];
    i32       bm_h[OME_MATR_MAX_BITMAPS];
} ome_matr_t;

/* Install the shared objects atlas (256 wide, 8bpp) used by "objets.gif"
 * bitmaps. Must outlive every library that references it. */
void ome_set_objets_atlas(const u8 *pix, i32 h);

/* Material for a face's slot id, or NULL. */
static inline const ome_mat_t *ome_matr_slot(const ome_matr_t *m, i32 slot) {
    if (!m || slot < 0 || slot >= OME_MATR_MAX_MATS || m->slot_map[slot] < 0) return NULL;
    return &m->mats[(i32)m->slot_map[slot]];
}

/*
 * ome_load_matr — decode the material library and texture of a whole .OME
 * file image. Returns 1 when at least one material was read.
 */
i32 ome_load_matr(const u8 *file, i32 file_size, ome_matr_t *out);

/* Release the texture owned by an ome_matr_t. */
void ome_matr_free(ome_matr_t *m);

/*
 * model_ome_open — load an .OME file from a real host path and populate
 * the chunk table. Returns 1 on success, 0 on failure. Frees any prior
 * contents of *out.
 */
i32 model_ome_open(const c8 *host_path, ome_file_t *out);

/*
 * model_ome_free — release the chunk table and raw buffer owned by *f.
 */
void model_ome_free(ome_file_t *f);

/* -------------------------------------------------------------------------
 * ATOMF body decoder (Slice F pass 3)
 * -------------------------------------------------------------------------
 *
 * Empirical layout (verified on OB98.OME and P00.OME):
 *
 *   body header (16 bytes):
 *     +0x00  i32 = 0
 *     +0x04  i32 = 0
 *     +0x08  i32 = heap ptr (original process face-list head, ignored)
 *     +0x0C  i32 = heap ptr (ignored)
 *
 *   vertex records (32 bytes each):
 *     [0] x   [1] y   [2] z          — position (signed, raw units)
 *     [3] nx  [4] ny  [5] nz         — per-vertex normal (same units)
 *     [6] next_ptr (heap, ignored)
 *     [7] flag: 0x00200000 if ptr present, else 0
 *     → last vertex record has flag == 0.
 *     → scan until field[7] is neither 0 nor 0x200000 to find face section.
 *
 *   face records (96 bytes each, preceded by a 4-byte heap ptr for faces 1+
 *   and followed by a 4-byte tail ptr; n_faces = remaining_bytes / 100):
 *     [0]  face type  (always 100 = 0x64)
 *     [1]  vcnt       (3 = triangle, 4 = quad)
 *     [2]  idx[0]
 *     [3]  idx[1]
 *     [4]  idx[2]
 *     [5]  idx[3]     (valid only when vcnt == 4)
 *     [6]  face_nx    (Q14: 16384 == 1.0)
 *     [7]  face_ny
 *     [8]  face_nz
 *     [9]  mat_id     (index into MATR material table)
 *     [10..23]        material params + UV data (not decoded in pass 3)
 * ------------------------------------------------------------------------- */

typedef struct {
    i32 x, y, z;     /* vertex position (raw .OME signed integers) */
    i32 nx, ny, nz;  /* per-vertex normal (raw, Q14-ish) */
} ome_vert_t;

typedef struct {
    u8  vcnt;        /* 3 = triangle, 4 = quad */
    u8  idx[4];      /* vertex indices (vcnt entries valid) */
    i32 nx, ny, nz;  /* face normal, Q14 */
    i32 mat_id;      /* material index */
} ome_face_t;

typedef struct {
    ome_vert_t *verts;
    i32         vert_count;
    ome_face_t *faces;
    i32         face_count;
} ome_atom_t;

/* -------------------------------------------------------------------------
 * OBJC chunk parser
 * ------------------------------------------------------------------------- */

#define OME_MAX_BONES      32
#define OME_MAX_ATOMSLOTS  24   /* P12 has 21 atom slots; keep headroom */

typedef struct {
    char name[16];
    i32  tx, ty, tz;
    i32  h2;               /* parent index; -1 = root. DERIVED from the
                            * child/sibling links, not read from +0x78 —
                            * see ome_parse_objc. */
    i32  first_child;      /* +0x7C, -1 = none */
    i32  next_sib;         /* +0x84, -1 = none */
    i32  slot;             /* +0x20 atom slot index, -1 = no geometry */
} ome_bone_t;

typedef struct {
    ome_bone_t  bones[OME_MAX_BONES];
    i32         n_bones;
    char        atomf_slot_name[OME_MAX_ATOMSLOTS][16];
    i32         n_atomslots;
    i32         world_tx[OME_MAX_ATOMSLOTS];
    i32         world_ty[OME_MAX_ATOMSLOTS];
    i32         world_tz[OME_MAX_ATOMSLOTS];
    /* Accumulated world rotation per atom (Q14 3x3, row-major m[row*3+col]),
     * from the CAN keyframe hierarchy — applied to the atom's vertices so the
     * limb geometry rotates with the pose, not just its pivot position. */
    i32         world_rot[OME_MAX_ATOMSLOTS][9];
    i32         can_to_render[OME_MAX_BONES]; /* bone j → render atom (-1 = no geo) */
    /* bone j → index into the engine's fixed 34-entry CAN slot order, i.e.
     * which keyframe channel drives this bone. -1 when the bone is not one
     * of the 34 the animation system knows about. */
    i32         can_slot[OME_MAX_BONES];
} ome_objc_t;

/* Parse OBJC chunk body. objc_sig_off = offset of "3DITOR v2.0 OBJC" in raw[]. */
i32 ome_parse_objc(const u8 *raw, i32 raw_len, i32 objc_sig_off, ome_objc_t *out);

/*
 * ome_decode_atomf — decode one ATOMF chunk body into verts + faces.
 *
 * raw     = pointer to sig_offset + 0x60 in the raw file buffer.
 * raw_len = bytes from raw to end of chunk (span - 0x60).
 * out     = caller-provided struct; filled on success.
 *
 * out->verts and out->faces are heap-allocated with malloc.
 * Call ome_atom_free() to release.
 * Returns 1 on success, 0 on failure (bad data or alloc error).
 */
i32  ome_decode_atomf(const u8 *raw, i32 raw_len, ome_atom_t *out);
void ome_atom_free(ome_atom_t *atom);
