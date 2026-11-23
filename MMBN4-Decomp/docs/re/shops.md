# RE-09: shops (2026-11-22)

**V** = read in the code (RS addresses; `code_03` is +8 in BM, `code_00` +0). **R** = checked against the ROM tables. **T** = tentative.

## Summary

- Every shop's stock lives in the **save file** (toolkit slot `+0x58`, save offset `+0x164C`, `0xDC4` bytes). It's copied there from one ROM table, `BN4_SHOP_STOCK_INIT`, when a new game starts. **V**
- Prices are u16 in units of **100 Zenny**. BugFrag shops (record kind 1) use the value as is. **V**
- Consequence for patches (WP-A3 key-item prices): editing the ROM table changes prices for **new games only**. An existing save keeps the stock it copied. A patch that must reach old saves also has to rewrite slot `+0x58` on load, or rewrite the prices in the buy and display routines.

## How a shop opens (V)

1. Script command `startShop` (`FB 05 nn`, handler `ScriptCmd_startShop`, RS `0x0805025C`) calls `ShopOpen` (`0x08005858`) with `nn` = shop index.
2. `ShopOpen` calls `ShopInit` (`0x08049DB4`). It saves the map place in the game state (`+0x38`–`+0x44`), sets game-state mode `0x24`, and starts the transition (`sub_08005AFC`).
3. `ShopInit`:
   - zeroes the shop state at `0x0200A740` (`0x34` bytes) and stores the index at `+4`;
   - stores the shop's record pointer at `+0x1C` (`BN4_SHOP_RECORDS + n × 0x10`);
   - stores a work buffer (`0x02026000`) at `+0x24`.
4. The shop runs as a state machine: `sub_08049DEC` dispatches on `[0x0200A740]` through the table at `0x08049E14`.
5. The first state, `ShopStateOpen` (`0x08049E40`):
   - calls `ShopBuildChipOrderList(index, 0x0201A000, 0x0201B400)`, which only does something for record kind 2;
   - sets up the screen;
   - stores the money held at state `+0x28`: `GetBugFrags` if record kind is 1, otherwise `GetZenny`.

## Shop records: `BN4_SHOP_RECORDS`, RS `0x08049CA4` / BM `0x08049CAC`, 17 × 16 bytes (V, R)

| Offset | Meaning |
|---|---|
| `+0x00` | u32 kind: **0** = Zenny shop, **1** = BugFrag shop (shops 12–13), **2** = chip-order list (shop 16) |
| `+0x04` | ROM pointer to an LZ77-compressed text archive (header `0x10`): the shopkeeper's dialogue (T). The only field that differs between RS and BM |
| `+0x08` | u32 offset of the shop's stock inside save slot `+0x58`: `n × 0x40` for shops 0–15, `0x400` for shop 16 |
| `+0x0C` | u32 **slot count**: 8 for shops 0–15, `0x140` (320) for shop 16. (The first pass called this an event-flag base; that was wrong.) |

## Stock entries: 8 bytes (V, R)

| Offset | Size | Meaning |
|---|---|---|
| `+0` | u8 | Type: 0 = empty slot (ends the list), **1** = item (id 160 = HPMemory: its 14 lines total 207,700Z, matching the guide's 14 purchases for about 208,000Z; T: ids 176–181 are the SubChips), **2** = battle chip, **3** = NaviCust program (T) |
| `+1` | u8 | Quantity left. `0xFF` = unlimited (never decremented) |
| `+2` | u16 | Item, chip or program id |
| `+4` | u8 | Chip code (26 = `*`); `0xFF` for items; for programs, probably the color (T) |
| `+5` | u8 | 0 in the ROM table. `ShopBuildChipOrderList` writes 1 here for chips that pass its first flag test (T: "new" marker) |
| `+6` | u16 | **Price**: × 100 Zenny, or BugFrags as is in kind-1 shops |

Shops 0–5 sell HPMemory (type 1, id 160) several times, at rising prices: e.g. shop 0 sells 2500, 6000 and 10000.

Shop 5 sells chips at 8, 24, 32 and 76, so 800, 2400, 3200 and **7600** Zenny. These match the guide's ACDC Area 2 prices except the last, which the guide gives as 7700. That mismatch is why the first pass's search for the guide's prices failed. (T: shop 5 = ACDC Area 2.)

## The table: `BN4_SHOP_STOCK_INIT`, RS `0x0804ADA0` / BM `0x0804ADA8`, `0xDC0` bytes (V, R)

- Shops 0–15 are at `n × 0x40` (8 slots each). Shop 16 starts at `0x400`.
- RS and BM differ in **one entry**: shop 13 (a BugFrag shop), slot 5. RS sells chip 307 code 7 and BM sells chip 305 code 18, both for 100 BugFrags. These are version-exclusive chips.
- **Shop 16** is a list of 266 chips (ids 1–280, one entry each, prices × 100). `ShopBuildChipOrderList` shows a chip only if one of the event flags `0x16C0 + id`, `0x18C0 + id` or `0x1AC0 + id` is set. `0x16C0 + id` is "Library: chip obtained" ([save-and-flags.md](save-and-flags.md) §2). This looks like the in-game chip order service (T).
  - Its 320 slots reach past the end of the 0xDC0-byte copy: the last few slots overlap whatever follows in the save.

## The routines (V)

| RS | Name (proposed) | What it does |
|---|---|---|
| `0x080049A8` | `NewGameInit` | Clears the tournaments, all event flags and the flag log, calls `ShopStockInit`, and initializes about 20 other systems (T: also run for New Game+) |
| `0x0804BB60` | `ShopStockInit` | `CopyWords(BN4_SHOP_STOCK_INIT → [sl+0x58], 0xDC0)` |
| `0x0804BB34` | `ShopHasStock` | r0 = shop: 1 if any slot has a nonzero byte `+1`. Called by `ScriptCmd_checkShopStock` |
| `0x0804BC38` | `ShopGetStock` | r0 = shop: returns `[sl+0x58] + record+8` |
| `0x0804BC4C` | `ShopTakeStock` | Buying one: finds the slot matching (type, id, code), decrements its quantity unless `0xFF`, and when it reaches 0 shifts the following slots up. With `r4 = 2` it only decrements. Called from the buy states `sub_08049FF4` and `sub_0804A1DC` |
| `0x0804BBFC` | `ShopRestockItem` | r1 = id: +1 quantity (cap 255) for the slot with that id. Called by `sub_0804C680` |
| `0x0804BB78` | `ShopAddStockList` | Merges an 8-byte-entry list into a shop: adds quantities to matching slots, or fills empty slots. No direct `bl` callers found (T: called through a pointer) |
| `0x0804BD18` | `ShopBuildChipOrderList` | Kind-2 shops only: splits the stock into two lists (`0x0201A000`, `0x0201B400`) by the Library flags, ordered by the chip record's `+0x1C` field |
| `0x0804ACF4` | `ShopPriceToMoney` | r1 = price: × 100 unless record kind 1 (display) |
| `0x0804A04C` | (inside the buy state) | The same × 100 rule, then `money − price`, refusing when negative |

## Which shop is which (done by Gemini, [`decomp/notes/shops-in-the-world.md`](../../decomp/notes/shops-in-the-world.md), V)

| Records | Where |
|---|---|
| 0–3, 5 | Net shops: Town Area3, Netopia Area, Undernet1, Undernet4, ACDC Area2 |
| 4 and 16 | **Higsby's**: the chip shop (4) and the chip-order service (16) |
| 6–11 | SubChip dealers: Park Area1 (net); ACDC Town, Elec Town1, MelSquar, ColosseumAve, YumRuins (real world) |
| 12–13 | BugFrag traders: Park Area3, Undernet3 |
| 14–15 | NaviCust program dealers: Castillo HP, Undernet6 |

Items are confirmed (save slot `+0x54`, Gemini's `save-slots.md`): 160 = HPMemory, and **176–181 = MiniEnrg, FullEnrg, SneakRun, Untrap, LocEnemy, Unlocker**.

## The Number Trader (done by Gemini, same note, V)

- In Higsby's shop (text opcode `FE 01`): an 8-digit keypad, `ScriptCmd_menuSelectNumberTrader` (RS `0x08050BB4`).
- The code is checked by `sub_0804DFD4` (RS `0x0804DFD4` / BM `0x0804DFDC`), which encodes each digit through a 10-byte table and compares against **32 codes** in `dat_0804DE3C` (RS) / `dat_0804DE44` (BM).
  - Each code is a 12-byte entry: kind (chip, item, SubChip or NaviCust program), code or color, id, the code's own event flag. The table ends with `0xFF`.
- On success, `sub_0809E0B0` grants the reward through the usual routines (chips through `CHIP_GIVE`), and the flag stops the code being used twice. A SubChip code whose pack is full is un-flagged, so it can be entered again.
- **For C2:** this is the obvious place for Z-Saver, Grand Prix Power and Duo (`../completion/chips.md` §3). Adding codes means relocating the 32-entry table to free space with more entries, and finding free event flags for them.

## Still open

- Other traders, if any (`sub_0804C680`, which restocks, is still unread).
