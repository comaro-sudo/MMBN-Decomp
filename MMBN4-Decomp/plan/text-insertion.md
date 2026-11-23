# Plan: new text in the ROM (TextPet pipeline)

**Status: plan (Claude, 2026-11-23).** Needed by work-order step 4 (the 9 friendly-match dialogues, `decomp/notes/canon-navis.md` §7) and step 6 (the Duo and Grand Prix Power chip names). Later, also any new NPC or BBS text.

## Constraints

- **No game text is committed.** The repo may hold only **our own new lines** and the **recipe** for inserting them. The original scripts are extracted from the player's ROM at build time.
  - `.gitignore` allows edited TextPet dumps under `patch/src/scripts/`, but a full edited dump still holds Capcom's text. Avoid that: commit only new or changed *messages*, as small source files.
- Both versions: text archives sit at different addresses in RS and BM (Gemini's `datashift_rs_bm.txt`). The pipeline runs once per version.
- TextPet is local (`tools/textpet/`: `TextPet.exe`, `plugins/` with BN4's command set, `bn4-utf8.tbl`). It reads and writes BN4's compressed text archives.

## How BN4 text works (known so far)

- Dialogue lives in **text archives**, many of them LZ77-compressed (header `0x10`), pointed to from per-map and per-system tables. Examples: the shop dialogue archives (`docs/re/shops.md`), and the map dialogue tables `dat_080523E0` / `dat_080523F8` (Gemini, `lobby-npcs.md`).
- A script inside an archive mixes text with commands (`E5` end, `E6` keyWait, `E7` msgOpen, `FB`/`FC`/`FE` groups…). The 126 handlers are named (`ScriptCmd_*`). The patch's own commands are `FC 09`–`0C` (lobby, Frequency Tuner, grant Soul).
- Gemini's lobby patch already builds a few small dialogues by hand: `lobby_open_dialogue` and its `OpenMapDialogue` hook.

## The pipeline

1. **Source files** (committed): `patch/src/text/<name>.txt`, one per new or changed dialogue, in TextPet's script syntax. They contain only our lines and commands. Example: the AquaMan-tournament friendly match with Dex, ending in `FC 0C 01` (grant GutsSoul) after a win.
2. **Build step** (`build.py`, new):
   1. extract the needed archives from the player's ROM to `build/text/` (gitignored), with TextPet;
   2. **new archives:** compile our `.txt` scripts into new archives with TextPet;
   3. **changed archives** (when an existing script must call a new one): apply our small edit to the extracted script, then recompile;
   4. recompress (LZ77) and write to `build/text/*.bin`.
3. **Assembly:** `main.asm` `.incbin`s each built archive into a free-space slot (allocation table in `defines.asm`) and repoints the table entry that names it, just as the lobby and Mystery Data packages repoint lists.
4. **Check:** decompress the inserted archive back with TextPet and diff against the source; then the PC-port test (does the dialogue show and run its commands?).

## Size

Free space: about 35 KiB of the tail is left after the current packages (next free slot `0x087FAA70`). Nine short dialogues are a few KiB compressed. If later text work outgrows it, the ROM can be expanded to 16 MiB (`docs/re/free-space.md` §4).

## Open questions (research, Gemini or Claude)

1. TextPet's exact CLI for extract, insert and compile with the BN4 plugin, and whether it can emit a standalone archive for `.incbin`.
2. How a tournament cutscene calls a dialogue archive. The friendly matches must hook into the tournament-day cutscenes (`wp-b1-design.md`: cutscene opcodes `0x51` battle, `0x52`/`0x53` tournament).
3. How the chip-name table is stored (for "Duo" and "GP Power"), and whether it is a TextPet archive too (Gemini round 8, task 3).
4. The font: does BN4's US font table (`bn4-utf8.tbl`) cover every character we need? Plain English should be fine.
