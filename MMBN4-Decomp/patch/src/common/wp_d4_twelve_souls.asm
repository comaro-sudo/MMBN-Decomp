; ============================================================
; WP-D4 step 1: all twelve Double Souls can unite, in both versions.
;
; BN4_SOUL_FLAGS gives each Soul's event flag. Each version writes 0xFF ("refused") for the other version's
; six Souls: RS 17 18 19 1A 1B 1C FF.., BM FF.. 1D 1E 1F 20 21 22. The unite code, the twelve forms' tables
; and their sprites are identical in both ROMs, so filling the table is enough for a Soul whose flag and
; pseudo-chip key are set (as trading Navi Soul data sets them).
; Verified 2026-11-22: Red Sun + a save holding Blue Moon's Navi data unites WoodSoul and fights in it
; (docs/re/soul-merge-experiment.md). The grant itself (getting the flags without a trade) is the next step.
; ============================================================
.org SOUL_FLAGS
    .byte 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C    ; Roll, Guts, Wind, Search, Fire, Thunder (Red Sun's)
    .byte 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22    ; Proto, Number, Metal, Junk, Aqua, Wood (Blue Moon's)
