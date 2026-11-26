/* Phase D1 / E2 portable C reference implementation of MMBN4 Double Soul
 * eligibility check (BN4_UNITE_CHECK sub_0801FF84) from code_01.s.
 *
 * Verified with `make modern-check` (-Wall -Wextra -Werror). */

#include "battle.h"
#include "toolkit.h"

/* 0x08020008 (RS & BM identical): chip kind that unites with each Soul 1..12 */
static const u8 gSoulKinds[12] = {
    0x04, 0x08, 0x0A, 0x07, 0x00, 0x02, /* RS: Roll, Guts, Wind, Search, Fire, Thunder */
    0x06, 0x05, 0x0B, 0x09, 0x01, 0x03, /* BM: Proto, Number, Metal, Junk, Aqua, Wood */
};

/* 0x08020018: event flags for all 12 Double Souls (RS 0x17..0x1C, BM 0x1D..0x22) */
static const u8 gSoulFlagsAll[12] = {
    0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
};

static int SoulEventFlagTest(const struct Toolkit *tk, u16 flag) {
    return (tk->eventFlags[flag >> 3] & (u8)(0x80u >> (flag & 7))) != 0;
}

/* Returns DoubleSoulId (1..12) if `chip` can trigger Double Soul, else SOUL_NONE (0).
 * `soulFlags` may be RS-only, BM-only, or gSoulFlagsAll (Violet Eclipse). */
enum DoubleSoulId FindDoubleSoulForChip(
    const struct Toolkit *tk,
    const struct BattleChipData *chip,
    const u8 soulFlags[12],
    u16 usedSoulBits
) {
    int i;
    u8 kind = chip->kind;

    for (i = 11; i >= 0; i--) {
        if (gSoulKinds[i] == kind) {
            u8 flag = soulFlags[i];
            if (flag == 0xFF)
                return SOUL_NONE;
            if (!SoulEventFlagTest(tk, flag))
                return SOUL_NONE;
            if (usedSoulBits & (u16)(1u << i))
                return SOUL_NONE;
            return (enum DoubleSoulId)(i + 1);
        }
    }
    return SOUL_NONE;
}

const u8 *GetAllDoubleSoulFlags(void) {
    return gSoulFlagsAll;
}
