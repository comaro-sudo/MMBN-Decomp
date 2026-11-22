/* Which BN4 build is running, and the RAM addresses the host knows for it. */
#ifndef MMBN4_GAME_H
#define MMBN4_GAME_H

#include <stdint.h>

typedef enum { GAME_UNKNOWN, GAME_BN4_RS_US, GAME_BN4_BM_US } GameId;

typedef struct {
    GameId id;
    const char* name;
    const char* short_name;     /* fits the PET's 320 px line */
    /* 0 = not mapped yet for this version. */
    uint32_t ram_rng;           /* main RNG state, u32 */
    uint32_t ram_battle_navi;   /* battle NaviStats, 0x40 per side; HP u16 at +0x30 */
} GameInfo;

/* code: mGBA's game code, e.g. "AGB-B4WE". */
const GameInfo* game_identify(const char* code);

#endif
