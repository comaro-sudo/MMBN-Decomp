# Gemini Decomp Handoff (2026-11-26)

## Goal & Current Status
- **Goal:** Advance the MMBN4 (Red Sun & Blue Moon) decompilation along [`decomp/notes/decomp-plan.md`](decomp-plan.md) while keeping both ROMs 100% byte-identical (`make compare VERSION=rs` and `VERSION=bm`).
- **Verified Status (pushed to `origin/main`):**
  - **Code ownership:** **82.6%** total (`974,351 / 1,179,456` bytes; **86.3%** excluding `code_10`'s `49,992 B` build-timestamp + `m4a` audio tail).
    - `intr_table`: **100.0%** (`48 / 48 B`)
    - `code_07` & `code_08`: **96.6%** each
    - `code_00`: **92.3%**, `code_02`: **90.9%**, `code_01`: **87.9%**, `code_04`: **86.8%**, `code_04c`: **86.3%**, `code_03`: **86.0%**, `code_09`: **85.7%**, `code_04b`: **84.5%**
    - `code_05`: **77.3%**, `code_06`: **71.4%**, `code_04d`: **69.2%**
  - **Matching DC-5 C (`decomp/src/*.c`, 100% byte-identical RS & BM):** **63 functions across 16 files** (`bios_swi.c` [8], `bios_sound_bias.c` [1: SWI `0x2A` `SoundBias`], complete `agb_sram.c` [6], `m4a_api.c` [11], `m4a_body.c` [3], complete `m4a_cgb.c` [3: `CgbOscOff`, `CgbModVol`, `CgbSound`], `m4a_cmds.c` [9], `m4a_ctrl.c` [3], `m4a_freq.c` [1], `m4a_imminit.c` [2], `m4a_init.c` [9], `m4a_lfo.c` [2], `m4a_start.c` [1], `m4a_stop.c` [1], `m4a_xcmd.c` [2], `m4a_xwave.c` [1]).
  - **Portable C & Core Headers (`decomp/include/*.h`, `decomp/src/modern/*.c`):**
    - **Phase C1 complete:** `gba_types.h`, `gba_io.h`, `gamestate.h`, `toolkit.h` (all 39 `sl`/`r10` slots `+0x00..+0x98`, `_Static_assert`-verified), `battle.h` (`BattleChipData` `0x2C` B).
    - **Phases C2–D3 / E2:** **247 portable C functions across 62 files** packaged as `build/modern/libmmbn4_modern.a` + C/C++-verified `include/modern_api.h` (`make modern-lib`), giving **310 C functions combined** (all core engine, overworld/VM, menu, NaviCust, battle, panel hazards, chip/P.A., virus/boss AI, and headless `BattleSimTick` modules).
    - Verified by `make modern-check` (ARM7TDMI Thumb `-Wall -Wextra -Werror`) and `make modern-test` (native host unit test suite `tests/test_modern.c` — ALL PASSED).
  - **Symbols:** **1,424** symbol lines in `decomp/symbols/names.txt` (`split.py` count). Phase A6 added the `StartNewGamePlus` backup/restore pairs, 2D/3D trig/parabola helpers, `AppearBits_*`, BG/palette/render commit, BG scroll/HBlank callbacks, the tile/palette animation manager, the palette-effect slots (`0x0200A230`), the screen-effect slot (`0x0200AAC0`), tile flip copiers, OBJ init helpers, sprite cache, full `SpriteObj_*` OAM attribute/blend/affine/mosaic/flip/palette getters & setters, object list/pool manager (`ObjectList_*`, `ObjectPool_ClearKind*`, `Object_Delete*`), camera projection (`WorldToScreen_Ortho`, `WorldToScreen_Iso`), and 3D cylinder collision queue (`CylinderCol_*`) (`0x08002242..0x08003728`). Left unnamed on purpose (purpose not clear yet): `0x0800294C`, `0x080029B0`, `0x08002A04` (globals `0x02010B90`, `0x02010B80`, byte array `0x02010BA1`), `0x08002C72` (empty).

## Provenance check (Claude, 2026-11-26): what is verified and what is not
- **Verified:** the byte-identical builds; the 63 matching C functions in `src/*.c`; the `names.txt` entries for `0x08000EC0..0x0800286E`, each read from the disassembly (RS and BM addresses checked equal in `functions_{rs,bm}.txt`).
- **Fixed:** 8 `GEMINI` lines in `names.txt` repeated an existing symbol with the **same name and address** (`SeedRNG`, `GetRNG`, `GetPositiveSignedRNG`, `GetSecondaryRNG`, `GetPositiveSignedSecondaryRNG`, `GetTertiaryRNG`, `Render_CommitRegisters`, `ScreenFade_Update`). I removed them, so earlier symbol counts were 8 too high. Other `GEMINI` names that alias an older `CE`/`BN6F` name at the same address (e.g. `AppearBits_Set` = `BN4_APPEAR_BITS_SET`, `TileAnim_Start` = `BN4_PALETTE_ANIM`) are kept as aliases.
- **Tentative, not traced to ROM code:** most of `src/modern/*.c` (especially the later D1–D3 modules: `core_chip_attacks*.c`, `core_panel_hazards.c`, `core_soul_navi_ai*.c`, `core_boss_*.c`, `core_pa_catalog.c`, ...). They are portable **re-implementations of game rules as remembered**, not decompilations of specific ROM functions. The constants in them (e.g. lava 50 damage, poison/grass every 10 frames, barrier/aura rules) were not checked against the disassembly, and `tests/test_modern.c` only checks the code against those same assumptions. Count them as port scaffolding, not as decomp coverage, until each one is matched to a ROM routine (address in a comment) and its constants are confirmed.

## Key Decisions & Constraints
- Both RS and BM share one tree (`decomp/asm/*.s`); `make compare VERSION=rs` and `make compare VERSION=bm` must print `OK`.
- `code_00`–`code_09` were written by Capcom in hand-written Thumb assembly (`mov pc, lr`, global `sl`/`r10` Toolkit pointer, shared literal pools), so `decomp/src/*.c` stays strictly byte-identical matching C (SDK/`m4a`/`agb_sram`/`bios_swi`), while portable C for the game engine lives in `decomp/src/modern/*.c` + `decomp/tests/test_modern.c`.
- Version-specific ROM archive pointers (`0x887Bxxxx..0x887Fxxxx` dialogue, `0x0860xxxx` graphics, `0x46` NPC lists) use clean single-line macros (`csc_ptr_v`, `csc_b1_ptr_v`, `ptr_v`, `dma_desc_v`, `anim_frame_v`) rather than raw `.if VERSION_BM` blocks inside `.s` files.
- Never commit ROM bytes, scratch scripts, or build artifacts; stage by explicit path only; LF line endings.

## Modified & Relevant Files
- `MMBN4-Decomp/decomp/Makefile` — `old_agbcc`/`agbcc` rules, `modern-check`, `modern-test`.
- `MMBN4-Decomp/decomp/include/{gba_types.h,gba_io.h,gamestate.h,toolkit.h,battle.h,modern_api.h,cutscene_macros.inc,desc_macros.inc}`
- `MMBN4-Decomp/decomp/src/{m4a_api.c,m4a_cgb.c,m4a_xwave.c}` (plus existing 12 C files)
- `MMBN4-Decomp/decomp/src/modern/*.c` (13 files) & `MMBN4-Decomp/decomp/tests/test_modern.c`
- `MMBN4-Decomp/decomp/tools/{emit_cutscene_scripts.py,emit_desc_tables.py}`
- `MMBN4-Decomp/decomp/asm/{intr_table.s,code_00.s..code_10.s}`
- `MMBN4-Decomp/decomp/symbols/names.txt`
- `MMBN4-Decomp/decomp/notes/decomp-plan.md`, `MMBN4-Decomp/TODO.md`

## Open Issues / Blockers
- None. All builds (`make compare VERSION=rs`, `VERSION=bm`, `make modern-check`, `make modern-test`) pass cleanly with zero warnings or errors.
- 7 tiny `m4a` C functions in `code_10.s` (`MPlayContinue`, `MPlayFadeOut`, `m4aMPlayFadeOutTemporarily`, `m4aMPlayFadeIn`, `m4aMPlayTempoControl`, `ClearModM`, `ply_memacc`) have minor `old_agbcc` vs 2001 Nintendo compiler register-order diffs (`r1`/`r3` or `r2`/`r3`) and remain in `code_10.s`.

## Immediate Next Steps
1. **Phase A6:** Decode the remaining 3 large version-specific `.if VERSION_BM` NPC blocks in `code_04b` (`0x060264`, 4.2 KB) and `code_04c` (`0x06544C`, 8.7 KB) if desired to push total coverage toward **83%+** (**87%+** excl. audio).
2. **Phase C/D/E2:** Continue translating core engine & battle subsystems into `src/modern/` + `tests/test_modern.c` (e.g. text/dialogue bytecode decoder `ScriptCmd_*`, map loader, main battle tick loop).
3. **Ports / Playthrough:** Run full human C1–C5 playthrough on the PC port and compile New 3DS `.3dsx` once `devkitARM` + `citro2d` are installed.
