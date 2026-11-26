/* Phase C3 / E2 portable C reference implementations of MMBN4 save-block
 * anti-cheat relocation, pointer rebuild, XOR scrambling, magic, and checksum
 * routines from code_00.s and code_03.s.
 *
 * Verified with `make modern-check` (-Wall -Wextra -Werror). */

#include "toolkit.h"

#define SAVE_IMAGE_SIZE      0x73D2u
#define SAVE_CHECKSUM_KEY_RS 0x16u
#define SAVE_CHECKSUM_KEY_BM 0x22u

static const char gSaveMagic[20] = "ROCKMANEXE4 20031022";

/* ROM table at 0x080061AC (same in RS and BM): 23 relocatable save-block offsets */
static const u32 gSaveSlotOffsets[23] = {
    0x0000, /* +0x40 gameState */
    0x0084, /* +0x44 progress */
    0x0118, /* +0x48 eventFlags */
    0x04FC, /* +0x4C folders */
    0x05B4, /* +0x50 chipPack */
    0x14B8, /* +0x54 items */
    0x164C, /* +0x58 shopStock */
    0x2410, /* +0x5C naviCustGrid */
    0x2434, /* +0x60 naviCustPlaced */
    0x2500, /* +0x64 naviCustBugs */
    0x2514, /* +0x68 activeModCard */
    0x2518, /* +0x6C maxHpBonus */
    0x251C, /* +0x70 modCardSlots */
    0x252C, /* +0x74 mysteryData */
    0x2D30, /* +0x78 naviStats */
    0x2F34, /* +0x7C naviAi */
    0x3638, /* +0x80 itemKeys */
    0x37CC, /* +0x84 chipKeysMirror */
    0x39D0, /* +0x88 chipKeys */
    0x3BD4, /* +0x8C zennyShadow */
    0x3BDC, /* +0x90 bugFragShadow */
    0x3BE4, /* +0x94 byteShadows */
    0x3CE8, /* +0x98 naviStatShadow */
};

/* 0x0800616A: RebuildSavePointers */
void RebuildSavePointers(struct Toolkit *tk, u8 *saveBlockBase, u32 shift) {
    u8 **slots = (u8 **)&tk->gameState;
    u8 *base = saveBlockBase + (shift & 0x1FEu);
    int i;

    for (i = 0; i < 23; i++)
        slots[i] = base + gSaveSlotOffsets[i];
}

/* 0x08006338 / 0x08006350: ScrambleSaveImage / UnscrambleSaveImage */
void XorSaveImage(u8 *ewramImage, u32 byteLen, u32 *keyWordSlot) {
    u32 key = *keyWordSlot;
    u8 keyByte = (u8)(key & 0xFFu);
    u32 i;

    for (i = byteLen; i > 0; i--)
        ewramImage[i - 1] ^= keyByte;

    /* Key word at 0x02001554 is always stored in the clear */
    *keyWordSlot = key;
}

/* 0x08048960: ComputeSaveChecksum */
u32 ComputeSaveChecksum(const u8 *ewramImage, struct ProgressState *progress, u32 versionKey) {
    u32 sum = 0;
    u32 i;

    progress->checksum = 0;
    for (i = 0; i < SAVE_IMAGE_SIZE; i++)
        sum += ewramImage[i];

    sum += versionKey;
    progress->checksum = sum;
    return sum;
}

/* 0x0804897C: WriteSaveMagic */
void WriteSaveMagic(struct ProgressState *progress) {
    int i;
    for (i = 0; i < 20; i++)
        progress->magic[i] = gSaveMagic[i];
}

/* 0x080489C0: VerifySaveMagic */
int VerifySaveMagic(const struct ProgressState *progress) {
    int i;
    for (i = 0; i < 20; i++) {
        if (progress->magic[i] != gSaveMagic[i])
            return 0;
    }
    return 1;
}
