/* Phase C3 / C4 / E2 portable C reference implementations of MMBN4 tournament
 * bracket round tracking (TournamentGetRound sub_08043F4C) and Mystery Data
 * state/advancement helpers (sub_080ADE38 / sub_080ADE5C) from code_03/06.
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "toolkit.h"

#define TOURNAMENT_ENTRANT_MEGAMAN 0x1Eu
#define TOURNAMENT_NOT_ENTERED     4u
#define MYSTERY_DATA_FLAG_BASE     0x0D00u
#define MYSTERY_DATA_SLOT_COUNT    1024u

struct TournamentBracketSlot {
    u8 entrantId; /* +0x00: 0x1E = MegaMan */
    u8 roundBits; /* +0x01: bit0=R1 win, bit1=R2 win, bit2=R3 (champion) */
    u8 unk02;     /* +0x02 */
    u8 unk03;     /* +0x03 */
};

struct MysteryDataEntry {
    u8 type;      /* +0x00: 0 = end of map list, 5 = skip */
    u8 pad01;     /* +0x01 */
    u16 flag;     /* +0x02: event flag in 0x0D00..0x10FF */
    u32 arg1;     /* +0x04 */
    u32 arg2;     /* +0x08 */
};

/* 0x08043F4C: TournamentGetRound
 * Returns 4 if not entered, 0=QF, 1=SF, 2=Final, 3=Champion. */
u8 TournamentGetRound(
    const u8 *tournamentEntered,
    const struct TournamentBracketSlot (*brackets)[8],
    u8 tournamentId
) {
    int i;

    if (!tournamentEntered[tournamentId])
        return TOURNAMENT_NOT_ENTERED;

    for (i = 0; i < 8; i++) {
        if (brackets[tournamentId][i].entrantId == TOURNAMENT_ENTRANT_MEGAMAN) {
            u8 bits = (u8)(brackets[tournamentId][i].roundBits & 0x0Fu);
            if (bits == 0)
                return 0;
            if (bits & 4u)
                return 3;
            if (bits & 2u)
                return 2;
            if (bits & 1u)
                return 1;
            return 0;
        }
    }
    return 0;
}

/* 0x080ADE38: MysteryDataSetSlot (stores 2-byte state for flag 0x0D00..0x10FF) */
int MysteryDataSetSlot(struct Toolkit *tk, u16 flag, u8 lo, u8 hi) {
    u32 idx;
    if (flag < MYSTERY_DATA_FLAG_BASE)
        return 0;
    idx = (u32)(flag - MYSTERY_DATA_FLAG_BASE);
    if (idx >= MYSTERY_DATA_SLOT_COUNT)
        return 0;
    tk->mysteryData[idx] = (u16)((u16)lo | ((u16)hi << 8));
    return 1;
}

u16 MysteryDataGetSlot(const struct Toolkit *tk, u16 flag) {
    u32 idx;
    if (flag < MYSTERY_DATA_FLAG_BASE)
        return 0;
    idx = (u32)(flag - MYSTERY_DATA_FLAG_BASE);
    if (idx >= MYSTERY_DATA_SLOT_COUNT)
        return 0;
    return tk->mysteryData[idx];
}

/* Advances a single map's Mystery Data list (type 0 ends, type 5 skips),
 * clearing each taken flag in tk->eventFlags. Returns number of entries updated. */
u32 MysteryDataResetList(struct Toolkit *tk, const struct MysteryDataEntry *entries) {
    u32 count = 0;

    while (entries->type != 0) {
        if (entries->type != 5) {
            u16 flag = entries->flag;
            tk->eventFlags[flag >> 3] &= (u8)~(0x80u >> (flag & 7));
            count++;
        }
        entries++;
    }
    return count;
}
