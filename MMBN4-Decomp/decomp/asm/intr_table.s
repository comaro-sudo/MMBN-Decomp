@ intr_table: RS 0x0800022C-0x0800025C, BM 0x0800022C-0x0800025C (shared source, tools/merge_bm.py)
@ Initial GBA interrupt handler table copied to IWRAM at boot (12 IRQ slots).

    .set SEG_BM_DELTA, 0x0   @ Blue Moon = RS + this, inside this segment

    .global dat_0800022C
dat_0800022C:   @ gIntrTableTemplate
    .word 0x0300593F   @ IRQ 0: VBlank (IWRAM Thumb handler)
    .word 0x0300593D   @ IRQ 1: HBlank (IWRAM empty handler)
    .word 0x0300593D   @ IRQ 2: VCount
    .word 0x0300593D   @ IRQ 3: Timer 0
    .word 0x0300593D   @ IRQ 4: Timer 1
    .word 0x0300593D   @ IRQ 5: Timer 2
    .word 0x0300593D   @ IRQ 6: Timer 3
    .word 0x0300593D   @ IRQ 7: Serial
    .word 0x0300593D   @ IRQ 8: DMA 0
    .word 0x0300593D   @ IRQ 9: DMA 1
    .word 0x0300593D   @ IRQ 10: DMA 2
    .word 0x0300593D   @ IRQ 11: DMA 3
