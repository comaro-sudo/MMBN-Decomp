# Three tasks for Gemini, round 4 (assigned by the owner, written by Claude, 2026-11-23)

Round 3 was very useful: the lobby, the battle data and the status screen are in `docs/pending/free-tournament-secret-chips.md`. **Claude is regenerating `decomp/asm/` (full runs, `decomp/build/`).** So:

- **Don't edit** `decomp/asm/`, `decomp/config/`, `decomp/symbols/*`, `decomp/Makefile`, the existing `decomp/tools/*.py`, `docs/`, `patch/` or `TODO.md`.
- **Don't run `make`** and don't write to `decomp/build/`.
- Read asm from git (`git show HEAD:MMBN4-Decomp/decomp/asm/<seg>.s`).
- Write only the new notes named below, your scripts in a folder of your own, and your handoff.
- Put proposed names in a section at the end of each note.
- Stage only your own files by path, and don't push.

## Task 1: find a trash can for JunkMan (`decomp/notes/junkman-trash-can.md`)

The owner's decision (`docs/pending/free-tournament-secret-chips.md`): JunkMan has no operator, so **Lan jacks into a trash can** and meets him in a small cyber-area.
- The first choice is an existing trash can in **Higsby's `(00, 06)` or a nearby map** (ACDC Town `(00, 00)` and its neighbours).
- **The fallback is to add a trash can to Higsby's**, not to use a PC.

Round 3's `lobby-npcs.md` covered jack-ins in general but **not the trash can**.

1. List every **jack-in-able object** on Higsby's map and the maps around it: what each object is (PC, TV, vending machine, trash can, …), its jack-in target map, and its flag.
   - Use `MapCheckJackIn` (`0x0802A2E8`) and the map's object or jack-in table.
   - BN4 has many object comps in group `0x8C`–`0x8E` (e.g. `VendMchComp`, `DogHousComp`, `CookComp`), so look for a **trash-can comp** or a trash-can object anywhere in the real world.
2. If a trash can exists in or near Higsby's: is it jack-in-able, and which cyber map does it lead to?
3. If not: what adding one to Higsby's takes. That means its map graphics or tile, its collision, the examine/jack-in hook, and which existing cyber map to send it to (round 3 suggested `(88, 00)` or `(8E, 00)` / `(8E, 02)`).

4. **Where the 11 operators fit** (the owner decided they stand **inside Higsby's and just outside the store**, on the street in front, split as space requires):
   - **BN4's real per-map NPC limit.** BN6's is 16 (`NUM_OVERWORLD_NPC_OBJECTS = 0x10` in `ext/bn6f/constants/constants.inc`). Find BN4's object-pool size, the table the NPC allocator uses (`NpcSpawnList` `0x08026748` → `sub_08026774`).
   - **The walkable floor** in Higsby's `(00, 06)`, and on the outside map in front of the store: which map that is (group/number), its current NPC count, and free spots near the door.
   - **A proposed split:** how many operators inside and how many outside, with coordinates.

## Task 2: can the status screen show 12 Souls in a 2 × 6 grid? (`decomp/notes/status-screen-2x6.md`)

Round 3 (`status-screen-souls.md`) found:
- the Soul icon sprite already has all 12 icons;
- the buffer `0x0201D000` has room;
- the 2 × 3 grid (X `28, 84`; Y `42, 60, 78`) can't fit 12;
- it proposed two pages with L/R, **or** a 2 × 6 grid if Y `96..148` and VRAM tiles `0x68..0x7C` are free.

The owner hasn't chosen yet, so settle the 2 × 6 question with evidence:
- What else the status screen draws below Y = 78, and in which BG or OBJ layer.
- Which OBJ VRAM tiles and OAM slots are in use while the screen is open.
- Whether `0x68..0x7C` and 6 more OAM entries are free.

If you can, confirm at runtime with the PC port: `build/pc-arm64/Release/mmbn4.exe <rom> --test N --input ... --shot X.bmp`, and `--dump` for EWRAM. Write screenshots and dumps to a folder **outside the repo** (e.g. your temp dir), never into `build/` or the repo. Use a late save from `ref/saves/` (local), converted with `python patch/tools/savetool.py --convert`, written outside the repo.

Conclude: 2 × 6 is safe / is not safe / is safe only if X is moved.

## Task 3: an in-game test plan for the four untested patch packages (`decomp/notes/patch-test-plan.md`)

`patch/README.md` lists the packages that are built but **not yet tested in-game**:
- WP-A2: battle Zenny × 1.5;
- WP-D3: a Bass battle gives both chips;
- WP-D4: the Souls completion star needs 12;
- WP-D2: version drops at random.

For each one, write a concrete test:
1. **Which save:** one of the 13 community saves in `ref/saves/` (`docs/re/community-saves.md`), where the player stands, and what's needed (e.g. post-game in Black Earth 2 for Bass; a virus family from `re06-encounters.md` §7 for WP-D2).
2. **The input script** for the PC port's `--test` mode (`frame=KEYS`), or a manual mGBA procedure if a script is impractical.
3. **What to read to decide pass or fail:** e.g. the pack count of chip 306 in save slot `+0x50` and Library flag `0x16C0 + 306` after a Bass V1 win; Zenny before and after a battle; for WP-D2, a tally over N battles against Mettaur showing both `Guard1 L` and `Guard1 V`.

You may **run** the tests:
- `python patch/build.py` writes the patched ROMs to `MMBN4-Decomp/build/`, which is fine to run but don't edit `patch/`;
- then the PC port with `--state-in` / `--dump`, writing outputs **outside the repo**.

Report what passed and what didn't.
