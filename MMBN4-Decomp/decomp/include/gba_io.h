/* GBA hardware MMIO register definitions (Phase C1). */

#ifndef MMBN4_GBA_IO_H
#define MMBN4_GBA_IO_H

#include "gba_types.h"

#define REG_BASE       0x04000000

/* LCD Control & Status */
#define REG_DISPCNT    (*(vu16 *)(REG_BASE + 0x000))
#define REG_DISPSTAT   (*(vu16 *)(REG_BASE + 0x004))
#define REG_VCOUNT     (*(vu16 *)(REG_BASE + 0x006))
#define REG_BG0CNT     (*(vu16 *)(REG_BASE + 0x008))
#define REG_BG1CNT     (*(vu16 *)(REG_BASE + 0x00A))
#define REG_BG2CNT     (*(vu16 *)(REG_BASE + 0x00C))
#define REG_BG3CNT     (*(vu16 *)(REG_BASE + 0x00E))
#define REG_BLDCNT     (*(vu16 *)(REG_BASE + 0x050))
#define REG_BLDALPHA   (*(vu16 *)(REG_BASE + 0x052))
#define REG_BLDY       (*(vu16 *)(REG_BASE + 0x054))

/* Sound (CGB + DirectSound) */
#define REG_NR10       (*(vu8  *)(REG_BASE + 0x060))
#define REG_NR11       (*(vu8  *)(REG_BASE + 0x062))
#define REG_NR12       (*(vu8  *)(REG_BASE + 0x063))
#define REG_NR13       (*(vu8  *)(REG_BASE + 0x064))
#define REG_NR14       (*(vu8  *)(REG_BASE + 0x065))
#define REG_NR21       (*(vu8  *)(REG_BASE + 0x068))
#define REG_NR22       (*(vu8  *)(REG_BASE + 0x069))
#define REG_NR23       (*(vu8  *)(REG_BASE + 0x06C))
#define REG_NR24       (*(vu8  *)(REG_BASE + 0x06D))
#define REG_NR30       (*(vu8  *)(REG_BASE + 0x070))
#define REG_NR31       (*(vu8  *)(REG_BASE + 0x072))
#define REG_NR32       (*(vu8  *)(REG_BASE + 0x073))
#define REG_NR33       (*(vu8  *)(REG_BASE + 0x074))
#define REG_NR34       (*(vu8  *)(REG_BASE + 0x075))
#define REG_NR41       (*(vu8  *)(REG_BASE + 0x078))
#define REG_NR42       (*(vu8  *)(REG_BASE + 0x079))
#define REG_NR43       (*(vu8  *)(REG_BASE + 0x07C))
#define REG_NR44       (*(vu8  *)(REG_BASE + 0x07D))
#define REG_SOUNDCNT_L (*(vu16 *)(REG_BASE + 0x080))
#define REG_NR51       (*(vu8  *)(REG_BASE + 0x081))
#define REG_SOUNDCNT_H (*(vu16 *)(REG_BASE + 0x082))
#define REG_SOUNDCNT_X (*(vu16 *)(REG_BASE + 0x084))
#define REG_SOUNDBIAS  (*(vu16 *)(REG_BASE + 0x088))
#define REG_SOUNDBIAS_H (*(vu8 *)(REG_BASE + 0x089))
#define REG_WAVE_RAM0  ((vu32  *)(REG_BASE + 0x090))
#define REG_FIFO_A     (*(vu32 *)(REG_BASE + 0x0A0))
#define REG_FIFO_B     (*(vu32 *)(REG_BASE + 0x0A4))

/* DMA */
#define REG_DMA0SAD    (*(vu32 *)(REG_BASE + 0x0B0))
#define REG_DMA0DAD    (*(vu32 *)(REG_BASE + 0x0B4))
#define REG_DMA0CNT    (*(vu32 *)(REG_BASE + 0x0B8))
#define REG_DMA1SAD    (*(vu32 *)(REG_BASE + 0x0BC))
#define REG_DMA1DAD    (*(vu32 *)(REG_BASE + 0x0C0))
#define REG_DMA1CNT    (*(vu32 *)(REG_BASE + 0x0C4))
#define REG_DMA2SAD    (*(vu32 *)(REG_BASE + 0x0C8))
#define REG_DMA2DAD    (*(vu32 *)(REG_BASE + 0x0CC))
#define REG_DMA2CNT    (*(vu32 *)(REG_BASE + 0x0D0))
#define REG_DMA3SAD    (*(vu32 *)(REG_BASE + 0x0D4))
#define REG_DMA3DAD    (*(vu32 *)(REG_BASE + 0x0D8))
#define REG_DMA3CNT    (*(vu32 *)(REG_BASE + 0x0DC))

/* Timers */
#define REG_TM0CNT_L   (*(vu16 *)(REG_BASE + 0x100))
#define REG_TM0CNT_H   (*(vu16 *)(REG_BASE + 0x102))
#define REG_TM1CNT_L   (*(vu16 *)(REG_BASE + 0x104))
#define REG_TM1CNT_H   (*(vu16 *)(REG_BASE + 0x106))

/* Keypad */
#define REG_KEYINPUT   (*(vu16 *)(REG_BASE + 0x130))
#define REG_KEYCNT     (*(vu16 *)(REG_BASE + 0x132))

#define KEY_A          (1u << 0)
#define KEY_B          (1u << 1)
#define KEY_SELECT     (1u << 2)
#define KEY_START      (1u << 3)
#define KEY_RIGHT      (1u << 4)
#define KEY_LEFT       (1u << 5)
#define KEY_UP         (1u << 6)
#define KEY_DOWN       (1u << 7)
#define KEY_R          (1u << 8)
#define KEY_L          (1u << 9)

/* Interrupts & Waitstate Control */
#define REG_IE         (*(vu16 *)(REG_BASE + 0x200))
#define REG_IF         (*(vu16 *)(REG_BASE + 0x202))
#define REG_WAITCNT    (*(vu16 *)(REG_BASE + 0x204))
#define REG_IME        (*(vu16 *)(REG_BASE + 0x208))

/* IWRAM IRQ vector pointer */
#define INTR_VECTOR    (*(vu32 *)0x03007FFC)

#endif /* MMBN4_GBA_IO_H */
