/* MMBN4 core game/progress/input state structures (Phase C1).
 * Offsets verified against asm/code_00.s and docs/re/save-and-flags.md. */

#ifndef MMBN4_GAMESTATE_H
#define MMBN4_GAMESTATE_H

#include "gba_types.h"

/* eToolkit->mainState (0x0200A7E0) */
struct MainState {
    u8 subsystem;        /* +0x00: index into MainSubsystemJumpTable (dat_0800032C) */
    u8 substate;         /* +0x01 */
    u8 pad02[2];         /* +0x02 */
    u8 softResetTimer;   /* +0x04: cooldown frames after A+B+Start+Select */
    u8 pad05[3];         /* +0x05 */
};

/* eToolkit->joypad (0x0200A700), updated by Joypad_Poll (sub_080003B0) */
struct JoypadState {
    u16 held;            /* +0x00: currently held buttons (~REG_KEYINPUT) */
    u16 pressed;         /* +0x02: newly pressed this frame */
    u16 repeated;        /* +0x04: held with auto-repeat */
    u16 prevHeld;        /* +0x06: held on previous frame */
    u8 holdTimers[10];   /* +0x08: per-button hold counters (0..16) */
    u8 pad12;            /* +0x12 */
    u8 repeatPhase;      /* +0x13: 0..4 repeat divider */
};

/* eToolkit->gameState (relocatable save block +0x0000, size 0x84) */
struct GameState {
    u8 mapState;         /* +0x00: BN4_MAP_STATES index */
    u8 pad01[3];         /* +0x01 */
    u8 mapGroup;         /* +0x04 */
    u8 mapNumber;        /* +0x05 */
    u8 pad06[0x13];      /* +0x06 */
    u8 currentBgm;       /* +0x19: read/written by BN4_PLAY_MUSIC (sub_08000568) */
    u8 pad1A[6];         /* +0x1A */
    u16 currentHp;       /* +0x20: cap 9999 */
    u16 maxHp;           /* +0x22: cap 9999 */
    u8 pad24[0x50];      /* +0x24 */
    u32 zenny;           /* +0x74: cap 999999 */
    u32 bugFrags;        /* +0x78: cap 9999 */
    u8 pad7C[8];         /* +0x7C */
};

/* eToolkit->progress (relocatable save block +0x0084, size 0x94) */
struct ProgressState {
    u8 pad00[0x0D];      /* +0x00 */
    u8 playthrough;      /* +0x0D: 0=1st, 1=2nd, 2=3rd+ (sub_08000C44/C4C/C5A) */
    u8 pad0E[6];         /* +0x0E */
    u16 hpA;             /* +0x14: 100 on new game */
    u16 hpB;             /* +0x16: 100 on new game */
    u8 pad18[0x14];      /* +0x18 */
    u32 gameTimeFrames;  /* +0x2C: capped at 0x014988F0 (99:59:59 @ 60fps) */
    u32 pad30;           /* +0x30 */
    u32 checksum;        /* +0x34: byte sum of 0x02000000..0x020073D1 (+0x16 RS / +0x22 BM) */
    u8 pad38[0x1C];      /* +0x38 */
    char magic[20];      /* +0x54: "ROCKMANEXE4 20031022" */
    u8 pad68[0x2C];      /* +0x68 */
};

#define EVENT_FLAG_BYTES 0x3E0
#define FOLDER_COUNT     3
#define FOLDER_SIZE      30
#define CHIP_PACK_COUNT  320
#define SAVE_BLOCK_BASE  0x02002130
#define SAVE_BLOCK_SIZE  0x3CF0
#define SAVE_SHIFT_ADDR  (*(vu32 *)0x02001550)
#define SAVE_KEY_ADDR    (*(vu32 *)0x02001554)

#endif /* MMBN4_GAMESTATE_H */
