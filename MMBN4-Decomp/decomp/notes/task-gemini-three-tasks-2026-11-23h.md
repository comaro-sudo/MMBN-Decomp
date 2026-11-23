# Three tasks for Gemini, round 8 (assigned by the owner, written by Claude, 2026-11-23)

The patch sprint was excellent; Claude reviewed it: builds and `--check` pass, and the new flags (`0x0E04`, `0x0E40`–`0x0E59`) collide with no Mystery Data. The new work order is in `TODO.md`.

**Claude is now doing step 1** (merging names, then a full regeneration of `decomp/asm/`) **and step 3** (the Soul-grant routine in `patch/`). So this round is **research only**:
- **don't edit** anything in `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `patch/`, `docs/` or `TODO.md`;
- **don't run `make`** and don't write to `decomp/build/`;
- you may run `python patch/build.py` and the PC port, with outputs **outside the repo**;
- write only the new notes below, your scripts in your own folder, and your handoff;
- stage only your own files by path, and don't push.

For each task, give exact RS and BM values that Claude can drop into `patch/` as they are.

## Task 1: lobby fixes, as exact values (`decomp/notes/lobby-fixes.md`)

1. **Operator sprites:** the correct sprite-list-7 index for each of the 7 operators still on placeholders in `wp_d3_lobby_rematches.asm`: Mr. Match, Shuko, Lily, Tamako, Sal, Raika, Raoul. Your handoff says Chaud, Mayl, Dex and Higsby are already exact. Also a **trash-can sprite** (any list), the list and index.
   - Method: find each character's existing NPC in the maps where they appear in the story, and read the sprite opcode `0x17`.
   - Give the exact table edit for `wp_d3_lobby_rematches.asm`.
2. **The "met that Navi" gate:** the lobby uses flags `0x0001 + id` (id 0–11). Find out what flags `0x0001`–`0x000C` actually mean (who sets them, in which scripts). If they aren't "Lan has met this Navi", find the right flags for each of the 12 Soul Navis: the story fight, a tournament match, or the new friendly match, whichever comes first. Give the corrected list.
3. **JunkMan's spot:** JunkMan was put in `WtrGodComp1 (8E, 00)`, the first map of AquaMan's side story. Check whether that scenario's scripts, NPCs or flags clash with JunkMan's NPC or the warp. If they do, propose a safer small cyber map (round 3 suggested `(8E, 02)` or `(88, 00)` Lan's HP).

## Task 2: SP+ attack × 1.5 and speed × 1.25, the feasibility research (`decomp/notes/sp-plus-research.md`)

The owner's SP+ design is in `docs/pending/boss-double-hp.md`: 4,095 HP, attack × 1.5, speed × 1.25, for every boss, only in SP+ fights. HP is data. Attack and speed need code. Find:
- **Damage:** where damage from an enemy attack to MegaMan is computed or applied. Is there one common routine where a "× 1.5 if SP+" check could go? Give the address and the registers.
- **Speed:** how Navi AI timing works: per-action frame counters or wait states. Is there a common place (a timer decrement, an animation speed, an AI state runner) where "× 1.25 speed" (waits × 0.8) could be applied for the enemy side only?
- **How to mark an SP+ fight:** a battle-record bit, or a RAM flag set before the battle starts, readable by those hooks.
- **HP:** confirm that setting the enemy record's HP to 4,095 is enough, or whether the battle copies HP elsewhere.

Conclude: feasible as specified / feasible only partly (what exactly) / not feasible, with the reasons.

## Task 3: Duo and Grand Prix Power: their art and names from the JP ROM (`decomp/notes/duo-gpp-research.md`)

Both chips (`0x137` Duo, `0x138` Grand Prix Power) have blank names and art in the US ROMs, and the Duo fight skips the results screen (your sprint notes).
1. **The chip data:** how the US ROM's records for `0x137` / `0x138` differ from a normal chip (the chip table `0x080197EC`, its icon/image/palette pointers, the name table). What's blank or missing?
2. **The JP ROM** (`roms/bn4_rs_jp.gba`, `bn4_bm_jp.gba`, local): where its art (icon, image, palette) for these two chips is, and its format (LZ77? sizes?). Can it be copied at build time into US free space and repointed, the way `build.py` already copies the other version's drop table? Which offsets and sizes?
3. **The names:** how the US chip-name text is stored (TextPet, the name archive), and what adding two English names ("Duo", "GP Power" or similar) takes.
4. **Duo's grant:** since the Duo fight skips the results screen, find a hook point after the Duo win (the post-battle script or the story flag set after Duo) where a patch can grant the Duo chip (`ChipGive`, `0x0801913E`). Give RS and BM addresses.
