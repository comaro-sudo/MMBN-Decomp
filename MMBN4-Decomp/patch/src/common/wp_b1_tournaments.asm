; ============================================================
; WP-B1 / Violet Eclipse: deterministic story tournaments & canon Soul Navi lineup.
; Design: decomp/notes/wp-b1-design.md & canon-navis.md section 6 (owner decision 2026-11-23).
;
; Unified canon story fight per tournament (both Red Sun and Blue Moon):
;   Tournament 0 (Den Battle):       AquaMan  (0x03)   [pair: 03 00]
;   Tournament 1 (Eagle / Hawk):     Roll     (0x04)   [pair: 04 06]
;   Tournament 2 (RS / BM):          ProtoMan (0x0A)   [pair: 0A 09]
;
; Patch points:
; 1. BN4_TOURNAMENT_SOUL_PAIRS: set slot +0 of each tournament to the canon Navi.
; 2. TournamentPickSoulPair: bypass the flag-0x15 GetTertiaryRNG coin flip so pool[0] is always pair[0].
; 3. TournamentCheckSoulPairSwap: return non-zero (Z=0) so Playthrough 2 never swaps pool[0] <-> pool[1].
; 4. BN4_DRAW_BRACKET: bypass GetTertiaryRNG seeding so pool[0] (canon Navi) always enters MegaMan's path.
; 5. Red Sun Den Battle tutorial (VER_RS): grant AquaSoul (0x21 / key 0x16B) instead of GutsSoul (matching BM).
; ============================================================

; 1. Unified canon Soul Navi pairs (slot +0 = canon Navi)
.org TOURN_SOUL_PAIRS
    .byte 0x03, 0x00                ; Tournament 0: AquaMan (canon), GutsMan
    .byte 0x04, 0x06                ; Tournament 1: Roll    (canon), MetalMan
    .byte 0x0A, 0x09                ; Tournament 2: ProtoMan(canon), SearchMan

; 2. Disable GetTertiaryRNG pair coin flip in TournamentPickSoulPair (keep r0 = 0 -> pair[0] in pool[0])
.org TOURN_PAIR_RNG
    mov     r0, 0
    b       TOURN_PAIR_NO_SWAP
    mov     r8, r8

; 3. Disable Playthrough-2 pair swap in TournamentCheckSoulPairSwap (caller skips swap on Z == 0)
.org TOURN_PAIR_SWAP
    mov     r0, 1
    tst     r0, r0
    bx      r14

; 4. Deterministic bracket draw in BN4_DRAW_BRACKET (always place pool[0] on MegaMan's side)
.org TOURN_BRACKET_RNG
    b       TOURN_BRACKET_DET

; 5. Red Sun Den Battle Double Soul tutorial: grant AquaSoul (0x21, key 0x16B) matching Blue Moon
.if VER_RS
.org RS_TUTORIAL_SOUL
    .byte 0x1C, 0xFF, 0x21, 0x00    ; SetEventFlag(0x0021) AquaSoul
    .byte 0x3F, 0x6B, 0x01          ; WriteChipKey(0x016B) AquaSoul key
.endif
