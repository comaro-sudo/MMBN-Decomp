# Handoff: Claude's DC-4 session, 2026-11-22 night (Claude → Gemini)

## Goal & Current Status

- **Goal:** the owner said "continue decompiling". Claude took **DC-4**: build Blue Moon byte-identical from the same tree as Red Sun.
- **Done and verified:** `asm/bm/` is gone. Both ROMs assemble from the shared `asm/<seg>.s`:
  - `make -B compare VERSION=rs` → `build/rs/bn4_rs.gba: OK`
  - `make -B compare VERSION=bm` → `build/bm/bn4_bm.gba: OK`
  - Coverage is unchanged: RS 598,984 / 1,179,456 (50.8%), BM 599,008 / 1,179,476 (50.8%).
  - Round trip on `code_05` (`disasm.py rs` → `disasm.py bm` → `merge_bm.py`) gives a byte-identical `asm/code_05.s`.
- **How it works:**
  - Every symbol in the shared source is named after its **RS address**.
  - In a BM build, `split.py` places each RS-named symbol at its BM address, using the per-segment delta from `config/segments_*.toml`.
  - The few BM-only bytes at the end of `code_01`, `code_02`, `code_03` and `code_09` get a `_bm` suffix (e.g. `sub_08020D6C_bm`).
  - Where the versions differ, `tools/merge_bm.py` writes `.if VERSION_BM` / `.else` / `.endif` blocks (`.if VERSION_RS` for RS-only lines).

| Segment | Version blocks | Segment | Version blocks |
|---|---|---|---|
| code_00 | 30 | code_06 | 31 |
| code_01 | 67 | code_07 | 44 |
| code_02 | 160 | code_08 | 29 |
| code_03 | 320 | code_09 | 139 |
| code_04 | 135 | code_10 | 86 |
| code_05 | 3 | crt0, intr_table | 0 |

## Key Decisions & Constraints

- **The renaming is one-to-one, and a line is shared only if both versions agree on it**, so a shared line can't point at the wrong twin. The build compare is the final check.
- **Workflow change for regenerating a segment** (also in `decomp/README.md`): `disasm.py rs X`, then `disasm.py bm X`, then `merge_bm.py X`, then both compares. Always regenerate RS first. `merge_bm.py` refuses to run on a file that's already shared.
- Until the merge runs, `split.py` assembles a fresh `asm/bm/X.s` through a renamed copy in `build/bm/norm/asm/bm/X.s`, so `disasm.py bm`'s self-check and its raw-instruction retry still work. Its error regex accepts that prefix.
- **Three kinds of version blocks:**
  1. Real differences: constants (e.g. `movs r1, #0x18` vs `#0x21` in `code_00`) and pointers into the data region (`.word 0x0869475C` vs `0x08694544`).
  2. Raw code pointers that were never symbolized (`.word 0x080A5C65` vs `0x080A5C71`), equal after the shift.
  3. **Shift drift:** after a mid-segment insertion (e.g. `code_04`'s data block at RS `0x0805AFC6`, 0xC bytes longer in BM), labels are named with the segment delta, not the true twin. These names are correct for the build but not meaningful.
- **Owner's assignment for Gemini:** [task-gemini-dc4-version-differences.md](task-gemini-dc4-version-differences.md). It maps the true shift points and the differing sites, so segments can be split where the delta changes and kinds 2 and 3 disappear.

## Modified & Relevant Files

- `tools/merge_bm.py` (new): the merge, plus the helpers `bm_to_rs`, `bm_value`, `normalize_file` and `active_lines`.
- `tools/split.py`: shared-source includes; BM externs named after RS addresses and placed at BM addresses; on-the-fly renaming of not-yet-merged BM views.
- `tools/coverage.py`: reads the shared file and counts only the lines active for the chosen version.
- `tools/disasm.py`: error regex and comment.
- `Makefile`: comment and dependency on `merge_bm.py`.
- `README.md`: DC-4 status and the regeneration workflow.
- `asm/*.s`: now shared. `asm/bm/*.s`: deleted.
- `notes/task-gemini-dc4-version-differences.md` (new): the Gemini task.

## Open Issues / Blockers

- No build issues.
- `TODO.md` (Gemini's) still describes `asm/bm/` and lists DC-4 as open. Gemini should mark DC-4's "one tree" item done and keep "reduce version blocks" open.
- Older notes and handoffs mention `asm/bm/`. They're historical and were left as they are.
- Downstream tools that look up symbols by name in the BM ELF see RS-address names (`sub_<RS addr>`), at correct BM values. Canonical names from `names.txt` (`symbols.inc`) are unaffected.

## Immediate Next Steps

1. (Gemini) Do the task in `task-gemini-dc4-version-differences.md`.
2. (Either) Split segments at the shift points it finds: add segments to both `segments_*.toml`, regenerate RS + BM, run `merge_bm.py`, then both compares. That should remove most kind-3 blocks.
3. (Either) Symbolize raw code-region `.word` pointers in `disasm.py`, so kind-2 blocks disappear.
4. Then DC-3/DC-5: names for the RE targets, and the first C infrastructure.
