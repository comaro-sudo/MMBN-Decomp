/*
 * Portable C Extended Chip Attack & Defense Mechanics (Phase D2).
 *
 * Implements additional iconic MMBN4 battle chip mechanics:
 *   - AreaGrab: steals the frontmost enemy column on the 6x3 battle grid
 *   - ShockWave: ground-traveling wave that stops on holes/missing panels
 *   - Boomerang: perimeter-sweeping projectile along outer rows/columns
 *   - Barrier / Aura: damage absorption & wind-break check
 *   - Invisible: timed invincibility frame countdown
 */
#include "modern_api.h"

/*
 * AreaGrab: finds the leftmost enemy-owned column (x in 1..6) and converts
 * all non-occupied panels in that column to player-owned (owner = 0).
 * Never steals column 6 if it is the enemy's last column (must leave >= 1
 * enemy column unless `allowLastColumn` is non-zero).
 * Returns the number of panels stolen (0..3).
 */
u32 ChipAreaGrabStealColumn(struct BattlePanel grid[18], u8 allowLastColumn) {
    u8 x;
    u8 y;
    u8 targetCol = 0u;
    u8 enemyCols = 0u;
    u32 stolen = 0u;

    for (x = 1u; x <= 6u; x++) {
        u32 hasEnemy = 0u;
        for (y = 1u; y <= 3u; y++) {
            struct BattlePanel *p = &grid[BattleGridIndex(x, y)];
            if (p->owner == 1u) {
                hasEnemy = 1u;
                break;
            }
        }
        if (hasEnemy) {
            if (targetCol == 0u) {
                targetCol = x;
            }
            enemyCols++;
        }
    }

    if (targetCol == 0u || (!allowLastColumn && enemyCols <= 1u)) {
        return 0u;
    }

    for (y = 1u; y <= 3u; y++) {
        struct BattlePanel *p = &grid[BattleGridIndex(targetCol, y)];
        if (p->owner == 1u) {
            p->owner = 0u;
            stolen++;
        }
    }
    return stolen;
}

/*
 * ShockWave step: advances wave from (*x, y) by `dir` (+1 or -1).
 * Stops (returns 0) if the next panel is out of bounds or broken/hole
 * (state == 2). Returns 1 if the wave successfully advances.
 */
u32 ChipShockWaveStep(const struct BattlePanel grid[18], s8 *x, s8 y, s8 dir) {
    s8 nx = (s8)(*x + dir);
    const struct BattlePanel *p;

    if (!BattleGridInBounds((u8)nx, (u8)y)) {
        return 0u;
    }
    p = &grid[BattleGridIndex((u8)nx, (u8)y)];
    if (p->state == 2u) {
        return 0u;
    }
    *x = nx;
    return 1u;
}

/*
 * Boomerang perimeter sweep step (clockwise from bottom-left (1,3) -> (6,3)
 * -> (6,1) -> (1,1)). Updates (*x, *y) to next perimeter cell.
 * Returns 1 while sweeping, 0 when it exits after (1,1).
 */
u32 ChipBoomerangPerimeterStep(s8 *x, s8 *y) {
    if (*y == 3 && *x < 6) {
        (*x)++;
        return 1u;
    }
    if (*x == 6 && *y > 1) {
        (*y)--;
        return 1u;
    }
    if (*y == 1 && *x > 1) {
        (*x)--;
        return 1u;
    }
    return 0u;
}

/*
 * Barrier absorption: subtracts `damage` from `*barrierHp`.
 * If `isWind` is non-zero, immediately shatters the barrier (sets to 0)
 * without leaking damage. Otherwise returns any overflow damage (> 0)
 * that breaks through when `*barrierHp` reaches 0.
 */
s32 ChipBarrierAbsorbHit(s16 *barrierHp, s32 damage, u8 isWind) {
    if (*barrierHp <= 0 || damage <= 0) {
        return damage > 0 ? damage : 0;
    }
    if (isWind) {
        *barrierHp = 0;
        return 0;
    }
    if (damage >= (s32)*barrierHp) {
        s32 overflow = damage - (s32)*barrierHp;
        *barrierHp = 0;
        return overflow;
    }
    *barrierHp = (s16)(*barrierHp - damage);
    return 0;
}

/*
 * Invisible frame timer tick: decrements `*invisFrames` if > 0.
 * Returns 1 if actor is still invisible after ticking, 0 otherwise.
 */
u32 ChipInvisTick(u16 *invisFrames) {
    if (*invisFrames > 0u) {
        (*invisFrames)--;
    }
    return (*invisFrames > 0u) ? 1u : 0u;
}
