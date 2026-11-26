/*
 * Portable C NaviCust Placement & Bug-Detection Rules (Phase C4/D1).
 *
 * Models MMBN4's 4 core NaviCust bug rules on the 5x5 grid:
 *   1. Adjacent (4-connected) distinct parts of the same color -> Bug
 *   2. Non-solid (program) part not touching Command Line (row 2) -> Bug
 *   3. Solid (plus/stat) part touching Command Line (row 2) -> Bug
 *   4. More than 4 distinct colors installed -> Bug
 */
#include "modern_api.h"

#define NAVICUST_BUG_SAME_COLOR   (1u << 0)
#define NAVICUST_BUG_OFF_CMD_LINE (1u << 1)
#define NAVICUST_BUG_ON_CMD_LINE  (1u << 2)
#define NAVICUST_BUG_TOO_MANY_COL (1u << 3)

/*
 * Check if any two 4-adjacent cells on the 5x5 grid belong to DIFFERENT
 * partIds that share the same color. Returns 1 if a same-color clash exists.
 */
u32 NaviCustHasColorClash(const u8 gridPartIds[25], const u8 gridColors[25]) {
    u32 y, x;
    for (y = 0; y < 5u; y++) {
        for (x = 0; x < 5u; x++) {
            u32 idx = y * 5u + x;
            u8 pid = gridPartIds[idx];
            u8 col = gridColors[idx];
            if (pid == 0u || col == 0u) {
                continue;
            }
            if (x + 1u < 5u) {
                u32 r = idx + 1u;
                if (gridPartIds[r] != 0u && gridPartIds[r] != pid && gridColors[r] == col) {
                    return 1u;
                }
            }
            if (y + 1u < 5u) {
                u32 d = idx + 5u;
                if (gridPartIds[d] != 0u && gridPartIds[d] != pid && gridColors[d] == col) {
                    return 1u;
                }
            }
        }
    }
    return 0u;
}

/*
 * Count how many distinct non-zero colors are present on the 5x5 grid.
 */
u32 NaviCustCountDistinctColors(const u8 gridColors[25]) {
    u32 mask = 0u;
    u32 count = 0u;
    u32 i;

    for (i = 0; i < 25u; i++) {
        u8 c = gridColors[i];
        if (c > 0u && c < 32u) {
            mask |= (1u << c);
        }
    }
    while (mask != 0u) {
        count += (mask & 1u);
        mask >>= 1;
    }
    return count;
}

/*
 * Evaluate all 4 NaviCust bug rules for a single part (`desc`) given whether
 * it touches row 2 (`touchesCmdLine`) and the full 5x5 grid arrays.
 * Returns a bitmask of NAVICUST_BUG_* flags (0 = bug-free).
 */
u32 NaviCustEvalPartBugs(const struct NaviCustPartDesc *desc,
                         u8 touchesCmdLine,
                         const u8 gridPartIds[25],
                         const u8 gridColors[25]) {
    u32 bugs = 0u;

    if (desc != 0 && desc->partId != 0u) {
        if (!desc->isSolid && !touchesCmdLine) {
            bugs |= NAVICUST_BUG_OFF_CMD_LINE;
        }
        if (desc->isSolid && touchesCmdLine) {
            bugs |= NAVICUST_BUG_ON_CMD_LINE;
        }
    }
    if (NaviCustHasColorClash(gridPartIds, gridColors)) {
        bugs |= NAVICUST_BUG_SAME_COLOR;
    }
    if (NaviCustCountDistinctColors(gridColors) > 4u) {
        bugs |= NAVICUST_BUG_TOO_MANY_COL;
    }
    return bugs;
}
