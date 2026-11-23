@ Shared macros for the MMBN4 disassembly.

@ incrom OFF, SIZE — include SIZE bytes of the player's ROM.
@ OFF is the Red Sun offset; a Blue Moon build reads at OFF + SEG_BM_DELTA,
@ the constant RS->BM shift of the segment being assembled (each segment
@ file sets it; see config/segments_rs.toml and notes/dc1-survey.md).
    .macro incrom off, size
    .if VERSION_BM
        .incbin "baserom.gba", (\off) + SEG_BM_DELTA, \size
    .else
        .incbin "baserom.gba", \off, \size
    .endif
    .endm
