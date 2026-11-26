/*
 * Portable C Overworld Map Tile & Collision Grid (Phase C3).
 *
 * Models overworld map tile collision, warp triggers, and conveyor/elevation
 * queries for native C map loading (code_01).
 */
#include "modern_api.h"

/*
 * Return the tile attribute byte at (tx, ty), or MAP_TILE_BLOCKED (1) if
 * out of bounds.
 */
u8 MapGridGetTile(const struct MapCollisionGrid *grid, s32 tx, s32 ty) {
    if (grid == 0 || grid->tiles == 0) {
        return 1u; /* MAP_TILE_BLOCKED */
    }
    if (tx < 0 || ty < 0 || (u32)tx >= grid->width || (u32)ty >= grid->height) {
        return 1u;
    }
    return grid->tiles[(u32)ty * grid->width + (u32)tx];
}

/*
 * Return 1 if the tile at (tx, ty) is walkable (0=Normal, 2=Warp, 3=Ice).
 */
u32 MapGridIsWalkable(const struct MapCollisionGrid *grid, s32 tx, s32 ty) {
    u8 t = MapGridGetTile(grid, tx, ty);
    return (t == 0u || t == 2u || t == 3u) ? 1u : 0u;
}

/*
 * Return 1 if stepping onto (tx, ty) triggers a map warp (tile == 2).
 */
u32 MapGridIsWarpTile(const struct MapCollisionGrid *grid, s32 tx, s32 ty) {
    return (MapGridGetTile(grid, tx, ty) == 2u) ? 1u : 0u;
}

/*
 * Attempt to step an overworld actor from (*tx, *ty) in 8-way direction `dir`.
 * If the destination tile is walkable, updates (*tx, *ty) and returns 1;
 * otherwise leaves position unchanged and returns 0.
 */
u32 MapGridTryStep(const struct MapCollisionGrid *grid, s32 *tx, s32 *ty, u8 dir) {
    s8 dx = 0, dy = 0;
    s32 nx, ny;

    if (tx == 0 || ty == 0) {
        return 0u;
    }
    MathFacingToDelta(dir, &dx, &dy);
    nx = *tx + (s32)dx;
    ny = *ty + (s32)dy;
    if (!MapGridIsWalkable(grid, nx, ny)) {
        return 0u;
    }
    *tx = nx;
    *ty = ny;
    return 1u;
}
