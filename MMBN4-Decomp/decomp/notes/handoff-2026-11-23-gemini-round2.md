# Handoff: Gemini Round 2 Read-Only Analysis (2026-11-23)

## Goal & Current Status
Completed all three read-only analysis tasks from `MMBN4-Decomp/decomp/notes/task-gemini-three-tasks-2026-11-23b.md`, without running `make` or modifying any existing files (leaving Claude's parallel edits in `decomp/asm/` and `decomp/config/` completely untouched):

1. **Task 1 — Bass rematch & dual-chip feasibility (`MMBN4-Decomp/decomp/notes/bass-rematch.md`):** **Verified (V, R).**
   - Documented Bass V1 (`BN4_STORY_BATTLES[176]`, flag `0x009F`, post-win `sub_0809C9C4` granting `HubBatc` Pink `0x124`), BassΩ (`(94,07)+0x10`, `sub_08035000` all-empty Mod Cards), and BassXX (`+0x20`, any Mod Card loaded).
   - Confirmed BassΩ and BassXX are already infinitely rematchable (nothing clears `0x009F` or blocks them on a win); only Bass V1 is one-shot.
   - Verified `BN4_REWARD_ROWS` for `0xE4`/`0xE5`/`0xE6` in both RS and BM (`Bass X` 301 / `BassAnly X` 306 for Bass V1; `BlakBarr A` 308 / `DeltaRay Z` 302 for BassΩ/XX at BL 7–S).
   - Mapped the exact reward grant path (`BN4_REWARD_PICK` → `sub_08021684` → `bl sub_0801913E` at RS `0x080216E2` / BM `0x080216E6`) for the recommended Main option ("drop both"), plus Black Earth 2's free MD flags (`0x0E3E`, `0x0E3F`) and Unlocker SubChip ID (`181` = `0xB5`) for Fallbacks 1–2.
2. **Task 2 — Story Soul grants & Blue Moon Soul keys (`MMBN4-Decomp/decomp/notes/soul-grant.md`):** **Verified (V, R).**
   - Solved the Blue Moon Soul key mystery: BM keeps its Soul pseudo-chip keys in the exact same place as RS (`[eToolkit+0x84][353..364]`), XORed with BM's `SAVE_XOR_KEY_1 = 0x47` (vs RS `0x64`); verified against saves `4429`, `4569`, `4891`, `22565`.
   - Located **all 12 story Soul grants** in both ROMs across both script engines (`map_scripts_commands` `0x0802A7D0` ops `0x1C`/`0x3F` for GutsSoul RS / AquaSoul BM; `BN4_CUTSCENE_SCRIPT_CMDS` `0x0802C140` ops `0x26`/`0x66` for the other 10 Souls — all 10 of which are already present in **both** RS and BM!).
   - Documented the exact calls a Violet Eclipse patch needs (`SetEventFlag(0x15)`, `SetEventFlag(0x17 + idx)`, `WriteChipKey(0x161 + idx)`).
3. **Task 3 — Shops in the world & Number Trader (`MMBN4-Decomp/decomp/notes/shops-in-the-world.md`):** **Verified (V, R).**
   - Mapped all 17 shop stock indices (`0..16`) from `startShop nn` (`FB 05 nn`) across all 794 text archives through `MapGetDialogueArchive` (`sub_0804E4B8`) to their exact `(group, map)` and official area names.
   - Disassembled the Number Trader (`sub_0809E0B0` → `sub_0804DFD4`) and decoded all 32 entries of `dat_0804DE3C` (RS) / `dat_0804DE44` (BM) via LUT `dat_0804DFC8` / `dat_0804DFD0` (identical in RS and BM, flags `0x1EC0..0x1EDF`: 12 SubChips, 9 NaviCust programs, 11 Chips).

## Key Decisions & Constraints
- Strictly read-only with respect to all existing files: all asm reads used `git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`.
- No `make` runs, no ROM bytes or extracted text committed, nothing pushed.
- Proposed symbol names are listed in `names.txt` format at the bottom of each note for Claude (or Gemini in a later non-parallel session) to merge into `decomp/symbols/names.txt`.

## Modified & Relevant Files
Created and committed (by explicit path only):
- `MMBN4-Decomp/decomp/notes/bass-rematch.md`
- `MMBN4-Decomp/decomp/notes/soul-grant.md`
- `MMBN4-Decomp/decomp/notes/shops-in-the-world.md`
- `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round2.md`

Uncommitted files belonging to Claude were left completely untouched.

## Open Issues / Blockers
None. All three tasks are complete and verified against both US ROMs and disassembly.

## Immediate Next Steps
1. Claude merges the 14 proposed symbol names from `bass-rematch.md` (4), `soul-grant.md` (4), and `shops-in-the-world.md` (6) into `MMBN4-Decomp/decomp/symbols/names.txt` when ready.
2. For Violet Eclipse patches:
   - **Bass dual-drop:** hook `bl sub_0801913E` at RS `0x080216E2` / BM `0x080216E6` (Main option), or add Black Earth 2 MD(s) on free flags `0x0E3E` / `0x0E3F` (Fallbacks 1–2).
   - **12 Souls grant:** call `SetEventFlag(0x15)`, `SetEventFlag(0x17 + idx)`, `WriteChipKey(0x161 + idx)`, plus update `BN4_STAR_SOUL_FLAGS`, `BN4_STATUS_SOUL_FLAGS` (`0x08034A78`), and `checkDoubleSouls` (`sub_0804F494`).
   - **Shops / Number Trader:** use `shops-in-the-world.md` to finish `docs/completion/mystery-data-shops-traders.md` and WP-A3 shop price patches.
