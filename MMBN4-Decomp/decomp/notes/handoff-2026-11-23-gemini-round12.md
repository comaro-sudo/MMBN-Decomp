# Handoff: Gemini Round 12 Complete (2026-11-23)

## Goal & Current Status

All **7 tasks** from `MMBN4-Decomp/decomp/notes/task-gemini-round12.md` are **complete and verified** in both US Red Sun and US Blue Moon ROMs (and in `mmbn4.exe` for runtime tests). No edits were made to `patch/`, `docs/`, `TODO.md`, or any `decomp/` assembly/build files.

### Deliverables written (all in `MMBN4-Decomp/decomp/notes/`)
1. **`patch-test-results-5.md`** — Re-tested `4f5ec2b` (`python patch/build.py --check` PASS, `bn4_rs_overhaul.gba` SHA-1 `8a3584c683a55c9cd919d6a675fbb6321dd0891a`) in `mmbn4.exe`:
   - **PASS:** Trash Can & Duo's Signal crystal prop (`list[7] 0x08`), Battle-Exit Clear (`0x020015EC..0x020015EF` cleared on lobby loss and Duo exit), WP-B4 GMD tier follows Frequency Tuner (PT1 `4440.sav` Tier 3 rolled virus-trapped Super Hard entry; Auto rolled Normal `Recov10 *`), all 13 Soul names (`0x161..0x16D`) byte-identical.
   - **Bugs caught:** (a) Duo/GP Power (`0x137`/`0x138`) render name & `"Speedy trio attack!"` cleanly, then wipe Folder Edit to `0/30` on the next frame (reproduces on vanilla too; chip table `flags = 0x07`: bit 6 `0x40` clear hides from Library, bit 2 `0x04` marks e-Reader). (b) Duo Rematch V1/β/Ω still skips the results screen despite `b1 = 0x00` because `b4 = 0x33` suppresses results (`0x0023`/`0x0E72`/`0x0E5A`/`0x0E5B` all remained `False`).
2. **`duo-soul-transform.md`** — Traced full unite-to-transform pipeline. Found **no `cmp #12` rejection** in transform code; Round 11 failed because hand-edited `duosoul.sav` lacked `WriteChipKey(0x16D)` (`0x9C`), so `BN4_CHIP_GATE` deleted `0x16D` after `0x0203BF88 |= 0x1000`. With valid key `0x9C`, DuoSoul transforms cleanly. **Caught real visual bug:** `BN4_FACE_GET` (`0x08016A20`, RS == BM) returns `face = 17` (RS) / `11` (BM), indexing `BN4_FACE_PALS` (11 entries) out of range → garbled yellow `"E F G H"` emotion window. Exact RS & BM fix + aura palette `sub_080E3818` documented.
3. **`duo-story-grant-test.md`** — **Major finding:** `0x080C4780` (RS) / `0x080C478C` (BM) is **Bass** (`BN4_OBJECT_KINDS[1][0x36]`, sets flag `0x006F`), NOT Duo! **Duo** (`0xE0..0xE3`) is `BN4_OBJECT_KINDS[1][0x2A]` (`RS 0x080C03F5 / BM 0x080C0401`), whose deletion state is **`dat_080C04F8[2]` = `sub_08010DF8` (RS == BM)** at **`RS 0x080C0500 / BM 0x080C050C`** (exclusively referenced by Duo). No veneer needed — patch the 4-byte table pointer directly. **Action required for Claude:** restore Bass's `0x080C4780` and move `VENEER_BATTLE_EXIT` off `0x080C4788`/`0x080C4794`.
4. **`veneer-space.md`** — Binary-verified **12 dead/unreferenced functions** in `code_00` (**RS == BM**, zero asm refs, zero decoded Thumb `bl` targets in `0x08000000..0x08150000` in RS or BM, zero 4-byte pointers anywhere in either 8 MiB ROM, outside chip table), yielding **35 × 8-byte word-aligned veneers**. Highlighted `sub_0800A144` (54 B = 6 × 8-byte slots: `0x0800A144..0x0800A16C`, RS == BM) as a single contiguous block covering all 5 needed veneers + 1 spare.
5. **`prop-sprites.md`** — Contact sheets (outside repo in `scratch/r12/`) of all 14 uncompressed `list[7]` and 91 uncompressed `list[6]` sprites (identical ID sets in RS & BM). Recommended picks:
   - **Duo's Signal (Meteor Comp 1):** `list[7] 0x07` (Duo's hexagonal Meteor Comp crest, `RS/BM 0x08494C54`, opcode `25 07 1C`, anim `0x00`).
   - **ACDC Town Trash Can:** `list[7] 0x4A` (crumpled debris/wreckage pile, `RS 0x084AB1FC / BM 0x084AB274`, opcode `25 4A 1C`, anim `0x00`).
   - Confirmed **no NPC script opcode** can decompress a compressed sprite on the fly (`NpcRunScript` only has `0x17` and `0x25`; compressed sprites require map-init `sub_08002A4A` preload into the 8-slot `0x02038800` cache).
6. **`waiting-room-protection.md`** — Exhaustive disassembly of `WaitingRoomReceiveTradedNavi` (`RS 0x0803FA90 / BM 0x0803FA98`), `WaitingRoomDeleteEntry` (`RS 0x0803F15C / BM 0x0803F164`), `WaitingRoomClearSlotForSend` (`RS 0x0803F450 / BM 0x0803F458`), and `SyncTradedNavisToPool` (`RS 0x0803F948 / BM 0x0803F950`). **Verified:** Waiting Room never writes Soul flags (`0x0017`–`0x0022`) or chip keys (`0x02000670`/`[sl+0x84]`). Documented the real risks (`0x02006F40` deletion/overwrite and unchecked `r6` slot index `0` or `>= 8` overflowing into MegaMan or `BN4_TOURNAMENT_POOLS` `0x020073A0`), exact RS/BM guard points, and remaining uses of Waiting Room in Violet Eclipse.
7. **`key-item-prices.md`** — Full dump of all sold upgrades/key items/programs in `BN4_SHOP_STOCK_INIT` (`RS 0x0804ADA0 / BM 0x0804ADA8`, `BM = RS + 8`): 14 HP Memories (207,700Z), 12 Zenny NaviCust programs (92,900Z), 12 BugFrag Trader items (468 BF), and Unlocker/LocEnemy SubChips. Verified none are missable and provided 3 factual pricing options with exact RS/BM ROM offsets and u16 LE bytes (owner decides).

## Key Decisions & Constraints

- **Ownership respected:** Zero edits to `patch/`, `docs/`, or `TODO.md`.
- **No ROM data committed:** All contact sheets, savestates, and test screenshots live outside the repo under `C:\Users\test\.gemini\antigravity\brain\7e09fd7a-7f26-455f-919d-1f878852d7c9\scratch\r12\`.
- **Committed by explicit path only; not pushed.**

## Modified & Relevant Files

- `MMBN4-Decomp/decomp/notes/patch-test-results-5.md`
- `MMBN4-Decomp/decomp/notes/duo-soul-transform.md`
- `MMBN4-Decomp/decomp/notes/duo-story-grant-test.md`
- `MMBN4-Decomp/decomp/notes/veneer-space.md`
- `MMBN4-Decomp/decomp/notes/prop-sprites.md`
- `MMBN4-Decomp/decomp/notes/waiting-room-protection.md`
- `MMBN4-Decomp/decomp/notes/key-item-prices.md`
- `MMBN4-Decomp/decomp/notes/handoff-2026-11-23-gemini-round12.md`

## Open Issues / Blockers (for Claude / Owner)

1. **Restore Bass (`0x080C4780` RS / `0x080C478C` BM):** Move `VENEER_BATTLE_EXIT` to `sub_0800A144` (from `veneer-space.md`) and hook Duo's real deletion state table entry at `RS 0x080C0500 / BM 0x080C050C` (`duo-story-grant-test.md`).
2. **Duo Rematch results screen:** Change `b4` from `0x33` (suppresses results) in `patch_duo_rematch` so V1/β/Ω show the results screen and grant `0x0023`/`0x0E72`/Duo D (`patch-test-results-5.md`).
3. **Duo / GP Power Folder Edit wipe & Library visibility:** Investigate chip `flags = 0x07` (bit 6 `0x40` clear, bit 2 `0x04` e-Reader) causing Folder Edit wipe to `0/30` (`patch-test-results-5.md`).
4. **DuoSoul emotion window face index:** Clamp/override `BN4_FACE_GET` (`0x08016A20`) for form 13 so it doesn't index `BN4_FACE_PALS` out of range (`duo-soul-transform.md`).
5. **Owner decision:** Pick Option 1, 2, or 3 (or custom) from `key-item-prices.md` for WP-A4, and confirm whether to apply to existing saves too.

## Immediate Next Steps

- Claude can apply the fixes above directly from the 7 Round 12 notes (every RS & BM address, byte, and veneer slot is verified).
- Next Gemini round (when assigned) can re-test Claude's Round 12+ patch build in `mmbn4.exe`.
