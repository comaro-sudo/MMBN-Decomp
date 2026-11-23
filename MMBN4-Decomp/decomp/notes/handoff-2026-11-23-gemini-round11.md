# Handoff: Gemini Round 11 (2026-11-23)

## 1. Goal & Current Status

Completed **all 7 tasks in order** from `MMBN4-Decomp/decomp/notes/task-gemini-round11.md` (zero edits to `patch/`, `docs/`, or `TODO.md`):

1. **Task 1 — In-game test of Claude's seven new packages (`patch-test-results-4.md`, VERIFIED):** Built private (`7708dc90...`) and `--public` (`cc4eeb84...`) RS builds (`--check` PASS) and tested all seven packages in `mmbn4.exe` (all outputs in `scratch/r11/`).
   - **PASS:** GP Power private chip art icon (`r11_priv_duo2.bmp`) & `--public` art-absent (`r11_pub_duo2.bmp`), GP Power BMD visible at ACDC Area 2 `(106, 326)`, Duo rematch NPC hidden until `0x0500` & `"Fight Duo?"` launches Duo V1 2000 HP, DuoSoul `UNITE` lights up for non-RegChip `Guard1 *` and sets bit 12 (`0x00001000`) of `0x0203BF88`, Status Screen Page 3 cycles cleanly via R/L (`r11_st_p3.bmp`).
   - **5 Real Bugs Diagnosed for Claude (with exact fixes in `patch-test-results-4.md`):**
     1. **Chip names & descriptions blank for `0x137`/`0x138`:** `sub_0802F154` (`RS 0x0802F1BC / BM 0x0802F1C0`) and `sub_08106BF0` (`RS 0x08106C28 / BM 0x08106C34`) have hardcoded `.word 0x00000137` e-Reader RAM redirects. **2-word fix:** change both to `.word 0x00000139`.
     2. **Trash Can (`list[7][0x44]`) & Duo's Signal (`list[6][0x63]`) still draw white circles:** both are LZ77-compressed (`0x88......`) sprites. Switch to uncompressed `list[7]` props (e.g. `list[7][0x08]` glowing blue crystal for Duo's Signal).
     3. **Duo rematch skipped results screen & `duo_defeat_hook` didn't set `0x0023`/`0x0E72`:** `b1 = 0x13` still suppresses the results screen (`0x020015EF` stayed `0x8D`, `0x0E5A=False`), and `0x0023`/`0x0E72` stayed `False` on Duo V1 1-HP win.
     4. **DuoSoul `UNITE` consumed `0x16D` without transforming MegaMan:** stays Normal MegaMan (`50` dmg charge shot, not `180`); downstream consumer of `[r7, #0x10] = 13` is rejecting `13` (`> 12`).
     5. **Stale marker `0x020015EF` confirmed:** fleeing Dex lobby rematch leaves `0x020015EF = 0x80` stale (solved by Task 2).
2. **Task 2 — Battle exit EWRAM clear hook (`battle-exit-hook.md`, VERIFIED):** `BattleExitCleanup` (`sub_08006E1E`, **RS == BM**) runs once on **every** battle exit (win after results screen, loss, flee, game over). Hook `0x08006E20` (`bl sub_080006D4`, `F9 F7 58 FC`) → veneer `RS 0x080C4788 / BM 0x080C4794` → single `str r1, [0x020015EC]` zeroes `0x020015EC..0x020015EF` atomically, then tail-calls `0x080006D4 | 1`.
3. **Task 3 — Duo form palette hook (`form-palette-hook.md`, VERIFIED):** `SpriteCopyPaletteToShadow` (`sub_0800295C`, **RS == BM**) already has a built-in custom palette pointer at **`[sprite_hdr + 0x34]`**! Hook `0x080107E8` (`bl BN4_SPRITE_PALETTE_SET` inside `BattleMegaManLoadFormSprite`, RS == BM) → veneer `RS 0x080C4790 / BM 0x080C479C` → store `=duo_soul_pal_set` (96 B: normal, hit-flash, shadow) into `[r3, #0x34]` when `BN4_NAVI_BYTE(0x24) == 13`, else `0`.
4. **Task 4 — Verified `BN4_FORM_ENTERS` passives (`duo-soul-passive.md`, VERIFIED):** Disassembled all 16 `BN4_FORM_ENTERS` entries and `BattleResetFormPassives` (`0x0800D8C2`). Confirmed `0x0800E1CA` is GutsSoul `+30` Null/Break hand ATK and `0x0800E208` is WindSoul `+10` Wind hand ATK. Proposed 3 verified Duo passives — **Option A (Recommended):** AirShoes+FloatShoes (`0x08012F82(0x30)`) + B+Left Reflect Shield (`[r5+0x58]->[0] = 0x6A`), both automatically cleaned up by `0x0800D8C2`.
5. **Task 5 — SP+ gaps & 80/20 reward (`sp-plus-design-2.md`, VERIFIED):** Confirmed Claude's `0x08006802` HP hook (`r2 = r1 | 0x0FFF`) and `0x08013A5A` attack hook; wrote full 10-entry roaming-SP scan (Hook 1) and full `BN4_OBJECT_RUNNER` `0x08003194` speed ×1.25 code (Hook 3); assigned Duo SP+ to flag `0x0E5C`; wrote exact 80% 15,000Z (`0x7A98`) / 20% 50 BugFrags (`0xC032`) RNG hook at `BN4_REWARD_CHOOSER` (`RS 0x080F57F4 / BM 0x080F5800`) using `GET_RNG` (`0x08001B16`).
6. **Task 6 — Friendly matches updated for `FC 0E` (`friendly-matches-design-2.md`, VERIFIED):** Full-ROM scan confirmed **0 vanilla NPCs** use talk indices `0xE0..0xEF`. Recommended a separate compiled archive (`patch/src/scripts/new/friendly_matches.tpl` → `friendly_dialogue_archive`) wired via 5 lines in `lobby_open_dialogue`, with `FC 0E <enemy_id> <soul_idx>` using bit 6 (`0x40`) of `0x020015EF`.
7. **Task 7 — Merged 15 Round 10–11 names & regenerated (`names.txt` + `asm/`, VERIFIED):** Merged all Round 10–11 symbols (with corrected `BattleBoostHandNullBreak30` / `BattleBoostHandWind10`), regenerated all 11 touched segments for RS & BM + `merge_bm.py`, and verified **`build/rs/bn4_rs.gba: OK`** and **`build/bm/bn4_bm.gba: OK`**.

---

## 2. Key Decisions & Constraints

- **Boundaries honored:** Zero edits to `patch/`, `docs/`, or `TODO.md`.
- **Both ROMs byte-identical:** `make -B compare VERSION=rs` and `VERSION=bm` passed after Task 7.
- **Not pushed:** All commits are local only, staged by explicit path.

---

## 3. Modified & Relevant Files

- **Committed (Task 7):** `MMBN4-Decomp/decomp/symbols/names.txt`, `MMBN4-Decomp/decomp/asm/{code_00,code_01,code_02,code_03,code_04,code_04d,code_05,code_06,code_07,code_08,code_09}.s`.
- **Committed (Round 11 notes):**
  - `MMBN4-Decomp/decomp/notes/patch-test-results-4.md`
  - `MMBN4-Decomp/decomp/notes/battle-exit-hook.md`
  - `MMBN4-Decomp/decomp/notes/form-palette-hook.md`
  - `MMBN4-Decomp/decomp/notes/duo-soul-passive.md`
  - `MMBN4-Decomp/decomp/notes/sp-plus-design-2.md`
  - `MMBN4-Decomp/decomp/notes/friendly-matches-design-2.md`
  - `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round11.md`

---

## 4. Open Issues / Blockers

- **5 runtime bugs caught in Task 1** (detailed in `patch-test-results-4.md`):
  1. Chip name/desc `0x00000137` e-Reader bypass (`0x0802F1BC` / `0x08106C28` → `0x00000139`).
  2. Trash Can & Duo's Signal white circles (switch from compressed `list[7][0x44]`/`list[6][0x63]` to uncompressed sprites).
  3. Duo rematch `b1 = 0x13` skipped results screen + `duo_defeat_hook` didn't fire on Duo V1 1-HP win.
  4. DuoSoul `UNITE` consumed `0x16D` without transforming MegaMan (downstream consumer of `[r7, #0x10] == 13` still to trace).
  5. Stale `0x020015EF` on flee (solved by Task 2's `BattleExitCleanup` hook).

---

## 5. Immediate Next Steps

1. Claude applies the 2-word `0x137 → 0x139` chip text fix, BattleExitCleanup (`0x08006E20`), Duo form palette (`0x080107E8`), Duo passive (`0x0800E0D4`), SP+ (`sp-plus-design-2.md`), and Friendly Matches (`FC 0E`), and investigates/fixes Duo rematch results screen + DuoSoul transform trigger.
2. In Round 12, Gemini can trace the DuoSoul `[r7, #0x10]` transform consumer and re-test all fixed packages in `mmbn4.exe`.
