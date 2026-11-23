; ============================================================
; WP-D2: Grand Prix Power (0x138, code G) Blue Mystery Data in ACDC Area 2 (90, 01).
; Design: decomp/notes/special-chip-md-design.md & placements.md (owner decision 2026-11-23); the same
; recipe as wp_d2_zsaver_md.asm, which passed in-game.
;
; ACDC Area 2's Mystery Data list (3 entries) and CW NPC/object list (13 of 16) move to free space
; (copied from the player's own ROM via .incbin). Appends one Blue Mystery Data (type 1) at (106, 326, 0),
; the quiet round dead-end pad on the lower-left peninsula, with free flag 0x0D0B (in ACDC Area 2's
; 0x0D08-0x0D0F block). Both content slots hold GP Power G, so New Game+ never changes the reward.
; Its art comes from wp_d2_special_chip_art.asm (private builds); its name is still blank (chip-text task).
; ============================================================

MD_FLAG_GPP     equ 0x0D0B          ; free flag in ACDC Area 2's 0x0D08-0x0D0F block
CODE_G          equ 6

; ---- Repoint ACDC Area 2 (90, 01) Mystery Data list & CW NPC/object list ----------------------
.org MD_SLOT_ACDC2
    .word FS_MD_ACDC2_LIST
.org CW_NPC_SLOT_ACDC2
    .word FS_CW_NPC_ACDC2

; ---- Relocated Mystery Data list + GP Power BMD entry -----------------------------------------
.org FS_MD_ACDC2_LIST
    .incbin ROM_IN, MD_LIST_ACDC2 - 0x08000000, MD_COUNT_ACDC2 * 12
    .byte 1, 0                      ; type = 1 (Blue Mystery Data), obj_flags = 0
    .halfword MD_FLAG_GPP        ; taken flag 0x0D0B
    .word FS_MD_GPP_PLACE
    .word FS_MD_GPP_CONT
    .word 0                         ; list terminator

FS_MD_GPP_PLACE:
    .byte 1, 0x20                   ; valid = 1, weight = 0x20
    .halfword 106, 326, 0           ; ACDC Area 2 (106, 326, 0)
    .word 0                         ; placement terminator

FS_MD_GPP_CONT:
    .byte 1, 0, 0xFF, CODE_G        ; kind = 1 (chip), code = G (6)
    .word CHIP_GPP
    .byte 1, 0, 0xFF, CODE_G        ; 2nd+ pickup (NG+ clamps here: always GP Power G)
    .word CHIP_GPP
    .word 0                         ; content terminator

; ---- Relocated CW NPC/object list + 11-byte MapObj_SpawnMysteryData script -------------------
.org FS_CW_NPC_ACDC2
    .incbin ROM_IN, CW_NPC_LIST_ACDC2 - 0x08000000, CW_NPC_COUNT_ACDC2 * 4
    .word FS_SPAWN_GPP
    .word 0xFF                      ; NPC list terminator

FS_SPAWN_GPP:
    .byte 0x08, 0x25, 0x08, 0x1C
    .byte 0x29, (MD_FLAG_GPP & 0xFF), (MD_FLAG_GPP >> 8)
    .byte 0x45, (MD_FLAG_GPP & 0xFF), (MD_FLAG_GPP >> 8)
    .byte 0x03

.if . > FS_GPP_MD + FS_GPP_MD_SIZE
    .error "WP-D2 GP Power MD: overruns its free-space slot"
.endif
