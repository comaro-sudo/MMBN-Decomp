/* DC-5 C (agbcc -O2): Nintendo m4a MusicPlayer immediate track init
 * (code_10, RS 0x08112A38-0x08112A7F, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define MPT_FLG_EXIST 0x80
#define MPT_FLG_START 0x40

struct MusicPlayerTrack {
    u8 flags;           /* +0x00 */
    u8 pad01[0x0E];
    u8 bendRange;       /* +0x0F */
    u8 pad10[0x03];
    u8 volX;            /* +0x13 */
    u8 pad14[0x05];
    u8 lfoSpeed;        /* +0x19 */
    u8 pad1A[0x0A];
    u8 tone;            /* +0x24 */
    u8 pad25[0x2B];
};

struct MusicPlayerInfo {
    const void *songHeader;          /* +0x00 */
    u32 status;                      /* +0x04 */
    u8 trackCount;                   /* +0x08 */
    u8 pad09[0x23];
    struct MusicPlayerTrack *tracks; /* +0x2C */
};

void sub_08112BB0(void *p); /* Clear64byte */

/* 0x08112A38: m4aMPlayImmInit */
void sub_08112A38(struct MusicPlayerInfo *mplayInfo) {
    s32 trackCount = mplayInfo->trackCount;
    struct MusicPlayerTrack *track = mplayInfo->tracks;

    while (trackCount > 0) {
        if (track->flags & MPT_FLG_EXIST) {
            if (track->flags & MPT_FLG_START) {
                sub_08112BB0(track);
                track->flags = MPT_FLG_EXIST;
                track->bendRange = 2;
                track->volX = 64;
                track->lfoSpeed = 22;
                track->tone = 1;
            }
        }

        trackCount--;
        track++;
    }
}
