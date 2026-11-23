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
; Undernet 1 (94, 00) Z-Saver Blue Mystery Data (wp_d2_zsaver_md.asm)
MD_SLOT_UNDER1      equ 0x080AC6D0
MD_LIST_UNDER1      equ 0x080AC6F4
MD_COUNT_UNDER1     equ 4
CW_NPC_SLOT_UNDER1  equ 0x08082A00
CW_NPC_LIST_UNDER1  equ 0x080834B4
CW_NPC_COUNT_UNDER1 equ 6
; Deterministic story tournaments (wp_b1_tournaments.asm)
TOURN_SOUL_PAIRS    equ 0x08044050
TOURN_PAIR_RNG      equ 0x08044020
TOURN_PAIR_NO_SWAP  equ 0x08044032
TOURN_PAIR_SWAP     equ 0x0804480C
TOURN_BRACKET_RNG   equ 0x0804432E
TOURN_BRACKET_DET   equ 0x0804435E
RS_TUTORIAL_SOUL    equ 0x08059EB6
; Free Tournament 36-secret-chip fix (wp_d4_free_tournament.asm)
TEST_EVENT_FLAG         equ 0x080250A0
FREE_TOURN_SCAN         equ 0x080448DA
FREE_TOURN_SHUFFLE      equ 0x080448F2
FREE_TOURN_GET_SOUL     equ 0x08044B40
FREE_TOURN_PLAYER_CASE0 equ 0x08044F8E
FREE_TOURN_SPAWN_MEGA   equ 0x08044FB6
FREE_TOURN_GET_TIER     equ 0x08045068
FREE_SECRET_CHIP_BASE   equ 0x08045528
; Lobby Rematches + JunkMan Trash Can (wp_d3_lobby_rematches.asm)
SET_EVENT_FLAG          equ 0x0802504C
CLEAR_EVENT_FLAG        equ 0x08025068
OPEN_MAP_DIALOGUE       equ 0x0804E0A0
OPEN_CUSTOM_DIALOGUE    equ 0x0804E3B4
CLOSE_DIALOGUE          equ 0x0804E660
SCRIPT_FC_TBL_PTR       equ 0x08050430
SCRIPT_FC_TBL_ORIG      equ 0x08050434
RESULTS_DONE_HOOK       equ 0x0802174C
LOBBY_RESULTS_VENEER    equ 0x08037CA0
NPC_IDLE_TALK_LOOP      equ 0x080A8EE8
WARP_CLEANUP_1          equ 0x080A6D62
WARP_CLEANUP_2          equ 0x080A6CDA
NPC_SLOT_WTRGOD1        equ 0x08075EF8
NPC_LIST_WTRGOD1        equ 0x08076518
NPC_COUNT_WTRGOD1       equ 1

DUO_DEFEAT_STATE        equ 0x080C4780      ; sub_080C4780: Duo defeated (sets flag 0x006F); WP-D2 Duo story grant
NPC_SLOT_METEOR1        equ 0x0806DA1C      ; Meteor Comp 1 (82,00) CW NPC list pointer (Gemini, duo-rematch-research.md)
NPC_LIST_METEOR1        equ 0x0806DCCC
NPC_COUNT_METEOR1       equ 3
MD_SLOT_ACDC2           equ 0x080AA434      ; ACDC Area 2 (90,01) Mystery Data list pointer (special-chip-md-design.md)
MD_LIST_ACDC2           equ 0x080AA6B4
MD_COUNT_ACDC2          equ 3
CW_NPC_SLOT_ACDC2       equ 0x0807748C      ; ACDC Area 2 (90,01) CW NPC/object list pointer
CW_NPC_LIST_ACDC2       equ 0x080780F4
CW_NPC_COUNT_ACDC2      equ 13
