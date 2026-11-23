# Can the Status screen show 12 Souls in a 2 × 6 grid? (2026-11-23)

Static + runtime verification (`mmbn4.exe` headless screenshot on `4752_rs.sav`) settling whether a 2 × 6 grid fits on the MegaMan Status screen.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes & runtime screenshot (`status_4752.bmp`).

---

## 1. Verdict: **2 × 6 is NOT SAFE** (R, V)

- **A 2 × 6 vertical grid (`Y = 96..148`) collides directly with the `NAVIGATOR` dialogue box**, which starts at **`Y = 94`** right beneath the bottom edge of the 3rd Soul icon (`Y = 78..94`) and spans the entire lower half of the screen (`X = 8..232`, `Y = 94..156`).
- **A 4 × 3 horizontal grid also does NOT fit** in the MegaMan box (`X = 8..118`, `Y = 34..94`): the 6 Soul icons are 16×16 circular emblems flanking MegaMan's standing sprite (`X = 40..72`):
  - Left column (`X = 28`, pixels `20..36`): only `12 px` remain to the left border (`X = 8..20`).
  - Right column (`X = 84`, pixels `76..92`): `26 px` remain to the `MEGA BUSTER` panel (`X = 120..232`), which fits at most 1 extra column (9 icons total, not 12).
- **Recommended design:** **2-page toggle (L/R or Select)** on the existing 6-slot layout (`status-screen-souls.md` §2 Option 1). It requires zero BG/layout changes, keeps the exact 6 UI objects and OBJ VRAM slots `0x50..0x64`, and displays all 12 Souls cleanly (page 1 = slots `0..5`, page 2 = slots `6..11`).

---

## 2. Exact Status screen layout (verified at runtime on `4752_rs.sav`) (R, V)

Captured via:
```powershell
mmbn4.exe roms/bn4_rs_us.gba --save <tmp>/4752_rs.sav --test 1250 \
  --input "600=START,606=,750=A,756=,950=START,956=,1010=DOWN,1016=,1030=DOWN,1036=,1050=DOWN,1056=,1080=A,1086=" \
  --shot <tmp>/status_4752.bmp
```

| Screen region (GBA 240×160) | Layer | What is drawn |
|---|---|---|
| `Y = 0..16`, full width | BG | Header bar (`MEGAMAN`) |
| `X = 18..102`, `Y = 16..34` | BG | HP box (`780/ 780`) |
| `X = 8..118`, `Y = 34..94` | BG + OBJ | Red grid panel with **MegaMan sprite** (`X = 40..72`, `Y = 42..86`) flanked by the **6 circular 16×16 Soul icons** at `(28, 42)`, `(28, 60)`, `(28, 78)` and `(84, 42)`, `(84, 60)`, `(84, 78)` |
| `X = 120..232`, `Y = 16..94` | BG + OBJ | `MEGA BUSTER` panel (`Attack LV`, `Speed LV`, `Charge LV`, `Pwr ChargeS` + green page arrow on right edge) |
| **`X = 8..232`, `Y = 94..156`** | **BG + OBJ** | **`NAVIGATOR` box** (MegaMan mugshot at left `X = 10..50`, menu text `"My status: >NaviCust / records"` at `X = 64..220`) — **blocks all vertical space below `Y = 94`!** |

---

## 3. UI object pool, OAM & OBJ VRAM usage (V, R)

From `sub_080332C8` (`code_02`):

| Spawn list (RS) | Count | UI object type(s) | Handler (RS) | Notes |
|---|---:|---|---|---|
| `dat_0803324C` | 3 | `0x13`, `0x14`, `0x49` | `0x08107374`, `0x08107494`, `0x0810DF40` | Cursor, MegaMan standing sprite (`0x39`, `(0x38, 0x50)` = `(56, 80)`, OAM `0x80`), panel arrows |
| `dat_0803325C` | 6 | `0x45` (×6) | `0x0810D9D4` | The 6 Soul icons (sprite `0x37`, VRAM tiles `0x50, 0x54, 0x58, 0x5C, 0x60, 0x64`; OAM offsets `0x00..0x50`) |
| `dat_08033278` | 1 | `0x50` | `0x0810EA94` | Dark/Emotion/status badge (`(56, 80)`, OAM `0x00`) |
| `dat_08033280` | 1 | `0x50` | `0x0810EA94` | Secondary badge |
| `dat_08033288` | 1 | `0x50` | `0x0810EA94` | Tertiary badge |
| **Total** | **12 / 24** | — | — | `sub_08053FA8` caps at 24 UI objects, so 6 extra objects *would* fit in the pool, and 6 extra OAM entries exist — **the blocker is purely screen geometry (`NAVIGATOR` box at `Y = 94..156`)**, not the UI object cap. |

---

## 4. Proposed symbol names (`names.txt` format)

```text
0803324C 08033250 data   GEMINI       BN4_STATUS_BASE_UI_LIST          3 x 4-byte UI spawn entries (types 0x13, 0x14, 0x49: cursor, MegaMan sprite, arrows) for Status screen
0810DF40 0810DF54 code   GEMINI       StatusMegaManSpriteTask          UI object 0x49: renders MegaMan's standing sprite (0x39) at (56, 80) on the Status screen
```
