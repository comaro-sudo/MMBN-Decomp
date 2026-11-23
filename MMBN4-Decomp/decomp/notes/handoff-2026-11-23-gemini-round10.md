# Handoff: Gemini Round 10 (2026-11-23)

## 1. Goal & Current Status

Completed **all 7 tasks in order** from `MMBN4-Decomp/decomp/notes/task-gemini-round10.md` without touching `patch/`, `docs/`, or `TODO.md` (Claude's active area):

1. **Task 1 — In-game regression test (`patch-test-results-3.md`, VERIFIED):** Built current patch (`python patch/build.py` & `--check` PASS) and tested all features in the PC port (`mmbn4.exe`, screenshots outside repo in scratch). All core features pass (lobby Soul-flag hiding/showing, exact operator sprites, Dex → GutsMan V1 rematch, Trash Can → WtrGodComp1 JunkMan warp, 2-page Status screen, Free Tournament 7 Soul Navis / 0 traded, Frequency Tuner `0x0E58`, Z-Saver BMD). **One visual bug found & diagnosed for Claude:** `lobby_npc_trashcan` drew a white circle on ACDC Town because `list[7][0x44]` only has 3 animations (`0..2`) and `.byte 0x16, 3` selected anim `3` — 1-byte fix is `.byte 0x16, 0`.
2. **Task 2 — Merged Round 8–9 names & full regeneration (COMMITTED `95c6e92`, VERIFIED):** Merged 19 verified RS/BM symbols into `decomp/symbols/names.txt`, regenerated RS + BM disassembly + `merge_bm.py`, and verified **`build/rs/bn4_rs.gba: OK`** and **`build/bm/bn4_bm.gba: OK`**.
3. **Task 3 — Carved decoded record tables (COMMITTED, VERIFIED):** Carved `BN4_FORM_ELEMENTS`, `BN4_FORM_COMPANIONS`, `BN4_FORM_FRAMES`/`DURATIONS`, `BN4_FORM_TURNS`, `BN4_FORM_LOADOUTS`, `BN4_CHIP_TABLE` (389 × 44 B), `BN4_SOUL_KINDS`, `BN4_SOUL_FLAGS`, `BN4_SHOP_RECORDS`, `BN4_SHOP_STOCK_INIT`, `BN4_NUMBER_TRADER_TABLE`, and `BN4_NUMBER_TRADER_LUT` as structured `fmt = "record"` tables in `segments_rs.toml` & `segments_bm.toml`. Regenerated `code_00`, `code_01`, `code_03` and verified both **`build/rs/bn4_rs.gba: OK`** and **`build/bm/bn4_bm.gba: OK`**.
   - **Coverage before:** `700,816 / 1,179,456 B` (**59.4%**)
   - **Coverage after:** `722,268 / 1,179,456 B` (**61.2%**, **+21,452 bytes**; `code_01` `61.3% → 86.3%`, `code_03` `77.5% → 81.4%`)
4. **Task 4 — Friendly matches design (`friendly-matches-design.md`, VERIFIED):** Exact RS/BM NPC slot & list addresses for DenDome Hall `(02,01)` (6→9/16), Castillo WaitRm `(03,02)` (5→8/16), and Colosseum Lobby `(04,06)` (8→11/16); native NPC opcodes `0x42`/`0x43` (`JumpIfTournamentRound`) & `0x04` (gate directly on `!<soul_flag>`, no extra flags needed); all 9 placements, sprites, battle launch (`FC 0D`, `BN4_ID_RAISE` enabled), and `OpenMapDialogue` talk index `0xE0..0xE8` hook.
5. **Task 5 — SP+ design (`sp-plus-design.md`, VERIFIED):** 21 free flags `0x0E5D..0x0E71` (12 Soul Navis + 8 shared Navis + Bass; Duo uses `0x0E5A..0x0E5C` tier 4); zero encounter-table edits needed for roaming SP ghosts (10-entry SP enemy ID check inside `BattleSpawnEnemy` hook at `0x08006802`); concrete reward word **`0xC032` (50 BugFrags)** verified against `RewardDecode`'s 14-bit cap (`16,383` max); exact RS/BM hook code & register usage for all 4 hooks.
6. **Task 6 — Duo Soul design (`duo-soul-design.md`, VERIFIED):** MetalSoul palette block (`0x0826D8F8`, Palette 0 at `0x0826D8FC`, RS == BM) mapped to 16 Duo BGR555 colors; charge shot (`BN4_POWER_ATTACKS[0x19]` @ `0x0800CAE0` → 180-dmg Guard-Break Meteor Knuckle reusing action `0x14`); passive (`BN4_FORM_ENTERS[13]` @ `0x0800E0D4` → SuperArmor `0x0800E1CA` + Reflect Shield `0x6A`); 10 non-conflicting class-`0x0C` → `0x0D` unite chips; 3rd Status page extension (`0/6/12`); exact post-Duo grant code at `DuoDefeatedSetFlag` (`RS 0x080C4792 / BM 0x080C479E`); `0x16D` icon pointer (`0x0801D788`, `0x80` B 4bpp).
7. **Task 7 — Chip text design (`chip-text-design.md`, VERIFIED):** Confirmed `0x16D` (`DuoSoul`) **already has** both its name and description in both US ROMs (`748C14`/`74F03C` script `109`). Tested TextPet empirically on a scratch ROM: a partial `.tpl` without reading the ROM overwrites the archive, **but a 2-line `.tpi` + `read-text-archives rom.gba` + `read-text-archives patch.tpl --format tpl --patch`** preserves all other scripts and touches zero other archives (verified 1,256 B diff for `748C14`). Provided exact `.tpl` blocks (our lines only) for `Duo` (`0x137`) and `GP Power` (`0x138`).

---

## 2. Key Decisions & Constraints

- **Boundaries honored:** Zero edits to `patch/`, `docs/`, or `TODO.md`. Claude's uncommitted `patch/` files were left completely untouched.
- **Byte-identical decomp verified:** Both `make -B compare VERSION=rs` and `make -B compare VERSION=bm` passed after Tasks 2 and 3.
- **Not pushed:** All commits are local only, staged by explicit path.

---

## 3. Modified & Relevant Files

- **Committed (Task 2, `95c6e92`):** `MMBN4-Decomp/decomp/symbols/names.txt`, `MMBN4-Decomp/decomp/asm/{code_00,code_01,code_02,code_03,code_06,code_07,code_08,code_09,crt0}.s`.
- **Committed (Task 3):** `MMBN4-Decomp/decomp/config/{segments_rs,segments_bm}.toml`, `MMBN4-Decomp/decomp/asm/{code_00,code_01,code_03}.s`.
- **Committed (Round 10 notes):**
  - `MMBN4-Decomp/decomp/notes/patch-test-results-3.md`
  - `MMBN4-Decomp/decomp/notes/friendly-matches-design.md`
  - `MMBN4-Decomp/decomp/notes/sp-plus-design.md`
  - `MMBN4-Decomp/decomp/notes/duo-soul-design.md`
  - `MMBN4-Decomp/decomp/notes/chip-text-design.md`
  - `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round10.md`

---

## 4. Open Issues / Blockers

- **Visual bug for Claude to fix in `patch/src/common/wp_d3_lobby_rematches.asm`:** `lobby_npc_trashcan` should change `.byte 0x16, 3` → `.byte 0x16, 0` so `list[7][0x44]` draws the junk-pile prop instead of an out-of-range white circle.
- **Proposed names for next decomp pass:** 7 new symbols listed at the bottoms of `friendly-matches-design.md` (4) and `duo-soul-design.md` (3), ready to merge into `names.txt` in Round 11.

---

## 5. Immediate Next Steps

1. Claude applies the 1-byte Trash Can fix (`.byte 0x16, 0`) and implements Friendly Matches, SP+, Duo Soul, and Chip Text directly from the four Round 10 design specs.
2. In Round 11, Gemini can regression-test the newly implemented packages in `mmbn4.exe` and continue carving decoded tables / functions in `decomp/`.
