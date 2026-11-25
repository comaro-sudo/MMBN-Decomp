#include "pet.h"

#include "ui_draw_sdl3.h"

bool pet_create(Pet* pet) {
    pet_ui_init(&pet->ui);
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
    pet_ui_draw(&ui, model, &pet->ui, PET_LAYOUT_QVGA);
}

const char* pet_click(Pet* pet, const PetModel* model, float win_x, float win_y,
                      Emu* emu, const GameInfo* game) {
    if (!pet->renderer) return NULL;
    float lx = 0, ly = 0;
    SDL_RenderCoordinatesFromWindow(pet->renderer, win_x, win_y, &lx, &ly);
    return pet_ui_click(&pet->ui, model, PET_LAYOUT_QVGA, (int) lx, (int) ly, emu, game);
}
