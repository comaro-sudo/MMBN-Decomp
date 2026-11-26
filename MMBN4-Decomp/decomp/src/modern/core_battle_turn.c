/*
 * Portable C Battle Turn & Phase Coordinator (Phase D1).
 *
 * Ties together MMBN4's top-level battle state machine (code_07):
 *   - Phases: INTRO -> CUSTOM -> COMBAT -> VICTORY / DEFEAT / ESCAPE
 *   - Turn counter & DoubleSoul 3-turn duration countdown
 *   - Frame counter & Custom Gauge full -> Custom Screen re-entry
 */
#include "gba_types.h"

u32 CustomGaugeStep(u16 *gauge, u16 speed);

enum BattlePhase {
    BATTLE_PHASE_INTRO   = 0,
    BATTLE_PHASE_CUSTOM  = 1,
    BATTLE_PHASE_COMBAT  = 2,
    BATTLE_PHASE_VICTORY = 3,
    BATTLE_PHASE_DEFEAT  = 4,
    BATTLE_PHASE_ESCAPE  = 5
};

struct BattleTurnState {
    u8 phase;
    u8 turnNumber;
    u8 activeSoul;        /* 0 = Normal, 1..13 = active DoubleSoul */
    u8 soulTurnsLeft;     /* starts at 3 when uniting; reverts to 0 on expiry */
    u16 customGauge;
    u16 gaugeSpeed;
    u32 totalCombatFrames;
    u8 enemiesAlive;
    u8 playerDeleted;
};

/*
 * Initialize a new battle in INTRO phase with given enemy count & gauge speed.
 */
void BattleTurnInit(struct BattleTurnState *bt, u8 enemiesAlive, u16 gaugeSpeed) {
    bt->phase = BATTLE_PHASE_INTRO;
    bt->turnNumber = 0u;
    bt->activeSoul = 0u;
    bt->soulTurnsLeft = 0u;
    bt->customGauge = 0u;
    bt->gaugeSpeed = gaugeSpeed;
    bt->totalCombatFrames = 0u;
    bt->enemiesAlive = enemiesAlive;
    bt->playerDeleted = 0u;
}

/*
 * Transition from INTRO or COMBAT into the Custom Screen.
 * Decrements active DoubleSoul turn counter (3-turn duration) and reverts
 * to Normal form when soulTurnsLeft hits 0.
 */
void BattleTurnEnterCustom(struct BattleTurnState *bt) {
    if (bt->phase == BATTLE_PHASE_VICTORY
     || bt->phase == BATTLE_PHASE_DEFEAT
     || bt->phase == BATTLE_PHASE_ESCAPE) {
        return;
    }
    if (bt->phase == BATTLE_PHASE_COMBAT && bt->activeSoul != 0u) {
        if (bt->soulTurnsLeft > 0u) {
            bt->soulTurnsLeft--;
        }
        if (bt->soulTurnsLeft == 0u) {
            bt->activeSoul = 0u;
        }
    }
    bt->turnNumber++;
    bt->customGauge = 0u;
    bt->phase = BATTLE_PHASE_CUSTOM;
}

/*
 * Commit Custom Screen selection (optionally activating a new DoubleSoul for
 * 3 turns) and enter COMBAT phase.
 */
void BattleTurnStartCombat(struct BattleTurnState *bt, u8 newSoulId) {
    if (bt->phase != BATTLE_PHASE_CUSTOM) {
        return;
    }
    if (newSoulId != 0u) {
        bt->activeSoul = newSoulId;
        bt->soulTurnsLeft = 3u;
    }
    bt->phase = BATTLE_PHASE_COMBAT;
}

/*
 * Advance 1 frame of COMBAT phase.
 * Checks win/loss conditions, advances combat frame timer & Custom Gauge,
 * and returns 1 if the Custom Gauge is ready (player may press L/R to open
 * Custom Screen), or 0 otherwise.
 */
u32 BattleTurnStepCombat(struct BattleTurnState *bt) {
    if (bt->phase != BATTLE_PHASE_COMBAT) {
        return 0;
    }
    if (bt->playerDeleted) {
        bt->phase = BATTLE_PHASE_DEFEAT;
        return 0;
    }
    if (bt->enemiesAlive == 0u) {
        bt->phase = BATTLE_PHASE_VICTORY;
        return 0;
    }
    bt->totalCombatFrames++;
    return CustomGaugeStep(&bt->customGauge, bt->gaugeSpeed);
}
