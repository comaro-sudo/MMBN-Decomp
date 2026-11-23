# Patch runtime test results (batch 2) — Status 2-Page, Duo Chip, Relocated NPCs (2026-11-23)

Live runtime verification of the three newest packages built via `python patch/build.py`:
- `build/bn4_rs_overhaul.gba`: SHA-1 `b163a6a913547249e57354b45f77396cfa6b1b23` (11,589 bytes differ, BPS OK)
- `build/bn4_bm_overhaul.gba`: SHA-1 `dfdf34b120708ebd0df7fd1fdc8133efe359d0c4` (11,591 bytes differ, BPS OK)

All tests ran headless on `build/pc-arm64/Release/mmbn4.exe` with all screenshots, save copies, and EWRAM dumps kept outside the repo.

---

## Summary

| Package | File | Result | Key finding |
|---|---|---|---|
| **Status 2-page Soul view** | `wp_d4_status_two_pages.asm` | **PARTIAL FAIL — REAL BUG CAUGHT!** | **L/R page switching works** (`0x0201D018` flips `0 ↔ 6` on 11-Soul save `22565`, stays `0` on 6-Soul save `4752`, `NAVIGATOR`/panels/closing all work), **BUT 3 of the 6 Soul icons are invisible on both pages!** Root cause: `StatusSoulLoadFlag` at `ICON_FREE` (`0x0810DAB8`) **overwrote `dat_0810DABF`** (`00 10 20 30 40 50`, OAM slot offsets still referenced by `_0810DA7C`). |
| **Duo Giga chip reward** | `wp_d2_duo_chip_reward.asm` | **BLOCKED — 2 ISSUES CAUGHT!** | 1. **Duo (`93`/`94`) skips the results screen** (`BN4_STORY_BATTLES` `+1 = 0x93`, bit 7 `0x80` set → goes straight to ending/credits). 2. Even when forced through the results screen, **`Duo` (`0x137`, #292) is an e-Reader RAM-loaded chip** whose `BN4_CHIP_TABLE` art/icon/pal pointers point to empty EWRAM (`0x020008A0`–`0x02000E00`) with `flags = 0x07` (Library bit `0x40` clear): results screen shows blank **`"        D"`** with a solid black box, and `LibFlag(0x17F7)` / pack stay unchanged! |
| **Relocated NPC lists** | `wp_d0_npc_lists.asm` | **PASS (RS & BM)** | Both Higsby's `(00, 06)` and ACDC Town `(00, 00)` spawn all original NPCs in their exact spots and talk normally in both Red Sun and Blue Moon. |

---

## 1. `wp_d4_status_two_pages.asm` — **PARTIAL FAIL (Bug Caught)**

- **Test runs (`bn4_rs_overhaul.gba`):**
  - **`22565.sav` (11 Souls):**
    - Open Status screen (`f=1200`): `0x0201D018 = 0` (Page 1).
    - Press `R` (`f=1200..1206`, capture `f=1240`): `0x0201D018 = 6` (Page 2).
    - Press `L` (`f=1240..1246`, capture `f=1280`): `0x0201D018 = 0` (back to Page 1).
    - Press `RIGHT`, `DOWN`, `B` (`f=1380`): cleanly closes Status screen back to PET menu (`r7_st_22565_nav_close.bmp`), no corruption.
  - **`4752.sav` (6 Souls):**
    - Open Status screen (`f=1200`) and press `R` (`f=1240`): `0x0201D018` stays `0` (L/R properly ignored when ≤ 6 Souls).
- **Visual bug caught in `r7_st_22565_p1.bmp` and `r7_st_22565_p2_R.bmp`:**
  - Page 1 shows only **3** icons (slots 1, 2, 5: GutsSoul, WindSoul, ThunSoul); slots 0, 3, 4 are missing.
  - Page 2 shows only **2** icons (slots 1, 2: NumberSoul, MetalSoul); slots 0, 3, 4 are missing.
- **Root cause (verified in `bn4_rs_overhaul.gba`):**
  - Vanilla `0x0810DAB8..0x0810DAD1` layout is:
    - `0x0810DAB8` (7 B): `dat_0810DAB8` (`17 18 19 1A 1B 1C FF`, old Soul flags)
    - **`0x0810DABF` (6 B): `dat_0810DABF` (`00 10 20 30 40 50`, OAM slot offsets — STILL REFERENCED BY `_0810DA7C`!)**
    - `0x0810DAC5` (6 B): `dat_0810DAC5` (old anims)
    - `0x0810DACB` (6 B): `dat_0810DACB` (old palettes)
  - `StatusSoulLoadFlag` (placed at `ICON_FREE = 0x0810DAB8`) overwrote `0x0810DABF..0x0810DAC4` with Thumb code bytes `19 8C 5C 01 22 70`, corrupting the OAM slot offsets for slots 0–5!
- **Exact fix for Claude:**
  - Copy the 6 bytes `00 10 20 30 40 50` to `FS_STATUS_SOULS + 0x30` and repoint `ICON_OAM_PTR` (**RS `0x0810DA7C` / BM `0x0810DA90`**) to `FS_STATUS_SOULS + 0x30`. Then all 25 bytes at `ICON_FREE` are genuinely unused and all 6 icons render on both pages.

---

## 2. `wp_d2_duo_chip_reward.asm` — **BLOCKED (2 Issues Caught)**

1. **Final boss Duo skips the results screen (verified in `BN4_STORY_BATTLES` & cutscene `0x0808C478`):**
   - `BN4_STORY_BATTLES[93]` (Duo) and `[94]` (DuoΩ) both have bytes `00 93 64 16 33 17 1F 00`: byte **`+1 = 0x93` has bit 7 (`0x80`) set**, which is the engine's "no results screen / no reward" flag (only 7 of 245 story battles set `+1` bit 7: `93, 94, 183, 187, 188, 189, 191`).
   - In cutscene `0x0808C478` (`51 5D 01` / `51 5E 01`), defeating Duo goes straight to the ending cutscene and credits, never showing the results screen or running `BN4_REWARD_PICK`.
2. **Even when forced through the results screen, `Duo` (`0x137`) has no ROM graphics/name and does not register in the Library:**
   - Forced `bm_win_f600.ss` with defeated species `0xE2` (Duoβ) and `0xE3` (DuoΩ) on `bn4_bm_overhaul.gba`:
     - Both correctly rolled reward word **`0x0737`** (`0x137 | (3 << 9)` = `Duo D`) from `wp_d2_duo_chip_reward.asm`.
     - **However**, `r7_duo_e2_rev.bmp` displayed **`"        D"` (blank name) with a solid black chip-art square**, `LibFlag(0x16C0 + 0x137 = 0x17F7)` stayed `False`, and no normal pack slot incremented!
   - **Root cause (`BN4_CHIP_TABLE` `0x080197EC`):** Both **`Duo` (`0x137`, #292)** and **`Grand Prix Power` (`0x138`, #291)** are Japan-only **e-Reader RAM-loaded chips** whose `BN4_CHIP_TABLE` pointers point to **empty EWRAM** (`0x02000E00 / 0x020008A0 / 0x02000DE0` for Duo; `0x020013E0 / 0x02000E80 / 0x020013C0` for Grand Prix Power) and have `flags = 0x07` (Library bit `0x40` clear).
   - **Implication:** To make `Duo` (`0x137`) and `Grand Prix Power` (`0x138`) usable in US ROMs without an e-Reader card, Violet Eclipse must either populate their chip data/art/names (or initialize their EWRAM e-Reader buffers from a ROM patch) and grant `Duo` via a post-Duo/post-clear hook (or clear `+1` bit 7 on DuoΩ). By contrast, **`Z-Saver` (`0xDA`)** is a normal ROM-backed chip (`flags = 0x42`, ROM pointers `0x08746AF8 / 0x08732578 / 0x0873F798`).

---

## 3. `wp_d0_npc_lists.asm` — **PASS (RS & BM)**

- **Test runs:**
  - `bn4_rs_overhaul.gba` in Higsby's `(00, 06)` (`r7_npc_rs_higsby_talk.bmp`) and ACDC Town `(00, 00)` (`r7_npc_rs_acdc.bmp`).
  - `bn4_bm_overhaul.gba` in Higsby's `(00, 06)` (`r7_npc_bm_higsby_talk.bmp`) and ACDC Town `(00, 00)` (`r7_npc_bm_acdc.bmp`).
- **Results:** All original NPCs spawn in their exact vanilla coordinates, render cleanly, and open dialogue normally when talked to (`"See that tra..."` in Higsby's). No crashes or regressions in either version.

---

## 4. Proposed symbol names (`names.txt` format)

```text
0810DABF 0810DAD3 data   GEMINI       BN4_STATUS_ICON_OAM_OFFSETS      6 bytes (00 10 20 30 40 50): OAM slot offsets for the 6 Status screen Soul icons (referenced by _0810DA7C RS / _0810DA90 BM; must NOT be overwritten!)
```
