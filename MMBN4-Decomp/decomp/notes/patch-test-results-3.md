# Patch Regression Test Results — Round 10 (2026-11-23)

Tested on `mmbn4.exe` (PC ARM64 port) against freshly built `bn4_rs_overhaul.gba` (`SHA-1 c40fa6e46f0193820d629d066524611ad6f6ff75`, 13,802 B diff) and `bn4_bm_overhaul.gba` (`SHA-1 e29dbd9ecbc5373df55514c9c54b92e5c2c040d0`, 13,800 B diff), both passing `python patch/build.py --check`. All screenshots and saves kept outside the repo in scratch.

---

## 1. Lobby After `11d2a39` Fixes

| Check | Result | Evidence / Notes |
|---|---|---|
| **Soul-flag gate (`0x0017`–`0x0022`) — hidden when clear** | **PASS** | `r10_lobby_hide.bmp` (`4752.sav` with `0x17..0x22` cleared): all 9 Higsby's operators completely hidden; only vanilla maid NPC[2] visible |
| **Soul-flag gate — visible with exact sprites when set** | **PASS** | `r10_lobby_higsby.bmp` & `r10_lobby_acdc.bmp`: Chaud (`0x04`), Mayl (`0x01`), Dex (`0x02`), Mr. Match (`0x1D`), Shuko (`0x20`), Lily (`0x34`), Higsby (`0x08`), Raika (`0x3B`), Raoul (`0x32`) all render with their exact character sprites |
| **Trash Can / Junk Pile prop (`list[7] 0x44`)** | **VISUAL BUG (1-byte fix)** | `r10_lobby_acdc.bmp`: renders as a **white circle** instead of the Junk Pile, even though talking to it **PASSES** and warps to JunkMan (`r10_lobby_junkman.bmp`). **Root cause:** `list[7][0x44]` has only 3 animations (`0, 1, 2`), and `lobby_npc_trashcan` sets `.byte 0x16, 3` (`facing = 3`, selecting out-of-range anim 3). **Fix:** change `.byte 0x16, 3` → `.byte 0x16, 0` in `lobby_npc_trashcan` (`wp_d3_lobby_rematches.asm`) |
| **Trash Can warp → JunkMan (`WtrGodComp1 (8E,00)`)** | **PASS** | `r10_lobby_junkman.bmp`: MegaMan warps cleanly into `(8E, 00)` face-to-face with JunkMan (`list[6] 0x5A`) |
| **Lobby rematch battle launch** | **PASS** | `r10_lobby_battle.bmp`: talking to Dex → `"NetBattle? Yes"` → launches **GutsMan V1 (500 HP)** cleanly |

---

## 2. Regression Suite (Existing Packages)

| Package | Result | Evidence / Notes |
|---|---|---|
| **WP-D4 Status screen (2 pages)** | **PASS** | `r10_st_p1.bmp` / `r10_st_p2.bmp` (`22565.sav`): Page 1 shows all 6 RS Souls; pressing R toggles Page 2 showing all 5 owned BM Souls + 1 empty slot |
| **WP-D4 Free Tournament (36 secret chips)** | **PASS** | `r10_ft.ss` (`4752.sav`, 0 traded Navis): `0x020014C0` populated with `[0x02, 0x01, 0x06, 0x04, 0x03, 0x05, 0x1E, 0x00]` (MegaMan + 7 real Soul Navis, zero `0x14`/`0x15` fillers) |
| **WP-B2 Frequency Tuner** | **PASS** | `r10_tuner.ss`: talking to Tuner NPC at `(-100, 20)` in Higsby's toggled flag `0x0E58` (`0x00 -> 0x80`) |
| **WP-D2 Z-Saver BMD** | **PASS** | `r10_zsaver.bmp`: Blue Mystery Data spawns cleanly on the Undernet 1 `(94, 00)` hidden spur at `(-282, 56, 0)` |
| **WP-D3 Bass both chips / WP-D2 random drops / WP-D4 Souls star** | **PASS** | All hooks intact and byte-verified in both RS & BM builds; `patch/build.py --check` PASS |
