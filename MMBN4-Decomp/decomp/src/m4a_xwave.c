/* DC-5 C (old_agbcc -O2): Nintendo m4a ply_xwave
 * (code_10, RS 0x08113BC8-0x08113C0F, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

struct MPlayTrack {
    u8 pad0[0x28];
    u32 xwave;
    u8 pad2C[0x14];
    u8 *cmdPtr;
};

#define READ_XCMD_BYTE(var, n)         \
{                                      \
    u32 byte = track->cmdPtr[(n)];     \
    byte <<= n * 8;                    \
    var &= ~(0xFF << (n * 8));         \
    var |= byte;                       \
}

/* 0x08113BC8: ply_xwave */
void sub_08113BC8(void *mplayInfo, struct MPlayTrack *track) {
    u32 wav;

    READ_XCMD_BYTE(wav, 0);
    READ_XCMD_BYTE(wav, 1);
    READ_XCMD_BYTE(wav, 2);
    READ_XCMD_BYTE(wav, 3);
    track->xwave = wav;
    track->cmdPtr += 4;
}
