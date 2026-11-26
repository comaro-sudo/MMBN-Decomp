/* Phase D1 / E2 portable C reference implementations of MMBN4 Busting Level
 * scoring (BattleScoreBustingLevel sub_080081BC) and New Game+ enemy tier
 * scaling (BN4_ID_RAISE sub_0800B036) from code_00.s.
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "gba_types.h"

#define BUSTING_RANK_MIN 1
#define BUSTING_RANK_S   11

/* 0x0800B036: BN4_ID_RAISE (New Game+ virus/Navi tier promotion) */
u8 RaiseEnemyIdForPlaythrough(u8 enemyId, u8 enemyRankField, u8 playthrough, u8 battleFlags) {
    s32 tier, maxRaise, step;

    if ((battleFlags & 0x80u) || enemyId == 0)
        return enemyId;

    tier = playthrough > 2 ? 2 : (s32)playthrough;

    if (enemyId < 0x80u) {
        /* Viruses advance +2 per playthrough tier, up to rank 5 */
        maxRaise = 5 - (s32)enemyRankField;
        if (maxRaise <= 0)
            return enemyId;
        step = tier * 2;
        if (step > maxRaise)
            step = maxRaise;
        return (u8)(enemyId + step);
    }

    if (enemyId < 0xE4u) {
        /* Navis advance +1 per playthrough tier, up to rank 2 */
        maxRaise = 2 - (s32)enemyRankField;
        if (maxRaise <= 0)
            return enemyId;
        step = tier > maxRaise ? maxRaise : tier;
        return (u8)(enemyId + step);
    }

    return enemyId;
}

/* 0x080081BC sub-components: Busting Level scoring */
s32 ScoreBustingFlinch(s32 flinchCount) {
    if (flinchCount < 4)
        return 1 - flinchCount; /* 0 hits -> +1, 1 -> 0, 2 -> -1, 3 -> -2 */
    return -3;
}

s32 ScoreBustingMovement(s32 moveCount) {
    return moveCount <= 2 ? 1 : 0;
}

s32 ScoreBustingMultiDelete(u8 maxSimultaneousDeletes) {
    return ((s32)maxSimultaneousDeletes - 1) * 2;
}

s32 ScoreBustingHpRatio(u16 currentHp, u16 maxHp) {
    if (currentHp < (maxHp >> 1))
        return 0;
    if (currentHp < ((maxHp >> 1) + (maxHp >> 2)))
        return 1;
    if (currentHp < maxHp)
        return 2;
    return 3;
}

u8 ClampBustingLevel(s32 rawScore) {
    if (rawScore < BUSTING_RANK_MIN)
        return BUSTING_RANK_MIN;
    if (rawScore > BUSTING_RANK_S)
        return BUSTING_RANK_S;
    return (u8)rawScore;
}
