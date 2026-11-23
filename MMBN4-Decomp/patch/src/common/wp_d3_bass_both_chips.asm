; ============================================================
; WP-D3 / Violet Eclipse: a Bass battle gives BOTH versions' chips (owner decision 2026-11-23, the "Main" option
; in docs/completion/bosses.md section 3). Bass V1 drops Bass X (RS) or BassAnly X (BM); BassOmega and BassXX
; drop BlakBarr A (RS) or DeltaRay Z (BM). With this patch, whichever of the four the battle gives, its pair
; from the other version is granted too.
;
; How (decomp/notes/bass-rematch.md, Gemini, section 3):
;   The results screen grants a chip reward with  push {r0} / bl CHIP_GIVE / pop {r0}  (CHIP_GIVE(id, code, 1)
;   adds it to the pack, sets its Library flag and writes its chip key). The bl is redirected to BassPairGive,
;   which does the original grant and then, for the four Bass chips, grants the pair as well.
;   BassPairGive lives in a dead 86-byte block of code_03 (unreachable code after an unconditional b in
;   sub_08037BE4; no branch or pointer into it in either ROM), which is within bl range of both ends.
; The results screen still names only the battle's own chip; the pair is added silently.
; ============================================================

.org BASS_PAIR_HOOK                 ; the bl CHIP_GIVE between push {r0} and pop {r0}
    bl      BassPairGive

.org BASS_PAIR_SPACE
BassPairGive:                       ; r0 chip id, r1 code, r2 quantity (as for CHIP_GIVE)
    push    r4, r5, r14
    mov     r4, r0
    bl      CHIP_GIVE               ; the original grant
    add     r5, =@@pairs
    mov     r3, 4
@@next:
    ldrh    r0, [r5]
    cmp     r0, r4
    beq     @@found
    add     r5, 4
    sub     r3, 1
    bne     @@next
    pop     r4, r5, r15
@@found:
    ldrh    r1, [r5, 2]             ; the pair, packed like a reward word: id | code << 9
    lsl     r0, r1, 23
    lsr     r0, r0, 23              ; id
    lsr     r1, r1, 9               ; code
    mov     r2, 1
    bl      CHIP_GIVE
    pop     r4, r5, r15
.align 4
@@pairs:                            ; chip, its other-version pair (id | code << 9); codes A=0, X=23, Z=25
    .halfword CHIP_BASS,     CHIP_BASSANLY | (23 << 9)
    .halfword CHIP_BASSANLY, CHIP_BASS     | (23 << 9)
    .halfword CHIP_BLAKBARR, CHIP_DELTARAY | (25 << 9)
    .halfword CHIP_DELTARAY, CHIP_BLAKBARR | (0 << 9)
.if . > BASS_PAIR_SPACE + 86
    .error "WP-D3 Bass pair: helper overruns the dead 86-byte block"
.endif
