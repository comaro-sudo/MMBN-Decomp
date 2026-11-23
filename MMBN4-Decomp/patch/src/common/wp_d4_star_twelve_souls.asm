; ============================================================
; WP-D4 step 2: the "all Souls" completion star needs all TWELVE Souls, in both versions.
;
; GetCompletionStars (0x08000D28, same address in RS and BM) awards its Souls star when every flag in
; BN4_STAR_SOUL_FLAGS (0x08000DB4) is set. That zero-terminated u16 list holds only the version's own six
; (RS 0x17-0x1C, BM 0x1D-0x22). Its one literal, at 0x08000DB0, is pointed at a new 12-entry list in free
; space (docs/re/free-space.md): flags 0x17-0x22, the same order as BN4_SOUL_FLAGS.
; Pairs with wp_d4_twelve_souls.asm (all 12 can unite). The status screen's 6-entry Soul list
; (BN4_STATUS_SOUL_FLAGS) is not changed here: its screen layout must be checked first.
; ============================================================

.org STAR_SOUL_FLAGS_PTR            ; the literal GetCompletionStars loads
    .word FS_STAR_SOUL_FLAGS

.org FS_STAR_SOUL_FLAGS
    .halfword 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C    ; Roll, Guts, Wind, Search, Fire, Thunder
    .halfword 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22    ; Proto, Number, Metal, Junk, Aqua, Wood
    .halfword 0                                     ; end
.if . > FS_STAR_SOUL_FLAGS + FS_STAR_SOUL_FLAGS_SIZE
    .error "WP-D4 star: list overruns its free-space slot"
.endif
