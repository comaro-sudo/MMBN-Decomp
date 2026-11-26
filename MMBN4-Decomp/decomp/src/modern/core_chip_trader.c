/*
 * Portable C Chip Trader & BugFrag Trader helpers (Phase C4).
 *
 * Models MMBN4's Chip Traders (3-chip, 10-chip Special, BugFrag Trader):
 *   - Cost & stock validation (3 chips, 10 chips, or 10 BugFrags)
 *   - Weighted table roll using MMBN4's StepRNG
 */
#include "gba_types.h"

u32 StepRNG(u32 *rng);

enum TraderKind {
    TRADER_3CHIP   = 0,
    TRADER_10CHIP  = 1,
    TRADER_BUGFRAG = 2
};

struct TraderPoolEntry {
    u16 chipId;
    u8 code;
    u8 weight;
};

/*
 * Return the required input count (chips or BugFrags) for a trader kind.
 */
u32 TraderRequiredCost(u8 kind) {
    if (kind == TRADER_3CHIP) {
        return 3u;
    }
    if (kind == TRADER_10CHIP || kind == TRADER_BUGFRAG) {
        return 10u;
    }
    return 0u;
}

/*
 * Check whether the player has enough chips/BugFrags to spin the trader.
 */
u32 TraderCanSpin(u8 kind, u32 available) {
    u32 cost = TraderRequiredCost(kind);
    return (cost > 0u && available >= cost) ? 1u : 0u;
}

/*
 * Roll a weighted reward entry from `pool[0..poolLen)` using `StepRNG`.
 * Returns pointer to the winning TraderPoolEntry, or NULL if pool is empty
 * or total weight is 0.
 */
const struct TraderPoolEntry *TraderRoll(const struct TraderPoolEntry *pool,
                                         u32 poolLen, u32 *rngSeed) {
    u32 totalWeight = 0u;
    u32 i, roll, acc = 0u;

    if (pool == 0 || poolLen == 0u || rngSeed == 0) {
        return 0;
    }
    for (i = 0; i < poolLen; i++) {
        totalWeight += pool[i].weight;
    }
    if (totalWeight == 0u) {
        return 0;
    }
    roll = StepRNG(rngSeed) % totalWeight;
    for (i = 0; i < poolLen; i++) {
        acc += pool[i].weight;
        if (roll < acc) {
            return &pool[i];
        }
    }
    return &pool[poolLen - 1u];
}
