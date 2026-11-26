/*
 * Portable C Sprite Frame Animation Controller (Phase C2).
 *
 * Models MMBN4's frame-table animation playback (code_00 / desc_macros.inc
 * anim_frame sequences): per-frame timer countdown, looping, and one-shot
 * completion flag.
 */
#include "modern_api.h"

/*
 * Initialize an animation controller to frame 0 with duration `firstDelay`.
 */
void SpriteAnimInit(struct SpriteAnimState *anim, u16 frameCount,
                    u16 firstDelay, u8 looping) {
    anim->frameIndex = 0u;
    anim->frameCount = frameCount;
    anim->timer = (firstDelay == 0u) ? 1u : firstDelay;
    anim->looping = looping;
    anim->finished = (frameCount == 0u) ? 1u : 0u;
}

/*
 * Advance animation by 1 tick using `durations[0..frameCount)` for frame delays.
 * Returns 1 if `frameIndex` changed this tick, 0 otherwise.
 */
u32 SpriteAnimStep(struct SpriteAnimState *anim, const u16 *durations) {
    if (anim == 0 || anim->finished || anim->frameCount == 0u) {
        return 0u;
    }
    if (anim->timer > 0u) {
        anim->timer--;
    }
    if (anim->timer > 0u) {
        return 0u;
    }
    if ((u32)anim->frameIndex + 1u < (u32)anim->frameCount) {
        anim->frameIndex++;
    } else if (anim->looping) {
        anim->frameIndex = 0u;
    } else {
        anim->finished = 1u;
        return 0u;
    }
    anim->timer = (durations != 0 && durations[anim->frameIndex] > 0u)
                  ? durations[anim->frameIndex] : 1u;
    return 1u;
}
