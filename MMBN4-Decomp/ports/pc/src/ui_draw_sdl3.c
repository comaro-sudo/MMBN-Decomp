#include "ui_draw_sdl3.h"

static void sdl3_clear(void* ctx, UiColor c) {
    SDL_Renderer* r = (SDL_Renderer*) ctx;
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderClear(r);
}

static void sdl3_rect(void* ctx, int x, int y, int w, int h, UiColor c) {
    SDL_Renderer* r = (SDL_Renderer*) ctx;
    SDL_FRect rc = { (float) x, (float) y, (float) w, (float) h };
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rc);
}

static void sdl3_rect_outline(void* ctx, int x, int y, int w, int h, UiColor c) {
    SDL_Renderer* r = (SDL_Renderer*) ctx;
    SDL_FRect rc = { (float) x, (float) y, (float) w, (float) h };
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderRect(r, &rc);
}

static void sdl3_line(void* ctx, int x1, int y1, int x2, int y2, UiColor c) {
    SDL_Renderer* r = (SDL_Renderer*) ctx;
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderLine(r, (float) x1, (float) y1, (float) x2, (float) y2);
}

static void sdl3_text(void* ctx, int x, int y, const char* s, UiColor c) {
    SDL_Renderer* r = (SDL_Renderer*) ctx;
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDebugText(r, (float) x, (float) y, s);
}

static void sdl3_clip(void* ctx, int x, int y, int w, int h) {
    SDL_Renderer* r = (SDL_Renderer*) ctx;
    if (w <= 0 || h <= 0) {
        SDL_SetRenderClipRect(r, NULL);
    } else {
        SDL_Rect rc = { x, y, w, h };
        SDL_SetRenderClipRect(r, &rc);
    }
}

UiDraw ui_draw_sdl3(SDL_Renderer* renderer) {
    UiDraw ui;
    ui.ctx          = renderer;
    ui.clear        = sdl3_clear;
    ui.rect         = sdl3_rect;
    ui.rect_outline = sdl3_rect_outline;
    ui.line         = sdl3_line;
    ui.text         = sdl3_text;
    ui.clip         = sdl3_clip;
    return ui;
}
