# Tasks for a second Claude agent (written by the main Claude session, 2026-11-27)

The owner asked for 10 tasks that don't interfere with the main Claude session.
**That session is rebuilding the decomp pipeline:** `decomp/tools/regen.sh` and the `emit_*.py` tools; it will change `decomp/asm/*.s`, `decomp/include/`, `decomp/rom.s` and `decomp/symbols/names.txt`.

## Rules for you

- **Don't edit anything under `MMBN4-Decomp/decomp/`**, nor `TODO.md` (Gemini's). Reading is fine.
- Read `AGENTS.md`, then `MMBN4-Decomp/docs/handoff-violet-eclipse.md` and `patch/README.md`.
- Your areas for these tasks: `MMBN4-Decomp/docs/`, `patch/`, `ports/`, `plan/`, and `MMBN-Principia-Trilogy/`.
- **Never commit ROM bytes** or extracted assets. `ref/` and `roms/` are gitignored.
- Stage by explicit path, one commit per task, and don't push.
- Check patch changes with `python patch/build.py` + `--check`, run from `MMBN4-Decomp/`.
- End with a handoff: `MMBN4-Decomp/docs/handoff-claude-agent-2026-11-27.md`.
- Decisions that belong to the owner: write the options down, don't decide.

## The tasks (in this order)

1. **Release packager** (`patch/tools/release.py`):
   - builds `--public` for both versions;
   - checks `--check`;
   - writes `build/release/` with the two `.bps`, their SHA-1s and the input ROM SHA-1s, plus a copy of the player README;
   - zips it.
   Don't commit the outputs (`build/` is gitignored). Follow `docs/release-checklist.md` §2.
2. **Credits and licences** (`docs/credits.md`): list every tool and third-party work the patch and ports use (armips, TextPet, Flips, mGBA, SDL3, Cemu/Dolphin for testing, `ext/mmbn-chip-tables`, Prof. 9's WoodMan fix, the community symbol sources and test saves). For each: the author, the licence (from its own repo or files), and what attribution it requires. Then fill the "Credits" section of `docs/player-readme-draft.md`.
3. **Licence options for the patch** (`docs/pending/patch-licence.md`): the ports are GPL-3.0 (owner, `D-EL3`). Write 2–3 options for the patch sources and `.bps` (e.g. GPL-3.0 like the ports, MIT, CC0), with what each means given the tools' licences from task 2. Facts only.
4. **Placeholder chip art options** (`docs/pending/public-chip-art.md`): for `--public` builds, Duo and Grand Prix Power have no picture.
   - Propose 2–3 approaches: a generated original emblem, a recoloured existing BN4 chip picture *taken from the player's ROM at build time* (like `patch/tools/chipart.py` does for icons), or blank.
   - Say how each fits the "no Capcom data in the release" rule.
   - You may prototype a generator in `patch/tools/`, but don't wire it into `build.py`.
5. **Player README review:**
   - check every claim in `docs/player-readme-draft.md` against `patch/README.md` and the package sources in `patch/src/common/`;
   - fix what's wrong;
   - list what you couldn't confirm.
6. **Port READMEs:** check that `ports/README.md`, `ports/pc/`, `ports/wiiu/` and `ports/3ds/` READMEs are current (the build commands work as written; the status matches `TODO.md`'s Done table). Fix the docs only, not the code.
7. **The 3DS port, static review:** read `ports/3ds/` and `ports/common/` for problems a compile would hit (missing includes, types, API misuse against libctru/citro2d's public headers), without devkitPro. Write the findings in `docs/ports-3ds-review.md`; fix clear errors in `ports/3ds/`.
8. **Collection README** (repo-root `README.md`, Claude's): make it a clear index of the collection's status. BN4 (the patch, ports, decomp %), the planned BN1–3/5/6, and the Principia Trilogy, with links.
9. **MMBN1 decomp bootstrap plan** (`MMBN1-Decomp/README.md` and `MMBN1-Decomp/plan/`): plan BN1 the way BN4 was started, using `MMBN4-Decomp/plan/` and `decomp/notes/decomp-plan.md` as models:
   - ROM identification (US/JP, SHA-1 to record);
   - known community resources;
   - the toolchain BN4 already has that can be reused;
   - the first milestones.
   Plan only: no ROM is needed, and don't create any `tools/`/`roms/` content. (The owner's order is BN4, then BN1.)
10. **Principia Trilogy design pass** (`MMBN-Principia-Trilogy/`): expand the README's plan B (BN1–3 merged into one game on the BN3 engine) into a design outline: scope per game, how saves and progression merge, what needs each game's decomp, risks, and the questions the owner must answer. Read `MMBN-Principia-Trilogy/README.md` first.
