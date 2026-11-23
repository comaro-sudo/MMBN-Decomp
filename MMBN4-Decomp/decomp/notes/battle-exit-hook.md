# Battle Exit Hook for Per-Battle EWRAM Bytes (Round 11, Task 2, 2026-11-23)

Exact drop-in hook to clear per-battle EWRAM scratch bytes (`0x020015EF` lobby/Duo rematch marker, `0x020015EE` SP+ active flag, `0x020015EC` roaming SP enemy ID) on **every** battle exit: **win, loss, flee, and game over**. Verified in `decomp/asm/code_00.s` and both US ROMs (`bn4_rs_us.gba` & `bn4_bm_us.gba`).

---

## 1. Why `0x08005478` / `0x080082B0` Were Wrong & What Runs on Every Exit (V)

- `0x08005478` and `0x080082B0` are mid-function addresses, not battle-exit entry points.
- The top-level battle state machine (`dat_08006B98`, RS == BM) has 3 states:
  - State 0: `sub_08006BA4` (Battle Start / Init — calls `sub_0801592C` which zeroes `0x0203BF88`).
  - State 1: `sub_08006BCC` → `sub_08006C78` (Battle Ready) → `sub_08006CB4` (Main Battle Loop).
  - When the battle ends for **any** reason (`sub_08017B88() == 4`: win after results screen, loss, flee, or game over), `sub_08006CB4` sets `[r5] = 0x0C` → **`sub_08006D6A` (Battle Exit Dispatcher)**.
- `sub_08006D6A` runs `exit_state[0]` (`sub_08006DA8`: 10-frame fadeout + audio cleanup) and then **`exit_state[1]` = `sub_08006E1E`** (`code_00.s:13209`, **identical in RS & BM**).
- Inside `sub_08006E1E`:
  - `r7[0x10] == 1` (Win — **after** `BattleResultsGrantReward` has already finished!)
  - `r7[0x10] == 2` (Loss → calls `sub_0800567E` Game Over if `(sub_08007EEC() & 4) != 0`, else returns to overworld)
  - `r7[0x10] == 3` / `4` (Flee / Draw)
  - `r7[0x10] == 5` (Special/abort)
- **Every single battle exit path runs `sub_08006E1E` exactly once, strictly after `BattleResultsGrantReward` has completed.**

---

## 2. Exact Hook Point, Bytes, Live Registers & Veneer (RS == BM) (V)

```arm
; code_00.s:13209 — identical in US Red Sun and US Blue Moon:
08006E1E: B5D0        push {r4, r6, r7, lr}
08006E20: F7F9 FC58   bl   sub_080006D4       ; <--- HOOK SITE (4 bytes)
08006E24: 4654        mov  r4, sl
```

| Field | Value (RS & BM) |
|---|---|
| **Hook address** | **`0x08006E20`** (RS == BM) |
| **Original 4 bytes** | `F9 F7 58 FC` (`bl sub_080006D4`) |
| **Live registers** | `r5` (battle main state), `r10` (`sl`). **`r0–r3` are completely scratch** (about to be clobbered by `sub_080006D4`). |
| **Veneer spot (in `bl` range)** | **RS `0x080C4788` / BM `0x080C4794`** (first 8 of the 20 dead bytes after `DUO_DEFEAT_STATE` thunk; distance `+0xBD968` / `+0xBD974` — well within Thumb `bl` ±4 MB range!). |

### Drop-in Thumb code
```arm
; At 0x08006E20 (RS == BM):
.org 0x08006E20
    bl      VENEER_BATTLE_EXIT      ; RS 0x080C4788 / BM 0x080C4794

; At VENEER_BATTLE_EXIT (8 bytes, word-aligned):
.org VENEER_BATTLE_EXIT
    ldr     r3, [r15, 0]
    bx      r3
    .word   battle_exit_clear | 1

; In free space:
battle_exit_clear:
    ldr     r0, =0x020015EC         ; word-aligned: clears 0x020015EC..0x020015EF in one str!
    mov     r1, 0                   ;   +0: 0x020015EC (sp_roam_enemy)
    str     r1, [r0]                ;   +1: 0x020015ED (sp_phase)
                                    ;   +2: 0x020015EE (sp_plus_active)
                                    ;   +3: 0x020015EF (lobby_rematch_marker)
    ldr     r3, =0x080006D4 | 1     ; tail-call original sub_080006D4 (RS == BM)
    bx      r3
    .pool
```
*(Bonus: because `0x020015EC` is word-aligned, a single `str r1, [r0]` zeroes all four per-battle scratch bytes `0x020015EC..0x020015EF` atomically in 12 bytes of code, and tail-calling `sub_080006D4` needs no `push`/`pop`!)*

---

## 3. Proposed Names (for `decomp/symbols/names.txt`)

```
08006E1E 08006E1E code   GEMINI       BattleExitCleanup                Universal battle exit handler (win, loss, flee, game over); runs after BattleResultsGrantReward
0801592C 0801592C code   GEMINI       BattleInitEwram                  Clears per-battle EWRAM state (incl. 0x0203BF88 used-Souls bitmask) at battle start
```
