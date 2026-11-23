# Gemini handoff — Round 6 (2026-11-23)

## Goal & Current Status

Completed all three read-only analysis tasks from `MMBN4-Decomp/decomp/notes/task-gemini-three-tasks-2026-11-23f.md`:

1. **Task 1 — NPC & Special-Chip BMD placements** → `MMBN4-Decomp/decomp/notes/placements.md` (**verified** in ROM tables + `mmbn4.exe` captures outside the repo):
   - **Higsby's `(00, 06)` (9 operators → 13/16 NPC slots):** 9 walkable `(x, y, z)` + facing spots along the right shelves (`X = 36`, since `X = 50` enters the shelf), beside the `WELCOME` rug (`Y = 112`), by the blue Chip Trader (`(-140, 44)` / `(-134, 72)`), and open center-left (`(-56, 56)`), keeping door/counter/terminal paths clear.
   - **ACDC Town `(00, 00)` (2 operators + trash can → 16/16 NPC slots):** trash can at `(-150, -90, 0)` (or `(-156, -84, 0)` on full pavement), outside operators at **`(-134, -76, 0)`** (verified in `p_acdc_op.bmp`) and **`(-164, -56, 0)`**, clear of door `(-176, -90)`.
   - **Grand Prix Power BMD:** **ACDC Area 2 `(90, 01)`** at **`(106, 326, 0)`** (quiet circular dead-end pad on the 5-finger peninsula, zero overlap with PMD/GMDs), free flag **`0x0D0B`**.
   - **Z-Saver BMD:** **Undernet 1 `(94, 00)`** at **`(-282, 56, 0)`** (verified walkable hidden red dead-end tile under the yellow bridge, >140 units from all existing Mystery Data), free flag **`0x0E04`**.
2. **Task 2 — WP-B1 deterministic tournament design** → `MMBN4-Decomp/decomp/notes/wp-b1-design.md` (**verified** in `code_02.s` & `code_03.s` on both US ROMs):
   - Exact patch points for `BN4_TOURNAMENT_SOUL_PAIRS` (`{03,00}, {04,06}, {0A,09}`), `TournamentPickSoulPair` (`RS 0x08043FE4 / BM 0x08043FEC`), `TournamentCheckSoulPairSwap` (`RS 0x0804480C / BM 0x08044814`), `BN4_DRAW_BRACKET` (`RS 0x080442EC / BM 0x080442F4`), and `TournamentFillEntrants`.
   - Exact cutscene opcodes for friendly matches (`0x51` `CutsceneCmd_StoryBattle`, `0x52` `StartTournament`, `0x53` `AdvanceRound`, `0x26`/`0x66` Soul grant) and per-ROM version notes (RS starter tutorial `0x08059EB6` → `AquaSoul` `0x21`/`0x16B`).
3. **Task 3 — Soul grant & `checkDoubleSouls` audit** → `MMBN4-Decomp/decomp/notes/soul-grant-patch-design.md` (**verified** in `code_00`–`code_10` & ROM text):
   - Existing script opcodes (`26`/`66` cutscene, `1C`/`3F` map) grant any Soul in **11 bytes** with zero new code; also designed a 28-byte Thumb `GrantSoulByIndex(idx)` helper.
   - `ScriptCmd_checkDoubleSouls` (**RS `0x0804F494` / BM `0x0804F4A0`, note BM delta `+0x0C`!**) is opcode `EE 2B` and is called by **only 1 script** (`RS 0x08777AE8`, archive `0x08777ACC`: *"Show the union of 6 souls."* Black Earth gate). Patch: RS `0x0804F498` `06 21 → 0C 21`; BM `0x0804F4A2` `1D 20 06 21 → 17 20 0C 21`.
   - Confirmed no other hardcoded 6-Soul readers exist and no other menu besides Status lists Souls (Custom Screen `BN4_UNITE_CHECK` already loops 12).

## Key Decisions & Constraints

- Strictly read-only outside our 4 new files; did not touch `decomp/asm/`, `decomp/symbols/*`, `docs/`, `patch/`, or `TODO.md`; did not run `make`; all screenshots/saves stayed in the external scratch directory.
- Claude's uncommitted changes in `decomp/asm/code_00.s`, `code_02.s`, `code_03.s`, and `decomp/symbols/names.txt` were left untouched.

## Modified & Relevant Files

- New (staged & committed by Gemini, explicit paths only, not pushed):
  - `MMBN4-Decomp/decomp/notes/placements.md`
  - `MMBN4-Decomp/decomp/notes/wp-b1-design.md`
  - `MMBN4-Decomp/decomp/notes/soul-grant-patch-design.md`
  - `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round6.md`

## Open Issues / Blockers

- None. All three designs are ready for Claude's steps 3, 4, and 7.

## Immediate Next Steps

1. Claude implements step 3 (`soul-grant-patch-design.md`: `ScriptCmd_checkDoubleSouls` + `GrantSoulByIndex` if needed).
2. Claude implements step 4 (`wp-b1-design.md`: deterministic bracket + friendly matches).
3. Claude implements step 7 (`placements.md`: 9 Higsby's operators, 2 ACDC operators + trash can, 2 special-chip BMDs).
