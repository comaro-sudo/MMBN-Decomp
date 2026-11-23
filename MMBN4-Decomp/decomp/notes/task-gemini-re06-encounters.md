# Task for Gemini: RE-06, decode the encounter tables and the enemy records

Assigned by the owner on 2026-11-22, written by Claude. Your RE-07 note (`re07-mystery-data.md`: 169 placements, 78/78 against the maps, the corrected flag range) was excellent. Claude is still regenerating `asm/` for DC-4 and will merge your RE-07 names after that. This task is read-only analysis and doesn't touch any of it.

## Why

Violet Eclipse must make every boss beatable and every drop obtainable in one save (criteria C2 and C3). The boss list in `docs/completion/` is preliminary, built from guides. The ROM's own tables would give:

- every random-encounter table per net area: which battles, how often, in which playthrough tier;
- every enemy record: HP, attack, element, and **drops by Busting Level**, the main source of chips and Zenny besides shops and Mystery Data;
- the Navi and boss battles (story, tournament, rematch, V2/V3/SP/Omega-style variants), and which version has which (Red Sun / Blue Moon).

## Leads (verify them; they're not proven)

- **Battle start:** script command `startRandomBattle` (`ScriptCmd_startRandomBattle`, RS `0x08050228`) and the other `start*Battle` handlers in `code_04` (`symbols/names.txt`, "Text-script command handlers").
- **Rewards:** the battle-reward decoder `REWARD_DECODE` and the AddZenny hook are documented in `docs/re/save-and-flags.md` §4b (the Phase 1 patch WP-A2 rewrites the decoder; see `patch/src/common/wp_a2_zenny_battle.asm` for its address and format notes). Start from there to find the reward tables.
- **Tournaments** (`docs/re/tournaments.md`): the Navi pools and the Soul Navi pairs are mapped. Their battle ids will appear in the battle tables.
- **Version differences:** your `dc4-version-differences.md` lists the version constants. Navi records and drop tables that differ by version probably show up as `differs` ranges in `datashift_rs_bm.txt`, or as code-region data in `code_02`/`code_03`.
- **Cross-reference:** `ext/bn6f` (the BN6 decomp, a descendant engine) names its battle-setting and enemy tables. Use them for the *logic*, as you did for Mystery Data, since BN4's layout may differ.

## What to produce

1. **`decomp/notes/re06-encounters.md`** (new):
   - the routines (RS and BM addresses) for picking a random encounter, setting up a battle, and computing rewards;
   - the table layouts, with RS and BM addresses;
   - **the encounter tables per area**, with odds and tiers;
   - **every enemy and Navi record**: id, name (from TextPet's name tables if you can, otherwise the id), HP, element, and drops per Busting Level;
   - the list of boss battles with their version (RS, BM or both) and how each is reached;
   - the RS/BM differences;
   - proposed names in the `names.txt` format, in a section at the end.
2. **`decomp/tools/encounters.py`** (new), the re-runnable extractor. It reads only the two ROMs and existing files.

## Constraints (Claude is working in parallel)

- **Create only the two new files above** plus your handoff. Don't edit or regenerate anything in `decomp/asm/`, `decomp/config/`, the existing `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py` (your own `vdiff.py`, `datashift.py` and `mysterydata.py` are yours), `TODO.md`, `docs/`, or `patch/`.
- **Don't run `make`** and don't write to `decomp/build/` or `build/`. Read asm from git (`git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`); the working tree's `asm/` is being rewritten.
- Never commit ROM bytes, extracted text or guide data. Tables in the note use ids, names and numbers, not raw dumps.
- Leave the files uncommitted, or commit only your own new files by explicit path. Then write `decomp/notes/handoff-<date>-<time>.md`.

## Done when

Every enemy and Navi record and every area's encounter table is decoded from the ROM, with drops by Busting Level. The boss list says which version has each boss. The RS/BM differences are listed.
