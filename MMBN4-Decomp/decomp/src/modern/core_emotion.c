/* Phase D1 / E2 portable C reference implementations of MMBN4 Emotion Window
 * and Dark Soul karma mechanics (Normal, Full Synchro, Anger, Anxious, Evil,
 * Dark Soul).
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "gba_types.h"

enum EmotionState {
    EMOTION_NORMAL       = 0,
    EMOTION_FULL_SYNCHRO = 1,
    EMOTION_ANGER        = 2,
    EMOTION_ANXIOUS      = 3,
    EMOTION_EVIL         = 4,
    EMOTION_DARK_SOUL    = 5,
};

#define KARMA_MAX 1024
#define KARMA_EVIL_THRESHOLD 128

/* Applies a karma delta (DarkChip use subtracts karma, Light actions add). */
s16 EmotionAdjustKarma(s16 karma, s16 delta) {
    s32 next = (s32)karma + delta;
    if (next < 0)
        next = 0;
    else if (next > KARMA_MAX)
        next = KARMA_MAX;
    return (s16)next;
}

/* Returns 1 if MegaMan is in Evil karma territory (eligible for DarkChips
 * without Light restrictions and Dark Soul trigger). */
int EmotionIsEvilKarma(s16 karma) {
    return karma < KARMA_EVIL_THRESHOLD;
}

/* Transitions Emotion Window on a Counter Hit (Full Synchro if non-Evil). */
enum EmotionState EmotionOnCounterHit(enum EmotionState cur, s16 karma) {
    if (cur == EMOTION_DARK_SOUL || EmotionIsEvilKarma(karma))
        return cur;
    return EMOTION_FULL_SYNCHRO;
}

/* Transitions Emotion Window when MegaMan takes damage (drops Full Synchro). */
enum EmotionState EmotionOnTakeHit(enum EmotionState cur, u16 damage, u16 maxHp) {
    if (cur == EMOTION_DARK_SOUL || cur == EMOTION_EVIL)
        return cur;
    if (cur == EMOTION_FULL_SYNCHRO)
        return EMOTION_NORMAL;
    /* Heavy single hit (>= 25% Max HP) can trigger Anger */
    if (maxHp > 0 && (u32)damage * 4u >= (u32)maxHp)
        return EMOTION_ANGER;
    return cur;
}

/* Returns next-chip damage multiplier (2 for Full Synchro / Anger, 1 otherwise). */
u8 EmotionChipDamageMultiplier(enum EmotionState cur) {
    if (cur == EMOTION_FULL_SYNCHRO || cur == EMOTION_ANGER)
        return 2u;
    return 1u;
}
