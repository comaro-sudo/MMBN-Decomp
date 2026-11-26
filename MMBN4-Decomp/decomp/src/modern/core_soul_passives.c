/*
 * Portable C DoubleSoul & Violet Eclipse Duo Soul Passive Traits (Phase D1/D2).
 *
 * Encodes passive combat flags granted by each of MMBN4's 12 DoubleSouls plus
 * Violet Eclipse's 13th Duo Soul (AirShoes + FloatShoes + B+Left Reflect).
 */
#include "battle.h"

enum SoulPassiveFlags {
    SOUL_PASSIVE_NONE        = 0u,
    SOUL_PASSIVE_AIR_SHOES   = (1u << 0),
    SOUL_PASSIVE_FLOAT_SHOES = (1u << 1),
    SOUL_PASSIVE_SUPER_ARMOR = (1u << 2),
    SOUL_PASSIVE_SHIELD      = (1u << 3), /* B+Left Shield / Reflect */
    SOUL_PASSIVE_ICE_NO_SLIP = (1u << 4),
    SOUL_PASSIVE_PLUS30_NULL = (1u << 5), /* GutsSoul +30 non-dimming Null chips */
    SOUL_PASSIVE_SHUFFLE     = (1u << 6)  /* SearchSoul Custom Screen shuffle */
};

#define SOUL_ID_DUO 13u

/*
 * Return the bitmask of passive combat traits active for `soulId`
 * (0=Normal, 1..12=Red Sun / Blue Moon DoubleSouls, 13=Violet Eclipse Duo Soul).
 */
u32 SoulGetPassiveFlags(u8 soulId) {
    switch (soulId) {
    case SOUL_GUTS:
        return SOUL_PASSIVE_SUPER_ARMOR | SOUL_PASSIVE_PLUS30_NULL;
    case SOUL_WIND:
        return SOUL_PASSIVE_AIR_SHOES | SOUL_PASSIVE_FLOAT_SHOES;
    case SOUL_SEARCH:
        return SOUL_PASSIVE_SHUFFLE;
    case SOUL_AQUA:
        return SOUL_PASSIVE_ICE_NO_SLIP;
    case SOUL_METAL:
        return SOUL_PASSIVE_SUPER_ARMOR;
    case SOUL_PROTO:
        return SOUL_PASSIVE_SHIELD;
    case SOUL_ID_DUO:
        return SOUL_PASSIVE_AIR_SHOES | SOUL_PASSIVE_FLOAT_SHOES | SOUL_PASSIVE_SHIELD;
    default:
        return SOUL_PASSIVE_NONE;
    }
}

/*
 * Return bonus damage added to a non-dimming Null-element chip by `soulId`
 * (GutsSoul grants +30; NumberSoul grants +10).
 */
u16 SoulGetNullChipBonus(u8 soulId) {
    if (soulId == SOUL_GUTS) {
        return 30u;
    }
    if (soulId == SOUL_NUMBER) {
        return 10u;
    }
    return 0u;
}
