/*
 * Portable C Overworld Camera & BG Scroll Helpers (Phase C2).
 *
 * Computes GBA 240x160 viewport camera clamping, smooth lerp tracking,
 * and screen-shake offsets for overworld maps (code_00 / code_01).
 */
#include "gba_types.h"

#define GBA_SCREEN_W 240
#define GBA_SCREEN_H 160

struct CameraState {
    s32 x;
    s32 y;
    s32 mapWidth;
    s32 mapHeight;
    u16 shakeFrames;
    s8 shakeAmp;
};

static s32 ClampS32(s32 v, s32 lo, s32 hi) {
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

/*
 * Center camera on (targetX, targetY) and clamp to [0..mapW-240, 0..mapH-160].
 */
void CameraCenterOn(struct CameraState *cam, s32 targetX, s32 targetY) {
    s32 maxX = (cam->mapWidth > GBA_SCREEN_W) ? (cam->mapWidth - GBA_SCREEN_W) : 0;
    s32 maxY = (cam->mapHeight > GBA_SCREEN_H) ? (cam->mapHeight - GBA_SCREEN_H) : 0;

    cam->x = ClampS32(targetX - (GBA_SCREEN_W / 2), 0, maxX);
    cam->y = ClampS32(targetY - (GBA_SCREEN_H / 2), 0, maxY);
}

/*
 * Trigger a screen shake of amplitude `amp` pixels for `frames` ticks.
 */
void CameraStartShake(struct CameraState *cam, s8 amp, u16 frames) {
    cam->shakeAmp = amp;
    cam->shakeFrames = frames;
}

/*
 * Step screen shake by 1 frame and return the signed pixel offset (+/-amp)
 * alternating each frame, or 0 when inactive.
 */
s32 CameraTickShakeOffset(struct CameraState *cam) {
    s32 off;

    if (cam == 0 || cam->shakeFrames == 0u) {
        return 0;
    }
    off = (cam->shakeFrames & 1u) ? (s32)cam->shakeAmp : -(s32)cam->shakeAmp;
    cam->shakeFrames--;
    return off;
}
