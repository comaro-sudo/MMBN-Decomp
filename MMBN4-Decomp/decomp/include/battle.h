/* MMBN4 battle, chip, Double Soul, and RNG definitions (Phase C1).
 * Addresses & layouts verified against asm/code_00.s, asm/code_01.s,
 * docs/re/save-and-flags.md, and ports/common/game.c. */

#ifndef MMBN4_BATTLE_H
#define MMBN4_BATTLE_H

#include "gba_types.h"

/* RNG state in EWRAM (SeedRNG sub_08001AF8, GetRNG sub_08001B00) */
#define gMainRng      (*(vu32 *)0x02001790)
#define gSecondaryRng (*(vu32 *)0x02001794)
#define RNG_INIT_SEED 0xA338244F
#define RNG_XOR_MASK  0x873CA9E5

/* Double Soul indices (BN4_UNITE_CHECK sub_0801FF84, BN4_SOUL_FLAGS 0x08020018) */
enum DoubleSoulId {
    SOUL_NONE    = 0,
    /* Red Sun Souls (event flags 0x17..0x1C) */
    SOUL_ROLL    = 1,
    SOUL_GUTS    = 2,
    SOUL_WIND    = 3,
    SOUL_SEARCH  = 4,
    SOUL_FIRE    = 5,
    SOUL_THUNDER = 6,
    /* Blue Moon Souls (event flags 0x1D..0x22) */
    SOUL_PROTO   = 7,
    SOUL_NUMBER  = 8,
    SOUL_METAL   = 9,
    SOUL_JUNK    = 10,
    SOUL_AQUA    = 11,
    SOUL_WOOD    = 12,
};

#define SOUL_USED_BITS (*(vu16 *)0x0203BF88)

/* Battle Chip table entry (ROM 0x080197EC, 0x20 bytes per chip) */
struct BattleChipData {
    u8 codes[4];         /* +0x00: up to 4 valid chip codes (26 = '*', 0xFF = none) */
    u8 element;          /* +0x04: 0=Null, 1=Fire, 2=Aqua, 3=Elec, 4=Wood, ... */
    u8 rarity;           /* +0x05 */
    u8 chipClass;        /* +0x06: 0=Std, 1=Mega, 2=Giga, 3=Dark */
    u8 mb;               /* +0x07: Regular memory MB */
    u8 kind;             /* +0x08: chip family / Double Soul sacrifice kind */
    u8 flags;            /* +0x09 */
    u16 attackPower;     /* +0x0A: base damage */
    u16 libraryIndex;    /* +0x0C */
    u16 unk0E;           /* +0x0E */
    const void *iconPtr; /* +0x10 */
    const void *artPtr;  /* +0x14 */
    const void *palPtr;  /* +0x18 */
    u32 unk1C;           /* +0x1C */
};

#define gBattleChipTable ((const struct BattleChipData *)0x080197EC)

/* Active battle Navi state (0x0203BEC0) */
struct BattleNaviState {
    u8 pad00[0x18];      /* +0x00 */
    u16 currentHp;       /* +0x18 */
    u16 maxHp;           /* +0x1A */
    u8 pad1C[0x0C];      /* +0x1C */
};

#define gBattleNaviState ((struct BattleNaviState *)0x0203BEC0)

#endif /* MMBN4_BATTLE_H */
