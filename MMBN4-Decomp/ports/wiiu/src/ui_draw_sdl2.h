/* SDL2 (Wii U GX2) implementation of the portable UiDraw interface,
 * with a built-in 5x7 ASCII bitmap font (no external font assets needed). */
#ifndef MMBN4_UI_DRAW_SDL2_H
#define MMBN4_UI_DRAW_SDL2_H

#include <SDL2/SDL.h>

#include "ui_draw.h"

UiDraw ui_draw_sdl2(SDL_Renderer* renderer);

#endif
