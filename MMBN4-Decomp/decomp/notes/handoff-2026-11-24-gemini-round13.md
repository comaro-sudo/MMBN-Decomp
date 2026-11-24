# Handoff: Gemini Round 13 (2026-11-24)

## Goal & Current Status
Completed all **5 tasks** from `MMBN4-Decomp/decomp/notes/task-gemini-round13.md` cleanly on both US Red Sun (`bn4_rs_overhaul.gba`) and US Blue Moon (`bn4_bm_overhaul.gba`), with `python patch/build.py`, `python patch/build.py --check`, RS/BM disassembly checks, and PC port (`mmbn4.exe`) smoke tests all passing. All commits are local on `main` (not pushed).

### `patch/` Changes & Commit IDs (for `docs/handoff-violet-eclipse.md`)
1. **`4de9524`** — `patch(wp-a3): cut 34 key-item and upgrade shop prices to 40% (Option 2)`
   - Added `patch/src/common/wp_a3_key_item_prices.asm` (14 HP Memories, 12 Zenny NaviCust in Shops 14–15, 4 Unlockers, 4 LocEnemies in `BN4_SHOP_STOCK_INIT`).
   - Added `SHOP_STOCK_INIT` (`RS 0x0804ADA0 / BM 0x0804ADA8`), included in `main.asm`, documented new-save caveat in `patch/README.md`.
2. **`71d149e`** — `patch(wp-b6): add Waiting Room Guard A slot bounds check (1..7)`
   - Added `patch/src/common/wp_b6_waiting_room_guard.asm` (`FS_WAITING_ROOM_GUARD = 0x087FC170`, size `0x30`), hooking `WaitingRoomReceiveTradedNavi` (`RS 0x0803FA90 / BM 0x0803FA98`) to return `r0 = 1` on `r6 == 0 || r6 > 7`.
3. **`acf65ad`** — `patch(wp-d3): add Duo rematch S-rank tier climb (V1->a->b->o->SP+) and research note`
   - Added `decomp/notes/duo-rematch-rank.md` explaining why `BattleModeFlags[0x33] = 0x002D` skips the results screen and why showing results on Duo's white-flash deletion is unsafe.
   - Hooked `BattleCalcBustingLevel` (`0x08007692`, RS == BM) via `VENEER_DUO_RANK` (`0x08001784`, first slot of dead `sub_08001784`) → `FS_DUO_RANK` (`0x087FC1A0`, size `0xB0`) so S-rank Duo rematch wins (`r0 >= 11`, `[0x020015EF] == 0x8D`) climb `0x0E5A`–`0x0E5B` (`V1 -> α -> β -> Ω`) and unlock DuoΩ SP+ (`0x0E5C`).
4. **`a6cf221`** — `patch(wp-b1,wp-d3): use b4=0x3D so friendly & lobby rematches show results screen; shift Junk crystal onto grass`
   - Fixed a critical bug caught by PC port smoke testing: `b4 = 0x3F` (`BattleModeFlags[0x3F] = 0x0039`, bit 1 = 0) skipped the results screen in both `wp_b1_friendly_matches.asm` and `wp_d3_lobby_rematches.asm`. Changed both to **`0x3D`** (`BattleModeFlags[0x3D] = 0x003B`, identical boss rank + retryable flags + results screen ON).
   - Fixed `fm_npc_higsby` dir (`7 -> 3`), `fm_npc_sal` dir (`3 -> 1`), and moved `fm_npc_junk` crystal onto grass (`(56, 36) -> (36, 48)`).
5. *(Final commit)* — `decomp/notes/patch-test-results-6.md`, `decomp/notes/handoff-2026-11-24-gemini-round13.md`, and `patch/README.md` test status updates.

## Key Decisions & Constraints
- **Free space next slot:** `0x087FC250` (up to `FS_TEXT = 0x087FD000`, `0xDB0` = 3,504 bytes free; plus `0x087FB670..0x087FB6B0` = `0x40` bytes).
- **Veneer pools (RS == BM):**
  - `sub_0800A144` (6 slots): all 6 allocated (`BATTLE_EXIT`, `SP_PLUS_1..3`, `DUO_PAL`, `SP_REWARD`).
  - `sub_08001784` (4 slots): 1 allocated (`VENEER_DUO_RANK` at `0x08001784`), 3 free (`0x0800178C`, `1794`, `179C`), plus 25 more verified slots in `decomp/notes/veneer-space.md`.
- **Verified vs Tentative:** Everything in `patch-test-results-6.md` is **verified** in both built ROMs (`build.py` + `--check`) and in-game via `mmbn4.exe`.

## Modified & Relevant Files
- `MMBN4-Decomp/patch/src/common/wp_a3_key_item_prices.asm` (new, `4de9524`)
- `MMBN4-Decomp/patch/src/common/wp_b6_waiting_room_guard.asm` (new, `71d149e`)
- `MMBN4-Decomp/patch/src/common/wp_d3_duo_rematch.asm` (modified, `acf65ad`)
- `MMBN4-Decomp/patch/src/common/wp_b1_friendly_matches.asm` (modified, `a6cf221`)
- `MMBN4-Decomp/patch/src/common/wp_d3_lobby_rematches.asm` (modified, `a6cf221`)
- `MMBN4-Decomp/patch/src/common/defines.asm`, `patch/src/rs/version.asm`, `patch/src/bm/version.asm`, `patch/src/main.asm`, `patch/README.md`
- `MMBN4-Decomp/decomp/notes/duo-rematch-rank.md` (new, `acf65ad`)
- `MMBN4-Decomp/decomp/notes/patch-test-results-6.md` (new)
- `MMBN4-Decomp/decomp/notes/handoff-2026-11-24-gemini-round13.md` (this file)

## Open Issues / Blockers
- **Zero functional blockers.**
- **One visual polish item found during smoke testing (`patch-test-results-6.md`):** `Higsby (0x08)` and `Sal (0x2C)` are LZ77-compressed (`bit 31 = 1` in `BN4_SPRITE_LIST_6`), so they render as a white circle on maps that don't preload them (Higsby in DenDome Hall; Sal in Higsby's and Castillo WaitRm). Both NPCs are still fully talkable and functional.

## Immediate Next Steps
1. **Visual fix for compressed NPC sprites (`Higsby 0x08` and `Sal 0x2C`):** add them to those maps' sprite preload lists (or decompress at build time) so zero white-circle fallbacks remain.
2. **Existing-save shop price sync hook (TODO step 9):** rewrite save slot `+0x58` on load if existing saves should also receive WP-A3's 40% prices.
3. **Update `docs/handoff-violet-eclipse.md`** (Claude) with the Round 13 commit IDs above.
