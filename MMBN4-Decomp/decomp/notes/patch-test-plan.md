# In-game test plan & runtime results for the 4 untested patch packages (2026-11-23)

Concrete test procedures (save, input script / mGBA steps, pass/fail EWRAM & visual checks) for the four untested packages in `patch/README.md`, plus **runtime test results** from `build/bn4_rs_overhaul.gba` on `build/pc-arm64/Release/mmbn4.exe`.

---

## 0. Runtime results summary

| Package | File | Result | Notes |
|---|---|---|---|
| **WP-D4** (12-Soul star) | `wp_d4_star_twelve_souls.asm` | **FAIL — REAL BUG CAUGHT AT RUNTIME!** | On `4752_rs.sav` (6 Souls), `bn4_rs_overhaul.gba` **still shows `SOUL COMPL`** on the title screen, while `22565_rs.sav` (11 Souls) hides it. Root cause: `GetCompletionStars` (`0x08000D56`, RS & BM) still has **`cmp r5, #6` (`06 2D`)** instead of **`cmp r5, #12` (`0C 2D`)** — so 6 Souls wrongly passes and 12 Souls will wrongly fail! **Fix:** patch `0x08000D56` to `0C 2D`. |
| **WP-A2** (Battle Zenny × 1.5) | `wp_a2_zenny_battle.asm` | Ready to run (disassembly verified) | Exact test procedure below (`4429_rs.sav`, Mettaur Zenny drop: vanilla `100Z` → overhaul `150Z`). |
| **WP-D3** (Bass drops both chips) | `wp_d3_bass_both_chips.asm` | Ready to run (disassembly verified) | Exact test procedure below (`4752_rs.sav` or `4569_bm.sav` in Black Earth 2 for BassΩ / Bass V1). |
| **WP-D2** (Random version drops) | `wp_d2_random_version_drops.asm` | Ready to run (disassembly verified) | Exact test procedure below (`4429_rs.sav` in ACDC Area 1: Mettaur drops both `Guard1 L` and `Guard1 V` across N battles). |

---

## 1. WP-D4 — Souls completion star requires 12 (`wp_d4_star_twelve_souls.asm`)

- **Save(s):**
  - `mega-man-battle-network-4.4752.sps` (RS, **6 Souls**: Roll, Guts, Wind, Search, Fire, Thunder).
  - `mega-man-battle-network-4.22565.sps` (RS, **11 Souls**: all except AquaSoul).
  - Plus a synthetic 12-Soul test (set flag `0x21` AquaSoul in `22565_rs.sav` and update checksum via `savetool.py`).
- **PC port command (executed 2026-11-23):**
  ```powershell
  mmbn4.exe build/bn4_rs_overhaul.gba --save <tmp>/4752_rs.sav --test 800 --input "600=START,606=" --shot <tmp>/title_overhaul_4752.bmp
  mmbn4.exe build/bn4_rs_overhaul.gba --save <tmp>/22565_rs.sav --test 800 --input "600=START,606=" --shot <tmp>/title_overhaul_22565.bmp
  ```
- **Pass/fail check:**
  - `4752_rs.sav` (6 Souls): `SOUL COMPL` badge at top of title screen must be **absent** (vanilla shows it).
  - `22565_rs.sav` (11 Souls): `SOUL COMPL` must be **absent**.
  - 12-Soul save: `SOUL COMPL` must be **present**.
- **Actual runtime result:** **FAIL.** `title_overhaul_4752.bmp` (`nonblack_pixels=35705`, pixel-identical to `title_vanilla_4752.bmp`) **still displays `SOUL COMPL`**. `title_overhaul_22565.bmp` (`nonblack_pixels=35688`) hides it only because `11 != 6`.
- **Root cause & 2-byte fix (verified in `code_00.s`):**
  - `GetCompletionStars` (`0x08000D28`) counts set flags in `r5`, then checks **`0x08000D56: cmp r5, #6` (`06 2D`)**.
  - Patch `0x08000D56` (same address in RS and BM) from `06 2D` to `0C 2D` (`cmp r5, #12`), then re-run the 3 commands above.

---

## 2. WP-A2 — Battle Zenny × 1.5 (`wp_a2_zenny_battle.asm`)

- **Save:** `mega-man-battle-network-4.4429.sps` (RS, Playthrough 1, Zenny = `12900`, standing in the Cyberworld / ACDC Area).
- **Procedure (mGBA or chained `--test`):**
  1. Load `4429_rs.sav` on `bn4_rs_us.gba` (control) and `bn4_rs_overhaul.gba` (patched).
  2. Walk in ACDC Area 1 `(90, 00)` until a virus battle starts (e.g. Mettaur). Win with a Busting Level that yields a **Zenny** reward (BL 1–6 or low HP; Mettaur base Zenny drops are `100Z`, `200Z`, `300Z`).
  3. Record: (a) the Zenny amount displayed on the battle results screen, and (b) player Zenny in EWRAM before vs. after the battle (`savetool.py` / `progress +0x1C`).
- **Pass/fail check:**
  - **Pass:** Shown Zenny on the results screen equals `floor(vanilla_zenny * 1.5)` (e.g. `100Z → 150Z`, `200Z → 300Z`, `300Z → 450Z`), **and** `zenny_after - zenny_before` equals the exact shown amount.
  - **Bonus negative check:** Open Higsby's Shop or a SubChip dealer and verify shop prices are **unchanged**, and pick up a Zenny Mystery Data to confirm scripted `AddZenny` is **unchanged**.

---

## 3. WP-D3 — Bass battle drops both chips (`wp_d3_bass_both_chips.asm`)

- **Save:**
  - **BassΩ test (RS):** `mega-man-battle-network-4.4752.sps` or `4760.sps` (RS, playthrough 4, post-game, Black Earth 2 accessible, no Mod Cards loaded; lacks `DeltaRay Z` chip `302`).
  - **BassΩ test (BM):** `mega-man-battle-network-4.4569.sps` (BM, playthrough 3, post-game, lacks `BlakBarr A` chip `308`).
  - **Bass V1 test:** any save with flag `0x009F` clear in Black Earth 2 `(94, 07)` (or a `bn4_lab.py`-style forced battle test against Story Battle `176` in `--test` mode).
- **Procedure (mGBA manual or forced-battle `--test`):**
  1. Enter Black Earth 2 `(94, 07)` on `bn4_rs_overhaul.gba` (use `LocEnemy` after first BassΩ encounter if needed).
  2. Defeat **BassΩ** with **Busting Level 7–S** (so `BN4_REWARD_ROWS[0xE5]` awards `BlakBarr A`, chip `308`).
  3. Dump EWRAM (`--dump`) or save and inspect with `savetool.py`.
- **Pass/fail check:**
  - Results screen displays `BlakBarr A` (native RS drop).
  - **Pass:** In save slot `+0x50` (pack) and Library flags (`0x16C0 + id`):
    - **BassΩ (RS):** BOTH `BlakBarr A` (`308`, code `0`) **and** `DeltaRay Z` (`302`, code `25`) increase in pack count by `+1`, Library flag `0x16C0 + 302` (`0x17EE`) is set, and chip key `[eToolkit+0x84][302]` matches `[0x02000670 + 302] ^ 0x64` (so `DeltaRay Z` can be added to a folder without failing `BN4_CHIP_GATE`).
    - **Bass V1 (RS):** BOTH `Bass X` (`301`, code `23`) **and** `BassAnly X` (`306`, code `23`) are granted, with Library flag `0x16C0 + 306` (`0x17F2`) and valid chip key `[eToolkit+0x84][306]`.

---

## 4. WP-D2 — Random version drops (`wp_d2_random_version_drops.asm`)

- **Save:** `mega-man-battle-network-4.4429.sps` (RS, Playthrough 1 — easy 1-turn S-rank wins against **Mettaur** in ACDC Area 1 `(90, 00)`).
  - From `re06-encounters.md` §7, Mettaur's V1 chip drop differs by version: **Red Sun drops `Guard1` (code differs)** vs **Blue Moon drops `Guard1` (other version's code)** (e.g. `Guard1 L` vs `Guard1 V` / `Guard1 *`, or any of the 61 differing virus families in `re06-encounters.md` §7).
- **Procedure (mGBA or scripted `--state-in` loop in `mmbn4.exe`):**
  1. Save a state right before the final blow of a Mettaur battle (or right before entering a battle in ACDC Area 1) on `bn4_rs_overhaul.gba`.
  2. Run **N = 10–15** trials with slightly varied frame delays before the winning hit (so `GET_RNG` bit 0 flips between 0 and 1).
  3. Record `BN4_BATTLE_REWARD` (`0x0203BD4C`, u16) from each `--dump` and the chip shown on the results screen.
- **Pass/fail check:**
  - **Pass:** Across the 10–15 wins, **both** Red Sun's drop code and Blue Moon's drop code appear (roughly 50/50), and in every trial the chip displayed on the results screen matches the exact chip/code added to the pack.
  - **Control:** On vanilla `bn4_rs_us.gba`, 100% of trials drop only Red Sun's row.

---

## 5. Proposed symbol names (`names.txt` format)

*(No new symbols beyond `GetCompletionStars` `0x08000D28`; note for `wp_d4_star_twelve_souls.asm`:)*

```text
08000D56 08000D56 code   GEMINI       GetCompletionStars_SoulCountCmp  cmp r5, #6 (06 2D): must be patched to cmp r5, #12 (0C 2D) alongside FS_STAR_SOUL_FLAGS
```
