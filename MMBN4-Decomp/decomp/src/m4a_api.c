/* DC-5 C (old_agbcc -O2): Nintendo m4a MusicPlayer public API
 * (code_10, RS 0x0811283C-0x0811297B, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;

#define ID_NUMBER 0x68736D53
#define MUSICPLAYER_STATUS_TRACK 0x0000FFFF
#define MUSICPLAYER_STATUS_PAUSE 0x80000000

struct SongHeader;

struct MusicPlayerInfo {
    const struct SongHeader *songHeader; /* +0x00 */
    u32 status;                          /* +0x04 */
    u8 pad08[0x1C];
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
extern char NUM_MUSIC_PLAYERS[];

void sub_08111B14(void);                                                    /* SoundMain */
void sub_08112F7C(struct MusicPlayerInfo *mplayInfo, const struct SongHeader *songHeader); /* MPlayStart */
void sub_08113060(struct MusicPlayerInfo *mplayInfo);                       /* m4aMPlayStop */
void sub_08112788(struct MusicPlayerInfo *mplayInfo);                       /* MPlayContinue */
void sub_081127A4(struct MusicPlayerInfo *mplayInfo, u16 speed);            /* MPlayFadeOut */

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
