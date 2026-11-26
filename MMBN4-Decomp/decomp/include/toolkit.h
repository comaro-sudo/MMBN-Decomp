/* MMBN4 global Toolkit pointer table (Phase C1).
 * Lives at EWRAM 0x02009E70 and is kept in register `sl` (`r10`) throughout
 * the game engine. Fixed slots +0x00..+0x3C are copied from dat_080060E8 by
 * InitToolkit (sub_080060CC); relocatable save slots +0x40..+0x98 are rebuilt
 * from 0x080061AC by RelocateSaveBlock (sub_08006128) / RebuildSavePointers
 * (sub_0800616A). */

#ifndef MMBN4_TOOLKIT_H
#define MMBN4_TOOLKIT_H

#include "gba_types.h"
#include "gamestate.h"

struct Toolkit {
    /* Fixed EWRAM/IWRAM engine pointers (+0x00..+0x3C, from dat_080060E8) */
    struct MainState *mainState;     /* +0x00: 0x0200A7E0 */
    struct JoypadState *joypad;      /* +0x04: 0x0200A700 */
    void *video;                     /* +0x08: 0x0200B150 (DISPCNT shadow at +0) */
    void *unk_0C;                    /* +0x0C: 0x0200A380 */
    void *unk_10;                    /* +0x10: 0x0200AA20 */
    void *unk_14;                    /* +0x14: 0x0200A430 */
    void *unk_18;                    /* +0x18: 0x02035810 */
    void *unk_1C;                    /* +0x1C: 0x02010BC0 */
    void *unk_20;                    /* +0x20: 0x0200A220 */
    u16 *frameCounter;               /* +0x24: 0x0200A6B8 */
    void *iwramBuf;                  /* +0x28: 0x03000000 */
    void *unk_2C;                    /* +0x2C: 0x0200A500 */
    void *unk_30;                    /* +0x30: 0x02039A10 */
    void *unk_34;                    /* +0x34: 0x0200A450 */
    void *unk_38;                    /* +0x38: 0x0200A6C0 */
    void *unk_3C;                    /* +0x3C: 0x02001460 */

    /* Relocatable anti-cheat save block pointers (+0x40..+0x98, 23 slots) */
    struct GameState *gameState;     /* +0x40: block +0x0000 (0x84 B) */
    struct ProgressState *progress;  /* +0x44: block +0x0084 (0x94 B) */
    u8 *eventFlags;                  /* +0x48: block +0x0118 (0x3E4 B) */
    u16 (*folders)[FOLDER_SIZE];     /* +0x4C: block +0x04FC (0xB8 B) */
    u8 (*chipPack)[12];              /* +0x50: block +0x05B4 (0xF04 B) */
    u8 *items;                       /* +0x54: block +0x14B8 (0x194 B) */
    u8 *shopStock;                   /* +0x58: block +0x164C (0xDC4 B) */
    u8 *naviCustGrid;                /* +0x5C: block +0x2410 (0x24 B) */
    u8 *naviCustPlaced;              /* +0x60: block +0x2434 (0xCC B) */
    u8 *naviCustBugs;                /* +0x64: block +0x2500 (0x14 B) */
    u32 *activeModCard;              /* +0x68: block +0x2514 (0x4 B) */
    u16 *maxHpBonus;                 /* +0x6C: block +0x2518 (0x4 B) */
    u8 *modCardSlots;                /* +0x70: block +0x251C (0x10 B) */
    u16 *mysteryData;                /* +0x74: block +0x252C (0x804 B) */
    u8 (*naviStats)[0x40];           /* +0x78: block +0x2D30 (0x204 B) */
    u8 (*naviAi)[0xE0];              /* +0x7C: block +0x2F34 (0x704 B) */
    u8 *itemKeys;                    /* +0x80: block +0x3638 (0x194 B) */
    u8 *chipKeysMirror;              /* +0x84: block +0x37CC (0x204 B) */
    u8 *chipKeys;                    /* +0x88: block +0x39D0 (0x204 B) */
    u32 *zennyShadow;                /* +0x8C: block +0x3BD4 (0x8 B) */
    u32 *bugFragShadow;              /* +0x90: block +0x3BDC (0x8 B) */
    u8 *byteShadows;                 /* +0x94: block +0x3BE4 (0x104 B) */
    u32 *naviStatShadow;             /* +0x98: block +0x3CE8 (0x8 B) */
};

#define gToolkit ((struct Toolkit *)0x02009E70)

#endif /* MMBN4_TOOLKIT_H */
