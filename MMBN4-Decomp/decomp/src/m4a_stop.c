/* DC-5 C (agbcc -O2): Nintendo m4a MusicPlayer stop
 * (code_10, RS 0x08113060-0x0811309F, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed int s32;

#define ID_NUMBER 0x68736D53

struct MusicPlayerTrack {
    u8 flags;
    u8 pad[0x4F];
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

void sub_0811241C(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track); /* TrackStop */

/* 0x08113060: m4aMPlayStop */
void sub_08113060(struct MusicPlayerInfo *mplayInfo) {
    s32 i;
    struct MusicPlayerTrack *track;

    if (mplayInfo->ident != ID_NUMBER)
        return;

    mplayInfo->ident++;
    mplayInfo->status |= 0x80000000;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0) {
        sub_0811241C(mplayInfo, track);
        i--;
        track++;
    }

    mplayInfo->ident = ID_NUMBER;
}
