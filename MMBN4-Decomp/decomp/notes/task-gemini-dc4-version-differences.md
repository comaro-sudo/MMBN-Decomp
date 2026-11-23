# Task for Gemini: catalogue every RS/BM difference in the code region (DC-4 support)

Assigned by the owner on 2026-11-22, written by Claude. Claude is merging `asm/bm/` into a shared `asm/` tree at the same time (DC-4). This task is designed not to touch anything Claude is editing.

## Why

Claude's new `tools/merge_bm.py` renames Blue Moon labels to their Red Sun twins using **one delta per segment** (`config/segments_*.toml`). Wherever that assumption breaks, the merge still byte-matches, but it falls back to `.if VERSION_BM` / `.else` blocks with misleading label names. Example: `code_04` has a +0xC insertion inside the data block at RS `0x0805AFC6` (`incrom 0x5AFC6, 0x1F96` in RS vs `0x1FA2` in BM). After that point, RS and BM labels are off by 0xC until something compensates.

A dry run gives about 1,050 version blocks, spread over `code_00`–`code_10`. Many are real differences: different constants, and different pointers into the data region. The others are shift drift. A precise map of the true insertion and deletion points lets Claude split segments where the shift actually changes, which removes the drift blocks.

## What to produce

1. **`decomp/notes/dc4-version-differences.md`** (new file). For the code region `0x08000000`–`0x08120000`:
   - **Shift changes:** every RS address where the RS→BM delta changes inside a segment. Give the old delta, the new delta, the inserted or removed byte count, and what's there (code, data table, padding).
   - **Differing sites:** every place where equal-length bytes differ, grouped by kind:
     - an immediate or constant that differs (e.g. `movs r1, #0x18` vs `#0x21`), with the enclosing function's name from `symbols/names.txt` if it has one;
     - a pointer into the data region (`0x081xxxxx`–`0x087xxxxx`), with both values;
     - a pointer into the code region that is the same function, shifted;
     - whole functions or blocks that are different code.
   - A short summary table per segment: counts per kind.
   - A **recommended segment split**: the RS/BM start addresses where each new segment should begin so that each one has one constant delta.
2. Optionally, **`decomp/tools/vdiff.py`** (new file): the script you used, so the result can be regenerated.

## Constraints (important: Claude is working in parallel)

- **Create only the two new files above.** Don't edit or regenerate anything in `decomp/asm/`, `decomp/config/`, `decomp/symbols/`, `decomp/Makefile`, or the existing `decomp/tools/*.py`. Don't edit `TODO.md` either.
- **Don't run `make`.** Don't write to `decomp/build/` or `build/`: a build there would collide with Claude's builds. Read the ROMs directly (`MMBN4-Decomp/roms/bn4_rs_us.gba`, `bn4_bm_us.gba`) with Python (Capstone is in `~/.venvs/mmbn4-decomp`). If you need the old per-version asm text, read it from git at the last commit before the merge (`git show 6e47d43:MMBN4-Decomp/decomp/asm/bm/code_04.s`). `asm/bm/` no longer exists: the shared `asm/<seg>.s` now holds both versions, with `.if VERSION_BM` blocks where they differ.
- Useful inputs, read-only: `symbols/map_rs_bm.txt` (7,772 matched function pairs), `symbols/shift_rs_bm.txt`, `symbols/functions_{rs,bm}.txt`, `config/segments_{rs,bm}.toml`.
- Never commit ROM bytes. Quote at most a few bytes or instructions per site as evidence.
- Commit only your two new files, staged by explicit path, or leave them uncommitted for the owner. Then write a short `decomp/notes/handoff-<date>-<time>.md` as usual.

## Done when

The note lists every shift change and every differing site in `0x08000000`–`0x08120000`, with counts. Its recommended split says, for each segment, where the delta changes.
