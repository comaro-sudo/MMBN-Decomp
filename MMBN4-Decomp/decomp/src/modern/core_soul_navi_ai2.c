/*
 * Portable C Extended Soul Navi Boss Mechanics (Phase D3).
 *
 * Covers additional MMBN4 Soul Navi boss mechanics on the 6x3 battle grid:
 *   - WindMan: Fan pull/push horizontal shift on player column (1..3)
 *   - ThunderMan: Homing thunder-ball 1-step pursuit toward MegaMan
 *   - Roll: RollArrow hit destroys player's queued next chip if present
 *   - AquaMan: AquaHose 2-panel horizontal splash check
 *   - NumberMan: DiceBomb random 1..6 damage roll (face * 30)
 */
#include "modern_api.h"

/*
 * WindMan Fan pull (+1 toward enemy) or push (-1 toward back row) on `playerX`,
 * clamped to player-owned columns [1..3]. Returns new playerX.
 */
s8 BossWindApplyGust(s8 playerX, s8 dir) {
    s8 nx = (s8)(playerX + dir);
    if (nx < 1) {
        nx = 1;
    }
    if (nx > 3) {
        nx = 3;
    }
    return nx;
}

/*
 * ThunderMan homing orb: steps 1 panel orthogonally closer to (playerX, playerY)
 * (prioritizing horizontal alignment first).
 */
void BossThunderOrbStep(s8 *orbX, s8 *orbY, s8 playerX, s8 playerY) {
    if (orbX == 0 || orbY == 0) {
        return;
    }
    if (*orbX != playerX) {
        *orbX = (*orbX > playerX) ? (s8)(*orbX - 1) : (s8)(*orbX + 1);
    } else if (*orbY != playerY) {
        *orbY = (*orbY > playerY) ? (s8)(*orbY - 1) : (s8)(*orbY + 1);
    }
}

/*
 * Roll Arrow hit: if the player has at least 1 queued chip (`queuedChips > 0`),
 * destroys the front chip and returns 1; otherwise returns 0.
 */
u32 BossRollDestroyChip(u8 *queuedChips) {
    if (queuedChips == 0 || *queuedChips == 0u) {
        return 0u;
    }
    (*queuedChips)--;
    return 1u;
}

/*
 * NumberMan DiceBomb: rolls die face 1..6 via StepRNG and returns damage
 * (`face * basePerPip`, e.g. 30..180).
 */
u16 BossNumberRollDiceDamage(u32 *rngSeed, u16 basePerPip) {
    u32 face = (StepRNG(rngSeed) % 6u) + 1u;
    return (u16)(face * (u32)basePerPip);
}

/*
 * AquaMan AquaHose splash: hits (hitX, hitY) and (hitX - 1, hitY).
 */
u32 BossAquaHoseHits(s8 hitX, s8 hitY, s8 playerX, s8 playerY) {
    if (playerY != hitY) {
        return 0u;
    }
    return (playerX == hitX || playerX == (s8)(hitX - 1)) ? 1u : 0u;
}
