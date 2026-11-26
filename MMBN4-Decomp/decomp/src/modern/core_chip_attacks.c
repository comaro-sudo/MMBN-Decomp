/*
 * Portable C Concrete Chip Attack Hitbox & State Machines (Phase D2).
 *
 * Implements concrete targeting & hit patterns for iconic MMBN4 chips
 * on the 6x3 battle grid (x in 1..6, y in 1..3):
 *   - Cannon / HiCannon / M-Cannon: instant horizontal raycast stopping at first target
 *   - WideSword (1x3 vertical slash at x+1)
 *   - LongSword (2x1 horizontal thrust at x+1, x+2)
 *   - MiniBomb / Lobbed Bomb: lands at (px + range, py), bypassing middle panels
 */
#include "modern_api.h"

/*
 * Cannon-family raycast from (px, py) rightward (+1) against `targets[0..count)`.
 * Returns the index of the closest hit target in row `py` with x > px, or -1 on miss.
 */
s32 ChipCannonFindTarget(s8 px, s8 py, const s8 *targetX, const s8 *targetY,
                         const u8 *targetAlive, u32 count) {
    s32 bestIdx = -1;
    s8 bestX = 127;
    u32 i;

    for (i = 0; i < count; i++) {
        if (!targetAlive[i] || targetY[i] != py) {
            continue;
        }
        if (targetX[i] > px && targetX[i] < bestX) {
            bestX = targetX[i];
            bestIdx = (s32)i;
        }
    }
    return bestIdx;
}

/*
 * WideSword hitbox: hits (px + 1, py - 1..py + 1). Returns 1 if (tx, ty) is hit.
 */
u32 ChipWideSwordHits(s8 px, s8 py, s8 tx, s8 ty) {
    if (tx != (s8)(px + 1)) {
        return 0u;
    }
    return (MathAbsS32((s32)ty - (s32)py) <= 1) ? 1u : 0u;
}

/*
 * LongSword hitbox: hits (px + 1, py) and (px + 2, py). Returns 1 if (tx, ty) is hit.
 */
u32 ChipLongSwordHits(s8 px, s8 py, s8 tx, s8 ty) {
    if (ty != py) {
        return 0u;
    }
    return (tx == (s8)(px + 1) || tx == (s8)(px + 2)) ? 1u : 0u;
}

/*
 * Lobbed bomb (MiniBomb / BlackBomb): lands at (px + throwDist, py).
 * Returns 1 if the landing panel is in-bounds and matches (tx, ty).
 */
u32 ChipLobBombHits(s8 px, s8 py, s8 throwDist, s8 tx, s8 ty) {
    s8 lx = (s8)(px + throwDist);
    if (!BattleGridInBounds((u8)lx, (u8)py)) {
        return 0u;
    }
    return (lx == tx && py == ty) ? 1u : 0u;
}
