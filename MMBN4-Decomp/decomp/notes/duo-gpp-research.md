# Duo (`0x137`) and Grand Prix Power (`0x138`) — Art, Names & Grant Research (Round 8, Task 3, 2026-11-23)

All values below are **verified against all four local ROMs**: `bn4_rs_us.gba`, `bn4_bm_us.gba`, `bn4_rs_jp.gba`, and `bn4_bm_jp.gba`.

---

## 1. The Chip Table Records (`0x137` Duo, `0x138` Grand Prix Power) (V, R)

Chip table address: **`0x080197EC`** in both `bn4_rs_us.gba` and `bn4_bm_us.gba` (`0x2C` bytes per entry; `0x080196F8` in both JP ROMs).

| Field (offset) | Normal Giga (`0x12D` Bass, RS US) | `0x137` **Duo** (RS & BM US) | `0x138` **GP Power** (RS & BM US) |
|---|---|---|---|
| Codes (`+0x00..0x03`) | `17 FF FF FF` (`X`) | `03 FF FF FF` (**`D`**) | `06 FF FF FF` (**`G`**) |
| Element / Rarity (`+0x04..0x05`) | `00` / `04` | `00` / `04` | `00` / `04` |
| Byte `+0x09` (library/flags) | `0x47` (`bit 6` set = standard ROM chip) | **`0x07`** (`bit 6` clear = event/e-Reader) | **`0x07`** (`bit 6` clear = event/e-Reader) |
| Damage (`+0x1A`, u16) | `60` (`0x003C`) | `200` (`0x00C8`) | `200` (`0x00C8`) |
| Icon ptr (`+0x20`, 4bpp `0x80` B) | `0x087471F8` (ROM) | **`0x02000E00`** (**EWRAM**) | **`0x020013E0`** (**EWRAM**) |
| Image ptr (`+0x24`, 4bpp `0x540` B) | `0x087398F8` (ROM) | **`0x020008A0`** (**EWRAM**) | **`0x02000E80`** (**EWRAM**) |
| Palette ptr (`+0x28`, 16-col `0x20` B) | `0x0873FF78` (ROM) | **`0x02000DE0`** (**EWRAM**) | **`0x020013C0`** (**EWRAM**) |

- Every normal chip's art is **uncompressed**: `img` = **`0x540` bytes** (7×6 4bpp tiles = 56×48 px), `pal` = **`0x20` bytes** (16 BGR555 colors), `icon` = **`0x80` bytes** (2×2 4bpp tiles = 16×16 px); total **`0x5E0` bytes** per chip.
- Both `0x137` and `0x138` point into the two `0x5E0`-byte EWRAM event-chip slots (`0x020008A0..0x02000E7F` and `0x02000E80..0x0200145F`, saved/loaded as a `0xBC0`-byte block in `sub_080011E4`, `code_00.s:2085`), which are all-zero unless populated by a Japanese event download / distribution cartridge.

---

## 2. The Japanese ROMs (`bn4_rs_jp.gba`, `bn4_bm_jp.gba`) — Critical Finding (R)

**Neither `bn4_rs_jp.gba` nor `bn4_bm_jp.gba` contains the art for Duo (`0x137`) or Grand Prix Power (`0x138`).**
- In **both** JP ROMs (`0x080196F8`), entries `0x137` and `0x138` have the **exact same EWRAM pointers** as the US ROMs:
  - `0x137` Duo: `icon=0x02000E00`, `img=0x020008A0`, `pal=0x02000DE0`
  - `0x138` GP Power: `icon=0x020013E0`, `img=0x02000E80`, `pal=0x020013C0`
- Checked bytes immediately following `0x136` (BugCurse) in `bn4_rs_jp.gba`: they belong to other chips (no unused Duo/GPP art is left in the JP ROM).
- **Consequence:** `build.py` **cannot** copy Duo or GP Power's art from `bn4_rs_jp.gba` / `bn4_bm_jp.gba`. To give them non-blank art in Violet Eclipse, the options are:
  1. **Original pixel art** created for Violet Eclipse (`0x5E0` bytes per chip = 56×48 4bpp image + 16-color palette + 16×16 4bpp icon, placed in free space and repointed at `CHIP_TABLE + id*0x2C + 0x20..0x28`); or
  2. **Extract from a JP distribution save** (not in the repo; would require an extra user-supplied file); or
  3. **Reuse an existing ROM boss/chip icon+image** as a placeholder until original art is drawn.

*(Note: when repointing `icon`/`img`/`pal` to ROM free space, also check whether byte `+0x09` needs `0x07 -> 0x47` so the Library displays them like standard Giga chips.)*

---

## 3. The Chip Names Archive (`Chip Names Bank 1`) (V, R)

Chip names are split into two **uncompressed** `u16`-offset archives (identical contents in RS and BM):
- **Bank 0 (`0x000..0x0FF`):** `RS 0x087481D8` / `BM 0x08747CCC` (`0xA3A` bytes).
- **Bank 1 (`0x100..0x185`, contains `0x137` and `0x138`):**
  - **RS:** `0x08748C14`, size **`0x4CC` bytes** (1,228 B), **8 ROM pointers:**
    `0x0801FDA0, 0x08021D14, 0x0802F1B8, 0x0802F3E4, 0x0803813C, 0x0803CFAC, 0x0804A7C8, 0x0804FB78`
  - **BM:** `0x08748708`, size **`0x4CC` bytes** (1,228 B), **8 ROM pointers:**
    `0x0801FDA0, 0x08021D18, 0x0802F1BC, 0x0802F3E8, 0x08038144, 0x0803CFB4, 0x0804A7D0, 0x0804FB84`

Current contents of entries `0x37` (`0x137` Duo) and `0x38` (`0x138` GP Power) in Bank 1:
- `0x137` (`+0x0292`): `FF 00 00 E5` (`printBuffer 0` — reads empty EWRAM event-chip name buffer 0!)
- `0x138` (`+0x0296`): `FF 00 01 E5` (`printBuffer 1` — reads empty EWRAM event-chip name buffer 1!)

### How to add English names (no TextPet rebuild required)
- **BN4 encoding (`bn4-utf8.tbl`):**
  - `"Duo"` + `E5` = **`0E 3A 34 E5`** (4 bytes — **fits in-place** over `FF 00 00 E5` at `RS 0x08748EA6 / BM 0x0874899A`!).
  - `"GP Power"` + `E5` = **`11 1A 00 1A 34 3C 2A 37 E5`** (9 bytes; or `"GP-Power"` = `11 1A 40 1A 34 3C 2A 37 E5`).
- **Relocating Bank 1 to free space** (to fit `"GP Power"` cleanly):
  1. `.incbin ROM_IN, CHIP_NAMES_B1 - 0x08000000, 0x4CC` into a `0x4E0`-byte free-space slot.
  2. Overwrite the 2 `u16` offset table entries `[0x37]` and `[0x38]` (`+0x6E` and `+0x70`) to point to the appended `"Duo\xE5"` and `"GP Power\xE5"` strings at `+0x4CC` and `+0x4D0`.
  3. Repoint the 8 ROM pointers listed above for RS/BM to the relocated Bank 1 address. Zero ROM text committed.

---

## 4. Duo's Post-Battle Grant Hook (`ChipGive(0x137, 3, 1)`) (V)

Because Duo's battle (`BN4_STORY_BATTLES` 93/94) skips the battle results screen (`wp_d2_duo_chip_reward.asm` never fires), there are **two verified post-Duo hook points** where `ChipGive(0x137, 3, 1)` (`CHIP_GIVE = 0x0801913E`, code `3` = `D`) can be called:

### Option A (Recommended): right when Duo is defeated (`sub_080C4780`, `code_07.s`)
```arm
sub_080C4780:                       ; RS 0x080C4780 / BM 0x080C478C
    push {lr}
    ldrb r0, [r5, #0xa]
    tst  r0, r0
    bne  _080C4796
    ldrb r0, [r5, #4]
    cmp  r0, #0
    beq  _080C4796
    movs r0, #0
    movs r1, #0x6f                  ; flag 0x006F = Duo defeated (Final Boss completion star!)
    bl   sub_08025048               ; SetEventFlagHL -> RS 0x080C4792 / BM 0x080C479E
_080C4796:
    bl   sub_08010D4C
    pop  {pc}
```
- **Hook site:** `RS 0x080C4792` / `BM 0x080C479E` (`bl SetEventFlagHL`, 4 bytes).
- **Behavior:** Fires immediately upon defeating Duo (sets flag `0x006F` and grants **Duo D** via `ChipGive(0x137, 3, 1)`), right before the ending/clear save writes the pack and Library to SRAM. Works on every Duo clear (first clear and rematches).

### Option B (Fallback): on New Game+ / Game Clear transition (`bl IncPlaythrough`, `code_00.s`)
- **Hook site:** **`0x08000E86`** (identical in RS & BM, `bl sub_08000C5A`).
- **Behavior:** Fires once when starting NG+ from a clear save. Simpler (single shared address), but grants Duo D only when starting NG+ rather than immediately on beating Duo.
