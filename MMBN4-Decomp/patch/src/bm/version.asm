; Mega Man Battle Network 4: Blue Moon (USA) — B4BE
; SHA-1 5e017c803ddc768efb8010314b46bc17e9757f71
VER_BM          equ 1
VER_RS          equ 0

CHIP_TABLE      equ 0x080197EC      ; source: mmbn-chip-tables README; verified (identity build)
WOODMAN_FIX     equ 0x080B114E      ; source: BN4-WoodMan-Fix source/bmu.asm (used by WP-A1)
SOUL_FLAGS      equ 0x08020018      ; BN4_SOUL_FLAGS, 12 bytes (docs/re/save-and-flags.md §4); verified in both ROMs
REWARD_DECODE   equ 0x08021A9C      ; battle reward word -> r0 kind, r1 amount (docs/re/save-and-flags.md §4b); 0x38 bytes + literal
BASS_PAIR_HOOK  equ 0x080216E6      ; results screen: the bl CHIP_GIVE between push {r0} / pop {r0} (decomp/notes/bass-rematch.md 3)
BASS_PAIR_SPACE equ 0x08037C60      ; dead 86-byte block after an unconditional b in sub_08037BE4 (code_03; unreferenced)
REWARD_ROW_PTR  equ 0x080F5F4C      ; RewardGetRowPtr: r0 row -> &BN4_REWARD_ROWS[row] (RE-06); 12 bytes + literal
REWARD_ROWS     equ 0x080F5F5C      ; BN4_REWARD_ROWS, 276 x 40 bytes
RANDOM_ROW_SPACE equ 0x08037D38     ; second dead 86-byte block after an unconditional b in code_03 (unreferenced)
; Status screen, two Soul pages (wp_d4_status_two_pages.asm; docs/handoff-violet-eclipse.md)
STATUS_STUB_CALL  equ 0x08033678
STATUS_SPAWN_CALL equ 0x08034A68
STATUS_LIST_PTR   equ 0x08034A70
VENEER_TOGGLE     equ 0x08034A7C
UI_SPAWN          equ 0x08053F98
ICON_FLAG_READ    equ 0x0810DA0E
ICON_FLAGS_PTR    equ 0x0810DA8C
ICON_OAM_PTR      equ 0x0810DA90
ICON_ANIMS_PTR    equ 0x0810DA9C
ICON_PALS_PTR     equ 0x0810DAA0
ICON_FREE         equ 0x0810DACC
; NPC lists (wp_d0_npc_lists.asm; decomp/notes/lobby-npcs.md)
NPC_SLOT_ACDC     equ 0x08054440
NPC_SLOT_HIGSBY   equ 0x08054458
NPC_LIST_ACDC     equ 0x08054F64
NPC_LIST_HIGSBY   equ 0x0805655C
NPC_COUNT_ACDC    equ 13
NPC_COUNT_HIGSBY  equ 4
; Undernet 1 (94, 00) Z-Saver Blue Mystery Data (wp_d2_zsaver_md.asm)
MD_SLOT_UNDER1      equ 0x080AC6DC
MD_LIST_UNDER1      equ 0x080AC700
MD_COUNT_UNDER1     equ 4
CW_NPC_SLOT_UNDER1  equ 0x08082A0C
CW_NPC_LIST_UNDER1  equ 0x080834C0
CW_NPC_COUNT_UNDER1 equ 6
