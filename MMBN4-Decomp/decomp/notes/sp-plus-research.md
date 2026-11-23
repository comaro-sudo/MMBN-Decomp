# SP+ Boss Difficulty (4,095 HP, Attack × 1.5, Speed × 1.25) — Feasibility Research (Round 8, Task 2, 2026-11-23)

**Verdict: FEASIBLE AS SPECIFIED.**
All three modifiers (4,095 HP, Attack × 1.5, Speed × 1.25) can be implemented cleanly via **three small battle-engine hooks in `code_00` / `code_01`**, all at **identical addresses in both Red Sun (USA) and Blue Moon (USA)**, gated on a single 1-byte EWRAM flag set before battle start. No new enemy IDs or per-boss AI edits are required.

---

## 1. How to Mark an SP+ Fight (V)

- Use a **1-byte EWRAM flag** right beside the lobby rematch marker (`0x020015EF`), e.g. **`0x020015EE`**:
  - `0x00` = normal battle.
  - `0x01` = **SP+ active** (set by the lobby rematch launcher when tier == SP+, or by roaming SP ghost encounters after an S-rank, or by Bass/Duo SP+ rematch).
  - Cleared to `0x00` on battle exit / `BattleResultsGrantReward` (`RESULTS_DONE_HOOK`, `RS 0x0802174C / BM 0x08021750`), and also cleared on flee/loss so it never leaks into the next battle.
- **No battle-record bit needed:** because `0x020015EE` is read directly by all three hooks below, it works uniformly for dynamic RAM battles (`lobby_fc_rematch`), story battles (Duo/Bass), and roaming SP ghost encounters.

---

## 2. HP = 4,095 (V)

- **Routine:** `sub_080067B4` (`code_00.s`, **identical in RS & BM**) is the single enemy-spawn routine that initializes every battle enemy actor from `BN4_ENEMY_RECORD` (`sub_08011878`).
- **Exact site (`0x08006802` RS & BM):**
  ```arm
  08006802: ldrh r1, [r7]           ; r7 = BN4_ENEMY_RECORD(enemy_id), r8 = enemy_id
            adds r2, r1, #0
            lsls r2, r2, #0x14
            lsrs r2, r2, #0x14      ; r2 = 12-bit HP (0..4095)
            strh r2, [r4, #0x24]    ; actor.cur_hp (u16)
            strh r2, [r4, #0x26]    ; actor.max_hp (u16)
            lsrs r1, r1, #0xc
            strb r1, [r4, #0x17]    ; actor.element (u8)
  ```
- **Confirmation:** `[r4, #0x24]` (`cur_hp`) and `[r4, #0x26]` (`max_hp`) are **16-bit halfwords** and are the sole copies of enemy HP in battle (the on-screen HP display and damage routines read `[r4, #0x24]` directly).
- **Hook:** When `[0x020015EE] != 0` and `enemy_id >= 0x80` (boss/Navi IDs; keeps any summoned viruses/objects untouched), override `r2 = 0x0FFF` (`4095`) before `strh r2, [r4, #0x24]` / `strh r2, [r4, #0x26]`.
  - *(Pure data also works if editing `BN4_ENEMY_STATS` directly, since `4095` fits in 12 bits, but hooking `0x08006802` avoids needing 22 new enemy IDs and keeps normal SP fights untouched.)*

---

## 3. Attack × 1.5 (Enemy → MegaMan) (V)

- **Routine:** `sub_08013A48` (`code_01.s`, **identical in RS & BM**) is the **single common MegaMan hit-application routine** (it is the sole caller of `sub_0800AB9E`, the MegaMan HP-decrement + hit-counter helper).
- **Exact disassembly (`0x08013A48`–`0x08013A86`, RS & BM identical):**
  ```arm
  sub_08013A48:                     ; r5 = MegaMan actor struct
      push {r4, r6, r7, lr}
      sub  sp, #4
      str  r0, [sp]
      ldr  r6, [r5, #0x54]          ; r6 = MegaMan's collision/hit struct
      ldr  r7, [r5, #0x58]
  08013A56:
      bl   sub_080133CE             ; Barrier / Aura / elemental / defense resolution on [r6, #0x28]
      bl   sub_08013410
  08013A5E:
      ldrh r4, [r6, #0x28]          ; r4 = incoming hit damage (u16)
      tst  r4, r4
      beq  _08013AA4
      adds r0, r4, #0
      bl   sub_0800C75C             ; hit reaction / damage popup setup
      ...                           ; Undershirt check (clamps r4 to cur_hp - 1)
  _08013A7E:
      adds r0, r4, #0
      bl   sub_0800AB9E             ; subtracts r0 from [r5, #0x24] (MegaMan cur_hp)
  ```
- **Recommended hook point:** **`0x08013A56`** (right before `bl sub_080133CE`, with `r6 = [r5, #0x54]` already loaded):
  - If `[0x020015EE] != 0`: load `r0 = [r6, #0x28]`, compute `r0 = r0 + (r0 >> 1)` (`× 1.5`), store back to `[r6, #0x28]`, then call `sub_080133CE`.
  - **Why before `sub_080133CE`:** Barriers and Auras (e.g., LifeAura 200) test against the true × 1.5 attack power, Undershirt still saves MegaMan at 1 HP, and `sub_0800C75C` / `sub_0800AB9E` receive the exact scaled damage.
  - **Covers 100% of enemy attacks** (every Navi's attacks, projectiles, contact hits, Bass, and Duo) in one hook, without touching any boss AI or `base_atk` table. *(Note: poison/bug tick damage of 1 HP goes through `sub_0800AB92` directly and is intentionally excluded.)*

---

## 4. Speed × 1.25 (Enemy Side Only) (V)

- **Why per-boss timer hooks aren't needed:** Boss AI wait timers are local per-boss state fields spread across `code_07`–`code_09`, **but every single battle object is ticked once per frame by one central dispatcher: `BN4_OBJECT_RUNNER` (`sub_0800314C`, `code_00.s`, RS & BM identical).**
- **Exact disassembly (`0x0800314C`–`0x080031A0`, RS & BM identical):**
  ```arm
  _08003156:
      ldr  r7, [r7, #4]             ; next node in active battle object linked list (0x02009E40..0x0200A4C0)
      ...
      movs r5, #0x10
      adds r5, r5, r7               ; r5 = battle object struct
      ...                           ; skips if paused or BN4_TIME_STOP (sub_08007948)
  08003184:
      ldrb r0, [r5, #2]             ; object kind: 1 = actor (MegaMan/enemies), 3 = attacks/projectiles, 4 = effects
      movs r1, #0xf
      ands r0, r1
      lsls r0, r0, #2
      ldr  r1, _080031B4            ; dat_080031B8 (BN4_OBJECT_KINDS)
      ldr  r0, [r0, r1]
      ldrb r1, [r5, #1]             ; object subtype / AI index
      lsls r1, r1, #2
      ldr  r0, [r0, r1]
      push {r7}
      mov  lr, pc
      bx   r0                       ; <--- 1-frame tick of this battle object
      pop  {r7}
  _0800319E:
      b    _08003156
  ```
- **How to apply exact × 1.25 speed to the enemy side only:**
  1. Maintain a 1-byte counter `sp_phase` (e.g. `0x020015ED`), incremented once per `BN4_OBJECT_RUNNER` call (`sp_phase = (sp_phase + 1) & 3`).
  2. Hook `0x08003196` (around `bx r0`): after the normal tick returns, if **all** of the following hold:
     - `[0x020015EE] != 0` (SP+ active),
     - `sp_phase == 0` (**1 out of every 4 frames → 5 ticks per 4 frames = exact × 1.25 speed**),
     - `([r5, #2] & 0x0F) == 1` (enemy actor; optionally also `== 3` for enemy projectiles/attacks — test both to see which feels best),
     - `[r5, #0x16] == 1` (**enemy side only** — MegaMan is side `0`, untouched),
     - `[r5] & 1 != 0` and `[r5, #0x24] != 0` (enemy still alive and not mid-deletion),
     then call **`bx r0` a second time**!
- **Caveats / testing notes:**
  - Because `BN4_OBJECT_RUNNER` already skips objects during `BN4_TIME_STOP` (Dimming chips / Custom Screen / intro), the extra tick automatically pauses during time-stops and never runs when MegaMan is frozen.
  - Double-ticking `kind == 1` speeds up **every** boss's movement, attack wind-ups, cooldowns, and animations by exactly × 1.25 uniformly across all 20 Navis, Bass, and Duo. Including `kind == 3` (`side == 1`) also speeds up enemy projectiles by × 1.25; both should be playtested on `mmbn4.exe` (especially multi-part bosses like Duo) to confirm no visual desync between body (`kind 1`) and detached fists/lasers (`kind 3`).

---

## 5. Summary of Hook Addresses (RS == BM for All Three)

| Modifier | Address (RS & BM) | Routine | Action when `[0x020015EE] != 0` |
|---|---|---|---|
| **HP = 4,095** | `0x08006802` | `sub_080067B4` (enemy spawn) | If `enemy_id >= 0x80`, set `r2 = 0x0FFF` (`4095`) before storing `[r4, #0x24]` & `[r4, #0x26]` |
| **Attack × 1.5** | `0x08013A56` | `sub_08013A48` (MegaMan hit) | Scale `[r6, #0x28]` by `dmg + (dmg >> 1)` before Barrier/Aura/Undershirt |
| **Speed × 1.25** | `0x08003196` | `BN4_OBJECT_RUNNER` (`sub_0800314C`) | Every 4th frame (`sp_phase == 0`), run a 2nd tick on live `side == 1` enemy objects (`kind == 1`, optionally `kind == 3`) |
| **Clear flag** | `0x0802174C` (RS) / `0x08021750` (BM) | `BattleResultsGrantReward` (+ battle exit) | Reset `[0x020015EE] = 0` |
