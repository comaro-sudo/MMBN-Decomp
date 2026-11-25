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
        0x04FC,     /* off_folder1 */
        0x080197EC, /* rom_chip_table */
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
        0x04FC,     /* off_folder1 */
        0x080197EC, /* rom_chip_table */
    },
};

static const GameInfo UNKNOWN = { GAME_UNKNOWN, "Unknown game", "Unknown", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const GameInfo* game_identify(const char* code) {
    const char* c = strrchr(code, '-');
    c = c ? c + 1 : code;
    if (strncmp(c, "B4WE", 4) == 0) return &GAMES[0];
    if (strncmp(c, "B4BE", 4) == 0) return &GAMES[1];
    return &UNKNOWN;
}

const char* game_map_name(uint8_t group, uint8_t id) {
    switch (group) {
    case 0x00:
        switch (id) {
        case 0x00: return "ACDC Town";
        case 0x01: return "Lan's House";
        case 0x02: return "Lan's Room";
        case 0x03: return "Mayl's House";
        case 0x04: return "Dex's Room";
        case 0x05: return "Yai's Room";
        case 0x06: return "Higsby's Shop";
        default:   return "ACDC Town (Area)";
        }
    case 0x01:
        switch (id) {
        case 0x00: return "ElecTown Square";
        case 0x01: return "DenDome Entrance";
        case 0x02: return "DenDome Colosseum";
        case 0x03: return "DenDome Waiting Rm";
        default:   return "ElecTown (Area)";
        }
    case 0x02:
        switch (id) {
        case 0x00: return "Castillo Entrance";
        case 0x01: return "Castillo Plaza";
        case 0x02: return "Castillo Colosseum";
        default:   return "Castillo (Area)";
        }
    case 0x03: return "NAXA / Space Ctr";
    case 0x80:
        switch (id) {
        case 0x00: return "ACDC Area 1";
        case 0x01: return "ACDC Area 2";
        case 0x02: return "ACDC Area 3";
        default:   return "ACDC Net";
        }
    case 0x81:
        switch (id) {
        case 0x00: return "Town Area 1";
        case 0x01: return "Town Area 2";
        case 0x02: return "Town Area 3";
        case 0x03: return "Town Area 4";
        default:   return "Town Net";
        }
    case 0x82:
        switch (id) {
        case 0x00: return "Park Area 1";
        case 0x01: return "Park Area 2";
        case 0x02: return "Park Area 3";
        default:   return "Park Net";
        }
    case 0x83:
        switch (id) {
        case 0x00: return "Undernet 1";
        case 0x01: return "Undernet 2";
        case 0x02: return "Undernet 3";
        case 0x03: return "Undernet 4";
        case 0x04: return "Undernet 5";
        case 0x05: return "Undernet 6";
        default:   return "Undernet";
        }
    case 0x84:
        switch (id) {
        case 0x00: return "Black Earth 1";
        case 0x01: return "Black Earth 2";
        default:   return "Black Earth";
        }
    case 0x90: return "Lan's HP / Comp";
    case 0x91: return "Town Comp";
    case 0x92: return "Castillo Comp";
    case 0x94: return "Meteor Comp";
    default:
        return (group >= 0x80) ? "Cyberworld Area" : "Real World Area";
    }
}
