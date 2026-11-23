; ============================================================
; WP-A1: WoodMan scenario fix (Prof. 9, "WoodMan Glitch Fix v2.0", ext/BN4-WoodMan-Fix; free to use,
; attribution welcome). The routine at WOODMAN_FIX+2 counts frames in [r7+2]; once the count passes
; 0x17 it keeps rising and, on the DS and some emulators, the scenario can crash. The fix reassembles
; the routine 2 bytes earlier (into the zero pad after its literal) to make room for one instruction
; that holds the count at 0x17, then repoints the routine's table entry at WOODMAN_FIX-0x3E.
; Mechanism and both versions' pointers verified: docs/re/save-and-flags.md §3.
; ============================================================
.org WOODMAN_FIX - 0x3E
    .word WOODMAN_FIX | 1               ; the routine's pointer, was WOODMAN_FIX+2 | 1

.org WOODMAN_FIX
    push    {r14}
    mov     r0, 0x3
    strb    r0, [r5]
    bl      WOODMAN_FIX + 0x96
    beq     WOODMAN_FIX + 0x60
    ldrb    r0, [r7, 0x2]
    add     r1, r0, 0x1
    strb    r1, [r7, 0x2]
    cmp     r0, 0x17
    bne     WOODMAN_FIX + 0x1E
    strb    r0, [r7, 0x2]               ; the fix: hold the count at 0x17
    mov     r2, 0x4
    strb    r2, [r7, 0x3]               ; ends at the original's +0x1C; the rest of the routine is unchanged
