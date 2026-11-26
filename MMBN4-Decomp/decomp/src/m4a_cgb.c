/* DC-5 C (old_agbcc -O2): Nintendo m4a CGB sound engine
 * (CgbOscOff, CgbModVol, CgbSound: code_10, RS 0x081132C4-0x081137C7, BM +0x14).
 * Compiled with pret/old_agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

#define REG_NR10 (*(volatile u8 *)0x04000060)
#define REG_NR11 (*(volatile u8 *)0x04000062)
#define REG_NR12 (*(volatile u8 *)0x04000063)
#define REG_NR13 (*(volatile u8 *)0x04000064)
#define REG_NR14 (*(volatile u8 *)0x04000065)
#define REG_NR21 (*(volatile u8 *)0x04000068)
#define REG_NR22 (*(volatile u8 *)0x04000069)
#define REG_NR23 (*(volatile u8 *)0x0400006C)
#define REG_NR24 (*(volatile u8 *)0x0400006D)
#define REG_NR30 (*(volatile u8 *)0x04000070)
#define REG_NR31 (*(volatile u8 *)0x04000072)
#define REG_NR32 (*(volatile u8 *)0x04000073)
#define REG_NR33 (*(volatile u8 *)0x04000074)
#define REG_NR34 (*(volatile u8 *)0x04000075)
#define REG_NR41 (*(volatile u8 *)0x04000078)
#define REG_NR42 (*(volatile u8 *)0x04000079)
#define REG_NR43 (*(volatile u8 *)0x0400007C)
#define REG_NR44 (*(volatile u8 *)0x0400007D)
#define REG_NR51 (*(volatile u8 *)0x04000081)
#define REG_SOUNDBIAS_H (*(volatile u8 *)0x04000089)
#define REG_WAVE_RAM0 ((volatile u32 *)0x04000090)

#define SOUND_INFO_PTR (*(struct SoundInfo **)0x03007FF0)

extern const u8 dat_0820A3E8[]; /* gCgb3Vol */

struct CgbChannel {
    u8 statusFlags;      /* +0x00 */
    u8 type;             /* +0x01 */
    u8 rightVolume;      /* +0x02 */
    u8 leftVolume;       /* +0x03 */
    u8 attack;           /* +0x04 */
    u8 decay;            /* +0x05 */
    u8 sustain;          /* +0x06 */
    u8 release;          /* +0x07 */
    u8 key;              /* +0x08 */
    u8 envelopeVolume;   /* +0x09 */
    u8 envelopeGoal;     /* +0x0A */
    u8 envelopeCounter;  /* +0x0B */
    u8 pseudoEchoVolume; /* +0x0C */
    u8 pseudoEchoLength; /* +0x0D */
    u8 dummy1;           /* +0x0E */
    u8 dummy2;           /* +0x0F */
    u8 gateTime;         /* +0x10 */
    u8 midiKey;          /* +0x11 */
    u8 velocity;         /* +0x12 */
    u8 priority;         /* +0x13 */
    u8 rhythmPan;        /* +0x14 */
    u8 dummy3[3];        /* +0x15 */
    u8 dummy5;           /* +0x18 */
    u8 sustainGoal;      /* +0x19 */
    u8 n4;               /* +0x1A */
    u8 pan;              /* +0x1B */
    u8 panMask;          /* +0x1C */
    u8 modify;           /* +0x1D */
    u8 length;           /* +0x1E */
    u8 sweep;            /* +0x1F */
    u32 frequency;       /* +0x20 */
    u32 *wavePointer;    /* +0x24 */
    u32 *currentPointer; /* +0x28 */
    void *track;         /* +0x2C */
    void *prevChannelPointer; /* +0x30 */
    void *nextChannelPointer; /* +0x34 */
    u8 dummy4[8];        /* +0x38 */
};

struct SoundInfo {
    u32 ident;
    vu8 pcmDmaCounter;
    u8 reverb;
    u8 maxChans;
    u8 masterVolume;
    u8 freq;
    u8 mode;
    u8 c15;
    u8 pcmDmaPeriod;
    u8 maxLines;
    u8 gap[3];
    s32 pcmSamplesPerVBlank;
    s32 pcmFreq;
    s32 divFreq;
    struct CgbChannel *cgbChans;
};

/* 0x081132C4: CgbOscOff */
void sub_081132C4(u8 ch) {
    switch (ch) {
    case 1:
        REG_NR12 = 8;
        REG_NR14 = 0x80;
        break;
    case 2:
        REG_NR22 = 8;
        REG_NR24 = 0x80;
        break;
    case 3:
        REG_NR30 = 0;
        break;
    default:
        REG_NR42 = 8;
        REG_NR44 = 0x80;
        break;
    }
}

/* 0x08113314: CgbModVol */
void sub_08113314(struct CgbChannel *chan) {
    if (chan->rightVolume >= chan->leftVolume) {
        if ((chan->rightVolume >> 1) >= chan->leftVolume) {
            chan->pan = 0x0F;
            goto clamp;
        }
    } else {
        if ((chan->leftVolume >> 1) >= chan->rightVolume) {
            chan->pan = 0xF0;
            goto clamp;
        }
    }

    chan->pan = 0xFF;
    chan->envelopeGoal = (u32)(chan->rightVolume + chan->leftVolume) >> 4;
    goto done;

clamp:
    chan->envelopeGoal = (u32)(chan->rightVolume + chan->leftVolume) >> 4;
    if (chan->envelopeGoal > 15)
        chan->envelopeGoal = 15;

done:
    chan->sustainGoal = (chan->envelopeGoal * chan->sustain + 15) >> 4;
    chan->pan &= chan->panMask;
}

/* 0x0811337C: CgbSound */
void sub_0811337C(void) {
    s32 ch;
    struct CgbChannel *channels;
    s32 evAdd;
    s32 prevC15;
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    vu8 *nrx0ptr;
    vu8 *nrx1ptr;
    vu8 *nrx2ptr;
    vu8 *nrx3ptr;
    vu8 *nrx4ptr;

    if (soundInfo->c15)
        soundInfo->c15--;
    else
        soundInfo->c15 = 14;

    for (ch = 1, channels = soundInfo->cgbChans; ch <= 4; ch++, channels++) {
        if (!(channels->statusFlags & 0xC7))
            continue;

        switch (ch) {
        case 1:
            nrx0ptr = &REG_NR10;
            nrx1ptr = &REG_NR11;
            nrx2ptr = &REG_NR12;
            nrx3ptr = &REG_NR13;
            nrx4ptr = &REG_NR14;
            break;
        case 2:
            nrx0ptr = &REG_NR10 + 1;
            nrx1ptr = &REG_NR21;
            nrx2ptr = &REG_NR22;
            nrx3ptr = &REG_NR23;
            nrx4ptr = &REG_NR24;
            break;
        case 3:
            nrx0ptr = &REG_NR30;
            nrx1ptr = &REG_NR31;
            nrx2ptr = &REG_NR32;
            nrx3ptr = &REG_NR33;
            nrx4ptr = &REG_NR34;
            break;
        default:
            nrx0ptr = &REG_NR30 + 1;
            nrx1ptr = &REG_NR41;
            nrx2ptr = &REG_NR42;
            nrx3ptr = &REG_NR43;
            nrx4ptr = &REG_NR44;
            break;
        }

        prevC15 = soundInfo->c15;
        evAdd = *nrx2ptr;

        if (channels->statusFlags & 0x80) {
            if (channels->statusFlags & 0x40)
                goto stop_channel;

            channels->statusFlags = 3;
            channels->modify = 3;
            sub_08113314(channels);

            switch (ch) {
            case 1:
                *nrx0ptr = channels->sweep;
            case 2:
                *nrx1ptr = ((u32)channels->wavePointer << 6) + channels->length;
                goto init_env_step_time_dir;
            case 3:
                if (channels->wavePointer != channels->currentPointer) {
                    *nrx0ptr = 0x40;
                    REG_WAVE_RAM0[0] = channels->wavePointer[0];
                    REG_WAVE_RAM0[1] = channels->wavePointer[1];
                    REG_WAVE_RAM0[2] = channels->wavePointer[2];
                    REG_WAVE_RAM0[3] = channels->wavePointer[3];
                    channels->currentPointer = channels->wavePointer;
                }
                *nrx0ptr = 0;
                *nrx1ptr = channels->length;
                if (channels->length)
                    channels->n4 = 0xC0;
                else
                    channels->n4 = 0x80;
                break;
            default:
                *nrx1ptr = channels->length;
                *nrx3ptr = (u32)channels->wavePointer << 3;
            init_env_step_time_dir:
                evAdd = channels->attack + 8;
                if (channels->length)
                    channels->n4 = 0x40;
                else
                    channels->n4 = 0;
                break;
            }

            if ((channels->envelopeCounter = channels->attack)) {
                channels->envelopeVolume = 0;
                goto envelope_step_complete;
            } else {
                goto envelope_attack_complete;
            }
        } else if (channels->statusFlags & 0x04) {
            if ((s8)(--channels->pseudoEchoLength) <= 0) {
            stop_channel:
                sub_081132C4(ch);
                channels->statusFlags = 0;
                goto channel_complete;
            }
            goto envelope_complete;
        } else if ((channels->statusFlags & 0x40) && (channels->statusFlags & 0x03)) {
            channels->statusFlags &= ~0x03;
            if ((channels->envelopeCounter = channels->release)) {
                channels->modify |= 1;
                if (ch != 3)
                    evAdd = channels->release;
                goto envelope_step_complete;
            } else {
                goto envelope_pseudoecho_mode;
            }
        }

    envelope_step_repeat:
        if (channels->envelopeCounter == 0) {
            s32 env;

            if (ch == 3)
                channels->modify |= 1;

            sub_08113314(channels);

            env = channels->statusFlags & 0x03;
            if (env == 0) {
                if ((s8)(--channels->envelopeVolume) <= 0) {
                envelope_pseudoecho_mode:
                    channels->envelopeVolume = ((channels->envelopeGoal * channels->pseudoEchoVolume) + 0xFF) >> 8;
                    if (channels->envelopeVolume) {
                        channels->statusFlags |= 4;
                        channels->modify |= 1;
                        if (ch != 3)
                            evAdd = 8;
                        goto envelope_complete;
                    } else {
                        goto stop_channel;
                    }
                } else {
                    channels->envelopeCounter = channels->release;
                }
            } else if (env == 1) {
            envelope_sustain:
                channels->envelopeVolume = channels->sustainGoal;
                channels->envelopeCounter = 7;
            } else if (env == 2) {
                if ((s8)(--channels->envelopeVolume) <= (s8)channels->sustainGoal) {
                envelope_decay_complete:
                    if (channels->sustain == 0) {
                        channels->statusFlags &= ~0x03;
                        goto envelope_pseudoecho_mode;
                    } else {
                        channels->statusFlags--;
                        channels->modify |= 1;
                        if (ch != 3)
                            evAdd = 8;
                        goto envelope_sustain;
                    }
                } else {
                    channels->envelopeCounter = channels->decay;
                }
            } else {
                if ((u8)(++channels->envelopeVolume) >= channels->envelopeGoal) {
                envelope_attack_complete:
                    channels->statusFlags--;
                    if ((channels->envelopeCounter = channels->decay)) {
                        channels->modify |= 1;
                        channels->envelopeVolume = channels->envelopeGoal;
                        if (ch != 3)
                            evAdd = channels->decay;
                    } else {
                        goto envelope_decay_complete;
                    }
                } else {
                    channels->envelopeCounter = channels->attack;
                }
            }
        }

    envelope_step_complete:
        channels->envelopeCounter--;

        if (prevC15 == 0) {
            prevC15--;
            goto envelope_step_repeat;
        }

    envelope_complete:
        if (channels->modify & 2) {
            if (ch < 4 && (channels->type & 8)) {
                s32 bias = REG_SOUNDBIAS_H;
                if (bias < 0x40)
                    channels->frequency = (channels->frequency + 2) & 0x7FC;
                else if (bias < 0x80)
                    channels->frequency = (channels->frequency + 1) & 0x7FE;
            }

            if (ch != 4)
                *nrx3ptr = channels->frequency;
            else
                *nrx3ptr = (*nrx3ptr & 8) | channels->frequency;

            *nrx4ptr = channels->n4 = (channels->n4 & 0xC0) + (*((u8 *)(&channels->frequency) + 1));
        }

        if (channels->modify & 1) {
            REG_NR51 = (REG_NR51 & ~channels->panMask) | channels->pan;
            if (ch == 3) {
                *nrx2ptr = dat_0820A3E8[channels->envelopeVolume];
                if (channels->n4 & 0x80) {
                    *nrx0ptr = 0x80;
                    *nrx4ptr = channels->n4;
                    channels->n4 &= 0x7F;
                }
            } else {
                evAdd &= 0xF;
                *nrx2ptr = (channels->envelopeVolume << 4) + evAdd;
                *nrx4ptr = channels->n4 | 0x80;
                if (ch == 1 && !(*nrx0ptr & 8))
                    *nrx4ptr = channels->n4 | 0x80;
            }
        }

    channel_complete:
        channels->modify = 0;
    }
}
