# Duo Soul Transform Trace (Round 12 Task 2, 2026-11-23)

## 1. Root Cause of Round 11's "Consumed Without Transforming"

There is **no `cmp #12` or 13-entry rejection** anywhere in the unite-to-transform pipeline. The entire path handles form `13` (`0x0D`) natively:

1. `BN4_UNITE_CHECK` (`0x0801FF84`, RS == BM): patched by `wp_d4_duo_soul.asm` to scan indices `12..0`, writes `[r5, #0x0D] = 13`.
2. `sub_080208F4` (`0x080208F4`, RS == BM): queues `13` (`0x0D`) into `[r5, #0x48]`.
3. Custom Screen `OK` handler (`0x0801F098`–`0x0801F0D2`, RS == BM):
   - `0x0801F09C`: `bl sub_0801F1FE` sets `0x0203BF88 |= (1 << (13 - 1))` (`0x00001000`).
   - `0x0801F0A0`: builds `r0 = 13 + 0x61 + 0xFF = 0x016D` (`DuoSoul` pseudo-chip).
   - `0x0801F0C0`: **`bl sub_08020306` (`BN4_PICK_GATE`)** → `sub_080085F4` → `sub_0800642C` → **`BN4_CHIP_GATE` (`0x080063C8`)**.
   - `BN4_CHIP_GATE` checks `[0x02000670 + 0x16D] ^ 0x64 == [eToolkit + 0x84][0x16D]`.
   - **Why Round 11 failed:** Round 11's hand-edited `duosoul.sav` set event flag `0x0023` without calling `WriteChipKey(0x16D)`, leaving `[0x02005ADC + 0x16D] = 0x07` instead of `0x9C`. `BN4_PICK_GATE` therefore returned `0x3784` (rejected), and `0x0801F0C8`–`0x0801F0CE` silently deleted `0x16D` from the hand after `0x0203BF88` had already been marked!
4. **Live verification with valid chip key (`[0x02005ADC + 0x16D] = 0x9C`):**
   - `BN4_FORM_TURN_START` (`0x0800B658`, RS == BM) calls `ChipIdToSoulId` (`0x0800B924`: `0x016D - 0x0160 = 13`) and stores `[AI_data + 0x10] = 0x0D`.
   - `sub_0800B8B0` (`0x0800B8B0`, RS == BM) checks only `!= 0xFF` and `!= 0`, returning `1`.
   - `0x08013B16` (RS == BM) triggers `BN4_SET_ACTION(0x0D)` → `sub_080EBA44` (`RS 0x080EBA44 / BM 0x080EBA50`).
   - `sub_080EBB6C` sets `BN4_NAVI_BYTE(0x24) = 13`, loads MetalSoul's sprite via `duo_form_sprite`, and `sub_080EBC20` calls `BN4_FORM_STATE` (`0x0800E05C`).
   - **Verified on screen (`scratch/r12/r12_ds_k2_300.bmp`):** MegaMan transforms cleanly into MetalSoul's sprite with `"DuoSoul"` text at bottom-left and 3 turns. In real gameplay, `duo_grant_rewards` and `FC 0C` both call `WriteChipKey(0x16D)`, so the transform works out of the box once granted.

## 2. Downstream Form-13 Issues Found (RS & BM)

### Bug A (Real visual corruption): Emotion Window Face OOB in `BN4_FACE_GET`
- **Observed on screen (`scratch/r12/r12_ds_k2_300.bmp`, `r12_ds_k2_420.bmp`):** While in DuoSoul, the top-left Emotion Window draws garbled yellow `"E F G H"` tiles and a solid orange turn-counter box.
- **Location:** `BN4_FACE_GET` (`0x08016A20`, **RS == BM**), specifically the return math at `0x08016A64`:
  - **RS `0x08016A64`** (6 bytes): `20 1C 04 30 10 BD` (`adds r0, r4, #0; adds r0, #4; pop {r4, pc}`) → for form `13`, returns `face = 17`.
  - **BM `0x08016A64`** (8 bytes): `20 1C 04 30 06 38 10 BD` (`adds r0, r4, #0; adds r0, #4; subs r0, #6; pop {r4, pc}`) → for form `13`, returns `face = 11`.
- **Why it corrupts:** Each ROM only has **11** faces (`0..4` moods + `5..10` that version's 6 Souls) in `BN4_FACE_PALS` (`RS 0x0870C1E0 / BM 0x0870BCD4`) and the Soul face tile sheet. `face = 17` (RS) and `face = 11` (BM) index out of range into unrelated ROM data. (The same applies to the other version's 6 Souls in RS/BM if `BN4_FACE_GET` isn't already patched for them.)
- **Fix:** Redirect `BN4_FACE_GET` at `0x08016A64` (or via veneer/hook) so when `r4 == 13`:
  - **BM:** map `r4 = 13` → `r0 = 7` (`MetalSoul` face = `9 + 4 - 6 = 7`), keeping `r1 = turns_left` (`3/2/1`).
  - **RS:** RS does not contain MetalSoul's face tiles; either map `r4 = 13` → `r0 = 0` (Normal MegaMan mood face, no turn digit) or borrow a valid RS Soul face (e.g. `r0 = 6`, GutsSoul, which keeps the `3/2/1` turn digit) — or copy BM's MetalSoul face + palette at build time (same pattern as Duo/GP Power art).

### Check B: Soul-Change Cutscene Aura Palette (`sub_080E3818`)
- **Location:** `sub_080E37D8` (`RS 0x080E37D8 / BM 0x080E37E4`) spawns battle object `0x13` with `[r5, #4] = form_id (13)`. Its init `sub_080E3818` (`RS 0x080E3818 / BM 0x080E3824`) loads sprite `(0x14, 2)` and calls `BN4_SPRITE_PALETTE_SET` at **`RS 0x080E383C / BM 0x080E3848`** (`28 79 xx F7 xx FF`) with `r0 = [r5, #4] = 13`.
- It did not crash during the transform cutscene (`r12_ds_k2_220.bmp`–`300.bmp`), and if palette 13 of `(0x14, 2)` is undesired, clamping `13 -> 9` (MetalSoul's aura palette) at `RS 0x080E383C / BM 0x080E3848` borrows MetalSoul's cutscene aura.

### Check C: All Other Form Tables (Verified Safe for Form 13)
Every other form-indexed table is already 16 entries and valid for form `13` in both RS and BM:
- `BN4_FORM_ENTERS` (`0x0800E0A0`, RS == BM): 16 entries, `[13] = sub_0800E0E0` (no-op; ready for `duo-soul-passive.md`).
- `BN4_FORM_COMPANIONS` (`0x08018068`, RS == BM): `[13] = 0x0000`.
- `BN4_FORM_FRAMES` (`0x08018088`, RS == BM): `[13] = 0x0258` (600, same as all Souls).
- `BN4_FORM_TURNS` (`0x080180A8`, RS == BM): `[13] = 3`.
- `BN4_FORM_LOADOUTS` (`0x080184F0`, RS == BM): `[13] = FF 00 FF 19`.
- `BN4_FORM_ELEMENTS` (`0x0800C964`, RS == BM): `[13] = 0x00` (Null).
- `dat_080EBC00` camera shake (`RS 0x080EBC00 / BM 0x080EBC0C`): 16 × 2 B, `[13] = 00 00`.
