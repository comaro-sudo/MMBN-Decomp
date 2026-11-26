/* Phase D1 / D2 / E2 portable C reference implementations of MMBN4 Custom
 * Screen mechanics: Custom Gauge progression, chip-selection compatibility
 * (same chip ID, same code, or '*' wildcard), and Program Advance detection.
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "gba_types.h"

#define CUSTOM_GAUGE_MAX     0x4000u
#define CUSTOM_MAX_SELECT    5u
#define CHIP_CODE_ASTERISK   26u

struct CustomChipSlot {
    u16 chipId;
    u8 code; /* 0='A'..25='Z', 26='*' */
    u8 selected;
};

struct ProgramAdvanceRecipe {
    u16 paChipId;
    u16 chips[3];
    u8 codes[3]; /* 0xFF = any sequential/matching code, else exact code */
};

/* Advances Custom Gauge by speed step (normal=32, FastGauge=64, SlowGauge=16).
 * Returns 1 when full. */
int CustomGaugeStep(u16 *gauge, u16 step) {
    u32 next = (u32)(*gauge) + step;
    if (next >= CUSTOM_GAUGE_MAX) {
        *gauge = CUSTOM_GAUGE_MAX;
        return 1;
    }
    *gauge = (u16)next;
    return 0;
}

/* Returns 1 if `candidate` can be added to the current selection of `selected`
 * chips (max 5; must either match all selected chip IDs, or share a non-'*'
 * code with all non-'*' selected chips). */
int CustomCanSelectChip(
    const struct CustomChipSlot *selected,
    u32 selectedCount,
    u16 chipId,
    u8 code
) {
    u32 i;
    int allSameId = 1;
    u8 lockedCode = CHIP_CODE_ASTERISK;

    if (selectedCount >= CUSTOM_MAX_SELECT)
        return 0;
    if (selectedCount == 0)
        return 1;

    for (i = 0; i < selectedCount; i++) {
        if (selected[i].chipId != chipId)
            allSameId = 0;
        if (selected[i].code != CHIP_CODE_ASTERISK) {
            if (lockedCode == CHIP_CODE_ASTERISK)
                lockedCode = selected[i].code;
            else if (lockedCode != selected[i].code)
                lockedCode = 0xFFu; /* already mixed codes via same-ID rule */
        }
    }

    if (allSameId)
        return 1;
    if (lockedCode == 0xFFu)
        return 0;
    if (code == CHIP_CODE_ASTERISK || lockedCode == CHIP_CODE_ASTERISK)
        return 1;
    return code == lockedCode;
}

/* Checks whether a 3-chip consecutive slice matches a Program Advance recipe
 * (exact chip IDs and either exact codes or consecutive ascending codes A-B-C). */
int CustomMatchesPA(
    const struct CustomChipSlot *seq,
    const struct ProgramAdvanceRecipe *recipe
) {
    int i;
    for (i = 0; i < 3; i++) {
        if (seq[i].chipId != recipe->chips[i])
            return 0;
    }
    if (recipe->codes[0] == 0xFFu) {
        /* Sequential code P.A. (e.g. Cannon A, B, C) */
        if (seq[0].code >= 24u)
            return 0;
        return (seq[1].code == (u8)(seq[0].code + 1u)
                && seq[2].code == (u8)(seq[0].code + 2u));
    }
    for (i = 0; i < 3; i++) {
        if (seq[i].code != recipe->codes[i])
            return 0;
    }
    return 1;
}

/* Scans a selected chip list for the first matching P.A.; returns recipe->paChipId
 * and writes start index to `*outStart`, or returns 0 if none matches. */
u16 CustomFindPA(
    const struct CustomChipSlot *selected,
    u32 selectedCount,
    const struct ProgramAdvanceRecipe *recipes,
    u32 recipeCount,
    u32 *outStart
) {
    u32 i, r;
    if (selectedCount < 3u)
        return 0;
    for (i = 0; i + 3u <= selectedCount; i++) {
        for (r = 0; r < recipeCount; r++) {
            if (CustomMatchesPA(&selected[i], &recipes[r])) {
                if (outStart)
                    *outStart = i;
                return recipes[r].paChipId;
            }
        }
    }
    return 0;
}
