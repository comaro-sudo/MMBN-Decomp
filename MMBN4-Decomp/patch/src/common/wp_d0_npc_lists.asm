; ============================================================
; WP-D0 (infrastructure): Higsby's and ACDC Town's NPC lists move to free space, with room to grow.
;
; MapSpawnNPCs (0x0802A3D2) looks a map's NPC list up through a per-group table of per-map pointers;
; NpcSpawnList (0x08026748) spawns each pointer until 0xFF. Both lists are tightly packed in the ROM, so
; adding NPCs (the lobby operators, JunkMan's trash can: docs/pending/free-tournament-secret-chips.md) means
; relocating them (decomp/notes/lobby-npcs.md). The original entries are copied from the player's own ROM
; at build time (.incbin ROM_IN), so the pointers are right for each version and no ROM data is committed.
; BN4 allows 16 NPCs per map: Higsby's 4 + 9 operators = 13; ACDC Town 13 + 2 operators + trash can = 16.
;
; New NPCs are added between the copied entries and the terminator below, by the packages that need them.
; With nothing added, the game behaves exactly as before.
; ============================================================

.org NPC_SLOT_HIGSBY                ; (00, 06) Higsby's
    .word FS_NPC_HIGSBY
.org NPC_SLOT_ACDC                  ; (00, 00) ACDC Town
    .word FS_NPC_ACDC

.org FS_NPC_HIGSBY
    .incbin ROM_IN, NPC_LIST_HIGSBY - 0x08000000, NPC_COUNT_HIGSBY * 4
    .word lobby_npc_chaud
    .word lobby_npc_mayl
    .word lobby_npc_dex
    .word lobby_npc_match
    .word lobby_npc_shuko
    .word lobby_npc_lily
    .word lobby_npc_tamako
    .word lobby_npc_sal
    .word lobby_npc_number
    .word freq_tuner_npc
    .word 0xFF
.if . > FS_NPC_HIGSBY + FS_NPC_LIST_SIZE
    .error "WP-D0: Higsby's NPC list overruns its slot"
.endif

.org FS_NPC_ACDC
    .incbin ROM_IN, NPC_LIST_ACDC - 0x08000000, NPC_COUNT_ACDC * 4
    .word lobby_npc_raika
    .word lobby_npc_raoul
    .word lobby_npc_trashcan
    .word 0xFF
.if . > FS_NPC_ACDC + FS_NPC_LIST_SIZE
    .error "WP-D0: ACDC Town's NPC list overruns its slot"
.endif
