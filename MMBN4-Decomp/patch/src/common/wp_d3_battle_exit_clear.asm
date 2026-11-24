; ============================================================
; WP-D3: clear the per-battle EWRAM bytes on every battle exit (win, loss, flee, game over).
; Design: Gemini, decomp/notes/battle-exit-hook.md (round 11). BattleExitCleanup (sub_08006E1E, same in
; RS and BM) runs once per battle, after BattleResultsGrantReward. Its first call (bl sub_080006D4 at
; 0x08006E20; r0-r3 scratch) goes through a veneer to a routine that zeroes 0x020015EC-0x020015EF in one
; word store, then tail-calls sub_080006D4:
;   +0 roaming SP enemy id, +1 SP+ speed phase, +2 SP+ active (SP+ is still to build), +3 rematch marker.
; Fixes the stale marker after a lost or fled rematch (patch-test-results-4.md).
; ============================================================

.org 0x08006E20
    bl      VENEER_BATTLE_EXIT

.org VENEER_BATTLE_EXIT             ; 0x0800A144 (sub_0800A144 dead-code veneer pool, RS == BM)
    ldr     r3, [r15, 0]
    bx      r3
    .word   battle_exit_clear | 1

.org FS_BATTLE_EXIT
battle_exit_clear:
    ldr     r0, =0x020015EC
    mov     r1, 0
    str     r1, [r0]
    ldr     r3, =0x080006D4 | 1     ; sub_080006D4 (same in RS and BM); lr is still our caller's
    bx      r3
    .pool
.if . > FS_BATTLE_EXIT + FS_BATTLE_EXIT_SIZE
    .error "WP-D3 battle exit clear: overruns its free-space slot"
.endif
