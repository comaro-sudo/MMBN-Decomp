# Task for Gemini: RE-07, decode the Mystery Data tables (blue, purple, green)

Assigned by the owner on 2026-11-22, written by Claude. Your data-region map (`datashift_rs_bm.txt`, 99.47% validated) was excellent; Claude is now using it to symbolize data-region pointers in `tools/disasm.py` / `merge_bm.py`, and is regenerating `asm/`. This task is read-only analysis and doesn't touch any of that. Claude has **stopped** working on RE-07, so it's yours alone.

## Why

Violet Eclipse must make every chip obtainable in one save (criterion C2), and Mystery Data is one of the main sources. Today our list comes from fan maps (`docs/completion/mystery-data-shops-traders.md`: 64 blue and 14 purple on the maps, one playthrough, probably Red Sun). The ROM's own tables would give us:

- every placement with its contents, for Normal, Hard and Super Hard;
- the green (random, respawning) Mystery Data tables per area;
- the RS/BM differences.

## Leads Claude found (verify them; they're not proven)

- **Flags:** blue/purple Mystery Data "taken" flags are in `0x15C0`–`0x163F`. `SetEventFlagLogged15C0` (RS `0x0802516E`) sets flag `r0` and `r0 + 0x80`, and appends `r0 - 0x15C0` to a log at `0x02006EC0`. New Game+ resets this range (`docs/re/completion-and-new-game-plus.md` §3).
- **Not the pickup:** `sub_08085EC0` (code_05) is a story hook that converts 12 story flags (`0x75`…) into Mystery Data flags (`0x15D0`…, clearing `0x1650`…). `sub_0802E0E4` (code_02) is a debug-menu action. Its neighbors start tournaments and open brackets.
- **BN6's format doesn't match directly.** `ext/bn6f/data/dat28.s` uses 12-byte entries `{u8 type (1 blue, 3 purple, 5 green), pad, u16 flag, ptr placements, ptr contents}` under map-group tables, read by `sub_809FB04`/`sub_809FB4C` in `ext/bn6f/asm/asm28_1.s`. A scan of the BN4 RS ROM for that exact layout (types 1–6, flags `0x1000`–`0x1FFF`, two ROM pointers) found nothing. BN4's layout is probably an ancestor of it: look for the same *logic* (count placements until a 0 type byte, 8-byte placement and content records, pick an index with RNG for green).
- **CE's note:** `ext/Mega-Man-Battle-Network-Cyberworld-Endless/docs/engine/emulation/layer.md` line ~119 mentions a check at `0x080A6144` whose only use is Mystery Data's, "group `0x96` map". That's probably the best entry point.
- Contents probably reuse the shop entry shape (`docs/re/shops.md`: 8 bytes, type 1 item / 2 chip / 3 NaviCust program, u16 id, code, amount). Zenny and BugFrag rewards will be a separate type.

## What to produce

1. **`decomp/notes/re07-mystery-data.md`** (new):
   - the routines (RS and BM addresses) for spawning Mystery Data on map load, picking it up, and choosing green contents;
   - the table layout(s), with RS and BM addresses;
   - **every blue and purple placement**: map (group/number), flag, contents, mode (Normal/Hard/Super Hard if it varies);
   - the green tables per area and tier, with their odds;
   - the RS/BM differences;
   - a diff against `ref/maps/legend.csv` (local, gitignored; read it but don't copy it into the repo), with every mismatch listed.
2. **`decomp/tools/mysterydata.py`** (new), the re-runnable extractor. It reads only the two ROMs and existing files.
3. Proposed names in the `names.txt` format, as a section at the end of your note. Claude or you merges them later. Don't edit `symbols/names.txt` now.

## Constraints (Claude is working in parallel)

- **Create only the two new files above** plus your handoff. Don't edit or regenerate anything in `decomp/asm/`, `decomp/config/`, the existing `decomp/symbols/*`, `decomp/Makefile`, `decomp/tools/disasm.py`, `split.py` or `merge_bm.py`, `TODO.md`, or anything in `docs/`.
- **Don't run `make`** and don't write to `decomp/build/` or `build/`. To read asm, use the committed copy (`git show HEAD:MMBN4-Decomp/decomp/asm/code_05.s`). The working tree's `asm/` is being rewritten, and its segment files change (`code_04` is now `code_04`, `04b`, `04c` and `04d`).
- Never commit ROM bytes or the guide data. Quote at most a few bytes per site as evidence. Placement tables in the note use ids and names, not raw dumps.
- Leave the files uncommitted, or commit only your own new files by explicit path. Then write `decomp/notes/handoff-<date>-<time>.md`.

## Done when

Every blue and purple Mystery Data placement is listed from the ROM with its contents and flag, the green tables are decoded, and the diff against the map list is done, with every mismatch explained.
