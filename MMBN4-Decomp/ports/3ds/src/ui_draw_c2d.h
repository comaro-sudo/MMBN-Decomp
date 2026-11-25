/* citro2d (Nintendo 3DS bottom screen, 320x240) implementation of UiDraw. */
#ifndef MMBN4_UI_DRAW_C2D_H
#define MMBN4_UI_DRAW_C2D_H

#include <citro2d.h>

#include "ui_draw.h"

typedef struct {
    C3D_RenderTarget* target;
    C2D_TextBuf text_buf;
} UiDrawC2D;

UiDraw ui_draw_c2d(UiDrawC2D* ctx);

#endif
