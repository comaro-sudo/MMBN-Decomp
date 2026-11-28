# Smoke test: notes on the two newer checks

The main description of the smoke test is the "Smoke test" section of `patch/README.md` and the docstring of `smoke.py`. This file records how the `tuner` (Blue Moon) and `mystery-data` checks put the player where the check needs him.

Run from `MMBN4-Decomp/`: `python patch/test/smoke/smoke.py` (all checks), `-k tuner`, `-k mystery-data`, `--list`.

## Editing a save to move Lan

`smoke.py` edits the game state of a community save (`ref/saves`, gitignored) before each run. The bytes that matter (offsets in the game-state block, `docs/re/save-and-flags.md`):

| Offset | Meaning |
|---|---|
| `+4`, `+5` | map group and map id |
| `+0x34` | map handler pointer (differs between the real world and the net) |
| `+0x38`, `+0x3C` | Lan's X and Y, signed 16.16 fixed point, in the same units as the NPC and Mystery Data positions in `patch/src` |

Changing only `+4`/`+5` loads the new map but keeps the old X/Y and the old handler.

## `tuner`, now in Red Sun and Blue Moon

The Blue Moon "souls" save (`4891`) was saved in the net (map `8D,01`). Moving it to Higsby's `(00,06)` by changing `+4`/`+5` loaded the room, but Lan still had his net X/Y, so he stood far outside the room: the screenshot showed the background colour, Lan and the corner of a wall (an "empty room"). The vanilla ROM does the same, so the patch is not involved. The Red Sun "souls" save was saved in ACDC Town at (-22.25, 26), a spot that is also inside Higsby's, so it worked by luck.

Fix: the check now also sets Lan's X/Y to (-22.25, 26) and `+2` to 0 for both versions, so the same walk (left, then down to the Tuner) works in both. Result: RS and BM both go Auto to Tier 1.

## `mystery-data`

For Z-Saver (Undernet 1, map `94,00`, MD at (-282, 56), flag `0x0E04`) and GP Power (ACDC Area 2, map `90,01`, MD at (106, 326), flag `0x0D0B`), in each version:

1. Take a net save (Red Sun: the "early" save `4440`, which was in the net; Blue Moon: the "souls" save `4891`). The Red Sun "souls" save was in ACDC Town, and a net map edited into it does not work (wrong handler at `+0x34`).
2. Set the map and put Lan 30 units left of the Mystery Data.
3. Walk right for 42 frames (frames 1300-1342), then press A 8 times, 25 frames apart. "MegaMan accessed the mystery data" appears and the text boxes close.
4. Check the taken flag: set in the patched ROM, clear in the patched ROM without the walk, clear in the vanilla ROM (where the Mystery Data does not exist).

It does not check that the chip reaches the pack (the pack layout is not decoded here). The inputs are frame-timed like the other checks.
