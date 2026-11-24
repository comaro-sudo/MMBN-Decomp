# WP-B6: Waiting Room Save Protection — Research (Round 12, Task 6)

**Date:** 2026-11-23  
**Verified in:** `code_03.s` (`sub_0803F15C`–`sub_0803FB70`), `code_00.s` (`sub_08001308`, `sub_080057C8`), and both `bn4_rs_us.gba` / `bn4_bm_us.gba`.  
**Delta:** All Waiting Room functions in `code_03` are **`BM = RS + 8`**.

---

## 1. Exact Link-Receive Path and Every Save Write

When a Waiting Room transfer completes, `sub_0803F394` (`RS 0x0803F3D6` / `BM 0x0803F3DE`) calls **`WaitingRoomReceiveTradedNavi`** (`sub_0803FA90`, `RS 0x0803FA90` / `BM 0x0803FA98`), using the 1-based slot index `r6 = [r5, #0x34]` (`1..7`) and the received link packet `r7 = [r5, #0x38]`.

### Every write performed by `WaitingRoomReceiveTradedNavi`

| Step | RS addr | BM addr | Target in RAM / Save | Size | What is written |
|---|---|---|---|---|---|
| 1 | `0x0803FABA` | `0x0803FAC2` | `0x020014F0 + (slot-1)*2 + 1` | 1 B | Translated Navi ID (`0..11`, from `dat_0803FBCC`) |
| 2 | `0x0803FAD8` | `0x0803FAE0` | `[sl + 0x78] + slot * 0x40` (`save +0x2D30`) | `0x40` B | Traded Navi stats (`CopyByEightWords`); MegaMan is slot 0 |
| 3 | `0x0803FAF0` | `0x0803FAF8` | `[sl + 0x7C] + slot * 0xE0` (`save +0x2F34`) | `0xE0` B | Traded Navi AI / chip pattern (`CopyByEightWords`); MegaMan is slot 0 |
| 4 | `0x0803FB0A` | `0x0803FB12` | `0x02007230 + slot * 0x2C` | `0x2C` B | Traded Navi SP times / battle record (`0x0200725C..0x0200738F`, 16 B before `BN4_TOURNAMENT_POOLS` at `0x020073A0`) |
| 5 | `0x0803FB20` | `0x0803FB28` | `0x02037C80 + slot * 0x2C` | `0x2C` B | `BN4_SP_TIMES` battle RAM mirror |
| 6 | `0x0803FB3A` | `0x0803FB42` | `[sl + 0x44] + 0x70 + slot * 4` (`save +0x00F4`) | 4 B | 3-char operator/Navi name tag |
| 7 | `0x0803FB52` | `0x0803FB5A` | `0x020014F0 + (slot-1)*2 + 0` | 1 B | Slot number `1..7` |
| 8 | `0x0803FB56` | `0x0803FB5E` | `0x02006F40 + i*2 + 1` (`BN4_TRADED_NAVIS`) | up to 7×1 B | Calls `SyncTradedNavisToPool` (`RS 0x0803F948` / `BM 0x0803F950`), copying every non-`0xFF` Navi ID from `0x020014F0` into `0x02006F40` |
| 9 | `0x0803FB5A` | `0x0803FB62` | SRAM (`0x0E000000`) | full save | Calls `SaveGameKeepPlace` (`RS 0x08048858` / `BM 0x08048860`) |

### Other Waiting Room write sites
- **`WaitingRoomDeleteEntry`** (`RS 0x0803F15C` / `BM 0x0803F164`): sets `[0x020014F0 + slot*2] = 0xFFFF`, **`[0x02006F40 + slot*2] = 0xFFFF`**, clears `[sl+0x44]+0x70+(slot+1)*4 = 0xFFFFFFFF`, resets slot `slot+1` stats/AI via `sub_0803FB70`, and saves (`sub_08048858`).
- **`WaitingRoomClearSlotForSend`** (`RS 0x0803F450` / `BM 0x0803F458`): when all 7 slots are full and the player chooses a slot to overwrite, writes **`[0x02006F40 + slot*2 + 1] = 0xFF`** (`RS 0x0803F494` / `BM 0x0803F49C`) and sets `r5[+0x34] = slot + 1` before `WaitingRoomReceiveTradedNavi` overwrites it.

---

## 2. Crucial Finding: Soul Flags and Chip Keys Are NOT Written by Waiting Room

Exhaustive inspection of the entire Waiting Room module (`0x0803F000`–`0x0803FC00`) confirms:
- **Soul flags `0x0017`–`0x0022` (`[sl + 0x48]`):** **ZERO** writes. No `SetEventFlag` / `ClearEventFlag` / `ClearEventFlagRange` call exists anywhere in the Waiting Room receive or delete paths.
- **Chip keys (`0x02000670` / `[sl + 0x84]`):** **ZERO** writes.
- **MegaMan's own Navi record (slot 0):** **Untouched** in normal operation because `r6` (`[r5, #0x34]`) is always `1..7` (`r1 + 1` at `RS 0x0803F32C` / `BM 0x0803F334`, or `r0 + 1` at `RS 0x0803F496` / `BM 0x0803F49E`).

### What CAN actually be lost or corrupted
1. **`0x02006F40` (`BN4_TRADED_NAVIS`):** `WaitingRoomDeleteEntry` (`RS 0x0803F196`–`0x0803F19A`), `WaitingRoomClearSlotForSend` (`RS 0x0803F494`), and `SyncTradedNavisToPool` (`RS 0x0803F948`) directly overwrite/clear `0x02006F40`. In vanilla BN4, `0x02006F40` gates whether opposite-version Soul Navis enter story tournaments (`sub_08044058`), which is why deleting/overwriting Waiting Room Navis could remove opposite-version Soul Navis from tournament pools. (In Violet Eclipse, opposite-version Souls come from the lobby/patch, not `0x02006F40`, so existing Soul flags `0x0017`–`0x0023` are already safe.)
2. **Unchecked `r6` slot index in `WaitingRoomReceiveTradedNavi`:** `WaitingRoomReceiveTradedNavi` does not bounds-check `r6 = [r5, #0x34]` before multiplying by `0x40`, `0xE0`, `0x2C`, and `4`:
   - If `r6 == 0` (e.g. corrupted state), it overwrites **MegaMan's own stats/AI/SP times** (slot 0).
   - If `r6 >= 8`, `0x02007230 + r6*0x2C >= 0x02007390` overflows directly into **`BN4_TOURNAMENT_POOLS` (`0x020073A0`)** and the end of the save image!
3. **Unchecked packet offsets (`r7[+2..+12]`):** `WaitingRoomReceiveTradedNavi` trusts `u16` offsets in the link buffer `r7` and immediately calls `SaveGameKeepPlace` (`sub_08048858`) on completion.

---

## 3. Proposed Guard (Exact RS & BM Code Points)

Because Soul flags `0x0017`–`0x0023` and chip keys are already untouched by the Waiting Room, WP-B6 only needs **two lightweight guards** (or one combined hook):

### Guard A — Bounds-check `r6` (`1 <= slot <= 7`) at entry to `WaitingRoomReceiveTradedNavi`
- **Hook site:** `RS 0x0803FA90` / `BM 0x0803FA98` (`WaitingRoomReceiveTradedNavi` prologue: `push {r4-r7, lr}; ldrh r6, [r5, #0x34]`, bytes `f0 b5 ae 8e`).
- **Behavior:** If `r6 == 0 || r6 > 7`, return `r0 = 1` (error; caller `sub_0803F394` at `RS 0x0803F3DA` / `BM 0x0803F3E2` branches to `sub_0803FBD8` error screen without saving). This guarantees slot 0 (MegaMan) and `BN4_TOURNAMENT_POOLS` (`0x020073A0`) can never be overwritten.

### Guard B — Protect `0x02006F40` (`BN4_TRADED_NAVIS`) from deletion (only needed if Violet Eclipse ever relies on `0x02006F40`)
- **`WaitingRoomDeleteEntry`** (`RS 0x0803F196`–`0x0803F19A` / `BM 0x0803F19E`–`0x0803F1A2`): `strb r1, [r0]; strb r1, [r0, #1]` (`01 70 41 70`) writes `0xFFFF` to `0x02006F40 + slot*2`. Replacing those 4 bytes with `nop; nop` (`c0 46 c0 46`) keeps `0x02006F40` intact while still deleting from `0x020014F0`.
- **`WaitingRoomClearSlotForSend`** (`RS 0x0803F494` / `BM 0x0803F49C`): `strb r1, [r2, #1]` (`51 70`) writes `0xFF` to `0x02006F40 + slot*2 + 1`. Replacing with `nop` (`c0 46`) skips clearing it.
- **`SyncTradedNavisToPool`** (`RS 0x0803F948` / `BM 0x0803F950`): currently copies non-`0xFF` IDs from `0x020014F0` into `0x02006F40` by index (overwriting existing slots). If `0x02006F40` is kept as an append-only/OR-merged set, hook `0x0803F948` (`BM 0x0803F950`) to only write into empty (`0xFF`) slots of `0x02006F40` instead of overwriting occupied indices.

---

## 4. What the Waiting Room Is Still Used For in Violet Eclipse

Even with every Soul (`0x0017`–`0x0023`) and all 36 Secret Chips earnable solo in Violet Eclipse:
1. **Free Tournament custom-AI opponents:** Players can still link-trade customized Navis (stats in `[sl+0x78]`, learned AI/chip patterns in `[sl+0x7C]`, SP times in `0x02007230`/`0x02037C80`, operator name in `[sl+0x44]+0x70`) and fight them in the Free Tournament at Higsby's.
2. **Vanilla Free Tournament entry gate:** `BN4_OPEN_FREE_BRACKET` (`RS/BM 0x080057C8`) checks `0x020014F0` for at least one non-`0xFF` entry before opening the Free Tournament bracket.
3. **Multiplayer / vanilla save compatibility:** Trading Navis with unpatched RS/BM saves in mGBA multiplayer or on hardware continues to work normally.
