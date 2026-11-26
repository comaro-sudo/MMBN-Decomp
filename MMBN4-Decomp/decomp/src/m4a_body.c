/* DC-5 C (agbcc -O2): Nintendo m4a FadeOutBody, TrkVolPitSet & MidiKeyToCgbFreq
 * (code_10, RS 0x081130A0-0x081132C3, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define MUSICPLAYER_STATUS_PAUSE 0x80000000
#define MPT_FLG_VOLSET 0x01
#define MPT_FLG_VOLCHG 0x03
#define MPT_FLG_PITSET 0x04
#define MPT_FLG_PITCHG 0x0C
#define MPT_FLG_EXIST  0x80

struct MusicPlayerTrack {
    u8 flags;           /* +0x00 */
    u8 pad01[0x07];
    u8 keyM;            /* +0x08 */
    u8 pitM;            /* +0x09 */
    s8 keyShift;        /* +0x0A */
    s8 keyShiftX;       /* +0x0B */
    s8 tune;            /* +0x0C */
    u8 pitX;            /* +0x0D */
    s8 bend;            /* +0x0E */
    u8 bendRange;       /* +0x0F */
    u8 volMR;           /* +0x10 */
    u8 volML;           /* +0x11 */
    u8 vol;             /* +0x12 */
    u8 volX;            /* +0x13 */
    s8 pan;             /* +0x14 */
    s8 panX;            /* +0x15 */
    s8 modM;            /* +0x16 */
    u8 lfoSpeed;        /* +0x17 */
    u8 modT;            /* +0x18 */
    u8 pad19[0x37];
};

struct MusicPlayerInfo {
    const void *songHeader;          /* +0x00 */
    u32 status;                      /* +0x04 */
    u8 trackCount;                   /* +0x08 */
    u8 pad09[0x1B];
    u16 fadeOC;                      /* +0x24 */
    u16 fadeOI;                      /* +0x26 */
    u16 fadeOV;                      /* +0x28 */
    u16 pad2A;
    struct MusicPlayerTrack *tracks; /* +0x2C */
};

extern const u8 dat_0820A3AC[];  /* gNoiseTable */
extern const u8 dat_0820A310[];  /* gCgbScaleTable */
extern const s16 dat_0820A394[]; /* gCgbFreqTable */

void sub_0811241C(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track); /* TrackStop */

/* 0x081130A0: FadeOutBody */
void sub_081130A0(struct MusicPlayerInfo *mplayInfo) {
    s32 i;
    struct MusicPlayerTrack *track;
    u16 fadeOV;

    if (mplayInfo->fadeOC == 0)
        return;
    if (--mplayInfo->fadeOI != 0)
        return;

    mplayInfo->fadeOI = mplayInfo->fadeOC;

    if (mplayInfo->fadeOV & 2) {
        if ((mplayInfo->fadeOV += 16) >= 256) {
            mplayInfo->fadeOV = 256;
            mplayInfo->fadeOC = 0;
        }
    } else {
        if ((s16)(mplayInfo->fadeOV -= 16) <= 0) {
            i = mplayInfo->trackCount;
            track = mplayInfo->tracks;

            while (i > 0) {
                sub_0811241C(mplayInfo, track);

                if (!(mplayInfo->fadeOV & 1))
                    track->flags = 0;

                i--;
                track++;
            }

            if (mplayInfo->fadeOV & 1)
                mplayInfo->status |= MUSICPLAYER_STATUS_PAUSE;
            else
                mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;

            mplayInfo->fadeOC = 0;
            return;
        }
    }

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0) {
        if (track->flags & MPT_FLG_EXIST) {
            fadeOV = mplayInfo->fadeOV;
            track->volX = fadeOV >> 2;
            track->flags |= MPT_FLG_VOLCHG;
        }

        i--;
        track++;
    }
}

/* 0x08113168: TrkVolPitSet */
void sub_08113168(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    if (track->flags & MPT_FLG_VOLSET) {
        s32 x;
        s32 y;

        x = (u32)(track->vol * track->volX) >> 5;

        if (track->modT == 1)
            x = (u32)(x * (track->modM + 128)) >> 7;

        y = 2 * track->pan + track->panX;

        if (track->modT == 2)
            y += track->modM;

        if (y < -128)
            y = -128;
        else if (y > 127)
            y = 127;

        track->volMR = (u32)((y + 128) * x) >> 8;
        track->volML = (u32)((127 - y) * x) >> 8;
    }

    if (track->flags & MPT_FLG_PITSET) {
        s32 bend = track->bend * track->bendRange;
        s32 x = (track->tune + bend) * 4
              + (track->keyShift << 8)
              + (track->keyShiftX << 8)
              + track->pitX;

        if (track->modT == 0)
            x += 16 * track->modM;

        track->keyM = x >> 8;
        track->pitM = x;
    }

    track->flags &= ~(MPT_FLG_PITSET | MPT_FLG_VOLSET);
}

/* 0x0811321C: MidiKeyToCgbFreq */
u32 sub_0811321C(u8 chanNum, u8 key, u8 fineAdjust) {
    if (chanNum == 4) {
        if (key <= 20) {
            key = 0;
        } else {
            key -= 21;
            if (key > 59)
                key = 59;
        }

        return dat_0820A3AC[key];
    } else {
        s32 val1;
        s32 val2;

        if (key <= 35) {
            fineAdjust = 0;
            key = 0;
        } else {
            key -= 36;
            if (key > 130) {
                key = 130;
                fineAdjust = 255;
            }
        }

        val1 = dat_0820A310[key];
        val1 = dat_0820A394[val1 & 0xF] >> (val1 >> 4);

        val2 = dat_0820A310[key + 1];
        val2 = dat_0820A394[val2 & 0xF] >> (val2 >> 4);

        return val1 + ((fineAdjust * (val2 - val1)) >> 8) + 2048;
    }
}
