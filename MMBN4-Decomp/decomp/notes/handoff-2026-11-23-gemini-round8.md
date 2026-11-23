# Handoff — Gemini Round 8 Research (2026-11-23)

## Goal & Current Status
Completed all three research-only tasks from `decomp/notes/task-gemini-three-tasks-2026-11-23h.md` (zero edits to `decomp/asm/`, `symbols/`, `patch/`, `docs/`, or `TODO.md`; Claude's uncommitted work left untouched):

1. **Task 1 — `decomp/notes/lobby-fixes.md`:**
   - Verified all 7 remaining operator sprites in `list[6]` (`RS 0x08027CE8 / BM 0x08027CEC`) by rendering front-facing contact sheets and checking RS/BM byte identity: **Mr. Match `0x1D`, Shuko `0x20`, Lily `0x34`, Tamako `0x1F`, Sal `0x2C`, Raika `0x3B`, Raoul `0x32`**.
   - Verified **Trash Can / Junk Pile** prop: **`list[7]` `0x44`** (Junk/Trash Pile; alt `0x5A` grey bin) using NPC opcode **`.byte 0x25, 0x44, 0x1C`** (`sub_080A8980`, which sets `[r5, #0x78] = 0x1C = 7*4`).
   - Proved flags `0x0001`–`0x000C` are **only** set for traded other-version Navis drawn into slot `+2` of `TournamentFillEntrants` (`sub_08044058`), never for native Soul Navis. Provided drop-in replacement using the **12 Double Soul flags (`0x0017`–`0x0022`)**.
   - Audited `WtrGodComp1 (8E, 00)` (`init 0x08075FA8`, `cont 0x08075FB1`, `NPC[0] 0x08076520`): **zero clash** with AquaMan's side story (which lives in `WtrGodComp2 (8E, 01)`).
2. **Task 2 — `decomp/notes/sp-plus-research.md`:**
   - **Verdict: FEASIBLE AS SPECIFIED.** All three modifiers live in `code_00`/`code_01` at **identical RS & BM addresses**, gated on a 1-byte EWRAM flag (`0x020015EE`):
     - **HP = 4,095:** `sub_080067B4` (`0x08006802`), override `r2 = 0x0FFF` when `enemy_id >= 0x80`.
     - **Attack × 1.5:** `sub_08013A48` (`0x08013A56`), scale `[r6, #0x28]` by `dmg + (dmg >> 1)` right before `sub_080133CE` (Barrier/Aura/Undershirt all work naturally).
     - **Speed × 1.25:** `BN4_OBJECT_RUNNER` (`sub_0800314C`, `0x08003196`), run a 2nd tick every 4th frame (`5/4 = 1.25×`) on live `side == 1` enemy objects (`kind == 1`, optionally `kind == 3`).
3. **Task 3 — `decomp/notes/duo-gpp-research.md`:**
   - **Critical finding:** Neither `bn4_rs_jp.gba` nor `bn4_bm_jp.gba` contains art for `0x137` (Duo) or `0x138` (GP Power) — all four ROMs point to the EWRAM event-download slots (`0x020008A0..0x0200145F`, `0x5E0` B uncompressed each). Original pixel art (or a placeholder ROM chip) is required instead of copying from JP ROMs.
   - Documented **Chip Names Bank 1** (`RS 0x08748C14 / BM 0x08748708`, `0x4CC` B uncompressed, all 8 ROM pointer addresses per version); `"Duo"` (`0E 3A 34 E5`) fits in-place over `FF 00 00 E5`, and `"GP Power"` fits by relocating Bank 1 to free space.
   - Documented two exact post-Duo grant hooks for `ChipGive(0x137, 3, 1)`: **`sub_080C4780` (`RS 0x080C4792 / BM 0x080C479E`, recommended)** right when Duo-defeated flag `0x006F` is set, and `bl IncPlaythrough` (`0x08000E86`, RS == BM).

## Modified & Relevant Files
- `MMBN4-Decomp/decomp/notes/lobby-fixes.md` (new)
- `MMBN4-Decomp/decomp/notes/sp-plus-research.md` (new)
- `MMBN4-Decomp/decomp/notes/duo-gpp-research.md` (new)
- `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round8.md` (new)

## Immediate Next Steps (for Claude)
1. Apply the drop-in sprite + Soul-flag table edit from `lobby-fixes.md` into `wp_d3_lobby_rematches.asm`.
2. Use `sp-plus-research.md` to implement the SP+ battle hook (`0x08006802`, `0x08013A56`, `0x08003196`).
3. Decide with the owner on art source for Duo (`0x137`) & GP Power (`0x138`) now that JP ROMs are confirmed to lack their art; relocate Chip Names Bank 1 and hook `RS 0x080C4792 / BM 0x080C479E` for Duo D.
