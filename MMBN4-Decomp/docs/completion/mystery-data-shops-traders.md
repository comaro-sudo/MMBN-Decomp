# Mystery Data, shops and traders (criteria C2, C4)

This summary comes from the 32 net-area maps in `ref/maps/` (local). The full per-area list is `ref/maps/legend.csv` (local, gitignored). The maps show one playthrough's placements (Normal mode, apparently Red Sun); Hard and Super Hard change some contents.

## Counts

| Source | Count | Notes |
|---|---|---|
| Blue Mystery Data (fixed item, one-shot) | 64 | 11 HP Memory, RegUps, chips, Zenny, BugFrags, NaviCust programs |
| Purple Mystery Data (locked, one-shot) | 14 | Some of the rarest chips (Geyser L, VarSwrd V, BlkBomb Z, Snake M, Guardian O, Snctuary S, NeoVari N) and the Giga Folder |
| Shop stock lines | 32 | 6 net shops; HP Memory prices climb per shop (1000Z up to 50000Z) |
| SubChip shop lines | 3 | Park Area 1 |
| BugFrag trader lines | 12 | Park Area 3 and Undernet 4, 8 to 100 BugFrags; HolyDrem (a Red Sun Giga) is one of them |
| Number Lotto codes | (guide) | Several chips, including GunSolEX |

Green Mystery Data (random, respawning) isn't on the maps. Its tables are per area and per tier (RE-07).

## What this means for Violet Eclipse

- **One-shot items:** 78 Blue and Purple Mystery Data are taken once per playthrough. New Game+ resets the Mystery Data flag range (`0x15C0`–`0x163F`, see [`../re/completion-and-new-game-plus.md`](../re/completion-and-new-game-plus.md) §3), so they come back, but anything that differs by tier must be taken in each mode. That's C4's risk. WP-B3 keeps every Mystery Data item reachable after any story point.
- **The economy:** Buying every HP Memory in the shops costs about 208,000Z (14 purchases). That's the data behind the Zenny ×1.75 and price changes (WP-A2/A3); the AddZenny hook is in [`../re/save-and-flags.md`](../re/save-and-flags.md) §4b.
- **Version differences:** the guide marks some codes as RS-only or BM-only by the virus or Navi that drops them (Billy/Silly/Killy families and others). Those are captured per code in the CSV.

## Shops and traders from the ROM itself (RE-09, 2026-11-22)

The game's own table, `BN4_SHOP_STOCK_INIT` ([`../re/shops.md`](../re/shops.md)), replaces the map-based list for shops. It holds 17 stock lists:

| Records | What they are | Lines | Price unit |
|---|---|---|---|
| 0–5 | Net shops (Zenny) | 43: 14 HP Memory + 29 chips | × 100 Zenny |
| 6–11 | SubChip shops | 3 each, unlimited quantity | × 100 Zenny |
| 12–13 | BugFrag traders | 6 each (3 chips, 3 NaviCust programs) | BugFrags |
| 14–15 | NaviCust program shops | 6 each | × 100 Zenny |
| 16 | Chip order list | 266 chips, offered once the chip's Library flag is set | × 100 Zenny |

- **Cross-check with the maps:** the 14 HP Memory lines (item 160) cost 2,077 × 100 = **207,700Z** in total, the most expensive being 50,000Z. That matches this page's "about 208,000Z, 14 purchases" and confirms the × 100 unit.
- **The only version difference in all 17 lists:** trader 13's last line, chip 307 code 7 in Red Sun and chip 305 code 18 in Blue Moon (100 BugFrags each).
- **For WP-A3 (prices):** stock is copied into the save at new game, so a table edit only reaches new games (see [`../re/shops.md`](../re/shops.md), Summary).
- **For C2 (every chip):** the chip-order list sells only chips already in the Library, so it can't be the first source for a missing chip. A Violet Eclipse change could unlock its lines differently.

## Next

- RE-07: decode the game's own Mystery Data and GMD tables, then diff them with this list to confirm every entry, and add the Hard and Super Hard variants.
- RE-09 (rest): which record is which shop in the world (the `startShop nn` script sites), and the Number Trader.
