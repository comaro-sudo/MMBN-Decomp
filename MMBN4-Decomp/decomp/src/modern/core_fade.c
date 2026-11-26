/*
 * Portable C GBA RGB555 Palette Fade & Brightness Helpers (Phase C2).
 *
 * Models GBA 15-bit BGR555 palette fading (fade-to-black and fade-to-white
 * with blend coefficient 0..16, mirroring code_00 palette fade routines).
 */
#include "gba_types.h"

/*
 * Pack 5-bit R, G, B components (0..31) into a GBA BGR555 color.
 */
u16 Rgb555Pack(u8 r, u8 g, u8 b) {
    return (u16)((r & 0x1Fu)
               | (((u16)(g & 0x1Fu)) << 5)
               | (((u16)(b & 0x1Fu)) << 10));
}

/*
 * Fade a single BGR555 color toward black (toWhite == 0) or white (toWhite != 0)
 * by `coeff` in [0..16] (0 = unchanged, 16 = full black/white).
 */
u16 Rgb555FadeColor(u16 color, u8 coeff, u8 toWhite) {
    u32 r = color & 0x1Fu;
    u32 g = (color >> 5) & 0x1Fu;
    u32 b = (color >> 10) & 0x1Fu;

    if (coeff > 16u) {
        coeff = 16u;
    }
    if (toWhite) {
        r = r + (((31u - r) * (u32)coeff) >> 4);
        g = g + (((31u - g) * (u32)coeff) >> 4);
        b = b + (((31u - b) * (u32)coeff) >> 4);
    } else {
        u32 inv = 16u - (u32)coeff;
        r = (r * inv) >> 4;
        g = (g * inv) >> 4;
        b = (b * inv) >> 4;
    }
    return Rgb555Pack((u8)r, (u8)g, (u8)b);
}

/*
 * Fade an entire palette buffer `src[0..count)` into `dst[0..count)`.
 */
void Rgb555FadePalette(const u16 *src, u16 *dst, u32 count, u8 coeff, u8 toWhite) {
    u32 i;
    for (i = 0; i < count; i++) {
        dst[i] = Rgb555FadeColor(src[i], coeff, toWhite);
    }
}
