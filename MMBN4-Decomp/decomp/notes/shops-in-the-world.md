# Shops in the world & Number Trader (2026-11-23)

Complete map of all 17 shop stock indices (`0..16`, `startShop nn` = `FB 05 nn`) to their in-world `(group, map)` and official area names, plus full disassembly and code/reward decode of the Number Trader.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`) and all 794 text archives in `mmbn4rs-us.tpi`.

---

## 1. Which shop is which (shops 0–16) (V, R)

Every `startShop nn` (`FB 05 nn`) occurrence across all 794 text archives was resolved back through the map dialogue pointer tables (`sub_0804E4B8` → `dat_0804E504`: mode-0 RW table at RS `0x080523E0`, mode-0 CW table at RS `0x08052410`; 3 story-phase archives per map via `dat_0804E518`) and matched against the official map-name table (RS `0x086E8966`, 14-byte entries) and `BN4_SHOP_RECORDS` (RS `0x08049CA4` / BM `0x08049CAC`).

| Shop # | Kind | Currency | `(group, map)` | Official area name | Shopkeeper / role | Representative RS text archive(s) |
|---:|---:|---|---|---|---|---|
| **0** | 0 | Zenny | `(91, 02)` | **Town Area3** | NetDealer (HPMemory, chips, NaviCust) | `0x087A4FFC`, `0x087A58D8`, `0x087A6464` |
| **1** | 0 | Zenny | `(93, 01)` | **Netopia Area** | NetDealer | `0x087ABD00`, `0x08776D0C` |
| **2** | 0 | Zenny | `(94, 00)` | **Undernet1** | NetDealer | `0x087ADD48` |
| **3** | 0 | Zenny | `(94, 03)` | **Undernet4** | NetDealer | `0x087AEB3C` |
| **4** | 0 | Zenny | `(00, 06)` | **Higsby's** | Higsby's Chip Shop (standard stock) | `0x0877D944`, `0x0877E0B0` |
| **5** | 0 | Zenny | `(90, 01)` | **ACDC Area2** | NetDealer | `0x0879F32C`, `0x0879FF70`, `0x087A04C8` |
| **6** | 0 | Zenny | `(92, 00)` | **Park Area1** | SubChip Dealer (Cyberworld) | `0x087A760C`, `0x087A81C8` |
| **7** | 0 | Zenny | `(00, 00)` | **ACDC Town** | SubChip Dealer (Real World) | `0x087781A8`, `0x08778EDC`, `0x087799E4` |
| **8** | 0 | Zenny | `(01, 00)` | **Elec Town1** | SubChip Dealer (Real World) | `0x0877E6A4`, `0x0877EE30`, `0x0877F524` |
| **9** | 0 | Zenny | `(03, 04)` | **MelSquar** (Castillo) | SubChip Dealer (Real World) | `0x08789080`, `0x08789A58` |
| **10** | 0 | Zenny | `(04, 01)` | **ColosseumAve** (Netopia) | SubChip Dealer (Real World) | `0x0878B794` |
| **11** | 0 | Zenny | `(04, 06)` | **YumRuins** (YumLand) | SubChip Dealer (Real World) | `0x0878D0C4` |
| **12** | 1 | BugFrags | `(92, 02)` | **Park Area3** | BugFrag Trader (NetDealer) | `0x087A9BC4`, `0x087AAB6C` |
| **13** | 1 | BugFrags | `(94, 02)` | **Undernet3** | BugFrag Dealer *(only version-diff slot: RS chip 307 H vs BM chip 305 S, 100 BF)* | `0x087AE4C0` |
| **14** | 0 | Zenny | `(88, 05)` | **Castillo HP** | ProgramDealer (NaviCust programs) | `0x08795694`, `0x08795F00` |
| **15** | 0 | Zenny | `(94, 05)` | **Undernet6** | Program Shop (NaviCust programs) | `0x087AF1E4` |
| **16** | 2 | Zenny | `(00, 06)` | **Higsby's** | Chip Order Service (266 Library-gated chips) | `0x0877D944`, `0x0877E0B0` |

*(Note: RS `0x0874A780` also contains a stray `FB 05 00` byte sequence inside non-script/enemy data; all 17 real shop indices `0..16` appear exclusively in the 16 map dialogue groups above.)*

---

## 2. Number Trader: code entry, verification & reward grant (V, R)

### 2.1 Flow & routines

1. **Entry (`ScriptCmd_menuSelectNumberTrader`, RS `0x08050BB4` / BM `0x08050BBC`):**
   - Triggered in Higsby's Shop (`0x0877D944`) by text script opcode **`FE 01`**.
   - Runs the 8-digit keypad UI, storing the 8 entered digits (`0..9`) into `[eToolkit+0x2C] + 0x4C..0x53`, then signals bit `0x08` in `0x0200A600`.
2. **Handler (`sub_0809E0B0`, RS `0x0809E0B0` / BM `0x0809E0BC`, `code_06`):**
   - Copies the 8 entered digits to the stack and calls **`sub_0804DFD4(digits_ptr)`** (RS `0x0804DFD4` / BM `0x0804DFDC`).
   - Return values of `sub_0804DFD4`:
     - `r0 == 0`: invalid code (shows dialogue `0x16`).
     - `r0 == 2`: already redeemed (shows dialogue `0x1D`).
     - `r0 == 1`: valid & newly redeemed (`r1 = kind`, `r2 = item_id`, `r3 = param`, `r4 = event_flag`).
   - **Reward dispatch in `sub_0809E0B0`:**
     - `kind == 0` (Battle Chip): calls `sub_0801913E(r2, r3, 1)` (adds chip to pack, sets Library flag, calls `WriteChipKey`).
     - `kind == 1` (Key Item / HP Memory / RegUP): calls `InventoryAddItem(r2, 1)` (`sub_0802D42C`), maps dialogue via `sub_0809E1AC`.
     - `kind == 2` (SubChip): calls `InventoryAddItem(r2, 1)` (`sub_0802D42C`); if pack full (`r0 != 0`), shows full-pack dialogue `0x1B` and **`sub_0809E1D8` clears the code's event flag** so it can be re-entered later!
     - `kind == 3` (NaviCust Program): calls `InventoryAddNaviCust(r2, r3, 1)` (`sub_0802D6AC`, `r3` = color).
3. **Verification & LUT (`sub_0804DFD4`, RS `0x0804DFD4` / BM `0x0804DFDC`, `code_03`):**
   - Digit-encoding LUT: **`dat_0804DFC8`** (RS) / **`dat_0804DFD0`** (BM), 10 bytes:
     - `0→3E, 1→45, 2→CC, 3→86, 4→90, 5→18, 6→4F, 7→09, 8→61, 9→E9` (identical in RS and BM).
   - Code & reward table: **`dat_0804DE3C`** (RS) / **`dat_0804DE44`** (BM), **32 entries × 12 bytes**, terminated by `0xFF`:
     - `+0x00` `u8 kind` (`0` = Chip, `1` = Item, `2` = SubChip, `3` = NaviCust)
     - `+0x01` `u8 param` (chip code `0..26`, NaviCust color `1..6`, or `0xFF` for SubChips)
     - `+0x02` `u16 item_id`
     - `+0x04` `u8 encoded_code[8]` (each byte matched against `LUT[entered_digit]`)
   - Redemption event flags: **`0x1EC0 + index`** (`0x1EC0..0x1EDF`), tested via `TestEventFlag` (`0x080250A0`) and set via `SetEventFlag` (`0x0802504C`).

### 2.2 Complete Number Trader table (all 32 entries — identical in RS & BM) (R)

*(NaviCust color IDs from `dat_0804563C`: `1=Pink, 2=Yellow, 3=Green, 4=Blue, 5=Red, 6=White`.)*

| # | Event flag | 8-digit code | Kind | Reward | Raw `(id, param)` |
|---:|---|---|---|---|---|
| 0 | `0x1EC0` | `16589650` | SubChip | **MiniEnrg** | `(0x00B0, 0xFF)` |
| 1 | `0x1EC1` | `45798331` | SubChip | **MiniEnrg** | `(0x00B0, 0xFF)` |
| 2 | `0x1EC2` | `84625799` | SubChip | **FullEnrg** | `(0x00B1, 0xFF)` |
| 3 | `0x1EC3` | `59891137` | SubChip | **FullEnrg** | `(0x00B1, 0xFF)` |
| 4 | `0x1EC4` | `24247309` | SubChip | **SneakRun** | `(0x00B2, 0xFF)` |
| 5 | `0x1EC5` | `89866302` | SubChip | **SneakRun** | `(0x00B2, 0xFF)` |
| 6 | `0x1EC6` | `02368995` | SubChip | **Untrap** | `(0x00B3, 0xFF)` |
| 7 | `0x1EC7` | `14769745` | SubChip | **Untrap** | `(0x00B3, 0xFF)` |
| 8 | `0x1EC8` | `27979609` | SubChip | **LocEnemy** | `(0x00B4, 0xFF)` |
| 9 | `0x1EC9` | `37198940` | SubChip | **LocEnemy** | `(0x00B4, 0xFF)` |
| 10 | `0x1ECA` | `00274304` | SubChip | **Unlocker** | `(0x00B5, 0xFF)` |
| 11 | `0x1ECB` | `94872322` | SubChip | **Unlocker** | `(0x00B5, 0xFF)` |
| 12 | `0x1ECC` | `75420107` | NaviCust | **SprArmr** (Green) | `(0x00CC, 3)` |
| 13 | `0x1ECD` | `02109544` | NaviCust | **Custom2** (White) | `(0x00D4, 6)` |
| 14 | `0x1ECE` | `05178924` | NaviCust | **Collect** (Blue) | `(0x0114, 4)` |
| 15 | `0x1ECF` | `43494372` | NaviCust | **Humor** (Red) | `(0x0118, 5)` |
| 16 | `0x1ED0` | `73298100` | NaviCust | **BugStop** (Green) | `(0x0128, 3)` |
| 17 | `0x1ED1` | `25435428` | NaviCust | **SoulClen** (Red) | `(0x012C, 5)` |
| 18 | `0x1ED2` | `10170506` | NaviCust | **Rush** (White) | `(0x0130, 6)` |
| 19 | `0x1ED3` | `30873642` | NaviCust | **HP+300** (Pink) | `(0x0178, 1)` |
| 20 | `0x1ED4` | `97618739` | NaviCust | **HP+300** (Yellow) | `(0x0178, 2)` |
| 21 | `0x1ED5` | `68009092` | Chip | **WideSht3 T** | `(11, 19)` |
| 22 | `0x1ED6` | `46292983` | Chip | **FlmLine1 G** | `(12, 6)` |
| 23 | `0x1ED7` | `57604335` | Chip | **FlmLine3 J** | `(14, 9)` |
| 24 | `0x1ED8` | `77038416` | Chip | **AirHoc3 V** | `(65, 21)` |
| 25 | `0x1ED9` | `88019791` | Chip | **TwnFng3 G** | `(71, 6)` |
| 26 | `0x1EDA` | `03696458` | Chip | **VarSwrd C** | `(55, 2)` |
| 27 | `0x1EDB` | `32108251` | Chip | **Snake R** | `(96, 17)` |
| 28 | `0x1EDC` | `66703422` | Chip | **Recov300 J** | `(114, 9)` |
| 29 | `0x1EDD` | `19095677` | Chip | **ColorPt \*** | `(127, 26)` |
| 30 | `0x1EDE` | `74293099` | Chip | **GunSolEX G** | `(219, 6)` |
| 31 | `0x1EDF` | `72794137` | Chip | **Z Saver Z** | `(218, 25)` |

---

## 3. Proposed symbol names (`names.txt` format)

```text
0804DFD4 0804DFDC code   GEMINI       NumberTraderCheckCode            r0 ptr to 8 digits (0..9): matches against BN4_NUMBER_TRADER_TABLE via LUT, checks/sets flag 0x1EC0+idx; returns r0=0 invalid / 1 ok / 2 used
0804DE3C 0804DE44 data   GEMINI       BN4_NUMBER_TRADER_TABLE          32 x 12-byte Number Trader entries ({u8 kind, u8 param, u16 id, u8 code[8]}), 0xFF-terminated; identical in RS and BM
0804DFC8 0804DFD0 data   GEMINI       BN4_NUMBER_TRADER_LUT            10-byte digit-encoding LUT for Number Trader codes (0..9 -> 3E 45 CC 86 90 18 4F 09 61 E9)
0809E0B0 0809E0BC code   GEMINI       NumberTraderGrantState           State machine handler: copies 8 digits, calls NumberTraderCheckCode, grants chip/item/SubChip/NaviCust reward
0809E1D8 0809E1E4 code   GEMINI       NumberTraderRefundFlag           Clears the Number Trader code's event flag (stored at [r5+0x6C]) when a SubChip reward cannot fit in the pack
0804E4B8 0804E4C0 code   GEMINI       MapGetDialogueArchive            Resolves (group, map, story_phase) -> LZ77 dialogue text archive via dat_0804E504 and dat_0804E518
```
