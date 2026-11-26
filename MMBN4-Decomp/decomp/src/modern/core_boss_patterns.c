/*
 * Portable C Boss AI Phase & Pattern Helpers (Phase D3).
 *
 * Models key boss mechanics in MMBN4 (code_08 / code_09):
 *   - HP-threshold enrage / phase-2 transition (e.g. Duo < 50% HP)
 *   - Bass LifeAura regeneration cooldown
 *   - Boss telegraph / counter-hit window check (drives Full Synchro)
 */
#include "gba_types.h"

struct BossPatternState {
    s16 hp;
    s16 maxHp;
    u8 phase;             /* 0 = normal, 1 = enraged (< 50% HP) */
    u16 auraRegenTimer;   /* Bass Aura regeneration countdown */
    u16 auraRegenPeriod;  /* frames between Aura regenerations */
    u16 counterWindow;    /* >0 during attack startup frames (counterable) */
};

/*
 * Update boss phase based on current HP: enters Phase 1 (enraged) when
 * hp * 2 <= maxHp. Returns the current phase (0 or 1).
 */
u8 BossUpdatePhase(struct BossPatternState *b) {
    if (b == 0 || b->maxHp <= 0) {
        return 0;
    }
    if (b->phase == 0u && ((s32)b->hp * 2) <= (s32)b->maxHp) {
        b->phase = 1u;
    }
    return b->phase;
}

/*
 * Step Bass-style Aura regeneration timer while his Aura is broken
 * (`auraActive == 0`). Returns 1 when the Aura should regenerate.
 */
u32 BossTickAuraRegen(struct BossPatternState *b, u8 auraActive) {
    if (b == 0 || auraActive || b->auraRegenPeriod == 0u) {
        return 0;
    }
    if (b->auraRegenTimer > 0u) {
        b->auraRegenTimer--;
    }
    if (b->auraRegenTimer == 0u) {
        b->auraRegenTimer = b->auraRegenPeriod;
        return 1;
    }
    return 0;
}

/*
 * Return 1 if an incoming hit lands during the boss's startup counter window,
 * triggering Counter Hit (Paralyze + Full Synchro).
 */
u32 BossCheckCounterHit(struct BossPatternState *b) {
    if (b == 0 || b->counterWindow == 0u) {
        return 0;
    }
    b->counterWindow = 0u;
    return 1;
}
