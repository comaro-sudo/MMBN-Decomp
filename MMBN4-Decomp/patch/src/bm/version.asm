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
; Deterministic story tournaments (wp_b1_tournaments.asm)
TOURN_SOUL_PAIRS    equ 0x08044058
TOURN_PAIR_RNG      equ 0x08044028
TOURN_PAIR_NO_SWAP  equ 0x0804403A
TOURN_PAIR_SWAP     equ 0x08044814
TOURN_BRACKET_RNG   equ 0x08044336
TOURN_BRACKET_DET   equ 0x08044366
; Free Tournament 36-secret-chip fix (wp_d4_free_tournament.asm)
TEST_EVENT_FLAG         equ 0x080250A4
FREE_TOURN_SCAN         equ 0x080448E2
FREE_TOURN_SHUFFLE      equ 0x080448FA
FREE_TOURN_GET_SOUL     equ 0x08044B48
FREE_TOURN_PLAYER_CASE0 equ 0x08044F96
FREE_TOURN_SPAWN_MEGA   equ 0x08044FBE
FREE_TOURN_GET_TIER     equ 0x08045070
FREE_SECRET_CHIP_BASE   equ 0x08045530
; Lobby Rematches + JunkMan Trash Can (wp_d3_lobby_rematches.asm)
SET_EVENT_FLAG          equ 0x08025050
CLEAR_EVENT_FLAG        equ 0x0802506C
OPEN_MAP_DIALOGUE       equ 0x0804E0A8
OPEN_CUSTOM_DIALOGUE    equ 0x0804E3BC
CLOSE_DIALOGUE          equ 0x0804E668
SCRIPT_FC_TBL_PTR       equ 0x0805043C
SCRIPT_FC_TBL_ORIG      equ 0x08050440
RESULTS_DONE_HOOK       equ 0x08021750
LOBBY_RESULTS_VENEER    equ 0x08037CA4
NPC_IDLE_TALK_LOOP      equ 0x080A8EF4
WARP_CLEANUP_1          equ 0x080A6D6E
WARP_CLEANUP_2          equ 0x080A6CE6
NPC_SLOT_WTRGOD1        equ 0x08075F04
NPC_LIST_WTRGOD1        equ 0x08076524
NPC_COUNT_WTRGOD1       equ 1

DUO_DEFEAT_STATE        equ 0x080C478C      ; sub_080C4780: Duo defeated (sets flag 0x006F); WP-D2 Duo story grant
NPC_SLOT_METEOR1        equ 0x0806DA28      ; Meteor Comp 1 (82,00) CW NPC list pointer (Gemini, duo-rematch-research.md)
NPC_LIST_METEOR1        equ 0x0806DCD8
NPC_COUNT_METEOR1       equ 3
MD_SLOT_ACDC2           equ 0x080AA440      ; ACDC Area 2 (90,01) Mystery Data list pointer (special-chip-md-design.md)
MD_LIST_ACDC2           equ 0x080AA6C0
MD_COUNT_ACDC2          equ 3
CW_NPC_SLOT_ACDC2       equ 0x08077498      ; ACDC Area 2 (90,01) CW NPC/object list pointer
CW_NPC_LIST_ACDC2       equ 0x08078100
CW_NPC_COUNT_ACDC2      equ 13
GMD_CONTENT_TABLE       equ 0x080ADF68      ; MysteryDataGetContentTable (re07-mystery-data.md); WP-B4
CHIP_NAME_EREADER_MIN   equ 0x0802F1C0      ; sub_0802F154's literal 0x137 (patch-test-results-4.md)
CHIP_DESC_EREADER_MIN   equ 0x08106C34      ; sub_08106BF0's literal 0x137
VENEER_BATTLE_EXIT      equ 0x080C4794      ; DUO_DEFEAT_STATE + 8 (dead bytes after the thunk); 1st of 2 veneers
