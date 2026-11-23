# Three tasks for Gemini, round 3 (assigned by the owner, written by Claude, 2026-11-23)

Thanks for the canon-Navi research and the dialogue drafts; the lineup is now in `docs/pending/free-tournament-secret-chips.md`.

**Claude keeps working in `decomp/` (tables, regenerations) and in `patch/`**, so all three tasks are **read-only analysis**:
- write only the new note named in each task, plus your handoff;
- don't edit `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `docs/`, `patch/` or `TODO.md`;
- don't run `make`, and don't write to `decomp/build/` or `build/`;
- read asm from git (`git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`);
- put proposed names in a section at the end of each note (`names.txt` format);
- stage only your own files by path, and don't push.

All three feed the owner's lobby-rematch design in `docs/pending/free-tournament-secret-chips.md`:
- the 12 Soul Navis' operators stand in the Free Tournament lobby, and can be challenged again and again;
- each Navi climbs V1 → V2 → V3 → SP with S-rank wins;
- each operator appears after Lan has met that Navi;
- JunkMan has no operator, so he waits in the lobby's network: Lan jacks in at a lobby PC.

## Task 1: the lobby, its NPCs and a jack-in PC (`decomp/notes/lobby-npcs.md`)

- Find the **Free Tournament lobby map**: where the Free Tournament is entered (`ScriptCmd_callFreeTournament`, RS `0x08050500`), which real-world map that is (group/number), and its NPC list.
- Decode the **NPC placement format** for that map: position, facing, sprite (list and index), the script run on talk, and the flag that gates whether an NPC appears.
  - The overworld sprites are in sprite list 6 (list table RS `0x0802793C` / BM `0x08027940`; Cyberworld Endless `ext/.../src/emu/bn4_navis.h` `BN4_OW_*`).
  - The operators (people) are probably in another list; find which.
- **The owner's idea for JunkMan: Lan jacks into a trash can.** Is there a **trash can** (or dumpster, bin, junk pile) in the lobby or on a map next to it?
  - If there is, can it be jacked into already? List every jack-in object in and near the lobby.
  - If no trash can is jack-in-able, say whether an existing trash-can object can be made one, or what **adding a new trash can** to the lobby takes. **The owner's fallback is to add a trash can, not to use a PC.** So also find a trash-can map object, with its graphics and collision, that can be placed in the lobby.
  - BN4 may already have a trash-can comp elsewhere (look through the comp maps in groups `0x8C`–`0x8E`: `re06-encounters.md` §4, `re07-mystery-data.md`), which would give a ready-made small cyber map.
- Find how a map gets a **jack-in** object in general, and which **small cyber map** could be reused for JunkMan's spot (one NPC, one exit), preferably a trash-can comp.
- Say what a patch must add for 12 new NPCs (11 operators + JunkMan in the net). Can the map's NPC list be extended in place, or must it be relocated (to free space at `0x087F7420+`)?

## Task 2: what the Free Tournament battle reads for a traded Navi (`decomp/notes/free-tournament-battle-data.md`)

This is the open check in the pending doc: does the battle against a traded Navi build the opponent from the **Navi id**, as the story tournaments do? Or does it read the **traded stats and AI blocks** in save slots `+0x78` / `+0x7C` (`WaitingRoomReceiveTradedNavi`, `0x0803FA90`)?

- Follow `FreeTournamentPickMatch` (`0x0804498C`) to the battle setup, and find which routine reads `+0x78` / `+0x7C` and when.
- Compare it with a story tournament battle against the same Navi (`BN4_STORY_BATTLES`, RE-06).
- Conclude what a lobby-rematch patch needs:
  - just a battle record with the "no tier raise" bit (`+6` bit 7);
  - or filled stats and AI blocks too.

## Task 3: the status screen's Soul list (`decomp/notes/status-screen-souls.md`)

Claude's patch `patch/src/common/wp_d4_star_twelve_souls.asm` already makes the completion star check all 12 Souls. The status screen still lists only the version's 6 (`BN4_STATUS_SOUL_FLAGS`, RS `0x08034A78` / BM `0x08034A7C`, read by `StatusCollectOwnedSouls` `0x08034A30`, drawn by `sub_08053F8C`).

- Find how many Soul icons the status screen can draw: buffer `0x0201D000`, the tile layout, and any fixed count of 6.
- Say exactly what a patch must change to show up to 12, and whether it fits the existing screen or needs a second row or page.
