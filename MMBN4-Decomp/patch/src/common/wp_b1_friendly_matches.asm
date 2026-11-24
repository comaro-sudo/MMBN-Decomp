; ============================================================
; WP-B1 / Violet Eclipse: 9 Friendly Tournament Matches (3 per tournament).
; Design: Gemini, decomp/notes/friendly-matches-design.md & friendly-matches-design-2.md.
;
; - 3 relocated RW venue NPC lists (identical vanilla counts in RS and BM):
;     T0 DenDome Hall (02,01):      6 -> 9/16  (Dex/GutsMan, Match/FireMan, Higsby/NumberMan)
;     T1 Castillo WaitRm (03,02):   5 -> 8/16  (Lily/WindMan, Tamako/MetalMan, Sal/WoodMan)
;     T2 Colosseum Lobby (04,06):   8 -> 11/16 (Raika/SearchMan, Raoul/ThunderMan, JunkMan signal)
; - Gated natively on tournament round (opcodes 0x42/0x43) and each Navi's Soul flag (0x04),
;   so zero extra event flags are consumed.
; - Talk indices 0xE0..0xE8 -> friendly_dialogue_archive (0 vanilla collisions).
; - FC 0E <enemy_id> <soul_idx>: launches plain battle at normal story tier (r5 = 0x5F, retryable
;   r4 = 3) with marker 0x020015EF = 0x40 | soul_idx; on victory, lobby_results_hook calls
;   friendly_grant_soul(soul_idx) to grant that Navi's Soul (0x0015 + 0x0017+idx + WriteChipKey).
; ============================================================

; ---- 1. Relocate the 3 tournament venue NPC lists ------------------------------------------
.org NPC_SLOT_DENDOME_HALL
    .word   friendly_npc_list_dendome
.org NPC_SLOT_CASTILLO_WAIT
    .word   friendly_npc_list_castillo
.org NPC_SLOT_COLOSSEUM_LOBBY
    .word   friendly_npc_list_colosseum

; ---- 2. Free space: FC 0E handler, Soul grant helper, NPC lists, NPCs, dialogue archive ----
.org FS_FRIENDLY

.align 2
friendly_fc_battle:                 ; FC 0E <enemy_id> <soul_idx>
    push    r4, r5, r6, r7, r14
    mov     r0, 1
    ldr     r3, =CLOSE_DIALOGUE | 1
    bl      lobby_call_r3
    ldrb    r0, [r5, 4]
    cmp     r0, 0
    bne     @@yield
    ldrb    r6, [r4, 2]             ; r6 = V1 enemy_id (0x84..0xAC)
    ldrb    r4, [r4, 3]             ; r4 = soul_idx (0..11)
    ldr     r0, =0x020015EF         ; bit 6 (0x40) = friendly-match marker
    mov     r1, 0x40
    orr     r1, r4
    strb    r1, [r0]
    ldr     r3, =INIT_BATTLE_REC | 1
    bl      lobby_call_r3
    push    r4, r5, r6
    mov     r0, 0
    mov     r1, 0x64
    mov     r2, 0x14
    mov     r3, 0x3F
    mov     r4, 3                   ; retryable (no Game Over on loss)
    mov     r5, 0x5F                ; bit 7 = 0 -> normal story playthrough tier
    mov     r6, 0
    ldr     r7, =SET_BATTLE_PARAMS | 1
    bl      lobby_call_r7
    pop     r4, r5, r6
    ldr     r1, =0x01020500
    orr     r1, r6
    mov     r0, 1
    ldr     r3, =BATTLE_ADD_ACTOR | 1
    bl      lobby_call_r3
    ldr     r1, =0x00020200         ; MegaMan
    mov     r0, 0
    ldr     r3, =BATTLE_ADD_ACTOR | 1
    bl      lobby_call_r3
    ldr     r0, =0x0200B480
    mov     r1, 1
    mov     r2, 0xFF
    mov     r3, 1
    ldr     r7, =START_OW_BATTLE | 1
    bl      lobby_call_r7
    mov     r0, 0x2C
    mov     r1, 0x10
    ldr     r3, =SET_MODE_FLAGS | 1
    bl      lobby_call_r3
    mov     r0, 1
    strb    r0, [r5, 4]
@@yield:
    mov     r0, 0
    pop     r4, r5, r6, r7, r15

; Grant Soul r0 = soul_idx (0..11) on friendly-match victory (called by lobby_results_hook)
.align 2
friendly_grant_soul:
    push    r4, r14
    mov     r4, r0
    mov     r0, 0x15                ; Double Soul unlocked
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      lobby_call_r3
    mov     r0, r4
    add     r0, 0x17                ; Soul flag 0x0017 + soul_idx
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      lobby_call_r3
    ldr     r0, =0x161
    add     r0, r0, r4              ; pseudo-chip key 0x0161 + soul_idx
    ldr     r3, =WRITE_CHIP_KEY | 1
    bl      lobby_call_r3
    pop     r4, r15
    .pool

; ---- 3. Relocated NPC lists (vanilla entries + 3 friendly-match NPCs + 0xFF) ---------------
.align 4
friendly_npc_list_dendome:
    .incbin ROM_IN, NPC_LIST_DENDOME_HALL - 0x08000000, 6 * 4
    .word   fm_npc_dex
    .word   fm_npc_match
    .word   fm_npc_higsby
    .word   0xFF

.align 4
friendly_npc_list_castillo:
    .incbin ROM_IN, NPC_LIST_CASTILLO_WAIT - 0x08000000, 5 * 4
    .word   fm_npc_lily
    .word   fm_npc_tamako
    .word   fm_npc_sal
    .word   0xFF

.align 4
friendly_npc_list_colosseum:
    .incbin ROM_IN, NPC_LIST_COLOSSEUM_LOBBY - 0x08000000, 8 * 4
    .word   fm_npc_raika
    .word   fm_npc_raoul
    .word   fm_npc_junk
    .word   0xFF

; ---- 4. The 9 Friendly-Match NPC scripts ---------------------------------------------------
; Macro for single-round window (t, r)
.macro fm_npc_round1, lbl, spr, tourn, rnd, flag, tidx, x, y, dir
.align 4
lbl:
    .byte   0x08
    .byte   0x17, spr
    .byte   0x43, tourn, rnd        ; hide if tournament != round
    .word   @@hide
    .byte   0x04
    .halfword flag                  ; hide if Soul already owned
    .word   @@hide
    .byte   0x18, tidx
    .byte   0x14
    .halfword (x) & 0xFFFF, (y) & 0xFFFF, 0
    .byte   0x16, dir
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00
.endmacro

; Macro for 2-round window (t, r in {0, 1})
.macro fm_npc_round01, lbl, spr, tourn, flag, tidx, x, y, dir
.align 4
lbl:
    .byte   0x08
    .byte   0x17, spr
    .byte   0x42, tourn, 0
    .word   @@chk
    .byte   0x43, tourn, 1
    .word   @@hide
@@chk:
    .byte   0x04
    .halfword flag
    .word   @@hide
    .byte   0x18, tidx
    .byte   0x14
    .halfword (x) & 0xFFFF, (y) & 0xFFFF, 0
    .byte   0x16, dir
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00
.endmacro

; T0 Den Battle — DenDome Hall (02,01)
fm_npc_round1  fm_npc_dex,    0x02, 0, 1, 0x0018, 0xE0, -120, 126, 3  ; Dex/GutsMan (after AquaMan)
fm_npc_round01 fm_npc_match,  0x1D, 0,    0x001B, 0xE1,   16, 152, 5  ; Match/FireMan
fm_npc_round01 fm_npc_higsby, 0x08, 0,    0x001E, 0xE2,   76, 196, 7  ; Higsby/NumberMan

; T1 Eagle / Hawk — Castillo Waiting Room (03,02)
fm_npc_round01 fm_npc_lily,   0x34, 1,    0x0019, 0xE3,   36, -16, 5  ; Lily/WindMan
fm_npc_round01 fm_npc_tamako, 0x1F, 1,    0x001F, 0xE4,   20,  24, 7  ; Tamako/MetalMan
fm_npc_round01 fm_npc_sal,    0x2C, 1,    0x0022, 0xE5,  -40,  20, 3  ; Sal/WoodMan

; T2 Red Sun / Blue Moon — Colosseum Lobby (04,06)
fm_npc_round1  fm_npc_raika,  0x3B, 2, 0, 0x001A, 0xE6,  -12,-108, 3  ; Raika/SearchMan (before Chaud)
fm_npc_round01 fm_npc_raoul,  0x32, 2,    0x001C, 0xE7,  -64,  16, 3  ; Raoul/ThunderMan

; JunkMan signal (uncompressed list[7] 0x08 crystal prop)
.align 4
fm_npc_junk:
    .byte   0x08
    .byte   0x25, 0x08, 0x1C        ; list[7] 0x08 glowing crystal signal
    .byte   0x42, 2, 0
    .word   @@chk
    .byte   0x43, 2, 1
    .word   @@hide
@@chk:
    .byte   0x04
    .halfword 0x0020                ; hide if JunkSoul owned
    .word   @@hide
    .byte   0x18, 0xE8
    .byte   0x14
    .halfword 56, 36, 0
    .byte   0x16, 0
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00

; ---- 5. Friendly-match dialogue archive (talk_idx 0xE0..0xE8 -> scripts 0..8; 9 = No end) --
.align 4
friendly_dialogue_archive:
    .halfword @@m0 - friendly_dialogue_archive
    .halfword @@m1 - friendly_dialogue_archive
    .halfword @@m2 - friendly_dialogue_archive
    .halfword @@m3 - friendly_dialogue_archive
    .halfword @@m4 - friendly_dialogue_archive
    .halfword @@m5 - friendly_dialogue_archive
    .halfword @@m6 - friendly_dialogue_archive
    .halfword @@m7 - friendly_dialogue_archive
    .halfword @@m8 - friendly_dialogue_archive
    .halfword @@m9 - friendly_dialogue_archive

; "Friendly match?\n  Yes   No" + FC 0E <enemy_id> <soul_idx> + E5
.macro fm_msg, lbl, eid, sidx
lbl:
    .byte   0xE7, 0x00
    .byte   0x10,0x37,0x2E,0x2A,0x33,0x29,0x31,0x3E,0x00,0x32,0x26,0x39,0x28,0x2D,0x45, 0xE8
    .byte   0xEA,0x00,0x11,0x00, 0x00,0x23,0x2A,0x38, 0x00,0x00
    .byte   0xEA,0x00,0x00,0x11, 0x00,0x18,0x34
    .byte   0xEC,0x05,0x80, 0xFF, 0x09  ; "No" -> entry 9 (end)
    .byte   0xFC, 0x0E, eid, sidx
    .byte   0xE5
.endmacro

fm_msg @@m0, 0x84, 1                ; GutsMan    -> GutsSoul (0x0018)
fm_msg @@m1, 0x90, 4                ; FireMan    -> FireSoul (0x001B)
fm_msg @@m2, 0x9C, 7                ; NumberMan  -> NumberSoul (0x001E)
fm_msg @@m3, 0x88, 2                ; WindMan    -> WindSoul (0x0019)
fm_msg @@m4, 0xA0, 8                ; MetalMan   -> MetalSoul (0x001F)
fm_msg @@m5, 0xAC, 11               ; WoodMan    -> WoodSoul (0x0022)
fm_msg @@m6, 0x8C, 3                ; SearchMan  -> SearchSoul (0x001A)
fm_msg @@m7, 0x94, 5                ; ThunderMan -> ThunderSoul (0x001C)
fm_msg @@m8, 0xA4, 9                ; JunkMan    -> JunkSoul (0x0020)
@@m9:
    .byte   0xE5

.if . > FS_FRIENDLY + FS_FRIENDLY_SIZE
    .error "WP-B1 friendly matches: overruns its free-space slot"
.endif
