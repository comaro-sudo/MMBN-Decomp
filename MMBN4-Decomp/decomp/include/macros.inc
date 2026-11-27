@ Shared macros and version constants for the MMBN4 disassembly.

@ Version-dependent constants shared across RS and BM source files.
    .set VERSION_INDEX, VERSION_BM
    .if VERSION_BM
        .set SAVE_CHECKSUM_KEY,     0x22
        .set SAVE_XOR_KEY_0,        0x71
        .set SAVE_XOR_KEY_1,        0x47
        .set SAVE_XOR_KEY_2,        0x8E
        .set SAVE_XOR_KEY_3,        0x31
        .set FIRST_SOUL_FLAG,       0x1D
        .set NGPLUS_POOL_SOUL_FLAG, 0x21
    .else
        .set SAVE_CHECKSUM_KEY,     0x16
        .set SAVE_XOR_KEY_0,        0x2C
        .set SAVE_XOR_KEY_1,        0x64
        .set SAVE_XOR_KEY_2,        0x38
        .set SAVE_XOR_KEY_3,        0x43
        .set FIRST_SOUL_FLAG,       0x17
        .set NGPLUS_POOL_SOUL_FLAG, 0x18
    .endif

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

    .include "include/data_macros.inc"   @ decoded-table macros (Claude)
