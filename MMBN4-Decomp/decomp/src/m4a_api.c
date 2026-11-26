/* DC-5 C (old_agbcc -O2): Nintendo m4a MusicPlayer public API
 * (m4aSoundInit..m4aMPlayFadeOut: code_10, RS 0x081127C4-0x081129EF, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define ID_NUMBER 0x68736D53
#define MUSICPLAYER_STATUS_TRACK 0x0000FFFF
#define MUSICPLAYER_STATUS_PAUSE 0x80000000

struct SongHeader;

struct MusicPlayerInfo {
    const struct SongHeader *songHeader; /* +0x00 */
    u32 status;                          /* +0x04 */
    u8 trackCount;                       /* +0x08 */
    u8 priority;                         /* +0x09 */
    u8 cmd;                              /* +0x0A */
    u8 unk_B;                            /* +0x0B */
    u32 clock;                           /* +0x0C */
    u8 gap[8];                           /* +0x10 */
    u8 *memAccArea;                      /* +0x18 */
    u8 pad1C[8];
    u16 fadeOC;                          /* +0x24 */
    u16 fadeOI;                          /* +0x26 */
    u16 fadeOV;                          /* +0x28 */
    u8 pad2A[0x0A];
    u32 ident;                           /* +0x34 */
};

struct MusicPlayer {
    struct MusicPlayerInfo *info;
    void *track;
    u8 unk_8;
    u16 unk_A;
};

struct Song {
    const struct SongHeader *header;
    u16 ms;
    u16 me;
};

extern const struct MusicPlayer dat_08155E78[];
extern const struct Song dat_08155FF8[];
extern char dat_08111B99[];
extern char NUM_MUSIC_PLAYERS[];

void sub_08111880(const void *src, void *dst, u32 control); /* CpuSet */
void sub_08111B14(void);                                    /* SoundMain */
void sub_08112BC4(void *si);                                /* SoundInit */
void sub_08112A80(void *cgb);                               /* MPlayExtender */
void sub_08112D60(u32 mode);                                /* m4aSoundMode */
void sub_08112F04(struct MusicPlayerInfo *mplayInfo, void *track, u8 trackCount); /* m4aMPlayInit */
void sub_08112F7C(struct MusicPlayerInfo *mplayInfo, const struct SongHeader *songHeader); /* MPlayStart */
void sub_08113060(struct MusicPlayerInfo *mplayInfo);       /* m4aMPlayStop */
void sub_08112788(struct MusicPlayerInfo *mplayInfo);       /* MPlayContinue */
void sub_081127A4(struct MusicPlayerInfo *mplayInfo, u16 speed); /* MPlayFadeOut */

/* 0x081127C4: m4aSoundInit */
void sub_081127C4(void) {
    s32 i;

    sub_08111880((void *)((u32)dat_08111B99 & ~1), (void *)0x030047E0, 0x04000100);
    sub_08112BC4((void *)0x030036A0);
    sub_08112A80((void *)0x030046E0);
    sub_08112D60(0x0093C400);

    for (i = 0; i < (u16)(u32)NUM_MUSIC_PLAYERS; i++) {
        struct MusicPlayerInfo *mplayInfo = dat_08155E78[i].info;
        sub_08112F04(mplayInfo, dat_08155E78[i].track, dat_08155E78[i].unk_8);
        mplayInfo->unk_B = dat_08155E78[i].unk_A;
        mplayInfo->memAccArea = (u8 *)0x02011DA0;
    }
}

/* 0x0811283C: m4aSoundMain */
void sub_0811283C(void) {
    sub_08111B14();
}

/* 0x08112848: m4aSongNumStart */
void sub_08112848(u16 n) {
    const struct MusicPlayer *mplayTable = dat_08155E78;
    const struct Song *songTable = dat_08155FF8;
    const struct Song *song = &songTable[n];
    const struct MusicPlayer *mplay = &mplayTable[song->ms];

    sub_08112F7C(mplay->info, song->header);
}

/* 0x08112874: m4aSongNumStartOrChange */
void sub_08112874(u16 n) {
    const struct MusicPlayer *mplayTable = dat_08155E78;
    const struct Song *songTable = dat_08155FF8;
    const struct Song *song = &songTable[n];
    const struct MusicPlayer *mplay = &mplayTable[song->ms];

    if (mplay->info->songHeader != song->header) {
        sub_08112F7C(mplay->info, song->header);
    } else {
        if ((mplay->info->status & MUSICPLAYER_STATUS_TRACK) == 0
            || (mplay->info->status & MUSICPLAYER_STATUS_PAUSE)) {
            sub_08112F7C(mplay->info, song->header);
        }
    }
}

/* 0x081128C0: m4aSongNumStartOrContinue */
void sub_081128C0(u16 n) {
    const struct MusicPlayer *mplayTable = dat_08155E78;
    const struct Song *songTable = dat_08155FF8;
    const struct Song *song = &songTable[n];
    const struct MusicPlayer *mplay = &mplayTable[song->ms];

    if (mplay->info->songHeader != song->header) {
        sub_08112F7C(mplay->info, song->header);
    } else if ((mplay->info->status & MUSICPLAYER_STATUS_TRACK) == 0) {
        sub_08112F7C(mplay->info, song->header);
    } else if (mplay->info->status & MUSICPLAYER_STATUS_PAUSE) {
        sub_08112788(mplay->info);
    }
}

/* 0x08112914: m4aSongNumStop */
void sub_08112914(u16 n) {
    const struct MusicPlayer *mplayTable = dat_08155E78;
    const struct Song *songTable = dat_08155FF8;
    const struct Song *song = &songTable[n];
    const struct MusicPlayer *mplay = &mplayTable[song->ms];

    if (mplay->info->songHeader == song->header) {
        sub_08113060(mplay->info);
    }
}

/* 0x08112948: m4aSongNumContinue */
void sub_08112948(u16 n) {
    const struct MusicPlayer *mplayTable = dat_08155E78;
    const struct Song *songTable = dat_08155FF8;
    const struct Song *song = &songTable[n];
    const struct MusicPlayer *mplay = &mplayTable[song->ms];

    if (mplay->info->songHeader == song->header) {
        sub_08112788(mplay->info);
    }
}

/* 0x0811297C: m4aMPlayAllStop */
void sub_0811297C(void) {
    s32 i;

    for (i = 0; i < (u16)(u32)NUM_MUSIC_PLAYERS; i++)
        sub_08113060(dat_08155E78[i].info);
}

/* 0x081129A8: m4aMPlayContinue */
void sub_081129A8(struct MusicPlayerInfo *mplayInfo) {
    sub_08112788(mplayInfo);
}

/* 0x081129B4: m4aMPlayAllContinue */
void sub_081129B4(void) {
    s32 i;

    for (i = 0; i < (u16)(u32)NUM_MUSIC_PLAYERS; i++)
        sub_08112788(dat_08155E78[i].info);
}

/* 0x081129E0: m4aMPlayFadeOut */
void sub_081129E0(struct MusicPlayerInfo *mplayInfo, u16 speed) {
    sub_081127A4(mplayInfo, speed);
}
