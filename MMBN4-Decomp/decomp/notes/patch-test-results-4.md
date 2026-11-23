# In-Game Test Results: Round 11 Seven Packages (2026-11-23)

Built and tested in the PC port (`mmbn4.exe`) on both private (`bn4_rs_overhaul.gba`, SHA-1 `7708dc90db590e3410eddd29c524d8344f7c4d34`, `--check` PASS) and `--public` (`SHA-1 cc4eeb849fdbce63bbf51cd2a3f34142d515987b`) builds. All savestates and screenshots were saved outside the repo in `scratch/r11/`.

---

## Summary Table

| Package | Result | Key Finding / Exact Fix for Claude |
|---|---|---|
| **Chip art (private & `--public`)** | **PASS** | GP Power (`0x138`) icon rendered crisply in Folder Edit (`r11_priv_duo2.bmp`) and was cleanly absent in `--public` (`r11_pub_duo2.bmp`). |
| **Chip text (`Duo` / `GP Power`)** | **BUG FOUND** | TextPet patched `0x7FC000` (`"Duo"`, `"GP Power"`) and updated all 8 pointers, **but Folder/Pack still renders a blank name** (`r11_priv_duo2.bmp`, `r11_pub_duo2.bmp`). Root cause: `sub_0802F154` (names) and `sub_08106BF0` (descriptions) both have a hardcoded `cmp chip_id, #0x137` e-Reader RAM bypass! **2-word fix below.** |
| **GP Power BMD (`0x0D0B`)** | **PARTIAL / VISUAL PASS** | Blue Mystery Data visibly spawns on the ACDC Area 2 lower-left round pad at `(106, 326)` (`r11_gpp_before.bmp`, `r11_gpp2_pos.bmp`). *(Note: `(106, 326)` sits on the lower-left lip of the round pad; the center of the round pad is closer to `(124, 326)` — worth nudging ~16 px NE if desired, though it's visible and reachable from the pad.)* |
| **Duo rematch (`wp_d3_duo_rematch.asm`)** | **2 BUGS FOUND** | (1) Hidden when `0x0500=0` (`r11_duo_hidden.bmp`) and visible when `0x0500=1` (`r11_duo_visible.bmp`), spot on Meteor Comp 1 center pad is good, `"Fight Duo? Yes / No"` opens (`r11_duo_talk7.bmp`), and launches Duo V1 2000 HP (`r11_duo_v1_fight7b.bmp`) — **PASS**. (2) **BUG:** Duo's Signal (`list[6][0x63]`, `0x884911EC`) draws a **white circle** (`r11_duo_visible.bmp`). (3) **BUG:** Winning Duo V1 (1-HP test: explosion `r11_duo_f260.bmp` → white flash `r11_duo_f400.bmp`) **skipped the results screen** and returned straight to Meteor Comp 1 (`r11_duo_f450.bmp`) with `0x020015EF` still `0x8D` and `0x0E5A=False` (`b1 = 0x13` still suppresses results!). |
| **Duo story grant (`wp_d2_duo_story_grant.asm`)** | **BUG FOUND** | On the Duo V1 1-HP win above (`r11_duo_f260.bmp` → `r11_duo_f450.bmp`), `0x0023` (DuoSoul) and `0x0E72` (Duo chip guard) remained **`False`** in live EWRAM (`EWRAM+0x2440`). `duo_defeat_hook` (`0x080C4780`) is correctly assembled, so either Duo's deletion in plain battle exits before state 2 (`0x080C4780`) with `r5[0x0A]==0`, or `r5[0x0A]` is non-zero when state 2 runs. |
| **Duo Soul (`wp_d4_duo_soul.asm`)** | **PARTIAL / BUG FOUND** | (1) Selecting a non-RegChip `Guard1 *` (Slot 1) **lights up `UNITE` in bright red** (`r11_ds_lit7.bmp`) and places `0x16D` (MetalSoul gear icon) in the chip queue (`r11_ds_united7.bmp`), and sets bit 12 (`0x00001000`) of `0x0203BF88` — **PASS**. (2) **BUG:** Confirming `OK` consumes `0x16D` from the queue **without playing the Soul cutscene or transforming MegaMan** (`r11_ds_f60.bmp`, `r11_ds_f120.bmp`, `r11_ds_battle8.bmp`); MegaMan stays Normal MegaMan and charge shot hits for `50` (`r11_ds_hit8.bmp`), not `180`. |
| **Status screen page 3 (`653233d`)** | **PASS** | With all 13 Souls (`0x0017..0x0023`) set, R cycles Page 1 → Page 2 → Page 3 (`r11_st_p3.bmp` shows the 13th Soul gear emblem cleanly), and L returns Page 3 → Page 2. |
| **Trash can (`c3a787f`)** | **BUG STILL PRESENT** | `lobby_npc_trashcan` on ACDC Town (`r11_trashcan.bmp`) **still draws a white circle** even with `.byte 0x16, 0`. |
| **Stale marker (`0x020015EF`)** | **CONFIRMED** | Starting Dex lobby rematch sets `0x020015EF = 0x80`; fleeing leaves `0x020015EF = 0x80` stale in EWRAM (solved by Task 2's `sub_08006E1E` battle-exit hook). |

---

## Detailed Bug Diagnostics & Exact Fixes for Claude

### 1. Chip Names & Descriptions Blank for `0x137` (`Duo`) and `0x138` (`GP Power`)
Both chip text renderers have a hardcoded `cmp chip_id, #0x137` e-Reader RAM redirect that bypasses our TextPet-patched Bank 1 archives (`0x087FC000`):
- **Chip Names (`sub_0802F154`, `code_02.s:24679`):**
  - Literal `_0802F1BC` = **`RS 0x0802F1BC / BM 0x0802F1C0`** (`.word 0x00000137`).
  - **1-word fix:** Patch `RS 0x0802F1BC / BM 0x0802F1C0` → **`.word 0x00000139`**.
- **Chip Descriptions (`sub_08106BF0`, `code_09.s:28301`):**
  - Literal `_08106C28` = **`RS 0x08106C28 / BM 0x08106C34`** (`.word 0x00000137`).
  - **1-word fix:** Patch `RS 0x08106C28 / BM 0x08106C34` → **`.word 0x00000139`**.

### 2. White Circles on Trash Can (`list[7][0x44]`) and Duo's Signal (`list[6][0x63]`)
- Both `list[7][0x44]` (`0x884A8E44`) and `list[6][0x63]` (`0x884911EC`) have **bit 31 set (`0x88......`, LZ77-compressed)**, and in the overworld NPC renderer they fall back to the 16×16 white-circle placeholder even on anim `0`.
- **Verified uncompressed `list[7]` prop sprites (`< 0x80000000`):** `0x06, 0x07, 0x08` (glowing blue Mystery Data crystal — confirmed working on all MD pickups!), `0x09, 0x12, 0x20, 0x45, 0x4A, 0x53, 0x5C, 0x5D, 0x67, 0x6B, 0x6D`.
- **Quick fix for Duo's Signal:** Switch `.byte 0x17, 0x63` → **`.byte 0x25, 0x08, 0x1C`** (`list[7][0x08]`, uncompressed glowing blue crystal, guaranteed to draw!).
- **Quick fix for Trash Can:** Switch `.byte 0x25, 0x44, 0x1C` to an uncompressed `list[7]` prop (e.g. `0x45`, `0x53`, or `0x08`) or a `list[6]` uncompressed sprite (e.g. `0x5A` JunkMan / `0x00..0x07`).

### 3. Duo Rematch Results Screen & `duo_defeat_hook`
- In `wp_d3_duo_rematch.asm`, `duo_rec` uses `b1 = 0x13` (story battle 93's `0x93` with bit 7 cleared). Bit 4 (`0x10`) or mode `0x03` still skips the results screen (`r11_duo_f400.bmp` → `r11_duo_f450.bmp`, `0x020015EF` stays `0x8D`). Compare against the lobby rematch's `SET_BATTLE_PARAMS` (`b1 = 0x00` or `0x03`) — clearing bit 4 (`0x13 → 0x03` or `0x00`) should restore the results screen.
- For `wp_d2_duo_story_grant.asm`: in the Duo V1 rematch win (`r11_duo_v1_done8.ss`), neither `0x0023` nor `0x0E72` was set, meaning either `sub_080C4780` was entered with `r5[0x0A] != 0` or Duo's deletion in plain battle takes a different state (`0x080C479C` state 3). Checking `r5[0x0A]` (or dropping the `r5[0x0A] == 0` guard since `0x0023` and `0x0E72` are already idempotently flag-guarded!) will make `duo_defeat_hook` fire unconditionally on Duo's deletion.

### 4. Duo Soul Unite Consumed Without Transforming
- `UNITE` works cleanly for non-Regular chips of class `0x0D` (`r11_ds_lit7.bmp` → `r11_ds_united7.bmp`, `r5[0x0D] = 0x0D`, `0x0203BF88 |= 0x1000`), confirming `BN4_UNITE_CHECK` and the 13-entry `duo_soul_kinds`/`duo_soul_flags` tables work.
- However, on `OK` (`r11_ds_f60.bmp..r11_ds_battle8.bmp`), MegaMan stays Normal MegaMan (`50` dmg charge shot). Note: `sub_0800B658` (`BN4_FORM_TURN_START`) computes `ChipIdToSoulId(0x16D) = 13` and stores `13` into `[r7, #0x10]`, so the downstream consumer of `[r7, #0x10]` (the Soul-transform trigger in `code_00`/`code_01`) is rejecting `13` (`> 12`). Worth checking in Round 12!
