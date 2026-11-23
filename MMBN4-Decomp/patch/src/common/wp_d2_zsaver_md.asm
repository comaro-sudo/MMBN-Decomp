; ============================================================
; WP-D2: Z-Saver (0x0DA, code Z) Blue Mystery Data in Undernet 1 (94, 00).
; Design: decomp/notes/special-chip-md-design.md & placements.md (owner decision 2026-11-23).
;
; Undernet 1's Mystery Data list (4 entries) and CW NPC/object list (6 entries) are both tightly
; packed in ROM, so both move to free space (copied from the player's own ROM via .incbin).
; Appends one Blue Mystery Data (type 1) at (-282, 56, 0) — the hidden red dead-end spur tucked
; under the western yellow bridge — using free flag 0x0E04 (in Undernet 1's 0x0E00-0x0E07 block).
; Both content slots hold Z-Saver Z so New Game+ advancing never changes the reward.
; Generic Mystery Data pickup script 0 (0x087585A0 RS / 0x08758030 BM) handles chips automatically.
; ============================================================

MD_FLAG_ZSAVER  equ 0x0E04          ; free flag in Undernet 1's 0x0E00-0x0E07 block
CHIP_ZSAVER     equ 0x00DA          ; Z-Saver (Mega, ROM-backed, flags=0x42)
CODE_Z          equ 25

; ---- Repoint Undernet 1 (94, 00) Mystery Data list & CW NPC/object list ----------------------
.org MD_SLOT_UNDER1
    .word FS_MD_UNDER1_LIST
.org CW_NPC_SLOT_UNDER1
    .word FS_CW_NPC_UNDER1

; ---- Relocated Mystery Data list + Z-Saver BMD entry -----------------------------------------
.org FS_MD_UNDER1_LIST
    .incbin ROM_IN, MD_LIST_UNDER1 - 0x08000000, MD_COUNT_UNDER1 * 12
    .byte 1, 0                      ; type = 1 (Blue Mystery Data), obj_flags = 0
    .halfword MD_FLAG_ZSAVER        ; taken flag 0x0E04
    .word FS_MD_ZSAVER_PLACE
    .word FS_MD_ZSAVER_CONT
    .word 0                         ; list terminator

FS_MD_ZSAVER_PLACE:
    .byte 1, 0x20                   ; valid = 1, weight = 0x20
    .halfword -282, 56, 0           ; Undernet 1 (-282, 56, 0)
    .word 0                         ; placement terminator

FS_MD_ZSAVER_CONT:
    .byte 1, 0, 0xFF, CODE_Z        ; kind = 1 (chip), code = Z (25)
    .word CHIP_ZSAVER
    .byte 1, 0, 0xFF, CODE_Z        ; 2nd+ pickup (NG+ clamps here: always Z-Saver Z)
    .word CHIP_ZSAVER
    .word 0                         ; content terminator

; ---- Relocated CW NPC/object list + 11-byte MapObj_SpawnMysteryData script -------------------
.org FS_CW_NPC_UNDER1
    .incbin ROM_IN, CW_NPC_LIST_UNDER1 - 0x08000000, CW_NPC_COUNT_UNDER1 * 4
    .word FS_SPAWN_ZSAVER
    .word 0xFF                      ; NPC list terminator

FS_SPAWN_ZSAVER:
    .byte 0x08, 0x25, 0x08, 0x1C
    .byte 0x29, (MD_FLAG_ZSAVER & 0xFF), (MD_FLAG_ZSAVER >> 8)
    .byte 0x45, (MD_FLAG_ZSAVER & 0xFF), (MD_FLAG_ZSAVER >> 8)
    .byte 0x03

.if . > FS_ZSAVER_MD + FS_ZSAVER_MD_SIZE
    .error "WP-D2 Z-Saver MD: overruns its free-space slot"
.endif
