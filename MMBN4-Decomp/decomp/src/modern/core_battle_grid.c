/* Phase D1 / D2 / E2 portable C reference implementations of MMBN4 6x3 battle
 * panel grid addressing, panel ownership/state transitions, and elemental
 * weakness calculation (Fire > Wood > Elec > Aqua > Fire).
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "gba_types.h"

#define BATTLE_GRID_COLS 6u
#define BATTLE_GRID_ROWS 3u

enum BattleElement {
    ELEM_NONE = 0,
    ELEM_FIRE = 1,
    ELEM_AQUA = 2,
    ELEM_ELEC = 3,
    ELEM_WOOD = 4,
};

enum BattlePanelState {
    PANEL_NORMAL  = 0,
    PANEL_CRACKED = 1,
    PANEL_BROKEN  = 2,
    PANEL_HOLE    = 3,
    PANEL_GRASS   = 4,
    PANEL_ICE     = 5,
    PANEL_LAVA    = 6,
    PANEL_HOLY    = 7,
    PANEL_POISON  = 8,
    PANEL_METAL   = 9,
};

struct BattlePanel {
    u8 owner; /* 0 = player (red), 1 = enemy (blue) */
    u8 state; /* enum BattlePanelState */
    u16 crackTimer;
};

/* MMBN4 battle coordinates are 1-based: col in 1..6, row in 1..3. */
int BattleGridInBounds(u8 col, u8 row) {
    return (col >= 1u && col <= BATTLE_GRID_COLS && row >= 1u && row <= BATTLE_GRID_ROWS);
}

int BattleGridIndex(u8 col, u8 row) {
    if (!BattleGridInBounds(col, row))
        return -1;
    return (int)((row - 1u) * BATTLE_GRID_COLS + (col - 1u));
}

void BattleGridInitDefault(struct BattlePanel grid[BATTLE_GRID_ROWS * BATTLE_GRID_COLS]) {
    u8 r, c;
    for (r = 1; r <= BATTLE_GRID_ROWS; r++) {
        for (c = 1; c <= BATTLE_GRID_COLS; c++) {
            int idx = BattleGridIndex(c, r);
            grid[idx].owner = (c <= 3u) ? 0u : 1u;
            grid[idx].state = PANEL_NORMAL;
            grid[idx].crackTimer = 0;
        }
    }
}

/* Returns 1 if `attackerElem` deals 2x weakness damage to `defenderElem`. */
int BattleIsElementWeakness(u8 attackerElem, u8 defenderElem) {
    if (attackerElem == ELEM_FIRE && defenderElem == ELEM_WOOD) return 1;
    if (attackerElem == ELEM_WOOD && defenderElem == ELEM_ELEC) return 1;
    if (attackerElem == ELEM_ELEC && defenderElem == ELEM_AQUA) return 1;
    if (attackerElem == ELEM_AQUA && defenderElem == ELEM_FIRE) return 1;
    return 0;
}

/* Computes final hit damage with elemental weakness (2x), Grass+Fire (2x),
 * Ice+Elec (2x), and Holy panel (1/2x). */
u16 BattleCalcHitDamage(u16 baseDamage, u8 attackerElem, u8 defenderElem, u8 panelState) {
    u32 dmg = baseDamage;
    if (BattleIsElementWeakness(attackerElem, defenderElem))
        dmg *= 2u;
    if (panelState == PANEL_GRASS && attackerElem == ELEM_FIRE)
        dmg *= 2u;
    else if (panelState == PANEL_ICE && attackerElem == ELEM_ELEC)
        dmg *= 2u;
    if (panelState == PANEL_HOLY)
        dmg >>= 1;
    if (dmg > 9999u)
        dmg = 9999u;
    return (u16)dmg;
}

/* Applies a crack/break effect to a panel (Metal/Hole immune). */
void BattlePanelCrack(struct BattlePanel *panel, int forceBreak) {
    if (panel->state == PANEL_METAL || panel->state == PANEL_HOLE)
        return;
    if (forceBreak || panel->state == PANEL_CRACKED) {
        panel->state = PANEL_BROKEN;
        panel->crackTimer = 600u;
    } else {
        panel->state = PANEL_CRACKED;
    }
}

int BattlePanelIsWalkable(const struct BattlePanel *panel, u8 hasAirShoes, u8 hasFloatShoes) {
    if (panel->state == PANEL_BROKEN || panel->state == PANEL_HOLE)
        return hasAirShoes != 0;
    (void)hasFloatShoes;
    return 1;
}
