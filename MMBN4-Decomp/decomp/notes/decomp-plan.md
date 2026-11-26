# MMBN4 Decompilation Plan (Ordered Roadmap)

*Updated (2026-11-26 after A1 + A2 + B1 batch): **66.8% code ownership** (`787,455 / 1,179,456` bytes, `392,001` bytes `incrom` remaining; **33 byte-identical C functions** across 7 files; RS & BM 100% byte-identical).*

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
| **A3** | **Complete `code_04`–`code_06` script & data decoding:** Add remaining variable-length opcodes (`0x07` map-script mem compare; `0x1A`/`0x39` cutscene ops), decode NPC/warp/prop/event descriptor structs and remaining pointer tables across `code_04`..`code_06`. | `code_04`..`code_06` (20–52% → 75–85%+) |
| **A4** | **Carve remaining `code_00`–`code_03` tables (`42 KB`):** Pointer tables, struct arrays (`uuww`, `uw`, DMA/VRAM/map/menu descriptors), and `intr_table` (`48B`). | `code_00`..`code_03` (84–91% → 97%+) |
| **A5** | **Carve remaining `code_07`–`code_09` tables (`40 KB`):** Battle chip/enemy/AI/attack/animation pointer & descriptor tables. | `code_07`..`code_09` (83–92% → 97%+) |
| **A6** | **Clean up version-specific script/table gaps:** Handle remaining RS-vs-BM differing script blocks cleanly where needed, and promote high-value unnamed `dat_`/`sub_` labels into `symbols/names.txt`. | All segments (~90–95%+) |

---

## Phase B — Nintendo SDK & Library C Decompilation (`code_10` → 100% C)

Goal: Finish all third-party/SDK C libraries first — they are cleanly compiled with `pret/agbcc` / `old_agbcc` and give reusable headers/types for the rest of the game.

| Task | Scope | Target |
|---|---|---|
| **B1** | **Complete `m4a` C (`src/m4a_*.c`):** Decompile remaining `m4a` functions (`m4aSoundVSyncOff`/`On`, `m4aMPlayInit`, `m4aMPlayStop`, `m4aMPlayFadeOut*`, `MPlayMain`, `TrackStop`, `CgbSound`/`CgbOscOff`/`CgbModVol`, `ply_*` handlers, `m4aSoundInit`). | `code_10` (~30–40 more C funcs) |
| **B2** | **GBA BIOS / libc / libgcc helpers (`code_10`):** Verify `bios_swi.c` (8 done), `agb_sram.c` (6/6 done), and decompile/document remaining runtime helpers (`__aeabi`/div/mod/call_via_rX, `MultiBoot` if present). | `code_10` SDK complete |

---

## Phase C — Core Engine & Shared Headers (`code_00`–`code_03` → Matching C)

Goal: Establish canonical C headers (`include/gba.h`, `include/structs.h`, `include/toolkit.h`, `include/save.h`) and decompile core engine modules bottom-up (leaf functions → managers).

| Task | Scope | Target |
|---|---|---|
| **C1** | **Core headers & Toolkit struct (`include/`):** Define `Toolkit` (`sl` / `r10` pointer table), `GameState`, `SaveData`, `BattleState`, `Entity`, `Input`, `DMA/VRAM` structs from `docs/re/`. | Shared C headers |
| **C2** | **Core utility & math (`code_00`):** RNG, `ZeroFillByWord`, `CopyWords`, LZ77/VRAM/DMA queue helpers, input/joypad, OAM/sprite managers, sound wrappers (`BN4_PLAY_MUSIC`). | `src/core_*.c` |
| **C3** | **Overworld, flags, save & inventory (`code_01`–`code_02`):** Save/load & anti-cheat relocation, event flag bitfields, key items, chip pack/folder helpers, map loader, `map_scripts_commands` & `BN4_CUTSCENE_COMMANDS` interpreters. | `src/overworld_*.c`, `src/save.c`, `src/scripts.c` |
| **C4** | **Menus, text & UI (`code_03`–`code_04`):** Text/dialogue bytecode engine (`ScriptCmd_*`), PET menus (Folder, NaviCust, Mail, Status, Key Items, Network/Link), shops, tournaments. | `src/menu_*.c`, `src/text.c` |

---

## Phase D — Battle Engine & AI (`code_07`–`code_09` → Matching C)

Goal: Decompile the combat system, chip behaviors, Souls, and boss/virus AI.

| Task | Scope | Target |
|---|---|---|
| **D1** | **Battle core & Custom Screen (`code_07`):** Grid/panel state, battle loop, Custom Gauge, chip selection, Double Soul / Dark Soul / emotion window, hit/damage/element calculation. | `src/battle_*.c` |
| **D2** | **Chip attacks & projectiles (`code_07`–`code_08`):** Standard, Mega, Giga, Dark, and P.A. attack state machines. | `src/chips_*.c` |
| **D3** | **Virus & Navi AI (`code_08`–`code_09`):** Virus AI, Soul Navi bosses, Duo, Bass, and SP/SP+ scaling hooks. | `src/ai_*.c` |

---

## Phase E — Verification, Portability & Violet Eclipse Integration

| Task | Scope | Target |
|---|---|---|
| **E1** | **100% matching RS & BM verification:** Both `make -B compare VERSION=rs` and `VERSION=bm` pass with 0 `incrom` code bytes left. | 100% matching decomp |
| **E2** | **Non-matching / portable C build (`MODERN` target):** Guard GBA-specific inline asm/register idioms so the decompiled C can also compile natively for PC / Wii U / 3DS / future ports (bridging Phase 2 emulation layer → Phase 3 native port). | Native C port readiness |

## Recommended Execution Order

1. **Immediate (today):** **A1** (finish current batch + commit) → **B1** (next batch of `m4a` C functions) → **A2** (`code_10` 50 KB tail audit).
2. **Short term (next few sessions):** **A3 → A4 → A5** to push total code ownership past **85–90%**, while continuing **B1** to finish all of `m4a` in C.
3. **Medium/long term:** **C1** (shared C headers) → **C2–C4** (core engine & menus) → **D1–D3** (battle & AI) → **E1–E2** (100% matching + portable C).
