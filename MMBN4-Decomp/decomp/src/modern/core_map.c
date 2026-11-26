/*
 * Portable C overworld map, warp, and random encounter helpers (Phase C3).
 *
 * Covers MMBN4's overworld area/subarea encoding, warp target validation,
 * and random-encounter step counter / SneakRun suppression (code_01).
 */
#include "toolkit.h"

struct MapWarpTarget {
    u8 mapGroup;
    u8 mapId;
    s16 x;
    s16 y;
    s16 z;
    u8 facing;
};

/*
 * Pack (mapGroup, mapId) into MMBN4's 16-bit map key (group << 8 | id).
 */
u16 MapPackKey(u8 mapGroup, u8 mapId) {
    return (u16)(((u16)mapGroup << 8) | (u16)mapId);
}

/*
 * Return 1 if the given mapGroup is in the Cyberworld (Internet / Comps:
 * groups >= 0x80), or 0 for Real World maps (groups < 0x80).
 */
u32 MapIsCyberworld(u8 mapGroup) {
    return (mapGroup >= 0x80u) ? 1u : 0u;
}

/*
 * Apply a warp target to the GameState's current map fields.
 */
void MapApplyWarp(struct GameState *gs, const struct MapWarpTarget *warp) {
    gs->mapGroup = warp->mapGroup;
    gs->mapNumber = warp->mapId;
}

/*
 * Advance the random-encounter step accumulator by stepDelta on a Cyberworld
 * map. If threshold is reached (and SneakRun does not suppress the encounter),
 * resets the counter and returns 1 (trigger battle); otherwise returns 0.
 *
 * SneakRun suppresses an encounter when active and playerMaxHp > enemyMaxHp.
 */
u32 MapStepEncounter(u16 *stepCounter, u16 stepDelta, u16 threshold,
                     u8 sneakRunActive, u16 playerMaxHp, u16 enemyMaxHp) {
    u32 next;

    if (stepCounter == 0 || threshold == 0u) {
        return 0;
    }
    next = (u32)(*stepCounter) + (u32)stepDelta;
    if (next < (u32)threshold) {
        *stepCounter = (u16)next;
        return 0;
    }
    *stepCounter = 0u;
    if (sneakRunActive && playerMaxHp > enemyMaxHp) {
        return 0;
    }
    return 1;
}
