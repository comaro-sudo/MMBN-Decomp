; ============================================================
; WP-D2 / WP-D4: beating Duo gives the Duo Soul and Duo D.
; The story's final battle (story battles 93/94) skips the results screen, so the drop table never
; pays out there (wp_d2_duo_chip_reward.asm only works in rematches). Round 12 (duo-story-grant-test.md)
; proved that sub_080C4780 is Bass (sets flag 0x006F), whereas Duo (object kind 1, ID 0x2A) dispatches
; its deletion state through the 4-byte table pointer dat_080C04F8[2] (RS 0x080C0500 / BM 0x080C050C),
; originally pointing to sub_08010DF8 (RS == BM, exclusively referenced by Duo).
;
; Replacing that 4-byte pointer needs no veneer and leaves Bass's sub_080C4780 untouched. When
; r5[0x0A] == 0 (first frame of Duo's deletion state, in story OR rematch, any version V1/a/b/o):
;   - the Duo Soul, if not owned: flags 0x0015 (Double Soul) and 0x0023, WriteChipKey(0x16D);
;   - the Duo chip D, once: guarded by flag 0x0E72;
;   - then tail-calls the original sub_08010DF8.
; ============================================================

DUO_CHIP_GIVEN_FLAG equ 0x0E72

.org DUO_DELETE_STATE_PTR           ; dat_080C04F8[2] (RS 0x080C0500 / BM 0x080C050C)
    .word   duo_defeat_hook | 1

.org FS_DUO_GRANT
duo_defeat_hook:
    push    r14
    ldrb    r0, [r5, 0x0A]
    cmp     r0, 0
    bne     @@tail
    bl      duo_grant_rewards
@@tail:
    ldr     r3, =0x08010DF8 | 1     ; original Duo deletion state (RS == BM)
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
