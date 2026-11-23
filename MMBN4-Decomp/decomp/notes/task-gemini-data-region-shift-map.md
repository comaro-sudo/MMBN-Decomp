# Task for Gemini: map the RS→BM shift of the data region (DC-4 support, part 2)

Assigned by the owner on 2026-11-22, written by Claude. It follows your `dc4-version-differences.md`, which was excellent and is being used now. Claude is working in parallel: splitting segments in `config/` per your §3, regenerating `asm/`, and symbolizing code-region literals in `tools/disasm.py`. This task doesn't touch any of that.

## Why

Your §4.5 found **5,802 pointers into the data region** (`0x08120000`–`0x087FFFFF`): 644 in literal pools and 5,158 inside `incrom` tables. They differ between RS and BM because the data region shifts by varying amounts (e.g. `-0x218`, `-0x570`, `-0x50C`, `+0x78`, ...). `symbols/shift_rs_bm.txt` only probes the code region coarsely and is `None` for most ranges.

With an exact RS→BM map of the data region, Claude can turn every data-region pointer into a symbol (`dat_<RS addr>`, placed at the BM address in a BM build), the way code pointers already work. Most of the remaining `.if VERSION_BM` blocks would then disappear, and later work (asset extraction, the PC port) gets a map of where each asset lives in both versions.

## What to produce

1. **`decomp/symbols/datashift_rs_bm.txt`** (new file), machine-readable, one range per line, sorted, covering `0x08120000`–`0x08800000` (RS) with no gaps:

   ```
   # rs_start rs_end bm_start kind note        (tools/datashift.py)
   08120000 08126500 08120014 same   m4a samples
   08126500 08126600 -        rs_only <what it is>
   ...
   ```

   - `kind` is one of: `same` (bytes identical at bm_start), `differs` (same length, same role, different bytes, e.g. version graphics), `rs_only`, `bm_only` (for BM-only ranges use `-` for the RS columns and add the BM range in the note).
   - A range boundary wherever the delta changes. Prefer asset boundaries: LZ77 (`0x10` header) blocks, pointer-table targets, palettes, text archives, m4a song headers.
2. **`decomp/notes/data-region-shift-map.md`** (new file): the method, per-delta summary (how many bytes at each delta), the largest version-specific assets and what they are (where you can tell, e.g. title screen, version mascot, Navi mugshots), and **validation**: for every one of the 5,802 pointer pairs in your §4.5, check that `BM == map(RS)`. Report how many pass and list every failure.
3. **`decomp/tools/datashift.py`** (new file): the script that builds and validates the map, re-runnable. It should read only the two ROMs plus existing files read-only.

## Method hints

- Anchor on the pointer pairs you already have: every pair (RS target, BM target) is a known correspondence point. Between anchors, extend matches byte-by-byte (or with a rolling hash) to find where identical runs start and end.
- The bit-31 flag (`0x88xxxxxx`) pointers are compressed assets: mask bit 31, then the target starts with an LZ77 header (`0x10` + 24-bit size), which gives exact asset lengths.
- Large identical runs can be found quickly with a hash of 64-byte blocks of BM, then lookups for RS blocks.

## Constraints (important: Claude is working in parallel)

- **Create only the four new files above** (plus your handoff). Don't edit or regenerate anything in `decomp/asm/`, `decomp/config/`, the existing `decomp/symbols/*` files, `decomp/Makefile`, or the existing `decomp/tools/*.py` (your `vdiff.py` is yours to change). Don't edit `TODO.md`.
- **Don't run `make`** and don't write to `decomp/build/` or `build/`. Claude's builds use them.
- Never commit ROM bytes. The map holds only addresses, lengths and short descriptions; quote at most a few bytes as evidence.
- Leave the files uncommitted, or commit only your own new files by explicit path. Then write `decomp/notes/handoff-<date>-<time>.md`.

## Done when

`datashift_rs_bm.txt` covers the whole RS data region with no gaps, and the validation passes for at least 99% of the 5,802 pointer pairs, with every failure listed and explained.
