/*
 * Portable C Save-Screen Playtime Formatter & Integrity Validator (Phase C3).
 *
 * Models MMBN4's Continue/Save screen helpers:
 *   - Frame count (60 FPS) -> HH:MM:SS (clamped to 99:59:59)
 *   - Combined magic + checksum validation
 *   - Title Screen completion star count (0..6)
 */
#include "modern_api.h"

/*
 * Convert `gameTimeFrames` (at 60 FPS) into hours (0..99), minutes (0..59),
 * and seconds (0..59).
 */
void SaveFormatPlaytime(u32 gameTimeFrames, u8 *outHours, u8 *outMinutes, u8 *outSeconds) {
    u32 totalSec = gameTimeFrames / 60u;
    u32 hrs = totalSec / 3600u;
    u32 mins = (totalSec % 3600u) / 60u;
    u32 secs = totalSec % 60u;

    if (hrs > 99u) {
        hrs = 99u;
        mins = 59u;
        secs = 59u;
    }
    if (outHours != 0) {
        *outHours = (u8)hrs;
    }
    if (outMinutes != 0) {
        *outMinutes = (u8)mins;
    }
    if (outSeconds != 0) {
        *outSeconds = (u8)secs;
    }
}

/*
 * Verify both the save magic string ("ROCKMANEXE4 20031022") and the
 * EWRAM checksum against `progress->checksum`. Returns 1 if valid, 0 if corrupt.
 */
u32 SaveValidateIntegrity(const u8 *ewramImage, struct ProgressState *progress, u32 versionKey) {
    u32 stored, expected;

    if (ewramImage == 0 || progress == 0) {
        return 0u;
    }
    if (!VerifySaveMagic(progress)) {
        return 0u;
    }
    stored = progress->checksum;
    expected = ComputeSaveChecksum(ewramImage, progress, versionKey);
    progress->checksum = stored;
    return (expected == stored) ? 1u : 0u;
}

/*
 * Count earned Title Screen completion stars from a bitmask (bits 0..5).
 */
u32 SaveCountTitleStars(u8 starMask) {
    u32 count = 0u;
    u32 m = (u32)(starMask & 0x3Fu);

    while (m != 0u) {
        count += (m & 1u);
        m >>= 1;
    }
    return count;
}
