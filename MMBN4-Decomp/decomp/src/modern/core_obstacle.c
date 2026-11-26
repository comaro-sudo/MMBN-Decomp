/*
 * Portable C Battle Grid Obstacles (RockCube / Push / Break) (Phase D2).
 *
 * Models MMBN4's destructible grid obstacles on the 6x3 battle grid:
 *   - RockCube (200 HP) taking damage & shattering
 *   - AirShot / Wind / GutsPunch horizontal push & 200-damage collision
 */
#include "modern_api.h"

/*
 * Initialize an obstacle at (x, y) with given HP and slide collision damage.
 */
void ObstacleSpawn(struct BattleObstacle *ob, s8 x, s8 y, s16 hp, u16 slideDamage) {
    ob->active = 1u;
    ob->x = x;
    ob->y = y;
    ob->hp = hp;
    ob->slideDamage = slideDamage;
}

/*
 * Apply `damage` to an obstacle. Destroys it when hp <= 0.
 * Returns 1 if destroyed, 0 if still intact or inactive.
 */
u32 ObstacleTakeDamage(struct BattleObstacle *ob, s32 damage) {
    if (ob == 0 || !ob->active || damage <= 0) {
        return 0u;
    }
    if (damage >= (s32)ob->hp) {
        ob->hp = 0;
        ob->active = 0u;
        return 1u;
    }
    ob->hp = (s16)(ob->hp - damage);
    return 0u;
}

/*
 * Push an obstacle horizontally in direction `dx` (+1 or -1) across the 6x3
 * grid toward a target at (targetX, targetY).
 *   - If it collides with the target in its lane, destroys the obstacle and
 *     returns `slideDamage` (e.g. 200 for RockCube + AirShot).
 *   - Otherwise slides to the edge of the grid and deactivates, returning 0.
 */
u16 ObstaclePushSlide(struct BattleObstacle *ob, s8 dx, s8 targetX, s8 targetY) {
    if (ob == 0 || !ob->active || (dx != 1 && dx != -1)) {
        return 0u;
    }
    while (1) {
        s8 nx = (s8)(ob->x + dx);
        if (!BattleGridInBounds((u8)nx, (u8)ob->y)) {
            ob->active = 0u;
            return 0u;
        }
        ob->x = nx;
        if (ob->x == targetX && ob->y == targetY) {
            u16 dmg = ob->slideDamage;
            ob->active = 0u;
            return dmg;
        }
    }
}
