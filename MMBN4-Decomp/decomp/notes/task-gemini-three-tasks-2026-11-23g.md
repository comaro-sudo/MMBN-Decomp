# Three tasks for Gemini, round 7 (assigned by the owner, written by Claude, 2026-11-23)

Round 6 is excellent: placements, WP-B1 and the Soul audit are all used now. **Claude is now writing WP-B1 (fixed brackets) and the friendly matches in `patch/`.** So:
- **don't edit** `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `docs/`, `patch/` or `TODO.md`;
- **don't run `make`** and don't write to `decomp/build/`;
- you may run `python patch/build.py` and the PC port, with outputs **outside the repo** (no images in the repo);
- write only the new notes named below, your scripts in your own folder, and your handoff;
- stage only your own files by path, and don't push.

## Task 1: test the three newest packages in-game (`decomp/notes/patch-test-results-2.md`)

Build the current patch and test, with evidence (RAM and screenshots kept outside the repo):

1. **`wp_d4_status_two_pages.asm`, the status screen:**
   - with save `22565` (RS, 11 Souls): open the MegaMan status screen; page 1 shows 6 Souls; **L or R** switches to page 2, which shows the other 5 with one empty slot; it switches back again; the cursor sound plays;
   - with a ≤6-Soul save (`4752`): L/R does nothing;
   - check that nothing else on the screen breaks: the NAVIGATOR menu, the panels, closing the screen.
2. **`wp_d2_duo_chip_reward.asm`, Duo:** do the Duo battles (Duoβ `0xE2`, DuoΩ `0xE3`; story battles 93/94, RE-06) reach the results screen and grant the reward? If a win gives the Duo chip (`0x137`, code D), check the pack and the Library flag `0x16C0 + 0x137`. If the final boss skips the results screen, say exactly what happens (credits? no reward?).
3. **`wp_d0_npc_lists.asm`, the relocated NPC lists:** enter Higsby's `(00, 06)` and walk around ACDC Town `(00, 00)`. All the original NPCs must appear, in the same places, and talk normally, in both versions.

## Task 2: the two special-chip Blue Mystery Data, as an exact patch design (`decomp/notes/special-chip-md-design.md`)

From your `placements.md`:
- **Grand Prix Power** (`0x138`, code G) at ACDC Area 2 `(90, 01)` `(106, 326, 0)`, flag `0x0D0B`;
- **Z-Saver** (`0x0DA`, code Z) at Undernet 1 `(94, 00)` `(-282, 56, 0)`, flag `0x0E04`.

Give everything Claude needs to write the patch, with RS and BM addresses:
- the placement-table format and where each area's list lives (RE-07), and whether the new entries fit in place or the list must move to free space (`0x087FA0A0+`);
- the content-entry format for a chip with a code. Every content slot must hold the chip, so New Game+ advancing changes nothing;
- the map-object spawn-script bytes each area needs, like the 11-byte `03 08 25 08 1C 29 <flag> 45 <flag>` sequence you found for Black Earth 2, and where those scripts are;
- the pickup text (does the generic Mystery Data pickup script handle a chip automatically?).

## Task 3: the lobby NPCs (operators, JunkMan, the trash can), as an exact patch design (`decomp/notes/lobby-npc-scripts-design.md`)

The owner's design is in `docs/pending/free-tournament-secret-chips.md`, with your positions from `placements.md`:
- 9 operators in Higsby's and 2 on the street;
- each appears only after Lan has met that Navi;
- talking to one starts a rematch, whose tier climbs V1 → V2 → V3 → SP with S-rank wins;
- JunkMan waits in a small cyber map, reached through a trash can outside Higsby's.

Design the NPC side:
- **The sprites:** for each of the 11 operators (Mayl, Dex, Mr. Match, Higsby, Lily, Tamako, Sal, Raika, Raoul, Chaud, Shuko, as the owner's lineup requires; Higsby is already in his shop, so say how his rematch works), their index in sprite list 7. Also a **trash-can** object sprite (any list), and JunkMan's in list 6 (`0x5A`).
- **The NPC bytecode** for one operator: position, facing, sprite, a talk script, and the **flag gate** ("met that Navi"; which flags mean "met" for each Soul Navi?).
- **The talk script:** how a script can pick a battle by a tier counter (2 bits per Navi in free flags; which flags are free?), start it from a `BN4_STORY_BATTLES` record with the "no tier raise" bit, and, after an S-rank win, raise the counter. Which script commands exist for "battle result / Busting Level"?
- **Text:** the minimum text needed. Can existing lines be reused, or does new text need TextPet insertion? Describe the text format and the encoding work if it's needed.
- **The trash can and JunkMan:** the jack-in script (warp into `(8E, 00)`, `(8E, 02)` or `(88, 00)`, round 3) and the JunkMan NPC there.
