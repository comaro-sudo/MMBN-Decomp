# Gemini round 9 (assigned by the owner, written by Claude, 2026-11-23): Duo Soul + Duo rematch research

Round 8 was excellent. Claude applied your lobby fixes (`11d2a39`), wired TextPet into the build, and wrote the chip-art converter. **This round is research only**, as before:
- **don't edit** `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `patch/`, `docs/` or `TODO.md`;
- **don't run `make`** and don't write to `decomp/build/`;
- you may run the PC port and `python patch/build.py`, with outputs **outside the repo**;
- write only the notes below, your scripts in your own folder, and your handoff;
- stage only your own files by path, and don't push.

## Task 1: the Duo Soul framework (`decomp/notes/duo-soul-research.md`)

The owner wants to add the **Duo Soul**, which was planned for BN4 but cut for space ("the framework for the soul still exists", wiki). The plan is in `docs/pending/duo-soul.md`. Find, in both RS and BM:

1. **The pseudo-chip `0x16D` (`DuoSoul`):** its full record (chip table `0x080197EC`, `0x2C` bytes each: picture, icon and palette pointers, name, flags). Is it complete, or blank or RAM-backed like the Duo chip?
2. **Soul tables:**
   - `BN4_SOUL_FLAGS` (`0x08020018`) and the Soul kinds (`0x08020008`);
   - the unite check (`BN4_UNITE_CHECK` `0x0801FF84`), and any table indexed by Soul id: the unite chip class/element per Soul, the form id, the charge shot, the passive.

   How many entries each has: exactly 12, or room for a 13th, or a leftover Duo entry? Is flag `0x0023` free?
3. **The form:**
   - MegaMan's Soul-form sprite and palette tables (how a Soul form picks its battle sprite and palette): is there a Duo form?
   - If not, what's the closest existing form to recolour?
   - Are Duo's own battle sprites (enemy `0xE0`) usable for effects?
4. **The abilities:** where the per-Soul charge shot and passive effects are dispatched, and whether a 13th id would fall through safely or crash.
5. **The UI:** where the Custom Screen lists unite-able Souls (and its limit), and every other place that assumes 12 Souls.
6. **Conclusion:** what exists, what's missing, and the work estimate for each scope:
   - **(a)** a full Duo Soul with new art;
   - **(b)** a palette-swap of an existing form, with Duo-themed abilities;
   - **(c)** not feasible.

## Task 2: the Duo rematch in the Meteor Comp (`decomp/notes/duo-rematch-research.md`)

Owner decisions (`docs/completion/bosses.md`, "Duo rematches"):
- after the ending, the player can fight Duo again **back in the Meteor Comp**;
- each S-rank win makes the next rematch the next version (Duo `0xE0` → Duoα `0xE1` → Duoβ `0xE2` → DuoΩ `0xE3`, then SP+);
- the rematch **replays the final battle** with that version, started as a **plain battle** (as the lobby rematches do) so the ending and credits don't replay, and so it reaches the results screen.

Find:
1. **The Meteor Comp map** where Duo is fought (group/map), and whether it's still reachable in the post-game. If it isn't, which Meteor Comp map is.
2. **The post-game flag:** Duo-defeated `0x006F` (your round-8 note) or the ending/GameClear flag. Which one is set after the credits?
3. **The final-battle records:** `BN4_STORY_BATTLES` 93 and 94 (RE-06) and their `+6` flags. Can a plain battle start (`START_OW_BATTLE`, as in `wp_d3_lobby_rematches.asm`) use them with any of the 4 Duo ids, with no story script attached? What does the record's mode byte do?
4. **A spot for a Duo NPC or object** in that map (NPC list, free slots, coordinates), with an existing Duo or Meteor sprite (list 6/7) to stand as "Duo's signal".
5. **Free flags** for Duo's 2-bit tier counter (after `0x0E59`; e.g. `0x0E5A`–`0x0E5B`), checked against the Mystery Data flags.
