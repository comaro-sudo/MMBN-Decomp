# Tasks for a second Claude agent, batch 4 (written by the main Claude session, 2026-11-27)

Batches 1–3 are done or finishing. **Stay out of these areas, which are in use:**
- the main session works in `MMBN4-Decomp/decomp/`;
- two agents are still in `MMBN4-Decomp/ports/` (the packagers, `ports/LICENSE`, the port READMEs, the PC ARM64 build, possibly `ports/common/`).

## Rules for you

- **Don't edit `MMBN4-Decomp/decomp/`, anything under `ports/`, or `TODO.md`.** Reading and running the PC port are fine.
- Read `AGENTS.md`, `MMBN4-Decomp/docs/handoff-violet-eclipse.md`, `docs/README.md`, `patch/README.md`, `docs/pending/findings-2026-11-27.md` (the open findings) and the files each task names.
- **Never commit ROM bytes**, saves, savestates, game images or build outputs (`roms/`, `ref/`, `tools/`, `ext/`, `build/` are gitignored). Keep screenshots and saves in a temp folder.
- **Test reports** go in new files under `MMBN4-Decomp/docs/tests/` (one per task). Don't tick `docs/release-checklist.md` yourself: the main session does it from your reports.
- Patch changes: `python patch/build.py` and `--check` pass, the smoke test passes (`python patch/test/smoke/smoke.py`), and you check the change in-game on the PC port.
- Stage by explicit path, one commit per task, and don't push.
- Owner decisions: write the options down, don't decide.

**Files two tasks share:** tasks 1 and 2 both update rows of `patch/README.md`, and tasks 7 and 9 both edit the player READMEs. Do each pair in the same session, one after the other.

## The tasks

1. **Frequency Tuner: possible soft-lock** (`patch/src/common/wp_b2_frequency_tuner.asm`). The smoke test saw the text box stay open and empty after "Yes" (findings §2).
   - Reproduce it in the windowed PC port (talk to the Tuner in Higsby's, choose Yes, then cycle again).
   - If it's real, find the cause in the `FC 0B` handler: the script return, the r4 advance, or the text box close. Compare with the working handlers `FC 09` / `FC 0C` in `wp_d3_lobby_rematches.asm` / `wp_d4_grant_soul.asm`.
   - Fix it, and test all four modes (Auto, Tier 1, 2, 3).
   - Update the `patch/README.md` row.
2. **Status screen page 3** (`patch/src/common/wp_d4_status_two_pages.asm`). Findings §3: page 3 appeared with only 9 or 11 Souls, and was blank with a save-edited Duo Soul.
   - From real saves (no flag edits), check page 3 with 11, 12 and 13 Souls.
   - Find out why `SOUL_BUF[12]` isn't `0xFF` with fewer Souls, if that's what happens. Fix it, test, and update the `patch/README.md` row.
3. **Library listing for Duo and Grand Prix Power: pre-checks** (research, written in `docs/tests/library-flags.md`; read `decomp/asm` only). Findings §1 proposes setting chip byte `+9` bit `0x40` and byte `+0x16` bits `0x04 | 0x08` for `0x137`/`0x138`. Check:
   - that the Mega and Giga Library *listing* loops (`code_03` ~lines 4648–4912) don't filter on bits `0x04`/`0x08`;
   - every other read of byte `+0x16` (drops, traders, NetBattle, the Custom screen), and what bits `0x04`/`0x08` do there;
   - which Library numbers and tabs the two chips would then appear under.

   Don't change the patch: the owner decides whether they should appear.
4. **Existing saves on the patched ROM** (`docs/tests/existing-saves.md`). Load a vanilla mid-game save and a post-game save (`ref/saves/`, converted with `patch/tools/savetool.py`) on each patched version. Check:
   - the shop prices are synced to 40%;
   - Souls and the status pages are right;
   - the lobby operators appear per owned Soul;
   - nothing crashes on load or save.

   This is the release checklist §3 "Existing saves" item.
5. **mGBA test of both release builds** (`docs/tests/mgba.md`).
   - Run each patched ROM (`python patch/build.py --public`, or `patch/tools/release.py`) in real mGBA (`tools/mgba/`), not the PC port. Its scripting or `-t` savestate options may help.
   - Check that it boots, that a new game starts, and one patched feature per version (e.g. the status pages with a save).
   - This is the release checklist §3 "Real hardware or an accurate emulator" item.
6. **Smoke test: two more checks** (`patch/test/smoke/smoke.py` and its section in `patch/README.md`):
   - the Frequency Tuner in Blue Moon (the save edit loaded an empty room there: find out why);
   - a Blue Mystery Data pickup: Z-Saver in Undernet 1, or GP Power in ACDC Area 2. Use a save placed next to the Mystery Data, so no long walk is needed.
7. **"Where to find": the in-game checks** (`docs/player-readme-draft.md` and the Spanish copy `docs/player-readme-es-draft.md`). The previous agent listed five open checks (commit `bbe7408`, `docs/player-readme-draft.md` history):
   - the friendly-match round window;
   - when Dex and Raika appear and disappear;
   - whether players will recognise the Z-Saver and GP Power location descriptions;
   - the Space Center route to Meteor Comp 1 after the ending;
   - the trash-can spot.

   Check each on the PC port with suitable saves, and fix both READMEs.
8. **C1–C5 matrix on the release build** (`docs/tests/c1-c5-release.md`). Re-run Gemini's end-to-end matrix (`decomp/notes/patch-test-results-7-c1-c5.md`) on the `--public` release builds, RS and BM, using the smoke-test approach (headless PC port, RAM checks). Record PASS/FAIL per item. This is the release checklist §3 "Automated C1–C5 matrix" item.
9. **Known issues** (`docs/known-issues.md`, then the player README's "Known issues" section and its Spanish copy). Collect every open player-visible issue, each with its source and whether it's a bug, a limit, or waiting on the owner. Sources:
   - `docs/pending/findings-2026-11-27.md`;
   - `docs/ports-3ds-review.md`;
   - the batch reports in `docs/tasks-claude-agent-*.md` history;
   - `patch/README.md` statuses.
10. **BN1 community resources** (`MMBN1-Decomp/plan/resources.md`): public documentation useful for planning a BN1 decomp: RAM maps, the text and script formats, chip and enemy tables, save format, existing tools (e.g. text editors, randomizers). Cite each source, note its licence, and mark what you couldn't confirm. Facts only.
