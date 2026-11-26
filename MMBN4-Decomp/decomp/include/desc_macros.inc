@ Assembler macros for MMBN4 pointer, DMA/VRAM transfer, and tile/palette
@ animation frame descriptor tables across code_00..code_10.

.ifndef DESC_MACROS_INC
.set DESC_MACROS_INC, 1

.macro ptr_v rs_ptr, bm_ptr
    .if VERSION_BM
    .4byte \bm_ptr
    .else
    .4byte \rs_ptr
    .endif
.endm

.macro dma_desc dst, ctrl, src
    .4byte \dst, \ctrl, \src
.endm

.macro dma_desc_v dst, ctrl, rs_src, bm_src
    .4byte \dst, \ctrl
    .if VERSION_BM
    .4byte \bm_src
    .else
    .4byte \rs_src
    .endif
.endm

.macro anim_frame val, src
    .4byte \val, \src
.endm

.macro anim_frame_v val, rs_src, bm_src
    .4byte \val
    .if VERSION_BM
    .4byte \bm_src
    .else
    .4byte \rs_src
    .endif
.endm

.endif
