# Gemini handoff — Round 7 (2026-11-23)

## Goal & Current Status

Completed all three tasks from `MMBN4-Decomp/decomp/notes/task-gemini-three-tasks-2026-11-23g.md`:

1. **Task 1 — Live runtime tests of 3 newest packages** → `MMBN4-Decomp/decomp/notes/patch-test-results-2.md` (**2 real bugs / blockers caught!**):
   - **`wp_d4_status_two_pages.asm`: PARTIAL FAIL (Bug caught!)** — L/R page switching (`0x0201D018` `0 ↔ 6` on 11-Soul `22565`, no-op on 6-Soul `4752`), `NAVIGATOR`, panels, and closing all work, **BUT 3 of 6 Soul icons are invisible on both pages** because `StatusSoulLoadFlag` at `ICON_FREE` (`0x0810DAB8`) **overwrote `dat_0810DABF`** (`00 10 20 30 40 50`, OAM slot offsets still referenced by `_0810DA7C` RS / `_0810DA90` BM). Fix: copy `00 10 20 30 40 50` to `FS_STATUS_SOULS + 0x30` and repoint `ICON_OAM_PTR`.
   - **`wp_d2_duo_chip_reward.asm`: BLOCKED (2 issues caught!)** — (1) `BN4_STORY_BATTLES[93]` (Duo) and `[94]` (DuoΩ) have **`+1 = 0x93` (bit 7 `0x80` = skip results screen)**, going straight to ending/credits without rolling rewards; (2) even when forced through the results screen (`0x0737` rolled cleanly), **`Duo` (`0x137`, #292) and `Grand Prix Power` (`0x138`, #291) are Japan-only e-Reader RAM-loaded chips** whose `BN4_CHIP_TABLE` pointers point to empty EWRAM (`0x020008A0..0x020013E0`, `flags = 0x07`), displaying blank `"        D"` with a black art box and not registering in the Library!
   - **`wp_d0_npc_lists.asm`: PASS (RS & BM)** — Higsby's `(00, 06)` and ACDC Town `(00, 00)` spawn all original NPCs in their exact spots and talk normally in both versions.
2. **Task 2 — Special-Chip Blue Mystery Data patch design** → `MMBN4-Decomp/decomp/notes/special-chip-md-design.md`:
   - Exact RS & BM addresses for relocating ACDC Area 2 `(90, 01)` (`0x080AA434 / 0x080AA440`, CW NPCs `0x0807748C / 0x08077498`) and Undernet 1 `(94, 00)` (`0x080AC6D0 / 0x080AC6DC`, CW NPCs `0x08082A00 / 0x08082A0C`), exact 12-byte MD records, 8-byte placement/content bytes (all slots identical for NG+), exact 11-byte `08 25 08 1C 29 <fl> 45 <fl> 03` spawn scripts, and confirmation that generic pickup script `0` in `0x087585A0` handles chips automatically. Flagged the e-Reader EWRAM-backed warning for Grand Prix Power (`0x138`) vs ROM-backed Z-Saver (`0xDA`).
3. **Task 3 — Lobby NPCs, Trash Can & JunkMan design** → `MMBN4-Decomp/decomp/notes/lobby-npc-scripts-design.md`:
   - `list[7]` operator sprites (verified Mayl `0x01`, Dex `0x02`, Higsby `0x08` + Higsby counter-dialogue note; JunkMan `list[6] 0x5A`), NPC bytecode with "met" flag gate (`0x0001 + tournament_id`), 24-bit tier counter in verified free flag block **`0x0E40`–`0x0E57`**, single custom `ScriptGroup_FC` rematch command (~90 B) calling `BN4_START_BATTLE` (no-tier-raise bit) + checking S-rank + calling `FreeTournamentSelectPrize` (`0x08045458`), minimal text/relocation plan, and trash-can jack-in warp into `(8E, 00)` / `(88, 00)`.

## Key Decisions & Constraints

- Strictly read-only outside our 4 new files; did not edit `decomp/asm/`, `decomp/symbols/*`, `docs/`, `patch/`, or `TODO.md`; did not run `make`; all screenshots/saves/dumps stayed in the external scratch directory.

## Modified & Relevant Files

- New (staged & committed by Gemini, explicit paths only, not pushed):
  - `MMBN4-Decomp/decomp/notes/patch-test-results-2.md`
  - `MMBN4-Decomp/decomp/notes/special-chip-md-design.md`
  - `MMBN4-Decomp/decomp/notes/lobby-npc-scripts-design.md`
  - `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round7.md`

## Open Issues / Blockers

- **`wp_d4_status_two_pages.asm`:** Needs 6-byte `dat_0810DABF` (`00 10 20 30 40 50`) moved to `FS_STATUS_SOULS + 0x30` + `_0810DA7C`/`_0810DA90` repointed so all 6 icons render.
- **`Duo` (`0x137`) & `Grand Prix Power` (`0x138`):** Both are e-Reader EWRAM-backed chips in US ROMs (blank name/art, `flags = 0x07`), and Duo (`93`/`94`) skips the results screen (`+1 = 0x93` bit 7). Claude/owner will need to decide how to populate their chip data/art and trigger Duo's grant.

## Immediate Next Steps

1. Claude fixes `ICON_OAM_PTR` in `wp_d4_status_two_pages.asm` and re-verifies 6 icons on Page 1 & 5 icons on Page 2 (`22565.sav`).
2. Claude decides on e-Reader chip data/art for `Duo` (`0x137`) and `Grand Prix Power` (`0x138`), and implements `Z-Saver` (`0xDA`) BMD from `special-chip-md-design.md`.
3. Claude implements the lobby rematch NPCs, trash can, and JunkMan from `lobby-npc-scripts-design.md`.
