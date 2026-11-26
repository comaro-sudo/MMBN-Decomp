/*
 * Portable C OAM sprite & DMA queue helpers (Phase C2).
 *
 * Models MMBN4's GBA OBJ attribute packing and deferred DMA transfer queue
 * (code_00) so sprite/DMA logic compiles natively on host & console ports.
 */
#include "gba_io.h"

#define OAM_OBJ_COUNT 128u
#define DMA_QUEUE_CAP 32u

struct OamAttr {
    u16 attr0;
    u16 attr1;
    u16 attr2;
    s16 affine;
};

struct DmaQueueEntry {
    const void *src;
    void *dst;
    u32 control; /* count + mode flags (DMA_32BIT, DMA_ENABLE, etc.) */
};

struct DmaQueue {
    struct DmaQueueEntry entries[DMA_QUEUE_CAP];
    u32 count;
};

/*
 * Hide all 128 OAM entries by setting OBJ mode bits [9:8] = 0b10 (disabled)
 * in attr0 (0x0200).
 */
void OamClearBuffer(struct OamAttr *oam, u32 count) {
    u32 i;
    if (count > OAM_OBJ_COUNT) {
        count = OAM_OBJ_COUNT;
    }
    for (i = 0; i < count; i++) {
        oam[i].attr0 = 0x0200u;
        oam[i].attr1 = 0u;
        oam[i].attr2 = 0u;
    }
}

/*
 * Pack a standard non-affine 16-color (4bpp) sprite into an OamAttr entry.
 *   x: 9-bit signed screen X (-256..255)
 *   y: 8-bit screen Y (0..255)
 *   shape: 0=Square, 1=Wide, 2=Tall
 *   size:  0..3
 *   tile:  10-bit base tile index (0..1023)
 *   pal:   4-bit palette bank (0..15)
 *   prio:  2-bit BG priority (0..3)
 */
void OamSetSprite(struct OamAttr *entry, s16 x, s16 y, u8 shape, u8 size,
                  u16 tile, u8 pal, u8 prio, u8 hFlip, u8 vFlip) {
    entry->attr0 = (u16)(((u16)y & 0x00FFu) | (((u16)shape & 3u) << 14));
    entry->attr1 = (u16)(((u16)x & 0x01FFu)
                       | (hFlip ? (1u << 12) : 0u)
                       | (vFlip ? (1u << 13) : 0u)
                       | (((u16)size & 3u) << 14));
    entry->attr2 = (u16)((tile & 0x03FFu)
                       | (((u16)prio & 3u) << 10)
                       | (((u16)pal & 0x0Fu) << 12));
}

/*
 * Reset the deferred DMA queue.
 */
void DmaQueueReset(struct DmaQueue *q) {
    q->count = 0u;
}

/*
 * Enqueue a deferred DMA transfer. Returns 1 on success, 0 if queue is full.
 */
u32 DmaQueuePush(struct DmaQueue *q, const void *src, void *dst, u32 control) {
    if (q->count >= DMA_QUEUE_CAP) {
        return 0;
    }
    q->entries[q->count].src = src;
    q->entries[q->count].dst = dst;
    q->entries[q->count].control = control;
    q->count++;
    return 1;
}
