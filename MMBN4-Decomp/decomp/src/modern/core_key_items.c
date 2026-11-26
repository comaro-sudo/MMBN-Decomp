/*
 * Portable C PET Upgrades & Key-Item Capacity Helpers (Phase C4).
 *
 * Models MMBN4's permanent PET upgrades:
 *   - RegMemory (base 4 MB + RegUP1/2/3, capped at 50 MB)
 *   - SubMemory (base 4 SubChips per slot + SubMem, capped at 8)
 *   - SpinParts bitmask (White/Yellow/Pink/Red/Blue/Green rotation unlocks)
 */
#include "modern_api.h"

#define REG_MEM_MAX 50u
#define SUB_MEM_MAX 8u

/*
 * Add `deltaMb` (1, 2, or 3 from RegUP1/2/3) to Regular Memory, capped at 50 MB.
 * Returns the new Regular Memory value.
 */
u8 PetApplyRegUp(u8 *regMemory, u8 deltaMb) {
    u32 next;

    if (regMemory == 0) {
        return 0u;
    }
    next = (u32)(*regMemory) + (u32)deltaMb;
    if (next > REG_MEM_MAX) {
        next = REG_MEM_MAX;
    }
    *regMemory = (u8)next;
    return *regMemory;
}

/*
 * Increase SubChip per-slot capacity by 1 (SubMem), capped at 8.
 * Returns 1 if upgraded, 0 if already at max (8).
 */
u32 PetApplySubMem(u8 *subMemCap) {
    if (subMemCap == 0 || *subMemCap >= SUB_MEM_MAX) {
        return 0u;
    }
    (*subMemCap)++;
    return 1u;
}

/*
 * Check whether NaviCust program color `colorIndex` (0..5) can be rotated
 * given the owned SpinParts bitmask.
 */
u32 PetCanRotateColor(u8 spinPartsMask, u8 colorIndex) {
    if (colorIndex >= 6u) {
        return 0u;
    }
    return (spinPartsMask & (u8)(1u << colorIndex)) ? 1u : 0u;
}
