/* Phase C2 / E2 portable C reference implementations of core MMBN4 engine
 * leaf routines from code_00.s and code_01.s (originally hand-written Thumb
 * assembly using register `sl` = gToolkit).
 *
 * Verified with `make modern-check` (-Wall -Wextra -Werror). */

#include "toolkit.h"
#include "battle.h"

#define MAX_GAME_TIME_FRAMES 0x014988F0u /* 99:59:59 @ 60 fps */
#define MAX_PLAYTHROUGH      0x62u       /* 99th run (0-indexed) */

/* 0x08000C2C: CapIncrementGameTimeFrames */
void CapIncrementGameTimeFrames(struct Toolkit *tk) {
    u32 frames = tk->progress->gameTimeFrames + 1;
    if (frames > MAX_GAME_TIME_FRAMES)
        frames = MAX_GAME_TIME_FRAMES;
    tk->progress->gameTimeFrames = frames;
}

/* 0x08000C44: GetPlaythrough */
u8 GetPlaythrough(const struct Toolkit *tk) {
    return tk->progress->playthrough;
}

/* 0x08000C4C: GetPlaythroughTier (0 = 1st run, 1 = 2nd, 2 = 3rd+) */
u8 GetPlaythroughTier(const struct Toolkit *tk) {
    u8 p = tk->progress->playthrough;
    return p < 2 ? p : 2;
}

/* 0x08000C5A: IncPlaythrough */
void IncPlaythrough(struct Toolkit *tk) {
    if (tk->progress->playthrough < MAX_PLAYTHROUGH)
        tk->progress->playthrough++;
}

/* 0x08001AF8: SeedRNG */
void SeedRNG(u32 *rng) {
    *rng = RNG_INIT_SEED;
}

/* 0x08001B00 / 0x08001B30: GetRNG / GetSecondaryRNG */
u32 StepRNG(u32 *rng) {
    u32 v = *rng;
    v = ((v << 1) | (v >> 31)) + 1;
    v ^= RNG_XOR_MASK;
    *rng = v;
    return v;
}

/* 0x08001B16 / 0x08001B46: GetPositiveSignedRNG */
s32 StepPositiveSignedRNG(u32 *rng) {
    return (s32)(StepRNG(rng) & 0x7FFFFFFFu);
}

/* 0x0802504C: EventFlagSet (MSB-first bitfield in tk->eventFlags) */
void EventFlagSet(struct Toolkit *tk, u16 flag) {
    tk->eventFlags[flag >> 3] |= (u8)(0x80u >> (flag & 7));
}

/* 0x08025068: EventFlagClear */
void EventFlagClear(struct Toolkit *tk, u16 flag) {
    tk->eventFlags[flag >> 3] &= (u8)~(0x80u >> (flag & 7));
}

/* 0x08025084: EventFlagToggle */
void EventFlagToggle(struct Toolkit *tk, u16 flag) {
    tk->eventFlags[flag >> 3] ^= (u8)(0x80u >> (flag & 7));
}

/* 0x080250A0: EventFlagTest */
int EventFlagTest(const struct Toolkit *tk, u16 flag) {
    return (tk->eventFlags[flag >> 3] & (u8)(0x80u >> (flag & 7))) != 0;
}
