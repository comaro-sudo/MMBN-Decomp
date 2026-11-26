/*
 * Portable C Friendly Tournament & Lobby Rematch Progression (Phase C3/E2).
 *
 * Models Violet Eclipse's friendly tournament matches (9 matches across
 * 3 venues, round-gated, retry-on-loss, win grants Soul & hides NPC) and
 * the 5-tier boss rematch ladder (V1 -> V2 -> V3 -> SP -> SP+).
 */
#include "modern_api.h"

#define FRIENDLY_MATCH_MAX_ROUND 2u
#define REMATCH_MAX_TIER         4u /* 0=V1, 1=V2, 2=V3, 3=SP, 4=SP+ */

/*
 * Return 1 if a friendly tournament match gated on `requiredRound` (0..2)
 * is currently available: player is in `currentRound >= requiredRound` and
 * the match's completion flag (`clearedFlag`) is not yet set.
 */
u32 FriendlyMatchIsAvailable(const struct Toolkit *tk, u8 currentRound,
                             u8 requiredRound, u16 clearedFlag) {
    if (tk == 0 || requiredRound > FRIENDLY_MATCH_MAX_ROUND) {
        return 0u;
    }
    if (currentRound < requiredRound) {
        return 0u;
    }
    return EventFlagTest(tk, clearedFlag) ? 0u : 1u;
}

/*
 * Resolve a friendly match outcome. On victory (`playerWon != 0`), sets both
 * `clearedFlag` (hides NPC) and `soulFlag` (grants opponent's DoubleSoul)
 * and returns 1. On loss, leaves flags untouched so the player can retry.
 */
u32 FriendlyMatchResolve(struct Toolkit *tk, u8 playerWon,
                         u16 clearedFlag, u16 soulFlag) {
    if (tk == 0 || !playerWon) {
        return 0u;
    }
    EventFlagSet(tk, clearedFlag);
    if (soulFlag != 0u) {
        EventFlagSet(tk, soulFlag);
    }
    return 1u;
}

/*
 * Advance a lobby/Duo rematch rank (0..4) after a battle victory.
 * Uses BossAdvanceRematchRank (S-rank required for Duo; any win for lobby).
 */
u8 RematchLadderOnWin(u8 currentTier, u8 bustingLevel, u8 isDuo) {
    if (currentTier >= REMATCH_MAX_TIER) {
        return REMATCH_MAX_TIER;
    }
    return BossAdvanceRematchRank(currentTier, bustingLevel, isDuo ? 1 : 0);
}
