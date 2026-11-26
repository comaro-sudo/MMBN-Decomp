# MMBN4 Decompilation Plan (Ordered Roadmap)

*Updated (2026-11-26 after C3/C4 `core_friendly_matches.c` & `core_navicust_bugs.c` — **44 modern modules / 238 C functions combined & 82.6% total / 86.3% excl. audio!**): **82.6% code ownership** (`974,351 / 1,179,456` bytes, `205,105` bytes `incrom` remaining — **86.3%** excluding `code_10`'s `49,992 B` audio tail; **62 byte-identical C functions** across 15 files + `include/gba_types.h`; **176 portable C functions** across 44 `src/modern/` files packaged as `libmmbn4_modern.a` + C/C++-verified `include/modern_api.h` = **238 C functions combined**; **1,179** verified symbols; RS & BM 100% byte-identical).*

## Where the Remaining `393,394 B` of `incrom` Lives

| Bucket | Segments | Owned / Total | Remaining `incrom` | % of Remaining |
|---|---|---:|---:|---:|
| Scripts, map-init & cutscene data | `code_04`..`code_06` | `142,084 / 397,250 B` | `255,166 B` | 64.9% |
| SDK / `m4a` / `code_10` tail | `code_10` | `35,056 / 90,384 B` | `55,328 B` | 14.1% |
| Engine tables & pools | `code_00`..`code_03` | `281,288 / 323,520 B` | `42,232 B` | 10.7% |
| Battle / AI / chip tables | `code_07`..`code_09` | `327,278 / 367,890 B` | `40,612 B` | 10.3% |
| Boot / IRQ table | `crt0`, `intr_table` | `356 / 412 B` | `56 B` | <0.1% |

---

## Phase A — Finish Bytecode, Script & Table Disassembly (66.6% → ~90%+)

Goal: Eliminate nearly all opaque `incrom` inside `code_00`..`code_10` using verified, RS+BM-shared symbolic macros and data tables (zero new `.if VERSION_BM` blocks unless truly version-specific).

| Task | Scope | Target |
|---|---|---|
| **A1** | ~~**Finish current batch:**~~ **DONE (2026-11-26):** Added cycle guards to `emit_map_scripts.py` & `emit_cutscene_scripts.py`, emitted 379 pointer tables + 197 cutscene blocks + 127 map-script blocks (`+14,506 B`), verified RS & BM `OK`. | `code_04`..`code_06` (**66.8%**) |
| **A2** | ~~**Audit `code_10` tail (`0x08113CB6..0x08120000`, `50 KB`):**~~ **DONE (2026-11-26):** Confirmed `0x08113CB8..0x08120000` (`49,992 B`, already marked `data_from` in `segments_rs.toml`) is the build timestamp (`"Apr 16 2004 14:40:18"`) + `m4a` audio data (stays `incrom` per no-ROM-data rule; real `code_10` code is already **86.8%** owned). | `code_10` audited |
| **A3** | **Complete `code_04`–`code_06` script & data decoding:** *(Parts 1–5 DONE: emitted **3,036 NPC lists & script blocks / +81,356 B** + **1,280 cutscene blocks / +51,114 B** incl. `0x1A` + `csc_b1_ptr_v`; `code_05` is **73.9%** and `code_06` crossed **70.4%**!)* Remaining: `0x39` multi-way jump tables and remaining non-script event/warp/prop/VRAM descriptor structs. | `code_04`..`code_06` (now 53.6–73.9% → 80%+) |
| **A4** | **Carve remaining `code_00`–`code_03` tables (`35.8 KB`):** *(Pointer tables + `intr_table` 100% + `emit_desc_tables.py` DONE: `code_00` **92.3%**, `code_01` **87.9%**, `code_02` **90.9%**, `code_03` **86.0%**.)* Remaining: static lookup/menu/non-pointer structs. | `code_00`..`code_03` (86–92% → 97%+) |
| **A5** | **Carve remaining `code_07`–`code_09` tables (`24.1 KB`):** *(Pointer tables + `emit_desc_tables.py` DONE: `code_07` **96.6%**, `code_08` **96.6%**, `code_09` **85.7%**!)* Remaining: battle chip/enemy/AI non-pointer structs. | `code_07`..`code_09` (86–97% → 97%+) |
| **A6** | **Clean up version-specific script/table gaps:** *(In progress 2026-11-26: promoted **+65** verified `agb_sram`, `m4a`, and `gIntrTableTemplate` symbols into `symbols/names.txt` → **1,175** symbols; decoded all 3 large version-specific `.if VERSION_BM` NPC list + `NpcRunScript` blocks in `code_04` [`73.4% → 86.8%`], `code_04b` [`63.2% → 84.5%`], and `code_04c` [`53.6% → 86.3%`] via `tools/emit_versioned_npcs.py` → **+19,576 B**, **82.6%** total / **86.3%** excl. audio.)* Remaining: promote high-value engine `sub_`/`dat_` labels and inspect remaining small versioned blocks. | All segments (~90–95%+) |

---

## Phase B — Nintendo SDK & Library C Decompilation (`code_10` → 100% C)

Goal: Finish all third-party/SDK C libraries first — they are cleanly compiled with `pret/agbcc` / `old_agbcc` and give reusable headers/types for the rest of the game.

| Task | Scope | Target |
|---|---|---|
| **B1** | **Complete `m4a` C (`src/m4a_*.c`):** *(48 C funcs done — ~92% of all `m4a.c` bytes!: `m4a_api.c` [11: `m4aSoundInit`..`m4aMPlayFadeOut`], `m4a_body.c` [3], complete `m4a_cgb.c` [3: `CgbOscOff`, `CgbModVol`, `CgbSound`], `m4a_cmds.c` [9], `m4a_ctrl.c` [3], `m4a_freq.c` [1], `m4a_imminit.c` [2], `m4a_init.c` [9], `m4a_lfo.c` [2], `m4a_start.c` [1], `m4a_stop.c` [1], `m4a_xcmd.c` [2], `m4a_xwave.c` [1].)* Remaining: 7 tiny `old_agbcc` register-order quirks (`MPlayContinue`, `MPlayFadeOut`, `m4aMPlayFadeOutTemporarily`, `m4aMPlayFadeIn`, `m4aMPlayTempoControl`, `ClearModM`, `ply_memacc`); `MPlayMain`/`TrackStop`/`ply_note` are hand-written `m4a_1.s` asm. | `code_10` (~7 small funcs left) |
| **B2** | **GBA BIOS / libc / libgcc helpers (`code_10`):** Verify `bios_swi.c` (8 done), `agb_sram.c` (6/6 done), and decompile/document remaining runtime helpers (`__aeabi`/div/mod/call_via_rX, `MultiBoot` if present). | `code_10` SDK complete |

---

## Phase C — Core Engine & Shared Headers (`code_00`–`code_03` → Matching C)

Goal: Establish canonical C headers (`include/gba.h`, `include/structs.h`, `include/toolkit.h`, `include/save.h`) and decompile core engine modules bottom-up (leaf functions → managers).

| Task | Scope | Target |
|---|---|---|
| **C1** | ~~**Core headers & Toolkit struct (`include/`):**~~ **DONE (2026-11-26):** `include/gba_types.h`, `include/gba_io.h`, `include/gamestate.h` (`MainState`, `JoypadState`, `GameState`, `ProgressState`), `include/toolkit.h` (all 39 `sl`/`r10` slots `+0x00..+0x98`), and `include/battle.h` (`BattleChipData`, `BattleNaviState`, `DoubleSoulId`, RNG). Verified with `_Static_assert` + `gcc`/`agbcc`/`old_agbcc`. | Shared C headers |
| **C2** | **Core utility & math (`code_00`):** *(In progress: `core_progress.c` [11], `core_input.c` [5], `core_lz77.c` [2], `core_oam_dma.c` [4], `core_sound_mgr.c` [5], `core_camera.c` [3], `core_fade.c` [3], `core_fixed_math.c` [5: Q16.16 mul/lerp, Manhattan, 8-way facing], verified by `make modern-check` + `make modern-test`.)* | `src/modern/core_*.c` |
| **C3** | **Overworld, flags, save & inventory (`code_01`–`code_02`):** *(In progress: `src/modern/core_save.c` [5 funcs], `src/modern/core_inventory.c` [9 funcs], `src/modern/core_folder.c` [3 funcs: `FolderValidate`, `FolderCanSetRegular`, `FolderShuffleDeck`], `src/modern/core_map.c` [4 funcs], verified by `make modern-test`.)* Remaining: full map tile/collision loader, overworld script VM. | `src/modern/core_*.c` |
| **C4** | **Menus, text & UI (`code_03`–`code_04`):** *(In progress: `src/modern/core_world.c` [4], `core_shop.c` [5], `core_subchips.c` [3], `core_chip_trader.c` [3], `core_navicust.c` [5], `core_navi_stats.c` [4], `core_mail.c` [4], `core_text_script.c` [3], `core_library.c` [4], all verified by `make modern-test`.)* Remaining: PET menu renderer/layout hooks. | `src/modern/core_*.c` |

---

## Phase D — Battle Engine & AI (`code_07`–`code_09` → Matching C)

Goal: Decompile the combat system, chip behaviors, Souls, and boss/virus AI.

| Task | Scope | Target |
|---|---|---|
| **D1** | **Battle core & Custom Screen (`code_07`):** *(In progress: `src/modern/core_battle_soul.c`, `src/modern/core_battle_score.c`, `src/modern/core_battle_grid.c`, `src/modern/core_battle_loop.c`, `src/modern/core_battle_turn.c` [4 funcs: Intro/Custom/Combat/Victory/Defeat coordinator + DoubleSoul 3-turn timer], `src/modern/core_custom_screen.c`, and `src/modern/core_emotion.c`, all verified by `make modern-test`.)* Remaining: full entity/projectile scheduler. | `src/modern/core_*.c` |
| **D2** | **Chip attacks & projectiles (`code_07`–`code_08`):** *(In progress: `src/modern/core_chips.c` [5 funcs] + `src/modern/core_projectiles.c` [3 funcs: `ProjectileSpawn`, `ProjectileStep`, `ProjectileCheckHit`], verified by `make modern-test`.)* Remaining: Standard, Mega, Giga, Dark, and P.A. attack state machines. | `src/modern/core_*.c` |
| **D3** | **Virus & Navi AI (`code_08`–`code_09`):** *(In progress: `core_enemy_ai.c` [5], `core_battle_rewards.c` [3], `core_status_effects.c` [4], `core_virus_ai.c` [4], `core_boss_patterns.c` [3: HP enrage phase, Bass Aura regen, counter-hit window], verified by `make modern-test`.)* Remaining: per-boss attack tables. | `src/modern/core_*.c` |

---

## Phase E — Verification, Portability & Violet Eclipse Integration

| Task | Scope | Target |
|---|---|---|
| **E1** | **100% matching RS & BM verification:** Both `make -B compare VERSION=rs` and `VERSION=bm` pass with 0 `incrom` code bytes left. | 100% matching decomp |
| **E2** | **Non-matching / portable C build (`MODERN` target):** *(In progress: C/C++-verified `include/modern_api.h` + `make modern-check` [ARM7TDMI Thumb `-Wall -Wextra -Werror`] + `make modern-lib` [`build/modern/libmmbn4_modern.a`] + `make modern-test` [consumes `modern_api.h` + `libmmbn4_modern.a` — ALL PASSED across 44 modules / 176 portable C functions].)* Guard GBA-specific idioms so decompiled C compiles natively for PC / Wii U / 3DS / future ports. | Native C port readiness |

## Recommended Execution Order

1. **Immediate (today):** **A1** (finish current batch + commit) → **B1** (next batch of `m4a` C functions) → **A2** (`code_10` 50 KB tail audit).
2. **Short term (next few sessions):** **A3 → A4 → A5** to push total code ownership past **85–90%**, while continuing **B1** to finish all of `m4a` in C.
3. **Medium/long term:** **C1** (shared C headers) → **C2–C4** (core engine & menus) → **D1–D3** (battle & AI) → **E1–E2** (100% matching + portable C).
