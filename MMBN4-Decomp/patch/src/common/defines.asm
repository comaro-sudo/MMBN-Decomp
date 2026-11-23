; Version-agnostic constants.
; Version-specific addresses live in patch/src/bm/version.asm and patch/src/rs/version.asm.

; Chip table: entries of 0x20 data bytes + 0x0C (icon/image/palette pointers)
CHIP_ENTRY_SIZE     equ 0x2C
CHIP_OFS_DAMAGE     equ 0x1A        ; halfword

; Chip IDs (see ext/mmbn-chip-tables/bn4b_chip_table.asm)
CHIP_CANNON         equ 0x0001
CHIP_HICANNON       equ 0x0002
CHIP_MCANNON        equ 0x0003

; Address of a field of a chip entry
.macro chip_field_org, id, field
    .org CHIP_TABLE + (id) * CHIP_ENTRY_SIZE + (field)
.endmacro
