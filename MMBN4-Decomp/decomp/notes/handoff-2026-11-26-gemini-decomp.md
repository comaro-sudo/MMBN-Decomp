# Gemini Decomp Handoff (2026-11-26)

## Goal & Current Status
- **Goal:** Advance the MMBN4 (Red Sun & Blue Moon) decompilation along [`decomp/notes/decomp-plan.md`](decomp-plan.md) while keeping both ROMs 100% byte-identical (`make compare VERSION=rs` and `VERSION=bm`).
- **Verified Status (pushed to `origin/main`):**
  - **Code ownership:** **82.6%** total (`974,351 / 1,179,456` bytes; **86.3%** excluding `code_10`'s `49,992 B` build-timestamp + `m4a` audio tail).
    - `intr_table`: **100.0%** (`48 / 48 B`)
    - `code_07` & `code_08`: **96.6%** each
    - `code_00`: **92.3%**, `code_02`: **90.9%**, `code_01`: **87.9%**, `code_04`: **86.8%**, `code_04c`: **86.3%**, `code_03`: **86.0%**, `code_09`: **85.7%**, `code_04b`: **84.5%**
    - `code_05`: **77.3%**, `code_06`: **71.4%**, `code_04d`: **69.2%**
  - **Matching DC-5 C (`decomp/src/*.c`, 100% byte-identical RS & BM):** **62 functions across 15 files** (`bios_swi.c` [8], complete `agb_sram.c` [6], `m4a_api.c` [11], `m4a_body.c` [3], complete `m4a_cgb.c` [3: `CgbOscOff`, `CgbModVol`, `CgbSound`], `m4a_cmds.c` [9], `m4a_ctrl.c` [3], `m4a_freq.c` [1], `m4a_imminit.c` [2], `m4a_init.c` [9], `m4a_lfo.c` [2], `m4a_start.c` [1], `m4a_stop.c` [1], `m4a_xcmd.c` [2], `m4a_xwave.c` [1]).
  - **Portable C & Core Headers (`decomp/include/*.h`, `decomp/src/modern/*.c`):**
    - **Phase C1 complete:** `gba_types.h`, `gba_io.h`, `gamestate.h`, `toolkit.h` (all 39 `sl`/`r10` slots `+0x00..+0x98`, `_Static_assert`-verified), `battle.h` (`BattleChipData` `0x2C` B).
    - **Phases C2–D3 / E2:** **147 portable C functions across 34 files** (`core_progress.c`, `core_input.c`, `core_lz77.c`, `core_oam_dma.c`, `core_sound_mgr.c`, `core_camera.c`, `core_fade.c`, `core_save.c`, `core_inventory.c`, `core_folder.c`, `core_map.c`, `core_world.c`, `core_shop.c`, `core_subchips.c`, `core_chip_trader.c`, `core_navicust.c`, `core_navi_stats.c`, `core_mail.c`, `core_text_script.c`, `core_library.c`, `core_battle_soul.c`, `core_battle_score.c`, `core_battle_rewards.c`, `core_battle_grid.c`, `core_battle_loop.c`, `core_battle_turn.c`, `core_chips.c`, `core_projectiles.c`, `core_custom_screen.c`, `core_emotion.c`, `core_enemy_ai.c`, `core_virus_ai.c`, `core_boss_patterns.c`, `core_status_effects.c`), giving **209 C functions combined**.
    - Verified by `make modern-check` (ARM7TDMI Thumb `-Wall -Wextra -Werror`) and `make modern-test` (native host unit test suite `tests/test_modern.c` — ALL PASSED).
  - **Symbols:** **1,179** verified symbols in `decomp/symbols/names.txt` (+69 promoted in Phase A6).

## Key Decisions & Constraints
- Both RS and BM share one tree (`decomp/asm/*.s`); `make compare VERSION=rs` and `make compare VERSION=bm` must print `OK`.
- `code_00`–`code_09` were written by Capcom in hand-written Thumb assembly (`mov pc, lr`, global `sl`/`r10` Toolkit pointer, shared literal pools), so `decomp/src/*.c` stays strictly byte-identical matching C (SDK/`m4a`/`agb_sram`/`bios_swi`), while portable C for the game engine lives in `decomp/src/modern/*.c` + `decomp/tests/test_modern.c`.
- Version-specific ROM archive pointers (`0x887Bxxxx..0x887Fxxxx` dialogue, `0x0860xxxx` graphics, `0x46` NPC lists) use clean single-line macros (`csc_ptr_v`, `csc_b1_ptr_v`, `ptr_v`, `dma_desc_v`, `anim_frame_v`) rather than raw `.if VERSION_BM` blocks inside `.s` files.
- Never commit ROM bytes, scratch scripts, or build artifacts; stage by explicit path only; LF line endings.

## Modified & Relevant Files
- `MMBN4-Decomp/decomp/Makefile` — `old_agbcc`/`agbcc` rules, `modern-check`, `modern-test`.
- `MMBN4-Decomp/decomp/include/{gba_types.h,gba_io.h,gamestate.h,toolkit.h,battle.h,cutscene_macros.inc,desc_macros.inc}`
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
