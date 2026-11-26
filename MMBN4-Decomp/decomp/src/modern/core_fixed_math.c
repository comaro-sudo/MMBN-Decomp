/*
 * Portable C GBA Fixed-Point & Grid Math Helpers (Phase C2).
 *
 * Provides Q16.16 fixed-point multiplication, linear interpolation,
 * integer clamping, Manhattan distance, and 8-direction vector lookup
 * for overworld & battle movement without floating-point math.
 */
#include "gba_types.h"

#define FX16_SHIFT 16
#define FX16_ONE   (1 << FX16_SHIFT)

/*
 * Multiply two signed Q16.16 fixed-point values using 64-bit intermediate.
 */
s32 Fx16Mul(s32 a, s32 b) {
    return (s32)(((long long)a * (long long)b) >> FX16_SHIFT);
}

/*
 * Linear interpolation between `a` and `b` using Q16.16 factor `t` in [0..FX16_ONE].
 */
s32 Fx16Lerp(s32 a, s32 b, s32 t) {
    return a + Fx16Mul(b - a, t);
}

/*
 * Integer absolute value.
 */
s32 MathAbsS32(s32 v) {
    return (v < 0) ? -v : v;
}

/*
 * Manhattan distance between two grid/tile points (x0, y0) and (x1, y1).
 */
s32 MathManhattan(s32 x0, s32 y0, s32 x1, s32 y1) {
    return MathAbsS32(x1 - x0) + MathAbsS32(y1 - y0);
}

/*
 * Map MMBN4's 8-way facing index (0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW)
 * to signed unit step deltas (dx, dy) in {-1, 0, +1}.
 */
void MathFacingToDelta(u8 dir, s8 *outDx, s8 *outDy) {
    static const s8 kDx[8] = {  0,  1,  1,  1,  0, -1, -1, -1 };
    static const s8 kDy[8] = { -1, -1,  0,  1,  1,  1,  0, -1 };
    u8 d = (u8)(dir & 7u);

    if (outDx != 0) {
        *outDx = kDx[d];
    }
    if (outDy != 0) {
        *outDy = kDy[d];
    }
}
