/*
 * Portable C Duo & Bass Boss Attack State Machines (Phase D3).
 *
 * Implements concrete boss attack pattern logic for MMBN4's final & postgame
 * bosses (code_08 / code_09):
 *   - Duo: Meteor Knuckle target lock & panel crack, Giga Laser row sweep
 *   - Bass: DarkArmBlade 2x3 front-zone slash hitbox, LifeAura 200 threshold check
 */
#include "modern_api.h"

/*
 * Duo Meteor Knuckle: cracks the panel at (targetX, targetY) on impact and
 * returns 1 if the player is standing on that panel (direct hit).
 */
u32 BossDuoMeteorImpact(struct BattlePanel grid[18],
                        s8 targetX, s8 targetY, s8 playerX, s8 playerY) {
    if (!BattleGridInBounds((u8)targetX, (u8)targetY)) {
        return 0u;
    }
    BattlePanelCrack(&grid[BattleGridIndex((u8)targetX, (u8)targetY)], 0);
    return (targetX == playerX && targetY == playerY) ? 1u : 0u;
}

/*
 * Duo Giga Laser: locks onto `laserRow` (1..3) and sweeps all player columns
 * (x in 1..3) in that row. Returns 1 if (playerX, playerY) is struck.
 */
u32 BossDuoGigaLaserHits(s8 laserRow, s8 playerX, s8 playerY) {
    return (playerY == laserRow && playerX >= 1 && playerX <= 3) ? 1u : 0u;
}

/*
 * Bass DarkArmBlade: teleports in front of MegaMan (at bassX) and slashes
 * a 2x3 zone covering columns [bassX - 2, bassX - 1] across all 3 rows.
 * Returns 1 if (playerX, playerY) is inside the slash zone.
 */
u32 BossBassDarkArmBladeHits(s8 bassX, s8 playerX, s8 playerY) {
    if (playerY < 1 || playerY > 3) {
        return 0u;
    }
    return (playerX == (s8)(bassX - 1) || playerX == (s8)(bassX - 2)) ? 1u : 0u;
}

/*
 * Return 1 if an incoming hit of `damage` breaks Bass's LifeAura (threshold
 * 200 in normal/Omega, or 300 in BassXX / SP+ variants).
 */
u32 BossBassAuraBreaks(s32 damage, s32 auraThreshold, u8 isWind) {
    if (isWind) {
        return 1u;
    }
    return (damage >= auraThreshold) ? 1u : 0u;
}
