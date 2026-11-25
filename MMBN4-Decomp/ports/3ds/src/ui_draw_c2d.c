#include "ui_draw_c2d.h"

static u32 to_c2d(UiColor c) {
    return C2D_Color32(c.r, c.g, c.b, c.a);
}

static void c2d_clear(void* ctx, UiColor c) {
    UiDrawC2D* d = (UiDrawC2D*) ctx;
    C2D_TargetClear(d->target, to_c2d(c));
    C2D_SceneBegin(d->target);
    C2D_TextBufClear(d->text_buf);
}

static void c2d_rect(void* ctx, int x, int y, int w, int h, UiColor c) {
    (void) ctx;
    C2D_DrawRectSolid((float) x, (float) y, 0.0f, (float) w, (float) h, to_c2d(c));
}

static void c2d_rect_outline(void* ctx, int x, int y, int w, int h, UiColor c) {
    (void) ctx;
    u32 col = to_c2d(c);
    C2D_DrawRectSolid((float) x, (float) y, 0.0f, (float) w, 1.0f, col);
    C2D_DrawRectSolid((float) x, (float) (y + h - 1), 0.0f, (float) w, 1.0f, col);
    C2D_DrawRectSolid((float) x, (float) y, 0.0f, 1.0f, (float) h, col);
    C2D_DrawRectSolid((float) (x + w - 1), (float) y, 0.0f, 1.0f, (float) h, col);
}

static void c2d_line(void* ctx, int x1, int y1, int x2, int y2, UiColor c) {
    (void) ctx;
    C2D_DrawLine((float) x1, (float) y1, to_c2d(c), (float) x2, (float) y2, to_c2d(c), 1.0f, 0.0f);
}

static void c2d_text(void* ctx, int x, int y, const char* s, UiColor c) {
    UiDrawC2D* d = (UiDrawC2D*) ctx;
    C2D_Text t;
    C2D_TextParse(&t, d->text_buf, s);
    C2D_TextOptimize(&t);
    C2D_DrawText(&t, C2D_WithColor, (float) x, (float) y, 0.0f, 0.38f, 0.38f, to_c2d(c));
}

static void c2d_clip(void* ctx, int x, int y, int w, int h) {
    (void) ctx; (void) x; (void) y; (void) w; (void) h;
}

UiDraw ui_draw_c2d(UiDrawC2D* ctx) {
    UiDraw ui;
    ui.ctx          = ctx;
    ui.clear        = c2d_clear;
    ui.rect         = c2d_rect;
    ui.rect_outline = c2d_rect_outline;
    ui.line         = c2d_line;
    ui.text         = c2d_text;
    ui.clip         = c2d_clip;
    return ui;
}
