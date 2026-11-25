/* SDL3 implementation of the portable UiDraw interface. */
#ifndef MMBN4_UI_DRAW_SDL3_H
#define MMBN4_UI_DRAW_SDL3_H

#include <SDL3/SDL.h>

#include "ui_draw.h"

UiDraw ui_draw_sdl3(SDL_Renderer* renderer);

#endif
