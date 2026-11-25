#include "game.h"

#include <string.h>

/* Verified EWRAM layout shared by both Red Sun (USA, B4WE) and Blue Moon
 * (USA, B4BE). Active save block is at ram_save_base + u8(ram_save_shift). */
static const GameInfo GAMES[] = {
    {
        GAME_BN4_RS_US,
        "Mega Man Battle Network 4: Red Sun (USA)",
        "BN4 Red Sun",
        0x02001790, /* ram_rng */
        0x0203BEC0, /* ram_battle_navi */
        0x02001550, /* ram_save_shift */
        0x02002130, /* ram_save_base */
        0x0004,     /* off_map_group */
        0x0005,     /* off_map_id */
        0x0020,     /* off_cur_hp */
        0x0022,     /* off_max_hp */
        0x0074,     /* off_zenny */
        0x0078,     /* off_bugfrags */
        0x0091,     /* off_playthrough (0x84 + 0x0D) */
        0x0118,     /* off_flags */
    },
    {
        GAME_BN4_BM_US,
        "Mega Man Battle Network 4: Blue Moon (USA)",
        "BN4 Blue Moon",
        0x02001790, /* ram_rng */
        0x0203BEC0, /* ram_battle_navi */
        0x02001550, /* ram_save_shift */
        0x02002130, /* ram_save_base */
        0x0004,     /* off_map_group */
        0x0005,     /* off_map_id */
        0x0020,     /* off_cur_hp */
        0x0022,     /* off_max_hp */
        0x0074,     /* off_zenny */
        0x0078,     /* off_bugfrags */
        0x0091,     /* off_playthrough (0x84 + 0x0D) */
        0x0118,     /* off_flags */
    },
};

static const GameInfo UNKNOWN = { GAME_UNKNOWN, "Unknown game", "Unknown", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const GameInfo* game_identify(const char* code) {
    const char* c = strrchr(code, '-');
    c = c ? c + 1 : code;
    if (strncmp(c, "B4WE", 4) == 0) return &GAMES[0];
    if (strncmp(c, "B4BE", 4) == 0) return &GAMES[1];
    return &UNKNOWN;
}
