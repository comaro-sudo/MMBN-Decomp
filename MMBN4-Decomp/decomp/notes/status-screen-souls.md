# Status screen Soul icon list (2026-11-23)

Static analysis of how the MegaMan Status screen collects and renders owned Double Soul icons, every hardcoded 6-Soul limit, and what a patch must change to display up to all 12 Souls.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`).

---

## 1. End-to-end Soul icon pipeline (V, R)

1. **Collect (`StatusCollectOwnedSouls`, RS `0x08034A30` / BM `0x08034A34`, `code_02`):**
   - Calls `sub_080351A8` (fills scratch buffer **`0x0201D000`** with `0xFF`, `0x20` = 32 bytes — **already large enough for 12+ Souls**).
   - Tests flag `0x0015` (Double Soul unlocked); if clear, returns.
   - Walks **`BN4_STATUS_SOUL_FLAGS`** (`dat_08034A78` RS / `0x08034A7C` BM: 6 bytes + `0xFF`), testing each flag via `TestEventFlag` and appending owned flags sequentially into **`0x0201D000[0..count-1]`**.
   - Calls `sub_08053F8C(dat_0803325C)` to spawn the Soul icon UI objects.
2. **Spawn list (`dat_0803325C`, RS `0x0803325C` / BM `0x08033260`, `0x1C` bytes):**
   - **6 entries** of 4 bytes `{u8 obj_type=0x45, u8 layer/id, u8 slot_idx, u8 0xFF}`, terminated by `0xFFFFFFFF`:
     `45 04 00 FF | 45 05 01 FF | 45 06 02 FF | 45 07 03 FF | 45 08 04 FF | 45 09 05 FF | FF FF FF FF`
   - Note: `sub_08053FA8` (the UI object runner, `code_04`) caps total active UI objects at **`0x18` (24)**.
3. **Init (`sub_0810D9D4` → `sub_0810D9F8`, RS `0x0810D9F8` / BM `0x0810DA0C`, `code_10`):**
   - Reads `r6 = [r5 + 4]` (slot index `0..5`), then `r4 = [0x0201D000 + r6]` (owned Soul flag, or `0xFF` if empty).
   - Sets `[r5] = 1` (hidden) if `r4 == 0xFF`, else `0x23` (visible).
   - Loads sprite **`0x37`** (`BN4_MENU_SPRITE_SET(0x37, 0x02031004)`). **Sprite `0x37` already contains all 12 Soul icon animations (`0..11`) in BOTH RS and BM!**
   - Searches **`dat_0810DAB8`** (RS: `17 18 19 1A 1B 1C FF`, BM: `1D 1E 1F 20 21 22 FF`) for `r4` → index `r0` (`0..5`).
   - Sets per-Soul attributes from `r0` and per-slot attributes from `r6`:

| Table (RS / BM) | Indexed by | Size (vanilla) | Vanilla RS values | Vanilla BM values | Purpose |
|---|---|---|---|---|---|
| `dat_0810DA94` / `0x0810DAA8` | slot `r6` (`0..5`) | 6 × `u16` (`0x0C` B) | `0x50, 0x54, 0x58, 0x5C, 0x60, 0x64` | same | OBJ VRAM tile base `[r7+8]` (4 tiles = 16×16 per icon) |
| `dat_0810DAA0` / `0x0810DAB4` | slot `r6` (`0..5`) | 6 × `(u16 X, u16 Y)` (`0x18` B) | `(28,42), (28,60), (28,78), (84,42), (84,60), (84,78)` | same | Initial `(X, Y)` screen position `[r7+0xC..0xE]` |
| `dat_0810DAF8` / `0x0810DB0C` | slot `r6` (`0..5`) | 6 × `(u16 X, u16 Y)` (`0x18` B) | *(duplicate of `dat_0810DAA0`)* | same | Base X used during slide-in update (`sub_0810DAD4`) |
| `dat_0810DAB8` / `0x0810DACC` | search → `r0` | 7 bytes (`6 + 0xFF`) | `17 18 19 1A 1B 1C FF` | `1D 1E 1F 20 21 22 FF` | Recognized Soul event flags |
| `dat_0810DABF` / `0x0810DAD3` | slot `r6` (`0..5`) | 6 bytes | `0x00, 0x10, 0x20, 0x30, 0x40, 0x50` | same | OAM / slot offset `[r7+0x15]` |
| `dat_0810DAC5` / `0x0810DAD9` | Soul `r0` (`0..5`) | 6 bytes | `0, 1, 2, 3, 4, 5` | `6, 7, 8, 9, 10, 11` | Sprite `0x37` animation frame (Roll..Thun = `0..5`, Proto..Wood = `6..11`) |
| `dat_0810DACB` / `0x0810DADF` | Soul `r0` (`0..5`) | 6 bytes | `0, 1, 1, 1, 0, 0` | `0, 1, 2, 0, 2, 1` | Sprite palette index `[r7+4]` |

4. **Update (`sub_0810DAD4`, RS `0x0810DAD4` / BM `0x0810DAE8`):**
   - Applies the menu slide-in X offset (`[sl+0x34] + 0x0C` × 8) to `dat_0810DAF8[r6].x`, stores into `[r7 + 0x0C]`, and calls `BN4_SPRITE_UPDATE` (`sub_0800285C`).

---

## 2. Does 12 fit on the existing screen, or does it need a second row / page? (V, R)

- **Existing layout:** 2 columns × 3 rows at **`X = 28, 84`** (`56 px` horizontal pitch) and **`Y = 42, 60, 78`** (`18 px` vertical pitch), occupying `X = 28..~140`, `Y = 42..94` on the `240×160` GBA screen.
  - Because each Soul badge in sprite `0x37` is a **wide icon/name graphic (~48–56 px wide × 16 px tall)** — which is why the two columns are `56 px` apart (`84 - 28 = 56`) rather than `16 px` apart — **4 columns × 3 rows will overlap horizontally**.
  - **Therefore, 12 wide Soul badges do NOT fit cleanly in the existing 2×3 box** unless either:
    1. **Option 1 — 2-page toggle (recommended, smallest & zero VRAM/OAM risk):** Keep the 6-slot UI layout (`dat_0803325C`, 6 UI objects, 6 VRAM slots `0x50..0x64`), expand **`BN4_STATUS_SOUL_FLAGS`** (`0x08034A78`) and **`dat_0810DAB8` / `dat_0810DAC5` / `dat_0810DACB`** to all **12 Souls** (`17..22 FF`, anims `0..11`, palettes `0,1,1,1,0,0, 0,1,2,0,2,1`), and add an **L/R (or Select) page toggle** that shifts the `0x0201D000` read offset by `+6` (`0..5` vs `6..11`) when more than 6 Souls are owned. Zero risk of overflowing the 24-slot UI object cap (`sub_08053FA8`) or colliding with OBJ VRAM tiles `0x68..0x7C`!
    2. **Option 2 — 2 columns × 6 rows (vertical expansion):** If the area below `Y = 94` (`Y = 96, 114, 132`) is free on the status screen BG, extend `dat_0803325C` to 12 entries (`slot 0..11`), relocate all 6 per-slot/per-Soul tables to free space with 12 entries (VRAM tiles `0x50..0x7C`, OAM offsets `0x00..0xB0`, 12 `(X, Y)` pairs in both `dat_0810DAA0` and `dat_0810DAF8`). Must verify in mGBA that (a) total active UI objects stay ≤ 24 (`sub_08053FA8`), (b) OBJ VRAM tiles `0x68..0x7C` are unused on the status screen, and (c) `Y = 96..148` does not overlap the bottom status frame/text.

---

## 3. Exact checklist of what a patch must change

1. **Both options need (relocate to free space `0x087F7420+` and repoint pool literals):**
   - `BN4_STATUS_SOUL_FLAGS` (`_08034A6C` in `StatusCollectOwnedSouls`): 13 bytes `17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 FF`.
   - `dat_0810DAB8` (`_0810DA78` in `sub_0810D9F8`): same 13 bytes `17..22 FF`.
   - `dat_0810DAC5` (`_0810DA88`): 12 bytes `00 01 02 03 04 05 06 07 08 09 0A 0B` (all 12 sprite `0x37` frames).
   - `dat_0810DACB` (`_0810DA8C`): 12 bytes `00 01 01 01 00 00 00 01 02 00 02 01` (exact RS + BM palettes).
2. **Option 1 (2-page toggle) additionally needs:** a small input check in the status screen update state flipping a page bit (`0` or `6`) and re-running `sub_0810D9F8` on the 6 objects with `0x0201D000 + page_offset + r6`.
3. **Option 2 (12 simultaneous icons) additionally needs:** 12-entry `dat_0803325C` (`_08034A74`), 12-entry `dat_0810DA94` (`_0810DA80`), 12-entry `dat_0810DABF` (`_0810DA7C`), and 12-entry `(X, Y)` tables for both `dat_0810DAA0` (`_0810DA84`) and `dat_0810DAF8` (`_0810DAF4`).

---

## 4. Proposed symbol names (`names.txt` format)

```text
0810D9D4 0810D9E8 code   GEMINI       StatusSoulIconTask               UI object 0x45 (Status screen Soul icon): dispatches to StatusSoulIconInit (0x0810D9F8) and StatusSoulIconUpdate (0x0810DAD4)
0810D9F8 0810DA0C code   GEMINI       StatusSoulIconInit               Reads owned Soul flag from 0x0201D000[slot], loads sprite 0x37 frame & palette, sets VRAM tile & (X,Y)
0810DAD4 0810DAE8 code   GEMINI       StatusSoulIconUpdate             Applies menu slide-in X offset to dat_0810DAF8[slot].x and calls BN4_SPRITE_UPDATE
0810DAB8 0810DACC data   GEMINI       BN4_STATUS_ICON_SOUL_FLAGS       0xFF-terminated u8 list of 6 Soul flags mapped to sprite 0x37 anims (dat_0810DAC5) and palettes (dat_0810DACB)
0803325C 08033260 data   GEMINI       BN4_STATUS_SOUL_SPAWN_LIST       6 x 4-byte UI-object-0x45 spawn entries (slots 0..5), 0xFFFFFFFF-terminated; passed to sub_08053F8C
```
