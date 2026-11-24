; ============================================================
; WP-D3: Duo rematches in the Meteor Comp (post-game). Owner decisions: docs/completion/bosses.md
; "Duo rematches"; research: Gemini, decomp/notes/duo-rematch-research.md (round 9).
;
; - "Duo's signal" NPC (list[6] 0x63, Duo's probe) in Meteor Comp 1 (82,00), right by the jack-in point,
;   shown once the Game Clear flag 0x0500 is set (kept across New Game+).
; - Talk (talk_idx 0xFD -> lobby archive entry 0x0D): "Fight Duo?  Yes / No" -> FC 0D 00.
; - FC 0D replays the final battle as a plain battle: story battle 93/94's header with b1 = 0
;   (0x93 and 0x13 both skip the results screen; nearly every story battle uses 0) and bit 7 of b6 set (0x1F -> 0x9F: no automatic
;   tier raise), so the chosen Duo version is fought as is, and the ending doesn't replay.
; - Tier: 2-bit counter in flags 0x0E5A-0x0E5B (0 Duo 0xE0, 1 Duo-alpha 0xE1, 2 Duo-beta 0xE2,
;   3 Duo-omega 0xE3). It reuses the lobby's results hook: marker id 13 makes the lobby's flag math
;   (0x0E40 + id * 2) land on 0x0E5A, so an S rank climbs one tier, up to Duo-omega. The lobby hook skips
;   its secret-chip award for Duo; Duo-beta/omega's own drop gives the Duo chip (wp_d2_duo_chip_reward.asm).
;   Flag 0x0E5C stays free for the SP+ step (docs/pending/boss-double-hp.md).
; ============================================================

DUO_MARKER_ID   equ 13

; ---- 1. Relocate Meteor Comp 1 (82,00)'s CW NPC list to add Duo's signal ----
.org NPC_SLOT_METEOR1
    .word   duo_npc_list_meteor1

.org FS_DUO_REMATCH

; ---- 2. FC 0D 00: start the rematch at Duo's current tier ----
.align 2
duo_fc_rematch:
    push    r4, r5, r6, r7, r14
    mov     r0, 1
    ldr     r3, =CLOSE_DIALOGUE | 1
    bl      lobby_call_r3
    ldrb    r0, [r5, 4]
    cmp     r0, 0
    bne     @@yield
    ldr     r0, =0x020015EF         ; the lobby's rematch marker (bit 7 + id)
    mov     r1, 0x80 | DUO_MARKER_ID
    strb    r1, [r0]
    ldr     r6, =0x0E40 + DUO_MARKER_ID * 2   ; 0x0E5A
    mov     r7, 0                   ; r7 = tier 0..3
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
    cmp     r7, 3                   ; Duo-omega -> activate SP+ if flag 0x0E5C is set
    bne     @@sp_done
    ldr     r0, =0x0E5C
    bl      sp_plus_check_activate
@@sp_done:
    lsl     r0, r7, 3
    lsl     r7, r7, 2
    add     r7, r0, r7              ; tier * 12
    ldr     r0, =duo_rematch_recs
    add     r0, r0, r7
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
    .pool

; ---- 3. Battle records: story battle 93/94's header as a plain battle, one per Duo version ----
.macro duo_rec, objs
    .byte   0x00, 0x00, 0x64, 0x16, 0x33, 0x17, 0x9F, 0x00  ; Duo's own battle header (b1=0, b6=0x9F no auto tier raise;
                                    ; b4=0x3F hangs Duo's battle fade-in). Duo Soul + Duo D paid by duo_defeat_hook.
    .word   objs
.endmacro
.macro duo_objs, enemy
    .byte   0x00, 0x02, 0x02, 0x00  ; MegaMan, col 2, row 2
    .byte   enemy, 0x05, 0x02, 0x01 ; Duo, col 5, row 2
    .byte   0xFF
.endmacro

.align 4
duo_rematch_recs:
    duo_rec duo_objs_v1
    duo_rec duo_objs_alpha
    duo_rec duo_objs_beta
    duo_rec duo_objs_omega
duo_objs_v1:
    duo_objs 0xE0
duo_objs_alpha:
    duo_objs 0xE1
duo_objs_beta:
    duo_objs 0xE2
duo_objs_omega:
    duo_objs 0xE3

; ---- 4. Meteor Comp 1 (82,00) NPC list (vanilla entries + Duo's signal) and the NPC script ----
.align 4
duo_npc_list_meteor1:
    .incbin ROM_IN, NPC_LIST_METEOR1 - 0x08000000, NPC_COUNT_METEOR1 * 4
    .word   duo_npc_signal
    .word   0xFF

.align 4
duo_npc_signal:
    .byte   0x08
    .byte   0x25, 0x07, 0x1C        ; list[7] 0x07, uncompressed Duo hexagonal crest (prop-sprites.md)
    .byte   0x05, 0x00, 0x05        ; hide until Game Clear (0x0500)
    .word   @@hide
    .byte   0x18, 0xFD              ; lobby archive entry 0x0D: "Fight Duo?"
    .byte   0x14
    .halfword 212, 112, 0           ; by the jack-in point (236, 132); Gemini, to check on screen
    .byte   0x16, 0                 ; animation 0 (always valid; see the trash can fix)
    .byte   0x36
    .word   NPC_IDLE_TALK_LOOP
@@hide:
    .byte   0x00

.if . > FS_DUO_REMATCH + FS_DUO_REMATCH_SIZE
    .error "WP-D3 Duo rematch: overruns its free-space slot"
.endif

; ---- 5. Duo rematch S-rank tier climb (decomp/notes/duo-rematch-rank.md) ----
; Duo's battle (b4 = 0x33) skips the results screen, so the lobby's results hook never runs.
; However, sub_08007642 ALWAYS calls sub_08008558 (BattleCalcBustingLevel, RS == BM) at 0x08007692
; on every win, returning 1..11 (11 = S-rank) using the standard boss formula.
; Hook 0x08007692 -> VENEER_DUO_RANK (0x08001784) -> duo_rank_climb_hook:
; on S-rank (r0 >= 11) in an active Duo rematch ([0x020015EF] == 0x80 | 13), climb
; V1 (0) -> alpha (1) -> beta (2) -> omega (3) via flags 0x0E5A-0x0E5B, and on omega S-rank
; set flag 0x0E5C to unlock Duo-omega SP+.

.org 0x08007692
    bl      VENEER_DUO_RANK

.org VENEER_DUO_RANK
.align 4
    ldr     r3, =duo_rank_climb_hook | 1
    bx      r3
    .pool

.org FS_DUO_RANK
.align 4
duo_rank_climb_hook:
    push    r4-r7, r14
    ldr     r3, =0x08008558 | 1     ; BattleCalcBustingLevel (RS == BM) -> r0 = 1..11 (11 = S)
    bl      @@call_r3
    mov     r4, r0                  ; preserve exact Busting Level to return in r0
    cmp     r4, 11                  ; S-rank?
    blt     @@done
    ldr     r0, =0x020015EF
    ldrb    r0, [r0]
    cmp     r0, 0x80 | DUO_MARKER_ID
    bne     @@done
    ldr     r6, =0x0E5A             ; Duo 2-bit tier flags: 0x0E5A (bit 0), 0x0E5B (bit 1)
    mov     r7, 0
    mov     r0, r6
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      @@call_r3
    beq     @@no_b0
    add     r7, 1
@@no_b0:
    add     r0, r6, 1
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      @@call_r3
    beq     @@no_b1
    add     r7, 2
@@no_b1:
    cmp     r7, 3                   ; already Duo-omega? -> unlock Duo-omega SP+ (0x0E5C)
    blt     @@climb
    ldr     r0, =0x0E5C
    ldr     r3, =SET_EVENT_FLAG | 1
    bl      @@call_r3
    b       @@done
@@climb:
    add     r7, 1                   ; next tier: 1 (alpha), 2 (beta), 3 (omega)
    mov     r0, r6                  ; bit 0 -> 0x0E5A
    mov     r1, 1
    tst     r7, r1
    beq     @@clr_b0
    ldr     r3, =SET_EVENT_FLAG | 1
    b       @@do_b0
@@clr_b0:
    ldr     r3, =CLEAR_EVENT_FLAG | 1
@@do_b0:
    bl      @@call_r3
    add     r0, r6, 1               ; bit 1 -> 0x0E5B
    mov     r1, 2
    tst     r7, r1
    beq     @@clr_b1
    ldr     r3, =SET_EVENT_FLAG | 1
    b       @@do_b1
@@clr_b1:
    ldr     r3, =CLEAR_EVENT_FLAG | 1
@@do_b1:
    bl      @@call_r3
@@done:
    mov     r0, r4
    pop     r4-r7, r15
@@call_r3:
    bx      r3
    .pool

.if . > FS_DUO_RANK + FS_DUO_RANK_SIZE
    .error "WP-D3 Duo rank climb: overruns its free-space slot"
.endif
