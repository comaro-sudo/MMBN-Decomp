; ============================================================
; WP-D3: SP+ Boss Difficulty Tier (4,095 HP, ATK x1.5, Speed x1.25, 80% 15,000Z / 20% 50 BF).
; Design: Gemini, decomp/notes/sp-plus-design-2.md (round 11), owner decision 0f446cc.
;
; Flags (0x0E5C-0x0E71):
;   0x0E5C          Duo SP+ unlocked (S-rank Duo-omega)
;   0x0E5D-0x0E68   12 Soul Navis SP+ unlocked (0x0E5D + navi_id, S-rank SP in lobby)
;   0x0E69-0x0E71   10 roaming SP bosses (TopM-omega..Bass-omega/BassXX, S-rank in world)
;
; EWRAM bytes (0x020015EC-0x020015EF, cleared on every battle exit by wp_d3_battle_exit_clear.asm):
;   0x020015EC      roaming SP enemy_id (0 if none)
;   0x020015ED      SP+ speed phase counter (0..3)
;   0x020015EE      SP+ active (1 = active, 0 = normal)
;   0x020015EF      lobby/Duo rematch marker (bit 7 + id)
; ============================================================

; ---- Hook 1: HP = 4,095 + roaming SP scan (0x08006806, RS == BM) ----
.org 0x08006806                     ; replaces 4B: ldrh r1, [r7]; adds r2, r1, #0
    bl      VENEER_SP_PLUS_1
.org VENEER_SP_PLUS_1
    ldr     r3, [r15, 0]
    bx      r3
    .word   sp_plus_hp_hook | 1

; ---- Hook 2: Attack x1.5 on MegaMan hits (0x08013A06, RS == BM) ----
.org 0x08013A06                     ; replaces 4B: bl sub_08013410
    bl      VENEER_SP_PLUS_2
.org VENEER_SP_PLUS_2
    ldr     r3, [r15, 0]
    bx      r3
    .word   sp_plus_atk_hook | 1

; ---- Hook 3: Speed x1.25 in BN4_OBJECT_RUNNER (0x08003196, RS == BM) ----
.org 0x08003196                     ; replaces 8B: push {r7}; mov lr, pc; bx r0; pop {r7}
    bl      VENEER_SP_PLUS_3
    nop
    nop
.org VENEER_SP_PLUS_3
    ldr     r3, [r15, 0]
    bx      r3
    .word   sp_plus_spd_hook | 1

; ---- Hook 4: Random 80% 15,000Z (0x7A98) / 20% 50 BF (0xC032) reward ----
.org SP_REWARD_HOOK                 ; RS 0x080F57F6 / BM 0x080F5802 (replaces bl BN4_REWARD_PICK)
    bl      VENEER_SP_REWARD
.org VENEER_SP_REWARD
    ldr     r3, [r15, 0]
    bx      r3
    .word   sp_plus_reward_hook | 1

; ---- Free space ------------------------------------------------------------
.org FS_SP_PLUS

.align 2
sp_plus_hp_hook:
    push    r0, r3, r4, r14
    mov     r4, r8                  ; r4 = enemy_id
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
    ldrb    r0, [r3, 1]             ; flag_lo (0x69..0x71)
    ldr     r1, =0x0E00
    orr     r0, r1
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      sp_call_r3
    beq     @@check_active
    ldr     r0, =0x020015EE         ; unlocked -> activate SP+
    mov     r1, 1
    strb    r1, [r0]
@@check_active:
    ldrh    r1, [r7]                ; r1 = original hp_elem (preserves element bits 12-15)
    mov     r2, r1
    ldr     r0, =0x020015EE
    ldrb    r0, [r0]
    cmp     r0, 0
    beq     @@done
    cmp     r4, 0x80                ; bosses / Navis only
    blt     @@done
    ldr     r0, =0x0FFF
    orr     r2, r0                  ; r2 low 12 bits = 0x0FFF (4,095 HP)
@@done:
    pop     r0, r3, r4, r15

.align 2
sp_plus_atk_hook:                   ; replaces bl sub_08013410 at 0x08013A06 (r6 = [r5+0x54])
    push    r14
    ldr     r3, =0x08013410 | 1
    bl      sp_call_r3
    ldr     r0, =0x020015EE
    ldrb    r0, [r0]
    cmp     r0, 0
    beq     @@done
    ldrh    r0, [r6, 0x28]          ; damage taken by MegaMan
    cmp     r0, 0
    beq     @@done
    lsr     r1, r0, 1
    add     r0, r0, r1              ; dmg * 1.5
    strh    r0, [r6, 0x28]
@@done:
    pop     r15

.align 2
sp_plus_spd_hook:                   ; replaces push {r7}; mov lr,pc; bx r0; pop {r7} at 0x08003196
    push    r0, r5, r7, r14
    bl      sp_call_r0              ; 1st (normal) tick
    ldrb    r1, [r5, 2]
    mov     r2, 0x0F
    and     r1, r2
    cmp     r1, 1                   ; object kind 1 (MegaMan / Navis / viruses)
    bne     @@done
    ldrb    r2, [r5, 0x16]          ; side: 0 = MegaMan, 1 = enemy
    cmp     r2, 0
    bne     @@enemy
    ldr     r3, =0x020015ED         ; advance phase 0..3 once per frame on MegaMan's tick
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
    ldrh    r1, [r5, 0x24]          ; cur_hp > 0?
    cmp     r1, 0
    beq     @@done
    ldr     r0, [sp, 0]             ; reload tick fn
    bl      sp_call_r0              ; 2nd tick
@@done:
    pop     r0, r5, r7, r15

.align 2
sp_plus_reward_hook:                ; replaces bl BN4_REWARD_PICK at SP_REWARD_HOOK
    push    r14
    ldr     r3, =SP_REWARD_PICK | 1
    bl      sp_call_r3              ; r0 = normal reward word
    ldr     r1, =0x020015EE
    ldrb    r1, [r1]
    cmp     r1, 0
    beq     @@done
    ldr     r3, =GET_RNG | 1
    bl      sp_call_r3
    lsr     r0, r0, 8
    mov     r1, 0xFF
    and     r0, r1
    cmp     r0, 51                  ; 51/256 (~20%) -> 50 BF; 205/256 (~80%) -> 15,000Z
    blt     @@bf
    ldr     r0, =0x7A98             ; 0x4000 | 15000 = 15,000 Zenny
    b       @@done
@@bf:
    ldr     r0, =0xC032             ; 0xC000 | 50 = 50 BugFrags
@@done:
    pop     r15

; Helper called by lobby_fc_rematch (r0 = 0x0E5D + navi_id) and duo_fc_rematch (r0 = 0x0E5C)
; when tier == 3: sets [0x020015EE] = 1 if that boss's SP+ flag is already set.
.align 2
sp_plus_check_activate:
    push    r14
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      sp_call_r3
    beq     @@off
    ldr     r0, =0x020015EE
    mov     r1, 1
    strb    r1, [r0]
@@off:
    pop     r15

; Helper called by lobby_results_hook on non-lobby battles: if [0x020015EC] is a roaming SP boss
; and Busting Level ([r5, 8]) >= 11 (S rank), sets its unlock flag (0x0E69..0x0E71).
.align 2
sp_plus_unlock_roam:
    push    r4, r14
    ldr     r0, =0x020015EC
    ldrb    r4, [r0]
    cmp     r4, 0
    beq     @@done
    ldrb    r0, [r5, 8]             ; Busting Level
    cmp     r0, 11
    blt     @@done
    ldr     r3, =sp_roam_table
@@scan:
    ldrb    r0, [r3, 0]
    cmp     r0, 0xFF
    beq     @@done
    cmp     r0, r4
    beq     @@hit
    add     r3, 2
    b       @@scan
@@hit:
    ldrb    r0, [r3, 1]
    ldr     r1, =0x0E00
    orr     r0, r1
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      sp_call_r3
@@done:
    pop     r4, r15

sp_call_r3:
    bx      r3
sp_call_r0:
    bx      r0
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

.if . > FS_SP_PLUS + FS_SP_PLUS_SIZE
    .error "WP-D3 SP+: overruns its free-space slot"
.endif
