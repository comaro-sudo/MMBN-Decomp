# WP-A4 (Plan WP-A3/A4): Key-Item & Upgrade Shop Prices — Facts for Owner Decision (Round 12, Task 7)

**Date:** 2026-11-23  
**Source:** Direct dump of `BN4_SHOP_STOCK_INIT` (`RS 0x0804ADA0` / `BM 0x0804ADA8`, 16 shops × 8 slots × 8 bytes; `BM = RS + 8`).  
**Price units:** u16 at slot `+0x06` = **× 100 Zenny** for Zenny shops (Shops 0–11, 14–15), or **BugFrags as-is** for BugFrag shops (Shops 12–13).  
**Technical note (`docs/re/shops.md`):** `BN4_SHOP_STOCK_INIT` is copied into the save (`[sl+0x58]`, save `+0x164C`) only at **New Game** (`ShopStockInit`, `RS 0x0804BB60` / `BM 0x0804BB68`). Editing the ROM table alone changes prices for **new saves only**; to also reach existing saves, either patch `ShopPriceToMoney` (`RS 0x0804ACF4` / `BM 0x0804ACFC`) + the buy deduction (`RS 0x0804A04C` / `BM 0x0804A054`), or refresh prices on save load.

---

## 1. Complete Inventory of Sold Key Items, Upgrades, SubChips & NaviCust Programs

- **SubMemory (`0xA5`), RegUP (`0xA2`–`0xA4`), ExpMemory (`0xA1`):** **0 sold in shops** (all come from BMD/PMD/story).
- **RS vs BM parity:** Every single HP Memory, SubChip, and NaviCust program slot across Shops 0–15 is **100% identical in RS and BM**. The **only** version difference in Shops 0–15 is Shop 13 slot 5 (`100 BF` Giga chip: `HolyDrem H` in RS vs `BugCurse S` in BM).

### A. HP Memory (`type = 1, id = 0x0A0`) — 14 purchases, **207,700Z** total

| Shop | Area | Story Gate (PT1) | Slot | RS Price Addr | BM Price Addr | Raw u16 (LE bytes) | Vanilla Price |
|---:|---|---|---:|---|---|---|---:|
| 5 | ACDC Area 2 | Chapter 1 (start) | 0 | `0x0804AEE6` | `0x0804AEEE` | `10` (`0a 00`) | 1,000Z |
| 5 | ACDC Area 2 | Chapter 1 (start) | 1 | `0x0804AEEE` | `0x0804AEF6` | `32` (`20 00`) | 3,200Z |
| 0 | Town Area 3 | Den Tournament | 0 | `0x0804ADA6` | `0x0804ADAE` | `25` (`19 00`) | 2,500Z |
| 0 | Town Area 3 | Den Tournament | 1 | `0x0804ADAE` | `0x0804ADB6` | `60` (`3c 00`) | 6,000Z |
| 0 | Town Area 3 | Den Tournament | 2 | `0x0804ADB6` | `0x0804ADBE` | `100` (`64 00`) | 10,000Z |
| 1 | Netopia Area | Eagle/Hawk Tournament | 0 | `0x0804ADE6` | `0x0804ADEE` | `60` (`3c 00`) | 6,000Z |
| 1 | Netopia Area | Eagle/Hawk Tournament | 1 | `0x0804ADEE` | `0x0804ADF6` | `100` (`64 00`) | 10,000Z |
| 1 | Netopia Area | Eagle/Hawk Tournament | 2 | `0x0804ADF6` | `0x0804ADFE` | `140` (`8c 00`) | 14,000Z |
| 2 | Undernet 1 | RS/BM Tournament | 0 | `0x0804AE26` | `0x0804AE2E` | `100` (`64 00`) | 10,000Z |
| 2 | Undernet 1 | RS/BM Tournament | 1 | `0x0804AE2E` | `0x0804AE36` | `150` (`96 00`) | 15,000Z |
| 2 | Undernet 1 | RS/BM Tournament | 2 | `0x0804AE36` | `0x0804AE3E` | `200` (`c8 00`) | 20,000Z |
| 2 | Undernet 1 | RS/BM Tournament | 3 | `0x0804AE3E` | `0x0804AE46` | `300` (`2c 01`) | 30,000Z |
| 3 | Undernet 4 | RS/BM Tournament (deep Undernet) | 0 | `0x0804AE66` | `0x0804AE6E` | `300` (`2c 01`) | 30,000Z |
| 3 | Undernet 4 | RS/BM Tournament (deep Undernet) | 1 | `0x0804AE6E` | `0x0804AE76` | `500` (`f4 01`) | 50,000Z |

### B. NaviCust Programs — Zenny (`type = 3`, Shops 14–15) — 12 purchases, **92,900Z** total

| Shop | Area | Story Gate (PT1) | Slot | Program (Color) | RS Price Addr | BM Price Addr | Raw u16 (LE) | Vanilla Price |
|---:|---|---|---:|---|---|---|---|---:|
| 14 | Castillo HP | Eagle/Hawk Tournament | 0 | `HP+100` (Green) | `0x0804B126` | `0x0804B12E` | `34` (`22 00`) | 3,400Z |
| 14 | Castillo HP | Eagle/Hawk Tournament | 1 | `ChargMAX` (Yellow) | `0x0804B12E` | `0x0804B136` | `48` (`30 00`) | 4,800Z |
| 14 | Castillo HP | Eagle/Hawk Tournament | 2 | `Shield` (Red) | `0x0804B136` | `0x0804B13E` | `55` (`37 00`) | 5,500Z |
| 14 | Castillo HP | Eagle/Hawk Tournament | 3 | `FstBarr` (Pink) | `0x0804B13E` | `0x0804B146` | `68` (`44 00`) | 6,800Z |
| 14 | Castillo HP | Eagle/Hawk Tournament | 4 | `MegFldr1` (White) | `0x0804B146` | `0x0804B14E` | `80` (`50 00`) | 8,000Z |
| 14 | Castillo HP | Eagle/Hawk Tournament | 5 | `Custom1` (Red) | `0x0804B14E` | `0x0804B156` | `96` (`60 00`) | 9,600Z |
| 15 | Undernet 6 | Post-game (Undernet 6) | 0 | `HP+300` (Yellow) | `0x0804B166` | `0x0804B16E` | `96` (`60 00`) | 9,600Z |
| 15 | Undernet 6 | Post-game (Undernet 6) | 1 | `OilBody` (Blue) | `0x0804B16E` | `0x0804B176` | `62` (`3e 00`) | 6,200Z |
| 15 | Undernet 6 | Post-game (Undernet 6) | 2 | `MegFldr1` (White) | `0x0804B176` | `0x0804B17E` | `80` (`50 00`) | 8,000Z |
| 15 | Undernet 6 | Post-game (Undernet 6) | 3 | `Custom1` (Red) | `0x0804B17E` | `0x0804B186` | `96` (`60 00`) | 9,600Z |
| 15 | Undernet 6 | Post-game (Undernet 6) | 4 | `FlotShoe` (White) | `0x0804B186` | `0x0804B18E` | `84` (`54 00`) | 8,400Z |
| 15 | Undernet 6 | Post-game (Undernet 6) | 5 | `Reflect` (Red) | `0x0804B18E` | `0x0804B196` | `130` (`82 00`) | 13,000Z |

### C. BugFrag Traders (Shops 12–13) — 6 NaviCust Programs (**196 BF**) + 6 Chips (**272 BF**) = **468 BF** total

| Shop | Area | Story Gate (PT1) | Slot | Item | RS Price Addr | BM Price Addr | Raw u16 (LE) | Vanilla Price |
|---:|---|---|---:|---|---|---|---|---:|
| 12 | Park Area 3 | Den Tournament | 0 | NaviCust `SneakRun` (Green) | `0x0804B0A6` | `0x0804B0AE` | `8` (`08 00`) | 8 BF |
| 12 | Park Area 3 | Den Tournament | 1 | Chip #139 `*` | `0x0804B0AE` | `0x0804B0B6` | `10` (`0a 00`) | 10 BF |
| 12 | Park Area 3 | Den Tournament | 2 | NaviCust `HP+400` (Yellow) | `0x0804B0B6` | `0x0804B0BE` | `16` (`10 00`) | 16 BF |
| 12 | Park Area 3 | Den Tournament | 3 | Chip #145 R | `0x0804B0BE` | `0x0804B0C6` | `24` (`18 00`) | 24 BF |
| 12 | Park Area 3 | Den Tournament | 4 | NaviCust `Collect` (Pink) | `0x0804B0C6` | `0x0804B0CE` | `42` (`2a 00`) | 42 BF |
| 12 | Park Area 3 | Den Tournament | 5 | Chip #210 R | `0x0804B0CE` | `0x0804B0D6` | `60` (`3c 00`) | 60 BF |
| 13 | Undernet 3 | RS/BM Tournament | 0 | NaviCust `AirShoes` (Red) | `0x0804B0E6` | `0x0804B0EE` | `22` (`16 00`) | 22 BF |
| 13 | Undernet 3 | RS/BM Tournament | 1 | Chip #150 `*` | `0x0804B0EE` | `0x0804B0F6` | `28` (`1c 00`) | 28 BF |
| 13 | Undernet 3 | RS/BM Tournament | 2 | NaviCust `AntiMagc` (Blue) | `0x0804B0F6` | `0x0804B0FE` | `36` (`24 00`) | 36 BF |
| 13 | Undernet 3 | RS/BM Tournament | 3 | Chip #125 U | `0x0804B0FE` | `0x0804B106` | `50` (`32 00`) | 50 BF |
| 13 | Undernet 3 | RS/BM Tournament | 4 | NaviCust `AttckMAX` (Pink) | `0x0804B106` | `0x0804B10E` | `72` (`48 00`) | 72 BF |
| 13 | Undernet 3 | RS/BM Tournament | 5 | Giga Chip #307 H (RS) / #305 S (BM) | `0x0804B10E` | `0x0804B116` | `100` (`64 00`) | 100 BF |

### D. Notable SubChips (`type = 1`, Shops 6–11, unlimited qty)

- **`Unlocker` (`0xB5`):** `4,000Z` (`raw 40` = `28 00`) at Shops 6 (`0x0804AF36`), 9 (`0x0804AFF6`), 10 (`0x0804B02E`), 11 (`0x0804B076`) (`BM +8`). 14 Purple Mystery Data exist (up to `56,000Z` if bought).
- **`LocEnemy` (`0xB4`):** `10,000Z` (`raw 100` = `64 00`) at Shops 6 (`0x0804AF2E`), 7 (`0x0804AF76`), 9 (`0x0804AFEE`), 10 (`0x0804B036`) (`BM +8`).
- Cheap consumables (`MiniEnrg` 100Z, `Untrap` 100Z, `SneakRun` 200Z, `FullEnrg` 1,000Z) are already inexpensive.

---

## 2. Missability & One-Save Economy Assessment

- **Missable?** **None.** All 16 shop areas remain accessible in post-game on Playthrough 1.
- **Unreasonably priced for a 1-playthrough save?**
  - Vanilla BN4 priced late shops for a 3-playthrough economy. On one playthrough:
    - **HP Memory:** **207,700Z** (the last 4 Undernet HP Memories alone cost **130,000Z**: `20k + 30k + 30k + 50k`).
    - **Zenny NaviCust:** **92,900Z** (combined Zenny upgrades + programs = **300,600Z**).
    - **SubChips for 100%:** `Unlocker` (`4,000Z` × up to 14 = `56,000Z`) and `LocEnemy` (`10,000Z` each for chip farming).
    - **BugFrag Traders:** **468 BF** total (`196 BF` for programs + `272 BF` for chips, including `72 BF` `AttckMAX` and `100 BF` version-exclusive Giga).

---

## 3. Pricing Options (Facts Only — Exact Bytes to Change)

*(All ROM addresses below are RS; **BM = RS + 8** for every entry.)*

### Option 1 — Plan Baseline (60% off → 40% price) on HP Memory Only (14 × 2 bytes)
- **Total HP Memory cost:** `207,700Z → 83,000Z` (saves `124,700Z`; max HP Memory `50,000Z → 20,000Z`).
- **Bytes to change (RS addr, `BM = RS + 8`):**
  - Shop 5: `0x0804AEE6`: `0a 00 → 04 00` (400Z); `0x0804AEEE`: `20 00 → 0d 00` (1,300Z)
  - Shop 0: `0x0804ADA6`: `19 00 → 0a 00` (1,000Z); `0x0804ADAE`: `3c 00 → 18 00` (2,400Z); `0x0804ADB6`: `64 00 → 28 00` (4,000Z)
  - Shop 1: `0x0804ADE6`: `3c 00 → 18 00` (2,400Z); `0x0804ADEE`: `64 00 → 28 00` (4,000Z); `0x0804ADF6`: `8c 00 → 38 00` (5,600Z)
  - Shop 2: `0x0804AE26`: `64 00 → 28 00` (4,000Z); `0x0804AE2E`: `96 00 → 3c 00` (6,000Z); `0x0804AE36`: `c8 00 → 50 00` (8,000Z); `0x0804AE3E`: `2c 01 → 78 00` (12,000Z)
  - Shop 3: `0x0804AE66`: `2c 01 → 78 00` (12,000Z); `0x0804AE6E`: `f4 01 → c8 00` (20,000Z)

### Option 2 — 60% off (40% price) on HP Memory + Zenny NaviCust + Unlocker & LocEnemy (34 × 2 bytes)
- **Totals:** HP Memory `207,700Z → 83,000Z`; Zenny NaviCust `92,900Z → 36,900Z` (**combined `300,600Z → 119,900Z`**); `Unlocker` `4,000Z → 1,600Z`; `LocEnemy` `10,000Z → 4,000Z`.
- **Bytes to change:** All 14 HP Memory slots from Option 1, **plus**:
  - Shop 14: `0x0804B126`: `22 00 → 0e 00` (1,400Z); `0x0804B12E`: `30 00 → 13 00` (1,900Z); `0x0804B136`: `37 00 → 16 00` (2,200Z); `0x0804B13E`: `44 00 → 1b 00` (2,700Z); `0x0804B146`: `50 00 → 20 00` (3,200Z); `0x0804B14E`: `60 00 → 26 00` (3,800Z)
  - Shop 15: `0x0804B166`: `60 00 → 26 00` (3,800Z); `0x0804B16E`: `3e 00 → 19 00` (2,500Z); `0x0804B176`: `50 00 → 20 00` (3,200Z); `0x0804B17E`: `60 00 → 26 00` (3,800Z); `0x0804B186`: `54 00 → 22 00` (3,400Z); `0x0804B18E`: `82 00 → 34 00` (5,200Z)
  - `Unlocker` (4 shops: `0x0804AF36`, `0x0804AFF6`, `0x0804B02E`, `0x0804B076`): `28 00 → 10 00` (1,600Z)
  - `LocEnemy` (4 shops: `0x0804AF2E`, `0x0804AF76`, `0x0804AFEE`, `0x0804B036`): `64 00 → 28 00` (4,000Z)

### Option 3 — Flat 50% Cut Across HP Memory, Zenny NaviCust & BugFrag Traders (38 × 2 bytes)
- **Totals:** HP Memory `207,700Z → 103,800Z`; Zenny NaviCust `92,900Z → 46,400Z` (**combined `300,600Z → 150,200Z`**); BugFrag Traders (Shops 12–13) **`468 BF → 234 BF`** (programs `196 → 98 BF`, chips `272 → 136 BF`).
- **Bytes to change (RS addr, `BM = RS + 8`):**
  - HP Memory (14 slots): `0a→05`, `20→10`, `19→0c`, `3c→1e`, `64→32`, `3c→1e`, `64→32`, `8c→46`, `64→32`, `96→4b`, `c8→64`, `2c 01→96 00`, `2c 01→96 00`, `f4 01→fa 00` (same 14 offsets as Option 1)
  - Shop 14 (6 slots): `22→11`, `30→18`, `37→1b`, `44→22`, `50→28`, `60→30` (same 6 offsets as Option 2)
  - Shop 15 (6 slots): `60→30`, `3e→1f`, `50→28`, `60→30`, `54→2a`, `82→41` (same 6 offsets as Option 2)
  - Shop 12 (6 slots, `0x0804B0A6..0x0804B0CE` step +8): `08→04`, `0a→05`, `10→08`, `18→0c`, `2a→15`, `3c→1e`
  - Shop 13 (6 slots, `0x0804B0E6..0x0804B10E` step +8): `16→0b`, `1c→0e`, `24→12`, `32→19`, `48→24`, `64→32`
