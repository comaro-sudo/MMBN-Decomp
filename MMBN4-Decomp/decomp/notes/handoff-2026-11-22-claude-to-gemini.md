# Handoff: Claude → Gemini, 2026-11-22 night (Claude at its usage limit)

Read this first, then `AGENTS.md` (repo root) and `decomp/README.md`. All paths below are inside `MMBN4-Decomp/` unless they start with `/`.

## Goal & Current Status

- **The repo is now a collection** (`C:\proyectos\MMBN-Decomp`, GitHub `comaro-sudo/MMBN-Decomp`): one folder per game. All BN4 work is in `MMBN4-Decomp/`. Run every command from there. See `AGENTS.md` §0 for the layout and §1 for ownership: **you still own `decomp/` and `TODO.md`**.
- **DC-4 is met: both ROMs build from one shared tree.** `asm/bm/` is gone, symbols are named after RS addresses, and version differences are `.if VERSION_BM` / `.else` / `.endif` blocks. **Verified** at commit `1fd88fd`:
  - `make -B compare` → `build/rs/bn4_rs.gba: OK` and `build/bm/bn4_bm.gba: OK`;
  - coverage RS 598,984 / 1,179,456 and BM 599,008 / 1,179,476 (50.8%).
- **Version blocks: about 1,050 → 92**, thanks to your two maps:
  - segments split at the true shift points: `code_04` is now `code_04`, `code_04b`, `code_04c`, `code_04d`, and the `code_09`/`code_10` boundary is RS `0x08109EF0` / BM `0x08109F04`;
  - code- and data-region words are written as `dat_` symbols, and BM data addresses map through your `symbols/datashift_rs_bm.txt`.
  - Per segment: `code_00` 18, `01` 10, `02` 7, `03` 19, `04`/`b`/`c`/`d` 3/1/1/0, `05` 2, `06` 0, `07` 3, `08` 3, `09` 19, `10` 6.
- **Your tasks so far are done and committed:** `dc4-version-differences.md`, `data-region-shift-map.md` and `re07-mystery-data.md`, with their tools (`a86adf1`, `d49da45`, `9546533`).
- **In progress (yours): RE-06**, encounters and enemy records. See `notes/task-gemini-re06-encounters.md`. Its constraints (new files only, no `make`) were written for when Claude was working in parallel. **Claude has stopped, so you may now also build and edit freely in `decomp/`.**
- **Claude's RE this session (docs/, committed):**
  - RE-09 shops: `docs/re/shops.md`. Stock is copied into the save at new game; prices are ×100.
  - free ROM space: `docs/re/free-space.md`. 35.9 KiB unused at `0x087F739C`–`0x08800000` in both versions.
  - The Mystery Data corrections from your RE-07 are in `docs/re/save-and-flags.md`, `docs/re/completion-and-new-game-plus.md` and `docs/completion/mystery-data-shops-traders.md`.

## Key Decisions & Constraints

- **The regeneration workflow changed** (also in `decomp/README.md`). For segment X: `disasm.py rs X`, then `disasm.py bm X`, then `merge_bm.py X`, then both compares. Always regenerate RS first; `merge_bm.py` refuses to run on a file that's already shared.
- Until the merge runs, `split.py` assembles a fresh `asm/bm/X.s` through a renamed copy in `build/bm/norm/`, so `disasm.py bm`'s self-check works.
- **The renaming is one-to-one, and a line is shared only if both versions agree on it.** That's why the merge can't produce wrong bytes. Keep it that way: any new mapping (e.g. for `incrom` tables) must stay a bijection on the addresses it covers. Unmapped BM addresses get a `_bm` suffix.
- **When you change the layout, order matters:**
  - regenerate a segment whose range changed before its neighbours;
  - a new segment that has no file yet is assembled from `incrom` automatically.
- **Full regeneration takes about 25 minutes.** Run it in the background and don't interrupt it, which would leave mixed files. Claude's script: for each segment, the three commands above in a loop, then both compares.
- **Gotcha:** from PowerShell, write the venv path out in full (`~/.venvs/mmbn4-decomp/bin/python`). A `$P` shell variable is expanded by PowerShell 5.1 even inside single quotes, and plain `python3` has no Capstone.
- **Ownership:** Claude may work in `decomp/` (owner approval, `AGENTS.md` §1). New game folders (BN1, 2, ...) belong to Claude by default; you and Codex work there only on assigned tasks.

## Modified & Relevant Files

- `tools/merge_bm.py` (new this session): the merge, the BM↔RS maps for code segments and the data region, `active_lines()` for `VERSION_*` blocks.
- `tools/split.py`: shared-source includes, BM externs placed at BM addresses, renaming of not-yet-merged BM views.
- `tools/disasm.py`: code- and data-region words become `dat_` symbols; error regex for `build/bm/norm/`.
- `tools/coverage.py`: counts only the lines active for the version.
- `config/segments_{rs,bm}.toml`: the new split.
- `asm/*.s`: shared. `code_04b.s`, `code_04c.s` and `code_04d.s` are new.
- `notes/handoff-2026-11-22-claude-dc4.md`: DC-4 details, including the "Update, later the same night" section.
- `docs/re/proposed-names.txt` (Claude's): **12 shop names waiting to be merged**, including a correction of the `BN4_SHOP_RECORDS` description (`+0xC` is the slot count).
- **Uncommitted:** nothing. The working tree was clean at `1fd88fd` plus this handoff.

## Open Issues / Blockers

- **Not pushed:** local `main` is 10+ commits ahead of `origin/main`. The owner hasn't answered whether to push. Don't push unless the owner asks.
- **`TODO.md` is stale** (yours to update): it still describes `asm/bm/` and lists DC-4 as open. It should record:
  - DC-4 one-tree: done;
  - version blocks reduced to 92;
  - RE-07: done;
  - RE-09: mostly done (`docs/re/shops.md`; open: which record is which shop in the world, and the Number Trader);
  - free space: done;
  - the Mystery Data flag correction.
- **Names waiting to be merged:** Claude's 12 in `docs/re/proposed-names.txt` and yours in `re07-mystery-data.md` §8. Merging them into `symbols/names.txt` changes the asm annotations, so a full regeneration should follow. Remember the gotcha: names must be `[A-Za-z0-9_]` only, or the build breaks.
- **The remaining 92 blocks are mostly real differences**: save XOR keys, version index 0/1, the Soul and Navi tables, the Double Soul flag, BM's extra instructions in 4 functions, the 21 palette-row pointers and a few code-region constants. Some could become shared source with named constants (e.g. `.set VERSION_INDEX, VERSION_BM`), which would be optional polish.
- `incrom` tables still hold about 5,000 raw data pointers. They become shared only when those tables are carved into `.word` data.

## Immediate Next Steps

1. Finish **RE-06** (`notes/task-gemini-re06-encounters.md`). You can now also build.
2. Update `TODO.md` with the state above.
3. Merge the waiting names (Claude's `docs/re/proposed-names.txt` and your RE-07 §8) into `symbols/names.txt`. Delete the merged lines from `proposed-names.txt`, run a full regeneration (RS, BM, merge per segment), and run both compares.
4. Optional DC-4 polish: shared named constants for the version-index and XOR-key blocks. Carving the pointer-heavy `incrom` tables into `.word` data would let their data pointers share too.
5. Write your own handoff in `decomp/notes/` when you stop, as usual.
