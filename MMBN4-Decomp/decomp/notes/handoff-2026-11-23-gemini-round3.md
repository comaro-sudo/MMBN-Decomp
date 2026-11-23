# Handoff: Gemini Round 3 Read-Only Analysis (2026-11-23)

## Goal & Current Status
Completed all three read-only analysis tasks from `MMBN4-Decomp/decomp/notes/task-gemini-three-tasks-2026-11-23c.md` (without running `make` or touching any existing files or Claude's parallel working-tree edits):

1. **Task 1 — Free Tournament lobby, NPCs & jack-in PC (`MMBN4-Decomp/decomp/notes/lobby-npcs.md`):** **Verified (V, R).**
   - Free Tournament lobby is **`(00, 06)` Higsby's** (`ScriptCmd_callFreeTournament` = `FC 07`, archive RS `0x0876D368` via examine table `0x080523F8` → `0x08052540`).
   - Decoded `MapSpawnNPCs` (`0x0802A3D2`) → `dat_08029C94` (RW) / `dat_08029E14` (CW) → `NpcSpawnList` (`0x08026748`), and the NPC bytecode opcodes (`sub_080A8598` → `dat_080A85D0`: `0x14` pos, `0x16` facing, `0x17` sprite, `0x18` talk script, `0x36` idle loop, `0x3B`/`0x42` phase/flag gates).
   - Confirmed sprite lists: **`list[7]`** (`0x08027E80`, 110 entries) holds human operators; **`list[6]`** (`0x08027CE8`, 102 entries) holds overworld Navis including JunkMan.
   - Higsby's 4-NPC list (`RS 0x08056550` / `BM 0x0805655C`) cannot be extended in place; relocate to free space (`0x087F7420+`) and repoint **RS `0x0805444C` / BM `0x08054458`**.
   - Documented jack-in PC registration (`sub_0802A2E8`, `0x0200B6D0`, flags `0x1150+slot`) and recommended `(88, 00)` Lan's HP (0 NPCs) or `(8E, 00)` / `(8E, 02)` (1 NPC) for JunkMan's spot.
2. **Task 2 — Free Tournament battle data (`MMBN4-Decomp/decomp/notes/free-tournament-battle-data.md`):** **Verified (V, R).**
   - Traced `FreeTournamentPickMatch` (`0x0804498C`) → `sub_08045208` → `sub_08044A8C` → `sub_08044B58` → `sub_08044F80`.
   - Proved that vanilla Free Tournament traded-Navi matches spawn **type `0x04`** (`sub_08007CB0(1, 0x04020501)` + `sub_0800EB98`) and **DO read save slots `+0x78` (stats) and `+0x7C` (AI/folder)** via `sub_0800EBAA`.
   - **Lobby rematches** using `BN4_STORY_BATTLES` (type `0x01` + enemy ID + `+6` bit 7 `0x80` "no tier raise") **do NOT need `+0x78` or `+0x7C` at all**; for the Free Tournament bracket itself, switching `sub_08044F80` case 0 to type `0x01` enemy IDs avoids needing `+0x78`/`+0x7C`.
3. **Task 3 — Status screen Soul list (`MMBN4-Decomp/decomp/notes/status-screen-souls.md`):** **Verified (V, R).**
   - Located UI object `0x45` (`StatusSoulIconTask` `sub_0810D9D4` → `sub_0810D9F8` init + `sub_0810DAD4` update, `code_10`).
   - Confirmed sprite `0x37` already holds all 12 Soul icon animations (`0..11`) in **both** RS and BM, and scratch buffer `0x0201D000` is `0x20` bytes (already big enough for 12).
   - Decoded all 6-entry tables (`dat_0810DA94` VRAM tiles `0x50..0x64`, `dat_0810DAA0`/`dat_0810DAF8` 2×3 grid `(28/84, 42/60/78)`, `dat_0810DAB8` flags, `dat_0810DAC5` anims, `dat_0810DACB` palettes).
   - Because each badge is ~56 px wide, 12 do not fit in the 2×3 box; recommended a **2-page L/R toggle** (keeps 6 UI objects & VRAM slots `0x50..0x64`, zero risk to the 24-object cap or OBJ VRAM) or 2×6 vertical expansion if `Y = 96..148` & VRAM `0x68..0x7C` are free.

## Key Decisions & Constraints
- Strictly read-only with respect to all existing files; all asm read from `git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`.
- No `make`, no ROM bytes or extracted text committed, nothing pushed.
- 16 proposed symbol names in `names.txt` format are at the bottom of the three notes for Claude to merge.

## Modified & Relevant Files
Created and committed (by explicit path only):
- `MMBN4-Decomp/decomp/notes/lobby-npcs.md`
- `MMBN4-Decomp/decomp/notes/free-tournament-battle-data.md`
- `MMBN4-Decomp/decomp/notes/status-screen-souls.md`
- `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round3.md`

Claude's uncommitted files in `decomp/asm/`, `decomp/config/`, and `patch/` were left completely untouched.

## Open Issues / Blockers
None.

## Immediate Next Steps
1. Claude merges the 16 proposed symbol names from `lobby-npcs.md` (6), `free-tournament-battle-data.md` (5), and `status-screen-souls.md` (5) into `decomp/symbols/names.txt`.
2. Use `free-tournament-battle-data.md` to finalize the lobby-rematch design in `docs/pending/free-tournament-secret-chips.md` (confirmed: `BN4_STORY_BATTLES` + `+6` bit 7 needs no `+0x78`/`+0x7C` data).
3. Implement the status-screen 12-Soul patch using `status-screen-souls.md` (2-page toggle recommended) and the lobby NPCs + JunkMan PC using `lobby-npcs.md`.
