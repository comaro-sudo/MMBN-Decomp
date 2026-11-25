/* The PET window on PC: thin SDL3 wrapper around ports/common/pet_ui.c. */
#ifndef MMBN4_PET_H
#define MMBN4_PET_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "pet_model.h"
#include "pet_ui.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    PetUiState ui;
    bool visible;
} Pet;

bool pet_create(Pet* pet);
void pet_destroy(Pet* pet);
void pet_toggle(Pet* pet);
/* Render + present (no-op while hidden). */
void pet_draw(Pet* pet, const PetModel* model);
/* Render only, without presenting (for reading pixels back). */
void pet_render(Pet* pet, const PetModel* model);
/* Handle a mouse click in window coordinates; returns action message or NULL. */
const char* pet_click(Pet* pet, const PetModel* model, float win_x, float win_y,
                      Emu* emu, const GameInfo* game);

#endif
