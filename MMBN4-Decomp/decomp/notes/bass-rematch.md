# Bass rematch & dual-chip feasibility (2026-11-23)

Static analysis of Bass V1, BassΩ, and BassXX encounters, rewards, post-win flags, Black Earth 2 Mystery Data placement, and the owner's four options for making all four Bass Giga chips (`Bass X`, `BassAnly X`, `BlakBarr A`, `DeltaRay Z`) obtainable in one save.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`). **T** = tentative.

---

## 1. Executive recommendation (owner's priority order)

| Priority | Option | Feasibility | Verdict |
|---|---|---|---|
| **Main** | First Bass fight drops both chips (`Bass X` + `BassAnly X`), and BassΩ / BassXX drops both (`BlakBarr A` + `DeltaRay Z`) | **Clean & small (~30-byte hook)** | **Recommended.** Hook right after `bl sub_0801913E` in `sub_08021684` (RS `0x080216E2` / BM `0x080216E6`): when the granted reward chip is one of the 4 Bass chips (`301`, `306`, `308`, `302`), call `sub_0801913E` a second time for its cross-version pair (`301↔306`, `308↔302`). Both chips enter the pack, Library (`0x16C0 + id`), and anti-cheat chip keys (`WriteChipKey`). Caveat: the results screen UI only displays **one** reward line (the ROM's native chip); the second chip is granted silently unless the UI is also extended. |
| **Fallback 1** | First fight drops 1 chip + Unlocker SubChip (`id 181 = 0xB5`), and a nearby PMD in Black Earth 2 holds the other chip | **Feasible, moderate work** | Works cleanly if the owner wants the player to see an explicit pickup for the second chip. Needs: (a) a small hook in `sub_0809C9C4` (or `sub_08021684`) to also grant Unlocker (`InventoryAddItem(0xB5, 1)` — note `0xB5` is capped at the SubChip pack limit `[sl+0x54][0xA5]`), and (b) one new PMD entry in Black Earth 2 using free flag `0x0E3E` (gated on flag `0x009F` if desired). |
| **Fallback 2** | After first fight, a BMD with Unlocker + a PMD with the other chip appear nearby | **Feasible, moderate work** | Uses both free flags in Black Earth 2's own block (`0x0E3E` and `0x0E3F`), gated on `checkFlag 0x009F` in Black Earth 2's map-object spawn script (`0x08084888`). Avoids the SubChip-full issue of Fallback 1, since the BMD stays until opened. |
| **Fallback 3** | Bass roams the same area and can be fought again | **Already true for BassΩ / BassXX; trivial for Bass V1** | **Important finding (V, R):** Nothing ever blocks **BassΩ** (`(94,07)+0x10`) or **BassXX** (`(94,07)+0x20`) from roaming Black Earth 2 after a win — they are already infinitely rematchable! Only **Bass V1** (story battle 176) is one-shot (blocked once flag `0x009F` is set). However, rematching alone only yields the ROM's own chip unless the drop table or reward hook alternates / grants the missing chip. |

---

## 2. What starts each Bass fight & whether a win blocks rematches (V, R)

### 2.1 Bass V1 — Story Battle `176` (`0xB0`) in Black Earth 2 `(94, 07)`

- **Story battle record (`BN4_STORY_BATTLES[176]`):** RS `0x080FB140` / BM `0x080FB14C` (12 bytes; enemy list at RS `0x080FBDFA` / BM `0x080FBE06`, enemy ID `0xE4` = `Bass`). **R**
- **Trigger & blocking flag:** Flag **`0x009F`** (`TestEventFlagHL(0, 0x9F)` / `SetEventFlagHL(0, 0x9F)`).
  - Black Earth 2's Bass V1 cutscene (referencing text archive RS `0x087EE934`) checks flag `0x009F`.
  - On victory, the cutscene calls **`sub_0809C9C4`** (RS `0x0809C9C4` / BM `0x0809C9D0`, pointer at RS `0x0809C985`):
    1. Calls `TestEventFlagHL(0, 0x9F)`.
    2. If clear: calls **`sub_0802D6AC(0x0124, 1, 1)`**, which gives the **HubBatc** NaviCust program (`id = 0x124`, color `1` = Pink, qty `1`). **V, R**
    3. Calls `SetEventFlagHL(0, 0x9F)` — **permanently sets flag `0x009F`**, which both ends the Bass V1 encounter and unlocks BassΩ / BassXX in Black Earth 2. **V**
- **Does `0x009F` survive New Game+?** Flags `0x000`–`0x0FF` are preserved across New Game+ (`docs/re/completion-and-new-game-plus.md`), and across all 794 text archives nothing clears `0x009F`. So Bass V1 is strictly **once per save** unless patched. **V, R**

### 2.2 BassΩ (`(94, 07) + 0x10`) & BassXX (`(94, 07) + 0x20`)

- **Encounter-table selector (`sub_080F56CA` RS / `0x080F56D6` BM, `BN4_STORY_OFFSET` for group `0x94`):** **V**
  - For map `7` (Black Earth 2), tests flag **`0x009F`**:
    - Clear → returns `+0x00` (normal Black Earth 2 virus encounters).
    - Set → calls **`sub_08035000`** (RS `0x08035000` / BM `0x08035004`):
      - `sub_08035000` scans all 7 Mod Card slots in save slot `+0x70`.
      - Returns `1` if all 7 slots are `0xFF` (no Mod Cards loaded) → `sub_080F56CA` returns **`+0x10` (BassΩ, enemy `0xE5`)**.
      - Returns `0` if any Mod Card is loaded → `sub_080F56CA` returns **`+0x20` (BassXX, enemy `0xE6`)**.
- **Does a win block BassΩ or BassXX?** **No.** Neither BassΩ nor BassXX sets any blocking flag on victory, and nothing clears `0x009F`. Both remain in Black Earth 2's random encounter table indefinitely (and can be targeted immediately with LocEnemy). **V, R**

### 2.3 Exact Bass reward rows (`BN4_REWARD_ROWS`, RS `0x080F5F50` / BM `0x080F5F5C`) (R)

Each row has 20 `u16` entries (`0x28` bytes): indices `0..9` = Busting Levels 1–S with MegaMan HP > 3/8, indices `10..19` = HP ≤ 3/8. Encoding: bits 14–15 = `0` chip (`id = w & 0x1FF`, `code = (w >> 9) & 0x1F`), `1` zenny (`w & 0x3FFF`), `2` HP.

| Enemy | Row | Red Sun (BL 1–6 / BL 7–S) | Blue Moon (BL 1–6 / BL 7–S) |
|---|---|---|---|
| **Bass V1** (`0xE4`) | `0xE4` | **`Bass X`** (chip `301`, all 20 slots) | **`BassAnly X`** (chip `306`, all 20 slots) |
| **BassΩ** (`0xE5`) | `0xE5` | `5000z` (BL 1–6) / **`BlakBarr A`** (chip `308`, BL 7–S) | `5000z` (BL 1–6) / **`DeltaRay Z`** (chip `302`, BL 7–S) |
| **BassXX** (`0xE6`) | `0xE6` | `5000z` (BL 1–6) / **`BlakBarr A`** (chip `308`, BL 7–S) | `5000z` (BL 1–6) / **`DeltaRay Z`** (chip `302`, BL 7–S) |

*(Note: `BN4_REWARD_ROW_OF` at RS `0x080F5EC4` / BM `0x080F5ED0` explicitly excludes indices 20 and 21 of `0x0800840C` — BassΩ `0xE5` and Duo/Laser? `0xE3` — from SP-row redirection, so BassΩ always uses row `0xE5`.)* **V, R**

---

## 3. The exact reward path & where to hook "drop both" (V)

1. **Pick:** `BN4_REWARD_PICK` (RS `0x080F580C` / BM `0x080F5818`) selects the winning `u16` from `BN4_REWARD_ROWS` and stores it into `BN4_BATTLE_REWARD` (`0x0203BD4C`, `[r5 + 0x10]` in the results screen state).
2. **Decode & grant:** Results-screen state **`sub_08021684`** (RS `0x08021684` / BM `0x08021688`):
   - Calls `RewardDecode` (`sub_08021A98` RS / `0x08021A9C` BM) → `r0 = kind`, `r1 = id`, `r2 = code`.
   - For `kind == 0` (chip):
     - Validates code against `BN4_GET_CHIP` (`sub_080190D0`),
     - Calls **`sub_0801913E(chip_id, code, qty=1)`** at **RS `0x080216E2` / BM `0x080216E6`** (adds chip to pack, sets Library flag `0x16C0 + chip_id`, and calls `WriteChipKey(chip_id)`),
     - Calls **`sub_08021CA8(chip_id, code)`** to draw the single reward chip name + code on the results screen, and plays SFX `0x99`.
3. **Hook site for Main option ("drop both"):**
   - Replace the `bl sub_0801913E` at **RS `0x080216E2` / BM `0x080216E6`** with a `bl` to a ~30-byte helper in free space:
     1. Save `r0` (`chip_id`), call `sub_0801913E(r0, r1, r2)` as normal.
     2. Check `chip_id` against the 4 Bass Giga chips:
        - `301` (`Bass X`) → also call `sub_0801913E(306, 23, 1)` (`BassAnly X`, code `X = 23`),
        - `306` (`BassAnly X`) → also call `sub_0801913E(301, 23, 1)` (`Bass X`, code `X = 23`),
        - `308` (`BlakBarr A`) → also call `sub_0801913E(302, 25, 1)` (`DeltaRay Z`, code `Z = 25`),
        - `302` (`DeltaRay Z`) → also call `sub_0801913E(308, 0, 1)` (`BlakBarr A`, code `A = 0`).
     3. Return cleanly to `sub_08021684` (which displays the native chip on the results screen).
   - **What the results screen shows:** Only **one** reward chip line (the native drop), because `sub_08021CA8` draws a single tile/text row. The paired chip is immediately in the pack and registered in the Giga Library. (If a visible notice is desired, Fallback 1 or 2 gives an explicit Mystery Data pickup.)

---

## 4. Fallback Mystery Data in Black Earth 2 `(94, 07)` (V, R)

- **Unlocker SubChip item ID:** **`181` (`0xB5`)**. **V, R**
  - Confirmed in `InventoryAddItem` (`sub_0802D42C` RS / `0x0802D430` BM, SubChip range `0xB0..0xB5`), `re07-mystery-data.md`, and the Number Trader table (`0x0804DE3C`, entries 10–11).
  - Full SubChip IDs: `0xB0` (176) MiniEnrg, `0xB1` (177) FullEnrg, `0xB2` (178) SneakRun, `0xB3` (179) Untrap, `0xB4` (180) LocEnemy, **`0xB5` (181) Unlocker**.
- **Free Mystery Data flags in `0x0D00`–`0x10FF`:**
  - Only 169 of 1,024 flags in `0x0D00`–`0x10FF` are used (`re07-mystery-data.md`).
  - Black Earth 2's own 8-flag block is **`0x0E38`–`0x0E3F`**:
    - Used: `0x0E38`–`0x0E3A` (3 BMDs), `0x0E3B` (1 PMD), `0x0E3C`–`0x0E3D` (2 GMDs).
    - **Free right inside Black Earth 2's block:** **`0x0E3E`** and **`0x0E3F`** (enough for both a BMD and a PMD in Fallback 2!).
- **Black Earth 2's MD placement & spawn script:**
  - **Map-object spawn script:** RS `0x08084888` (`code_05`) — spawns each MD (`0x0E38`..`0x0E3D`) using the 11-byte sequence `03 08 25 08 1C 29 <flag_lo> <flag_hi> 45 <flag_lo> <flag_hi>`, terminated by `03 00`. Because this is standard map-object bytecode, an entry can be gated behind `checkFlag 0x009F` so it only appears after Bass V1 is defeated. **R**
  - **MD placement table:** RS `0x080ADA68` (`code_06`, 12-byte entries `{u32 prev_ptr, u16 type_playthrough, u16 flag, u32 coords_ptr, u32 contents_ptr}`):
    - `0x080ADA6C`: `0x0E38` (BMD)
    - `0x080ADA78`: `0x0E39` (BMD)
    - `0x080ADA84`: `0x0E3A` (BMD)
    - `0x080ADA90`: `0x0E3B` (PMD)
    - `0x080ADA9C`: `0x0E3C` (GMD)
    - `0x080ADAA8`: `0x0E3D` (GMD)
  - **Dynamic "chip you lack" content hook:** If a fallback PMD should dynamically award whichever Bass chip the player lacks, hook `MapObj_SpawnMysteryData` / the MD open routine (`sub_080A89BC` RS) for flag `0x0E3E`: test Library flag `0x16C0 + 301` (`Bass`) and `0x16C0 + 306` (`BassAnly`) via `TestEventFlag` (`0x080250A0`) and substitute the missing chip's `(id, code)`. **T**

---

## 5. Proposed symbol names (`names.txt` format)

```text
0809C9C4 0809C9D0 code   GEMINI       BassV1PostWinGrant               Black Earth 2 Bass V1 post-win: if flag 0x9F clear, gives HubBatc Pink (0x124, 1, 1), then sets flag 0x9F
08035000 08035004 code   GEMINI       ModCardSlotsAllEmpty             Returns 1 if all 7 Mod Card slots in save slot +0x70 are 0xFF (selects BassOmega vs BassXX in Black Earth 2)
0802D6AC 0802D6B0 code   GEMINI       InventoryAddNaviCust             r0 program id (0xC0+), r1 color, r2 qty: resolves color variant via sub_0802D6E8 and adds via InventoryAddItem
08021684 08021688 code   GEMINI       BattleResultsGrantReward         Decodes BN4_BATTLE_REWARD (RewardDecode), grants chip/zenny/HP, and displays single reward line (sub_08021CA8)
```
