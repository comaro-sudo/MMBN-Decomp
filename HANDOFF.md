# Handoff: Claude main session (2026-11-27, written at 97% context)

Cross-area handoff: decomp, patch, ports, docs, and parallel agents. Resume with: "Read HANDOFF.md and continue from Immediate Next Steps."

## Goal & Current Status

- **Decomp (verified, both `make -B compare` OK):**
  - `tools/regen.sh` rebuilds the whole asm tree from the two ROMs (`27de1e7`). Coverage is **86.5%**.
  - The stricter cutscene-seed rule is adopted (`94a58f7`).
  - Docs: `decomp/notes/handoff-2026-11-27-claude-decomp.md`.
- **Patch:** every package PASSes (see `patch/README.md`). Release dry run OK; checklist §2 ticked (`b507de8`).
- **Parallel agents:** batches 1–3 merged. Batch 4 (`MMBN4-Decomp/docs/tasks-claude-agent-2026-11-27d.md`) is partly merged:
  - **merged:** task 3 Library research (`e565508`), task 4 existing saves (`ba58f86`), task 10 BN1 resources (`e0e4d1b`);
  - **done, NOT merged:** tasks 1+2 on branch `claude-agent/d12` (worktree `C:\proyectos\MMBN-wt\d12`):
    - `87510c1`: the Tuner soft-lock fix. FC 0B now ends through the script jump `sub_0804E610` to entry 0x0F;
    - `ce66f09`: the status page 3 fix, widening the game's 0xFF fill of `0x0201D000` from 8 to 0x20 bytes at RS `0x080351AC` / BM `0x080351B4`;
  - **merged after this was written:** task 6 smoke checks (`a59f2f7` cherry-picked; Tuner in BM + Mystery Data pickups, all 5 checks pass);
  - **still running:** task 5 (mGBA, `d5`), tasks 7+9 (`d79`), task 8 (C1–C5 on the release build, `d8`).
- `main` is about 75 commits ahead of `origin`, **not pushed** (ask the owner).

## Key Decisions & Constraints

- **Owner decisions:** `docs/pending/*.md`, including `owner-decisions-2026-11-24.md` and `findings-2026-11-27.md`.
- **Still open for the owner:**
  - the patch licence (`docs/pending/patch-licence.md`);
  - the public chip art (`docs/pending/public-chip-art.md`);
  - whether Duo and GP Power appear in the Library;
  - the version scheme (`docs/release-notes-template.md`);
  - the BN1 and Principia questions.
- **Parallel-agent pattern:**
  - each agent gets its own worktree, created by hand: `git worktree add -b claude-agent/<t> C:/proyectos/MMBN-wt/<t> main`. The built-in isolation fails on the drive-letter case.
  - Merge with `git cherry-pick`.
  - Before removing a worktree, check for junctions (`Get-ChildItem -Attributes ReparsePoint`); remove them with `cmd /c rmdir`, never recursively.
- **Test agents must not touch the live PC save** `%APPDATA%\MMBN4Overhaul\PC\B4WE.sav` (one did and restored it).
- **Never commit ROM bytes, saves or images.** Stage by path.

## Modified & Relevant Files

- New: `docs/tests/` (test reports), `docs/known-issues.md` (coming from `d79`), `patch/test/smoke/`.
- No uncommitted changes in `main` other than this file.

## Open Issues / Blockers

1. **Check `ce66f09` before merging.** `0x0201D010`+ appears as a second buffer: `code_03.s` lines ~15371 and ~35785 have `.4byte 0x0201D000, 0x0201D010` pairs, and line ~1174 has `0x0201D020`. Filling 0x20 bytes from `0x0201D000` may clobber `0x0201D010`–`0x0201D01F`, and the page byte at `+0x18`. Find out what `0x0201D010` holds on the status screen.
   - The tests passed, but if it's shared, fill only 0x0D bytes (13 Souls) instead of 0x20.
2. **Once `d12` is merged, `smoke.py`'s `check_status_pages` must be updated** (the agent's edit was blocked as outside its brief). It should expect:
   - 2 pages with the 11/9-Soul saves, R wrapping;
   - 3 pages with flags 0x17–0x23 set.

   Agent `d6` also edits `smoke.py`, so merge `d6` first, then adjust.
3. Release checklist §3: tick from the `docs/tests/*` reports once `d5` and `d8` land. The "Existing saves" item can be ticked after the page-3 fix is merged.

## Immediate Next Steps

1. Wait for the agents in `d5`, `d6`, `d79` and `d8`, then review and cherry-pick each.
2. Resolve Open Issue 1, then cherry-pick `87510c1` and `ce66f09` from `claude-agent/d12`. Rebuild (`python patch/build.py`, `--check`), then update `smoke.py` (Open Issue 2) and run it.
3. Tick release checklist §3 from the reports.
4. Remove the `d*` worktrees (junction check first) and delete the `claude-agent/*` branches.
5. Ask the owner to push. Report the open owner decisions.
