@ Assembler macros for MMBN4 NpcRunScript bytecode (sub_080A8598,
@ jump table dat_080A85D0 in asm/code_06.s).
@ Uses .2byte/.4byte so unaligned operands and relocatable ROM targets
@ assemble without automatic padding.

.ifndef NPC_MACROS_INC
.set NPC_MACROS_INC, 1

.macro npc_end
    .byte 0x00
.endm

.macro npc_jump target
    .byte 0x02
    .4byte \target
.endm

.macro npc_hide
    .byte 0x03
.endm

.macro npc_jump_if_flag_set flag, target
    .byte 0x04
    .2byte \flag
    .4byte \target
.endm

.macro npc_jump_if_flag_clear flag, target
    .byte 0x05
    .2byte \flag
    .4byte \target
.endm

.macro npc_set_flag flag
    .byte 0x06
    .2byte \flag
.endm

.macro npc_clear_flag flag
    .byte 0x07
    .2byte \flag
.endm

.macro npc_set_pos x, y, z
    .byte 0x14
    .2byte \x, \y, \z
.endm

.macro npc_set_facing dir
    .byte 0x16, \dir
.endm

.macro npc_set_sprite id
    .byte 0x17, \id
.endm

.macro npc_set_talk idx
    .byte 0x18, \idx
.endm

.macro npc_stop
    .byte 0x34
.endm

.macro npc_call target
    .byte 0x36
    .4byte \target
.endm

.macro npc_call_ex b1, b2, b3, target
    .byte 0x38, \b1, \b2, \b3
    .4byte \target
.endm

.macro npc_ret
    .byte 0x3A
.endm

.macro npc_jump_if_phase lo, hi, target
    .byte 0x3B, \lo, \hi
    .4byte \target
.endm

.macro npc_jump_if_tourney_eq t, r, target
    .byte 0x42, \t, \r
    .4byte \target
.endm

.macro npc_jump_if_tourney_ne t, r, target
    .byte 0x43, \t, \r
    .4byte \target
.endm

@ Generic NpcRunScript opcode macros by operand shape
.macro npc_op op
    .byte \op
.endm

.macro npc_b1 op, a
    .byte \op, \a
.endm

.macro npc_b2 op, a, b
    .byte \op, \a, \b
.endm

.macro npc_b3 op, a, b, c
    .byte \op, \a, \b, \c
.endm

.macro npc_ptr op, p1
    .byte \op
    .4byte \p1
.endm

.macro npc_b1_ptr op, a, p1
    .byte \op, \a
    .4byte \p1
.endm

.macro npc_b2_ptr op, a, b, p1
    .byte \op, \a, \b
    .4byte \p1
.endm

.macro npc_b4_ptr op, a, b, c, d, p1
    .byte \op, \a, \b, \c, \d
    .4byte \p1
.endm

.endif
