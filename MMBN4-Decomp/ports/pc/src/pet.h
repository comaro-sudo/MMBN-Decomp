/* The PET window on PC: thin SDL3 wrapper around ports/common/pet_ui.c. */
#ifndef MMBN4_PET_H
#define MMBN4_PET_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "pet_model.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool visible;
} Pet;

bool pet_create(Pet* pet);
void pet_destroy(Pet* pet);
void pet_toggle(Pet* pet);
/* Render + present (no-op while hidden). */
void pet_draw(Pet* pet, const PetModel* model);
/* Render only, without presenting (for reading pixels back). */
void pet_render(Pet* pet, const PetModel* model);

#endif
