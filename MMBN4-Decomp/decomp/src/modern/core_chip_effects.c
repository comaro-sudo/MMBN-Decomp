/*
 * Portable C Battle Grid Chip Effects (Phase D2).
 *
 * Models MMBN4's classic grid-manipulation chips on the 6x3 battle grid:
 *   - AreaGrab: steals the leftmost column owned by the opponent (stopping
 *     before the last column so the opponent keeps at least 1 column)
 *   - Geddon1: cracks all normal/special panels (breaks already-cracked)
 *   - Sanctuary: turns all player-owned intact panels into Holy panels
 */
#include "modern_api.h"

/*
 * Player AreaGrab (owner=0) or Enemy AreaGrab (owner=1).
 * Steals the nearest opponent-owned column (preserving at least 1 column
 * for the defender). Returns number of panels stolen (0..3).
 */
u32 ChipApplyAreaGrab(struct BattlePanel grid[18], u8 attackerOwner) {
    s32 col, row;
    u32 stolen = 0;

    if (attackerOwner == 0u) {
        /* Scan left-to-right for leftmost enemy column (cols 1..5; col 6 stays enemy) */
        for (col = 1; col <= 5; col++) {
            u32 hasEnemy = 0;
            for (row = 1; row <= 3; row++) {
                if (grid[BattleGridIndex((u8)col, (u8)row)].owner == 1u) {
                    hasEnemy = 1u;
                }
            }
            if (hasEnemy) {
                for (row = 1; row <= 3; row++) {
                    struct BattlePanel *p = &grid[BattleGridIndex((u8)col, (u8)row)];
                    if (p->owner == 1u) {
                        p->owner = 0u;
                        stolen++;
                    }
                }
                break;
            }
        }
    } else {
        /* Enemy AreaGrab: scan right-to-left (cols 6..2; col 1 stays player) */
        for (col = 6; col >= 2; col--) {
            u32 hasPlayer = 0;
            for (row = 1; row <= 3; row++) {
                if (grid[BattleGridIndex((u8)col, (u8)row)].owner == 0u) {
                    hasPlayer = 1u;
                }
            }
            if (hasPlayer) {
                for (row = 1; row <= 3; row++) {
                    struct BattlePanel *p = &grid[BattleGridIndex((u8)col, (u8)row)];
                    if (p->owner == 0u) {
                        p->owner = 1u;
                        stolen++;
                    }
                }
                break;
            }
        }
    }
    return stolen;
}

/*
 * Geddon1: cracks every intact panel across the 6x3 grid.
 */
void ChipApplyGeddon1(struct BattlePanel grid[18]) {
    u32 i;
    for (i = 0; i < 18u; i++) {
        BattlePanelCrack(&grid[i], 0);
    }
}

/*
 * Sanctuary: sets every intact (not broken/hole) panel owned by `owner`
 * to PANEL_HOLY (7). Returns number of panels converted.
 */
u32 ChipApplySanctuary(struct BattlePanel grid[18], u8 owner) {
    u32 i;
    u32 count = 0;
    for (i = 0; i < 18u; i++) {
        if (grid[i].owner == owner && grid[i].state != 2u && grid[i].state != 3u) {
            grid[i].state = 7u; /* PANEL_HOLY */
            count++;
        }
    }
    return count;
}
