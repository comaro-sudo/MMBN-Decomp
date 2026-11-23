; Mega Man Battle Network 4: Red Sun (USA) — B4WE
; SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1
VER_BM          equ 0
VER_RS          equ 1

CHIP_TABLE      equ 0x080197EC      ; source: mmbn-chip-tables README; verified (identity build)
WOODMAN_FIX     equ 0x080B1142      ; source: BN4-WoodMan-Fix source/rsu.asm (used by WP-A1)
SOUL_FLAGS      equ 0x08020018      ; BN4_SOUL_FLAGS, 12 bytes (docs/re/save-and-flags.md §4); verified in both ROMs
REWARD_DECODE   equ 0x08021A98      ; battle reward word -> r0 kind, r1 amount (docs/re/save-and-flags.md §4b); 0x38 bytes + literal
BASS_PAIR_HOOK  equ 0x080216E2      ; results screen: the bl CHIP_GIVE between push {r0} / pop {r0} (decomp/notes/bass-rematch.md 3)
BASS_PAIR_SPACE equ 0x08037C58      ; dead 86-byte block after an unconditional b in sub_08037BE4 (code_03; unreferenced)
