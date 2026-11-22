#include "game.h"

#include <string.h>

/* Red Sun addresses: Cyberworld Endless docs/symbols/bn4-red-sun-us.csv (MIT),
 * names BN4_RNG_PRIMARY and BN4_BATTLE_NAVI, made from our exact ROM
 * (SHA-1 a97e96a7...). Blue Moon is not mapped yet (plan/02, D2). */
static const GameInfo GAMES[] = {
    { GAME_BN4_RS_US, "Mega Man Battle Network 4: Red Sun (USA)", "BN4 Red Sun", 0x02001790, 0x0203BEC0 },
    { GAME_BN4_BM_US, "Mega Man Battle Network 4: Blue Moon (USA)", "BN4 Blue Moon", 0, 0 },
};
static const GameInfo UNKNOWN = { GAME_UNKNOWN, "Unknown game", "Unknown", 0, 0 };

const GameInfo* game_identify(const char* code) {
    const char* c = strrchr(code, '-');
    c = c ? c + 1 : code;
    if (strncmp(c, "B4WE", 4) == 0) return &GAMES[0];
    if (strncmp(c, "B4BE", 4) == 0) return &GAMES[1];
    return &UNKNOWN;
}
