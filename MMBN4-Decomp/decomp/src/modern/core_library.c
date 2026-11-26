/*
 * Portable C Data Library & Title Screen Star helpers (Phase C4).
 *
 * Models MMBN4's chip library bitfields (Standard, Mega, Giga, P.A.) and
 * Title Screen completion star checks, including Violet Eclipse's 12-Soul
 * completion star rule.
 */
#include "gba_types.h"

#define LIB_STD_COUNT  150u
#define LIB_MEGA_COUNT 60u
#define LIB_GIGA_COUNT 5u
#define LIB_PA_COUNT   30u

/*
 * Test whether entry index `idx` is recorded in an MSB-first or LSB-first
 * library bitfield (standard LSB-first byte array: byte[idx >> 3] & (1 << (idx & 7))).
 */
u32 LibraryBitTest(const u8 *bits, u32 idx, u32 maxEntries) {
    if (bits == 0 || idx >= maxEntries) {
        return 0;
    }
    return (bits[idx >> 3] & (u8)(1u << (idx & 7u))) ? 1u : 0u;
}

/*
 * Mark entry index `idx` as recorded in the library bitfield.
 * Returns 1 if newly recorded, 0 if already set or out of range.
 */
u32 LibraryBitSet(u8 *bits, u32 idx, u32 maxEntries) {
    u8 mask;

    if (bits == 0 || idx >= maxEntries) {
        return 0;
    }
    mask = (u8)(1u << (idx & 7u));
    if ((bits[idx >> 3] & mask) != 0u) {
        return 0;
    }
    bits[idx >> 3] |= mask;
    return 1;
}

/*
 * Count how many entries in [0..maxEntries) are recorded in `bits`.
 */
u32 LibraryCountRecorded(const u8 *bits, u32 maxEntries) {
    u32 i;
    u32 count = 0;

    if (bits == 0) {
        return 0;
    }
    for (i = 0; i < maxEntries; i++) {
        if ((bits[i >> 3] & (u8)(1u << (i & 7u))) != 0u) {
            count++;
        }
    }
    return count;
}

/*
 * Return 1 if the Soul completion star should be awarded.
 * Vanilla requires 6 Souls; Violet Eclipse requires all 12 base Souls
 * (Duo Soul is a bonus 13th Soul and does not block the star).
 */
u32 LibraryHasSoulStar(u32 soulMask, u32 requiredCount) {
    u32 count = 0;
    u32 m = soulMask & 0x0FFFu; /* 12 base Souls (bits 0..11) */

    while (m != 0u) {
        count += (m & 1u);
        m >>= 1;
    }
    return (count >= requiredCount) ? 1u : 0u;
}
