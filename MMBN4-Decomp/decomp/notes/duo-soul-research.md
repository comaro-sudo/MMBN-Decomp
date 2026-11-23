# Duo Soul (13th Double Soul) Framework Research (Round 9, Task 1, 2026-11-23)

All addresses and tables below are **verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba`**.

---

## 1. Pseudo-Chip `0x16D` (`DuoSoul`) (V, R)

Located in `BN4_CHIP_TABLE` (`0x080197EC`, identical in RS & BM, `0x2C` bytes per entry) and Chip Names Bank 1 (`RS 0x08748C14 / BM 0x08748708`):

| Field | Value (RS US / BM US) | Notes |
|---|---|---|
| Name (Bank 1 idx `0x6D`) | **`DuoSoul`** | Already present in both RS and BM (right beside `0x16E FrteSoul` and `0x16F DarkSoul`) |
| Codes (`+0x00..0x03`) | `00 01 02 1A` (`A B C *`) | Same as all 12 Soul pseudo-chips `0x161`–`0x16C` |
| Byte `+0x07` (unite class) | `0x0F` (Soul pseudo-chip) | `sub_0800B924` automatically computes `soul_id = chip_id - 0x160 = 13` (`0x0D`)! |
| Damage (`+0x1A`) | `40` (`0x0028`) | Same as `0x161`–`0x16C` |
| Icon ptr (`+0x20`, `0x80` B) | `RS 0x087400F8 / BM 0x0873FBEC` | **Placeholder blank chip icon** (shared with `0x16E FrteSoul`; `0x161`–`0x16C` have unique icons at `RS 0x087478F8..0x08747E78`) |
| Image / Palette (`+0x24..0x28`) | `RS 0x0870CED8 / BM 0x0870C9CC` | Same ROM placeholder as all 12 Soul pseudo-chips (Soul pseudo-chips are never drawn as full chip cards) |

**Verdict:** `0x16D` is **NOT** RAM-backed like the Duo/GP Power chips — it is a complete ROM pseudo-chip record, missing only a `0x80`-byte Soul icon.

---

## 2. Soul Tables & Flag `0x0023` (V, R)

### 2.1 Unite check & tables (`code_01.s`, RS == BM)
- **`BN4_UNITE_CHECK` (`sub_0801FF84`):**
  - Reads the hovered chip's unite class `r2 = [chip + 7]`.
  - Scans `BN4_SOUL_KINDS` (`dat_08020008`, 12 bytes: `04 08 0A 07 00 02 06 05 0B 09 01 03`) with `movs r3, #0xb` (indices `0..11`).
  - Checks `BN4_SOUL_FLAGS` (`dat_08020018`, 12 bytes: `0x17..0x22`, already patched to all 12 in `wp_d4_twelve_souls.asm`) via `TestEventFlag`, and checks the per-battle "already used this Soul" bitmask at `0x0203BF88` (`1 << r3`, a 32-bit word — plenty of room for bit 12!).
- **Room for a 13th Soul:**
  - `BN4_SOUL_KINDS` and `BN4_SOUL_FLAGS` are tightly packed (12 bytes each), so adding a 13th Soul requires relocating both 13-byte tables to free space and changing `movs r3, #0xb` (`0x0801FFC6`) → `movs r3, #0xc`.
  - **Unused unite class `0x0D`:** Across all 389 chips, `chip[+7]` uses `0x00`–`0x0B` (the 12 Souls), `0x0C` (149 non-uniting Null chips), `0x0E` (32 Program Advances), and `0x0F` (17 Soul pseudo-chips). **Class `0x0D` has 0 chips** — completely free to assign to a curated set of Duo-themed chips (e.g. `MeteorEarth`, `Duo`, `AntiDmg`, etc.) so `BN4_SOUL_KINDS[12] = 0x0D` works natively without touching any other Soul!
- **Flag `0x0023`:** **100% FREE** — zero code references in `decomp/asm/` and `0` across all 17 community saves.

### 2.2 Core battle Soul tables (`code_00.s` / `code_01.s`, RS == BM) — **All 16 Entries (`0..15`) Already Exist!**
Every core battle table indexed by `NaviByte(0x24)` (`0` = Normal, `1..12` = 12 Souls, **`13` = DuoSoul**, `14` = ForteSoul, `15` = DarkSoul) **already has a live 16-entry layout**:

| Table | Address (RS == BM) | Entries | Entry `[13]` (`DuoSoul`) |
|---|---|---:|---|
| `BN4_FORM_LOADOUTS` (charge shot) | `0x080184F0` | 16 × u32 | **`0x19FF00FF`** (unique charge-shot weapon ID `0x19` already present; Normal=`0x01`, Forte=`0x1A`) |
| `BN4_FORM_DURATIONS` | `0x08018088` | 16 × u16 | **`0x0258`** (600 = standard 3 turns, same as all 12 Souls) |
| `BN4_FORM_ELEMENTS` | `0x0800C964` | 16 × u8 | **`0x00`** (Null element) |
| `BN4_FORM_ENTERS` (passive init) | `0x0800E0A0` | 16 × fn ptr | **`0x0800E0E1`** (`sub_0800E0E0` = safe `mov pc, lr` no-op; can be repointed to custom DuoSoul passive) |
| Companion / aura table | `0x08018068` | 16 × u16 | **`0x0000`** (none; safe) |

---

## 3. Form Graphics, Sprites & Palettes (V, R)

- **MegaMan battle sprite list `list[0]` (`RS 0x08027964 / BM 0x08027968`):**
  - Contains **exactly 13 entries (`0..12`)**: `[0]` Normal MegaMan (`0x08214000`) + `[1..12]` the 12 Soul forms (`0x0821CB68..0x0827F770`, identical in RS & BM). `list[1]` starts immediately at `0x08027998` (`+13 × 4`).
  - **There is NO DuoSoul battle sprite in the ROM.**
  - **Size constraint:** Each full MegaMan Soul form sprite archive is **~33–40 KB uncompressed**. Remaining ROM free space (`0x087FAA30..0x08800000`) is **~21.5 KB** — a full new uncompressed form sprite would NOT fit without LZ77 compression (~15–20 KB compressed, consuming almost all remaining free space).
- **Sprite loader `sub_0800B90A` (`code_00.s`, RS == BM):**
  - Computes `sprite_idx = base(0) + soul_id` (except `soul_id == 0xF` DarkSoul, which stays on `list[0][0]` Normal MegaMan).
  - For `soul_id == 13`, **`sub_0800B90A` WOULD READ `list[0][13]` (= `list[1][0]`, out-of-bounds crash/garbage!)** unless hooked to redirect `soul_id == 13` to a chosen existing `list[0]` form!
- **Closest existing form to recolour (scope b):**
  - **MetalSoul (`list[0][9]`)** or **GutsSoul (`list[0][2]`)** (heavy armored/melee silhouette fitting Duo's meteoric fist/armor aesthetic) or **ProtoSoul (`list[0][7]`)** — a 32-byte BGR555 Duo palette override (dark violet/indigo armor, crimson crest, gold/cyan highlights) costs **32 bytes + a ~20-byte palette hook**, zero sprite archive space.
- **Duo's battle sprites (`enemy 0xE0`):**
  - Duo (`BN4_ENEMY_STATS[0xE0]`, `0x08011888 + 0xE0*0xC`) is a full-screen boss actor (`0x2A`), so its body sprites aren't drop-in MegaMan sprites, **but Duo's attack objects (Meteor Fist, Laser, Rocket Fist — `kind == 3` attacks)** are already in both ROMs and can be spawned by DuoSoul's charge shot.

---

## 4. Abilities & Crash Safety (V)

- **Passive init (`BN4_FORM_ENTERS[13]` = `0x0800E0E0`):** Safe `bx lr` no-op — **does not crash**. Repointing `0x0800E0A0 + 13*4` lets DuoSoul set any passive flags (e.g., SuperArmor + FloatShoes + Null chip +20/+30 atk) cleanly on unite.
- **Charge shot (`BN4_FORM_LOADOUTS[13]` = `0x19FF00FF`):** Already has a valid table slot at `0x080184F0 + 13*4` (no out-of-bounds crash); can be edited in-place to any existing buster/charge attack ID (or a custom Duo fist/laser charge shot).
- **Per-Soul inline checks (`code_00.s` / `code_08.s`):** Inline `cmp r0, #1..#0xc` checks (e.g. RollSoul heal, ProtoSoul sword charge, WoodSoul grass heal) safely fall through when `r0 == 13`.

---

## 5. UI Places Assuming 12 Souls (V)

1. **Custom Screen (`BN4_UNITE_CHECK` `0x0801FF84`):** Hardcoded `movs r3, #0xb` (12 Souls); `0x0203BF88` bitmask is 32-bit (supports up to 32 Souls). The Custom Screen only displays the **one** Soul matching the currently highlighted chip, so there is no fixed-slot overflow on the Custom Screen — only the `0x16D` icon (`0x80` B) and unite animation banner need a valid entry.
2. **Status Screen (`wp_d4_status_two_pages.asm`):** Currently 2 pages × 6 icons = 12 Souls (`FS_STATUS_SOULS` has 12 entries). A 13th Soul requires either a 3rd page (1 icon) or a dedicated badge/slot on Page 1/2.
3. **Completion Star (`wp_d4_star_twelve_souls.asm`):** Currently checks 12 flags (`STAR_SOUL_COUNT_CMP = 12`); optional whether DuoSoul counts as a bonus 13th Soul or requires 13/13.
4. **Soul Grant (`wp_d4_grant_soul.asm`):** `FC 0C` currently bounds-checks `idx 0..11`; extending to `0..12` (flag `0x0023` + `WriteChipKey(0x16D)`) is a 2-line edit.

---

## 6. Conclusion & Work Estimates

The wiki is right: **the entire core battle framework for a 13th Soul (`soul_id = 13`, `0x16D DuoSoul`, 16-entry loadout/duration/element/enter tables, free flag `0x0023`, free unite class `0x0D`) already exists in both RS and BM.** Only the battle sprite (`list[0]`), `0x80`-byte icon, unite table entry, and UI slot are missing.

| Scope | Feasibility | Estimate | Notes |
|---|---|---|---|
| **(a) Full Duo Soul with new battle sprite art** | **Tight / High risk** | **L–XL** | Requires drawing a full MegaMan animation sheet (~35–40 KB uncompressed) + LZ77-compressing it into the ~21.5 KB remaining free space (tight fit) + relocating `list[0]` + icon + abilities + UI. |
| **(b) Palette-swap of an existing Soul form (e.g., MetalSoul/GutsSoul) + Duo-themed abilities** | **Fully feasible (Recommended)** | **M** | Only needs: 1-hook `sub_0800B90A` redirect (`13 -> MetalSoul/GutsSoul`), 32-byte Duo palette override, `0x80`-byte Soul icon for `0x16D`, relocated 13-entry `BN4_SOUL_KINDS`/`BN4_SOUL_FLAGS` (`class 0x0D`, flag `0x0023`), `BN4_FORM_ENTERS[13]` passive + `BN4_FORM_LOADOUTS[13]` charge shot, and Status Screen/grant extension. Uses < 1 KB of free space. |
| **(c) Not feasible** | N/A | — | Not the case; (b) is very clean and low-risk. |
