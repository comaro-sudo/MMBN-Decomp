; ============================================================
; WP-D3 / Violet Eclipse: Lobby Rematches (11 operators + JunkMan Trash Can).
; Design: decomp/notes/lobby-npc-scripts-design.md & decomp/notes/placements.md.
;
; - 9 operators inside Higsby's (00,06) + 2 operators & Trash Can on ACDC Town (00,00) +
;   JunkMan inside WtrGodComp1 (8E,00) (12 Soul Navis total, each gated on met flag 0x0001+id).
; - Dialogue redirect hook at sub_0804E610: talk_idx >= 0xF0 reads FS_LOBBY_DIALOGUE (zero
;   vanilla text archives touched, works on every story phase).
; - ScriptGroup_FC table extended with:
;     FC 09 <navi_id> : launch rematch at Navi's current 2-bit tier (flags 0x0E40+id*2: V1/V2/V3/SP)
;     FC 0A 00        : jack in from Trash Can to WtrGodComp1 (8E,00) for JunkMan
; - BattleResultsGrantReward hook: on lobby rematch victory, climbs tier on S-rank (BL >= 11)
;   and grants the Navi's Secret Chip (BL 1-3 V1, 4-6 V2, 7-9 V3, 10/S highest unowned).
; ============================================================

; ---- 1. Hook OpenMapDialogue (talk_idx >= 0xF0 -> lobby_dialogue_archive) --------------------
.org OPEN_MAP_DIALOGUE              ; 0x0804E0A0 (RS) / 0x0804E0A8 (BM), word-aligned
    ldr     r3, [r15, 0]
    bx      r3
    .word   lobby_open_dialogue | 1

; ---- 2. Repoint ScriptGroup_FC jump table -> lobby_fc_table ----------------------------------
.org SCRIPT_FC_TBL_PTR              ; 0x08050430 (RS) / 0x0805043C (BM)
    .word   lobby_fc_table

; ---- 3. Hook BattleResultsGrantReward exit -> veneer -> lobby_results_hook -------------------
.org RESULTS_DONE_HOOK              ; 0x0802174C (RS) / 0x08021750 (BM)
    bl      LOBBY_RESULTS_VENEER

.org LOBBY_RESULTS_VENEER           ; dead block tail after BassPairGive (word-aligned)
    ldr     r3, [r15, 0]
    bx      r3
    .word   lobby_results_hook | 1

; ---- 4. Relocate WtrGodComp1 (8E,00) CW NPC list to add JunkMan ------------------------------
.org NPC_SLOT_WTRGOD1
    .word   lobby_npc_list_wtrgod1

; ---- 5. Code, tables, NPC scripts, and dialogue archive in free space ------------------------
.org FS_LOBBY_REMATCH

; -- 5a. OpenMapDialogue redirect hook (0xF0..0xFF lobby, 0xE0..0xEF friendly matches) --
lobby_open_dialogue:
    push    r4, r5, r14
    mov     r1, r0
    cmp     r1, 0xF0
    bge     @@lobby
    cmp     r1, 0xE0
    blt     @@vanilla
    ldr     r0, =friendly_dialogue_archive
    sub     r1, 0xE0
    b       @@open
@@lobby:
    ldr     r0, =lobby_dialogue_archive
    sub     r1, 0xF0
    b       @@open
@@vanilla:
    ldr     r0, =0x02030000
@@open:
    ldr     r3, =OPEN_CUSTOM_DIALOGUE | 1
    bl      lobby_call_r3
    pop     r4, r5, r15

; -- 5b. ScriptGroup_FC 09 <navi_id>: launch rematch at current 2-bit tier --
.align 2
lobby_fc_rematch:
    push    r4, r5, r6, r7, r14
    mov     r0, 1
    ldr     r3, =CLOSE_DIALOGUE | 1
    bl      lobby_call_r3
    ldrb    r0, [r5, 4]
    cmp     r0, 0
    bne     @@yield
    ldrb    r4, [r4, 2]             ; r4 = navi_id (0..11)
    ldr     r0, =0x020015EF         ; 1-byte rematch marker in EWRAM
    mov     r1, 0x80
    orr     r1, r4
    strb    r1, [r0]
    ; Read 2-bit tier (0..3) from flags 0x0E40 + navi_id*2
    ldr     r6, =0x0E40
    lsl     r0, r4, 1
    add     r6, r6, r0              ; r6 = flag_lo
    mov     r7, 0                   ; r7 = tier
    mov     r0, r6
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      lobby_call_r3
    beq     @@no_b0
    add     r7, 1
@@no_b0:
    add     r0, r6, 1
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      lobby_call_r3
    beq     @@no_b1
    add     r7, 2
@@no_b1:
    ; If tier == 3 (SP), activate SP+ when this Navi's SP+ flag (0x0E5D + navi_id) is set
    cmp     r7, 3
    bne     @@sp_done
    ldr     r0, =0x0E5D
    add     r0, r0, r4
    bl      sp_plus_check_activate
@@sp_done:
    ; Map navi_id (0..11) -> soul_index (1..12) -> enemy_id = 0x7C + soul_index*4 + tier
    mov     r2, r4
    ldr     r3, =FREE_TOURN_GET_SOUL | 1
    bl      lobby_call_r3           ; r2 = soul_index (1..12)
    lsl     r6, r2, 2
    add     r6, 0x7C
    add     r6, r6, r7              ; r6 = enemy_id (0x80..0xAF)
    ; Build dynamic RAM battle record & launch
    ldr     r3, =INIT_BATTLE_REC | 1
    bl      lobby_call_r3
    push    r4, r5, r6
    mov     r0, 0
    mov     r1, 0x64
    mov     r2, 0x14
    mov     r3, 0x3F
    mov     r4, 3
    mov     r5, 0xDF
    mov     r6, 0
    ldr     r7, =SET_BATTLE_PARAMS | 1
    bl      lobby_call_r7
    pop     r4, r5, r6
    ldr     r1, =0x01020500
    orr     r1, r6                  ; standard type-0x01 Soul Navi
    mov     r0, 1
    ldr     r3, =BATTLE_ADD_ACTOR | 1
    bl      lobby_call_r3
    ldr     r1, =0x00020200         ; MegaMan
    mov     r0, 0
    ldr     r3, =BATTLE_ADD_ACTOR | 1
    bl      lobby_call_r3
    ldr     r0, =0x0200B480         ; dynamic RAM battle record
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

; -- 5c. ScriptGroup_FC 0A 00: Trash Can warp -> WtrGodComp1 (8E,00) --
.align 2
lobby_fc_trash_warp:
    push    r4, r5, r6, r7, r14
    mov     r0, 1
    ldr     r3, =CLOSE_DIALOGUE | 1
    bl      lobby_call_r3
    ldrb    r0, [r5, 4]
    cmp     r0, 0
    bne     @@yield
    ldr     r0, =lobby_junkman_warp
    mov     r1, 0
    ldr     r3, =WARP_BY_TABLE | 1
    bl      lobby_call_r3
    mov     r0, 0x0C
    mov     r1, 0x10
    ldr     r3, =SET_MODE_FLAGS | 1
    bl      lobby_call_r3
    mov     r7, r10
    ldr     r2, [r7, 0x40]
    mov     r0, 0x0C
    strb    r0, [r2]
    ldr     r3, =WARP_CLEANUP_1 | 1
    bl      lobby_call_r3
    ldr     r3, =WARP_CLEANUP_2 | 1
    bl      lobby_call_r3
    mov     r0, 1
    strb    r0, [r5, 4]
@@yield:
    mov     r0, 0
    pop     r4, r5, r6, r7, r15

; -- 5d. Battle results hook: S-rank tier climb + Secret Chip award --
.align 2
lobby_results_hook:
    push    r4, r5, r6, r7, r14
    mov     r0, 0
    strh    r0, [r5, 2]             ; replaced original instruction
    ldr     r0, =0x020015EF
    ldrb    r4, [r0]
    mov     r1, 0x80
    tst     r4, r1
    bne     @@is_lobby
    mov     r1, 0x40
    tst     r4, r1
    beq     @@not_ours
    mov     r1, 0
    strb    r1, [r0]                ; clear friendly-match marker
    mov     r0, r4
    sub     r0, 0x40                ; soul_idx (0..11)
    bl      friendly_grant_soul
    b       @@ret
@@not_ours:
    bl      sp_plus_unlock_roam     ; not a lobby/friendly match: check roaming SP S-rank unlock
    b       @@ret
@@is_lobby:
    mov     r1, 0
    strb    r1, [r0]                ; clear marker
    sub     r4, 0x80                ; r4 = navi_id (0..11) or 13 (Duo)
    ldrb    r6, [r5, 8]             ; r6 = Busting Level (1..11, 11=S)
    ; S-rank tier climb (V1 -> V2 -> V3 -> SP)
    cmp     r6, 11
    blt     @@grant_secret
    ldr     r5, =0x0E40
    lsl     r0, r4, 1
    add     r5, r5, r0              ; r5 = flag_lo
    mov     r7, 0
    mov     r0, r5
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      lobby_call_r3
    beq     @@t_b0
    add     r7, 1
@@t_b0:
    add     r0, r5, 1
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      lobby_call_r3
    beq     @@t_b1
    add     r7, 2
@@t_b1:
    cmp     r7, 3
    blt     @@climb_tier
    ; Already at SP / Duo-omega and got S rank -> unlock SP+!
    cmp     r4, 12
    blt     @@unlock_navi_sp
    ldr     r0, =0x0E5C             ; Duo SP+ unlock flag
    b       @@do_unlock_sp
@@unlock_navi_sp:
    ldr     r0, =0x0E5D
    add     r0, r0, r4              ; 0x0E5D + navi_id
@@do_unlock_sp:
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      lobby_call_r3
    b       @@grant_secret
@@climb_tier:
    add     r7, 1
    ; Write bit 0
    mov     r0, r5
    mov     r1, 1
    tst     r7, r1
    beq     @@clr_b0
    ldr     r3, =SET_EVENT_FLAG | 1
    b       @@do_b0
@@clr_b0:
    ldr     r3, =CLEAR_EVENT_FLAG | 1
@@do_b0:
    bl      lobby_call_r3
    ; Write bit 1
    add     r0, r5, 1
    mov     r1, 2
    tst     r7, r1
    beq     @@clr_b1
    ldr     r3, =SET_EVENT_FLAG | 1
    b       @@do_b1
@@clr_b1:
    ldr     r3, =CLEAR_EVENT_FLAG | 1
@@do_b1:
    bl      lobby_call_r3

@@grant_secret:
    cmp     r4, 12                  ; Duo (marker id 13, wp_d3_duo_rematch.asm): no secret chip, but the Duo Soul
    blt     @@navi_secret           ; and Duo D (once), since Duo's deletion state may not run in a plain battle
    bl      duo_grant_rewards       ; (Gemini round 11). Beta/omega also drop Duo D (wp_d2_duo_chip_reward.asm)
    b       @@ret
@@navi_secret:
    ; Pick Secret Chip: BL 1-3 V1, 4-6 V2, 7-9 V3, 10/S highest unowned
    ldr     r0, =FREE_SECRET_CHIP_BASE
    lsl     r1, r4, 1
    ldrh    r7, [r0, r1]            ; r7 = base V1 chip ID
    cmp     r6, 10
    bge     @@missing_first
    cmp     r6, 7
    bge     @@pick_v3
    cmp     r6, 4
    bge     @@pick_v2
    b       @@give_chip
@@pick_v2:
    add     r7, 1
    b       @@give_chip
@@pick_v3:
    add     r7, 2
    b       @@give_chip
@@missing_first:
    mov     r5, 2                   ; check V3 (2), V2 (1), V1 (0)
@@chk_miss:
    ldr     r0, =0x16C0
    add     r0, r0, r7
    add     r0, r0, r5
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      lobby_call_r3
    beq     @@found_miss
    sub     r5, 1
    bge     @@chk_miss
    mov     r5, 2                   ; all 3 owned -> V3
@@found_miss:
    add     r7, r7, r5
@@give_chip:
    mov     r0, r7
    ldr     r3, =BN4_GET_CHIP | 1
    bl      lobby_call_r3
    ldrb    r1, [r0]                ; first valid chip code
    mov     r0, r7
    mov     r2, 1
    ldr     r3, =CHIP_GIVE | 1
    bl      lobby_call_r3
@@ret:
    pop     r4, r5, r6, r7, r15

lobby_call_r3:
    bx      r3
lobby_call_r7:
    bx      r7
    .pool

; -- 5e. Extended ScriptGroup_FC table (9 vanilla + FC 09 rematch + FC 0A trash warp) --
.align 4
lobby_fc_table:
    .incbin ROM_IN, SCRIPT_FC_TBL_ORIG - 0x08000000, 9 * 4
    .word   lobby_fc_rematch | 1    ; FC 09 <navi_id>
    .word   lobby_fc_trash_warp | 1 ; FC 0A 00
    .word   freq_tuner_fc_cycle | 1 ; FC 0B 00 (WP-B2 Frequency Tuner)
    .word   grant_soul_fc | 1       ; FC 0C <idx> (WP-D4 grant Soul, wp_d4_grant_soul.asm)
    .word   duo_fc_rematch | 1      ; FC 0D 00 (WP-D3 Duo rematch, wp_d3_duo_rematch.asm)
    .word   friendly_fc_battle | 1  ; FC 0E <eid> <sidx> (WP-B1 friendly matches)

; -- 5f. Trash Can warp descriptor -> WtrGodComp1 (8E,00) at (0, 0, 0) facing 7 --
.align 4
lobby_junkman_warp:
    .byte   0x8E, 0x00, 0x00, 0x07
    .word   0, 0, 0

; -- 5g. Relocated WtrGodComp1 (8E,00) CW NPC list (1 vanilla + JunkMan) --
.align 4
lobby_npc_list_wtrgod1:
    .incbin ROM_IN, NPC_LIST_WTRGOD1 - 0x08000000, NPC_COUNT_WTRGOD1 * 4
    .word   lobby_npc_junkman
    .word   0xFF

; -- 5h. Operator & Trash Can & JunkMan NPC scripts (25 bytes each) --
.macro lobby_npc, lbl, spr, met_flag, talk_idx, x, y, dir
.align 4
lbl:
    .byte   0x08
    .byte   0x17, spr
    .byte   0x05, (met_flag) & 0xFF, ((met_flag) >> 8) & 0xFF
    .word   @@hide
    .byte   0x18, talk_idx
    .byte   0x14
    .halfword (x) & 0xFFFF, (y) & 0xFFFF, 0
    .byte   0x16, dir
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00
.endmacro

; 9 Higsby's operators (placements.md §1). Sprites and the gates (the Navi's Double Soul flag 0x0017-0x0022) from Gemini's lobby-fixes.md:
; flags 0x0001-0x000C, used before, are only set for traded Navis
lobby_npc lobby_npc_chaud,  0x04, 0x001D, 0xFA,   36,  42, 5  ; ProtoMan (0x0A)
lobby_npc lobby_npc_mayl,   0x01, 0x0017, 0xF4,   36,  70, 5  ; Roll     (0x04)
lobby_npc lobby_npc_dex,    0x02, 0x0018, 0xF0,   36,  98, 5  ; GutsMan  (0x00)
lobby_npc lobby_npc_match,  0x1D, 0x001B, 0xF1,   12, 112, 7  ; FireMan  (0x01)
lobby_npc lobby_npc_shuko,  0x20, 0x0021, 0xF3,  -54, 112, 7  ; AquaMan  (0x03)
lobby_npc lobby_npc_lily,   0x34, 0x0019, 0xF5,  -84, 112, 7  ; WindMan  (0x05)
lobby_npc lobby_npc_tamako, 0x1F, 0x001F, 0xF6, -140,  44, 3  ; MetalMan (0x06)
lobby_npc lobby_npc_sal,    0x2C, 0x0022, 0xF7, -134,  72, 1  ; WoodMan  (0x07)
lobby_npc lobby_npc_number, 0x08, 0x001E, 0xF2,  -56,  56, 3  ; NumberMan(0x02)

; 2 ACDC Town street operators + Trash Can (placements.md §2)
lobby_npc lobby_npc_raika,    0x3B, 0x001A, 0xF9, -134, -76, 5 ; SearchMan  (0x09)
lobby_npc lobby_npc_raoul,    0x32, 0x001C, 0xF8, -164, -56, 7 ; ThunderMan (0x08)
; Trash Can / Junk Pile (Gemini round 8, lobby-fixes.md): op 0x25 -> list[7] 0x44, gated on JunkSoul 0x0020
.align 4
lobby_npc_trashcan:
    .byte   0x08
    .byte   0x25, 0x4A, 0x1C        ; list[7] 0x4A, uncompressed debris/wreckage pile (prop-sprites.md)
    .byte   0x05, 0x20, 0x00        ; hide while JunkSoul (0x0020) is clear
    .word   @@hide
    .byte   0x18, 0xFC
    .byte   0x14
    .halfword (-150) & 0xFFFF, (-90) & 0xFFFF, 0
    .byte   0x16, 0                 ; anim 0: list[7] 0x44 has only anims 0-2 (3 drew a white circle; Gemini round 10)
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00

; JunkMan inside WtrGodComp1 (8E,00) (list[6] sprite 0x5A)
lobby_npc lobby_npc_junkman,  0x5A, 0x0020, 0xFB,    0, -24, 3 ; JunkMan (0x0B)

; -- 5i. Custom Yes/No dialogue archive (indices 0x00..0x0F = talk_idx 0xF0..0xFF; 0x0F is only the "No" target) --
.align 4
lobby_dialogue_archive:
    .halfword @@s0 - lobby_dialogue_archive
    .halfword @@s1 - lobby_dialogue_archive
    .halfword @@s2 - lobby_dialogue_archive
    .halfword @@s3 - lobby_dialogue_archive
    .halfword @@s4 - lobby_dialogue_archive
    .halfword @@s5 - lobby_dialogue_archive
    .halfword @@s6 - lobby_dialogue_archive
    .halfword @@s7 - lobby_dialogue_archive
    .halfword @@s8 - lobby_dialogue_archive
    .halfword @@s9 - lobby_dialogue_archive
    .halfword @@sA - lobby_dialogue_archive
    .halfword @@sB - lobby_dialogue_archive
    .halfword @@sC - lobby_dialogue_archive
    .halfword @@sD - lobby_dialogue_archive
    .halfword @@sE - lobby_dialogue_archive
    .halfword @@sF - lobby_dialogue_archive

; "NetBattle? \n  Yes   No" + FC 09 <navi_id> + E5
.macro lobby_rematch_msg, lbl, nid
lbl:
    .byte   0xE7, 0x00
    .byte   0x18,0x2A,0x39,0x0C,0x26,0x39,0x39,0x31,0x2A,0x45, 0xE8
    .byte   0xEA,0x00,0x11,0x00, 0x00,0x23,0x2A,0x38, 0x00,0x00
    .byte   0xEA,0x00,0x00,0x11, 0x00,0x18,0x34
    .byte   0xEC,0x05,0x80, 0xFF, 0x0F  ; "No" -> entry 0x0F (end)
    .byte   0xFC, 0x09, nid
    .byte   0xE5
.endmacro

lobby_rematch_msg @@s0, 0x00
lobby_rematch_msg @@s1, 0x01
lobby_rematch_msg @@s2, 0x02
lobby_rematch_msg @@s3, 0x03
lobby_rematch_msg @@s4, 0x04
lobby_rematch_msg @@s5, 0x05
lobby_rematch_msg @@s6, 0x06
lobby_rematch_msg @@s7, 0x07
lobby_rematch_msg @@s8, 0x08
lobby_rematch_msg @@s9, 0x09
lobby_rematch_msg @@sA, 0x0A
lobby_rematch_msg @@sB, 0x0B

; Trash Can: "Jack in? \n  Yes   No" + FC 0A 00 + E5
@@sC:
    .byte   0xE7, 0x00
    .byte   0x14,0x26,0x28,0x30,0x00,0x2E,0x33,0x45, 0xE8
    .byte   0xEA,0x00,0x11,0x00, 0x00,0x23,0x2A,0x38, 0x00,0x00
    .byte   0xEA,0x00,0x00,0x11, 0x00,0x18,0x34
    .byte   0xEC,0x05,0x80, 0xFF, 0x0F  ; "No" -> entry 0x0F (end)
    .byte   0xFC, 0x0A, 0x00
    .byte   0xE5

; Duo rematch (0x0D = talk_idx 0xFD, wp_d3_duo_rematch.asm): "Fight Duo? / Yes / No" + FC 0D 00 + E5
@@sD:
    .byte   0xE7, 0x00
    .byte   0x10,0x2E,0x2C,0x2D,0x39,0x00,0x0E,0x3A,0x34,0x45, 0xE8
    .byte   0xEA,0x00,0x11,0x00, 0x00,0x23,0x2A,0x38, 0x00,0x00
    .byte   0xEA,0x00,0x00,0x11, 0x00,0x18,0x34
    .byte   0xEC,0x05,0x80, 0xFF, 0x0F  ; "No" -> entry 0x0F (end)
    .byte   0xFC, 0x0D, 0x00
    .byte   0xE5

; Frequency Tuner (0x0E = talk_idx 0xFE): "Tune Tier? \n  Yes   No" + FC 0B 00 + E5
@@sE:
    .byte   0xE7, 0x00
    .byte   0x1E,0x3A,0x33,0x2A,0x00,0x1E,0x2E,0x2A,0x37,0x45, 0xE8
    .byte   0xEA,0x00,0x11,0x00, 0x00,0x23,0x2A,0x38, 0x00,0x00
    .byte   0xEA,0x00,0x00,0x11, 0x00,0x18,0x34
    .byte   0xEC,0x05,0x80, 0xFF, 0x0F  ; "No" -> entry 0x0F (end)
    .byte   0xFC, 0x0B, 0x00
    .byte   0xE5

; "No" target (0x0F): just end (was 0x0D until the Duo rematch took that entry)
@@sF:
    .byte   0xE5

.if . > FS_LOBBY_REMATCH + FS_LOBBY_REMATCH_SIZE
    .error "WP-D3 Lobby Rematches: overruns free-space slot"
.endif
