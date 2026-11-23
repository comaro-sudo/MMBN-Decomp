; ============================================================
; WP-D2 / WP-D4: beating Duo gives the Duo chip (D) and the Duo Soul.
; The story's final battle (story battles 93/94) skips the results screen, so the drop table never
; pays out there (wp_d2_duo_chip_reward.asm only works in rematches). Gemini found Duo's deletion state
; (decomp/notes/duo-gpp-research.md section 4, option A), state 2 of Duo's table at 0x080C474C:
;
;   sub_080C4780 (RS 0x080C4780 / BM 0x080C478C):
;       if (r5[0x0A] == 0 && r5[0x04] != 0) SetEventFlagHL(0, 0x6F)
;       sub_08010D4C()                                               ; uses r5 only
;
; r5[0x04] is Duo's version (0 Duo, 1 alpha, 2 beta, 3 omega: Duo's init sub_080C46B8 picks the palette
; from it), so flag 0x006F (the final-boss star) only comes from a harder Duo. r5[0x0A] == 0 is the
; deletion's first sub-state.
;
; Its entry becomes a thunk to free space (a bl can't reach 0x087F....). The new handler, when
; r5[0x0A] == 0, on any version (a won rematch is also paid by the lobby results hook: Gemini's
; round-11 test saw no grant from this state in a plain battle):
;   - the Duo Soul, if not owned: flags 0x0015 (Double Soul) and 0x0023, WriteChipKey(0x16D)
;     (owner: "beat the first version of Duo"; V1 is always the first one beaten);
;   - the Duo chip D, once: guarded by flag 0x0E72 (free, Gemini's 0x0E3B-0x0E7F scan; after SP+'s
;     0x0E5D-0x0E71), so it comes even on a first playthrough, where 0x006F stays clear;
;   - then the original: flag 0x006F when the version isn't 0.
; ============================================================

DUO_CHIP_GIVEN_FLAG equ 0x0E72

.org DUO_DEFEAT_STATE               ; word-aligned in both versions; the 20 bytes after the thunk go unused
    ldr     r3, [r15, 0]            ; = the literal below (pc = this + 4)
    bx      r3
    .word   duo_defeat_hook | 1

.org FS_DUO_GRANT
duo_defeat_hook:
    push    r14
    ldrb    r0, [r5, 0x0A]
    cmp     r0, 0
    bne     @@tail
    bl      duo_grant_rewards
    ldrb    r0, [r5, 0x04]          ; the original: 0x006F for a harder Duo
    cmp     r0, 0
    beq     @@tail
    mov     r0, 0x6F
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      duo_call_r3
@@tail:
    ldr     r3, =0x08010D4C | 1     ; sub_08010D4C, same in RS and BM
    bl      duo_call_r3
    pop     r15

; The Duo Soul if not owned, and Duo D once. Also called by the lobby results hook for a won Duo rematch.
; Uses r0-r3 only.
duo_grant_rewards:
    push    r14
    mov     r0, 0x23                ; Duo Soul owned?
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      duo_call_r3
    bne     @@chip
    mov     r0, 0x15                ; Double Soul unlocked
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      duo_call_r3
    mov     r0, 0x23                ; DuoSoul
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      duo_call_r3
    ldr     r0, =0x16D              ; DuoSoul's pseudo-chip key
    ldr     r3, =WRITE_CHIP_KEY | 1
    bl      duo_call_r3
@@chip:
    ldr     r0, =DUO_CHIP_GIVEN_FLAG
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      duo_call_r3
    bne     @@done
    ldr     r0, =DUO_CHIP_GIVEN_FLAG
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      duo_call_r3
    ldr     r0, =CHIP_DUO
    mov     r1, 3                   ; code D
    mov     r2, 1
    ldr     r3, =CHIP_GIVE | 1
    bl      duo_call_r3
@@done:
    pop     r15
duo_call_r3:
    bx      r3
    .pool
.if . > FS_DUO_GRANT + FS_DUO_GRANT_SIZE
    .error "WP-D2 Duo story grant: overruns its free-space slot"
.endif
