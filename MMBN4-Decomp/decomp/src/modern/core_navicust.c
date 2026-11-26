/* Phase C4 / E2 portable C reference implementations of MMBN4 NaviCust 5x5
 * grid and placed-program helpers (Toolkit +0x5C/+0x60/+0x64).
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "toolkit.h"

#define NAVICUST_GRID_DIM     5u
#define NAVICUST_GRID_CELLS   25u
#define NAVICUST_MAX_PROGRAMS 25u

struct NaviCustPlacedProgram {
    u8 programId; /* +0x00: 0 = empty */
    u8 color;     /* +0x01 */
    u8 x;         /* +0x02: 0..4 */
    u8 y;         /* +0x03: 0..4 */
    u8 rotation;  /* +0x04: 0..3 */
    u8 compressed;/* +0x05 */
    u8 pad06[2];  /* +0x06 */
};

void NaviCustClearGrid(struct Toolkit *tk) {
    u32 i;
    for (i = 0; i < NAVICUST_GRID_CELLS; i++)
        tk->naviCustGrid[i] = 0;
}

int NaviCustSetCell(struct Toolkit *tk, u8 x, u8 y, u8 slotIndex) {
    if (x >= NAVICUST_GRID_DIM || y >= NAVICUST_GRID_DIM)
        return 0;
    tk->naviCustGrid[y * NAVICUST_GRID_DIM + x] = slotIndex;
    return 1;
}

u8 NaviCustGetCell(const struct Toolkit *tk, u8 x, u8 y) {
    if (x >= NAVICUST_GRID_DIM || y >= NAVICUST_GRID_DIM)
        return 0;
    return tk->naviCustGrid[y * NAVICUST_GRID_DIM + x];
}

/* Counts how many of the 25 placed-program slots (+0x60, 25 x 8B) are active. */
u32 NaviCustCountPlaced(const struct Toolkit *tk) {
    const struct NaviCustPlacedProgram *list =
        (const struct NaviCustPlacedProgram *)tk->naviCustPlaced;
    u32 i, count = 0;
    for (i = 0; i < NAVICUST_MAX_PROGRAMS; i++) {
        if (list[i].programId != 0)
            count++;
    }
    return count;
}

/* Returns 1 if any of the 25 cells on the central Command Line (row y == 2)
 * contains `slotIndex`. */
int NaviCustOnCommandLine(const struct Toolkit *tk, u8 slotIndex) {
    u8 x;
    if (slotIndex == 0)
        return 0;
    for (x = 0; x < NAVICUST_GRID_DIM; x++) {
        if (tk->naviCustGrid[2u * NAVICUST_GRID_DIM + x] == slotIndex)
            return 1;
    }
    return 0;
}
