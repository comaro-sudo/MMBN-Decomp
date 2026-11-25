; ============================================================
; WP-D0 / Violet Eclipse: On-demand LZ77 sprite decompression for Higsby (0x08) & Sal (0x2C).
;
; In BN4_SPRITE_LIST_6 (RS 0x08027CE8 / BM 0x08027CEC), Higsby (0x08) and Sal (0x2C) have
; bit 31 = 1 (LZ77-compressed). On maps that do not preload them into BN4_SPRITE_CACHE
; (0x0200F720) — such as Higsby in DenDome Hall or Sal in Higsby's / Castillo WaitRm —
; BN4_SPRITE_LOAD (0x0800287C, RS == BM) falls back to the white-circle placeholder.
;
; Hook 0x08002894 (bl BN4_SPRITE_CACHED, RS == BM) -> VENEER_SPRITE_LAZY (0x0800178C) ->
; sprite_lazy_load: if BN4_SPRITE_CACHED(key) returns 0 and key is 0x1808 (Higsby) or
; 0x182C (Sal), decompress into BN4_SPRITE_CACHE via BIOS swi 0x11 (LZ77UnCompWram) using
; the exact layout of BN4_SPRITE_PRELOAD (0x08002B48..0x08002B88), and return buf + 4.
; ============================================================

.org 0x08002894                     ; bl 0x08002ACA (BN4_SPRITE_CACHED), RS == BM
    bl      VENEER_SPRITE_LAZY

.org VENEER_SPRITE_LAZY
.align 4
    ldr     r3, =sprite_lazy_load | 1
    bx      r3
    .pool

.org FS_SPRITE_LAZY
.align 4
sprite_lazy_load:
    push    r4-r7, r14
    mov     r4, r0                  ; save key = (list_ofs << 8) | number
    ldr     r3, =0x08002ACA | 1     ; BN4_SPRITE_CACHED (RS == BM)
    bl      @@call_r3
    cmp     r0, 0
    bne     @@done                  ; already cached -> return ptr
    ldr     r1, =0x1808             ; list[6] (ofs 0x18), Higsby (0x08)
    cmp     r4, r1
    beq     @@do_load
    ldr     r1, =0x182C             ; list[6] (ofs 0x18), Sal (0x2C)
    cmp     r4, r1
    bne     @@ret0
@@do_load:
    mov     r0, 0xFF
    and     r0, r4                  ; number (0x08 or 0x2C)
    lsl     r0, r0, 2
    ldr     r1, =SPRITE_LIST_6
    ldr     r0, [r1, r0]            ; ROM pointer with bit 31 = 1
    lsl     r0, r0, 1
    lsr     r0, r0, 1               ; r0 = LZ77 source pointer
    ldr     r2, [r0]
    lsr     r2, r2, 8               ; r2 = decompressed size
    ldr     r5, =0x0200F720         ; BN4_SPRITE_CACHE
    ldr     r1, [r5, 0x34]          ; r1 = next free byte in cache buffer
    add     r3, r1, r2
    ldr     r6, =0x02040000         ; overworld sprite cache buffer runs 0x02038800..0x02040000
    cmp     r3, r6
    bge     @@ret0
    ldrb    r6, [r5, 0]             ; cache entry count (max 8)
    cmp     r6, 8
    bge     @@ret0
    ; Register cache entry: keys at +4 (u16), ptrs at +20 (u32 = dst + 4)
    lsl     r7, r6, 1
    add     r7, 4
    strh    r4, [r5, r7]
    lsl     r7, r6, 2
    add     r7, 20
    add     r3, r1, 4               ; r3 = sprite ptr (past 4-byte size header)
    str     r3, [r5, r7]
    add     r6, 1
    strb    r6, [r5, 0]
    ; Decompress LZ77 into r1 and advance next_free_ptr (+0x34)
    push    r1, r2, r3
    swi     0x11                    ; LZ77UnCompWram(r0=src, r1=dst)
    pop     r1, r2, r3
    add     r1, r1, r2
    str     r1, [r5, 0x34]
    mov     r0, r3                  ; return buf + 4
    b       @@done
@@ret0:
    mov     r0, 0
@@done:
    pop     r4-r7, r15
@@call_r3:
    bx      r3
    .pool

.if . > FS_SPRITE_LAZY + FS_SPRITE_LAZY_SIZE
    .error "WP-D0 sprite lazy load: overruns its free-space slot"
.endif
