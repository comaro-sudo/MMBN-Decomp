; ============================================================
; WP-B2 / Violet Eclipse: Frequency Tuner (V1 / V2 / V3 / Auto virus tier override).
;
; Hooks BN4_ID_RAISE (0x0800B050, identical in RS and BM) to read a 2-bit Frequency Tuner
; setting from free event flags 0x0E58..0x0E59:
;   00 = Auto (vanilla playthrough count [sl+0x44]+0x0D, capped at 2)
;   01 = Tier 1 (V1 / PT1 -> r3 = 0)
;   10 = Tier 2 (V2 / PT2 -> r3 = 1)
;   11 = Tier 3 (V3 / PT3 -> r3 = 2)
;
; Exposes FC 0B 00 (cycles 00 -> 01 -> 10 -> 11 -> 00 with confirm SFX 0x73), invoked by
; freq_tuner_npc inside Higsby's (00,06) at (-100, 20, 0) (talk_idx 0xFE, always visible).
; ============================================================

; ---- 1. Hook BN4_ID_RAISE playthrough read (0x0800B050, same in RS & BM) ---------------------
.org 0x0800B050                     ; word-aligned; 12 bytes before 0x0800B05C
    ldr     r3, [r15, 0]
    bx      r3
    .word   freq_tuner_id_raise | 1

; ---- 2. Hook code, FC 0B cycler, and Higsby's Tuner NPC in free space ------------------------
.org FS_FREQ_TUNER

freq_tuner_id_raise:
    ; Preserve r0-r2, r4-r5 (r4 = enemy_id, r5 = BN4_ENEMY_RECORD ptr); return tier (0..2) in r3.
    push    r0, r1, r2, r6, r7
    mov     r6, 0
    ldr     r0, =0x0E58
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      freq_tuner_call_r3
    beq     @@no_b0
    add     r6, 1
@@no_b0:
    ldr     r0, =0x0E59
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      freq_tuner_call_r3
    beq     @@no_b1
    add     r6, 2
@@no_b1:
    cmp     r6, 0
    beq     @@auto
    sub     r3, r6, 1               ; 1->0 (V1), 2->1 (V2), 3->2 (V3)
    b       @@done
@@auto:
    mov     r3, r10
    ldr     r3, [r3, 0x44]
    ldrb    r3, [r3, 0x0D]
    cmp     r3, 2
    ble     @@done
    mov     r3, 2
@@done:
    pop     r0, r1, r2, r6, r7
    ldr     r2, =0x0800B05C | 1
    bx      r2

; -- FC 0B 00: cycle Frequency Tuner mode (0 -> 1 -> 2 -> 3 -> 0) and play confirm SFX --
.align 2
freq_tuner_fc_cycle:
    push    r4, r5, r6, r7, r14
    mov     r6, 0
    ldr     r0, =0x0E58
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      freq_tuner_call_r3
    beq     @@c_no0
    add     r6, 1
@@c_no0:
    ldr     r0, =0x0E59
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      freq_tuner_call_r3
    beq     @@c_no1
    add     r6, 2
@@c_no1:
    add     r6, 1
    mov     r1, 3
    and     r6, r1                  ; (mode + 1) & 3
    ; Write bit 0 -> 0x0E58
    ldr     r0, =0x0E58
    mov     r1, 1
    tst     r6, r1
    beq     @@clr0
    ldr     r3, =SET_EVENT_FLAG | 1
    b       @@w0
@@clr0:
    ldr     r3, =CLEAR_EVENT_FLAG | 1
@@w0:
    bl      freq_tuner_call_r3
    ; Write bit 1 -> 0x0E59
    ldr     r0, =0x0E59
    mov     r1, 2
    tst     r6, r1
    beq     @@clr1
    ldr     r3, =SET_EVENT_FLAG | 1
    b       @@w1
@@clr1:
    ldr     r3, =CLEAR_EVENT_FLAG | 1
@@w1:
    bl      freq_tuner_call_r3
    mov     r0, 0x73                ; confirm SFX
    ldr     r3, =PLAY_SFX | 1
    bl      freq_tuner_call_r3
    pop     r4, r5, r6, r7
    add     r4, 3                   ; advance past FC 0B 00
    mov     r0, 1
    pop     r15

freq_tuner_call_r3:
    bx      r3
    .pool

; -- Always-visible Frequency Tuner NPC inside Higsby's (00,06) at (-100, 20, 0) --
.align 4
freq_tuner_npc:
    .byte   0x08
    .byte   0x17, 0x0F              ; list[7] technician/man sprite
    .byte   0x18, 0xFE              ; talk_idx 0xFE -> @@sE ("Tune Tier? Yes/No")
    .byte   0x14
    .halfword (-100) & 0xFFFF, 20, 0
    .byte   0x16, 3
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP

.if . > FS_FREQ_TUNER + FS_FREQ_TUNER_SIZE
    .error "WP-B2 Frequency Tuner: overruns free-space slot"
.endif
