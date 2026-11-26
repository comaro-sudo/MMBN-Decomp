/*
 * Portable C Battle Panel Hazards & Terrain Interactions (Phase D1/D2).
 *
 * Implements MMBN4 6x3 battle grid panel state interactions:
 *   state 0 = Normal, 1 = Cracked, 2 = Broken/Hole, 3 = Ice,
 *   4 = Grass, 5 = Lava, 6 = Poison, 7 = Holy.
 */
#include "modern_api.h"

/*
 * Stepping off a Cracked panel (state == 1) breaks it into a Hole (state == 2).
 * Returns 1 if the panel broke, 0 otherwise.
 */
u32 PanelStepOffCrack(struct BattlePanel grid[18], u8 x, u8 y) {
    struct BattlePanel *p;
    if (!BattleGridInBounds(x, y)) {
        return 0u;
    }
    p = &grid[BattleGridIndex(x, y)];
    if (p->state == 1u) {
        p->state = 2u;
        return 1u;
    }
    return 0u;
}

/*
 * Ice panel slide: while standing on Ice (state == 3) without FloatShoes,
 * slides (*x, *y) along (dx, dy) across contiguous owned, non-broken panels
 * until landing on a non-Ice panel or hitting an edge/enemy/hole boundary.
 * Returns the number of panels slid.
 */
u32 PanelIceSlideStep(const struct BattlePanel grid[18], s8 *x, s8 *y,
                      s8 dx, s8 dy, u8 owner, u8 hasFloatShoes) {
    u32 slid = 0u;
    if (hasFloatShoes || (dx == 0 && dy == 0)) {
        return 0u;
    }
    while (BattleGridInBounds((u8)*x, (u8)*y) &&
           grid[BattleGridIndex((u8)*x, (u8)*y)].state == 3u) {
        s8 nx = (s8)(*x + dx);
        s8 ny = (s8)(*y + dy);
        const struct BattlePanel *next;
        if (!BattleGridInBounds((u8)nx, (u8)ny)) {
            break;
        }
        next = &grid[BattleGridIndex((u8)nx, (u8)ny)];
        if (next->owner != owner || next->state == 2u) {
            break;
        }
        *x = nx;
        *y = ny;
        slid++;
    }
    return slid;
}

/*
 * Lava panel damage (state == 5): stepping on Lava deals 50 damage unless
 * the actor is Fire-element (element == 1) or has FloatShoes.
 * Clamps *hp to >= 0 and returns damage dealt.
 */
s32 PanelApplyLavaDamage(const struct BattlePanel grid[18], u8 x, u8 y,
                         s16 *hp, u8 element, u8 hasFloatShoes) {
    if (!BattleGridInBounds(x, y) || element == 1u || hasFloatShoes) {
        return 0;
    }
    if (grid[BattleGridIndex(x, y)].state != 5u) {
        return 0;
    }
    if (*hp <= 50) {
        s32 dealt = (s32)*hp;
        *hp = 0;
        return dealt;
    }
    *hp = (s16)(*hp - 50);
    return 50;
}

/*
 * Poison panel tick (state == 6): drains 1 HP every 10 frames unless
 * the actor has FloatShoes. Returns 1 if HP drained this frame, 0 otherwise.
 */
u32 PanelTickPoisonDamage(const struct BattlePanel grid[18], u8 x, u8 y,
                          s16 *hp, u16 *poisonTimer, u8 hasFloatShoes) {
    if (!BattleGridInBounds(x, y) || hasFloatShoes || *hp <= 0) {
        *poisonTimer = 0u;
        return 0u;
    }
    if (grid[BattleGridIndex(x, y)].state != 6u) {
        *poisonTimer = 0u;
        return 0u;
    }
    (*poisonTimer)++;
    if (*poisonTimer >= 10u) {
        *poisonTimer = 0u;
        (*hp)--;
        return 1u;
    }
    return 0u;
}

/*
 * Grass panel Wood-Navi regeneration (state == 4): heals 1 HP every 10 frames
 * up to maxHp when element == 4 (Wood). Returns 1 if healed this frame.
 */
u32 PanelTickGrassHeal(const struct BattlePanel grid[18], u8 x, u8 y,
                       s16 *hp, s16 maxHp, u16 *healTimer, u8 element) {
    if (!BattleGridInBounds(x, y) || element != 4u || *hp <= 0 || *hp >= maxHp) {
        *healTimer = 0u;
        return 0u;
    }
    if (grid[BattleGridIndex(x, y)].state != 4u) {
        *healTimer = 0u;
        return 0u;
    }
    (*healTimer)++;
    if (*healTimer >= 10u) {
        *healTimer = 0u;
        (*hp)++;
        return 1u;
    }
    return 0u;
}
