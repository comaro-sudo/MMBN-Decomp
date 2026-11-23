; ============================================================
; WP-D4 step 4: script command FC 0C <idx> grants any of the 12 Double Souls, in either version.
; Used by the tournament friendly matches (a win gives that Navi's Soul; owner decision) and by any later
; script. Design: Gemini, decomp/notes/soul-grant-patch-design.md (and soul-grant.md): a Soul is granted by
; event flag 0x0015 (Double Soul unlocked) + flag 0x17 + idx + WriteChipKey(0x161 + idx); WriteChipKey
; applies the running ROM's own XOR key, so the same code works in RS and BM.
;
; idx: 0 Roll, 1 Guts, 2 Wind, 3 Search, 4 Fire, 5 Thunder, 6 Proto, 7 Number, 8 Metal, 9 Junk, 10 Aqua, 11 Wood
; (the BN4_SOUL_FLAGS order). The handler follows the ScriptGroup_FC convention used by the lobby and the
; Frequency Tuner handlers: r4 = script pointer, argument at [r4, 2]; advance r4 by 3; return r0 = 1.
; The FC table (wp_d3_lobby_rematches.asm, lobby_fc_table) gets this as entry 0x0C.
; ============================================================

.org FS_GRANT_SOUL
grant_soul_fc:                      ; FC 0C <idx>
    push    r4-r7, r14
    ldrb    r5, [r4, 2]             ; idx 0..11
    cmp     r5, 11
    bhi     @@done                  ; ignore a bad index
    mov     r0, 0x15                ; Double Soul unlocked
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      @@call_r3
    mov     r0, r5
    add     r0, 0x17                ; the Soul's flag
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      @@call_r3
    ldr     r0, =0x161
    add     r0, r0, r5              ; the Soul's pseudo-chip key
    ldr     r3, =WRITE_CHIP_KEY | 1
    bl      @@call_r3
@@done:
    pop     r4-r7
    add     r4, 3                   ; past FC 0C <idx>
    mov     r0, 1
    pop     r15
@@call_r3:
    bx      r3
    .pool
.if . > FS_GRANT_SOUL + FS_GRANT_SOUL_SIZE
    .error "WP-D4 grant Soul: overruns its free-space slot"
.endif
