# RE-09: shops (first pass, 2026-11-22)

**V** = read in the code (RS addresses; `code_03` is +8 in BM, `code_00` +0). **T** = tentative.

## How a shop opens (V)

1. Script command `startShop` (`FB 05 nn`, handler `ScriptCmd_startShop`, RS `0x0805025C`) calls `ShopOpen` (`0x08005858`) with `nn` = shop index.
2. `ShopOpen` calls `ShopInit` (`0x08049DB4`). It saves the map place in the game state (`+0x38`–`+0x44`), sets game-state mode `0x24`, and starts the transition (`sub_08005AFC`).
3. `ShopInit`:
   - zeroes the shop state at `0x0200A740` (`0x34` bytes) and stores the index at `+4`;
   - stores the shop's record pointer at `+0x1C` (`BN4_SHOP_RECORDS + n × 0x10`);
   - stores a work buffer (`0x02026000`) at `+0x24`.
4. The shop runs as a state machine: `sub_08049DEC` dispatches on `[0x0200A740]` through the table at `0x08049E14`.

## Shop records: `BN4_SHOP_RECORDS`, RS `0x08049CA4` / BM `0x08049CAC`, 17 × 16 bytes (V; same values in both versions except the dialogue pointers)

| Offset | Meaning |
|---|---|
| `+0x00` | u32 kind: 0 for most shops; 1 for shops 12–13; 2 for shop 16 (T: chip shop / SubChip shop / BugFrag trader?) |
| `+0x04` | ROM pointer to an LZ77-compressed text archive (header `0x10`): the shopkeeper's dialogue (T) |
| `+0x08` | u32: `n × 0x40`, an offset into a per-shop block (T: the shop's stock state) |
| `+0x0C` | u32 **event-flag base**: `sub_0804932C` clears flag `base + item byte` (the item byte from a RAM list at `0x0201A000`). 8 for shops 0–15, `0x140` for shop 16 |

## Not found yet

- **The stock and price lists.** The ACDC Area 2 shop's known prices (800, 2400, 3200, 7700) don't appear in the ROM as plain u16/u32 values or in units of 100. So the stock list is encoded differently, or built from another table into `0x0201A000`.
- **Next:** find who fills `0x0201A000`, through the shop state machine's states at `0x08049E14`. That leads to the price table that the key-item price change (WP-A3) needs.
