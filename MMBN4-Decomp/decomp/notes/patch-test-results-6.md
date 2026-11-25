# Patch Test Results 6 — Round 13 PC Port Smoke Tests (2026-11-24)

**Build tested:** `build/bn4_rs_overhaul.gba` (SHA-1 `5f3c23b7a86df6d4d063d2002a61d9c9776f6e25`) via `build/pc-arm64/Release/mmbn4.exe` (both RS & BM pass `python patch/build.py` and `python patch/build.py --check`). All test artifacts live in the local scratch folder (`scratch/r13/`); zero ROM or save bytes committed.

## Summary

| Feature / Area | Result | Evidence |
|---|---|---|
| **WP-A3 Key-Item Shop Prices (New Save)** | **PASS** | Brand-new save (`r13_blank.sav` -> `NewGameInit`) initialized EWRAM save slot `+0x58` with exact 40% prices: ACDC Area 2 HPMemry `400Z`/`1300Z`, Town Area 3 HPMemry `1000Z`, Undernet 4 HPMemry `20000Z`, ACDC Town LocEnemy `4000Z`, Park Area 1 Unlocker `1600Z`, Castillo HP `HP+100` `1400Z`, Undernet 6 `Reflect` `5200Z`. All 34 slots verified byte-exact in both RS & BM ROMs. |
| **WP-B1 Friendly Matches (9 NPCs, 3 venues)** | **PASS (after `b4=0x3D` fix)** | Verified all 3 venues (`DenDome Hall 02,01`, `Castillo WaitRm 03,02`, `Colosseum Lobby 04,06`), tournament-round gating (visible in `r=0/1`, hidden in `r=2`), **Loss retry** (`map=(02,01)`, no Game Over, `FireSoul=False`, `marker=00000000`, retryable), **Win Soul grant** (`DoubleSoul 0x0015=True`, `FireSoul 0x001B=True`, `marker` cleared), and **Hide once owned** (Match disappears in DenDome Hall once `0x001B=True`). |
| **ACDC Town Debris-Pile Trash Can (`list[7] 0x4A`)** | **PASS** | Renders cleanly on ACDC Town street at `(-150, -90)` beside Higsby's (`t2d_ow.bmp`), and talking to it warps cleanly to `WtrGodComp1 (8E, 00)`. |
| **Duo Rematch S-Rank Climb (`wp_d3_duo_rematch.asm`)** | **PASS** | Duo V1 S-rank climbed `0x0E5A: False -> True` (V1 → α) and granted DuoSoul `0x0023` + Duo D `0x0E72` (`marker=00000000`). DuoΩ (`3500 HP`) S-rank set `0x0E5C = True` (unlocked DuoΩ SP+, `marker=00000000`). |
| **Regression: Status Pages 1–3 (13 Souls)** | **PASS** | Opened PET → MegaMan → Status with all 13 Souls (`0x0017..0x0023`); pressed R twice: Page 1 (6 Souls), Page 2 (6 Souls), Page 3 (Duo Soul in top-left with MetalSoul icon), zero visual glitches. |
| **Regression: Frequency Tuner (Higsby's)** | **PASS** | Talked to Tuner at `(-100, 20)` in Higsby's (`00,06`): toggled `0x0E58: False -> True`, `0x0E59=False` cleanly. |
| **Regression: Lobby Rematches & SP+ (`FireMan SP -> SP+`)** | **PASS (after `b4=0x3D` fix)** | S-ranking FireMan SP (`2000 HP`, `2.14s`, Busting LV. S) unlocked `0x0E5E = True` and cleared `marker=00000000`. Next rematch spawned FireMan SP+ with **`4095 HP`** (`0x020015EE = 1`), confirmed ATK ×1.5 (277 damage hit) and Speed ×1.25 (faster movement), and rolled **`0x7A98` (15,000 Zenny)** on the results screen (80/20 SP+ reward). |

## Bugs Caught & Fixed During Smoke Testing

1. **Critical (`a6cf221`): `b4 = 0x3F` skipped the results screen in both `wp_b1_friendly_matches.asm` and `wp_d3_lobby_rematches.asm`.**
   - `BattleModeFlags[0x3F] = 0x0039` has bit 1 (`0x02`, show results screen) = `0`, so winning a friendly match or lobby rematch jumped straight back to the overworld without calling `lobby_results_hook` (no Soul grant, no tier climb, no SP+ unlock, no 80/20 reward).
   - **Fix:** Changed `mov r3, 0x3F` to **`mov r3, 0x3D`** (`BattleModeFlags[0x3D] = 0x003B`: identical boss rank + retryable/no-Game-Over flags, plus bit 1 = show results screen ON) in both files. Re-tested both flows end-to-end: **PASS**.
2. **Placement (`a6cf221`): JunkMan's Colosseum signal crystal (`fm_npc_junk`)** moved from `(56, 36)` (overlapping the stone wall) to **`(36, 48)`** (cleanly on walkable grass in `t5_fm_t2_r0.bmp`).

## Known Visual Polish Item — RESOLVED (`ede0993`)

- **LZ77-compressed character sprites (`Higsby 0x08` and `Sal 0x2C`) — FIXED & PASS in-game:**
  - Added `wp_d0_sprite_lazy.asm` (`0x08002894` via `VENEER_SPRITE_LAZY` `0x0800178C`), lazily decompressing `0x1808` (Higsby) and `0x182C` (Sal) into overworld `BN4_SPRITE_CACHE` (`0x0200F720`, `0x02038800`–`0x02040000`) via BIOS `swi 0x11`.
  - Visually confirmed zero white circles: Higsby in DenDome Hall (`r14_dendome_higsby.bmp`), Sal in Castillo WaitRm (`r14_castillo_sal.bmp`), and Sal in Higsby's (`r14_higsby_sal.bmp`).

## Follow-Up Smoke Tests (`ede0993` + post-Round 13)

| Feature / Package | Result | Evidence |
|---|---|---|
| **WP-A3 Existing-Save Shop Sync (`SaveLoad` hook)** | **PASS** | Vanilla save prices migrated idempotently on load (`Shop 3,1` HPMemry `50,000Z -> 20,000Z`, `Shop 7,2` LocEnemy `10,000Z -> 4,000Z`, `Shop 6,2` Unlocker `4,000Z -> 1,600Z`, `Shop 14,0` HP+100 `3,400Z -> 1,400Z`, `Shop 15,5` Reflect `13,000Z -> 5,200Z`); shifted already-bought slots untouched. |
| **WP-D2 Grand Prix Power BMD + e-Reader Chip Text** | **PASS** | ACDC Area 2 BMD at `(106, 326)` opened cleanly, displayed `"GP Power G"`, set `0x0D0B = True` and Library flag `0x16C0 + 0x138 = True` (`r14_gpp_open.bmp`). |
| **WP-B4 GMD Tier Override (`wp_b4_gmd_tier.asm`)** | **PASS** | On PT1 (`playthrough = 0`), opening ACDC Area 2 GMD `0x0D09` at `(206, 290)` with identical RNG seed gave `"300 Zennys"` on **Auto** (`r15_gmd_auto_res.bmp`) vs `"700 Zennys"` on **Tier 3** (`r15_gmd_t3_res.bmp`). |
| **WP-D3 Roaming SP+ Unlock (`wp_d3_sp_plus.asm`)** | **PASS** | Non-lobby S-rank (`0x020015EF = 0`, `0x020015EC = 0xE5` BassΩ) set `0x0E71 = True` and cleared `marker = 00000000`. |
