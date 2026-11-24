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
; (wp_d4_grant_soul.asm); the status screen's 3rd page is in wp_d4_status_two_pages.asm.
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

; ---- 4. Passive (Option A, duo-soul-passive.md): AirShoes + FloatShoes + B+Left Reflect -------
.org 0x0800E0D4                     ; BN4_FORM_ENTERS[13] (RS == BM)
    .word   duo_soul_enter | 1

; ---- 4b. Duo palette hook (form-palette-hook.md): override [sprite_hdr+0x34] for form 13 ------
.org 0x080107E2                     ; bl sub_08002DEC (f2 f7 03 fb) inside BattleMegaManLoadFormSprite (RS == BM)
    bl      VENEER_DUO_PAL
.org VENEER_DUO_PAL
    ldr     r3, [r15, 0]
    bx      r3
    .word   duo_form_palette_hook | 1

; ---- 4c. Emotion window face fix (duo-soul-transform.md): clamp OOB face in BN4_FACE_GET ------
.org 0x08016A20                     ; BN4_FACE_GET (RS == BM, word-aligned)
    ldr     r3, [r15, 0]
    bx      r3
    .word   duo_face_get | 1

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

; ---- Free space (extension: face fix, palette hook + 3x32B palettes, passive) -----------------
.org FS_DUO_EXT
.align 2
duo_face_get:
    push    r14
    bl      @@run_orig
    .if VER_BM
    cmp     r0, 11                  ; BM DuoSoul (13+4-6=11) -> MetalSoul face (7), keeps turns in r1
    bne     @@chk_oob
    mov     r0, 7
    b       @@done
    .endif
@@chk_oob:
    cmp     r0, 10                  ; unsigned: 0..10 valid; >=11 (or wrapped negative) -> clean face 0
    bls     @@done
    mov     r0, 0
    mov     r1, 0
@@done:
    pop     r15
@@run_orig:
    push    r4, r14                 ; displaced 0x08016A20..0x08016A27
    mov     r0, r10
    ldr     r0, [r0, 0x18]
    ldr     r0, [r0, 0x44]
    ldr     r3, =0x08016A28 | 1
    bx      r3

.align 2
duo_soul_enter:                     ; BN4_FORM_ENTERS[13]: AirShoes+FloatShoes (0x30) + Reflect Shield (0x6A)
    push    r14
    mov     r0, 0x30
    ldr     r3, =0x08012F82 | 1     ; BattleSetStatusBits (RS == BM)
    bl      @@call_r3
    ldr     r0, [r5, 0x58]
    mov     r1, 0x6A                ; B+Left Reflect Shield (auto-restored by sub_0800D8C2 on Soul end)
    strb    r1, [r0]
    pop     r15
@@call_r3:
    bx      r3

.align 2
duo_form_palette_hook:              ; replaces bl BN4_SPRITE_PALETTE_SET at 0x080107E8
    push    r4, r14
    ldr     r2, =0x08002DEC | 1     ; BN4_SPRITE_PALETTE_SET: returns sprite_hdr in r3
    bl      @@call_r2
    mov     r4, r3
    mov     r1, 0x24                ; active form id
    ldr     r2, =0x0800D7CA | 1     ; BN4_NAVI_BYTE (RS == BM)
    bl      @@call_r2
    mov     r1, 0
    cmp     r0, 13                  ; form 13 = DuoSoul?
    bne     @@store
    ldr     r1, =duo_soul_pal_set
@@store:
    str     r1, [r4, 0x34]          ; [sprite_hdr + 0x34] custom palette override
    pop     r4, r15
@@call_r2:
    bx      r2
    .pool

.align 4
duo_soul_pal_set:
    ; Palette 0: normal DuoSoul (duo-soul-design.md §1)
    .halfword 0x0020, 0x0000, 0x0000, 0x1043, 0x149F, 0x0C36, 0x0810, 0x7EB0
    .halfword 0x5CEB, 0x4488, 0x3045, 0x2024, 0x7F73, 0x037F, 0x48C9, 0x3466
    ; Palette 1: hit-flash (MetalSoul @ 0x0826D91C, RS == BM)
    .incbin ROM_IN, 0x0826D91C - 0x08000000, 0x20
    ; Palette 2: shadow/dark (dimmed DuoSoul)
    .halfword 0x0842, 0x0842, 0x0000, 0x0842, 0x0834, 0x0410, 0x0408, 0x4A30
    .halfword 0x3466, 0x2444, 0x1822, 0x1002, 0x4A90, 0x0258, 0x2865, 0x1C23

.if . > FS_DUO_EXT + FS_DUO_EXT_SIZE
    .error "WP-D4 Duo Soul ext: overruns its free-space slot"
.endif
