; ============================================================
; WP-A2: battle Zenny x1.5 (owner decision 2026-11-22: built in, 1.5x).
;
; Every battle reward goes through one decoder (REWARD_DECODE): u16 0xFFFF none, bits 14-15 the kind
; (0 chip: id bits 0-8, code 9-13; 1 Zenny, 2 HP, 3 BugFrags: amount bits 0-13). Its three callers are the
; results screen's reveal, the grant (AddZenny) with its amount display, and the HP reward; none reads r2
; for kinds 1-3. Scaling kind 1 here keeps the shown and the granted amount equal. Shop prices and
; scripted Zenny (other AddZenny callers) are untouched. The rewrite fits the original 0x38 bytes.
; ============================================================
.org REWARD_DECODE
    ldr     r1, =0xFFFF
    cmp     r0, r1
    beq     @@none
    lsr     r1, r0, 14              ; kind
    bne     @@amount
    lsr     r2, r0, 9               ; chip: r0 0, r1 id, r2 code (as the original)
    lsl     r1, r0, 23
    lsr     r1, r1, 23
    mov     r0, 0
    mov     r15, r14
@@amount:
    lsl     r0, r0, 18
    lsr     r0, r0, 18              ; amount, 0-16383
    cmp     r1, 1
    bne     @@swap
    lsr     r2, r0, 1
    add     r0, r0, r2              ; Zenny x1.5 (at most 24574; AddZenny caps at 999999)
@@swap:
    mov     r2, r1
    mov     r1, r0                  ; r1 amount
    mov     r0, r2                  ; r0 kind
    mov     r15, r14
@@none:
    mov     r0, 0xFF
    mov     r15, r14
    .pool
.if . > REWARD_DECODE + 0x3C
    .error "WP-A2: decoder rewrite overruns the original"
.endif
