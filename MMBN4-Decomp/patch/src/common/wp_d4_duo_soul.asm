; ============================================================
; WP-D4: Duo Soul, the 13th Double Soul (owner, 2026-11-23: a MetalSoul recolour, granted by beating Duo).
; Framework (Gemini, decomp/notes/duo-soul-research.md): form 13 = DuoSoul already exists in the 16-entry
; form tables, pseudo-chip 0x16D has its name, flag 0x0023 and unite class 0x0D are free.
; Design: Gemini, decomp/notes/duo-soul-design.md, checked by Claude; what's built here:
;
; 1. Unite: BN4_SOUL_KINDS / BN4_SOUL_FLAGS (12 bytes each, packed between literals) move to free space
;    with a 13th entry (class 0x0D, flag 0x0023), and the unite loop starts at 12 instead of 11.
;    The per-battle "already used" mask at 0x0203BF88 is a zeroed 32-bit word, so bit 12 just works.
; 2. Look: BattleGetFormSprite (0x0800B90A) gives form 13 MetalSoul's sprite (list[0][9]); list[0] has
;    only 13 entries, so 13 would read past it. Same colours as MetalSoul for now: the Duo palette
;    needs the battle's OBJ palette load point, still to find (the design leaves it open).
; 3. Charge shot: BN4_POWER_ATTACKS[0x19] (form 13's charge, BN4_FORM_LOADOUTS[13] = FF 00 FF 19) gets
;    MetalFist's routine (0x0800CE28, action 0x14, which matches MetalSoul's sprite) at 180 damage
;    instead of 150. (The design's "0x0808 guard-break" word isn't what MetalFist uses; we keep its own.)
; 4. Passive: none yet. The design's "SuperArmor" helper 0x0800E1CA is really GutsSoul's +30 to the
;    hand's Null/break chips, so its passive proposal needs another look. BN4_FORM_ENTERS[13] stays a no-op.
; 5. Unite chips (class 0x0C, unused by any Soul, -> 0x0D): Guard1-3, Barrier, HolyPanl, AntiNavi,
;    AntiDmg, AntiRecv, LifeAura, Duo. GutsSoul's +30 also checks class 0x0C, so these ten lose it
;    (they're not attack chips, except the counters' own hit).
; 6. Icon: 0x16D's icon pointer, a blank placeholder today, takes MetalSoul's (0x169's) icon.
; The grant (flag 0x0023 + WriteChipKey(0x16D)) is in wp_d2_duo_story_grant.asm; FC 0C takes idx 12
; (wp_d4_grant_soul.asm); the status screen's 3rd page is still to do.
; ============================================================

DUO_SOUL_FLAG       equ 0x23
DUO_SOUL_CLASS      equ 0x0D
DUO_SOUL_CHIP       equ 0x16D
METAL_SOUL_CHIP     equ 0x169

; ---- 1. Unite tables -------------------------------------------------------------------------
.org 0x0801FFC6                     ; BN4_UNITE_CHECK: movs r3, #0xb -> #0xc (13 Souls, index 12..0)
    mov     r3, 12
.org 0x08020004                     ; literal: BN4_SOUL_KINDS
    .word   duo_soul_kinds
.org 0x08020014                     ; literal: BN4_SOUL_FLAGS
    .word   duo_soul_flags

; ---- 2. Form sprite thunk (0x0800B90A is halfword-aligned: ldr, bx, pad, literal) -------------
.org 0x0800B90A
    ldr     r3, [r15, 4]            ; pc = 0x0800B90C -> literal at 0x0800B910
    bx      r3
    .halfword 0
    .word   duo_form_sprite | 1

; ---- 3. Charge shot ---------------------------------------------------------------------------
.org 0x0800CAE0                     ; BN4_POWER_ATTACKS[0x19]
    .word   duo_charge_shot | 1

; ---- 5. Unite chips ---------------------------------------------------------------------------
.macro duo_unite, id
    chip_field_org id, 7
    .byte   DUO_SOUL_CLASS
.endmacro
duo_unite 0x058                     ; Guard1
duo_unite 0x059                     ; Guard2
duo_unite 0x05A                     ; Guard3
duo_unite 0x087                     ; Barrier
duo_unite 0x08B                     ; HolyPanl
duo_unite 0x090                     ; AntiNavi
duo_unite 0x091                     ; AntiDmg
duo_unite 0x093                     ; AntiRecv
duo_unite 0x0C9                     ; LifeAura
duo_unite CHIP_DUO                  ; Duo

; ---- 6. Icon ----------------------------------------------------------------------------------
chip_field_org DUO_SOUL_CHIP, 0x20
    .incbin ROM_IN, CHIP_TABLE + METAL_SOUL_CHIP * CHIP_ENTRY_SIZE + 0x20 - 0x08000000, 4

; ---- Free space -------------------------------------------------------------------------------
.org FS_DUO_SOUL
duo_soul_kinds:
    .incbin ROM_IN, 0x08020008 - 0x08000000, 12
    .byte   DUO_SOUL_CLASS
duo_soul_flags:
    .byte   0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C    ; Roll, Guts, Wind, Search, Fire, Thunder
    .byte   0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22    ; Proto, Number, Metal, Junk, Aqua, Wood
    .byte   DUO_SOUL_FLAG                         ; Duo

.align 2
duo_form_sprite:                    ; BattleGetFormSprite: r0 list, r1 form -> r1, r2 (same as the original)
    add     r3, r0, r0
    ldr     r2, =0x08017F98         ; BN4_FORM_SPRITES (the original's literal; same in RS and BM)
    add     r2, r2, r3
    mov     r3, 0
    cmp     r0, 0
    bne     @@go
    cmp     r1, 0x0F
    beq     @@go
    mov     r3, r1
    cmp     r3, 13
    bne     @@go
    mov     r3, 9                   ; DuoSoul -> MetalSoul's sprite
@@go:
    ldrb    r1, [r2]
    ldrb    r2, [r2, 1]
    add     r2, r2, r3
    bx      r14

.align 2
duo_charge_shot:                    ; MetalFist (0x0800CE28) at 180 damage
    mov     r0, 0
    strb    r0, [r7, 2]
    strb    r0, [r7, 3]
    strb    r0, [r7, 4]
    strb    r0, [r7, 5]
    strh    r0, [r7, 6]
    mov     r0, 0xA8                ; sound
    strh    r0, [r7, 0x0A]
    mov     r0, 180                 ; damage (MetalFist 150)
    strh    r0, [r7, 8]
    ldr     r0, =0x06050601         ; MetalFist's own word
    str     r0, [r7, 0x0C]
    mov     r0, 0x14                ; action: fist slam
    bx      r14
    .pool

.if . > FS_DUO_SOUL + FS_DUO_SOUL_SIZE
    .error "WP-D4 Duo Soul: overruns its free-space slot"
.endif
