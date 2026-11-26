/* Phase D3 / E2 portable C reference implementations of MMBN4 boss/Navi SP
 * record time-bonus calculation, SP+ Violet Eclipse scaling, and enemy rank
 * helpers.
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "gba_types.h"

#define SP_RECORD_THRESHOLD_FRAMES (30u * 60u) /* 30.0s @ 60fps */
#define SP_BONUS_MAX               100u
#define SP_PLUS_HP_CAP             4095u

/* Computes MMBN4 SP Navi chip ATK bonus from best SP deletion time in frames:
 * max +100 ATK at <= 10.0s (600 frames), scaling down to +0 at >= 30.0s. */
u16 BossCalcSpRecordBonus(u32 bestFrames) {
    u32 sec;
    if (bestFrames == 0)
        return 0;
    sec = bestFrames / 60u;
    if (sec <= 10u)
        return SP_BONUS_MAX;
    if (sec >= 30u)
        return 0;
    return (u16)((30u - sec) * 5u);
}

/* Scales boss HP/ATK/speed for Violet Eclipse SP+ tier (4,095 HP cap, ATK x1.5,
 * Speed x1.25). */
u16 BossScaleSpPlusHp(u16 baseHp) {
    u32 scaled = (u32)baseHp * 2u;
    return (u16)(scaled > SP_PLUS_HP_CAP ? SP_PLUS_HP_CAP : scaled);
}

u16 BossScaleSpPlusAtk(u16 baseAtk) {
    return (u16)(((u32)baseAtk * 3u) >> 1);
}

u16 BossScaleSpPlusSpeed(u16 baseSpeed) {
    return (u16)(((u32)baseSpeed * 5u) >> 2);
}

/* Advances boss rematch rank (V1=0 -> V2=1 -> V3=2 -> SP=3 -> SP+=4) on S-rank
 * or standard win, clamping at SP+ (4). */
u8 BossAdvanceRematchRank(u8 currentRank, u8 bustingLevel, int requireSRank) {
    if (requireSRank && bustingLevel < 11u)
        return currentRank;
    if (currentRank < 4u)
        return (u8)(currentRank + 1u);
    return 4u;
}
