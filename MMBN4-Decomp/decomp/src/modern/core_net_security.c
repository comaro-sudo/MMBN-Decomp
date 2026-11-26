/*
 * Portable C Cyberworld Security Gates & Frequency Tuner Helpers (Phase C3/C4).
 *
 * Models MMBN4's security cube / Black Earth gate checks and Violet Eclipse's
 * Frequency Tuner encounter/GMD tier override (WP-B2 / WP-B4).
 */
#include "modern_api.h"

/*
 * Return 1 if MegaMan meets the Black Earth 1 gate requirement:
 * at least `requiredSouls` base Souls (6 in vanilla & Violet Eclipse)
 * AND the given story/Evil-chip gate flag is set (or bypassed when gateFlag == 0).
 */
u32 NetCanOpenBlackEarthGate(const struct Toolkit *tk, u32 soulMask,
                             u32 requiredSouls, u16 gateFlag) {
    if (!LibraryHasSoulStar(soulMask, requiredSouls)) {
        return 0;
    }
    if (gateFlag != 0u && (tk == 0 || !EventFlagTest(tk, gateFlag))) {
        return 0;
    }
    return 1;
}

/*
 * Check whether a security cube requiring `keyItemId` can be unlocked
 * (either already opened via `openedFlag`, or player owns >= 1 of `keyItemId`).
 */
u32 NetCanUnlockSecurityCube(const struct Toolkit *tk, u16 openedFlag, u8 keyItemId) {
    if (tk == 0) {
        return 0;
    }
    if (EventFlagTest(tk, openedFlag)) {
        return 1;
    }
    return (GetItemCount(tk, keyItemId) > 0u) ? 1u : 0u;
}

/*
 * Resolve effective playthrough tier (0=1st, 1=2nd, 2=3rd+) under Violet
 * Eclipse's Frequency Tuner (WP-B2/WP-B4):
 *   tunerSetting == 0 -> follow actual save playthrough tier (0..2)
 *   tunerSetting 1..3 -> force tier (tunerSetting - 1)
 */
u8 NetResolveTunerTier(const struct Toolkit *tk, u8 tunerSetting) {
    if (tunerSetting >= 1u && tunerSetting <= 3u) {
        return (u8)(tunerSetting - 1u);
    }
    return (tk != 0) ? GetPlaythroughTier(tk) : 0u;
}
