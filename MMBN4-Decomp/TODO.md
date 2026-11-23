# Violet Eclipse (MMBN4) TODO

Updated: 2026-11-23 by Claude, at the owner's request (this file is normally Gemini's; see [`AGENTS.md`](../AGENTS.md)).
The short, execution-oriented backlog. Detailed plans: [`plan/`](plan/README.md). Current handoffs:
[`docs/handoff-violet-eclipse.md`](docs/handoff-violet-eclipse.md) (Claude) and the newest `decomp/notes/handoff-*.md` (Gemini).
Open owner decisions: [`docs/pending/`](docs/pending/).

## Coordination

- Ownership: **Gemini** owns `decomp/` and this file (Claude may also work in `decomp/`). **Claude** owns `plan/`, `docs/`, `patch/` and `ports/`.
  New game folders (BN1, 2, …, the Principia Trilogy) are Claude's by default.
- Stage by explicit path, never `git add -A`. Never commit ROM bytes.
- [ ] **Push to GitHub:** local `main` is **85 commits ahead** of `origin/main` (last push `6e47d43`). The owner decides when.

## Current verified state

- **Decomp:** US Red Sun and Blue Moon both rebuild byte-for-byte from **one shared `decomp/asm/` tree** (DC-4).
  - Symbols follow RS addresses. BM data addresses map through `symbols/datashift_rs_bm.txt`.
  - There are **132 `.if VERSION_*` blocks**: genuine differences, plus pointer tables into the `code_04` cutscene scripts, which shift differently per version.
  - Coverage is **59.4%** (RS 700,816 / 1,179,456 code bytes). The DC-2 table carving is done: the encounter tables plus 208 pointer-dense tables.
- **Patch:** six packages, **all built and passing**. Four were tested in-game by Gemini (`decomp/notes/patch-test-results.md`); see [`patch/README.md`](patch/README.md). Building needs **both ROMs**.
- **Tests run:** a PC-port boot smoke test, plus live tests of WP-A2, WP-D3, WP-D4 (star) and WP-D2.

## Decomp (DC-2 / DC-3 / DC-5)

- [x] DC-4: one shared RS/BM tree; segments split at the true shift points; code- and data-region pointers as symbols.
- [x] DC-2 carving: the encounter tables, 4 big and 204 small pointer tables (`db01bda`, `1c8fca7`, `43ad295`).
- [x] DC-3: names merged through Gemini's round 2 (`11f8a5b`).
- [ ] Merge the names proposed in Gemini's round-3 to round-5 notes (`lobby-npcs.md`, `free-tournament-battle-data.md`,
  `status-screen-souls.md`, `waiting-room.md`, `status-screen-two-pages.md`, …) into `symbols/names.txt`, then do a full regeneration.
- [ ] Remaining `incrom` is mostly mixed data. Carve each table only once its record layout is decoded, as structured data, never as bulk hex.
- [ ] Reduce the version blocks: the `code_02` and `code_05` pointers into the `code_04` cutscene scripts need a per-range map of those scripts.
- [ ] DC-5 (later): hybrid C infrastructure, per `plan/decomp/05-milestones-and-risks.md`.

## Reverse engineering

- [x] Save format, flags, the playthrough counter, Zenny/BugFrags, Souls, completion stars, New Game+.
- [x] RE-04 tournaments, RE-06 encounters/enemies/drops, RE-07 Mystery Data, RE-09 shops and the Number Trader, all save slots, free ROM space.
- [x] The Soul grant: both ROMs contain all 12 grant scripts; the BM Soul keys are in the same place (`decomp/notes/soul-grant.md`).
- [x] Bass rematch feasibility, the lobby (Higsby's) NPCs and jack-ins, the Free Tournament battle data, the status screen, BN4's NPC cap (16).
- [ ] Higsby's walkable floor grid (for exact operator positions); a dead end in the Undernet and a corner in ACDC for the two special-chip Mystery Data.
- [ ] Flag `0x14` (tournaments); other traders (`sub_0804C680`).

## Violet Eclipse patch: to build (owner decisions recorded in `docs/pending/` and `docs/completion/`)

- [ ] **Status screen, two pages of 6 Souls** (L/R or Select). Gemini's design is in `decomp/notes/status-screen-two-pages.md`, about 176 bytes.
- [ ] **Lobby rematches:**
  - 9 operators inside Higsby's and 2 on the ACDC Town street (relocate both NPC lists to free space);
  - each one appears after Lan meets that Navi (fallback: the unmet ones appear in the post-game);
  - the V1 → V2 → V3 → SP climb with S-rank wins (lobby only), using `BN4_STORY_BATTLES` records with the "no tier raise" bit.
- [ ] **JunkMan:** add a trash-can NPC outside Higsby's at `(-150, -90)` that jacks into a small cyber map (`(8E, 00)`, `(8E, 02)` or `(88, 00)`), where JunkMan waits.
- [ ] **Free Tournament fix** for the 36 secret chips: draw the 12 Soul Navis from the ROM's records instead of the traded Waiting Room (owner: pending in `docs/pending/free-tournament-secret-chips.md`).
- [ ] **Tournaments:** one canon story fight plus 3 friendly matches each (lineup in `decomp/notes/canon-navis.md` §6; all friendlies before the final; a win gives the Soul).
- [ ] **Special chips:** Duo as the reward for beating Duo at his highest tier. Z-Saver in a Blue Mystery Data in an Undernet dead end. Grand Prix Power in a Blue Mystery Data in a quiet ACDC corner.
- [ ] **Other version's Souls** without a trade (set the flag + `WriteChipKey`, both ROMs have the scripts); `checkDoubleSouls` for 12.
- [ ] Deterministic tournament pairs (WP-B1), Frequency Tuner (WP-B2: `BN4_ID_RAISE` is the single hook), save protection, the remaining boss rematches (WP-D3).
- [ ] Tier A leftovers: story-battle rewards, key-item prices (owner decision pending), ToyRobo, cutscene skip ([`plan/03`](plan/03-phase1-rom-overhaul.md)).
- [ ] Existing saves: a load hook for anything copied into the save at new game (shop stock).

## Ports

- [x] PC-1: the ARM64 Windows app (embedded mGBA + SDL3, PET window, headless `--test`, `--dump`, `--state-in/out`).
- [ ] EL-0: refactor into `ports/common`. Then PC-2: BKPT hooks, PET state, touch Custom Screen, net map (our own design inspired by MMBN1 DS and MMBN5 DS, no copied code or assets).
- [ ] The Wii U, New 3DS and NDS routes ([`plan/emulation-layer/`](plan/emulation-layer/README.md)).

## Collection

- [ ] BN1, BN2, BN3, BN5 and BN6 decomps, after BN4. The second game moves the shared tools and emulators to the repo root (`AGENTS.md` §4).
- [ ] [MMBN Principia Trilogy](../MMBN-Principia-Trilogy/README.md): BN1–3 merged into one game, for ports only (PC, NDS, 3DS, PSP, PS Vita, Wii U, Android). It needs the BN1–3 decomps first.

## Owner decisions still open

- [ ] Use the Free Tournament fix for the 36 secret chips? (`docs/pending/free-tournament-secret-chips.md`)
- [ ] RS/BM version choice in the merged ROM (`plan/DEFINITIVE-EDITION.md` §7 q1). The canon lineup assumes one merged story.
- [ ] Key-item prices fixed or as in-game options; Tier C content in 1.0; D1–D9 ([`plan/00` §4](plan/00-decisions-and-feasibility.md)).
- [ ] Hardware owned for the console ports; the GBARunner licence ([`plan/emulation-layer/06`](plan/emulation-layer/06-milestones-and-risks.md)).
- [ ] Principia Trilogy: the base engine, which mechanics survive, how the story joins, the name.

## Before committing

- [ ] `git diff --check`; both forced byte-match builds; `python patch/build.py --check`; coverage when the asm changes.
