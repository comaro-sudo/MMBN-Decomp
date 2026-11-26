/*
 * Portable C MegaMan Stat Aggregation & NaviCust Bonus Calculator (Phase C4/D1).
 *
 * Computes effective combat stats from base progression + NaviCust parts:
 *   - Max HP (base 100 + HPMemory*20 + NaviCust HP bonuses - DarkChip penalty,
 *     clamped to 1..9999)
 *   - Custom Screen hand size (base 5 + Custom+ bonus, clamped to 5..10)
 *   - Mega / Giga chip folder limits (base 5/1 + Folder+ bonuses)
 */
#include "gba_types.h"

#define NAVI_BASE_HP      100
#define NAVI_HP_MEM_STEP  20
#define NAVI_MAX_HP_CAP   9999
#define CUSTOM_HAND_BASE  5u
#define CUSTOM_HAND_CAP   10u

struct NaviBuildBonuses {
    u16 hpMemoryCount;
    s16 naviCustHpBonus;
    u16 darkChipPenalty;
    u8 customPlus;
    u8 megaFolderPlus;
    u8 gigaFolderPlus;
};

/*
 * Compute MegaMan's effective Max HP, clamped to [1, 9999].
 */
u16 NaviCalcMaxHp(const struct NaviBuildBonuses *b) {
    s32 hp;

    if (b == 0) {
        return (u16)NAVI_BASE_HP;
    }
    hp = NAVI_BASE_HP
       + (s32)b->hpMemoryCount * NAVI_HP_MEM_STEP
       + (s32)b->naviCustHpBonus
       - (s32)b->darkChipPenalty;
    if (hp < 1) {
        hp = 1;
    }
    if (hp > NAVI_MAX_HP_CAP) {
        hp = NAVI_MAX_HP_CAP;
    }
    return (u16)hp;
}

/*
 * Compute Custom Screen hand size (base 5 + Custom+ parts, capped at 10).
 */
u8 NaviCalcCustomHandSize(const struct NaviBuildBonuses *b) {
    u32 hand = CUSTOM_HAND_BASE + (b ? (u32)b->customPlus : 0u);
    if (hand > CUSTOM_HAND_CAP) {
        hand = CUSTOM_HAND_CAP;
    }
    return (u8)hand;
}

/*
 * Compute MegaChip folder limit (base 5 + MegaFolder+ parts, capped at 10).
 */
u8 NaviCalcMegaLimit(const struct NaviBuildBonuses *b) {
    u32 lim = 5u + (b ? (u32)b->megaFolderPlus : 0u);
    return (lim > 10u) ? 10u : (u8)lim;
}

/*
 * Compute GigaChip folder limit (base 1 + GigaFolder+ parts, capped at 5).
 */
u8 NaviCalcGigaLimit(const struct NaviBuildBonuses *b) {
    u32 lim = 1u + (b ? (u32)b->gigaFolderPlus : 0u);
    return (lim > 5u) ? 5u : (u8)lim;
}
