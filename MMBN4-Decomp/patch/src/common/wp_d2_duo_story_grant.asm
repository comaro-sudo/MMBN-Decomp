; ============================================================
; WP-D2: beating Duo in the story gives the Duo chip (D).
; The story's final battle (story battles 93/94) skips the results screen, so the drop table never
; pays out (wp_d2_duo_chip_reward.asm only works in rematches). Gemini found the state handler that runs
; when Duo is defeated (decomp/notes/duo-gpp-research.md section 4, option A):
;
;   sub_080C4780 (RS 0x080C4780 / BM 0x080C478C), called through Duo's state table:
;       if (r5[0x0A] == 0 && r5[0x04] != 0) SetEventFlagHL(0, 0x6F)   ; Duo defeated
;       sub_08010D4C()                                               ; uses r5 only
;
; Its entry becomes a thunk to free space (a bl can't reach 0x087F....). The new handler does the same,
; and in the branch that sets flag 0x006F it first gives Duo D, but only if 0x006F was still clear, so
; the chip comes once even if the handler runs on several frames, and a post-game rematch (flag already
; set) doesn't give a second copy through this path.
; Later: the Duo Soul grant (flag 0x0023 + WriteChipKey(0x16D), docs/pending/duo-soul.md) goes here too.
; ============================================================

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
    ldrb    r0, [r5, 0x04]
    cmp     r0, 0
    beq     @@tail
    mov     r0, 0x6F                ; Duo defeated
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      @@call_r3               ; Z clear = already set
    bne     @@set
    ldr     r0, =CHIP_DUO
    mov     r1, 3                   ; code D
    mov     r2, 1
    ldr     r3, =CHIP_GIVE | 1
    bl      @@call_r3
@@set:
    mov     r0, 0x6F
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      @@call_r3
@@tail:
    ldr     r3, =0x08010D4C | 1     ; sub_08010D4C, same in RS and BM
    bl      @@call_r3
    pop     r15
@@call_r3:
    bx      r3
    .pool
.if . > FS_DUO_GRANT + FS_DUO_GRANT_SIZE
    .error "WP-D2 Duo story grant: overruns its free-space slot"
.endif
