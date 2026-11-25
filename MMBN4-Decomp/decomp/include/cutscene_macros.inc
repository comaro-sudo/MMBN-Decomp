@ Assembler macros for MMBN4 code_04..code_05 map-init & cutscene dispatch bytecode
@ (see decomp/notes/re-cutscene-bytecode.md).
@ Uses .4byte so unaligned 32-bit relocatable ROM targets assemble without padding.

.ifndef CUTSCENE_MACROS_INC
.set CUTSCENE_MACROS_INC, 1

.macro cs_range lo, hi, target
    .byte 0x02, \lo, \hi
    .4byte \target
.endm

.macro cs_jump target
    .byte 0x01
    .4byte \target
.endm

.macro cs_end
    .byte 0xFF
.endm

.endif
