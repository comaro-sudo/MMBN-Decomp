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
    uint32_t ram_save_shift;    /* EWRAM save-shift byte (u8) */
    uint32_t ram_save_base;     /* EWRAM base before adding save shift */
    /* Offsets relative to active save block (ram_save_base + shift). */
    uint32_t off_map_group;     /* u8 */
    uint32_t off_map_id;        /* u8 */
    uint32_t off_cur_hp;        /* u16 */
    uint32_t off_max_hp;        /* u16 */
    uint32_t off_zenny;         /* u32 */
    uint32_t off_bugfrags;      /* u32 */
    uint32_t off_playthrough;   /* u8 (0-based; display +1) */
    uint32_t off_flags;         /* event flag bitfield (MSB-first per byte) */
} GameInfo;

/* code: mGBA's game code, e.g. "AGB-B4WE" or "AGB-B4BE". */
const GameInfo* game_identify(const char* code);

#endif
