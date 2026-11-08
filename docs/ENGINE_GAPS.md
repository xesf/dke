# Engine gaps

What is still missing from the dkev.exe port, what to reimplement, and what the port skips on purpose.
Per-function detail is in [metadata/exe-coverage.md](metadata/exe-coverage.md).

## Sources

`tools/ida7/dump.sh` dumps the exe with IDA Pro 7.0. That version has the 32-bit Hex-Rays decompiler and runs headless
with IDC only, so it needs no Python. The script replays the function names, global names and structs of the IDA 9
database (`tools/ida7/names.idc`, `tools/ida7/types.h`) and writes these files to `disassembly/ida7/`, which git ignores:

| File | Content |
|---|---|
| `c/<ADDR>_<name>.c` | Hex-Rays pseudocode, one file per function |
| `asm/<ADDR>_<name>.asm` | Assembly, one file per function, with cross-references |
| `dkev.c`, `dkev.asm`, `dkev.lst` | The full dumps |
| `index.json` | Name and C line count per address |
| `dkev_ida7.idb` | The named IDA 7 database, which can be opened in IDA 7.0 |

The dump does not carry the IDA 9 prototypes and comments: the MCP server cannot export them. Argument types are
therefore Hex-Rays guesses.

## Coverage (2026-11-01)

| | Functions |
|---|---|
| In the exe | 1343 |
| Cited by address or name in `src/` | 637 |
| Ported under another name | 132 |
| Partly ported | 63 |
| Missing | 68 |
| Not needed | 443 |

`tools/ida7/classification.json` holds the classification. Agents made it by reading the pseudocode. Large functions
were read in full; small helpers were judged from name, size and callers. Check a verdict before you rely on it.

## Reimplement

The packages are in priority order. Each package names its functions; port them with the `decomp` skill and cite the
address in a comment.

### 1. Pause (P key)

- `display_pause_screen` 0040A2B0 and 0040A280: the PAUSED%d.GIF screen, the cheat codes (easy mode, big head, dwarf
  and other model overrides), and the screenshot.
- 0044D68C, 0044D6C9; `ls_pause_sound` 004802B0, `ls_resume_sound` 00480260: called from `process_input_command`.
- Time compensation 004067EA, 0041FAA2, 0041FAD1: timers shift by the paused time. Without it, cooldowns and
  animations jump ahead after a pause or the inventory.

### 2. Gamma and colour settings

- Gamma picker on first run 0040ADE0, `apply_screen_gamma` 0040ACC0, and 0040C920, 0040CA80, 0040CBF0.
- The `DKE.CFG` loader 0045B361 also reads Brightness, Contrast, Saturation, Language and Detail. The port reads none
  of these.

### 3. Level `.DAT` options and render modes

- Done (`src/envfx.c`): flash 0042F565/0046BE40 and environment tint 0040C010, applied to the frame at blit time.
- Not needed: coloured lights `ncolorlight` is 0 in the only DAT that has the key (level 13), so 0x4355E0 never runs.
- Still to parse in `scene_load_dat_config`: clipzperso/clipzvague/multiclip (levels 30, 38, 39). They only pick the low-detail
  render path for far actors, a speed trade, so they matter most for the planned GPU renderer.
- Underwater mode (level 35): warp 0042FADF in `envfx.c` and bubbles 00430081 in `fx.c` are in. The bubble spawn period
  uses time since level start, not the player timer at +0xD5C. `g_render_mode_flag` is still never set (HUD parts).
- Low-detail actor path when multiclip is nonzero: 0042A5E3, 0042B63D, 0042B0A6.

### 4. Combat effects

- Done: enemy flame projectile, blood spray, smoke puffs and the flying imp P92 (`combat_damage.c`, `fx.c`, `combat_ai.c`).
- Done: P40 guard phase (+0xD70 == 2) of sub_460444 (`boss_guard_tick`). In other phases P40 now runs the common brain
  (before, it did nothing at all).
- Not done: the remaining part of sub_460444's else branch uses the same helpers as the common brain, so it needs no port.

### 5. Scripted fades and big animations

- LIEU op 42 fades and crossfades: `sub_42BE0E` with 0042BC7C, 0042BD44, 0042EA19, 0042EE82. There is a TODO in
  `lieu_cb.c`.
- Done (`src/cjr.c`): the CJR big-animation player for LIEU op 17. CJR files are Autodesk FLC; only DELTA_FLC chunks occur.
  Room and camera changes (cases 9, 10) and ESC wait while it runs. The fall offset of sub_454690 is skipped: it writes through
  `dword_4F91CC`, which nothing in the binary ever sets (null), so it looks dead.

### 6. Movies

- Done: subtitles (`DATAS>SBT>SC<nn>_<lang>.TXT`, drawn with the game font at y = 452), the `_nnn` end fade (ms, 0x4330DD; it is not a start frame), and the OVER
  card after movies 16, 21, 22 and 23 (`movie.c`, `scene.c`). Not run against a real movie yet.
- Colour correction (brightness, contrast, gamma tables): 00431D38, 004327B2, 00432A3B, 00432BE1, 00432D7D.

### 7. Objects across levels

- Done: LIEU LinkObjet (`LA_LINKOBJET`, 0x44B... case 0x20) now takes and releases the object for real: `weapon_link`
  (sub_47404E, palm bone from the script, word_509B40 row updated) and `weapon_unlink` (sub_474462). Not run in a level yet.
- Not needed: 0047306E and 00476E86 only keep a dropped item's position until the level is released, which the port's
  per-level object reset already does. 0047347E is the debug object editor.

### 8. Renderer fidelity

- Done: colour-key texels. Material flag 0x200 (only P28 "CHEVEUX2" in the data) skips texels equal to the low flag byte (0x495B56).
- Done: faces crossing the near plane are cut at cz = -32768 (0x49A790) in the in-game batch path, with shade and texel interpolated. The older single-model paths still drop them.
- Matrix pose blend for CAN flag 0x800: 0041553F, 0041560C, 00492AC0 to 00492F30. The original turns the relative rotation by
  a fraction of its axis-angle; the port lerps the angles instead. Small difference; port it only with a visual check.
- Actor shadows in the game (`dword_4B7240` modes 1 to 4, `update_game_hud` 00466D4F): a render-to-texture silhouette
  projected on the floor. 00417E40, 00418300, 00418470 are only the 2D polygon edge clipper it uses. The port has no shadows
  at all (the earlier note about the character screen was wrong). This is the largest item left.
- Per-axis model scale `sub_48FCD0` (entity +0x6C/+0x70/+0x74, 8.8 fixed point). First check whether any model uses
  a scale other than 256.
- Alpha and shaded 2D blits: 004774BC, 0047776C, 00477BB0, 004781E4, 0047826B, 004782DC.
- Smaller items: foot-bone ordering on walk and run restarts (0041FB48 and `sub_41AD23`), adaptive mesh detail
  (00406B51, 00406C30), edge smoothing (004540AD, 004542FD), subtitle box and font (00416320, 004168F0, 00416950),
  player-flag reset 00461AF6.

## Skip

The port does not need these functions:

- MSVC C runtime and libm.
- Win32, DirectDraw, DirectSound, MCI, joystick and timer glue. The platform layer replaces them.
- The original memory manager, path manager, named-file cache and DLL loader.
- GIF, PCX, LBM and TGA loaders and savers, and the image encoders. The game data uses CIF.
- Bit-depth remapping and the 16-bit colour kernels (0049B086 to 0049D796). The port renders palette and GRD shading
  to RGBA32.
- The 3DS `.ASC` parser and the camera build (004204BF, 0042035A, 00420418). They run only when the cached `.INF` is
  missing, and the port reads the `.INF`.
- Wireframe, line and point primitives, the MMX rasterisers (never enabled), and the 3DF/Glide texture loader.
- Editor and debug code: `rectangle_editor`, the RemoteDev log (0x406D74), the debug variable viewer, the debug
  text prompt, and the zone overlay. The zone overlay is already ported as a debug view.
- Functions with no callers (`unused_*` and others).
