/* DC-5 C (old_agbcc -O2): Nintendo m4a MusicPlayer LFO & modulation control
 * (code_10, RS 0x08113954-0x08113A3B, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define ID_NUMBER     0x68736D53
#define MPT_FLG_EXIST 0x80

struct MusicPlayerTrack {
    u8 flags;           /* +0x00 */
    u8 pad01[0x16];
    u8 lfoSpeed;        /* +0x17 */
    u8 modT;            /* +0x18 */
    u8 mod;             /* +0x19 */
    u8 lfoSpeedC;       /* +0x1A */
    u8 pad1B[0x35];
};

struct MusicPlayerInfo {
    const void *songHeader;          /* +0x00 */
    u32 status;                      /* +0x04 */
    u8 trackCount;                   /* +0x08 */
    u8 pad09[0x23];
    struct MusicPlayerTrack *tracks; /* +0x2C */
    void *tone;                      /* +0x30 */
    u32 ident;                       /* +0x34 */
};

void sub_08113934(struct MusicPlayerTrack *track); /* ClearModM */

/* 0x08113954: m4aMPlayModDepthSet */
void sub_08113954(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u8 modDepth) {
    s32 i;
    u32 bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->ident != ID_NUMBER)
        return;

    mplayInfo->ident++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->flags & MPT_FLG_EXIST) {
                track->lfoSpeed = modDepth;
                if (!track->lfoSpeed)
                    sub_08113934(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->ident = ID_NUMBER;
}

/* 0x081139C8: m4aMPlayLFOSpeedSet */
void sub_081139C8(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u8 lfoSpeed) {
    s32 i;
    u32 bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->ident != ID_NUMBER)
        return;

    mplayInfo->ident++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->flags & MPT_FLG_EXIST) {
                track->mod = lfoSpeed;
                if (!track->mod)
                    sub_08113934(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->ident = ID_NUMBER;
}
