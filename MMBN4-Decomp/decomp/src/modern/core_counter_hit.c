/*
 * Portable C Counter-Hit & DarkChip Custom-Screen Offer Rules (Phase D1/D2).
 *
 * Models MMBN4's DarkChip offer trigger in the Custom Screen and
 * Counter-Hit resolution (Paralyze stun + Full Synchro transition).
 */
#include "modern_api.h"

#define COUNTER_PARALYZE_FRAMES 90u /* 1.5s stun on Counter Hit */

/*
 * Return 1 if MMBN4 should offer a DarkChip in the Custom Screen this turn:
 *   - Disabled if `darkChipsBanned != 0`
 *   - Offered when HP <= 50% of MaxHP, OR emotion is ANXIOUS/EVIL/DARK_SOUL
 */
u32 DarkChipShouldOffer(u16 curHp, u16 maxHp, enum EmotionState emotion,
                        u8 darkChipsBanned) {
    if (darkChipsBanned || maxHp == 0u) {
        return 0u;
    }
    if (((u32)curHp * 2u) <= (u32)maxHp) {
        return 1u;
    }
    if (emotion == EMOTION_ANXIOUS
     || emotion == EMOTION_EVIL
     || emotion == EMOTION_DARK_SOUL) {
        return 1u;
    }
    return 0u;
}

/*
 * Resolve a Counter Hit landed on `boss` with `bossStatus`:
 *   - If `BossCheckCounterHit(boss)` triggers:
 *       * Inflicts Paralyze (90 frames) on `bossStatus` (if not StatusGuarded)
 *       * Updates player emotion via `EmotionOnCounterHit(*playerEmotion, karma)`
 *       * Returns 1
 *   - Otherwise returns 0.
 */
u32 BattleResolveCounterHit(struct BossPatternState *boss,
                            struct BattleStatusState *bossStatus,
                            enum EmotionState *playerEmotion,
                            s16 playerKarma) {
    if (!BossCheckCounterHit(boss)) {
        return 0u;
    }
    if (bossStatus != 0) {
        StatusInflict(bossStatus, 1u /* STATUS_PARALYZE */, COUNTER_PARALYZE_FRAMES);
    }
    if (playerEmotion != 0) {
        *playerEmotion = EmotionOnCounterHit(*playerEmotion, playerKarma);
    }
    return 1u;
}

/*
 * Return 1 if MegaMan is in a 2x chip-damage emotion state
 * (Full Synchro or Anger).
 */
u32 BattleIsDoubleDamageEmotion(enum EmotionState emotion) {
    return (EmotionChipDamageMultiplier(emotion) >= 2u) ? 1u : 0u;
}

/*
 * Return 1 if a DarkSoul takeover should trigger (evil karma + DarkChip use).
 */
u32 BattleShouldTriggerDarkSoul(s16 karma, u8 usedDarkChipThisTurn) {
    return (usedDarkChipThisTurn && EmotionIsEvilKarma(karma)) ? 1u : 0u;
}

/*
 * Compute remaining HP percentage (0..100) for HUD / AI threshold checks.
 */
u8 BattleCalcHpPercent(u16 curHp, u16 maxHp) {
    u32 pct;
    if (maxHp == 0u) {
        return 0u;
    }
    pct = ((u32)curHp * 100u) / (u32)maxHp;
    return (pct > 100u) ? 100u : (u8)pct;
}

/*
 * Return 1 if HP is in critical "low HP" warning range (<= 25% of MaxHP).
 */
u32 BattleIsLowHp(u16 curHp, u16 maxHp) {
    return (maxHp > 0u && ((u32)curHp * 4u) <= (u32)maxHp) ? 1u : 0u;
}
