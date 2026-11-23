# Three tasks for Gemini, round 6 (assigned by the owner, written by Claude, 2026-11-23)

Round 5 was excellent: 4 of 4 patches pass in-game. The owner's decisions since are in `docs/pending/free-tournament-secret-chips.md`:
- no waiting room;
- **9 operators inside Higsby's, 2 outside** plus the trash can.

The work order is now in `TODO.md` ("Order of work"). **Claude is doing steps 1–2:** merging names with a full regeneration of `decomp/asm/`, then the status-screen patch in `patch/`. Your three tasks prepare later steps, so they're **read-only analysis**:
- **don't edit** `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `docs/`, `patch/` or `TODO.md`;
- **don't run `make`** and don't write to `decomp/build/`;
- read asm from git (`git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`);
- you may run the PC port and `python patch/build.py`, with outputs **outside the repo** (no images in the repo);
- write only the new notes named below, your scripts in your own folder, and your handoff;
- put proposed names in a section at the end of each note;
- stage only your own files by path, and don't push.

## Task 1: positions for the new NPCs and Mystery Data (order step 7) (`decomp/notes/placements.md`)

1. **Higsby's `(00, 06)`, 9 operators.** Give 9 walkable spots that keep the paths from the door to the counter, the left table and the Free Tournament terminal clear, as map coordinates `(x, y, z)` and facing. Use the map's collision data or test positions in the PC port. Suggested areas (the owner's screenshot): along the right wall under the shelves, the front-left corner by the blue machine, beside the rug.
2. **ACDC Town `(00, 00)`, 2 operators + the trash can.** The trash can is at `(-150, -90, 0)` (round 5). Give 2 more free spots near Higsby's entrance, clear of the door `(-176, -90)` and the existing NPCs.
3. **The two special-chip Blue Mystery Data:**
   - **Z-Saver** in an **Undernet dead end** off the main path;
   - **Grand Prix Power** in a **quiet ACDC corner** (ACDC Area 1, 2 or 3).

   For each: the area `(group, map)`, coordinates, and why it's a dead end or corner (`ref/maps/` and RE-07's placements, so it doesn't overlap an existing Mystery Data). Also two free "taken" flags in `0x0D00`–`0x10FF`, preferably in those areas' own flag blocks.

## Task 2: the deterministic-tournament design (WP-B1, order step 4) (`decomp/notes/wp-b1-design.md`)

The owner's tournament design (`decomp/notes/canon-navis.md` §6, and `docs/pending/` "Exhibition matches, in detail"):
- each story tournament has **one canon story fight** (1st: AquaMan; 2nd: Roll; 3rd: ProtoMan) and **3 friendly matches** against its other Soul Navis;
- all the friendlies happen before the final, 2 or 3 of them before the canon fight;
- Dex/GutsMan challenges Lan after the 1st canon fight, and Raika/SearchMan before the Chaud fight.

Design the patch points:
- **Fix the bracket:** the pair coin flip (flag `0x15`, `GetTertiaryRNG`), `TournamentCheckSoulPairSwap` (`0x0804480C`, flag `0x9B`), `BN4_DRAW_BRACKET` (`0x080442EC`) and the entrant fill. The canon Navi must be MegaMan's Soul-Navi opponent in every playthrough.
- **Where the friendly matches can be inserted:** which scripts run between rounds and before the final (tournament day flow), and how a script could start a battle against a Soul Navi's `BN4_STORY_BATTLES` record and then run the Soul grant.
- **Version note:** the canon lineup mixes both versions' Navis (AquaMan and ProtoMan are BM's, Roll is RS's). Say what each ROM needs to field them.

## Task 3: granting any Soul, and `checkDoubleSouls` for 12 (order step 3) (`decomp/notes/soul-grant-patch-design.md`)

Your `soul-grant.md` showed how a Soul is granted: flag `0x17 + idx` + `WriteChipKey(0x161 + idx)`, and both ROMs have all 12 grant scripts. Design what the patch needs:
1. A **callable "grant Soul idx" routine** (or a script command sequence), usable from the friendly-match scripts (task 2) and from anywhere else: where to put it, its size, and whether an existing script command can do it without new code.
2. **`ScriptCmd_checkDoubleSouls`** (`0x0804F494`, RS flag `0x17` / BM `0x1D`, `TestEventFlagRangeAll(.., 6)`): what it's used for, which scripts call it, and how it should change for 12 Souls (count 12 from `0x17`?). List every reader of the Soul flag range, and of the counts 6 and 12, that could still assume 6, like the `cmp r5, #6` bug in the star.
3. **The status-screen Soul list** is Claude's step 2. Just note any other screen or menu that lists Souls.
