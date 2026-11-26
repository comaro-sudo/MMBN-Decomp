/* DC-5 C (agbcc -O2): Nintendo m4a VSync & MusicPlayer init
 * (code_10, RS 0x08112EC8-0x08112F7B, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define ID_NUMBER      0x68736D53
#define SOUND_INFO_PTR (*(struct SoundInfo **)0x03007FF0)
#define REG_DMA1CNT_H  (*(volatile u16 *)0x040000C6)
#define REG_DMA2CNT_H  (*(volatile u16 *)0x040000D2)

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
    volatile u8 pcmDmaCounter;       /* +0x04 */
    u8 pad05[0x1B];
    void (*MPlayMainHead)(void *);   /* +0x20 */
    struct MusicPlayerInfo *musicPlayerHead; /* +0x24 */
};

extern char dat_081121B5[];
void sub_08112BB0(void *p); /* Clear64byte */

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
