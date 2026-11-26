/* DC-5 C (agbcc -O2): Nintendo m4a sound mode, clear, VSync & MPlay init
 * (code_10, RS 0x08112D60-0x08112F7B, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define ID_NUMBER      0x68736D53
#define SOUND_INFO_PTR (*(struct SoundInfo **)0x03007FF0)
#define REG_SOUNDBIAS_H (*(vu8 *)0x04000089)
#define REG_DMA1CNT    (*(vu32 *)0x040000C4)
#define REG_DMA1CNT_H  (*(vu16 *)0x040000C6)
#define REG_DMA2CNT    (*(vu32 *)0x040000D0)
#define REG_DMA2CNT_H  (*(vu16 *)0x040000D2)

#define SOUND_MODE_REVERB_VAL  0x0000007F
#define SOUND_MODE_REVERB_SET  0x00000080
#define SOUND_MODE_MAXCHN      0x00000F00
#define SOUND_MODE_MASVOL      0x0000F000
#define SOUND_MODE_FREQ        0x000F0000
#define SOUND_MODE_DA_BIT      0x00B00000
#define SOUND_MODE_DA_BIT_MASK 0x00300000

struct SoundChannel {
    u8 statusFlags;
    u8 pad[0x3F];
};

struct CgbChannel {
    u8 statusFlags;
    u8 pad[0x3F];
};

struct MusicPlayerTrack {
    u8 flags;
    u8 pad[0x4F];
};

struct MusicPlayerInfo {
    const void *songHeader;          /* +0x00 */
    u32 status;                      /* +0x04 */
    u8 trackCount;                   /* +0x08 */
    u8 priority;                     /* +0x09 */
    u8 cmd;                          /* +0x0A */
    u8 unk_B;                        /* +0x0B */
    u32 clock;                       /* +0x0C */
    u8 gap_10[0x1C];
    struct MusicPlayerTrack *tracks; /* +0x2C */
    void *tone;                      /* +0x30 */
    u32 ident;                       /* +0x34 */
    void (*func)(void *);            /* +0x38 */
    void *intp;                      /* +0x3C */
};

struct SoundInfo {
    u32 ident;                       /* +0x00 */
    vu8 pcmDmaCounter;               /* +0x04 */
    u8 reverb;                       /* +0x05 */
    u8 maxChn;                       /* +0x06 */
    u8 masterVolume;                 /* +0x07 */
    u8 freq;                         /* +0x08 */
    u8 mode;                         /* +0x09 */
    u8 c15;                          /* +0x0A */
    u8 pcmDmaPeriod;                 /* +0x0B */
    u8 maxLines;                     /* +0x0C */
    u8 gap_0D[3];
    s32 pcmSamplesPerVBlank;         /* +0x10 */
    s32 pcmFreq;                     /* +0x14 */
    s32 divFreq;                     /* +0x18 */
    struct CgbChannel *cgbChans;     /* +0x1C */
    void (*MPlayMainHead)(void *);   /* +0x20 */
    struct MusicPlayerInfo *musicPlayerHead; /* +0x24 */
    void (*CgbSound)(void);          /* +0x28 */
    void (*CgbOscOff)(u8);           /* +0x2C */
    u32 (*MidiKeyToCgbFreq)(u8, u8, u8); /* +0x30 */
    u8 pad34[0x1C];
    struct SoundChannel chans[12];   /* +0x50..+0x34F */
    s8 pcmBuffer[0xC60];             /* +0x350 */
};

extern char dat_081121B5[];
void sub_08111880(const void *src, void *dst, u32 control); /* CpuSet */
void sub_08112BB0(void *p);                                 /* Clear64byte */
void sub_08112CBC(u32 mode);                                /* SampleFreqSet */
void sub_08112E4C(void);                                    /* m4aSoundVSyncOff */

/* 0x08112D60: m4aSoundMode */
void sub_08112D60(u32 mode) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    u32 temp;

    if (soundInfo->ident != ID_NUMBER)
        return;

    soundInfo->ident++;

    temp = mode & (SOUND_MODE_REVERB_SET | SOUND_MODE_REVERB_VAL);
    if (temp)
        soundInfo->reverb = temp & SOUND_MODE_REVERB_VAL;

    temp = mode & SOUND_MODE_MAXCHN;
    if (temp) {
        struct SoundChannel *chan;

        soundInfo->maxChn = temp >> 8;

        temp = 12;
        chan = &soundInfo->chans[0];

        while (temp > 0) {
            chan->statusFlags = 0;
            temp--;
            chan++;
        }
    }

    temp = mode & SOUND_MODE_MASVOL;
    if (temp)
        soundInfo->masterVolume = temp >> 12;

    temp = mode & SOUND_MODE_DA_BIT;
    if (temp) {
        temp = (temp & SOUND_MODE_DA_BIT_MASK) >> 14;
        REG_SOUNDBIAS_H = (REG_SOUNDBIAS_H & 0x3F) | temp;
    }

    temp = mode & SOUND_MODE_FREQ;
    if (temp) {
        sub_08112E4C();
        sub_08112CBC(temp);
    }

    soundInfo->ident = ID_NUMBER;
}

/* 0x08112DF8: m4aSoundClear */
void sub_08112DF8(void) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    s32 i;
    void *chan;

    if (soundInfo->ident != ID_NUMBER)
        return;

    soundInfo->ident++;

    i = 12;
    chan = &soundInfo->chans[0];

    while (i > 0) {
        ((struct SoundChannel *)chan)->statusFlags = 0;
        i--;
        chan = (void *)((u32)chan + sizeof(struct SoundChannel));
    }

    chan = soundInfo->cgbChans;

    if (chan) {
        i = 1;

        while (i <= 4) {
            soundInfo->CgbOscOff(i);
            ((struct CgbChannel *)chan)->statusFlags = 0;
            i++;
            chan = (void *)((u32)chan + sizeof(struct CgbChannel));
        }
    }

    soundInfo->ident = ID_NUMBER;
}

/* 0x08112E4C: m4aSoundVSyncOff */
void sub_08112E4C(void) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;

    if (soundInfo->ident >= ID_NUMBER && soundInfo->ident <= ID_NUMBER + 1) {
        soundInfo->ident += 10;

        if (REG_DMA1CNT & 0x02000000)
            REG_DMA1CNT = 0x84400004;

        if (REG_DMA2CNT & 0x02000000)
            REG_DMA2CNT = 0x84400004;

        REG_DMA1CNT_H = 0x0400;
        REG_DMA2CNT_H = 0x0400;

        {
            u32 zero = 0;
            sub_08111880(&zero, soundInfo->pcmBuffer, 0x05000318);
        }
    }
}

/* 0x08112EC8: m4aSoundVSyncOn */
void sub_08112EC8(void) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    u32 ident = soundInfo->ident;

    if (ident == ID_NUMBER)
        return;

    REG_DMA1CNT_H = 0xB600;
    REG_DMA2CNT_H = 0xB600;

    soundInfo->pcmDmaCounter = 0;
    soundInfo->ident = ident - 10;
}

/* 0x08112F04: m4aMPlayInit */
void sub_08112F04(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *trackBuf, u8 trackCount) {
    struct SoundInfo *soundInfo;

    if (trackCount == 0)
        return;

    if (trackCount > 16)
        trackCount = 16;

    soundInfo = SOUND_INFO_PTR;
    if (soundInfo->ident != ID_NUMBER)
        return;

    soundInfo->ident++;

    sub_08112BB0(mplayInfo);

    mplayInfo->tracks = trackBuf;
    mplayInfo->trackCount = trackCount;
    mplayInfo->status = 0x80000000;

    while (trackCount != 0) {
        trackBuf->flags = 0;
        trackCount--;
        trackBuf++;
    }

    if (soundInfo->MPlayMainHead != 0) {
        mplayInfo->func = soundInfo->MPlayMainHead;
        mplayInfo->intp = soundInfo->musicPlayerHead;
        soundInfo->MPlayMainHead = 0;
    }

    soundInfo->musicPlayerHead = mplayInfo;
    soundInfo->MPlayMainHead = (void (*)(void *))dat_081121B5;
    soundInfo->ident = ID_NUMBER;
    mplayInfo->ident = ID_NUMBER;
}
