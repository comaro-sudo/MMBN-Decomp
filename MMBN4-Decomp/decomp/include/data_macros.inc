@ Data-table macros for the decoded tables (Claude, 2026-11-27). Loaded once, by include/macros.inc,
@ before every segment: GNU as needs a macro defined before its first use, and rom.s assembles
@ the segments in address order. Used by the decomp/tools/emit_*.py table emitters.

.macro chip_code id, code
    .hword (\id) | ((\code) << 9)
.endm

.macro formation id, col, row, kind
    .byte \id, \col, \row, \kind
.endm

.macro formation_end
    .byte 0xFF
.endm

.macro chip_codes id, c0, c1, c2, c3
    .hword \id
    .byte \c0, \c1, \c2, \c3
.endm

.macro event_spawn x, y, z, unk, params
    .hword \x, \y, \z, \unk
    .word \params
.endm

.macro event_spawn_end
    .word 0, 0, 0
.endm

.macro flag_prop list, sprite, anim, pal, attr, test, flag_a, flag_b, unk
    .byte \list, \sprite, \anim, \pal
    .word \attr
    .hword \test, \flag_a, \flag_b, \unk
.endm

.macro obj_spawn kind, arg, unk, x, y, z, param
    .byte \kind, \arg
    .hword \unk
    .word \x, \y, \z, \param
.endm

.macro obj_spawn_end
    .word 0xFF
.endm
