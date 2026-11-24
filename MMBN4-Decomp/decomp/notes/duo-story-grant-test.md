# Duo Story Grant & Deletion State Diagnosis (Round 12 Task 3, 2026-11-23)

## 1. Root Cause: `0x080C4780` Is **Bass**, Not Duo!

In Round 11 and Round 12 Task 1, beating Duo V1 (1-HP win) never set `0x0023` or `0x0E72` and never triggered `duo_defeat_hook` at `RS 0x080C4780 / BM 0x080C478C`.

Inspecting Duo's live battle object in EWRAM (`scratch/r12/r12_d9_v1_bat.ewram`, `r12_dkill_380.ewram`, base `r5 = 0x0203B258`) and `BN4_OBJECT_KINDS[1]` (`0x08003A24`) proved why:

- **`0x080C4694` (RS) / `0x080C46A0` (BM)** (`BN4_OBJECT_KINDS[1][0x36]` at `0x08003AFC`) is **Bass** (sprite `list 8, 0x19`), whose deletion state `sub_080C4780` (RS) / `sub_080C478C` (BM) sets flag **`0x006F`** (the Bass SP/Omega completion star in `GetCompletionStars`).
- **Duo (`0xE0`–`0xE3`)** is **object kind 1, ID `0x2A`** (`[r5 + 0x01] = 0x2A`, `[r5 + 0x0F] = 0xE0`):
  - Table slot: `0x08003A24 + 0x2A * 4 = 0x08003ACC` (RS == BM) → **`RS 0x080C03F5 / BM 0x080C0401`** (`sub_080C03F4`, sprite `list 8, 0x18`).
  - Duo's main step (`sub_080C04E2`) dispatches `BN4_NAVI_RUNNER` on **`dat_080C04F8`** (`RS 0x080C04F8 / BM 0x080C0504`).
  - When Duo's HP hits 0 (verified live in `r12_dkill_380.ewram`), `[r5 + 0x09]` transitions `0x06 -> 0x02`, entering **`dat_080C04F8[2]`**:
    - **RS:** `0x080C0500` contains `.word 0x08010DF9` (`sub_08010DF8`)
    - **BM:** `0x080C050C` contains `.word 0x08010DF9` (`sub_08010DF8`)
  - **`sub_08010DF8` (RS == BM)** is **exclusively referenced by Duo** (only 1 pointer in the entire RS ROM: `0x080C0500`; only 1 in BM: `0x080C050C`). Its 3 sub-states (`dat_08010E0C`, indexed by `[r5, #0x0A]`):
    - `0x00`: `sub_08010D68` (first frame of deletion; clears attacks, sets `[r5, #0x0A] = 4`)
    - `0x04`: `sub_08010E18` (spawns explosions for `0x5A` frames, fades screen to white via `sub_08005AFC(4, 4)`, sets `[r5, #0x0A] = 8`)
    - `0x08`: `sub_08010E62` (waits for white fade, calls `sub_08022218`, frees Duo `[r5, #8] = 8`)

Because `sub_080C4780` is Bass's deletion state, **beating story Duo (PT1 battle 93) does NOT fire `duo_defeat_hook` either** — it enters `dat_080C04F8[2]` (`sub_08010DF8`).

## 2. Exact Hook Fix (RS & BM) — No Veneer Needed!

Because Duo's deletion state is dispatched through a **4-byte ROM function pointer** in `dat_080C04F8[2]`, hooking it requires **no veneer and no inline thunk** — just replace the 4-byte table entry!

- **RS hook site:** `0x080C0500` (original: `F9 0D 01 08` = `0x08010DF9`) → `.word duo_defeat_hook | 1`
- **BM hook site:** `0x080C050C` (original: `F9 0D 01 08` = `0x08010DF9`) → `.word duo_defeat_hook | 1`

```asm
duo_defeat_hook:                    ; fires for ALL Duo battles (story 93/94 and rematches, V1/α/β/Ω)
    push    r14
    ldrb    r0, [r5, 0x0A]
    cmp     r0, 0
    bne     @@tail
    bl      duo_grant_rewards       ; sets 0x0015, 0x0023, WriteChipKey(0x16D), and once: 0x0E72 + Duo D
@@tail:
    ldr     r3, =0x08010DF8 | 1     ; original Duo deletion state (RS == BM)
    bl      duo_call_r3
    pop     r15
```

### Important Cleanup Note for `0x080C4780` (Bass) & `VENEER_BATTLE_EXIT`
- **Restore Bass's deletion state** at `RS 0x080C4780..0x080C479B / BM 0x080C478C..0x080C47A7` (`wp_d2_duo_story_grant.asm` must stop patching `0x080C4780`, otherwise beating Bass grants Duo Soul and Bass's `0x006F` star logic is hijacked).
- **Move `VENEER_BATTLE_EXIT`** (`wp_d3_battle_exit_clear.asm`, currently at `RS 0x080C4788 / BM 0x080C4794`) to one of the verified unreferenced veneer slots in `decomp/notes/veneer-space.md` (Task 4), because `0x080C4788..0x080C479B` is live code inside Bass's `sub_080C4780` once Bass is restored!

## 3. Bonus Note on Duo Rematch Skipping Results Screen
In `wp_d3_duo_rematch.asm`, `duo_rec` has `b4 = 0x33` (copied from story battle 93/94), whereas normal boss battles that show the results screen (such as lobby rematches) have `b4 = 0x00` (or `b4` without the skip-results bits). Changing `b4` from `0x33` to a normal boss battle's `b4` (e.g. `0x00`) should allow the results screen to appear for Duo rematches.
