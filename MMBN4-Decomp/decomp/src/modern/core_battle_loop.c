/*
 * Portable C battle entity tick & hit resolution helpers (Phase D1).
 *
 * Models MMBN4's per-frame combatant state (code_07):
 *   - Invincibility (flashing) & hitstun timers
 *   - Barrier / Aura absorption (Aura blocks hits < threshold; Barrier absorbs)
 *   - Guard / Invis / Shadow piercing rules
 *   - HP subtraction, clamping, and deletion check
 */
#include "gba_types.h"

enum HitFlags {
    HIT_FLAG_NONE        = 0,
    HIT_FLAG_PIERCE_INV  = (1u << 0), /* pierces Invis */
    HIT_FLAG_WIND        = (1u << 1), /* strips Barrier / Aura immediately */
    HIT_FLAG_BREAK       = (1u << 2), /* pierces Guard */
    HIT_FLAG_SWORD       = (1u << 3), /* pierces Shadow */
    HIT_FLAG_FLINCH      = (1u << 4)  /* grants flashing i-frames on hit */
};

enum BarrierKind {
    BARRIER_NONE   = 0,
    BARRIER_NORMAL = 1, /* absorbs up to barrierHp, breaks when depleted */
    BARRIER_AURA   = 2  /* nullifies hits < barrierHp; breaks if hit >= barrierHp */
};

struct BattleActorState {
    s16 hp;
    s16 maxHp;
    u16 invFrames;
    u16 hitstunFrames;
    u16 invisFrames;
    u8 guardActive;
    u8 shadowActive;
    u8 barrierKind;
    s16 barrierHp;
    u8 deleted;
};

/*
 * Advance per-frame battle timers on an actor.
 */
void BattleActorTickTimers(struct BattleActorState *actor) {
    if (actor->invFrames > 0) {
        actor->invFrames--;
    }
    if (actor->hitstunFrames > 0) {
        actor->hitstunFrames--;
    }
    if (actor->invisFrames > 0) {
        actor->invisFrames--;
    }
}

/*
 * Resolve an incoming hit of rawDamage (with HitFlags) against actor.
 * Returns the actual HP damage dealt (0 if blocked, absorbed, or dodged).
 */
s32 BattleActorApplyHit(struct BattleActorState *actor, s32 rawDamage, u32 flags) {
    s32 dmg = rawDamage;

    if (actor->deleted || dmg <= 0) {
        return 0;
    }

    /* Flashing invincibility frames ignore non-piercing hits */
    if (actor->invFrames > 0 && (flags & HIT_FLAG_PIERCE_INV) == 0u) {
        return 0;
    }

    /* Invis dodges unless pierced */
    if (actor->invisFrames > 0 && (flags & HIT_FLAG_PIERCE_INV) == 0u) {
        return 0;
    }

    /* Shadow dodges non-sword hits */
    if (actor->shadowActive && (flags & HIT_FLAG_SWORD) == 0u) {
        return 0;
    }

    /* Guard blocks non-break hits */
    if (actor->guardActive && (flags & HIT_FLAG_BREAK) == 0u) {
        return 0;
    }

    /* Wind immediately strips any Barrier or Aura */
    if ((flags & HIT_FLAG_WIND) != 0u && actor->barrierKind != BARRIER_NONE) {
        actor->barrierKind = BARRIER_NONE;
        actor->barrierHp = 0;
    }

    /* Barrier / Aura resolution */
    if (actor->barrierKind == BARRIER_AURA) {
        if (dmg < (s32)actor->barrierHp) {
            return 0;
        }
        actor->barrierKind = BARRIER_NONE;
        actor->barrierHp = 0;
        return 0; /* Aura breaks while absorbing the breaking hit */
    } else if (actor->barrierKind == BARRIER_NORMAL) {
        if (dmg < (s32)actor->barrierHp) {
            actor->barrierHp = (s16)(actor->barrierHp - dmg);
            return 0;
        }
        actor->barrierKind = BARRIER_NONE;
        actor->barrierHp = 0;
        return 0; /* Barrier absorbs the breaking hit without bleed-through */
    }

    /* Apply HP damage */
    if (dmg >= (s32)actor->hp) {
        s32 dealt = (s32)actor->hp;
        actor->hp = 0;
        actor->deleted = 1;
        return dealt;
    }

    actor->hp = (s16)(actor->hp - dmg);
    if ((flags & HIT_FLAG_FLINCH) != 0u) {
        actor->invFrames = 120u; /* standard 2-second flashing window at 60 FPS */
        actor->hitstunFrames = 16u;
    }
    return dmg;
}

/*
 * Heal an actor by amount, clamped to maxHp. No-op if already deleted.
 */
s32 BattleActorHeal(struct BattleActorState *actor, s32 amount) {
    s32 before;

    if (actor->deleted || amount <= 0) {
        return 0;
    }
    before = (s32)actor->hp;
    if (before + amount > (s32)actor->maxHp) {
        actor->hp = actor->maxHp;
    } else {
        actor->hp = (s16)(before + amount);
    }
    return (s32)actor->hp - before;
}
