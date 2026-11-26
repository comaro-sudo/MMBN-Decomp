/* DC-5 C (old_agbcc -O2): Nintendo m4a MPlayStart
 * (code_10, RS 0x08112F7C-0x0811305F, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define ID_NUMBER                0x68736D53
#define MUSICPLAYER_STATUS_TRACK 0x0000FFFF
#define MUSICPLAYER_STATUS_PAUSE 0x80000000

struct SongHeader {
    u8 trackCount;
    u8 blockCount;
    u8 priority;
    u8 reverb;
    void *tone;
    u8 *part[1];
};

struct MusicPlayerTrack {
    u8 flags;
    u8 pad01[0x1F];
    void *chan;
    u8 pad24[0x1C];
    u8 *cmdPtr;
    u8 pad44[0x0C];
};

struct MusicPlayerInfo {
    const struct SongHeader *songHeader; /* +0x00 */
    u32 status;                          /* +0x04 */
    u8 trackCount;                       /* +0x08 */
    u8 priority;                         /* +0x09 */
    u8 cmd;                              /* +0x0A */
    u8 unk_B;                            /* +0x0B */
    u32 clock;                           /* +0x0C */
    u8 gap_10[0x0C];
    u16 tempoD;                          /* +0x1C */
    u16 tempoU;                          /* +0x1E */
    u16 tempoI;                          /* +0x20 */
    u16 tempoC;                          /* +0x22 */
    u16 fadeOC;                          /* +0x24 */
    u16 fadeOI;                          /* +0x26 */
    u16 fadeOV;                          /* +0x28 */
    u16 pad2A;
    struct MusicPlayerTrack *tracks;     /* +0x2C */
    void *tone;                          /* +0x30 */
    u32 ident;                           /* +0x34 */
};

void sub_0811241C(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track); /* TrackStop */
void sub_08112D60(u32 mode);                                                          /* m4aSoundMode */

/* 0x08112F7C: MPlayStart */
void sub_08112F7C(struct MusicPlayerInfo *mplayInfo, const struct SongHeader *songHeader) {
    s32 i;
    u8 undef;
    struct MusicPlayerTrack *track;

    if (mplayInfo->ident != ID_NUMBER)
        return;

    undef = mplayInfo->unk_B;

    if (!undef
        || ((!mplayInfo->songHeader || !(mplayInfo->tracks[0].flags & 0x40))
            && ((mplayInfo->status & MUSICPLAYER_STATUS_TRACK) == 0
                || (mplayInfo->status & MUSICPLAYER_STATUS_PAUSE)))
        || (mplayInfo->priority <= songHeader->priority)) {
        mplayInfo->ident++;
        mplayInfo->status = 0;
        mplayInfo->songHeader = songHeader;
        mplayInfo->tone = songHeader->tone;
        mplayInfo->priority = songHeader->priority;
        mplayInfo->clock = 0;
        mplayInfo->tempoD = 150;
        mplayInfo->tempoI = 150;
        mplayInfo->tempoU = 256;
        mplayInfo->tempoC = 0;
        mplayInfo->fadeOC = 0;

        i = 0;
        track = mplayInfo->tracks;

        while (i < songHeader->trackCount && i < mplayInfo->trackCount) {
            sub_0811241C(mplayInfo, track);
            track->flags = 0xC0;
            track->chan = 0;
            track->cmdPtr = songHeader->part[i];
            i++;
            track++;
        }

        while (i < mplayInfo->trackCount) {
            sub_0811241C(mplayInfo, track);
            track->flags = 0;
            i++;
            track++;
        }

        if (songHeader->reverb & 0x80)
            sub_08112D60(songHeader->reverb);

        mplayInfo->ident = ID_NUMBER;
    }
}
