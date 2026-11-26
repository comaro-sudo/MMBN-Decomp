@ Assembler macros for MMBN4 code_04..code_05 map-init & cutscene bytecode
@ (interpreter: dat_0802A7D0 map_scripts_commands in asm/code_02.s;
@  see decomp/notes/re-cutscene-bytecode.md).
@ Uses .2byte/.4byte so unaligned operands and relocatable ROM targets
@ assemble without automatic padding.

.ifndef CUTSCENE_MACROS_INC
.set CUTSCENE_MACROS_INC, 1

@ --- Story-progress dispatch table macros ---
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

@ --- Map-init / event action bytecode macros (dat_0802A7D0) ---
.macro ms_end
    .byte 0x00
.endm

.macro ms_jump target
    .byte 0x01
    .4byte \target
.endm

.macro ms_jump_if_flag_set bank, flag, target
    .byte 0x03, \bank
    .2byte \flag
    .4byte \target
.endm

.macro ms_jump_if_flag_clear bank, flag, target
    .byte 0x04, \bank
    .2byte \flag
    .4byte \target
.endm

.macro ms_jump_if_flag2_set bank, flag, target
    .byte 0x05, \bank
    .2byte \flag
    .4byte \target
.endm

.macro ms_jump_if_flag2_clear bank, flag, target
    .byte 0x06, \bank
    .2byte \flag
    .4byte \target
.endm

.macro ms_jump_if_item item, lo, hi, target
    .byte 0x08, \item, \lo, \hi
    .4byte \target
.endm

.macro ms_jump_if_chip chip, code, lo, hi, target
    .byte 0x09
    .2byte \chip
    .byte \code, \lo, \hi
    .4byte \target
.endm

.macro ms_jump_if_u8 op, val, target
    .byte \op, \val
    .4byte \target
.endm

.macro ms_jump_if_u16 op, val, target
    .byte \op
    .2byte \val
    .4byte \target
.endm

.macro ms_jump_if_u8_pair op, a, b, target
    .byte \op, \a, \b
    .4byte \target
.endm

.macro ms_set_flag bank, flag
    .byte 0x1C, \bank
    .2byte \flag
.endm

.macro ms_clear_flag bank, flag
    .byte 0x1D, \bank
    .2byte \flag
.endm

.macro ms_set_flag2 bank, flag
    .byte 0x1E, \bank
    .2byte \flag
.endm

.macro ms_clear_flag2 bank, flag
    .byte 0x1F, \bank
    .2byte \flag
.endm

.macro ms_set_ptr_pair ptr1, ptr2
    .byte 0x22
    .4byte \ptr1
    .4byte \ptr2
.endm

.macro ms_start_cutscene script, arg
    .byte 0x23
    .4byte \script
    .4byte \arg
.endm

.macro ms_write_u8 addr, val
    .byte 0x24
    .4byte \addr
    .byte \val
.endm

.macro ms_write_u16 addr, val
    .byte 0x25
    .4byte \addr
    .2byte \val
.endm

.macro ms_write_u32 addr, val
    .byte 0x26
    .4byte \addr
    .4byte \val
.endm

.macro ms_ptr op, ptr
    .byte \op
    .4byte \ptr
.endm

.macro ms_ptr3 op, p1, p2, p3
    .byte \op
    .4byte \p1
    .4byte \p2
    .4byte \p3
.endm

.macro ms_clear_cont
    .byte 0x36, 0x01
.endm

.macro ms_set_cont mode, ptr
    .byte 0x36, \mode
    .4byte \ptr
.endm

.macro ms_op op
    .byte \op
.endm

.macro ms_u8 op, a
    .byte \op, \a
.endm

.macro ms_u8x2 op, a, b
    .byte \op, \a, \b
.endm

.macro ms_u8x3 op, a, b, c
    .byte \op, \a, \b, \c
.endm

.macro ms_u16 op, val
    .byte \op
    .2byte \val
.endm

.macro ms_u16_u8 op, val, a
    .byte \op
    .2byte \val
    .byte \a
.endm

.macro ms_u16_u8x2 op, val, a, b
    .byte \op
    .2byte \val
    .byte \a, \b
.endm

.macro ms_3d_u8 val
    .byte 0x3D, 0x01, \val
.endm

.macro ms_3d_ptr mode, ptr
    .byte 0x3D, \mode
    .4byte \ptr
.endm

@ --- Cutscene script bytecode macros (dat_0802C140 BN4_CUTSCENE_COMMANDS) ---
.macro csc_end
    .byte 0x00
.endm

.macro csc_end_alt
    .byte 0x01
.endm

.macro csc_thread_end
    .byte 0x10
.endm

.macro csc_spawn slot, target
    .byte 0x11, \slot
    .4byte \target
.endm

.macro csc_jump target
    .byte 0x14
    .4byte \target
.endm

.macro csc_op op
    .byte \op
.endm

.macro csc_b1 op, a
    .byte \op, \a
.endm

.macro csc_b2 op, a, b
    .byte \op, \a, \b
.endm

.macro csc_b3 op, a, b, c
    .byte \op, \a, \b, \c
.endm

.macro csc_b4 op, a, b, c, d
    .byte \op, \a, \b, \c, \d
.endm

.macro csc_b5 op, a, b, c, d, e
    .byte \op, \a, \b, \c, \d, \e
.endm

.macro csc_b9 op, a, b, c, d, e, f, g, h, i
    .byte \op, \a, \b, \c, \d, \e, \f, \g, \h, \i
.endm

.macro csc_ptr op, p1
    .byte \op
    .4byte \p1
.endm

.macro csc_b1_ptr op, a, p1
    .byte \op, \a
    .4byte \p1
.endm

.macro csc_b2_ptr op, a, b, p1
    .byte \op, \a, \b
    .4byte \p1
.endm

.macro csc_b3_ptr op, a, b, c, p1
    .byte \op, \a, \b, \c
    .4byte \p1
.endm

.macro csc_b5_ptr op, a, b, c, d, e, p1
    .byte \op, \a, \b, \c, \d, \e
    .4byte \p1
.endm

.macro csc_ptr_b1 op, p1, a
    .byte \op
    .4byte \p1
    .byte \a
.endm

.macro csc_ptr_b2 op, p1, a, b
    .byte \op
    .4byte \p1
    .byte \a, \b
.endm

.macro csc_ptr2 op, p1, p2
    .byte \op
    .4byte \p1
    .4byte \p2
.endm

.macro csc_ptr3 op, p1, p2, p3
    .byte \op
    .4byte \p1
    .4byte \p2
    .4byte \p3
.endm

.macro csc_45_13 b1, b2, x, y, z, p1
    .byte 0x45, \b1, \b2
    .2byte \x, \y, \z
    .4byte \p1
.endm

.macro csc_45_14 b1, b2, b3, x, y, z, p1
    .byte 0x45, \b1, \b2, \b3
    .2byte \x, \y, \z
    .4byte \p1
.endm

@ Version-specific ROM archive pointers (e.g. 0x3A compressed text archives
@ in 0x887Bxxxx..0x887Fxxxx whose BM offsets vary with dialogue size).
.macro csc_ptr_v op, rs_ptr, bm_ptr
    .byte \op
    .if VERSION_BM
    .4byte \bm_ptr
    .else
    .4byte \rs_ptr
    .endif
.endm

.macro csc_b1_ptr_v op, a, rs_ptr, bm_ptr
    .byte \op, \a
    .if VERSION_BM
    .4byte \bm_ptr
    .else
    .4byte \rs_ptr
    .endif
.endm

@ 0x1A: compare RAM u8/u16/u32 at addr against imm, jump to target if equal.
.macro csc_1a_u8 addr, target, val
    .byte 0x1A, 0x00
    .4byte \addr, \target
    .byte \val
.endm

.macro csc_1a_u16 addr, target, val
    .byte 0x1A, 0x01
    .4byte \addr, \target
    .2byte \val
.endm

.macro csc_1a_u32 addr, target, val
    .byte 0x1A, 0x02
    .4byte \addr, \target, \val
.endm

.endif
