/*
 * Portable C Overworld NPC Collision & Talk-Ray Helpers (Phase C3).
 *
 * Models overworld actor bounding-box overlap and facing-direction talk
 * interaction checks (code_01 / code_06).
 */
#include "modern_api.h"

/*
 * Return 1 if two 2D axis-aligned boxes centered at (ax, ay) and (bx, by)
 * with half-extents (ahw, ahh) and (bhw, bhh) overlap.
 */
u32 OverworldAabbOverlap(s32 ax, s32 ay, s32 ahw, s32 ahh,
                         s32 bx, s32 by, s32 bhw, s32 bhh) {
    if (MathAbsS32(ax - bx) >= (ahw + bhw)) {
        return 0;
    }
    if (MathAbsS32(ay - by) >= (ahh + bhh)) {
        return 0;
    }
    return 1;
}

/*
 * Return 1 if a player at (px, py) facing 8-way direction `facing` can talk
 * to an NPC at (nx, ny) within `reach` pixels along the facing vector.
 */
u32 OverworldCanTalkToNpc(s32 px, s32 py, u8 facing,
                          s32 nx, s32 ny, s32 reach, s32 npcRadius) {
    s8 dx = 0, dy = 0;
    s32 probeX, probeY;

    MathFacingToDelta(facing, &dx, &dy);
    probeX = px + (s32)dx * reach;
    probeY = py + (s32)dy * reach;
    return OverworldAabbOverlap(probeX, probeY, 4, 4, nx, ny, npcRadius, npcRadius);
}
