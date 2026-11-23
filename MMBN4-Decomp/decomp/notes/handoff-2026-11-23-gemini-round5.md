# Gemini handoff — Round 5 (2026-11-23)

## Goal & Current Status

Completed all three read-only / runtime-test tasks from `MMBN4-Decomp/decomp/notes/task-gemini-three-tasks-2026-11-23e.md`:

1. **Task 1 — Waiting room (Option A) & outside trash can** → `MMBN4-Decomp/decomp/notes/waiting-room.md` (**verified** statically + runtime `mmbn4.exe` captures of `(02,02)`, `(04,03)`, `(03,02)`, `(00,00)`).
   - BN4 has **34 real-world maps** across 6 groups (`0x00`–`0x05`) and **zero unused/hidden maps**.
   - BN4's hard per-map NPC limit is **16** (`sub_08004224`, `0x02006140..0x02006EC0`, stride `0xD8`).
   - **Best Option A candidate: `(02, 02)` DomeRoom** — bright tournament waiting room with a bracket poster on the wall, 2 doors, ample open floor space, and only **2 existing NPCs** (14 free slots → fits all 11 operators with 3 slots to spare). `(04, 03)` ColosseumRm is an outdoor stone staircase and `(03, 02)` WaitingRoom is an ornate castle hall with 5 NPCs (0 spare slots).
   - Documented the exact 16-byte warp descriptor format, Group `00`/`02` map-loader hook addresses (`RS 0x08054284 / BM 0x08054290`, `RS 0x08059234 / BM 0x08059240`), step-on-door trigger (`sub_0800524C`), and confirmed only 2 warps to `(00, 06)` exist in the ROM (ACDC street door `0x080541EC` + Free Tournament return `0x080438B4`), so flag-gating Warp 5 leaves all story cutscenes untouched.
   - **Outside trash can:** ACDC Town `(00, 00)` (13/16 NPCs → 3 free slots), verified sidewalk spot **`(-150, -90, 0)`** (front-right corner of Higsby's under the awning, clear of door `(-176, -90)` and front-left NPC boy). Existing trash cans in `(00, 04)` and `(01, 00)` are BG tiles, not sprites.
2. **Task 2 — Live runtime patch tests** → `MMBN4-Decomp/decomp/notes/patch-test-results.md` (**all 4 PASS** on freshly built `bn4_rs_overhaul.gba` / `bn4_bm_overhaul.gba` via `mmbn4.exe`):
   - **WP-D4:** PASS on both RS and BM (6 Souls → no `SOUL COMPL`; 12 Souls → `SOUL COMPL` shown).
   - **WP-A2:** PASS (`50 Z` → `75 Z` shown on results screen; EWRAM Zenny `1660 → 1710 (+50)` vanilla vs `1660 → 1735 (+75)` patched).
   - **WP-D3:** PASS for both Bass V1 (`BassAnly X` + `Bass X`) and BassΩ (`DeltaRay Z` + `BlakBarr A`) — both Library flags `False → True` and both pack counts `0 → 1`.
   - **WP-D2:** PASS over 30 random RNG seeds on Billy (`0x0D`): vanilla BM dropped `6 Thunder1 P / 0 Thunder1 L`; patched BM dropped `4 Thunder1 P / 4 Thunder1 L`.
3. **Task 3 — Status screen two-page Soul view design** → `MMBN4-Decomp/decomp/notes/status-screen-two-pages.md` (**verified** in `code_02.s` & `code_10.s`, RS & BM addresses confirmed):
   - **`L` (`0x0200`), `R` (`0x0100`), and `SELECT` (`0x0004`) are all free** on the Status screen.
   - Cleanest hook point: **`sub_08034EB0`** (RS `0x08034EB0` / BM `0x08034EB4`), an **empty no-op stub** called every idle frame from `StatusScreenState0`.
   - Store page offset (`0` or `6`) at **`0x0201D018`** (unused byte inside the existing 32-byte `0x0201D000..0x0201D01F` buffer), refresh all 6 Soul icons on the same frame by resetting `[obj + 8] = 0` for all `type == 0x45` entries in the 24-slot UI pool at **`0x02008BB0`** (stride `0x58`).
   - Total size estimate: **~176 B** (40 B 12-Soul tables + ~136 B code); noted Thumb `bl` > 4 MiB range constraint to free space.

## Key Decisions & Constraints

- Did **not** touch `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, `decomp/tools/*.py`, `docs/`, `patch/`, or `TODO.md`; did **not** run `make`.
- All runtime screenshots, save copies, save states, and EWRAM dumps were written to the external scratch directory (`C:\Users\test\.gemini\antigravity\brain\7e09fd7a-7f26-455f-919d-1f878852d7c9\scratch\`), never inside the repo.
- Claude's uncommitted changes in `decomp/asm/` and `decomp/symbols/names.txt` were left completely untouched.

## Modified & Relevant Files

- New (staged & committed by Gemini, explicit paths only, not pushed):
  - `MMBN4-Decomp/decomp/notes/waiting-room.md`
  - `MMBN4-Decomp/decomp/notes/patch-test-results.md`
  - `MMBN4-Decomp/decomp/notes/status-screen-two-pages.md`
  - `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round5.md`

## Open Issues / Blockers

- None. All 4 patch packages passed runtime verification, and both the waiting-room/trash-can plan and the two-page Status screen design are ready for Claude to implement.

## Immediate Next Steps

1. Claude can implement the two-page Status screen patch directly from `decomp/notes/status-screen-two-pages.md` (noting the Thumb `bl` range to free space vs. dead blocks in `code_02`/`code_03`).
2. Claude can implement Option A waiting room (`(02, 02)` DomeRoom) and the outside ACDC Town `(00, 00)` trash-can NPC at `(-150, -90, 0)` from `decomp/notes/waiting-room.md`.
