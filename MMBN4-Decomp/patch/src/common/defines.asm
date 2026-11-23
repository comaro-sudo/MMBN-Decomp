; Version-agnostic constants.
; Version-specific addresses live in patch/src/bm/version.asm and patch/src/rs/version.asm.

; Chip table: entries of 0x20 data bytes + 0x0C (icon/image/palette pointers)
CHIP_ENTRY_SIZE     equ 0x2C
CHIP_OFS_DAMAGE     equ 0x1A        ; halfword

; Chip IDs (see ext/mmbn-chip-tables/bn4b_chip_table.asm)
CHIP_CANNON         equ 0x0001
CHIP_HICANNON       equ 0x0002
CHIP_MCANNON        equ 0x0003

; The four Bass Giga chips (decomp/notes/bass-rematch.md, Gemini)
CHIP_BASS           equ 301
CHIP_DELTARAY       equ 302
CHIP_BASSANLY       equ 306
CHIP_BLAKBARR       equ 308

; Give a chip: r0 id, r1 code, r2 quantity (pack + Library flag + chip key). code_01, same address in RS and BM
CHIP_GIVE           equ 0x0801913E

; Address of a field of a chip entry
.macro chip_field_org, id, field
    .org CHIP_TABLE + (id) * CHIP_ENTRY_SIZE + (field)
.endmacro
