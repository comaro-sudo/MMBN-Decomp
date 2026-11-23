# Remaining Save-Block Toolkit Slots (`eToolkit +0x50` through `+0x80`)

Analysis by Gemini (2026-11-22), completing [`docs/re/save-and-flags.md`](../../docs/re/save-and-flags.md) §1.1.
Status: **V** = verified in disassembly, **R** = confirmed across all 13 community saves in `ref/saves/` via `patch/tools/savetool.py` (read-only).

## 1. Complete Slot Identification Table

| Toolkit Slot | Save Block Offset | Size | Name | Contents & Layout | Status |
|---|---|---|---|---|---|
| `+0x50` | `+0x05B4` | `0xF04` (`0xF00` used) | **Chip Pack** | `320 × 12` bytes: for each chip ID, bytes `+0..+3` hold owned counts for its 4 valid codes (remaining bytes hold pack metadata/sort). Gated by `BN4_CHIP_GATE` (`0x080063C8`). Cleared by `ChipPackInit` (`0x080192A8`), summed by `ChipPackCountById` (`0x0801922C`). | V, R |
| `+0x54` | `+0x14B8` | `0x194` (`0x190` used) | **Items, SubChips & NaviCust Inventory** | `400` bytes indexed by item ID (`0x00`–`0x18F`): key items, upgrades (`0xA0` HPMemory, `0xA1` ExpMemory, `0xA2..A4` RegUP, `0xA5` SubMemory initialized to `4`), SubChips (`0xB0`–`0xB5`, capped at `[+0xA5]`), and NaviCust programs (`0xC0+`, capped at `9`). Cleared by `InventoryInit` (`0x0802D410`), modified by `InventoryAddItem` (`0x0802D42C`). | V, R |
| `+0x58` | `+0x164C` | `0xDC4` (`0xDC0` used) | **Shop Stock** | Already documented in `docs/re/shops.md`. | V, R |
| `+0x5C` | `+0x2410` | `0x24` (`0x20` used) | **NaviCust Grid** | 5×5 NaviCust program-placement grid (25 cells + padding; copied to `0x0201C000` by `NaviCustLoadToWorkBuf` `0x08046DB8`). | V, R |
| `+0x60` | `+0x2434` | `0xCC` (`0xC8` used) | **NaviCust Placed Programs** | `25 × 8` bytes (`0xC8` bytes): placed program records `{u8 prog_id, u8 color, ...}`, copied to `0x0201C020` by `NaviCustLoadToWorkBuf` (`0x08046DB8`) and indexed by `NaviCustGetPlacedProgram` (`0x08046DAC`). | V, R |
| `+0x64` | `+0x2500` | `0x14` (`0x10` used) | **NaviCust Compiled Bug Flags** | 16-byte bug/status table computed when compiling the 5×5 NaviCust grid (`NaviCustCompileBugs`, `0x08047344`, which zeroes `+0x64` alongside three 25-byte scratch grids). | V, R |
| `+0x68` | `+0x2514` | `0x4` (`0x1` used) | **Active Mod Card / Patch ID** | `u8` active e-Reader Mod Card ID (0 = none). Dispatched through the 134-entry Mod Card effect table `dat_08041E8C` by `ModCardApplyActive` (`0x08041E5C`) / `ModCardSetAndApply` (`0x08041E6A`); cleared by `ModCardClearActive` (`0x08041E50`). | V, R |
| `+0x6C` | `+0x2518` | `0x4` (`0x2` used) | **NaviCust / Mod Card HP Bonus** | `u16` additive Max HP bonus (`+50, +100, +200, +300, +400, +500` via `sub_08041DE0`–`sub_08041E38`). | V, R |
| `+0x70` | `+0x251C` | `0x10` (`0xE` used) | **Loaded e-Reader / Mod Card Slots** | 7-slot e-Reader / Mod Card ID list (`0xFF` = empty, cleared by `ModCardSlotsInit` `0x08034FAC`; byte `+5` checked by `sub_0802F6F0`). | V, R |
| `+0x74` | `+0x252C` | `0x804` (`0x800` used) | **Mystery Data State** | Already documented in `decomp/notes/re07-mystery-data.md` (`1024 × 2` bytes for flags `0x0D00`–`0x10FF`). | V |
| `+0x78` | `+0x2D30` | `0x204` (`0x200` used) | **Navi Stats (MegaMan + 7 Waiting Room Navis)** | `8 × 0x40` bytes: slot `0` = MegaMan, slots `1`–`7` = the 7 Waiting Room traded Navis (written by `WaitingRoomReceiveTradedNavi` `0x0803FA90`). | V, R |
| `+0x7C` | `+0x2F34` | `0x704` (`0x700` used) | **Navi AI / Battle Pattern Data (MegaMan + 7 Waiting Room Navis)** | `8 × 0xE0` bytes: slot `0` = MegaMan, slots `1`–`7` = the 7 Waiting Room traded Navis' AI/chip-pattern blocks. Initialized to `0xFF` by `NaviPatternInitAll` (`0x0800E8DA`) / `NaviPatternInitMegaMan` (`0x0800E8EC`); slots `1`–`7` populated by `WaitingRoomReceiveTradedNavi` (`0x0803FA90`). | V, R |
| `+0x80` | `+0x3638` | `0x194` (`0x190` used) | **Inventory Anti-Cheat Key Table** | `400` bytes mirroring `+0x54`: for every acquired item `id`, `[+0x80][id] = [0x02000240 + id] XOR SAVE_XOR_KEY_0` (`0x2C` RS / `0x71` BM), written by `WriteItemAntiCheatKey` (`0x08006380`, called by `InventoryAddItem` `0x0802D42C`). | V, R |

## 2. Confirmation Across the 13 Community Saves (R)

Every save in `ref/saves/*.sps` was decrypted read-only (XOR key at `0x1554`, shift at `0x1550 & 0x1FE`) and checked:

| Save File | Version | PT | `+0x50` Chips | `+0x54` Items | `+0x5C` Grid Cells | `+0x60` Placed Progs | `+0x68` ModCard | `+0x6C` HP Bonus | `+0x7C` Active Navis | `+0x80` Key Matches (RS / BM) |
|---|---|---|---|---|---|---|---|---|---|---|
| `4420.sps` | BM | 0 | 30 | 19 | 0 | 0 | `0x00` | `0` | 1/8 | 0 / 24 |
| `4429.sps` | RS | 0 | 32 | 40 | 12 | 4 | `0x00` | `0` | 1/8 | 52 / 0 |
| `4440.sps` | RS | 0 | 59 | 74 | 10 | 6 | `0x00` | `0` | 1/8 | 113 / 0 |
| `4454.sps` | RS | 1 | 69 | 45 | 23 | 6 | `0x00` | `500` | 1/8 | 122 / 0 |
| `4462.sps` | RS | 1 | 101 | 72 | 23 | 6 | `0x00` | `500` | 1/8 | 135 / 0 |
| `4526.sps` | RS | 2 | 94 | 61 | 23 | 6 | `0x00` | `500` | 1/8 | 135 / 0 |
| `4569.sps` | BM | 2 | 170 | 92 | 25 | 8 | `0x00` | `100` | 1/8 | 0 / 154 |
| `4579.sps` | BM | 2 | 203 | 94 | 25 | 8 | `0x00` | `100` | 1/8 | 0 / 156 |
| `5779.sps` | BM | 2 | 87 | 39 | 20 | 10 | `0x00` | `350` | 1/8 | 0 / 124 |
| `7830.sps` | BM | 2 | 203 | 94 | 25 | 8 | `0x00` | `100` | 1/8 | 0 / 156 |
| `4752.sps` | RS | 3 | 220 | 69 | 22 | 6 | `0x00` | `0` | 1/8 | 166 / 0 |
| `4760.sps` | RS | 3 | 219 | 77 | 22 | 6 | `0x00` | `0` | 1/8 | 169 / 0 |
| `4891.sps` | BM | 3 | 221 | 122 | 25 | 7 | `0x00` | `500` | 1/8 | 0 / 181 |
| `22565.sps` | RS | 4 | 276 | 123 | 25 | 7 | `0x00` | `250` | **7/8** | 198 / 0 |

Key confirmations:
- **`22565.sps`** (the only save with 6 traded Blue Moon Navis) is the **only** save with `7/8` populated `0xE0`-byte blocks in `+0x7C` (MegaMan + 6 traded Navis); every other save has `1/8` (MegaMan only).
- **`+0x6C`** matches exact multiples of 50 (`0, 100, 250, 350, 500`) corresponding to equipped NaviCust HP programs.
- **`+0x80`** cleanly separates every Red Sun save (`^ 0x2C`, 0 BM matches) from every Blue Moon save (`^ 0x71`, 0 RS matches).
- **`+0x68`** (`0x00`) and **`+0x70`** (all `0xFF`) confirm no US community save has active e-Reader Mod Cards loaded.

## 3. Proposed Symbol Names (Merged into `decomp/symbols/names.txt`)

```
08006380 08006380 code   GEMINI       WriteItemAntiCheatKey            Writes [eToolkit+0x80][id] = [0x02000240 + id] XOR SAVE_XOR_KEY_0
0800E8DA 0800E8DA code   GEMINI       NaviPatternInitAll               Fills save slot +0x7C (8 x 0xE0 bytes, MegaMan + 7 Waiting Room Navis) with 0xFF
0800E8EC 0800E8EC code   GEMINI       NaviPatternInitMegaMan           Fills save slot +0x7C slot 0 (0xE0 bytes, MegaMan) with 0xFF
0801922C 0801922C code   GEMINI       ChipPackCountById                r0 chip_id: returns total owned count across 4 code slots in save slot +0x50 (gated by BN4_CHIP_GATE)
080192A8 080192A8 code   GEMINI       ChipPackInit                     Zeroes save slot +0x50 (0xF00 bytes = 320 x 12B chip pack)
0802D410 0802D414 code   GEMINI       InventoryInit                    Zeroes save slot +0x54 (0x190 bytes) and sets SubMemry (0xA5) = 4
0802D42C 0802D430 code   GEMINI       InventoryAddItem                 Adds item/SubChip/NaviCust program to save slot +0x54 with caps and WriteItemAntiCheatKey
08034FAC 08034FB0 code   GEMINI       ModCardSlotsInit                 Fills save slot +0x70 (7 Mod Card slots) with 0xFF
08041E50 08041E58 code   GEMINI       ModCardClearActive               Clears active Mod Card ID in save slot +0x68 to 0
08041E5C 08041E64 code   GEMINI       ModCardApplyActive               Applies active Mod Card from save slot +0x68 via dat_08041E8C (134 handlers)
08041E6A 08041E72 code   GEMINI       ModCardSetAndApply               Stores r0 to save slot +0x68 and applies its Mod Card handler
08046DAC 08046DB4 code   GEMINI       NaviCustGetPlacedProgram         r0 idx: returns &[eToolkit+0x60] + idx * 8
08046DB8 08046DC0 code   GEMINI       NaviCustLoadToWorkBuf            Copies NaviCust grid (+0x5C, 0x20B) -> 0x0201C000 and placed programs (+0x60, 0xC8B) -> 0x0201C020
08047344 0804734C code   GEMINI       NaviCustCompileBugs              Zeroes NaviCust bug flags (+0x64, 0x10B) and three 5x5 scratch grids, then compiles bugs
```
