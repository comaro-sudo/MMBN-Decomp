/*
 * Portable C GBA BIOS LZ77 decompressor (Phase C2).
 *
 * Mirrors GBA SWI 0x11 (LZ77UnCompWram) / 0x12 (LZ77UnCompVram) used
 * throughout MMBN4 for sprites, tilesets, maps, and Violet Eclipse's
 * on-demand sprite decompression.
 */
#include "gba_types.h"

#define LZ77_MAGIC_TYPE 0x10u

/*
 * Read the uncompressed size (24-bit) from a GBA LZ77 header.
 * Returns 0 if the header magic is not 0x10 or input is too short.
 */
u32 Lz77GetUncompressedSize(const u8 *src, u32 srcLen) {
    if (src == 0 || srcLen < 4u) {
        return 0;
    }
    if (src[0] != LZ77_MAGIC_TYPE) {
        return 0;
    }
    return (u32)src[1] | ((u32)src[2] << 8) | ((u32)src[3] << 16);
}

/*
 * Decompress a GBA type-0x10 LZ77 stream from src[0..srcLen) into
 * dst[0..dstCap). Returns the number of bytes written on success, or 0 on
 * invalid header, truncated input, invalid back-reference, or insufficient
 * output capacity.
 */
u32 Lz77Decompress(const u8 *src, u32 srcLen, u8 *dst, u32 dstCap) {
    u32 outSize;
    u32 srcPos = 4u;
    u32 dstPos = 0u;

    outSize = Lz77GetUncompressedSize(src, srcLen);
    if (outSize == 0u || dst == 0 || dstCap < outSize) {
        return 0;
    }

    while (dstPos < outSize) {
        u8 flags;
        u32 bit;

        if (srcPos >= srcLen) {
            return 0;
        }
        flags = src[srcPos++];

        for (bit = 0; bit < 8u && dstPos < outSize; bit++) {
            if ((flags & (0x80u >> bit)) == 0u) {
                if (srcPos >= srcLen) {
                    return 0;
                }
                dst[dstPos++] = src[srcPos++];
            } else {
                u8 b1, b2;
                u32 disp, count, i;

                if (srcPos + 1u >= srcLen) {
                    return 0;
                }
                b1 = src[srcPos++];
                b2 = src[srcPos++];
                count = (u32)((b1 >> 4) & 0x0Fu) + 3u;
                disp = (u32)(((u32)(b1 & 0x0Fu) << 8) | (u32)b2) + 1u;

                if (disp > dstPos) {
                    return 0;
                }
                for (i = 0; i < count && dstPos < outSize; i++) {
                    dst[dstPos] = dst[dstPos - disp];
                    dstPos++;
                }
            }
        }
    }

    return dstPos;
}
