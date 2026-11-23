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
REWARD_ROW_PTR  equ 0x080F5F40      ; RewardGetRowPtr: r0 row -> &BN4_REWARD_ROWS[row] (RE-06); 12 bytes + literal
REWARD_ROWS     equ 0x080F5F50      ; BN4_REWARD_ROWS, 276 x 40 bytes
RANDOM_ROW_SPACE equ 0x08037D30     ; second dead 86-byte block after an unconditional b in code_03 (unreferenced)
; Status screen, two Soul pages (wp_d4_status_two_pages.asm; docs/handoff-violet-eclipse.md)
STATUS_STUB_CALL  equ 0x08033674
STATUS_SPAWN_CALL equ 0x08034A64
STATUS_LIST_PTR   equ 0x08034A6C
VENEER_TOGGLE     equ 0x08034A78
UI_SPAWN          equ 0x08053F8C
ICON_FLAG_READ    equ 0x0810D9FA
ICON_FLAGS_PTR    equ 0x0810DA78
ICON_OAM_PTR      equ 0x0810DA7C
ICON_ANIMS_PTR    equ 0x0810DA88
ICON_PALS_PTR     equ 0x0810DA8C
ICON_FREE         equ 0x0810DAB8
; NPC lists (wp_d0_npc_lists.asm; decomp/notes/lobby-npcs.md)
NPC_SLOT_ACDC     equ 0x08054434
NPC_SLOT_HIGSBY   equ 0x0805444C
NPC_LIST_ACDC     equ 0x08054F58
NPC_LIST_HIGSBY   equ 0x08056550
NPC_COUNT_ACDC    equ 13
NPC_COUNT_HIGSBY  equ 4
