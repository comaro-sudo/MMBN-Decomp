# Patch Test Results 7 — End-to-End C1–C5 Matrix on BOTH Red Sun & Blue Moon (2026-11-24)

**Builds tested:**
- Red Sun: `build/bn4_rs_overhaul.gba` (SHA-1 `a3e60d9ef16280295cd69a81978991e69fb93ca9`)
- Blue Moon: `build/bn4_bm_overhaul.gba` (SHA-1 `9d4c99fff7c680e7422b05724dda1a1e2627b5f0`)
- Runner: `build/pc-arm64/Release/mmbn4.exe` (`scratch/r13/test_c1_c5_both.py` + `check_bm_tuner.py`). Zero ROM or save bytes committed.

## Matrix Summary: **16 / 16 PASS (8/8 RS, 8/8 BM)**

| Criterion / Test | Red Sun (RS) | Blue Moon (BM) | Evidence |
|---|---|---|---|
| **C1: Status 3 Pages (13 Souls)** | **PASS** | **PASS** | All 13 Souls (`0x0017`–`0x0023`) cycle cleanly Page 1 → Page 2 → Page 3 (Duo Soul) via R |
| **C2: Grand Prix Power BMD + Text** | **PASS** | **PASS** | ACDC Area 2 BMD `(106, 326)` opens cleanly, displays `"GP Power G"`, sets `0x0D0B = True` and Library `0x16C0 + 0x138 = True` |
| **C3a: Duo V1 S-Rank Climb + Grant** | **PASS** | **PASS** | Meteor Comp 1 `(82,00)` Duo V1 S-rank sets `0x0023 = True` (DuoSoul), `0x0E72 = True` (Duo D), `0x0E5A = True` (climbs V1 → α), and clears `0x020015EC..0x020015EF = 0` |
| **C3b: Lobby SP+ (4,095 HP + 80/20 Reward)** | **PASS** | **PASS** | FireMan SP+ spawns with **`4095 HP`** (`0x020015EE = 1`) and rolls **`0x7A98` (15,000 Zenny)** on the results screen |
| **C4: Friendly Match Soul Grant (`b4 = 0x3D`)** | **PASS** | **PASS** | DenDome Hall `(02,01)` Match/FireMan friendly battle grants `DoubleSoul (0x0015) = True` and `FireSoul (0x001B) = True` on win |
| **C5a: WP-A3 New Save 40% Prices** | **PASS** | **PASS** | Fresh New Game initializes save slot `+0x58` with ACDC Area 2 HPMemry `400Z`, LocEnemy `4,000Z`, Undernet 6 Reflect `5,200Z` |
| **C5b: WP-A3 Existing-Save `SaveLoad` Sync** | **PASS** | **PASS** | Vanilla save prices migrate idempotently on load (`20,000Z` HPMemry, `4,000Z` LocEnemy, `1,600Z` Unlocker, `1,400Z` HP+100) |
| **C5c: WP-B2 Frequency Tuner + Sal Sprite** | **PASS** | **PASS** | Higsby's `(00,06)` Tuner toggles `0x0E58: False -> True` (`0x0E59 = False`); Sal (`0x2C`) renders cleanly with zero white circles in both RS & BM |
