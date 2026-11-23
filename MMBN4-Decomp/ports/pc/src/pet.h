/* The PET window: prototype of the second screen (bottom screen on DS/3DS,
 * GamePad on Wii U). Today it shows status and live RAM reads. */
#ifndef MMBN4_PET_H
#define MMBN4_PET_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "emu.h"
#include "game.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool visible;
} Pet;

typedef struct {
    double fps;
    bool fast_forward;
    uint32_t keys;
    const char* message;  /* last status message, may be NULL */
} PetStatus;

bool pet_create(Pet* pet);
void pet_destroy(Pet* pet);
void pet_toggle(Pet* pet);
/* Render + present (no-op while hidden). */
void pet_draw(Pet* pet, Emu* emu, const GameInfo* game, const char* code, const PetStatus* st);
/* Render only, without presenting (for reading pixels back). */
void pet_render(Pet* pet, Emu* emu, const GameInfo* game, const char* code, const PetStatus* st);

#endif
