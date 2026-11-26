/*
 * Portable C Soul Navi Boss AI Pattern Helpers (Phase D3).
 *
 * Implements signature attack & positioning helpers for MMBN4's Soul Navi
 * bosses on the 6x3 battle grid (code_08 / code_09):
 *   - GutsMan: GutsPunch panel crack & Break impact
 *   - ProtoMan: StepSword teleport panel lookup in front of MegaMan
 *   - FireMan: Cross-pattern flame hitbox (center + 4 orthogonal neighbors)
 *   - SearchMan: Scope-lock reticle hit check
 */
#include "modern_api.h"

/*
 * GutsMan GutsHammer / GutsPunch: cracks the two panels directly in front
 * of GutsMan (gutsX - 1 and gutsX - 2 on gutsY). Returns panels cracked.
 */
u32 BossGutsPunchCrack(struct BattlePanel grid[18], s8 gutsX, s8 gutsY) {
    u32 cracked = 0u;
    s8 dx;

    for (dx = 1; dx <= 2; dx++) {
        s8 tx = (s8)(gutsX - dx);
        if (BattleGridInBounds((u8)tx, (u8)gutsY)) {
            BattlePanelCrack(&grid[BattleGridIndex((u8)tx, (u8)gutsY)], 0);
            cracked++;
        }
    }
    return cracked;
}

/*
 * ProtoMan StepSword teleport: returns 1 and writes (playerX + 1, playerY)
 * if the panel in front of MegaMan is in-bounds, or 0 otherwise.
 */
u32 BossProtoStepTarget(s8 playerX, s8 playerY, s8 *outX, s8 *outY) {
    s8 tx = (s8)(playerX + 1);
    if (!BattleGridInBounds((u8)tx, (u8)playerY)) {
        return 0u;
    }
    if (outX != 0) {
        *outX = tx;
    }
    if (outY != 0) {
        *outY = playerY;
    }
    return 1u;
}

/*
 * FireMan / Cross-pattern explosion centered at (cx, cy): hits center plus
 * the 4 orthogonal neighbors (Manhattan distance <= 1).
 */
u32 BossCrossExplosionHits(s8 cx, s8 cy, s8 playerX, s8 playerY) {
    return (MathManhattan((s32)cx, (s32)cy, (s32)playerX, (s32)playerY) <= 1) ? 1u : 0u;
}

/*
 * SearchMan Scope-lock: returns 1 if the moving reticle (scopeX, scopeY)
 * locks onto MegaMan at (playerX, playerY).
 */
u32 BossSearchScopeLocks(s8 scopeX, s8 scopeY, s8 playerX, s8 playerY) {
    return (scopeX == playerX && scopeY == playerY) ? 1u : 0u;
}
