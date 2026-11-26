/*
 * Portable C SubChip inventory & effect helpers (Phase C4).
 *
 * Models MMBN4's 6 SubChips (MiniEnrg, FullEnrg, SneakRun, Untrap,
 * LocEnemy, Unlocker):
 *   - Capacity clamping (default 4, expandable via SubMem upgrades up to 8)
 *   - MiniEnrg (+50 HP) & FullEnrg (full HP restore) usage rules
 *   - SneakRun / Untrap / LocEnemy activation flags
 */
#include "gba_types.h"

enum SubChipId {
    SUBCHIP_MINI_ENRG = 0,
    SUBCHIP_FULL_ENRG = 1,
    SUBCHIP_SNEAK_RUN = 2,
    SUBCHIP_UNTRAP    = 3,
    SUBCHIP_LOC_ENEMY = 4,
    SUBCHIP_UNLOCKER  = 5,
    SUBCHIP_COUNT     = 6
};

struct SubChipState {
    u8 qty[SUBCHIP_COUNT];
    u8 maxPerSlot;      /* default 4, up to 8 with SubMem */
    u8 sneakRunActive;
    u8 untrapActive;
    u8 locEnemyActive;
};

/*
 * Add `amount` of a SubChip, clamped to maxPerSlot.
 * Returns the actual number added.
 */
u32 SubChipAdd(struct SubChipState *sc, u8 id, u8 amount) {
    u32 cap, cur, added;

    if (sc == 0 || id >= SUBCHIP_COUNT || amount == 0u) {
        return 0;
    }
    cap = sc->maxPerSlot;
    cur = sc->qty[id];
    if (cur >= cap) {
        return 0;
    }
    added = (cur + amount > cap) ? (cap - cur) : (u32)amount;
    sc->qty[id] = (u8)(cur + added);
    return added;
}

/*
 * Use MiniEnrg (+50 HP) or FullEnrg (full HP).
 * Returns 1 if consumed and HP healed, 0 if none in stock or already at max HP.
 */
u32 SubChipUseRecovery(struct SubChipState *sc, u8 id, u16 *curHp, u16 maxHp) {
    u32 heal;

    if (sc == 0 || curHp == 0 || (id != SUBCHIP_MINI_ENRG && id != SUBCHIP_FULL_ENRG)) {
        return 0;
    }
    if (sc->qty[id] == 0u || *curHp >= maxHp) {
        return 0;
    }
    sc->qty[id]--;
    heal = (id == SUBCHIP_MINI_ENRG) ? 50u : (u32)maxHp;
    if ((u32)(*curHp) + heal > (u32)maxHp) {
        *curHp = maxHp;
    } else {
        *curHp = (u16)(*curHp + heal);
    }
    return 1;
}

/*
 * Activate an overworld SubChip (SneakRun, Untrap, or LocEnemy).
 * Returns 1 if consumed and activated, 0 if out of stock or already active.
 */
u32 SubChipActivateField(struct SubChipState *sc, u8 id) {
    u8 *flag = 0;

    if (sc == 0 || id >= SUBCHIP_COUNT || sc->qty[id] == 0u) {
        return 0;
    }
    if (id == SUBCHIP_SNEAK_RUN) {
        flag = &sc->sneakRunActive;
    } else if (id == SUBCHIP_UNTRAP) {
        flag = &sc->untrapActive;
    } else if (id == SUBCHIP_LOC_ENEMY) {
        flag = &sc->locEnemyActive;
    } else {
        return 0;
    }
    if (*flag != 0u) {
        return 0;
    }
    sc->qty[id]--;
    *flag = 1u;
    return 1;
}
