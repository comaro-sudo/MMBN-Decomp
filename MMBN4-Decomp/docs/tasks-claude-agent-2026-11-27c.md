# Tasks for a second Claude agent, batch 3 (written by the main Claude session, 2026-11-27)

Batches 1 and 2 are done or finishing. **Stay out of these areas, which are in use:**
- the main session is working in `MMBN4-Decomp/decomp/` (the regeneration pipeline) and on the Duo / Grand Prix Power chip records in `patch/src/` (the Library bit);
- another agent is still writing `patch/test/smoke/` and `patch/README.md`.

## Rules for you

- **Don't edit `MMBN4-Decomp/decomp/`, `patch/src/`, `patch/README.md`, `patch/test/`, or `TODO.md`.** Reading is fine.
- Read `AGENTS.md`, `MMBN4-Decomp/docs/handoff-violet-eclipse.md`, `docs/README.md` (the docs index) and the files each task names.
- **Never commit ROM bytes**, game images, saves, or build outputs. `roms/`, `ref/`, `tools/`, `ext/`, `build/` are gitignored.
- Stage by explicit path, one commit per task, and don't push.
- Decisions that belong to the owner: write the options down, don't decide. Pending: the patch licence (`docs/pending/patch-licence.md`) and the public chip art (`docs/pending/public-chip-art.md`).
- End with a report of commit ids, files, what you verified and what you couldn't.

## The tasks

1. **GPL-3.0 text in the port packages.**
   - The ports are GPL-3.0 (owner decision `D-EL3`), but their release zips don't ship the licence text.
   - Add the GPL-3.0 text as `MMBN4-Decomp/ports/LICENSE`, the official text from gnu.org, unmodified.
   - Make `ports/pc/package_pc.py` and `ports/wiiu/package_wiiu.py` put it in the zip (e.g. `LICENSES/GPL-3.0.txt`), and mention it in `NOTICE.txt`.
   - Run the PC packager to check.
2. **Stale docs.** Update the status lines only, keeping the design text as the record:
   - `docs/pending/free-tournament-secret-chips.md`, `docs/pending/duo-soul.md` and `docs/pending/boss-double-hp.md` still say "undecided", "idea" or "plan", but all three are built and pass in-game (see `patch/README.md`);
   - in `plan/DEFINITIVE-EDITION.md`, §4 still recommends the merged ROM, but the owner chose Option C, two complete patches.
3. **3DS: sleep and HOME-menu resume** (`ports/3ds/` only):
   - handle APT events (`aptHook`) so the emulation pauses on suspend;
   - make the ndsp audio (`src/audio_ndsp.c`) recover on resume.
   - There's no devkitPro here, so check the calls against the public libctru sources, and update `docs/ports-3ds-review.md`.
4. **PC port: ARM64 build and a Folder-tab check.**
   - Build the ARM64 PC port (`ports/pc/README.md`).
   - Run it with a save that has chips in the folder. See how `docs/pending/public-chip-art.md`'s baseline section made one: `savetool.py`, chip keys, checksum.
   - Check that the PET Folder tab shows every chip's name, attack and element, including Duo `0x137` and GP Power `0x138` (`ports/common/pet_model.c` now uses `GameInfo.rom_chip_count`).
   - Report only. Fix the code only if something in `ports/common/` is clearly broken.
5. **Wii U: a fresh build and package.**
   - devkitPPC and wut are installed in WSL (`docs/reference/toolchain.md`). Build the Wii U port (`ports/wiiu/README.md`), then run `ports/wiiu/package_wiiu.py` on the fresh `.wuhb`.
   - Boot it in Cemu if you can (`docs/reference/toolchain.md`); this checks the new per-platform PET footer text.
   - Don't edit `package_wiiu.py` (task 1 does); report packaging problems instead.
6. **Release dry run.**
   - Run `python patch/tools/release.py` (from `MMBN4-Decomp/`).
   - Apply each `.bps` with Flips (`tools/flips/`) to a clean copy of its US ROM, and check the result's SHA-1 against `RELEASE-SHA1.txt`.
   - Tick what passed in `docs/release-checklist.md` §2, with the date and the hashes.
7. **devkitPro library licences:**
   - WUT, wiiu-sdl2 and devkitPPC's libraries (Wii U), and libctru, citro2d, citro3d and devkitARM's newlib (3DS): read each licence from its public repository.
   - Fill the unconfirmed rows of `docs/credits.md`, and say what binary releases must ship.
8. **Spanish player README** (`docs/player-readme-es-draft.md`):
   - a faithful Spanish translation of `docs/player-readme-draft.md`;
   - keep the game's official English names for chips, Navis, Souls and places;
   - note at the top that the English draft is the reference.
9. **Port player guide** (`docs/ports-player-guide.md`): for players of the PC, Wii U and 3DS ports, cover:
   - controls per platform;
   - the PET tabs and what each shows;
   - Off-TV / single-window modes;
   - key remapping (`mmbn4.ini`);
   - where saves go;
   - how to point the port at a patched ROM.

   Take everything from the port READMEs and `main.c` files; mark anything unverified.
10. **Release notes template and versioning options** (`docs/release-notes-template.md`):
    - a template for each Violet Eclipse release: version, date, input ROM SHA-1s, `.bps` SHA-1s, what changed (by package), known issues, upgrade notes for saves;
    - 2–3 version-numbering options for the owner (e.g. 1.0.0 semantic, date-based), with what each implies.
