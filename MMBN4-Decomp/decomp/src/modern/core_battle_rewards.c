/*
 * Portable C Post-Battle Reward & Violet Eclipse Drop Resolver (Phase D1/D3).
 *
 * Models MMBN4 + Violet Eclipse battle reward calculations:
 *   - Busting-level reward tier (1..3=Low, 4..6=Mid, 7..8=High, 9..10=Top, 11=S)
 *   - Zenny multiplier (vanilla 1.0x vs Violet Eclipse WP-A2 1.5x)
 *   - Counter-hit BugFrag bonus
 *   - Violet Eclipse SP+ 80/20 reward roll (15,000Z vs 50 BugFrags)
 */
#include "gba_types.h"

u32 StepRNG(u32 *rng);

enum RewardTier {
    REWARD_TIER_LOW  = 0, /* Busting 1..3 */
    REWARD_TIER_MID  = 1, /* Busting 4..6 */
    REWARD_TIER_HIGH = 2, /* Busting 7..8 */
    REWARD_TIER_TOP  = 3, /* Busting 9..10 */
    REWARD_TIER_S    = 4  /* Busting S (11) */
};

struct SpPlusReward {
    u32 zenny;
    u32 bugFrags;
};

/*
 * Map a clamped Busting Level (1..11, where 11 = S) to its reward table tier.
 */
u8 BattleRewardTierFromBusting(u8 bustingLevel) {
    if (bustingLevel >= 11u) {
        return REWARD_TIER_S;
    }
    if (bustingLevel >= 9u) {
        return REWARD_TIER_TOP;
    }
    if (bustingLevel >= 7u) {
        return REWARD_TIER_HIGH;
    }
    if (bustingLevel >= 4u) {
        return REWARD_TIER_MID;
    }
    return REWARD_TIER_LOW;
}

/*
 * Scale a base Zenny drop by Violet Eclipse's WP-A2 1.5x multiplier
 * (base + base/2) when `violetEclipse` is non-zero.
 */
u32 BattleScaleZenny(u32 baseZenny, u8 violetEclipse) {
    if (!violetEclipse) {
        return baseZenny;
    }
    return baseZenny + (baseZenny >> 1);
}

/*
 * Roll the Violet Eclipse SP+ boss rematch reward:
 * 80% chance -> 15,000 Zenny; 20% chance -> 50 BugFrags.
 */
struct SpPlusReward BattleRollSpPlusReward(u32 *rngSeed) {
    struct SpPlusReward r = { 0u, 0u };
    u32 roll = StepRNG(rngSeed) % 100u;

    if (roll < 80u) {
        r.zenny = 15000u;
    } else {
        r.bugFrags = 50u;
    }
    return r;
}
