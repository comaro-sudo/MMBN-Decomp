/* Phase C3 / E2 portable C reference implementations of MMBN4 chip table,
 * folder decoding, chip pack, item inventory, and Zenny/BugFrag helpers
 * from code_01.s and code_02.s.
 *
 * Verified with `make modern-check` (-Wall -Wextra -Werror). */

#include "battle.h"
#include "toolkit.h"

#define MAX_ZENNY    999999u
#define MAX_BUGFRAGS 9999u

/* 0x080190D0: BN4_GET_CHIP */
const struct BattleChipData *GetChipRecord(const struct BattleChipData *table, u16 chipId) {
    return &table[chipId];
}

/* Folder entry bitfield: bits 0..8 = chip id, bits 9..15 = code (26 = '*') */
u16 FolderEntryChipId(u16 entry) {
    return (u16)(entry & 0x01FFu);
}

u8 FolderEntryChipCode(u16 entry) {
    return (u8)(entry >> 9);
}

u16 FolderMakeEntry(u16 chipId, u8 code) {
    return (u16)((chipId & 0x01FFu) | ((u16)code << 9));
}

/* 0x0801922C: ChipPackCountById (sum of owned counts across 4 chip codes) */
u32 ChipPackCountById(const struct Toolkit *tk, u16 chipId) {
    const u8 *row = tk->chipPack[chipId];
    return (u32)row[0] + row[1] + row[2] + row[3];
}

/* Zenny & BugFrag helpers (capped per GameState spec) */
u32 AddZenny(struct Toolkit *tk, s32 delta) {
    s32 val = (s32)tk->gameState->zenny + delta;
    if (val < 0)
        val = 0;
    else if ((u32)val > MAX_ZENNY)
        val = (s32)MAX_ZENNY;
    tk->gameState->zenny = (u32)val;
    return (u32)val;
}

u32 AddBugFrags(struct Toolkit *tk, s32 delta) {
    s32 val = (s32)tk->gameState->bugFrags + delta;
    if (val < 0)
        val = 0;
    else if ((u32)val > MAX_BUGFRAGS)
        val = (s32)MAX_BUGFRAGS;
    tk->gameState->bugFrags = (u32)val;
    return (u32)val;
}

u8 GetItemCount(const struct Toolkit *tk, u8 itemId) {
    return tk->items[itemId];
}

u8 AddItemClamped(struct Toolkit *tk, u8 itemId, u8 amount, u8 cap) {
    u32 next = (u32)tk->items[itemId] + amount;
    if (next > cap)
        next = cap;
    tk->items[itemId] = (u8)next;
    return (u8)next;
}
