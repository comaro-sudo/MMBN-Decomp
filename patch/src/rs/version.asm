; Mega Man Battle Network 4: Red Sun (USA) — B4WE
; SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1
VER_BM          equ 0
VER_RS          equ 1

CHIP_TABLE      equ 0x080197EC      ; source: mmbn-chip-tables README; verified (identity build)
WOODMAN_FIX     equ 0x080B1142      ; source: BN4-WoodMan-Fix source/rsu.asm (used by WP-A1)
SOUL_FLAGS      equ 0x08020018      ; BN4_SOUL_FLAGS, 12 bytes (docs/re/save-and-flags.md §4); verified in both ROMs
