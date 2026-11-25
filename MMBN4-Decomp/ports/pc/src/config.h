/* PC-6 configuration file (mmbn4.ini) for keyboard remapping and UI prefs. */
#ifndef MMBN4_PC_CONFIG_H
#define MMBN4_PC_CONFIG_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Scancode key_a;
    SDL_Scancode key_b;
    SDL_Scancode key_select;
    SDL_Scancode key_start;
    SDL_Scancode key_right;
    SDL_Scancode key_left;
    SDL_Scancode key_up;
    SDL_Scancode key_down;
    SDL_Scancode key_r;
    SDL_Scancode key_l;
    SDL_Scancode key_ff;
    bool off_tv;
} PcConfig;

void pc_config_defaults(PcConfig* cfg);
/* Loads cfg from path (creating a default file if missing). */
void pc_config_load(PcConfig* cfg, const char* path);
bool pc_config_save(const PcConfig* cfg, const char* path);

#endif
