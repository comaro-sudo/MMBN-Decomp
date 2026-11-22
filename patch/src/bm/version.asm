; Mega Man Battle Network 4: Blue Moon (USA) — B4BE
; SHA-1 5e017c803ddc768efb8010314b46bc17e9757f71
VER_BM          equ 1
VER_RS          equ 0

CHIP_TABLE      equ 0x080197EC      ; source: mmbn-chip-tables README; verified (identity build)
WOODMAN_FIX     equ 0x080B114E      ; source: BN4-WoodMan-Fix source/bmu.asm (used by WP-A1)
SOUL_FLAGS      equ 0x08020018      ; BN4_SOUL_FLAGS, 12 bytes (docs/re/save-and-flags.md §4); verified in both ROMs
