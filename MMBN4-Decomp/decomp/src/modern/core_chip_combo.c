/*
 * Portable C Battle Chip Attack Modifier & Combo Pipeline (Phase D2).
 *
 * Models MMBN4's full chip damage stacking order:
 *   1. Base chip damage + flat NaviCust/Soul/Atk+10/Atk+30/DoublePoint/ColorPoint
 *   2. Emotion multiplier (2x on Full Synchro or Anger)
 *   3. Element weakness & panel multipliers (Grass/Ice/Holy via BattleCalcHitDamage)
 *   4. Clamped to [0, 9999]
 */
#include "modern_api.h"

/*
 * Accumulate flat damage modifiers onto `baseDamage` (clamped at 9999).
 * `panelsStolen` is used by DoublePoint/ColorPoint (+10 per panel).
 */
u16 ChipApplyFlatModifiers(u16 baseDamage, u16 flatBonus, u8 panelsStolen) {
    u32 total = (u32)baseDamage + (u32)flatBonus + ((u32)panelsStolen * 10u);
    if (total > 9999u) {
        total = 9999u;
    }
    return (u16)total;
}

/*
 * Compute final chip hit damage combining flat bonuses, Emotion Window
 * multiplier (Full Synchro / Anger = 2x), element weakness, and panel state.
 */
u16 ChipCalcFinalDamage(u16 baseDamage, u16 flatBonus,
                        enum EmotionState emotion,
                        u8 attackerElem, u8 defenderElem, u8 panelState) {
    u32 dmg = (u32)ChipApplyFlatModifiers(baseDamage, flatBonus, 0u);
    u8 emoMult = EmotionChipDamageMultiplier(emotion);

    dmg *= (u32)emoMult;
    if (dmg > 9999u) {
        dmg = 9999u;
    }
    dmg = (u32)BattleCalcHitDamage((u16)dmg, attackerElem, defenderElem, panelState);
    if (dmg > 9999u) {
        dmg = 9999u;
    }
    return (u16)dmg;
}
