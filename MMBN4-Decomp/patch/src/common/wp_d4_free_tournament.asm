; ============================================================
; WP-D4 / Violet Eclipse: Free Tournament 36-secret-chip fix (owner decision 2026-11-23).
; Design: docs/pending/free-tournament-secret-chips.md & decomp/notes/free-tournament-battle-data.md.
;
; 1. FreeTournamentSetup: instead of reading the link-trade Waiting Room (0x020014F0), fills the 7
;    opponent slots directly from the 12 Soul Navis (ids 0-11), picking Navis whose 3 secret chips
;    (Library flags 0x16C0 + base..base+2) are still missing first, then filling remaining slots.
;    The vanilla Fisher-Yates shuffle and bracket placement run unchanged.
; 2. FreeTournamentSetupPlayerMatch (case 0): spawns Soul Navi opponents as standard type-0x01
;    ROM enemies (enemy_id = 0x7C + soul_index*4 + tier, 0x80..0xAE) instead of type-0x04 traded
;    Navis, removing all dependency on traded save slots +0x78 / +0x7C.
; 3. Prizes (FreeTournamentSelectPrize 0x08045458) stay unchanged: BL 1-3 V1, 4-6 V2, 7-9 V3,
;    BL 10/S awards the highest unowned secret chip for that Navi.
; ============================================================

; ---- 0. Bypass empty Waiting Room gate (same address in RS & BM) -----------------------------
.org 0x080057CE                     ; FreeTournamentStart: allow starting with 0 traded Navis
    mov     r0, 1
    b       0x080057E6

; ---- 1. Hook FreeTournamentSetup Waiting-Room scan -> FS_FREE_TOURN_FILL ---------------------
.org FREE_TOURN_SCAN                ; 0x080448DA (RS) / 0x080448E2 (BM); 24 bytes before shuffle
    mov     r8, r8                  ; align to word boundary (+2)
    ldr     r3, [r15, 0]
    bx      r3
    .word   FS_FREE_TOURN_FILL | 1

; ---- 2. Hook FreeTournamentSetupPlayerMatch case 0 -> free_tourn_case0 (type-0x01) -----------
.org FREE_TOURN_PLAYER_CASE0        ; 0x08044F8E (RS) / 0x08044F96 (BM); both 2 mod 4
    mov     r8, r8                  ; align to word boundary (+2)
    ldr     r3, [r15, 0]
    bx      r3
    .word   free_tourn_case0 | 1

; ---- 3. Missing-chips-first Soul Navi selector in free space ---------------------------------
.org FS_FREE_TOURN_FILL
    ; In: r7 = 0x0201A000 (entry 0 = MegaMan), r6 = 2. Out: r6 = 16 (8 entrants), jumps to shuffle.
    mov     r4, 0                   ; bitmask of Soul Navis (0..11) already added
    ; Pass 1: add any Soul Navi (0..11) missing at least one of its 3 secret chips
    mov     r5, 0
@@pass1:
    cmp     r6, 16
    bge     @@done
    ldr     r0, =FREE_SECRET_CHIP_BASE
    lsl     r1, r5, 1
    ldrh    r2, [r0, r1]            ; base V1 secret chip id
    ldr     r0, =0x16C0
    add     r2, r2, r0              ; Library flag for V1
    mov     r3, 3                   ; check V1, V2, V3
@@chk_chip:
    push    r2, r3
    mov     r0, r2
    ldr     r3, =TEST_EVENT_FLAG | 1
    bl      free_tourn_call_r3
    pop     r2, r3
    beq     @@add_p1                ; flag clear -> missing this chip!
    add     r2, 1
    sub     r3, 1
    bne     @@chk_chip
    b       @@next_p1
@@add_p1:
    lsl     r0, r5, 8               ; {slot=0, navi_id=r5}
    strh    r0, [r7, r6]
    add     r6, 2
    mov     r0, 1
    lsl     r0, r5
    orr     r4, r0
@@next_p1:
    add     r5, 1
    cmp     r5, 12
    blt     @@pass1

    ; Pass 2: fill any remaining slots (< 16) with unpicked Soul Navis (0..11)
    mov     r5, 0
@@pass2:
    cmp     r6, 16
    bge     @@done
    mov     r0, 1
    lsl     r0, r5
    tst     r4, r0
    bne     @@next_p2
    lsl     r0, r5, 8
    strh    r0, [r7, r6]
    add     r6, 2
@@next_p2:
    add     r5, 1
    cmp     r5, 12
    blt     @@pass2

@@done:
    ldr     r3, =FREE_TOURN_SHUFFLE | 1
    bx      r3

; ---- 4. Case 0 spawner: Soul Navi as standard type-0x01 enemy (no +0x78/+0x7C reads) --------
.align 2
free_tourn_case0:
    ; In: r2 = navi_id (0..11) from FreeTournamentClassifyEntrant; r6 = 0x020015E0
    ldr     r3, =FREE_TOURN_GET_SOUL | 1
    bl      free_tourn_call_r3               ; r2 = soul_index (1..12)
    lsl     r4, r2, 2
    add     r4, 0x7C                ; base enemy_id: 0x80 (Roll) .. 0xAC (WoodMan)
    mov     r0, 0
    ldr     r3, =FREE_TOURN_GET_TIER | 1
    bl      free_tourn_call_r3               ; r0 = tier (0..2)
    add     r4, r4, r0              ; V1 / V2 / V3 enemy_id
    ldr     r1, =0x01020500
    orr     r1, r4                  ; standard type-0x01 Navi at col 5, row 2
    mov     r0, 1
    ldr     r3, =BATTLE_ADD_ACTOR | 1
    bl      free_tourn_call_r3
    ldr     r3, =FREE_TOURN_SPAWN_MEGA | 1
    bx      r3

free_tourn_call_r3:
    bx      r3
    .pool
.if . > FS_FREE_TOURN_FILL + FS_FREE_TOURN_SIZE
    .error "WP-D4 Free Tournament: selector overruns its free-space slot"
.endif
