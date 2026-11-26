/*
 * Portable C 8-Entrant Tournament Bracket Helpers (Phase C3).
 *
 * Models MMBN4's 8-entrant single-elimination tournament bracket
 * (Den Battle, Eagle/Hawk, Red Sun/Blue Moon, Free Tournament):
 *   - Round 0 (QF): pairs (0,1), (2,3), (4,5), (6,7)
 *   - Round 1 (SF): winners of {0,1} vs {2,3}, and {4,5} vs {6,7}
 *   - Round 2 (Final): winner of top half {0..3} vs bottom half {4..7}
 */
#include "modern_api.h"

/*
 * Return the partner slot index for `slot` in Round 0 (Quarterfinals).
 */
u8 BracketQfOpponentSlot(u8 slot) {
    return (u8)((slot & 7u) ^ 1u);
}

/*
 * Return 1 if `slot` has won round `roundIdx` (0=QF, 1=SF, 2=Final).
 */
u32 BracketHasWonRound(const struct TournamentBracketSlot bracket[8],
                       u8 slot, u8 roundIdx) {
    if (bracket == 0 || slot >= 8u || roundIdx >= 3u) {
        return 0u;
    }
    return (bracket[slot].roundBits & (u8)(1u << roundIdx)) ? 1u : 0u;
}

/*
 * Mark `slot` as winner of `roundIdx` (0..2).
 */
void BracketMarkRoundWin(struct TournamentBracketSlot bracket[8],
                         u8 slot, u8 roundIdx) {
    if (bracket == 0 || slot >= 8u || roundIdx >= 3u) {
        return;
    }
    bracket[slot].roundBits |= (u8)(1u << roundIdx);
}

/*
 * Find the entrantId of the tournament champion (first slot with bit 2 set),
 * or 0xFF if no champion has been crowned yet.
 */
u8 BracketGetChampion(const struct TournamentBracketSlot bracket[8]) {
    u32 i;
    if (bracket == 0) {
        return 0xFFu;
    }
    for (i = 0; i < 8u; i++) {
        if ((bracket[i].roundBits & 0x04u) != 0u) {
            return bracket[i].entrantId;
        }
    }
    return 0xFFu;
}
