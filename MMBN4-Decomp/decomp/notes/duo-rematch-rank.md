# Duo Rematch S-Rank & Results Screen Research (Round 13 Task 3, 2026-11-24)

**Status:** Verified in both US RS and US BM ROMs; implemented in `patch/src/common/wp_d3_duo_rematch.asm`.

## 1. Why Duo's Battle Skips the Results Screen (V)

Every battle stores its mode byte (`b4` from the 8-byte battle header) at `[BattleState + 0x0F]` (`[sl+0x18]+0x0F`). At battle win (`sub_08007642`, RS == BM):

```asm
0800768E: cmp  r0, #1               ; 1 = player win
08007690: bne  _080076D2
08007692: bl   sub_08008558         ; BattleCalcBustingLevel() -> r0 = 1..11 (11 = S rank)
08007696: strb r0, [r5, #0x1E]      ; [BattleState + 0x1E] = Busting Level
08007698: bl   sub_08007E46         ; r0 = [BattleState + 0x0F] (b4)
...
080076A8: bl   sub_08007EEC         ; r0 = BattleModeFlags[b4] (table at 0x08018787, RS == BM)
080076AC: movs r1, #2
080076AE: tst  r0, r1               ; bit 1 (0x02) = show results screen
080076B0: beq  _0800770E            ; 0 -> state 0x14 (sub_08007796, skip results straight to exit)
                                    ; 1 -> state 0x10 (sub_08007718 -> sub_08021348, results screen)
```

In `BattleModeFlags` (`0x08018787`, RS == BM; u16 entries indexed by `b4 * 2`, low byte holds the flags):

| Mode (`b4`) | `BattleModeFlags[b4]` | Bit 1 (`0x02` results) | Bit 3 (`0x08` boss rank) | Notes |
|---|---|---|---|---|
| `0x00` | `0x0006` | **1** | 0 | Normal virus battle (flee + results) |
| `0x05` | `0x000A` | **1** | **1** | Standard boss battle (no flee + boss rank + results) |
| `0x33` | `0x002D` | **0** | **1** | **Duo's battle header** (bit 1 = 0 -> skips results screen!) |
| `0x31` | `0x002F` | **1** | **1** | Same as `0x33` + bit 1 (results screen, flee allowed) |
| `0x35` | `0x002B` | **1** | **1** | Same as `0x33` + bit 1, no flee |
| `0x3F` | `0x0039` | **0** | **1** | Sets bit 4 (`0x10`), hangs Duo's fade-in |

## 2. Why Showing the Normal Results Screen on Duo Is Risky (V)

Even if `b4` were changed from `0x33` to `0x35` so `sub_08007642` enters state `0x10` (results screen), Duo's own deletion state (`sub_08010DF8`, RS == BM) calls `SetModeFlags(4, 4)` at `0x08010E48` to fade the entire screen to **solid white** before deleting Duo. The results screen (`sub_08007772` -> `sub_08021348`) never fades back in because normal battles do not fade to white beforehand, and battle exit (`sub_08007796` at `0x080077AA`) explicitly checks `BN4_BY_NAME_RANGE(0xE0, 0xE3)` to skip the exit fade-out on a Duo win because it expects the screen to still be white.

## 3. Clean Solution: Hook `BattleCalcBustingLevel` at `0x08007692` (RS == BM) (V)

Even when a battle skips the results screen, `sub_08007642` **always** calls `sub_08008558` (`BattleCalcBustingLevel`) at `0x08007692` (RS == BM) on every win:

- `sub_08008558` (RS == BM) checks `BattleModeFlags[0x33] & 8` (`0x2D & 8 != 0`), so it calls `sub_080081BC(is_boss=1, player=0)` — the exact standard boss Busting Level formula (time, hits taken, movement, NaviChips) returning `1..11` where **`11` = S-rank** (the results screen at `0x08021EBC` checks `cmp r0, #10; ble` for S-rank).
- At `0x08007692`, `0x020015EF` still holds `0x80 | 13` (`0x8D`, set by `FC 0D` in `wp_d3_duo_rematch.asm`, cleared later on battle exit by `wp_d3_battle_exit_clear.asm`). In the story Duo fight, `0x020015EF == 0`.

### Implementation (`wp_d3_duo_rematch.asm`)
- Replace `0x08007692` (`f000 ff61`, `bl 0x08008558`, RS == BM) with `bl VENEER_DUO_RANK` (`0x08001784`, first 8-byte slot of verified dead `code_00` function `sub_08001784`, RS == BM).
- Handler `duo_rank_climb_hook` at `FS_DUO_RANK` (`0x087FC1A0`, size `0x80`):
  1. Calls `0x08008558` -> saves exact Busting Level in `r4`.
  2. If `r4 < 11` (not S-rank) or `[0x020015EF] != 0x8D` (not a Duo rematch), returns `r0 = r4`.
  3. Reads 2-bit Duo tier from flags `0x0E5A`–`0x0E5B` (`0` = V1, `1` = α, `2` = β, `3` = Ω):
     - If tier `< 3`: increments tier (`V1 -> α -> β -> Ω`) and writes `0x0E5A`–`0x0E5B`.
     - If tier `== 3` (DuoΩ): sets flag `0x0E5C` (unlocks **DuoΩ SP+**).
  4. Returns `r0 = r4` so `[BattleState + 0x1E]` receives the exact unmodified Busting Level.
