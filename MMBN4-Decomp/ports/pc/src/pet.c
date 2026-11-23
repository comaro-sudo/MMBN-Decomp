#include "pet.h"

#include <stdio.h>
#include <string.h>

#define PET_W 320
#define PET_H 240
#define LINE 10

bool pet_create(Pet* pet) {
    if (!SDL_CreateWindowAndRenderer("MMBN4 - PET", PET_W * 2, PET_H * 2, SDL_WINDOW_RESIZABLE,
                                     &pet->window, &pet->renderer)) {
        return false;
    }
    SDL_SetRenderLogicalPresentation(pet->renderer, PET_W, PET_H, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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

static void text(SDL_Renderer* r, int x, int* y, const char* s) {
    SDL_RenderDebugText(r, (float) x, (float) *y, s);
    *y += LINE;
}

void pet_draw(Pet* pet, Emu* emu, const GameInfo* game, const char* code, const PetStatus* st) {
    if (!pet->visible) return;
    pet_render(pet, emu, game, code, st);
    SDL_RenderPresent(pet->renderer);
}

void pet_render(Pet* pet, Emu* emu, const GameInfo* game, const char* code, const PetStatus* st) {
    SDL_Renderer* r = pet->renderer;
    char buf[96];
    int y = 6;

    SDL_SetRenderDrawColor(r, 10, 24, 48, 255);
    SDL_RenderClear(r);

    SDL_SetRenderDrawColor(r, 0, 200, 255, 255);
    text(r, 6, &y, "MMBN4 OVERHAUL  -  PET (prototype)");
    y += 4;
    SDL_SetRenderDrawColor(r, 230, 230, 230, 255);
    snprintf(buf, sizeof buf, "Game : %s  %s", code, game->short_name);
    text(r, 6, &y, buf);
    snprintf(buf, sizeof buf, "CRC32: %08X", (unsigned) emu_crc32(emu));
    text(r, 6, &y, buf);
    snprintf(buf, sizeof buf, "Frame: %u   FPS: %.1f%s", (unsigned) emu_frame(emu), st->fps,
             st->fast_forward ? "  >> FAST" : "");
    text(r, 6, &y, buf);

    static const char* names[] = { "A", "B", "Sel", "Sta", "R", "L", "U", "D", "RB", "LB" };
    char keys[64] = "Keys : ";
    for (int i = 0; i < 10; ++i) {
        if (st->keys & (1u << i)) {
            strncat(keys, names[i], sizeof keys - strlen(keys) - 2);
            strncat(keys, " ", sizeof keys - strlen(keys) - 1);
        }
    }
    text(r, 6, &y, keys);

    y += 6;
    SDL_SetRenderDrawColor(r, 0, 200, 255, 255);
    text(r, 6, &y, "Live RAM (read between frames)");
    SDL_SetRenderDrawColor(r, 230, 230, 230, 255);
    if (game->ram_rng) {
        snprintf(buf, sizeof buf, "RNG state      : %08X", (unsigned) emu_read32(emu, game->ram_rng));
        text(r, 6, &y, buf);
        snprintf(buf, sizeof buf, "Battle HP P1/P2: %u / %u  (raw)",
                 (unsigned) emu_read16(emu, game->ram_battle_navi + 0x30),
                 (unsigned) emu_read16(emu, game->ram_battle_navi + 0x40 + 0x30));
        text(r, 6, &y, buf);
    } else {
        text(r, 6, &y, "(no RAM map for this version yet)");
    }

    y = PET_H - 6 - LINE * 5;
    SDL_SetRenderDrawColor(r, 150, 170, 200, 255);
    text(r, 6, &y, "Arrows move  X=A  Z=B  A/S=L/R");
    text(r, 6, &y, "Enter=Start  Backspace=Select");
    text(r, 6, &y, "Tab=fast  F5/F7=save/load state");
    text(r, 6, &y, "F1=PET  F11=fullscreen  Esc=quit");
    if (st->message) {
        SDL_SetRenderDrawColor(r, 255, 220, 0, 255);
        text(r, 6, &y, st->message);
    }
}
