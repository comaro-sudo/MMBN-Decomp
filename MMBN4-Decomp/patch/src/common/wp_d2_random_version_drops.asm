; ============================================================
; WP-D2 / Violet Eclipse: version drops at random (owner decision 2026-11-23, plan/DEFINITIVE-EDITION.md
; section 7 question 4, rule (a)). Wherever Red Sun's and Blue Moon's drop rows differ (61 virus families,
; Bass, BassOmega, BassXX: RE-06, decomp/notes/re06-encounters.md section 7), a battle's drop comes from
; either version's row, 50/50, each battle.
;
; How: every path of BN4_REWARD_PICK gets its drop row from RewardGetRowPtr (r0 row -> &BN4_REWARD_ROWS[row]).
; That 12-byte routine is rewritten to call RandomRowPtr, which returns the row from this ROM's table or,
; on one RNG bit, the same row from the OTHER version's table. That copy is read from the player's other ROM
; at build time (build.py, OTHER_REWARD_ROWS) and placed in free space. The choice is made once, before the
; slot is picked, so the reveal and the grant always agree. Rows that don't differ give the same result.
; Side effect: one extra RNG draw per drop-row lookup (it shifts the RNG sequence, not its odds).
; RandomRowPtr sits in the second dead 86-byte block of code_03 (after an unconditional b; unreferenced).
; ============================================================

.org REWARD_ROW_PTR                 ; RewardGetRowPtr: 12 bytes of code + its literal
    push    r14
    bl      RandomRowPtr
    pop     r15

.org RANDOM_ROW_SPACE
RandomRowPtr:                       ; r0 row -> r0 row pointer; keeps r3 (a caller passes it on)
    push    r3, r4, r14
    mov     r4, r0
    bl      GET_RNG                 ; r0 = random (r1-r3 clobbered)
    mov     r1, 1
    and     r0, r1                  ; flags from the coin
    ldr     r0, =REWARD_ROWS        ; ldr keeps the flags
    beq     @@own
    ldr     r0, =FS_OTHER_REWARD_ROWS
@@own:
    mov     r1, 0x28
    mul     r1, r4
    add     r0, r0, r1
    pop     r3, r4, r15
    .pool
.if . > RANDOM_ROW_SPACE + 86
    .error "WP-D2 random drops: helper overruns the dead 86-byte block"
.endif

.org FS_OTHER_REWARD_ROWS
    .incbin OTHER_REWARD_ROWS       ; 276 x 40 bytes, the other version's BN4_REWARD_ROWS
.if . > FS_OTHER_REWARD_ROWS + FS_OTHER_REWARD_ROWS_SIZE
    .error "WP-D2 random drops: table overruns its free-space slot"
.endif
