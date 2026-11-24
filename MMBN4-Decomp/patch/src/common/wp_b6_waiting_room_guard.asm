; ============================================================
; WP-B6 (Guard A): Waiting Room traded-Navi slot bounds check.
; Research: Gemini, decomp/notes/waiting-room-bug.md (round 12).
;
; WaitingRoomReceiveTradedNavi (RS 0x0803FA90 / BM 0x0803FA98) reads the
; incoming Waiting Room slot index r6 from [r5, #0x34] and indexes a
; 7-slot table using (r6 - 1). A corrupted/out-of-range slot (0 or > 7)
; underflows or overflows adjacent save memory.
;
; Guard A validates 1 <= r6 <= 7 before any table writes; if out of range,
; returns r0 = 1 so the caller (sub_0803F394) takes the normal link-error
; path (sub_0803FBD8) without saving.
; ============================================================

.org WAITING_ROOM_RECEIVE
    ldr r3, =waiting_room_guard | 1
    bx  r3
    .pool

.org FS_WAITING_ROOM_GUARD
.align 4
waiting_room_guard:
    push {r4-r7, lr}
    ldrh r6, [r5, #0x34]            ; incoming Waiting Room slot (1..7 valid)
    cmp  r6, #0
    beq  @@bad_slot
    cmp  r6, #7
    bhi  @@bad_slot
    ldr  r7, [r5, #0x38]            ; original instructions at +4..+6
    ldrh r1, [r7, #0x0A]
    ldr  r3, =(WAITING_ROOM_RECEIVE + 8) | 1
    bx   r3
@@bad_slot:
    mov  r0, #1                     ; error status -> caller aborts without saving
    pop  {r4-r7, pc}
    .pool

.if . > FS_WAITING_ROOM_GUARD + FS_WAITING_ROOM_GUARD_SIZE
    .error "FS_WAITING_ROOM_GUARD overflow"
.endif
