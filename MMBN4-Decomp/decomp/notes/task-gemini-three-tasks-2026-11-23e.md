# Three tasks for Gemini, round 5 (assigned by the owner, written by Claude, 2026-11-23)

Round 4 caught a real bug: the Souls star's `cmp r5, #6`. **Fixed in `79398a4`**; the byte at `0x08000D56` is now `0C 2D` in both versions. Thank you.

**Claude is still regenerating `decomp/asm/`.** So:
- **don't edit** `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `docs/`, `patch/` or `TODO.md`;
- **don't run `make`** and don't write to `decomp/build/`;
- you may run `python patch/build.py`, which writes the patched ROMs to `MMBN4-Decomp/build/`, and the PC port, with outputs **outside the repo**;
- write only the new notes named below, your scripts in your own folder, and your handoff;
- stage only your own files by path, and don't push.

## Task 1: the waiting room (owner's option A) and the outside trash can (`decomp/notes/waiting-room.md`)

Round 4 didn't cover these; both are owner decisions in `docs/pending/free-tournament-secret-chips.md`.

**a) The waiting room, the owner's preferred option.** The street door of Higsby's leads into an **existing unused or rarely used room** that looks like a shop interior, and that room has a door into the real Higsby's `(00, 06)`. It's enabled only after a flag, so story scenes keep working. It holds the 11 operators (its own NPC slots). Find:
- the real-world map table layout (group `00` and others), and any **unused or unreferenced maps**, or maps used only once in the story, that look like an interior;
- **how doors and warps between maps are defined** (the street → Higsby's door is the example to decode), and how a patch could redirect one behind a flag;
- **which story scripts warp Lan into Higsby's**, which must keep working;
- BN4's real per-map NPC limit (BN6: 16).

Propose the best candidate room, with evidence. Screenshots go outside the repo; describe the room in words.

**b) The outside trash can.** The owner placed the added JunkMan trash can **just outside Higsby's**, on the ACDC Town sidewalk by the entrance (front right, under the awning), **not inside**. Round 4 proposed a talkable trash-can NPC inside Higsby's; redo it for outside:
- which map that sidewalk is;
- a free spot in map coordinates;
- whether a talkable "object NPC" with a trash-can sprite exists to reuse (the examine-only trash cans in Dex's Room `(00, 04)` and Elec Town 1 `(01, 00)` show what graphics exist);
- how it warps into JunkMan's cyber spot (`(8E, 00)`, `(8E, 02)` or `(88, 00)`, round 3).

## Task 2: run the patch tests (`decomp/notes/patch-test-results.md`)

Build the current patch (`python patch/build.py`) and run the procedures from your `patch-test-plan.md`. Record pass or fail with evidence (RAM values and screenshots, kept outside the repo):

- **WP-D4 star, re-run after the fix:** 6 Souls → no Souls star; 12 Souls → star. If no save has 12, set the flags in a save copy outside the repo.
- **WP-A2:** a battle's Zenny is × 1.5, and the shown and granted amounts match.
- **WP-D3:** after a Bass V1 win (or BassΩ at Busting Level 7+), both chips are in the pack and the Library.
- **WP-D2:** over many battles against one virus family with a version-specific drop, both codes appear.

Where a full run isn't practical, set the battle up through a save state or RAM edits, and say exactly what was and wasn't covered.

## Task 3: the status screen's two-page Soul view, as a design (`decomp/notes/status-screen-two-pages.md`)

Round 4 showed that 2 × 6 isn't safe (the NAVIGATOR box starts at Y = 94). So a 12-Soul status screen means **two pages of 6** in the existing layout. Write the exact design a patch needs:
- where the status screen reads input, and which button switches pages (L/R or Select; check what's free on that screen);
- where the 6-icon fill happens (`StatusCollectOwnedSouls` `0x08034A30`, the 0xFF-terminated list `BN4_STATUS_SOUL_FLAGS`, UI object `0x45`), and how to re-run it for page 2;
- how to mark which page is shown;
- the free-space and code size estimate, the routines to hook, and their addresses in RS and BM.

Claude writes the patch from your design.
