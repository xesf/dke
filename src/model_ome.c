/*
 * model_ome.c — .OME chunk inventory pass.
 *
 * Walks the "3DITOR v?.? TAG" signatures inside a whole-file buffer and
 * classifies each chunk by its 4/5-char tag (MATR, OBJC, ATOMF). Chunk
 * bodies are NOT decoded here — that's the next slice, blocked on
 * pointer fix-ups. Callers get a list of (kind, offset, length, name)
 * tuples plus per-kind counts.
 *
 * Sample output for bin/game/persos/P00.OME:
 *   model_ome: bin/.../P00.OME size=124071 chunks=22
 *     +0x011c4 MATR  (36221 b)  "P00.MAT"
 *     +0x09f41 OBJC  ( 5848 b)  ""
 *     +0x0b619 ATOMF ( 5080 b)  ""
 *     ... etc
 *   model_ome: P00.OME → MATR=1 OBJC=1 ATOMF=20
 */

#include <stdio.h>
#include <stdlib.h>

#define CAN_SLOT_COUNT 34
#include <string.h>

#include "libsys/common.h"
#include "libsys/file_system.h"
#include "model_ome.h"
#include "libsys/graphics/cif.h"

/* Longest signature we recognise: "3DITOR v2.0 ATOMF" = 17 chars. */
#define OME_SIG_PREFIX  "3DITOR v"
#define OME_SIG_MIN_LEN 16
#define OME_SIG_MAX_LEN 17

/*
 * classify_tag — map a chunk tag string to its enum kind.
 */
static ome_chunk_kind_e classify_tag(const c8 *tag) {
    if (!tag) return OME_CHUNK_UNKNOWN;
    if (strncmp(tag, "MATR",  4) == 0) return OME_CHUNK_MATR;
    if (strncmp(tag, "OBJC",  4) == 0) return OME_CHUNK_OBJC;
    if (strncmp(tag, "ATOMF", 5) == 0) return OME_CHUNK_ATOMF;
    return OME_CHUNK_UNKNOWN;
}

/*
 * find_next_signature — scan raw[start..end] for the next "3DITOR v"
 * marker and return its byte offset. Returns -1 when not found.
 */
static i32 find_next_signature(const u8 *raw, i32 start, i32 end) {
    const i32 pre_len = (i32)sizeof(OME_SIG_PREFIX) - 1;
    for (i32 i = start; i + pre_len < end; i++) {
        if (raw[i] == '3' &&
            memcmp(raw + i, OME_SIG_PREFIX, (size_t)pre_len) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * read_tag_at — extract the space-delimited 4/5-char TAG right after
 * "3DITOR v?.? ". Writes up to 8 chars (incl. NUL) into out.
 */
static void read_tag_at(const u8 *raw, i32 sig_off, c8 *out, i32 out_cap) {
    /* After "3DITOR v" comes "1.3 " or "2.0 " (4 chars incl. trailing space). */
    i32 t = sig_off + 8 /*strlen("3DITOR v")*/ + 4;
    i32 j = 0;
    while (j < out_cap - 1 && t < INT32_MAX) {
        u8 c = raw[t];
        if (c < 'A' || c > 'Z') break;
        out[j++] = (c8)c;
        t++;
    }
    out[j] = '\0';
}

/*
 * read_name_at — extract the 16-byte filename field that sits at
 * sig_off + 0x20 (past sig + dashes). Truncated to out_cap. Copies
 * up to first NUL / non-printable byte.
 */
static void read_name_at(const u8 *raw, i32 sig_off, i32 file_size,
                          c8 *out, i32 out_cap) {
    i32 name_off = sig_off + 0x20;
    i32 j = 0;
    for (; j < 15 && name_off + j < file_size && j < out_cap - 1; j++) {
        u8 c = raw[name_off + j];
        if (c == 0 || c < 0x20 || c > 0x7E) break;
        out[j] = (c8)c;
    }
    out[j] = '\0';
}

static i32 read_i32_le(const u8 *p) {
    return (i32)((u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24));
}

static const u8 *g_objets_atlas = NULL;    /* dword_4FB1DC */
static i32       g_objets_atlas_h = 0;

void ome_set_objets_atlas(const u8 *pix, i32 h) { g_objets_atlas = pix; g_objets_atlas_h = h; }

i32 ome_load_matr(const u8 *file, i32 size, ome_matr_t *out) {     /* sub_44F92D */
    if (!file || !out || size < 16) return 0;
    memset(out, 0, sizeof(*out));
    memset(out->slot_map, -1, sizeof(out->slot_map));
    i32 m = (memcmp(file, "CTX3", 4) == 0) ? read_i32_le(file + 8) : 0;   /* 0x44F501 */
    if (m < 0 || m + 0x180 > size) return 0;
    if (memcmp(file + m + 7, "v1.3", 4) != 0 && memcmp(file + m + 0xC, "MATR", 4) != 0) {
        print("ome_load_matr: Bad MAT Library header\n");
        return 0;
    }
    const u8 *lib = file + m + 0x20;
    i32 nslots = read_i32_le(lib + 0x148), nbitmaps = read_i32_le(lib + 0x14C);
    if (nslots < 0 || nslots > OME_MATR_MAX_MATS || nbitmaps < 0 || nbitmaps > OME_MATR_MAX_BITMAPS) return 0;
    out->nbitmaps = nbitmaps;
    i32 shared[OME_MATR_MAX_BITMAPS] = {0}, any_embedded = 0;
    for (i32 b = 0; b < nbitmaps; b++) {
        /* sub_44FFB9: a bitmap whose name ends in "objets.gif" uses the
         * shared atlas and decodes nothing. */
        c8 nm[257];
        memcpy(nm, file + m + 0x180 + b * 0x114, 256);
        nm[256] = 0;
        for (c8 *q = nm; *q; q++) if (*q >= 'A' && *q <= 'Z') *q += 32;
        size_t L = strlen(nm);
        shared[b] = (L >= 10 && strcmp(nm + L - 10, "objets.gif") == 0);
        if (!shared[b]) any_embedded = 1;
    }
    i32 recs = m + 0x180 + nbitmaps * 0x114;
    i32 img  = recs + nslots * 0x48;
    if (img > size) return 0;

    /* sub_44F6E8: keep the named slots, in order. */
    for (i32 sl = 0; sl < nslots; sl++) {
        const u8 *r = file + recs + sl * 0x48;
        if (r[0x20] == 0 || (read_i32_le(r + 0x40) & 0x100)) continue;
        ome_mat_t *mt = &out->mats[out->mat_count];
        memcpy(mt->name, r + 0x20, 15);
        mt->name[15] = 0;
        mt->x = read_i32_le(r + 0x30); mt->y = read_i32_le(r + 0x34);
        mt->w = read_i32_le(r + 0x38); mt->h = read_i32_le(r + 0x3C);
        mt->bitmap = read_i32_le(r + 0x44);
        mt->flags = read_i32_le(r + 0x40);
        if (mt->bitmap < 0) mt->bitmap = 0;
        if (nbitmaps > 0 && mt->bitmap >= nbitmaps) mt->bitmap = nbitmaps - 1;
        out->slot_map[sl] = (i8)out->mat_count++;
    }

    /* sub_44F58E: the embedded 8bpp image. Its length is not stored; the
     * LZW stream ends with code 257 before the next chunk. */
    if (any_embedded && img + 0x310 < size) {
        i32 w = read_i32_le(file + img + 8), h = read_i32_le(file + img + 0xC);
        if (w > 0 && w <= 1024 && h > 0 && h <= 1024) {
            out->tex_pix = (u8 *)calloc((size_t)w * (size_t)h, 1);
            if (out->tex_pix) {
                memcpy(out->tex_pal, file + img + 0x10, 768);
                cif_decompress_lzw(file + img + 0x310, size - (img + 0x310), w, h, out->tex_pix);
                out->tex_w = w;
                out->tex_h = h;
            }
        }
    }

    /* The engine decodes the same embedded image for every non-shared
     * bitmap (its data pointer does not advance). */
    for (i32 b = 0; b < nbitmaps; b++) {
        if (shared[b]) { out->bm_pix[b] = g_objets_atlas; out->bm_h[b] = g_objets_atlas_h; }
        else if (out->tex_w == 256) { out->bm_pix[b] = out->tex_pix; out->bm_h[b] = out->tex_h; }
    }

    /* Mean colour per material (embedded palette), for untextured fallbacks. */
    for (i32 k = 0; k < out->mat_count; k++) {
        ome_mat_t *mt = &out->mats[k];
        u32 sr = 0, sg = 0, sb = 0, n = 0;
        const u8 *bp = out->bm_pix[mt->bitmap];
        i32 bh = out->bm_h[mt->bitmap];
        for (i32 y = mt->y; bp && bp == out->tex_pix && y < mt->y + mt->h && y < bh; y++)
            for (i32 x = mt->x; x < mt->x + mt->w && x < 256; x++) {
                const u8 *c = out->tex_pal + 3 * bp[y * 256 + x];
                sr += c[0]; sg += c[1]; sb += c[2]; n++;
            }
        if (n) { mt->r = (u8)(sr / n); mt->g = (u8)(sg / n); mt->b = (u8)(sb / n); }
        else   { mt->r = mt->g = mt->b = 0x80; }
    }
    return out->mat_count > 0;
}

void ome_matr_free(ome_matr_t *m) {
    if (!m) return;
    free(m->tex_pix);
    m->tex_pix = NULL;
    m->tex_w = m->tex_h = 0;
}

i32 model_ome_open(const c8 *host_path, ome_file_t *out) {
    if (!out || !host_path) return 0;
    model_ome_free(out);

    FILE *f = ls_open(host_path, "rb");
    if (!f) {
        print("model_ome: cannot open '%s'\n", host_path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 8 * 1024 * 1024) {   /* cap at 8 MB */
        print("model_ome: '%s' size %ld out of range\n", host_path, sz);
        ls_close(f);
        return 0;
    }
    u8 *raw = (u8 *)malloc((size_t)sz);
    if (!raw) { ls_close(f); return 0; }
    if (fread(raw, 1, (size_t)sz, f) != (size_t)sz) {
        free(raw);
        ls_close(f);
        return 0;
    }
    ls_close(f);

    /* First pass: count signatures. */
    i32 file_size = (i32)sz;
    i32 count = 0;
    for (i32 pos = 0; pos < file_size; ) {
        i32 next = find_next_signature(raw, pos, file_size);
        if (next < 0) break;
        count++;
        pos = next + OME_SIG_MIN_LEN;
    }
    if (count == 0) {
        print("model_ome: no 3DITOR signatures in '%s'\n", host_path);
        free(raw);
        return 0;
    }

    ome_chunk_t *chunks = (ome_chunk_t *)calloc((size_t)count, sizeof(*chunks));
    if (!chunks) { free(raw); return 0; }

    /* Second pass: fill chunk table. */
    i32 idx = 0;
    for (i32 pos = 0; pos < file_size && idx < count; ) {
        i32 sig = find_next_signature(raw, pos, file_size);
        if (sig < 0) break;
        c8 tag[8];
        read_tag_at(raw, sig, tag, sizeof(tag));
        chunks[idx].kind   = classify_tag(tag);
        chunks[idx].offset = sig;
        read_name_at(raw, sig, file_size,
                     chunks[idx].name, (i32)sizeof(chunks[idx].name));
        idx++;
        pos = sig + OME_SIG_MIN_LEN;
    }
    /* Chunk lengths = distance to next chunk (or EOF for last one). */
    for (i32 i = 0; i < idx; i++) {
        i32 next_off = (i + 1 < idx) ? chunks[i + 1].offset : file_size;
        chunks[i].length = next_off - chunks[i].offset;
    }

    out->chunks      = chunks;
    out->chunk_count = idx;
    out->raw         = raw;
    out->raw_size    = file_size;
    out->matr_count  = 0;
    out->objc_count  = 0;
    out->atomf_count = 0;
    for (i32 i = 0; i < idx; i++) {
        switch (chunks[i].kind) {
            case OME_CHUNK_MATR:  out->matr_count++;  break;
            case OME_CHUNK_OBJC:  out->objc_count++;  break;
            case OME_CHUNK_ATOMF: out->atomf_count++; break;
            default: break;
        }
    }

    /* Report inventory. */
    print("model_ome: '%s' size=%d chunks=%d (MATR=%d OBJC=%d ATOMF=%d)\n",
          host_path, file_size, out->chunk_count,
          out->matr_count, out->objc_count, out->atomf_count);
    for (i32 i = 0; i < idx && i < 16; i++) {
        static const c8 *KIND_NAME[] = {"MATR", "OBJC", "ATOMF", "?"};
        i32 k = (i32)chunks[i].kind;
        if (k < 0 || k > 3) k = 3;
        print("model_ome:   [%02d] +0x%05x %-5s (%d b) name='%s'\n",
              i, chunks[i].offset, KIND_NAME[k],
              chunks[i].length, chunks[i].name);
    }
    if (idx > 16) {
        print("model_ome:   ... %d more chunks omitted from listing\n", idx - 16);
    }
    return 1;
}

void model_ome_free(ome_file_t *f) {
    if (!f) return;
    if (f->chunks) { free(f->chunks); f->chunks = NULL; }
    if (f->raw)    { free(f->raw);    f->raw    = NULL; }
    f->chunk_count = 0;
    f->raw_size    = 0;
    f->matr_count  = 0;
    f->objc_count  = 0;
    f->atomf_count = 0;
}

/* -------------------------------------------------------------------------
 * ATOMF body decoder
 * See model_ome.h for the empirical layout notes.
 * ------------------------------------------------------------------------- */

i32 ome_decode_atomf(const u8 *raw, i32 raw_len, ome_atom_t *out) {
    if (!raw || !out || raw_len < 16) return 0;
    out->verts      = NULL;
    out->vert_count = 0;
    out->faces      = NULL;
    out->face_count = 0;

    /* Skip 16-byte body header. */
    i32 pos = 16;

    /* Count vertex records: 32 bytes each. field[7] is a bit set of flags
     * combining 0x00200000 (next-ptr present) and 0x40000000 (seen on some
     * atoms, e.g. P00 slot 18 = the head); a record is still a vertex as long
     * as no OTHER bits are set. The face section (ftype!=0x64...) breaks it. */
    i32 n_verts = 0;
    while (pos + 32 <= raw_len) {
        u32 flag = (u32)read_i32_le(raw + pos + 28);
        if (flag & ~0x40200000u) break;
        n_verts++;
        pos += 32;
    }

    i32 face_section_start = pos;

    /* Count faces with a greedy scan (mirrors tools/src/dump_ome.py).
     *
     * Layout: face 0 starts directly at face_section_start (no preceding ptr).
     * Faces 1+ are each preceded by a 4-byte heap ptr (ignored).
     * Stop when there is not enough space or the face type / vcnt is invalid.
     *
     * This correctly handles varying tail-byte counts (0, 4, or 8 trailing
     * bytes) across different .OME files without requiring exact divisibility. */
    i32 n_faces = 0;
    {
        i32 scan = face_section_start;
        i32 first_face = 1;
        while (scan + 96 <= raw_len) {
            if (!first_face) {
                if (scan + 4 + 96 > raw_len) break;
                scan += 4; /* skip 4-byte heap ptr before face 1+ */
            }
            first_face = 0;
            i32 ftype = read_i32_le(raw + scan);
            i32 vcnt  = read_i32_le(raw + scan + 4);
            if (ftype != 100 || vcnt < 3 || vcnt > 4) break;
            n_faces++;
            scan += 96;
        }
    }

    if (n_verts == 0 && n_faces == 0) {
        print("ome_decode_atomf: empty atom (raw_len=%d)\n", raw_len);
        return 1; /* success — caller may check counts */
    }

    /* Allocate. */
    if (n_verts > 0) {
        out->verts = (ome_vert_t *)malloc((size_t)n_verts * sizeof(ome_vert_t));
        if (!out->verts) { print("ome_decode_atomf: vert alloc failed\n"); return 0; }
    }
    if (n_faces > 0) {
        out->faces = (ome_face_t *)malloc((size_t)n_faces * sizeof(ome_face_t));
        if (!out->faces) {
            free(out->verts); out->verts = NULL;
            print("ome_decode_atomf: face alloc failed\n");
            return 0;
        }
    }

    /* Decode vertices. */
    pos = 16;
    for (i32 i = 0; i < n_verts; i++) {
        const u8 *r = raw + pos;
        out->verts[i].x  = read_i32_le(r +  0);
        out->verts[i].y  = read_i32_le(r +  4);
        out->verts[i].z  = read_i32_le(r +  8);
        out->verts[i].nx = read_i32_le(r + 12);
        out->verts[i].ny = read_i32_le(r + 16);
        out->verts[i].nz = read_i32_le(r + 20);
        pos += 32;
    }
    out->vert_count = n_verts;

    /* Decode faces — same greedy scan as the counting pass. */
    pos = face_section_start;
    for (i32 i = 0; i < n_faces; i++) {
        if (i > 0) {
            pos += 4; /* skip 4-byte heap ptr before face 1+ */
        }
        if (pos + 96 > raw_len) break;

        const u8 *r = raw + pos;
        i32 ftype = read_i32_le(r +  0);
        i32 vcnt  = read_i32_le(r +  4);

        if (ftype != 100 || vcnt < 3 || vcnt > 4) {
            print("ome_decode_atomf: face %d bad type=%d vcnt=%d — truncating\n",
                  i, ftype, vcnt);
            n_faces = i;
            break;
        }

        out->faces[i].vcnt   = (u8)vcnt;
        out->faces[i].idx[0] = (u8)read_i32_le(r +  8);
        out->faces[i].idx[1] = (u8)read_i32_le(r + 12);
        out->faces[i].idx[2] = (u8)read_i32_le(r + 16);
        out->faces[i].idx[3] = (vcnt == 4) ? (u8)read_i32_le(r + 20) : out->faces[i].idx[2];
        out->faces[i].nx     = read_i32_le(r + 24); /* Q14 */
        out->faces[i].ny     = read_i32_le(r + 28);
        out->faces[i].nz     = read_i32_le(r + 32);
        out->faces[i].mat_id = read_i32_le(r + 36);
        pos += 96;
    }
    out->face_count = n_faces;

    print("ome_decode_atomf: %d verts %d faces\n", out->vert_count, out->face_count);
    return 1;
}

void ome_atom_free(ome_atom_t *atom) {
    if (!atom) return;
    if (atom->verts) { free(atom->verts); atom->verts = NULL; }
    if (atom->faces) { free(atom->faces); atom->faces = NULL; }
    atom->vert_count = 0;
    atom->face_count = 0;
}

/* -------------------------------------------------------------------------
 * OBJC chunk parser
 * See model_ome.h for the empirical layout notes.
 * ------------------------------------------------------------------------- */

i32 ome_parse_objc(const u8 *raw, i32 raw_len, i32 objc_sig_off, ome_objc_t *out) {
    if (!raw || !out || objc_sig_off < 0 || objc_sig_off + 0x30 > raw_len) return 0;
    memset(out, 0, sizeof(*out));

    const u8 *hdr = raw + objc_sig_off;

    /* n_bones at header[0x10], n_atomslots at header[0x20] */
    i32 n_bones = read_i32_le(hdr + 0x10);
    i32 n_atomslots = read_i32_le(hdr + 0x20);
    if (n_bones <= 0 || n_bones > OME_MAX_BONES) return 0;
    if (n_atomslots <= 0 || n_atomslots > OME_MAX_ATOMSLOTS) return 0;
    if (objc_sig_off + 0x30 + n_bones * 0xB4 + n_atomslots * 16 > raw_len) return 0;

    out->n_bones = n_bones;
    out->n_atomslots = n_atomslots;

    /* Parse bones */
    for (i32 i = 0; i < n_bones; i++) {
        const u8 *rec = hdr + 0x30 + i * 0xB4;  /* record[i] */
        const u8 *dat = rec + 4;                   /* data after 4-byte header */
        ome_bone_t *b = &out->bones[i];
        /* name: null-terminated, copy up to 15 chars */
        i32 nlen = 0;
        while (nlen < 15 && dat[nlen] != 0) nlen++;
        memcpy(b->name, dat, (size_t)nlen);
        b->name[nlen] = '\0';
        b->tx = read_i32_le(dat + 0x38);
        b->ty = read_i32_le(dat + 0x3C);
        b->tz = read_i32_le(dat + 0x40);
        b->first_child = read_i32_le(dat + 0x78);   /* rec + 0x7C */
        b->next_sib    = read_i32_le(dat + 0x80);   /* rec + 0x84 */
        b->slot        = read_i32_le(dat + 0x1C);   /* rec + 0x20 */
        b->h2 = -1;                                  /* derived below */
    }

    /*
     * Derive each bone's parent by walking the first-child / next-sibling
     * links, exactly as the engine does at load time: sub_4699D9 @ 0x4699D9
     * traverses +0x7C / +0x84 and WRITES the parent back to +0x78 (and the
     * previous sibling to +0x80). Those two fields are runtime state — what
     * the modelling tool left in the file is stale and disagrees with the
     * real rig wherever a new sibling chain starts. Reading +0x78 directly
     * gave P00 'hangau' (left hip) a parent of 'piedro' (the RIGHT foot) and
     * 'epagau' (left shoulder) a parent of 'poidro' (the right wrist), so the
     * whole left side hung off the end of the right limb — wrong rest pose
     * and wrong animation, with limbs swinging far from the body.
     */
    for (i32 i = 0; i < n_bones; i++) {
        i32 c = out->bones[i].first_child;
        while (c >= 0 && c < n_bones) {
            out->bones[c].h2 = i;
            i32 nxt = out->bones[c].next_sib;
            if (nxt == c) break;                     /* malformed self-link */
            c = nxt;
        }
    }

    /* Parse ATOMF slot names */
    const u8 *slots = hdr + 0x30 + n_bones * 0xB4;
    for (i32 i = 0; i < n_atomslots; i++) {
        const u8 *s = slots + i * 16;
        i32 nlen = 0;
        while (nlen < 15 && s[nlen] != 0) nlen++;
        memcpy(out->atomf_slot_name[i], s, (size_t)nlen);
        out->atomf_slot_name[i][nlen] = '\0';
    }

    /*
     * Bone -> CAN keyframe channel.
     *
     * A CAN keyframe is 34 fixed channels in the engine's own bone order
     * (the table sub_41F6BA @ 0x41F6BA matches scene-node names against when
     * filling the bone pointer array at entity+0xFC). That order has nothing
     * to do with the order bones happen to appear in an OME file: P00 stores
     * tree, basdos, handro, gendro, ... while channel 2 is 'hangau' and
     * channel 3 is 'handro'. Indexing the keyframe by the file position — as
     * this did — drives every bone from another bone's channel, which is why
     * the animations looked wrong.
     */
    static const c8 *const CAN_SLOT_NAMES[CAN_SLOT_COUNT] = {
        "tree",   "basdos", "hangau", "handro", "gendro",
        "piedro", "gengau", "piegau", "mildos", "haudos",
        "bascou", "epagau", "epadro", "coudro", "poidro",
        "cougau", "poigau", "tete",   "talgau", "taldro",
        "ddpgau", "ddpdro", "acces0", "acces1", "acces2",
        "acces3", "acces4", "pacdro", "pacgau", "hacdro",
        "hacgau", "arme",   "armea",  "armem"
    };
    for (i32 j = 0; j < n_bones; j++) {
        out->can_slot[j] = -1;
        for (i32 c = 0; c < CAN_SLOT_COUNT; c++) {
            if (strcmp(out->bones[j].name, CAN_SLOT_NAMES[c]) == 0) {
                out->can_slot[j] = c;
                break;
            }
        }
    }

    /* Bone -> ATOMF slot. The record carries the slot index directly at
     * +0x20 (-1 when the bone has no geometry); fall back to matching the
     * bone name against the slot name table when it looks out of range. */
    for (i32 j = 0; j < n_bones; j++) {
        i32 slot = out->bones[j].slot;
        if (slot >= 0 && slot < n_atomslots) {
            out->can_to_render[j] = slot;
            continue;
        }
        out->can_to_render[j] = -1;
        for (i32 i = 0; i < n_atomslots; i++) {
            if (strcmp(out->bones[j].name, out->atomf_slot_name[i]) == 0) {
                out->can_to_render[j] = i;
                break;
            }
        }
    }

    if (getenv("DKE_BONEDBG")) {
        print("objc: %d bones, %d atom slots\n", n_bones, n_atomslots);
        for (i32 i = 0; i < n_atomslots; i++)
            print("objc:   slot[%2d] name='%s'\n", i, out->atomf_slot_name[i]);
        i32 mapped = 0;
        for (i32 j = 0; j < n_bones; j++) {
            print("objc:   bone[%2d] '%-10s' parent=%-3d t=(%d,%d,%d) -> slot %d\n",
                  j, out->bones[j].name, out->bones[j].h2,
                  out->bones[j].tx, out->bones[j].ty, out->bones[j].tz,
                  out->can_to_render[j]);
            if (out->can_to_render[j] >= 0) mapped++;
        }
        print("objc: %d/%d bones carry geometry\n", mapped, n_bones);
    }

    /* Compute world translations for each render atom i:
     * Walk parent chain from slot_bone upward, exclude root (h2==-1) bone. */
    for (i32 i = 0; i < n_atomslots; i++) {
        /* Find which bone corresponds to ATOMF slot i */
        i32 bone_idx = -1;
        for (i32 j = 0; j < n_bones; j++) {
            if (out->can_to_render[j] == i) { bone_idx = j; break; }
        }
        if (bone_idx < 0) continue;

        i32 wtx = 0, wty = 0, wtz = 0;
        i32 idx = bone_idx;
        while (idx >= 0 && idx < n_bones) {
            const ome_bone_t *b = &out->bones[idx];
            if (b->h2 == -1) break; /* root bone: skip its translation */
            wtx += b->tx; wty += b->ty; wtz += b->tz;
            idx = b->h2;
        }
        out->world_tx[i] = wtx;
        out->world_ty[i] = wty;
        out->world_tz[i] = wtz;
    }

    print("ome_parse_objc: n_bones=%d n_atomslots=%d\n", n_bones, n_atomslots);
    for (i32 i = 0; i < n_atomslots; i++)
        print("  slot[%2d] %-12s -> world(%d,%d,%d)\n",
              i, out->atomf_slot_name[i],
              out->world_tx[i], out->world_ty[i], out->world_tz[i]);
    return 1;
}
