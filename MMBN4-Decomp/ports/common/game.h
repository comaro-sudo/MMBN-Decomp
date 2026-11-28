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
    uint32_t ram_save_shift;    /* EWRAM save-shift, a u32 (even, 0..0x1FE: GetRNG() & 0x1FE; docs/re/save-and-flags.md) */
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
    uint32_t off_folder1;       /* 30 x u16: chip_id | (code << 9) */
    uint32_t rom_chip_table;    /* ROM address of BN4_CHIP_TABLE (44B/entry) */
    uint32_t rom_chip_count;    /* entries in that table (ids 0..count-1). The patch only rewrites
                                   existing entries (Duo 0x137, GP Power 0x138 reuse unused slots),
                                   so the count is the same for patched and original ROMs. */
} GameInfo;

/* code: mGBA's game code, e.g. "AGB-B4WE" or "AGB-B4BE". */
const GameInfo* game_identify(const char* code);

/* Human-readable area name for BN4 (map_group, map_id). */
const char* game_map_name(uint8_t group, uint8_t id);

#endif
