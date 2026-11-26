/* DC-5 C (agbcc -O2): Nintendo m4a MPlayImmInit & MPlayExtender
 * (code_10, RS 0x08112A38-0x08112B97, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define ID_NUMBER     0x68736D53
#define SOUND_INFO_PTR (*(struct SoundInfo **)0x03007FF0)
#define REG_NR12      (*(vu8 *)0x04000063)
#define REG_NR14      (*(vu8 *)0x04000065)
#define REG_NR22      (*(vu8 *)0x04000069)
#define REG_NR24      (*(vu8 *)0x0400006D)
#define REG_NR30      (*(vu8 *)0x04000070)
#define REG_NR42      (*(vu8 *)0x04000079)
#define REG_NR44      (*(vu8 *)0x0400007D)
#define REG_SOUNDCNT_L (*(vu16 *)0x04000080)
#define REG_SOUNDCNT_L_B (*(vu8 *)0x04000080)
#define REG_SOUNDCNT_X (*(vu16 *)0x04000084)

#define MPT_FLG_EXIST 0x80
#define MPT_FLG_START 0x40

struct CgbChannel {
    u8 statusFlags;     /* +0x00 */
    u8 type;            /* +0x01 */
    u8 pad02[0x1A];
    u8 panMask;         /* +0x1C */
    u8 pad1D[0x23];
};

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

struct SoundInfo {
    u32 ident;                       /* +0x00 */
    u8 pad04[0x08];
    u8 maxLines;                     /* +0x0C */
    u8 pad0D[0x0F];
    struct CgbChannel *cgbChans;     /* +0x1C */
    void *MPlayMainHead;             /* +0x20 */
    void *musicPlayerHead;           /* +0x24 */
    void (*CgbSound)(void);          /* +0x28 */
    void (*CgbOscOff)(u8);           /* +0x2C */
    u32 (*MidiKeyToCgbFreq)(u8, u8, u8); /* +0x30 */
};

void sub_08111880(const void *src, void *dst, u32 control); /* CpuSet */
void sub_08112BB0(void *p);                                 /* Clear64byte */
void sub_08113A3C(void);
void sub_081126F8(void);
void sub_0811270C(void);
void sub_08113B94(void);
void sub_08112690(void);
void sub_08112CBC(void);
void sub_0811241C(void);
void sub_081130A0(void);
void sub_08113168(void);
void sub_0811337C(void);
void sub_081132C4(u8);
u32 sub_0811321C(u8, u8, u8);

extern char gMaxLines_0[];

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

/* 0x08112A80: MPlayExtender */
void sub_08112A80(struct CgbChannel *cgbChans) {
    struct SoundInfo *soundInfo;
    u32 ident;

    REG_SOUNDCNT_X = 0x008F;
    REG_SOUNDCNT_L = 0;
    REG_NR12 = 0x08;
    REG_NR22 = 0x08;
    REG_NR42 = 0x08;
    REG_NR14 = 0x80;
    REG_NR24 = 0x80;
    REG_NR44 = 0x80;
    REG_NR30 = 0;
    REG_SOUNDCNT_L_B = 0x77;

    soundInfo = SOUND_INFO_PTR;
    ident = soundInfo->ident;

    if (ident != ID_NUMBER)
        return;

    soundInfo->ident++;

    {
        void (**jt)(void) = (void (**)(void))0x03004650;
        jt[8]  = sub_08113A3C;
        jt[17] = sub_081126F8;
        jt[19] = sub_0811270C;
        jt[28] = sub_08113B94;
        jt[29] = sub_08112690;
        jt[30] = sub_08112CBC;
        jt[31] = sub_0811241C;
        jt[32] = sub_081130A0;
        jt[33] = sub_08113168;
    }

    soundInfo->cgbChans = cgbChans;
    soundInfo->CgbSound = sub_0811337C;
    soundInfo->CgbOscOff = sub_081132C4;
    soundInfo->MidiKeyToCgbFreq = sub_0811321C;
    soundInfo->maxLines = (u32)gMaxLines_0;

    {
        u32 zero = 0;
        sub_08111880(&zero, cgbChans, 0x05000040);
    }

    cgbChans[0].type = 1;
    cgbChans[0].panMask = 0x11;
    cgbChans[1].type = 2;
    cgbChans[1].panMask = 0x22;
    cgbChans[2].type = 3;
    cgbChans[2].panMask = 0x44;
    cgbChans[3].type = 4;
    cgbChans[3].panMask = 0x88;

    soundInfo->ident = ident;
}
