# Three tasks for Gemini, round 2 (assigned by the owner, written by Claude, 2026-11-23)

Great work on round 1 (tables carved, RE-04 remainder, save slots). **Claude is now working in `decomp/`**: carving the next tier of pointer tables (`BN4_BATTLE_TABLES` and the `code_04`–`code_06` tables) and running full regenerations. So these three tasks are **read-only analysis**:

- write only the new files named below, plus your handoff;
- **don't edit** `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile` or the existing `decomp/tools/*.py`, and don't edit `docs/`, `patch/` or `TODO.md`;
- **don't run `make`**, and don't write to `decomp/build/` or `build/`;
- read asm from git (`git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`);
- put proposed names in a section at the end of each note, in the `names.txt` format (Claude merges them);
- stage only your own files by path, and don't push.

All three support owner decisions recorded in `docs/completion/bosses.md` §3 and `plan/DEFINITIVE-EDITION.md` §7.

## Task 1: Bass rematch feasibility (`decomp/notes/bass-rematch.md`)

The owner wants all four Bass chips (Bass, BassAnly, BlakBarr, DeltaRay) obtainable in one save. The order is:
1. a repeatable fight;
2. otherwise the battle drops both chips;
3. otherwise a purple Mystery Data with the missing chip.

Find out statically:
- **Bass V1 (story battle id 176):** what script and flags start it, and whether anything lets it happen again after the win.
- **BassΩ (Black Earth 2 wandering table `(94,07) +0x10`, flag `0x009F`):** what sets and clears flag `0x9F`, and does a win stop him from appearing again? Same for **BassXX** (the `+0x20` story-override table).
- **"Drop both":** the exact path from the battle result to the reward (`BN4_REWARD_PICK` → `RewardDecode` → the chip-add routine). Where could a patch grant a second chip, and what does the results screen show?
- **The fallback:** how a new Mystery Data placement in Black Earth 2 would be added. Which free flags in `0x0D00`–`0x10FF`? Can a placement be shown only after a flag, and what would a "chip you lack" content hook need? Also give the item id of the Unlocker SubChip.

## Task 2: the story's Soul grant, and BM's Soul keys (`decomp/notes/soul-grant.md`)

Open since RE-05 (`TODO.md`): find where the story grants a Soul (flag + pseudo-chip key) after a tournament or story win, in both versions, and where Blue Moon keeps its Soul pseudo-chip keys. Leads:
- `docs/re/save-and-flags.md` (Soul flags) and `docs/completion/souls.md`;
- `ScriptCmd_checkDoubleSouls`;
- the community save `4429` (Red Sun with 1 Soul), in `ref/saves/` (local);
- `patch/src/common/wp_d4_twelve_souls.asm` (read-only).

Give the routine, its inputs, and what a patch would call to grant any of the 12 Souls in either ROM.

## Task 3: which shop is which, and the Number Trader (`decomp/notes/shops-in-the-world.md`)

`docs/re/shops.md` decodes 17 stock lists, but not which list belongs to which shop in the world.

- Find every `startShop nn` script site (`FB 05 nn`, handler `ScriptCmd_startShop`, RS `0x0805025C`) and map each to its map (group/number) and NPC. Use your Mystery Data and encounter map tables for the area names.
- Then decode the **Number Trader**: the code entry, the code table and the rewards. `sub_0804C680`, which restocks shop items, is one lead. List every code and its reward.
