# Handoff: Claude main session (2026-11-27, written at 97% context)

Cross-area handoff: decomp, patch, ports, docs, and parallel agents. Resume with: "Read HANDOFF.md and continue from Immediate Next Steps."

## Goal & Current Status

- **Decomp (verified, both `make -B compare` OK):**
  - `tools/regen.sh` rebuilds the whole asm tree from the two ROMs (`27de1e7`). Coverage is **86.5%**.
  - The stricter cutscene-seed rule is adopted (`94a58f7`).
  - Docs: `decomp/notes/handoff-2026-11-27-claude-decomp.md`.
- **Patch:** every package PASSes (see `patch/README.md`). Release dry run OK; checklist §2 ticked (`b507de8`).
- **Parallel agents:** batches 1–3 merged. Batch 4 (`MMBN4-Decomp/docs/tasks-claude-agent-2026-11-27d.md`):
  - **merged:** tasks 1+2 (`f90e7de` Tuner soft-lock fix; `55edb4f` status page-3 fix, refined to fill `0x18` bytes so `+0x18` is untouched), task 3 Library research (`e565508`), task 4 existing saves (`ba58f86`), task 5 mGBA test (`docs/tests/mgba.md`), task 6 smoke checks (`5c5d86f`, plus `check_status_pages` updated for 2-page 9/11-Soul & 3-page 13-Soul saves — all 5 smoke checks PASS), task 10 BN1 resources (`e0e4d1b`);
  - **not finished (worktrees were empty/unstarted when previous session ended):** tasks 7+9 (`docs/known-issues.md` + player README in-game checks), task 8 (C1–C5 matrix on the `--public` release build).
  - All `d*` worktrees (junctions safely removed first) and `claude-agent/d*` branches have been cleaned up.
- `main` is ahead of `origin` and **not pushed** (ask the owner before pushing).

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

1. **Batch 4 remaining tasks (`d79`, `d8`):** `d79` (in-game "Where to find" checks + `docs/known-issues.md`) and `d8` (C1–C5 matrix on `--public` release builds) had empty worktrees when the previous session ended and still need to be run.
2. **Release dry-run SHA-1s:** `docs/release-checklist.md` §2 SHA-1s were recorded before the `d12` Tuner & status page-3 fixes landed; re-run `python patch/tools/release.py` once ready to refresh `RELEASE-SHA1.txt` and §2.

## Immediate Next Steps

1. Ask the owner to push `main` to `origin` and confirm the open owner decisions listed above.
2. Run remaining batch 4 tasks: `d8` (C1–C5 matrix on `--public` release builds → `docs/tests/c1-c5-release.md`, then tick checklist §3) and `d79` (`docs/known-issues.md` + player README in-game checks).
3. Re-run `python patch/tools/release.py` to refresh `--public` release SHA-1s after the `d12` fixes.
