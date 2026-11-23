; ============================================================
; WP-B4: Green Mystery Data contents follow the Frequency Tuner (plan/03-phase1-rom-overhaul.md).
; Every GMD has 3 content blocks of 0x44 bytes (Normal / Hard / Super Hard), picked by
; MysteryDataGetContentTable (RS 0x080ADF5C / BM 0x080ADF68) as min(GetPlaythrough(), 2)
; (Gemini, decomp/notes/re07-mystery-data.md). With this patch the block follows the same setting as the
; virus tiers (WP-B2, flags 0x0E58-0x0E59): Auto = the playthrough as before, Tier 1/2/3 = block 0/1/2.
; So every GMD reward is reachable on any playthrough.
;
; The routine keeps its first instruction (push {r2-r7, lr}, which saves r3) and then jumps to free
; space; the rest of the routine is rebuilt there and returns with the original pop.
; ============================================================

.org GMD_CONTENT_TABLE              ; word-aligned in both versions
    push    r2-r7, r14
    ldr     r3, [r15, 4]            ; pc = +2 + 4, aligned -> literal at +8
    bx      r3
    .halfword 0
    .word   gmd_tier_block | 1

.org FS_GMD_TIER
gmd_tier_block:                  ; r0 Mystery Data type, r1 content list -> r0 (the tier's block for a GMD)
    mov     r4, r1
    cmp     r0, 5
    bne     @@plain
    mov     r6, 0                   ; Tuner mode 0..3 from flags 0x0E58 (bit 0), 0x0E59 (bit 1)
    ldr     r0, =0x0E58
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      @@call_r3
    beq     @@b1
    add     r6, 1
@@b1:
    ldr     r0, =0x0E59
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      @@call_r3
    beq     @@mode
    add     r6, 2
@@mode:
    sub     r0, r6, 1               ; Tier 1/2/3 -> block 0/1/2
    bge     @@block
    ldr     r3, =0x08000C44 | 1     ; Auto: GetPlaythrough (same in RS and BM)
    bl      @@call_r3
    cmp     r0, 2
    ble     @@block
    mov     r0, 2
@@block:
    lsl     r1, r0, 6               ; block * 0x44
    lsl     r0, r0, 2
    add     r0, r0, r1
    add     r0, r0, r4
    pop     r2-r7, r15
@@plain:
    mov     r0, r4
    pop     r2-r7, r15
@@call_r3:
    bx      r3
    .pool
.if . > FS_GMD_TIER + FS_GMD_TIER_SIZE
    .error "WP-B4 GMD tier: overruns its free-space slot"
.endif
