# SP+ Boss Difficulty: Completed Hook Spec & 80/20 Reward (Round 11, Task 5, 2026-11-23)

Completes `sp-plus-design.md` with: full roaming-SP table scan (Hook 1), full `BN4_OBJECT_RUNNER` speed ×1.25 code (Hook 3), confirmation of Claude's hook-site changes, Duo SP+ on flag `0x0E5C`, and the revised **80% 15,000 Zenny (`0x7A98`) / 20% 50 BugFrags (`0xC032`)** random SP+ reward (owner decision `0f446cc`). All addresses verified in both US ROMs.

---

## 1. Confirmation of Claude's Hook-Site Changes (V)

1. **Hook 1 (HP = 4,095 at `0x08006802`, RS == BM) — CONFIRMED!**
   - `0x08006802` is `ldrh r1, [r7]; adds r2, r1, #0` (4 bytes: `39 88 0A 1C`).
   - Replacing those 4 bytes with `bl VENEER_SP_HP` returning `r1` raw (so `0x08006812: lsrs r1, r1, #0xC` still extracts the boss's element untouched!) and `r2 = r1 | 0x0FFF` (so `lsls r2, #20; lsrs r2, #20` at `0x08006806..0x08006808` yields **`0x0FFF = 4,095`**!) is **cleaner and 4 bytes shorter** than replacing 8 bytes!
2. **Hook 2 (Attack ×1.5 at `0x08013A5A`, RS == BM) — CONFIRMED!**
   - At `0x08013A5A` (`bl sub_08013410`, 4 bytes: `FF F7 D9 FC`), `[r6, #0x28]` is only read at `0x08013A5E` (`ldrh r4, [r6, #0x28]`), so hooking `bl sub_08013410` and scaling `[r6, #0x28]` by `dmg + (dmg >> 1)` **after** `sub_08013410` returns works cleanly (note: scaling *after* `sub_080133CE` means Barriers/Auras test against base damage, while Undershirt and MegaMan's HP take the full ×1.5 hit — completely safe!).
3. **Veneer slots — CONFIRMED (note: 20 dead bytes = 2 × 8-byte veneers, 4 bytes left):**
   - `RS 0x080C4788 / BM 0x080C4794`..`+0x14` holds **2** word-aligned 8-byte veneers (`+0x00` and `+0x08`).
   - All `code_00` / `code_01` hooks (`0x08003194`, `0x08006802`, `0x08006E20`, `0x080107E8`, `0x08013A5A`) are within ±4 MB Thumb `bl` range of `code_07` (`0x080C4788`), AND any additional 8-byte veneers can go in other dead padding inside `code_00`–`code_02` (e.g. `0x08021740` tail after `BassPairGive`, or `wp_d4_status_two_pages.asm` freed bytes).
4. **Duo SP+ flag:** **`0x0E5C`** ("Duo SP+ unlocked", right after Duo's 2-bit tier `0x0E5A..0x0E5B`, leaving `0x0E72` untouched for `DUO_CHIP_GIVEN_FLAG`).

---

## 2. Hook 1: Full Roaming-SP Table Scan + HP = 4,095 (`0x08006802`, RS == BM) (V)

Live registers at `0x08006802`: `r4` (actor), `r7` (`BN4_ENEMY_RECORD`), `r8` (`enemy_id`). Scratch: `r0, r3` (save `r0, r3, lr`; return `r1` raw, `r2` = `r1 | 0x0FFF` if SP+ boss else `r1`).

```arm
; At 0x08006802 (replaces 4B: ldrh r1,[r7]; adds r2,r1,#0):
.org 0x08006802
    bl      VENEER_SP_HP

; In free space:
sp_plus_hp_hook:
    push    r0, r3, r4, r14
    mov     r4, r8                  ; r4 = enemy_id
    ; 1. Scan 10-entry roaming SP table (8 shared Navis + BassOmega + BassXX)
    ldr     r3, =sp_roam_table
@@scan:
    ldrb    r0, [r3, 0]
    cmp     r0, 0xFF
    beq     @@check_active
    cmp     r0, r4
    beq     @@roam_hit
    add     r3, 2
    b       @@scan
@@roam_hit:
    ldr     r0, =0x020015EC         ; record roaming SP enemy_id for results hook
    strb    r4, [r0]
    ldrb    r0, [r3, 1]             ; flag low byte (0x69..0x71)
    ldr     r1, =0x0E00
    orr     r0, r1                  ; r0 = 0x0E69..0x0E71
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      @@call_r3
    beq     @@check_active          ; not yet S-ranked -> fight normal SP
    ldr     r0, =0x020015EE         ; unlocked -> activate SP+!
    mov     r1, 1
    strb    r1, [r0]
@@check_active:
    ldrh    r1, [r7]                ; r1 = original hp_elem (preserved for element!)
    mov     r2, r1                  ; r2 = default hp_elem
    ldr     r0, =0x020015EE
    ldrb    r0, [r0]
    cmp     r0, 0
    beq     @@done
    cmp     r4, 0x80                ; bosses / Navis only (viruses/objects untouched)
    blt     @@done
    ldr     r0, =0x0FFF
    orr     r2, r0                  ; r2 low 12 bits = 0x0FFF (4,095 HP)
@@done:
    pop     r0, r3, r4, r15
@@call_r3:
    bx      r3
    .pool

sp_roam_table:                      ; { enemy_id, flag_lo (0x0E00 | flag_lo) }
    .byte   0xB3, 0x69              ; TopM-Omega    -> 0x0E69
    .byte   0xB7, 0x6A              ; ShadM-Omega   -> 0x0E6A
    .byte   0xBB, 0x6B              ; BurnM-Omega   -> 0x0E6B
    .byte   0xBF, 0x6C              ; ColdM-Omega   -> 0x0E6C
    .byte   0xC3, 0x6D              ; SprkM-Omega   -> 0x0E6D
    .byte   0xD3, 0x6E              ; LasrM-Omega   -> 0x0E6E
    .byte   0xD7, 0x6F              ; KendM-Omega   -> 0x0E6F
    .byte   0xDB, 0x70              ; VideM-Omega   -> 0x0E70
    .byte   0xE5, 0x71              ; Bass-Omega    -> 0x0E71
    .byte   0xE6, 0x71              ; BassXX        -> 0x0E71
    .byte   0xFF, 0xFF
```

---

## 3. Hook 3: Speed ×1.25 (`BN4_OBJECT_RUNNER`, `0x0800314C` & `0x08003194`, RS == BM) (V)

- **Per-frame phase counter (`0x020015ED`):** Hook `0x0800314C` (`push {r4, r5, r7, lr}; movs r0, #0` — or better, increment `0x020015ED` inside `sub_08006CB4` / `BN4_OBJECT_RUNNER` entry once per frame: `sp_phase = (sp_phase + 1) & 3`).
  - Even cleaner — **zero extra hook needed!** Increment `0x020015ED` inside `sp_plus_spd_hook` whenever `r5` is MegaMan (`([r5,#2]&0xF)==1 && [r5,#0x16]==0`, which runs **exactly once per `BN4_OBJECT_RUNNER` frame** before enemy actors!).
- **Exact site (`0x08003194`, RS == BM):**
  ```arm
  08003194: B480   push {r7}
  08003196: 46FE   mov  lr, pc
  08003198: 4700   bx   r0
  0800319A: BC80   pop  {r7}
  ```
  Replace the 8 bytes `0x08003194..0x0800319B` with `bl VENEER_SP_SPD; nop; nop` (4 + 2 + 2 = 8 B).
- **Live registers:** `r0` (object tick function), `r5` (battle object struct), `r7` (linked-list node). `r1–r3` scratch.

```arm
; In free space:
sp_plus_spd_hook:
    push    r0, r5, r7, r14         ; preserve tick fn (r0), obj (r5), list node (r7), lr
    bl      @@call_r0               ; 1st (normal) tick
    ; If this was MegaMan (kind 1, side 0), advance per-frame phase counter 0x020015ED once/frame
    ldrb    r1, [r5, 2]
    mov     r2, 0x0F
    and     r1, r2
    cmp     r1, 1
    bne     @@done
    ldrb    r2, [r5, 0x16]          ; side: 0 = MegaMan, 1 = enemy
    cmp     r2, 0
    bne     @@enemy
    ldr     r3, =0x020015ED
    ldrb    r1, [r3]
    add     r1, 1
    mov     r2, 3
    and     r1, r2
    strb    r1, [r3]
    b       @@done
@@enemy:
    ldr     r3, =0x020015EE         ; SP+ active?
    ldrb    r1, [r3]
    cmp     r1, 0
    beq     @@done
    ldr     r3, =0x020015ED         ; 1 in 4 frames (phase == 0 -> 5 ticks / 4 frames = x1.25)
    ldrb    r1, [r3]
    cmp     r1, 0
    bne     @@done
    ldrb    r1, [r5, 0]             ; object active bit 0?
    lsr     r1, r1, 1
    bcc     @@done
    ldrh    r1, [r5, 0x24]          ; cur_hp > 0 (not mid-deletion)?
    cmp     r1, 0
    beq     @@done
    ldr     r0, [sp, 0]             ; reload saved tick function r0
    bl      @@call_r0               ; 2nd tick!
@@done:
    pop     r0, r5, r7, r15
@@call_r0:
    bx      r0
    .pool
```

---

## 4. Revised SP+ Reward: Random 80% 15,000Z (`0x7A98`) / 20% 50 BugFrags (`0xC032`) (V)

- **Where `BN4_BATTLE_REWARD` (`0x0203BD4C`) is chosen for the results screen:**
  `BN4_REWARD_CHOOSER` (`code_09.s`, **`RS 0x080F57DC / BM 0x080F57E8`**):
  ```arm
  ; RS 0x080F57F4 / BM 0x080F5800 (4 bytes: bl BN4_REWARD_PICK):
      bl   sub_080F580C             ; returns 16-bit reward word in r0
      str  r0, [sp, #0x1c]          ; returned in r0 -> caller stores into 0x0203BD4C (BN4_BATTLE_REWARD)
  ```
- **Hook site:** Replace `bl sub_080F580C` at **`RS 0x080F57F4 / BM 0x080F5800`** with `bl VENEER_SP_REWARD` (or hook right after `BN4_REWARD_PICK` returns).
- **Exact 80/20 RNG code using `GET_RNG` (`0x08001B16`, RS == BM):**
  - `GET_RNG()` returns a 32-bit pseudo-random number in `r0`.
  - Compute `r0 = r0 & 0xFF` (0..255), or `r0 = (r0 >> 8) & 0xFF`:
    - `cmp r0, 51` (`51 / 256 = 19.92%`, exact 20% within 0.08%!) → `< 51` gives **`0xC032` (50 BugFrags)**, `>= 51` (`205 / 256 = 80.08%`) gives **`0x7A98` (15,000 Zenny)**!
    - *(Or exact integer `mod 5` via `0x08001B16` → `r0 = r0 & 0xFF`, subtract 5 loop / `svc 6`: `0` = 50 BugFrags 20.0%, `1..4` = 15,000Z 80.0%! Using `cmp r0, 51` takes 2 instructions and avoids SWI overhead.)*

```arm
; At RS 0x080F57F4 / BM 0x080F5800 (replaces bl BN4_REWARD_PICK):
sp_plus_reward_hook:
    push    r14
    ldr     r3, =BN4_REWARD_PICK | 1    ; RS 0x080F580D / BM 0x080F5819
    bl      @@call_r3                   ; r0 = normal reward word
    ldr     r1, =0x020015EE
    ldrb    r1, [r1]
    cmp     r1, 0
    beq     @@done                      ; not SP+ -> keep normal reward
    ldr     r3, =0x08001B16 | 1         ; GET_RNG (RS == BM)
    bl      @@call_r3
    lsr     r0, r0, 8
    mov     r1, 0xFF
    and     r0, r1                      ; r0 = 0..255
    cmp     r0, 51                      ; 51/256 (~20%) -> 50 BugFrags; 205/256 (~80%) -> 15,000Z
    blt     @@bugfrags
    ldr     r0, =0x7A98                 ; 0x4000 | 15000 = 15,000 Zenny
    b       @@done
@@bugfrags:
    ldr     r0, =0xC032                 ; 0xC000 | 50 = 50 BugFrags
@@done:
    pop     r15
@@call_r3:
    bx      r3
    .pool
```
*(In `lobby_results_hook`, when `[0x020015EE] != 0`, skip overwriting `BN4_BATTLE_REWARD` with a Secret Chip so the results screen displays and pays out the rolled 15,000Z or 50 BugFrags!)*
