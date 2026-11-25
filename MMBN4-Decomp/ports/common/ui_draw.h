/* Portable immediate-mode 2D draw interface for the PET screen.
 * Implemented once per platform: SDL3 on PC, GX2 on Wii U, citro2d on 3DS. */
#ifndef MMBN4_UI_DRAW_H
#define MMBN4_UI_DRAW_H

#include <stdint.h>

typedef struct {
    uint8_t r, g, b, a;
} UiColor;

static inline UiColor ui_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    UiColor c = { r, g, b, a };
    return c;
}

static inline UiColor ui_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ui_rgba(r, g, b, 255);
}

typedef struct {
    void* ctx;
    void (*clear)(void* ctx, UiColor c);
    void (*rect)(void* ctx, int x, int y, int w, int h, UiColor c);
    void (*rect_outline)(void* ctx, int x, int y, int w, int h, UiColor c);
    void (*line)(void* ctx, int x1, int y1, int x2, int y2, UiColor c);
    void (*text)(void* ctx, int x, int y, const char* s, UiColor c);
    void (*clip)(void* ctx, int x, int y, int w, int h); /* w <= 0 clears clip */
} UiDraw;

#endif
