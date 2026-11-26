/*
 * Portable C NetBattle & Tournament Match Ruleset (Phase D1/E2).
 *
 * Models MMBN4's PvP / Free Tournament match rules:
 *   - 15-turn time limit tiebreaker by remaining HP percentage
 *   - Best-of-3 series win counter & match-complete check
 */
#include "modern_api.h"

#define NETBATTLE_MAX_TURNS 15u

/*
 * Resolve winner at the 15-turn Sudden Death limit by remaining HP ratio
 * (hp * 1000 / maxHp). Returns:
 *   1 = player 1 wins, 2 = player 2 wins, 0 = draw.
 */
u32 NetBattleResolveTiebreak(u16 p1Hp, u16 p1MaxHp, u16 p2Hp, u16 p2MaxHp) {
    u32 r1 = (p1MaxHp > 0u) ? ((u32)p1Hp * 1000u / (u32)p1MaxHp) : 0u;
    u32 r2 = (p2MaxHp > 0u) ? ((u32)p2Hp * 1000u / (u32)p2MaxHp) : 0u;

    if (r1 > r2) {
        return 1u;
    }
    if (r2 > r1) {
        return 2u;
    }
    return 0u;
}

/*
 * Return 1 if the NetBattle has reached or exceeded the 15-turn limit.
 */
u32 NetBattleIsTurnLimitReached(u8 turnNumber) {
    return (turnNumber >= NETBATTLE_MAX_TURNS) ? 1u : 0u;
}

/*
 * Record a round win (winner == 1 or 2) in a best-of-`targetWins` series
 * (e.g. targetWins=2 for Best-of-3). Returns the series winner (1 or 2)
 * once reached, or 0 if the series is still ongoing.
 */
u32 NetBattleRecordRoundWin(u8 *p1Wins, u8 *p2Wins, u8 winner, u8 targetWins) {
    if (winner == 1u && p1Wins != 0) {
        (*p1Wins)++;
    } else if (winner == 2u && p2Wins != 0) {
        (*p2Wins)++;
    }
    if (p1Wins != 0 && *p1Wins >= targetWins) {
        return 1u;
    }
    if (p2Wins != 0 && *p2Wins >= targetWins) {
        return 2u;
    }
    return 0u;
}
