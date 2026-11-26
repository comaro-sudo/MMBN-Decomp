/* DC-5 C (agbcc -O2): Nintendo m4a MidiKey2Freq
 * (code_10, RS 0x08112720-0x08112783, BM +0x14).
 * Compiled with pret/agbcc -O2 -mthumb-interwork and verified 100%
 * byte-identical in both Red Sun and Blue Moon. */

#include "gba_types.h"

struct WaveData {
    u16 type;
    u16 status;
    u32 freq;
};

extern const u8 dat_0820A214[];  /* gScaleTable */
extern const u32 dat_0820A2C8[]; /* gFreqTable */

u32 sub_08111B04(u32 a, u32 b);  /* umul3232H32 */

/* 0x08112720: MidiKey2Freq */
u32 sub_08112720(struct WaveData *wav, u8 key, u8 fineAdjust) {
    u32 val1;
    u32 val2;
    u32 fineAdjustShifted = fineAdjust << 24;

    if (key > 178) {
        key = 178;
        fineAdjustShifted = 255 << 24;
    }

    val1 = dat_0820A214[key];
    val1 = dat_0820A2C8[val1 & 0xF] >> (val1 >> 4);

    val2 = dat_0820A214[key + 1];
    val2 = dat_0820A2C8[val2 & 0xF] >> (val2 >> 4);

    return sub_08111B04(wav->freq, val1 + sub_08111B04(val2 - val1, fineAdjustShifted));
}
