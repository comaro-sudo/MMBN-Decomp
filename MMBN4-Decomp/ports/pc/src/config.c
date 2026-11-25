#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pc_config_defaults(PcConfig* cfg) {
    cfg->key_a      = SDL_SCANCODE_X;
    cfg->key_b      = SDL_SCANCODE_Z;
    cfg->key_select = SDL_SCANCODE_BACKSPACE;
    cfg->key_start  = SDL_SCANCODE_RETURN;
    cfg->key_right  = SDL_SCANCODE_RIGHT;
    cfg->key_left   = SDL_SCANCODE_LEFT;
    cfg->key_up     = SDL_SCANCODE_UP;
    cfg->key_down   = SDL_SCANCODE_DOWN;
    cfg->key_r      = SDL_SCANCODE_S;
    cfg->key_l      = SDL_SCANCODE_A;
    cfg->key_ff     = SDL_SCANCODE_TAB;
    cfg->off_tv     = false;
}

static void set_key(SDL_Scancode* dst, const char* val) {
    SDL_Scancode sc = SDL_GetScancodeFromName(val);
    if (sc != SDL_SCANCODE_UNKNOWN) *dst = sc;
}

static void trim(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n' || s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
}

void pc_config_load(PcConfig* cfg, const char* path) {
    pc_config_defaults(cfg);
    FILE* f = fopen(path, "r");
    if (!f) {
        pc_config_save(cfg, path);
        return;
    }
    char line[128];
    while (fgets(line, sizeof line, f)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') continue;
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        const char* k = line;
        const char* v = eq + 1;
        if      (!strcmp(k, "key_a"))      set_key(&cfg->key_a, v);
        else if (!strcmp(k, "key_b"))      set_key(&cfg->key_b, v);
        else if (!strcmp(k, "key_select")) set_key(&cfg->key_select, v);
        else if (!strcmp(k, "key_start"))  set_key(&cfg->key_start, v);
        else if (!strcmp(k, "key_right"))  set_key(&cfg->key_right, v);
        else if (!strcmp(k, "key_left"))   set_key(&cfg->key_left, v);
        else if (!strcmp(k, "key_up"))     set_key(&cfg->key_up, v);
        else if (!strcmp(k, "key_down"))   set_key(&cfg->key_down, v);
        else if (!strcmp(k, "key_r"))      set_key(&cfg->key_r, v);
        else if (!strcmp(k, "key_l"))      set_key(&cfg->key_l, v);
        else if (!strcmp(k, "key_ff"))     set_key(&cfg->key_ff, v);
        else if (!strcmp(k, "off_tv"))     cfg->off_tv = (atoi(v) != 0);
    }
    fclose(f);
}

bool pc_config_save(const PcConfig* cfg, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) return false;
    fprintf(f, "# MMBN4 Violet Eclipse - PC configuration\n");
    fprintf(f, "# Edit key names (SDL scancode names, e.g. X, Z, Return, Space, Tab)\n");
    fprintf(f, "key_a=%s\n",      SDL_GetScancodeName(cfg->key_a));
    fprintf(f, "key_b=%s\n",      SDL_GetScancodeName(cfg->key_b));
    fprintf(f, "key_select=%s\n", SDL_GetScancodeName(cfg->key_select));
    fprintf(f, "key_start=%s\n",  SDL_GetScancodeName(cfg->key_start));
    fprintf(f, "key_right=%s\n",  SDL_GetScancodeName(cfg->key_right));
    fprintf(f, "key_left=%s\n",   SDL_GetScancodeName(cfg->key_left));
    fprintf(f, "key_up=%s\n",     SDL_GetScancodeName(cfg->key_up));
    fprintf(f, "key_down=%s\n",   SDL_GetScancodeName(cfg->key_down));
    fprintf(f, "key_r=%s\n",      SDL_GetScancodeName(cfg->key_r));
    fprintf(f, "key_l=%s\n",      SDL_GetScancodeName(cfg->key_l));
    fprintf(f, "key_ff=%s\n",     SDL_GetScancodeName(cfg->key_ff));
    fprintf(f, "off_tv=%d\n",     cfg->off_tv ? 1 : 0);
    fclose(f);
    return true;
}
