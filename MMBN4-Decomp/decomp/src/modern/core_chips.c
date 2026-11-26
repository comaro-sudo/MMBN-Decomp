/*
 * Portable C chip & buster mechanics helpers (Phase D2).
 *
 * Covers MMBN4's buster stats, DarkChip penalties, and chip-effect scaling
 * (code_07 / code_08):
 *   - Buster Attack / Rapid / Charge stat clamping (1..5)
 *   - Charge shot damage & charge frame duration
 *   - DarkChip max-HP permanent -1 penalty (floored at 1) & karma drop
 *   - DoubleSoul sacrifice chip eligibility check
 */
#include "battle.h"

#define BUSTER_STAT_MIN 1u
#define BUSTER_STAT_MAX 5u

/*
 * Clamp a buster stat (Attack, Rapid, or Charge) to MMBN4's valid 1..5 range.
 */
u8 BusterClampStat(s32 rawStat) {
    if (rawStat < (s32)BUSTER_STAT_MIN) {
        return (u8)BUSTER_STAT_MIN;
    }
    if (rawStat > (s32)BUSTER_STAT_MAX) {
        return (u8)BUSTER_STAT_MAX;
    }
    return (u8)rawStat;
}

/*
 * Standard MegaBuster charge shot damage: Attack * 10 (10..50).
 */
u16 BusterChargeDamage(u8 attackStat) {
    return (u16)(BusterClampStat((s32)attackStat) * 10u);
}

/*
 * Frames required to reach full buster charge for Charge level 1..5.
 * Level 1 = 90 frames (1.5s), decreasing by 15 frames per level down to 30 frames.
 */
u16 BusterChargeFrames(u8 chargeStat) {
    u8 lvl = BusterClampStat((s32)chargeStat);
    return (u16)(105u - (u16)lvl * 15u);
}

/*
 * Apply MMBN4's permanent -1 Max HP penalty when using a DarkChip.
 * Max HP never drops below 1; current HP is clamped to the new Max HP.
 * Returns 1 if Max HP decreased, 0 if already at floor (1).
 */
u32 DarkChipApplyHpPenalty(u16 *maxHp, u16 *curHp) {
    if (maxHp == 0 || *maxHp <= 1u) {
        return 0;
    }
    *maxHp = (u16)(*maxHp - 1u);
    if (curHp != 0 && *curHp > *maxHp) {
        *curHp = *maxHp;
    }
    return 1;
}

/*
 * Return 1 if a chip with given element and chipClass (0=Std, 1=Mega, 2=Giga,
 * 3=Dark) can be sacrificed to activate a DoubleSoul requiring targetElem.
 * Only Standard chips (chipClass == 0) of matching element may be sacrificed.
 */
u32 ChipCanSacrificeForSoul(u8 chipElement, u8 chipClass, u8 targetElem) {
    if (chipClass != 0u || targetElem == 0u) {
        return 0;
    }
    return (chipElement == targetElem) ? 1u : 0u;
}
