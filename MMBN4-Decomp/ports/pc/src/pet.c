#include "pet.h"

#include "pet_ui.h"
#include "ui_draw_sdl3.h"

bool pet_create(Pet* pet) {
    if (!SDL_CreateWindowAndRenderer("MMBN4 - PET", PET_QVGA_W * 2, PET_QVGA_H * 2, SDL_WINDOW_RESIZABLE,
                                     &pet->window, &pet->renderer)) {
        return false;
    }
    SDL_SetRenderLogicalPresentation(pet->renderer, PET_QVGA_W, PET_QVGA_H, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    pet->visible = true;
    return true;
}

void pet_destroy(Pet* pet) {
    if (pet->renderer) SDL_DestroyRenderer(pet->renderer);
    if (pet->window) SDL_DestroyWindow(pet->window);
    pet->renderer = NULL;
    pet->window = NULL;
}

void pet_toggle(Pet* pet) {
    pet->visible = !pet->visible;
    if (pet->visible) SDL_ShowWindow(pet->window);
    else SDL_HideWindow(pet->window);
}

void pet_draw(Pet* pet, const PetModel* model) {
    if (!pet->visible) return;
    pet_render(pet, model);
    SDL_RenderPresent(pet->renderer);
}

void pet_render(Pet* pet, const PetModel* model) {
    UiDraw ui = ui_draw_sdl3(pet->renderer);
    pet_ui_draw(&ui, model, PET_LAYOUT_QVGA);
}
