# Tasks for a second Claude agent, batch 2 (written by the main Claude session, 2026-11-27)

The first batch (`tasks-claude-agent-2026-11-27.md`) is done and merged. **The main session is working in `MMBN4-Decomp/decomp/`** (the regeneration pipeline `decomp/tools/regen.sh` and the emitters). So these tasks stay out of it.

## Rules for you

- **Don't edit anything under `MMBN4-Decomp/decomp/`**, nor `TODO.md` (Gemini's). Reading is fine.
- Read `AGENTS.md`, `MMBN4-Decomp/docs/handoff-violet-eclipse.md`, `patch/README.md`, and the file each task names.
- **Never commit ROM bytes**, extracted assets, screenshots of the game, or build outputs. `roms/`, `ref/`, `tools/`, `ext/`, `build/` are gitignored.
- Stage by explicit path, one commit per task, and don't push.
- Patch changes: `python patch/build.py` + `--check` (from `MMBN4-Decomp/`) must pass.
- Port code changes: build the PC port if you touched shared code (see `ports/pc/README.md`).
- Decisions that belong to the owner: write the options down, don't decide. Pending ones are in `docs/pending/` (patch licence, public chip art).
- End with a handoff: `MMBN4-Decomp/docs/handoff-claude-agent-2026-11-27b.md`.

## The tasks

1. **Shared port code fixes** (`ports/common/`), from `docs/ports-3ds-review.md`'s list of `ports/common` findings:
   - the PET status footer shows PC keyboard help on the 3DS and Wii U (make it per-platform);
   - `pet_model.c` reads `ram_save_shift` with a 32-bit read, but `game.h` documents it as a `u8`;
   - the chip count `cid < 389` is hard-coded, though the patch adds chips (read it from the ROM's table, or explain why 389 is right);
   - the small leak on `emu_open()`'s `core->init` failure path.

   Rebuild and run the PC port (`ports/pc/README.md`: its `--bench` or test mode) to show nothing regressed.
2. **3DS audio** (`ports/3ds/` only): replace the per-frame audio drain with an `ndsp` backend (stereo s16 from `emu_read_audio`, double-buffered wave buffers). There's no devkitPro here, so check every call against the public libctru headers. Note what still needs a real 3DS. Update `docs/ports-3ds-review.md`.
3. **What `--public` builds show for Duo and Grand Prix Power**: build `python patch/build.py --public`, then use the PC port's test/screenshot options with a save that holds both chips. `patch/tools/savetool.py` can help; the test saves are in `ref/saves/`. Check what the Library and Folder show: a blank picture, garbage, or a crash. Write the result into `docs/pending/public-chip-art.md` (baseline of option C). Don't commit screenshots.
4. **A committed patch smoke test** (`patch/test/smoke/`): a script that drives the PC port headlessly (inputs, frames, RAM reads at the addresses in `patch/README.md` and `docs/re/`) and checks a few packages end to end. Suggested checks: the 3 status pages, the Frequency Tuner flag toggle, shop prices on a new game, and a Blue Mystery Data flag after pickup. It should read the ROMs and saves from the gitignored folders at run time, and commit no ROM data, states or images. Document how to run it in `patch/README.md`.
5. **Licence notices in port packages**: per `docs/credits.md`, binary releases must ship mGBA's MPL-2.0 licence with a source pointer, SDL3's zlib licence, and the notices of mGBA's bundled libraries. Update `ports/pc/package_pc.py` (and the Wii U packaging under `ports/wiiu/`) so the release zip includes them, read from the gitignored `ext/`/`tools/` copies at package time. Run the PC packager to check it.
6. **Docs index**: make `MMBN4-Decomp/docs/README.md` a current index of everything in `docs/` (re/, completion/, pending/, design/, hardware/, reference/, the handoffs, release checklist, credits, player README), one line each, with which are current and which are historical.
7. **`patch/README.md` clarity pass**: the package table has grown to ~25 rows. Group it by area (economy, Souls, tournaments, rematches and SP+, chips and Mystery Data, infrastructure), keep every package and its test status, and make sure each row says what the player sees. Don't change any package.
8. **`plan/DEFINITIVE-EDITION.md` status pass**: mark what is built and tested (from `patch/README.md` and `TODO.md`'s Done table), what is open, and which owner decisions are recorded where. Plan text only.
9. **BN1 resource check** (`MMBN1-Decomp/plan/README.md`): the plan lists Silenthal/bn1 with its licence and completeness unconfirmed. Check its repository (licence file, README, how much is disassembled or matching, last activity) and any other public BN1 disassembly, and update the plan with sources. Facts only; whether to build on it is the owner's call.
10. **"Where to find" table check**: the player README (`docs/player-readme-draft.md`) gives the locations of the new content from asm comments. Check each against the map ids and coordinates in the patch sources (`patch/src/common/*.asm`) and the map names in `docs/re/` and `decomp/notes/` (read only). Fix the table, and list anything that needs an in-game look.
