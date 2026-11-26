# MMBN4 Decompilation Plan (Ordered Roadmap)

*Updated (2026-11-26 after A4/A5 `emit_desc_tables.py` — **81.0% total / 84.5% excl. audio tail!**): **81.0% code ownership** (`954,775 / 1,179,456` bytes, `224,681` bytes `incrom` remaining — **84.5%** excluding `code_10`'s `49,992 B` audio tail; **62 byte-identical C functions** across 15 files + `include/gba_types.h`; RS & BM 100% byte-identical).*

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
| **A6** | **Clean up version-specific script/table gaps:** Handle remaining RS-vs-BM differing script blocks cleanly where needed, and promote high-value unnamed `dat_`/`sub_` labels into `symbols/names.txt`. | All segments (~90–95%+) |

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
| **C2** | **Core utility & math (`code_00`):** *(In progress: `src/modern/core_progress.c` [11 funcs: game time, playthrough, RNG, event flags] + `src/modern/core_input.c` [5 funcs: `Joypad_Poll`, `CheckSoftReset`, `ZeroFillByWord`, `CopyBytes`, `CopyWords`], verified by `make modern-check`.)* Remaining: LZ77/VRAM/DMA queue helpers, OAM/sprite managers, `BN4_PLAY_MUSIC`. | `src/modern/core_*.c` |
| **C3** | **Overworld, flags, save & inventory (`code_01`–`code_02`):** *(In progress: `src/modern/core_save.c` [5 funcs: `RebuildSavePointers`, `XorSaveImage`, `ComputeSaveChecksum`, `WriteSaveMagic`, `VerifySaveMagic`] + `src/modern/core_inventory.c` [9 funcs: `GetChipRecord`, folder bitfield helpers, `ChipPackCountById`, `AddZenny`, `AddBugFrags`, `GetItemCount`/`AddItemClamped`].)* Remaining: map loader, script interpreters. | `src/modern/core_*.c` |
| **C4** | **Menus, text & UI (`code_03`–`code_04`):** Text/dialogue bytecode engine (`ScriptCmd_*`), PET menus (Folder, NaviCust, Mail, Status, Key Items, Network/Link), shops, tournaments. | `src/menu_*.c`, `src/text.c` |

---

## Phase D — Battle Engine & AI (`code_07`–`code_09` → Matching C)

Goal: Decompile the combat system, chip behaviors, Souls, and boss/virus AI.

| Task | Scope | Target |
|---|---|---|
| **D1** | **Battle core & Custom Screen (`code_07`):** *(Started 2026-11-26: `src/modern/core_battle_soul.c` [`FindDoubleSoulForChip` / `BN4_UNITE_CHECK` for RS, BM, and Violet Eclipse all-12 Souls].)* Remaining: grid/panel state, battle loop, Custom Gauge, chip selection, emotion window, hit/damage/element calculation. | `src/modern/core_battle_*.c` |
| **D2** | **Chip attacks & projectiles (`code_07`–`code_08`):** Standard, Mega, Giga, Dark, and P.A. attack state machines. | `src/chips_*.c` |
| **D3** | **Virus & Navi AI (`code_08`–`code_09`):** Virus AI, Soul Navi bosses, Duo, Bass, and SP/SP+ scaling hooks. | `src/ai_*.c` |

---

## Phase E — Verification, Portability & Violet Eclipse Integration

| Task | Scope | Target |
|---|---|---|
| **E1** | **100% matching RS & BM verification:** Both `make -B compare VERSION=rs` and `VERSION=bm` pass with 0 `incrom` code bytes left. | 100% matching decomp |
| **E2** | **Non-matching / portable C build (`MODERN` target):** *(Started 2026-11-26: `make modern-check` [ARM7TDMI Thumb `-Wall -Wextra -Werror`] + `make modern-test` [native host `tests/test_modern.c` — ALL PASSED across 5 modules / 32 functions].)* Guard GBA-specific idioms so decompiled C compiles natively for PC / Wii U / 3DS / future ports. | Native C port readiness |

## Recommended Execution Order

1. **Immediate (today):** **A1** (finish current batch + commit) → **B1** (next batch of `m4a` C functions) → **A2** (`code_10` 50 KB tail audit).
2. **Short term (next few sessions):** **A3 → A4 → A5** to push total code ownership past **85–90%**, while continuing **B1** to finish all of `m4a` in C.
3. **Medium/long term:** **C1** (shared C headers) → **C2–C4** (core engine & menus) → **D1–D3** (battle & AI) → **E1–E2** (100% matching + portable C).
