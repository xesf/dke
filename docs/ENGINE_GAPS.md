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
- Still to parse and use in `scene_load_dat_config`: clipzperso/clipzvague/multiclip (levels 38, 39), coloured lights `ncolorlight` (level 13).
- Underwater mode (level 35): warp 0042FADF in `envfx.c` and bubbles 00430081 in `fx.c` are in. The bubble spawn period
  uses time since level start, not the player timer at +0xD5C. `g_render_mode_flag` is still never set (HUD parts).
- Low-detail actor path when multiclip is nonzero: 0042A5E3, 0042B63D, 0042B0A6.

### 4. Combat effects

- Enemy flame projectile: 00410E8A, 004119B7, 00411C27, 00411D09, 00411FE8. It flies, hits, makes sparks and a
  154-particle burst, and does −100 HP. There is a TODO in `combat_damage.c`.
- Blood spray 0042D03F: 32 particles on a hit. There is a TODO in `combat_damage.c`.
- Screen-space sprite effects 004305EF: effect entity types 6 to 8. This probably covers the smoke sprites.
- AI for the flying imp P92 (type 0x5C): 0046034D, 004603CB. There is a TODO in `combat_ai.c`.

### 5. Scripted fades and big animations

- LIEU op 42 fades and crossfades: `sub_42BE0E` with 0042BC7C, 0042BD44, 0042EA19, 0042EE82. There is a TODO in
  `lieu_cb.c`.
- Done (`src/cjr.c`): the CJR big-animation player for LIEU op 17. CJR files are Autodesk FLC; only DELTA_FLC chunks occur.
  Not done: blocking level exits and ESC while the animation runs (`bonus_mode_active` checks in 0044A58B, 00407159),
  and the player/camera fall offset of sub_454690 for ANIM00 and ANIM02.

### 6. Movies

- Done: subtitles (`DATAS>SBT>SC<nn>_<lang>.TXT`, drawn with the game font at y = 452), the `_nnn` start frame, and the OVER
  card after movies 16, 21, 22 and 23 (`movie.c`, `scene.c`). Not run against a real movie yet.
- Colour correction (brightness, contrast, gamma tables): 00431D38, 004327B2, 00432A3B, 00432BE1, 00432D7D.
- End fade 004330DD.

### 7. Objects across levels

- Object positions remembered per level: 0047306E, 0047347E.
- Object slot cleanup on level load: 00476E86.
- The remaining equip form 0047404E. `scene_weapon_entity_attach` covers only the `sub_473756` form.

### 8. Renderer fidelity

- Colour-key texels (face flag 0x200): 00495B56, 00496738, 004967B1, 00497188, 00497266. These faces now draw solid.
- Near-plane clipping: 00498330, 0049A790. Triangles that cross the near plane are now dropped.
- Matrix pose blend for CAN flag 0x800: 0041553F, 0041560C, 00492AC0 to 00492F30. The port lerps the angles instead.
- Actor shadows in the game: 00417E40, 00418300, 00418470. Only the character screen has them now.
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
