# Existing saves on the patched ROM (release checklist §3, "Existing saves")

Date: 2026-11-27. Tester: Claude (batch-4 agent, task 4). Tree: `a076ca6` (`claude-agent/d4` worktree), `python patch/build.py --public`.

**Result: all four requested checks PASS on five real vanilla saves (2 Red Sun, 3 Blue Moon), with one known failure carried over from `docs/pending/findings-2026-11-27.md` §3 (status page 3 is reachable, and blank, with 9 or 11 Souls).** Nothing crashed on load or on save.

## What was tested

| | |
|---|---|
| Patched ROMs | The `--public` builds (the shareable ones; the local-only Duo / GP Power art is left out). RS SHA-1 `8ccc929a05411169c4ece2f056474bcc727f13ed`, BM SHA-1 `a2c5295d86fdecf18003f6ccbe018982d9de3edb` (both `.bps` round-trip OK in the build). The private build was not run: it differs only by that art. |
| Emulator | The PC port (`build/pc-arm64/Release/mmbn4.exe`, headless `--test`, scripted input, EWRAM dumps and screenshots). Not mGBA, not hardware (tasks 5). |
| Control | The same save, loaded the same way in the **vanilla** US ROM, so every comparison is "patched vs vanilla". |
| Saves | Community saves `ref/saves/*.sps` (gitignored), converted to a 32 KB `.sav` with `patch/tools/savetool.py` (`read_sram`). They are vanilla saves: never touched by the patch before. Nothing was edited unless noted. |

Saves used (names are the `ref/saves/` file stems; summaries from `ref/saves/index.md`):

| Short name | File | Version | State |
|---|---|---|---|
| RS mid | `mega-man-battle-network-4.4440` | Red Sun | playthrough 1, 10:45, not cleared, 3 Souls (Roll, Guts, Search), HP Memories still in the shops |
| RS post | `mega-man-battle-network-4.22565` | Red Sun | playthrough 5, 56:50, cleared, 11 Souls (all but Aqua), 158,070Z |
| BM early | `mega-man-battle-network-4.4420` | Blue Moon | playthrough 1, 2:21, not cleared, 0 Souls. **Cheated** (HP 9999 over a 180 max) but loads and saves normally; it is the only BM save before the ending. |
| BM mid | `mega-man-battle-network-4.4569` | Blue Moon | playthrough 3, 36:37, cleared, 6 Souls (Proto, Number, Metal, Junk, Aqua, Wood) |
| BM post | `mega-man-battle-network-4.4891` | Blue Moon | playthrough 4, 99:59, cleared, 9 Souls (all but Guts, Wind, Thunder) |

No Blue Moon save is a clean "mid-game, not cleared" one. `4420` is the only uncleared BM save and it is an early, cheated one; `4569` stands in for the middle. `mega-man-battle-network-4.5779` (BM) was **tried and set aside**: it was saved inside the Chip Trader screen, so even the vanilla ROM changes event flags on load and the Save menu never opens (an artefact of the save, not of the patch). No Duo-owning save exists (real Duo Soul needs the patched game), so a Duo case would need a save edit and was not done.

## Method (per save, vanilla and patched ROM, same inputs)

Boot with `START`, then `A` through Continue (the smoke test's `BOOT` script), run 1,300 frames, dump EWRAM. The save block is at `0x02002130 + shift`, the shop stock at block `+0x164C`, event flags at `+0x118`.

1. **Load:** compare the live save block (`+0x0000..+0x2000`, shops excluded) between the vanilla and the patched run; compare the live event flags and map with the file's.
2. **Shop prices:** all 128 shop slots, patched vs vanilla. Expected: HP Memory (item 160) = exactly 40% of vanilla; Unlocker (181) = 16 (1,600Z); LocEnemy (180) = 40 (4,000Z); Zenny NaviCust in shops 14-15 = the patched ROM's price for that program; every other slot, and every field except the price, unchanged.
3. **Souls and status pages:** the Souls owned (flags `0x17`-`0x22`) from the file; the MegaMan status screen (Menu, 4th entry) with R, R+R, R+L, R+R+R; the icon columns compared as pixels, and the screenshots looked at.
4. **Lobby operators:** the save's map set to Higsby's `(00,06)`, ACDC Town `(00,00)` and WtrGodComp1 `(8E,00)` (the smoke test's map edit, a 2-byte save edit for this check only). Every operator script (`wp_d3_lobby_rematches.asm`, `0x08 0x17 <spr> 0x05 <flag>` and the trash can's `0x25`) that the game parks as hidden leaves its ROM address in EWRAM, so the hidden set is read from the dump; expected hidden = the operators whose Soul flag (`0x17`-`0x22`) the file does not have. Control: with all 12 Soul flags set (a save edit, RS post) no script is hidden, and BM early (0 Souls) hides all of them. Screenshots of Higsby's confirmed the operators draw (RS post, 8 visible).
5. **Save and reload:** Menu, Save (8th entry), Yes, "Save complete!", then A and B back to the field. Then: the `.sav` decoded with `savetool.decode` (checksum valid, same version), its event flags equal to the original's, a diff of the decoded save block (what changed), and the saved file loaded again in the patched ROM (prices, flags) and in the **vanilla** ROM (flags).

## Results

### 1. Load: no crash, nothing changed but the prices - PASS

| Save | Map on load (file / vanilla / patched) | Live event flags = file's | Save block, patched vs vanilla (outside shops) |
|---|---|---|---|
| RS mid | (82,00) all three | yes, both ROMs | 3 bytes at `+0x34..+0x36` differ (a frame/time counter: the patched ROM boots a few frames apart) |
| RS post | (00,02) | yes | identical |
| BM early | (90,00) | yes | identical |
| BM mid | (94,06) | yes | identical |
| BM post | (8D,01) | yes | identical |

Screenshots after load show the right place and a normal HUD (HP 396, 1,250, 9,999, 839, 1,455 as in the save). No black screen, no freeze.

### 2. Shop prices synced to 40% - PASS (5 of 5)

Every slot matched the rules above; **0 slots wrong, 0 unexpected price changes** in all five saves. Slots synced on load:

| Save | Slots changed | HP Memory lines (vanilla Z -> patched Z) | Zenny programs (shops 14-15) |
|---|---|---|---|
| RS mid | 20 | shop 2: 10,000 -> 4,000, 15,000 -> 6,000, 20,000 -> 8,000, 30,000 -> 12,000; shop 3: 30,000 -> 12,000, 50,000 -> 20,000 | 6 lines (e.g. 9,600 -> 3,800, 13,000 -> 5,200) |
| RS post | 11 | none left in the shops | 3 lines (8,000 -> 3,200 twice, 9,600 -> 3,800) plus 8 Unlocker / LocEnemy lines |
| BM early | 31 | 11 lines (shops 0-3, e.g. 6,000 -> 2,400, 50,000 -> 20,000) | 12 lines |
| BM mid | 23 | 4 lines (shop 2) | 11 lines |
| BM post | 8 | none left | none (Unlocker / LocEnemy lines only) |

Each HP Memory line is exactly 40% (`patched * 5 == vanilla * 2`). After Save, the new `.sav` holds the 40% prices (the price bytes are the only shop bytes that differ from the original), and loading that `.sav` again in the patched ROM gives the same prices (the sync is idempotent).

### 3. Souls and status pages - PASS for Souls and pages 1-2, one known FAIL (page 3)

- **Souls:** the live flags `0x17`-`0x22` equal the file's in every save (11 / 3 / 0 / 6 / 9 Souls). The patch adds or removes no Soul.
- **Page 1 and 2:** with 7 or more Souls, R shows page 2 (RS post: page 1 has 6 icons, page 2 has the other 5; BM post: 6 and 3), R then L returns to page 1 (pixel-identical), three R presses wrap to page 1. With 3, 5 or 0 Souls (RS mid, BM mid at 6, BM early) R does nothing, as designed (pages appear at 7+ Souls); the icons match the Souls owned.
- **Page 3 (known issue, FAIL):** with **11 Souls (RS post) and 9 Souls (BM post)**, neither of which owns Duo, R from page 2 goes to a **third, empty page** (no icons) before wrapping. The design says page 3 exists only with a 13th Soul. This reproduces `docs/pending/findings-2026-11-27.md` §3 on real saves with no edit; it is still open (task 2 owns the cause). It is cosmetic: no crash and no wrong data. Not reproduced: the "page 3 blank with a save-edited Duo Soul" half (no Duo save was used).

### 4. Lobby operators appear per owned Soul - PASS (5 of 5, all 3 rooms)

Visible operators = the Souls the save owns (the gate is the Soul flag). Observed (read from the hidden-script dump) equals expected in every room of every save:

| Save | Souls | Higsby's (9 operators) | ACDC street (Raika, Raoul + trash can) | WtrGodComp1 (JunkMan) |
|---|---|---|---|---|
| RS mid | 3 | 2 visible | 1 (Raika) | hidden |
| RS post | 11 | 8 | 3 (both operators and the trash can) | visible |
| BM early | 0 | 0 | 0 | hidden |
| BM mid | 6 | 5 | 1 (Raika) | visible |
| BM post | 9 | 7 | 2 (Raika, trash can; no Raoul) | visible |

The three-room check also proves the operators work on saves that never had the patch: the old "met" flags `0x0001`-`0x000C` are not used. Visually confirmed in Higsby's for RS post (operators drawn with the right sprites). Not done in-game: talking to an operator and the rematch itself (covered by the package's own PASS in `patch/README.md`).

### 5. Saving - PASS (5 of 5)

Menu, Save, Yes gave "Save complete!" in every save, and A / B returned to the field with the game running (EWRAM keeps changing, HP shown correctly). The written `.sav` decodes with a **valid checksum** and the right version. Its event flags equal the original save's, and the changed bytes of the decoded block are only: the progress/time counters (`+0xB0..+0xBA`, play time and checksum area) and the shop prices (`+0x16D2..` etc.). The saved `.sav` loads again in the patched ROM (prices equal to the first load, flags equal) and **in the vanilla ROM** (flags equal): a save made on the patched game is not damaged, though the vanilla game would show the lowered prices.

## Not covered

- mGBA and real hardware (task 5); the private (non-`--public`) build; an RS or BM save with a real Duo Soul (13 Souls) or with Duo / Grand Prix Power in the folder.
- A save from the patched game that went through a story event (a friendly match) - that is the continuous playthrough item.
- The shop screen itself (buying at the new prices); only the saved stock data was compared. The smoke test's `shop-migrate` check also reads the RAM only.
- Playing on after load for more than a few seconds (a second save slot, battles, the whole post-game).
- The "hidden-script address" oracle for the operators is indirect; it agreed with the Soul flags in all 15 room checks and with the screenshots that were looked at (Higsby's RS post), and its control (all 12 flags set: nothing hidden) was run once on RS post; BM early (0 Souls) hides every script.

## Reproducing

Run from `MMBN4-Decomp/`: `python patch/build.py --public`, then drive `build/pc-arm64/Release/mmbn4.exe <rom> --save <sav> --test <frames> --input "<script>" --dump <ram> --shot <bmp>` with the saves converted by `patch/tools/savetool.py` (the helpers in `patch/test/smoke/smoke.py`: `Smoke.save`, `Smoke.run`, `block`, `flag`, `BOOT`, `STATUS_MENU`, `bmp_region`). The scripts used here were throwaway (kept outside the repo); the save/shop/Soul/operator checks could become `smoke.py` checks later.
