/*
 * Portable C battle status-effect state machine (Phase D3).
 *
 * Models MMBN4's combat status ailments (code_07 / code_08):
 *   - Stun/immobilize ailments: Paralyze, Freeze, Bubble
 *   - Control/vision ailments: Confuse, Blind
 *   - StatusGuard immunity
 *   - Elemental/type shatter multipliers:
 *       * Break hit on Frozen target -> 2x damage & shatters Freeze
 *       * Elec hit on Bubbled target -> 2x damage & pops Bubble
 */
#include "gba_types.h"

enum StatusKind {
    STATUS_NONE     = 0,
    STATUS_PARALYZE = 1,
    STATUS_FREEZE   = 2,
    STATUS_BUBBLE   = 3,
    STATUS_CONFUSE  = 4,
    STATUS_BLIND    = 5
};

struct BattleStatusState {
    u8 activeStatus;
    u16 durationFrames;
    u8 statusGuard; /* 1 = immune to immobilize/confuse/blind ailments */
};

/*
 * Attempt to inflict a status ailment for `frames` ticks.
 * Returns 1 if applied, 0 if blocked by StatusGuard or invalid.
 */
u32 StatusInflict(struct BattleStatusState *st, u8 statusKind, u16 frames) {
    if (st == 0 || statusKind == STATUS_NONE || frames == 0u) {
        return 0;
    }
    if (st->statusGuard) {
        return 0;
    }
    st->activeStatus = statusKind;
    st->durationFrames = frames;
    return 1;
}

/*
 * Advance status timer by 1 frame, clearing activeStatus on expiry.
 */
void StatusTick(struct BattleStatusState *st) {
    if (st == 0 || st->activeStatus == STATUS_NONE) {
        return;
    }
    if (st->durationFrames > 0u) {
        st->durationFrames--;
    }
    if (st->durationFrames == 0u) {
        st->activeStatus = STATUS_NONE;
    }
}

/*
 * Return 1 if the current status prevents the actor from moving or attacking.
 */
u32 StatusIsImmobilized(const struct BattleStatusState *st) {
    if (st == 0) {
        return 0;
    }
    return (st->activeStatus == STATUS_PARALYZE
         || st->activeStatus == STATUS_FREEZE
         || st->activeStatus == STATUS_BUBBLE) ? 1u : 0u;
}

/*
 * Apply Freeze+Break or Bubble+Elec 2x shatter/pop bonus to `damage`,
 * clearing the status when triggered.
 *   isBreak: 1 if incoming hit has Break property
 *   element: 3 = Elec (per battle.h)
 */
s32 StatusApplyHitBonus(struct BattleStatusState *st, s32 damage,
                        u8 isBreak, u8 element) {
    if (st == 0 || damage <= 0) {
        return damage;
    }
    if (st->activeStatus == STATUS_FREEZE && isBreak) {
        st->activeStatus = STATUS_NONE;
        st->durationFrames = 0u;
        return damage * 2;
    }
    if (st->activeStatus == STATUS_BUBBLE && element == 3u) {
        st->activeStatus = STATUS_NONE;
        st->durationFrames = 0u;
        return damage * 2;
    }
    return damage;
}
