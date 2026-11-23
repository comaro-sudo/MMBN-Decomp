# Duo Soul (13th Double Soul): Exact Patch Design (Round 10, Task 6, 2026-11-23)

Exact drop-in patch specification for **Duo Soul** (MetalSoul recolour, granted by beating Duo V1; owner decisions in `docs/pending/duo-soul.md`, framework verified in `decomp/notes/duo-soul-research.md`). All addresses, palettes, tables, and routines are **verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba`**.

---

## 1. Duo Palette Mapped onto MetalSoul (`list[0][9]`) (V, R)

- **MetalSoul sprite archive:** `list[0][9]` = **`0x08264D1C`** (**identical in RS & BM**).
- **MetalSoul palette block:** `0x08264D1C + 0x8BDC` = **`0x0826D8F8`** (4-byte header, followed by 3 × 32-byte BGR555 palettes; **100% identical in RS & BM**):
  - Palette 0 (normal): **`0x0826D8FC`** (32 bytes)
  - Palette 1 (hit-flash): `0x0826D91C`
  - Palette 2 (shadow/dark): `0x0826D93C`

### MetalSoul Palette 0 slot meanings & proposed 16-color Duo BGR555 palette

| Slot | MetalSoul BGR555 | MetalSoul RGB | Visual Role on MetalSoul | Proposed Duo BGR555 | Proposed Duo RGB | Duo Visual Role |
|---:|---|---|---|---|---|---|
| `0` | `0x0020` | `(0, 8, 0)` | Transparent BG | **`0x0020`** | `(0, 8, 0)` | Transparent BG (keep) |
| `1` | `0x0000` | `(0, 0, 0)` | Black outline | **`0x0000`** | `(0, 0, 0)` | Black outline (keep) |
| `2` | `0x0000` | `(0, 0, 0)` | Deep shadow | **`0x0000`** | `(0, 0, 0)` | Deep shadow (keep) |
| `3` | `0x0842` | `(16, 16, 16)` | Darkest grey outline | **`0x1043`** | `(24, 16, 32)` | Deep obsidian-violet outline |
| `4` | `0x1C7C` | `(224, 24, 56)` | Bright crimson accent | **`0x149F`** | `(248, 32, 40)` | Bright Duo crimson crest |
| `5` | `0x0854` | `(160, 16, 16)` | Mid red accent | **`0x0C36`** | `(176, 8, 24)` | Mid crimson crest |
| `6` | `0x044F` | `(120, 16, 8)` | Dark red shadow | **`0x0810`** | `(128, 0, 16)` | Dark crimson shadow |
| `7` | `0x5675` | `(168, 152, 168)` | Light silver metal | **`0x7EB0`** | `(128, 168, 248)` | Luminous cosmic cyan-violet highlight |
| `8` | `0x6183` | `(24, 96, 192)` | Bright MegaMan blue | **`0x5CEB`** | `(88, 56, 184)` | Bright violet-indigo armour |
| `9` | `0x4922` | `(16, 72, 144)` | Mid MegaMan blue | **`0x4488`** | `(64, 32, 136)` | Mid dark-violet armour |
| `10` | `0x2CA2` | `(16, 40, 88)` | Dark MegaMan blue | **`0x3045`** | `(40, 16, 96)` | Deep indigo-violet shadow |
| `11` | `0x1CA6` | `(48, 40, 56)` | Dark gunmetal | **`0x2024`** | `(32, 8, 64)` | Darkest violet-charcoal |
| `12` | `0x739C` | `(224, 224, 224)` | Near-white highlight | **`0x7F73`** | `(152, 216, 248)` | Bright cyan-white energy highlight |
| `13` | `0x033E` | `(240, 200, 0)` | Bright gold visor/crest | **`0x037F`** | `(248, 216, 0)` | Radiant Duo gold emblem/eyes |
| `14` | `0x398D` | `(104, 96, 112)` | Mid gunmetal armour | **`0x48C9`** | `(72, 48, 144)` | Mid violet-steel armour |
| `15` | `0x2909` | `(72, 64, 80)` | Shaded gunmetal | **`0x3466`** | `(48, 24, 104)` | Shaded violet-steel |

**Exact 32-byte `.hword` table (`duo_soul_pal` in free space):**
```arm
duo_soul_pal:
    .hword 0x0020, 0x0000, 0x0000, 0x1043, 0x149F, 0x0C36, 0x0810, 0x7EB0
    .hword 0x5CEB, 0x4488, 0x3045, 0x2024, 0x7F73, 0x037F, 0x48C9, 0x3466
```
- **Sprite redirect hook (`BattleGetFormSprite`, `0x0800B90A`, RS == BM):** When `r0 == 0` and `r1 == 13`, redirect `r3 = 9` (`list[0][9]` MetalSoul) instead of `13` (which would read out of bounds into `list[1][0]`).
- **Palette override:** In `BN4_FORM_ENTERS[13]` (or in the battle sprite palette-load helper when `NaviByte(0x24) == 13`), DMA/copy the 32 bytes from `duo_soul_pal` over MegaMan's OBJ palette slot in `0x02037A00` / OBJ PAL RAM (`0x05000200 + slot*0x20`).

---

## 2. Charge Shot (`BN4_FORM_LOADOUTS[13]` & `BN4_POWER_ATTACKS[0x19]`) (V)

- `BN4_FORM_LOADOUTS` (`0x080184F0`, RS == BM) already has entry `[13]` = `FF 00 FF 19` (**charge weapon ID `0x19`**).
- `BN4_POWER_ATTACKS[0x19]` (`0x0800CA7C + 0x19*4 = 0x0800CAE0`, RS == BM) currently points to `0x0800CC2F` (a stub plain buster shot).
- **Recommended default — "Meteor Knuckle" (repoint `0x0800CAE0` → `duo_soul_charge_shot | 1`):**
  - Uses the exact same 12-instruction template as `MetalFist` (`0x0800CE28`, action `0x14`), so it **matches MetalSoul's fist-slam animation frames 100% with zero sprite glitches**, upgraded to Duo's signature power:
    - `[r7, #0x08]` = **`180` (`0x00B4`) damage** (MetalFist is 150)
    - `[r7, #0x0A]` = `0x00A8` (MetalFist/GutsPunch SFX)
    - `[r7, #0x0C]` = **`0x00000808`** (Guard-Break + Crack Panel)
    - Returns `r0 = 0x14` (heavy fist-slam action)
  - *(Alternative pure-chip option: `duo_soul_charge_shot` calls `BN4_CHIP_ATTACK` (`0x0800D406`) with `r0 = 0x00A3` (`Meteors1`, raining meteors) or `r0 = 0x00D2` (`ShotStar`) — 5 instructions total!)*

---

## 3. Passive Ability (`BN4_FORM_ENTERS[13]`) (V)

- `BN4_FORM_ENTERS` (`0x0800E0A0`, RS == BM) entry `[13]` is at **`0x0800E0D4`** (currently `0x0800E0E1` = `bx lr` no-op).
- Other Souls' `BN4_FORM_ENTERS` routines set MegaMan's form passives by calling native `code_00` helpers (all RS == BM):
  - `0x0800E1CA(side)` — **SuperArmor** (used by GutsSoul `0x0800E0E4`)
  - `0x0800E208(side)` — **FloatShoes** (used by WindSoul `0x0800E0F4`)
  - `[r5+0x58]->[0] = 0x6A` — **B+Left Reflect Shield** (used by ProtoSoul `0x0800E12C`)
- **Recommended default (repoint `0x0800E0D4` → `duo_soul_enter | 1`):** **SuperArmor + B+Left Reflect Shield** (evoking Duo's immovable armor and meteor barrier; automatically cleared when the Soul expires):
  ```arm
  duo_soul_enter:
      push    r4, r14
      ldrb    r0, [r5, 0x16]          ; side (0 = MegaMan)
      ldr     r3, =0x0800E1CB         ; SetSuperArmor | 1 (RS == BM)
      bl      @@call_r3
      ldr     r0, [r5, 0x58]
      mov     r1, 0x6A                ; B+Left Reflect Shield
      strb    r1, [r0]
      pop     r4, r15
  @@call_r3:
      bx      r3
  ```

---

## 4. Unite Chips (Class `0x0D` in `BN4_CHIP_TABLE`) (V, R)

Unite class `0x0D` is **100% unused** across all 389 chips in both ROMs. Relocate `BN4_SOUL_KINDS` (`0x08020008`, 13 bytes: append `0x0D`) and `BN4_SOUL_FLAGS` (`0x08020018`, 13 bytes: append `0x23`), and change `0x0801FFC6` (`movs r3, #0xb`) → `movs r3, #0xc`.

### Recommended `chip[+7] = 0x0D` edits (currently all class `0x0C` non-uniting Null chips — zero conflicts with any other Soul!)
`BN4_CHIP_TABLE` base = `0x080197EC` (RS == BM, `0x2C` bytes/entry; edit byte at `0x080197EC + chip_id * 0x2C + 7`):

| Chip ID | Name | Current `chip[+7]` | New `chip[+7]` | Why |
|---:|---|---:|---:|---|
| `0x058`–`0x05A` | `Guard1`, `Guard2`, `Guard3` | `0x0C` | **`0x0D`** | Defensive counter-attack (Duo's justice barrier) |
| `0x087` | `Barrier` | `0x0C` | **`0x0D`** | Energy barrier |
| `0x08B` | `HolyPanl` | `0x0C` | **`0x0D`** | Justice / sanctuary panel |
| `0x090`, `0x091`, `0x093` | `AntiNavi`, `AntiDmg`, `AntiRecv` | `0x0C` | **`0x0D`** | Counter / judgement chips |
| `0x0C9` | `LifeAura` | `0x0C` | **`0x0D`** | Ultimate barrier |
| `0x137` | `Duo` | `0x0C` | **`0x0D`** | Duo's own Giga/Secret chip |

*(Note: `Meteors1/2/3` `0x0A3..0x0A5` and `ShotStar` `0x0D2` are Fire-element `cls=0x00` FireSoul chips; keeping them on FireSoul avoids stealing FireSoul's unite pool while giving DuoSoul a strong, distinct 10-chip Null/Defence/Counter pool!)*

---

## 5. Third Status Screen Page (Extending `wp_d4_status_two_pages.asm`) (V)

`wp_d4_status_two_pages.asm` uses `SOUL_BUF + 0x18` (`0x0201D018`) as the page offset (`0` or `6`). Extending to **13 Souls (3 pages: offsets `0`, `6`, `12`)** takes 4 small edits:
1. **`FS_STATUS_SOULS` (13 flags + `0xFF`):** Append `0x23` before `0xFF` (`0x17..0x23, 0xFF`).
2. **`FS_STATUS_SOULS + 0x10` (anims) & `+ 0x20` (pals):** Append `0x08` (MetalSoul icon anim, or a custom anim) and `0x02` (violet/dark palette index) as entry `[12]`.
3. **Page cycle in `StatusSoulPageToggle`:** Replace `eor r0, 6` with a 3-way cycle:
   - On **R** (`0x0100`): `page = (page == 12) ? 0 : page + 6` — **skip page 12 if `[SOUL_BUF + 12] == 0xFF`** (i.e. if < 13 Souls are owned, wrap `6 -> 0` just like today!).
   - On **L** (`0x0200`): `page = (page == 0) ? (has_13th ? 12 : 6) : page - 6`.
4. **`wp_d4_grant_soul.asm` (`FC 0C`):** Raise bounds check `cmp r0, 12` → `cmp r0, 13` so `FC 0C 0C` grants flag `0x0023` + `WriteChipKey(0x016D)`.

---

## 6. Post-Duo Grant Hook (`DuoDefeatedSetFlag` `sub_080C4780`) (V)

Verified disassembly of `DuoDefeatedSetFlag` (`RS 0x080C4780 / BM 0x080C478C`):
```arm
; RS 0x080C478E..0x080C4795 (BM 0x080C479A..0x080C47A1):
    movs r0, #0
    movs r1, #0x6f
    bl   SetEventFlag               ; RS 0x08025048 / BM 0x0802504C (sets flag 0x006F)
    bl   sub_08010D4C               ; RS 0x080C4796 / BM 0x080C47A2
```
- **Exact hook site:** Replace `bl SetEventFlag` at **`RS 0x080C4792 / BM 0x080C479E`** (4 bytes) with `bl` to a veneer (or directly to `duo_soul_post_duo_grant` if in range; `code_07` → free space needs an 8-byte veneer in `code_07` dead padding, or hook `RS 0x080C4796 / BM 0x080C47A2` `bl sub_08010D4C`).
- **Handler (`duo_soul_post_duo_grant` in free space):**
  ```arm
  duo_soul_post_duo_grant:
      push    r4, r14
      ; 1. Original call: SetEventFlag(0x006F) (r0=0, r1=0x6F on entry)
      ldr     r3, =SET_EVENT_FLAG_PAIR | 1    ; RS 0x08025049 / BM 0x0802504D
      bl      @@call_r3
      ; 2. Grant DuoSoul: SetEventFlag(0x0023) + WriteChipKey(0x016D)
      mov     r0, 0x23
      ldr     r3, =SET_EVENT_FLAG | 1         ; RS 0x08005E11 / BM 0x08005E11
      bl      @@call_r3
      ldr     r0, =0x016D
      ldr     r3, =WRITE_CHIP_KEY | 1         ; RS 0x0801911D / BM 0x0801911D
      bl      @@call_r3
      pop     r4, r15
  @@call_r3:
      bx      r3
  ```
- **Meteor Comp rematch grant:** In `wp_d3_duo_rematch.asm`'s post-battle victory check, also call `SetEventFlag(0x0023)` + `WriteChipKey(0x016D)` if `!TestEventFlag(0x0023)` (idempotent).

---

## 7. `0x16D` (`DuoSoul`) Chip Icon Pointer & Format (V, R)

- **Location:** `BN4_CHIP_TABLE[0x16D] + 0x20` = **`0x0801D788`** (RS == BM, 4-byte ROM pointer).
- **Current value:** `RS 0x087400F8 / BM 0x0873FBEC` (the blank placeholder chip icon, shared with `0x16E FrteSoul`).
- **Format:** **Uncompressed `0x80` bytes (128 bytes) = 4 GBA 4bpp tiles (16×16 px, 2×2 tiles in row-major order)** using the standard chip-icon 16-color palette (same format as the 12 Soul icons at `RS 0x087478F8..0x08747E78`, 128 bytes each).
- **Drop-in plan:** Place a `0x80`-byte Duo emblem 4bpp icon at `FS_DUO_SOUL_ICON` and repoint the word at `0x0801D788` (RS == BM) to `FS_DUO_SOUL_ICON` (or temporarily point it to `RS 0x08747CF8 / BM 0x087477EC` MetalSoul icon until custom 16×16 pixel art is drawn).

---

## 8. Proposed Names (for `decomp/symbols/names.txt`)

```
0800E1CA 0800E1CA code   GEMINI       BattleSetSuperArmor              r0 side: sets SuperArmor passive flag on Navi (called by GutsSoul enter)
0800E208 0800E208 code   GEMINI       BattleSetFloatShoes              r0 side: sets FloatShoes passive flag on Navi (called by WindSoul enter)
08025048 0802504C code   GEMINI       SetEventFlagPair                 r0 hi_byte, r1 lo_byte: sets event flag (r0<<8 | r1) (called by DuoDefeatedSetFlag)
```
