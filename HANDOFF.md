# Handoff: Claude main session (2026-11-27, written at 97% context)

Cross-area handoff: decomp, patch, ports, docs, and parallel agents. Resume with: "Read HANDOFF.md and continue from Immediate Next Steps."

## Goal & Current Status

- **Decomp (verified, both `make -B compare` OK):**
  - `tools/regen.sh` rebuilds the whole asm tree from the two ROMs (`27de1e7`). Coverage is **86.5%**.
  - The stricter cutscene-seed rule is adopted (`94a58f7`).
  - Docs: `decomp/notes/handoff-2026-11-27-claude-decomp.md`.
- **Patch:** every package PASSes (see `patch/README.md`). Release dry run OK; checklist §2 ticked (`b507de8`).
- **Parallel agents:** batches 1–3 merged. Batch 4 (`MMBN4-Decomp/docs/tasks-claude-agent-2026-11-27d.md`):
  - **done & merged:** tasks 1+2 (`f90e7de` Tuner soft-lock fix; `55edb4f` + `1d1be30` status page-3 `0x18`-byte fill), task 3 Library research (`e565508`), task 4 existing saves (`ba58f86`), task 5 mGBA test (`565af34`, `docs/tests/mgba.md`), task 6 smoke checks (`5c5d86f` + `1d1be30`, all 5 checks PASS on `--public`), task 8 `--public` release check (`docs/tests/c1-c5-release.md`, 10/10 `smoke.py` checks PASS; C3a/C3b/C4 battle checks not re-run), task 9 known issues (`docs/known-issues.md` + both player README drafts updated), task 10 BN1 resources (`e0e4d1b`);
  - **remaining:** task 7 (5 in-game "Where to find" visual spot-checks on the PC port).
  - All `d*` worktrees (junctions safely removed first) and `claude-agent/d*` branches have been cleaned up.
- `origin/main` is synced through `1d1be30` (plus the new `docs/known-issues.md`, `docs/tests/c1-c5-release.md`, checklist §2 refresh commit).

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
2. Batch 4 task 7: the 5 in-game "Where to find" spot-checks on the PC port (see `docs/tasks-claude-agent-2026-11-27d.md`).
3. (Done: release SHA-1s refreshed after the d12 fixes, `docs/release-checklist.md` §2.)

## Resolved 2026-11-28: the page-3 fill and `0x0201D010` (Claude)

- **Not a problem.** On the status screen, `0x0201D000`–`0x0201D01F` is one 32-byte status buffer: `sub_08035A7C` (`code_03.s` ~line 1162, next to the status code) restores it as a block with `CopyByEightWords(0x0201D020 -> 0x0201D000, 0x20)`. The `0x0201D000`/`0x0201D010` pointer pair in `dat_0803D4AC` belongs to another screen, which reuses this scratch RAM at another time. The `0x18`-byte fill (`1d1be30`) stays inside the status buffer and before the page byte at `+0x18`.
