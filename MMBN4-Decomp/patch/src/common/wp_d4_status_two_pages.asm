; ============================================================
; WP-D4 step 3: the status screen shows all 13 Double Souls (12 + Duo), as pages of 6: R next page, L previous.
; Page 3 (Duo) appears only when a 13th Soul is owned (2026-11-23, with wp_d4_duo_soul.asm).
; A 2 x 6 grid doesn't fit (the NAVIGATOR box starts at Y = 94; decomp/notes/status-screen-2x6.md).
; Design: Gemini, decomp/notes/status-screen-two-pages.md. Corrected call site and veneer spots:
; docs/handoff-violet-eclipse.md ("Status-screen two-page patch").
;
; - The two 6-entry Soul lists (status screen and icon) and the icon animation/palette lists become 12-entry
;   lists in free space; their four literals are repointed. The old lists' bytes are then free.
; - StatusSoulIconInit reads buf[slot + page] instead of buf[slot], via StatusSoulLoadFlag, placed in the
;   freed icon-list bytes in code_10 (within bl range). Slots 0-5 keep their tiles, OAM and positions.
; - StatusCollectOwnedSouls resets the page to 0 when the screen opens (InitPage, via a veneer in the old
;   completion-star list at 0x08000DB4, unused since wp_d4_star_twelve_souls.asm).
; - The empty per-frame stub's call goes to StatusSoulPageToggle (via a veneer in the freed status-list
;   bytes): on L or R, when 7+ Souls are owned, change the page (0 / 6 / 12), play a cursor sound and reset the
;   six Soul icon objects so they redraw on the same frame.
; The page byte is 0x0201D018, an unused byte of the screen's 32-byte Soul buffer.
; ============================================================

SOUL_BUF        equ 0x0201D000      ; owned Soul flags, 0xFF-filled (32 bytes); +0x18 = page offset (0, 6 or 12)
UI_POOL         equ 0x02008BB0      ; 24 UI objects x 0x58: +0 active (bit 0), +1 type, +8 state
UI_TYPE_SOUL    equ 0x45
SFX_PAGE        equ 0x6E

; ---- the lists -------------------------------------------------------------------------------
.org STATUS_LIST_PTR                ; StatusCollectOwnedSouls: owned-Soul list
    .word FS_STATUS_SOULS
.org ICON_FLAGS_PTR                 ; StatusSoulIconInit: Soul flag -> icon index
    .word FS_STATUS_SOULS
.org ICON_OAM_PTR                   ; StatusSoulIconInit: 6-byte OAM slot offsets (moved before ICON_FREE reuse)
    .word FS_STATUS_OAM
.org ICON_ANIMS_PTR
    .word FS_STATUS_SOULS + 0x10
.org ICON_PALS_PTR
    .word FS_STATUS_SOULS + 0x20

.org FS_STATUS_SOULS
    .byte 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0xFF  ; Roll..Wood, Duo, end
.org FS_STATUS_SOULS + 0x10
    .byte 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x08  ; icon animations (Duo: MetalSoul's)
.org FS_STATUS_SOULS + 0x20
    .byte 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x02, 0x01, 0x02  ; icon palettes (Duo: MetalSoul's)
.org FS_STATUS_OAM
    .byte 0x00, 0x10, 0x20, 0x30, 0x40, 0x50                                            ; OAM slot offsets (slots 0-5)

; ---- the buffer is filled with 0xFF to the end -----------------------------------------------
; StatusCollectOwnedSouls starts with ByteFill(SOUL_BUF, 8, 0xFF): enough for the vanilla six slots, but now the
; buffer is read up to index 12. The bytes past 8 were whatever the scratch area held (zeros), so "is there a
; 7th / 13th Soul?" (buf[6] / buf[12] != 0xFF) was true with 9 or 11 Souls and page 3 showed up empty.
; Fill all 32 bytes: the `movs r1, 8` of that ByteFill call (in sub_080351A8, only the status screen calls it).
.if STATUS_LIST_PTR == 0x08034A6C
    .org 0x080351AC                 ; RS
.else
    .org 0x080351B4                 ; BM (the function sits 8 bytes later)
.endif
    .halfword 0x2120                ; movs r1, 0x20

; ---- the icon reads buf[slot + page] ---------------------------------------------------------
.org ICON_FLAG_READ                 ; movs r2,1 / ldrb r6,[r5,4] / ldr r1,=SOUL_BUF / ldrb r4,[r1,r6]
    bl      StatusSoulLoadFlag
    mov     r8, r8
    mov     r8, r8

.org ICON_FREE                      ; the old icon lists' bytes (unreferenced now)
StatusSoulLoadFlag:                 ; out: r6 slot (0-5), r1 SOUL_BUF, r4 flag, r2 = 1 (as the original)
    ldrb    r6, [r5, 4]
    ldr     r1, =SOUL_BUF
    ldrb    r2, [r1, 0x18]
    add     r2, r2, r6
    ldrb    r4, [r1, r2]
    mov     r2, 1
    bx      r14
    .pool
.if . > ICON_FREE + 28
    .error "WP-D4 status: StatusSoulLoadFlag overruns the freed icon lists"
.endif

; ---- page reset when the screen opens --------------------------------------------------------
.org STATUS_SPAWN_CALL              ; bl UI_SPAWN (spawns the six Soul icon objects)
    bl      VENEER_INIT_PAGE
.org VENEER_INIT_PAGE               ; 0x08000DB4: the old completion-star list
    ldr     r3, [r15, 0]
    bx      r3
    .word   StatusSoulInitPage | 1

; ---- page toggle, every idle frame -----------------------------------------------------------
.org STATUS_STUB_CALL               ; bl to the empty per-frame stub
    bl      VENEER_TOGGLE
.org VENEER_TOGGLE                  ; the old status list's 8 bytes
    ldr     r3, [r15, 0]
    bx      r3
    .word   StatusSoulPageToggle | 1

.org FS_STATUS_CODE
StatusSoulInitPage:                 ; r0 = the spawn list; lr = the caller's return
    ldr     r3, =SOUL_BUF
    mov     r2, 0
    strb    r2, [r3, 0x18]
    ldr     r3, =UI_SPAWN | 1
    bx      r3                      ; tail call: UI_SPAWN returns to our caller

StatusSoulPageToggle:
    push    r4-r7, r14
    ldr     r4, =SOUL_BUF
    ldrb    r0, [r4, 6]             ; a 7th owned Soul?
    cmp     r0, 0xFF
    beq     @@done
    mov     r0, r10                 ; sl
    ldr     r0, [r0, 4]             ; input
    ldrh    r2, [r0, 2]             ; newly pressed
    ldr     r1, =0x0300             ; L | R
    tst     r2, r1
    beq     @@done
    ldrb    r0, [r4, 0x18]          ; page offset 0 / 6 / 12
    lsl     r3, r2, 22              ; L (bit 9) -> sign bit
    bmi     @@left
    add     r0, 6                   ; R: next page, if it has a Soul; else back to page 1
    cmp     r0, 12
    bgt     @@first
    ldrb    r1, [r4, r0]
    cmp     r1, 0xFF
    bne     @@set
@@first:
    mov     r0, 0
    b       @@set
@@left:
    sub     r0, 6                   ; L: previous page; from page 1 to the last one with a Soul
    bge     @@set
    mov     r0, 12
    ldrb    r1, [r4, 12]
    cmp     r1, 0xFF
    bne     @@set
    mov     r0, 6
@@set:
    strb    r0, [r4, 0x18]
    mov     r0, SFX_PAGE
    ldr     r3, =PLAY_SFX | 1
    bl      @@call_r3
    ldr     r5, =UI_POOL
    mov     r6, 24
@@loop:
    ldrb    r0, [r5, 0]
    lsr     r0, r0, 1
    bcc     @@next                  ; inactive
    ldrb    r0, [r5, 1]
    cmp     r0, UI_TYPE_SOUL
    bne     @@next
    mov     r0, 0
    strb    r0, [r5, 8]             ; state 0: re-runs StatusSoulIconInit this frame
@@next:
    add     r5, 0x58
    sub     r6, 1
    bne     @@loop
@@done:
    pop     r4-r7, r15
@@call_r3:
    bx      r3
    .pool
.if . > FS_STATUS_CODE + FS_STATUS_CODE_SIZE
    .error "WP-D4 status: code overruns its free-space slot"
.endif
