/* DC-5 C (old_agbcc -O2): Nintendo m4a MusicPlayer real-time control API
 * (code_10, RS 0x081137F0-0x08113933, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define ID_NUMBER   0x68736D53
#define MPT_FLG_VOLSET 0x01
#define MPT_FLG_VOLCHG 0x03
#define MPT_FLG_PITSET 0x04
#define MPT_FLG_PITCHG 0x0C
#define MPT_FLG_EXIST  0x80

struct MusicPlayerTrack {
    u8 flags;           /* +0x00 */
    u8 pad01[0x0A];
    s8 keyM;            /* +0x0B */
    s8 pitM;            /* +0x0C */
    u8 pitX;            /* +0x0D */
    u8 pad0E[0x05];
    u8 volX;            /* +0x13 */
    s8 pan;             /* +0x14 */
    s8 panX;            /* +0x15 */
    s8 modM;            /* +0x16 */
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
    u8 pad09[0x13];
    u16 tempoD;                      /* +0x1C */
    u16 tempoU;                      /* +0x1E */
    u16 tempoI;                      /* +0x20 */
    u8 pad22[0x0A];
    struct MusicPlayerTrack *tracks; /* +0x2C */
    void *tone;                      /* +0x30 */
    u32 ident;                       /* +0x34 */
};

/* 0x081137F0: m4aMPlayVolumeControl */
void sub_081137F0(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u16 volume) {
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
                track->volX = volume / 4;
                track->flags |= MPT_FLG_VOLCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->ident = ID_NUMBER;
}

/* 0x08113858: m4aMPlayPitchControl */
void sub_08113858(struct MusicPlayerInfo *mplayInfo, u16 trackBits, s16 pitch) {
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
                track->keyM = pitch >> 8;
                track->pitX = pitch;
                track->flags |= MPT_FLG_PITCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->ident = ID_NUMBER;
}

/* 0x081138CC: m4aMPlayPanpotControl */
void sub_081138CC(struct MusicPlayerInfo *mplayInfo, u16 trackBits, s8 pan) {
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
                track->panX = pan;
                track->flags |= MPT_FLG_VOLCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->ident = ID_NUMBER;
}
