# NPC & Special-Chip Blue Mystery Data Placements (2026-11-23)

Verified overworld and cyberworld coordinates for:
1. **9 Soul Navi operators** inside Higsby's `(00, 06)`;
2. **2 Soul Navi operators + the JunkMan trash can** outside Higsby's in ACDC Town `(00, 00)`;
3. **2 new Blue Mystery Data** (**Grand Prix Power** in ACDC Area 2 and **Z-Saver** in Undernet 1) with free flags in `0x0D00`–`0x10FF`.

**V** = verified in disassembly & ROM tables (`bn4_rs_us.gba`, `bn4_bm_us.gba`). **R** = visually verified on headless `mmbn4.exe` captures (kept outside the repo).

---

## 1. Higsby's `(00, 06)` — 9 Operators (R, V)

### Existing layout & constraints
- **NPC cap:** 16 per map (`sub_08004224`, `0x02006140..0x02006EC0`, stride `0xD8`). Higsby's currently has **4 NPCs** (`RS 0x08056550` / `BM 0x0805655C`), so adding **9 operators** uses **13 / 16** slots (3 slots spare).
  - Existing NPC positions to stay clear of: `NPC[0]` (Higsby behind counter, `(-46, -32)` / `(-64, 0)`), `NPC[1]` (`(-84, -32)`), `NPC[2]` (girl by left display case, `(-96, 62)` / `(-84, -38)`), `NPC[3]` (girl near front-left, `(-112, 78)` / `(14, 48)`).
  - **Door entrance (`WELCOME` mat):** `(-20, 116, 0)` (`higsby_door.bmp`).
  - **Key paths kept completely clear:** door `(-20, 116)` → Higsby's counter (`(-46, -32)`), door → left display/table & Free Tournament terminal (`(-64, -56)`), and door → Chip Trader / Number Trader (`(-140, 40)`).
  - **Coordinate axes (verified in `higsby_door.bmp`, `higsby_right.bmp`, `p_higsby_shelf.bmp`, `p_higsby_left.bmp`):** `+X` = up-right toward the right shelves (`X = 50` enters the shelf; `X = 36` is cleanly on the floor in front of it); `+Y` = down-right toward the front wall (`Y = 112..116`).

### 9 Verified Operator Spots in Higsby's `(00, 06)`

| Slot | Zone | Coords `(x, y, z)` | Facing | Notes |
|---:|---|---|---:|---|
| 1 | Right wall, top of shelves (near counter) | `(36, 42, 0)` | `5` (SW) | Clean floor right by counter corner & upper shelf (`higsby_right.bmp` verified `(40, 40)` clear) |
| 2 | Right wall, middle of shelves | `(36, 70, 0)` | `5` (SW) | 28 units down the right shelf from Slot 1; `X = 36` stays clear of shelf collision (`X = 50`) |
| 3 | Right wall, bottom of shelves | `(36, 98, 0)` | `5` (SW) | Near front-right corner, clear of the `WELCOME` mat |
| 4 | Front-right wall, right of `WELCOME` rug | `(12, 112, 0)` | `7` (NW) | Beside the red `WELCOME` rug on the right, clear of door trigger `(-20, 116)` |
| 5 | Front-left wall, left of `WELCOME` rug | `(-54, 112, 0)` | `7` (NW) | Beside the red `WELCOME` rug on the left, wide aisle kept open to counter |
| 6 | Front-left wall, toward bottom corner | `(-84, 112, 0)` | `7` (NW) | 30 units left of Slot 5, clear of `NPC[3]` at `(-112, 78)` |
| 7 | Front-left corner, beside blue Chip Trader | `(-140, 44, 0)` | `3` (SE) | Verified in `p_higsby_left.bmp` (`(-140, 40)`); leaves Number Trader & Chip Trader accessible from front-right |
| 8 | Front-left wall, below blue Chip Trader | `(-134, 72, 0)` | `1` (NE) | Clear of `NPC[3]` (`(-112, 78)`) and clear of Chip Trader interaction tile |
| 9 | Center-left floor, between left case & mat | `(-56, 56, 0)` | `3` (SE) | Open tile between counter aisle and front-left area; keeps full walking lanes on all 4 sides |

*(NPC list pointer to patch when relocating Higsby's NPC list to free space: **RS `0x0805444C` / BM `0x08054458`**.)*

---

## 2. ACDC Town `(00, 00)` — 2 Operators + Trash Can (R, V)

- **NPC cap:** ACDC Town currently has **13 NPCs** (`RS 0x08054434` / `BM 0x08054440` → `RS 0x08054F58`). Adding **2 operators + 1 trash-can NPC** uses **16 / 16** slots (exact fit).
- **Clearance:** Higsby's door exit is `(-176, -90, 0)` (`dir = 3`); existing boy NPC stands on the front-left of the shop (`(-198, -2)` / `(-172, 0)`).

| Object / NPC | Coords `(x, y, z)` | Facing | Notes |
|---|---|---:|---|
| **JunkMan Trash Can** | `(-150, -90, 0)` | `3` (SE) | Front-right corner of Higsby's under the awning / edge of bushes (`acdc_trash.bmp`; use `(-156, -84, 0)` if preferring full pavement) |
| **Outside Operator #10** | `(-134, -76, 0)` | `5` (SW) | Verified in `p_acdc_op.bmp`: open sidewalk just right of Higsby's bushes, facing the shop entrance |
| **Outside Operator #11** | `(-164, -56, 0)` | `7` (NW) | Across the sidewalk by the squirrel-statue grass border; keeps the full walkway in front of `(-176, -90)` unobstructed |

---

## 3. The Two Special-Chip Blue Mystery Data (V, R)

Checked against `re07-mystery-data.md` and the raw Mystery Data tables (`RS 0x080A9110`–`0x080ADD8C`, CW master table `0x080ADD34` RS / `0x080ADD40` BM) plus `ref/maps/ACDC-2.png` and `ref/maps/Undernet-1.png`.

### 3.1 Grand Prix Power — ACDC Area 2 `(90, 01)`

- **Area:** **`(90, 01)` — ACDC Area 2** (`RS table @ 0x080AA6B4` / `BM @ 0x080AA6C0`).
  - Currently has **0 BMDs** (only 1 PMD `0x0D08` at `(242, -294, 0)` and 2 GMDs `0x0D09`/`0x0D0A`).
- **Coordinates:** **`(106, 326, 0)`** (on the quiet circular dead-end pad of the lower-left 5-finger peninsula, adjacent to the exit arrow on `p_acdc2_f5.bmp`, off the main ACDC-1 ↔ ACDC-3 / YumLand thoroughfare).
  - **Zero overlap:** Nearest existing Mystery Data spawns are GMD `0x0D0A` at `(142, 322, 0)` (36 units away on the next finger) and GMD `0x0D09` at `(206, 290, 0)` (106 units away).
- **Free "taken" flag:** **`0x0D0B`** (inside ACDC Area 2's own 8-flag block `0x0D08`–`0x0D0F`; only `0x0D08`, `0x0D09`, `0x0D0A` are used).

### 3.2 Z-Saver — Undernet 1 `(94, 00)`

- **Area:** **`(94, 00)` — Undernet 1** (`RS table @ 0x080AC6F4` / `BM @ 0x080AC700`).
  - Currently has 1 BMD `0x0E00` at `(-90, 214, 0)`, 1 PMD `0x0E01` at `(322, -34, 0)`, and 2 GMDs `0x0E02`/`0x0E03`.
- **Coordinates:** **`(-282, 56, 0)`** (verified walkable in `p_under1_spur.bmp`: a hidden dead-end red tile tucked directly underneath the elevated yellow bridge on the western side of Undernet 1, completely off the Town-4 ↔ Sharo / Undernet-2 main paths).
  - **Zero overlap:** Nearest existing Mystery Data spawns are GMD `0x0E03` at `(-152, -12, 0)` / `(-154, 118, 0)` / `(-378, -106, 0)` (all > 140 units away).
- **Free "taken" flag:** **`0x0E04`** (inside Undernet 1's own 8-flag block `0x0E00`–`0x0E07`; only `0x0E00`–`0x0E03` are used).

### How to register each new BMD in a patch
1. Relocate the map's 12-byte Mystery Data entry list (`(90, 01)` at `RS 0x080AA6B4` / `(94, 00)` at `RS 0x080AC6F4`) to free space, appending `{ u8 type=1 (BMD), u8 obj_flags=0, u16 flag, ptr placement_8B, ptr content_16B }` before the `0` terminator.
2. Relocate the map's CW NPC/object spawn list (`dat_08029E14`) to free space and append one 4-byte `MapObj_SpawnMysteryData` (`0x080A89BC`) script entry passing `flag` (`0x0D0B` or `0x0E04`).

---

## 4. Proposed symbol names (`names.txt` format)

```text
080AA6B4 080AA6C0 data   GEMINI       BN4_MD_ACDC2                     ACDC Area 2 (90,01) Mystery Data list (1 PMD 0x0D08, 2 GMDs 0x0D09..0x0D0A; 0x0D0B free)
080AC6F4 080AC700 data   GEMINI       BN4_MD_UNDERNET1                 Undernet 1 (94,00) Mystery Data list (1 BMD 0x0E00, 1 PMD 0x0E01, 2 GMDs 0x0E02..0x0E03; 0x0E04 free)
```
