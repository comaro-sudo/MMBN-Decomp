# Patch runtime test results — WP-D4, WP-A2, WP-D3, WP-D2 (2026-11-23)

Live runtime verification of the four patch packages built via `python patch/build.py` (after `79398a4` fixed WP-D4's `cmp r5, #6` → `cmp r5, #12` at `0x08000D56`):
- `build/bn4_rs_overhaul.gba`: SHA-1 `4234a59ec06af902214051adbefd1ec6a239634f` (11,234 bytes differ, BPS round-trip OK)
- `build/bn4_bm_overhaul.gba`: SHA-1 `6e05e02e05c4379d42b05ad114f9dd143e726232` (11,234 bytes differ, BPS round-trip OK)

All tests were executed headless on `build/pc-arm64/Release/mmbn4.exe` (`--test`, `--shot`, `--dump`, `--state-in`, `--state-out`) with all screenshots, save copies, save states, and EWRAM dumps kept outside the repo.

---

## Summary

| Package | File | Result | Key evidence |
|---|---|---|---|
| **WP-D4** (12-Soul star) | `wp_d4_star_twelve_souls.asm` | **PASS (RS & BM)** | RS 6 Souls → no `SOUL COMPL` (`35688` px); RS 12 Souls → `SOUL COMPL` shown (`35705` px). BM 6 Souls → no star (`32273` px); BM 12 Souls → star shown (`32287` px). |
| **WP-A2** (Battle Zenny × 1.5) | `wp_a2_zenny_battle.asm` | **PASS** | Results screen shows `50 Z` (vanilla) vs **`75 Z`** (patched); EWRAM Zenny (`blk + 0x74`) goes `1660 → 1710 (+50)` (vanilla) vs **`1660 → 1735 (+75)`** (patched). Shown == granted == `1.5 × vanilla`. |
| **WP-D3** (Bass drops both chips) | `wp_d3_bass_both_chips.asm` | **PASS** | Both Bass V1 (`BassAnly X` → `+Bass X`) and BassΩ (`DeltaRay Z` → `+BlakBarr A`) set **both** Library flags (`0x16C0 + id`) `False → True` and increment **both** pack slots `0 → 1`. |
| **WP-D2** (Random version drops) | `wp_d2_random_version_drops.asm` | **PASS** | Billy (`0x0D`) over 30 random RNG seeds on BM: vanilla drops `6 Thunder1 P, 0 Thunder1 L`; patched drops **`4 Thunder1 P, 4 Thunder1 L`** (exact 50/50). |

---

## 1. WP-D4 — Souls completion star requires 12 (`wp_d4_star_twelve_souls.asm`) — **PASS**

- **Setup:**
  - **Red Sun:** `build/saves/mega-man-battle-network-4.4752.sav` (6 Souls: Roll, Guts, Wind, Search, Fire, Thunder) vs. a scratch copy (`rs_12souls.sav`) with all 12 Soul flags `0x17`–`0x22` set and checksum re-signed (`+0x16`).
  - **Blue Moon:** `build/saves/mega-man-battle-network-4.4569.sav` (6 Souls: Proto, Number, Metal, Junk, Aqua, Wood) vs. scratch copies (`bm_6souls.sav`, `bm_12souls.sav`, checksum `+0x22`).
  - Booted to the Continue menu (`--test 720 --input "600=START,606="`) and visually inspected the screenshots.
- **Results:**
  - `bn4_rs_us.gba` (vanilla control) + 6 Souls: `nonblack_pixels=35705` — **`SOUL COMPL` shown**.
  - `bn4_rs_overhaul.gba` + 6 Souls: `nonblack_pixels=35688` — **`SOUL COMPL` absent** (confirmed visually in `wpd4_rs_6souls_menu.bmp`).
  - `bn4_rs_overhaul.gba` + 12 Souls: `nonblack_pixels=35705` — **`SOUL COMPL` shown** (confirmed visually in `wpd4_rs_12souls_menu.bmp`).
  - `bn4_bm_overhaul.gba` + 6 Souls: `nonblack_pixels=32273` — **`SOUL COMPL` absent**.
  - `bn4_bm_overhaul.gba` + 12 Souls: `nonblack_pixels=32287` — **`SOUL COMPL` shown**.
- **Coverage:** Full end-to-end title-screen check on both RS and BM; no RAM edits needed.

---

## 2. WP-A2 — Battle Zenny × 1.5 (`wp_a2_zenny_battle.asm`) — **PASS**

- **Setup:**
  - Triggered a real random encounter in `(90, 00)` on `4420.sav` (Blue Moon, PT1, initial Zenny = `1660`), defeated all 3 viruses (Busting LV 5 → reward word `0x4032` = `50Z`), and saved state `bm_win_f750.ss` right at `GET DATA — PRESS A BUTTON`.
  - Ran `bm_win_f750.ss` on both `bn4_bm_us.gba` (vanilla) and `bn4_bm_overhaul.gba` (patched): pressed `A` to reveal `GET DATA` (captured screenshot at +100 frames), then pressed `A` to accept and exit to the overworld (dumped EWRAM at +260 frames, locating the shifted save block via `ROCKMANEXE4` magic).
- **Results:**
  - **Vanilla (`bn4_bm_us.gba`):**
    - Results screen (`wpa2_vanilla_revealed.bmp`): **`50 Z`**
    - EWRAM Zenny (`blk + 0x74`): `1660` (`blk=0x0200218C`) → **`1710`** (`blk=0x020021F8`), delta **`+50`**
  - **Patched (`bn4_bm_overhaul.gba`):**
    - Results screen (`wpa2_patched_revealed.bmp`): **`75 Z`**
    - EWRAM Zenny (`blk + 0x74`): `1660` (`blk=0x0200218C`) → **`1735`** (`blk=0x020021F8`), delta **`+75`**
- **Coverage:** End-to-end battle results reveal + `AddZenny` grant verified (shown == granted == `1.5 × vanilla`). Negative checks (shop prices and Zenny Mystery Data untouched) were verified statically in `wp_a2_zenny_battle.asm` (only `REWARD_DECODE` is modified), not re-run at runtime.

---

## 3. WP-D3 — Bass battle drops both chips (`wp_d3_bass_both_chips.asm`) — **PASS**

- **Setup:**
  - Starting from `bm_win_f750.ss` (paused at `GET DATA — PRESS A BUTTON` before `BASS_PAIR_HOOK` / `CHIP_GIVE` executes), set the reward word copies in EWRAM (`0x0203BD4C` and the results-screen mirrors at `0x020363D8`, `0x020363E4`, `0x0203B840`, `0x0203B844`, `0x0203E39C`, `0x0203E49C`) to:
    1. **Bass V1 drop:** `0x2F32` = **`BassAnly X`** (`id=306, code=23`)
    2. **BassΩ / BassXX drop:** `0x332E` = **`DeltaRay Z`** (`id=302, code=25`)
  - Ran `bn4_bm_overhaul.gba` through `A` (reveal screenshot) and `A` (grant + exit to overworld), then compared EWRAM Library flags (`blk + 0x118`, bit `0x16C0 + id`) and pack counts (`blk + 0x500..0x3A00`) pre- vs. post-grant.
- **Results:**
  - **Bass V1 (`0x2F32` `BassAnly X`):**
    - Visual reveal (`d3b_306_revealed.bmp`): displays **`BassAnly X`** with chip art.
    - Library flags: **`BassAnly` (`306`) `False → True`** AND **`Bass` (`301`) `False → True`**.
    - Pack increments in `blk`: **exactly 2 slots** incremented `0 → 1` (`blk+0x13D0`, `blk+0x140C`).
  - **BassΩ (`0x332E` `DeltaRay Z`):**
    - Visual reveal (`d3b_302_revealed.bmp`): displays **`DeltaRay Z`** with chip art.
    - Library flags: **`DeltaRay` (`302`) `False → True`** AND **`BlakBarr` (`308`) `False → True`**.
    - Pack increments in `blk`: **exactly 2 slots** incremented `0 → 1` (`blk+0x13DC`, `blk+0x1424`).
- **Coverage:** Full results-screen reveal + `BASS_PAIR_HOOK` → `BassPairGive` → `CHIP_GIVE` × 2 execution verified on the unmodified `bn4_bm_overhaul.gba` ROM for both Bass V1 and BassΩ pairs. Playing through the live Black Earth 2 Bass encounter itself was bypassed via save-state reward-word setup; RS-side directions (`Bass X → +BassAnly X`, `BlakBarr A → +DeltaRay Z`) share the same 4-entry `@@pairs` loop in `wp_d3_bass_both_chips.asm`.

---

## 4. WP-D2 — Random version drops (`wp_d2_random_version_drops.asm`) — **PASS**

- **Setup:**
  - Starting from `bm_win_f600.ss` (all 3 enemies deleted, immediately before `BN4_REWARD_PICK` → `RewardGetRowPtr` executes), replaced the defeated-enemy species list (`0x02035870..0x02035872` and enemy structs `0x0203B267`, `0x0203B33F`, `0x0203B417`) with **`0x0D` (Billy)**, whose drop row differs between versions (`Thunder1 L` in Red Sun vs. `Thunder1 P` in Blue Moon; chip id `57`).
  - Ran **N = 30 trials** with identical pseudo-random RNG seeds written to `0x02001790` on both `bn4_bm_us.gba` (control) and `bn4_bm_overhaul.gba` (patched), reading `BN4_BATTLE_REWARD` (`0x0203BD4C`) from each EWRAM dump.
- **Results (30 seeds):**
  - **Vanilla (`bn4_bm_us.gba`):** `50Z`: 14, `100Z`: 10, **`Thunder1 P` (BM): 6**, **`Thunder1 L` (RS): 0** (100% BM-only).
  - **Patched (`bn4_bm_overhaul.gba`):** `50Z`: 14, `100Z`: 8, **`Thunder1 P` (BM): 4**, **`Thunder1 L` (RS): 4** (exact 50/50 split across both versions' drop codes).
- **Coverage:** Full `BN4_REWARD_PICK` → `RewardGetRowPtr` → `RandomRowPtr` → `FS_OTHER_REWARD_ROWS` path verified across 30 RNG seeds against vanilla control. Enemy species was substituted in the post-kill save state rather than walking into 30 separate Billy encounters.
