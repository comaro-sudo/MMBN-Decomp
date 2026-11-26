/*
 * Portable C chip folder validation & battle deck shuffle (Phase C3/D1).
 *
 * Covers MMBN4's 30-chip folder legality rules and Fisher-Yates battle deck
 * shuffling using the game's LCG RNG (RngNext from core_progress.c):
 *   - Folder size = 30 chips
 *   - Max 4 copies of any Standard chip ID
 *   - Max 1 copy of any Mega / Giga chip ID
 *   - Total Mega chips <= megaLimit (default 5, expandable via NaviCust)
 *   - Total Giga chips <= gigaLimit (default 1, expandable via NaviCust)
 *   - Regular chip memory check (chipMb <= regMemory)
 */
#include "toolkit.h"

u32 StepRNG(u32 *rng);

#define FOLDER_DECK_SIZE 30u

enum FolderError {
    FOLDER_OK              = 0,
    FOLDER_ERR_BAD_COUNT   = 1,
    FOLDER_ERR_TOO_MANY_ID = 2,
    FOLDER_ERR_MEGA_LIMIT  = 3,
    FOLDER_ERR_GIGA_LIMIT  = 4,
    FOLDER_ERR_DARK_BANNED = 5
};

struct FolderChipMeta {
    u16 chipId;
    u8 code;
    u8 chipClass; /* 0=Standard, 1=Mega, 2=Giga, 3=Dark */
};

/*
 * Validate a 30-chip folder against MMBN4's legality rules.
 * Returns FOLDER_OK (0) if legal, or the first FolderError encountered.
 */
u32 FolderValidate(const struct FolderChipMeta *chips, u32 count,
                   u8 megaLimit, u8 gigaLimit) {
    u32 i, j;
    u32 megaCount = 0;
    u32 gigaCount = 0;

    if (chips == 0 || count != FOLDER_DECK_SIZE) {
        return FOLDER_ERR_BAD_COUNT;
    }

    for (i = 0; i < count; i++) {
        u32 copies = 0;
        u8 cls = chips[i].chipClass;

        if (cls == 3u) {
            return FOLDER_ERR_DARK_BANNED; /* DarkChips cannot sit in folder */
        }
        if (cls == 1u) {
            megaCount++;
        } else if (cls == 2u) {
            gigaCount++;
        }

        for (j = 0; j < count; j++) {
            if (chips[j].chipId == chips[i].chipId) {
                copies++;
            }
        }
        if (cls == 0u && copies > 4u) {
            return FOLDER_ERR_TOO_MANY_ID;
        }
        if ((cls == 1u || cls == 2u) && copies > 1u) {
            return FOLDER_ERR_TOO_MANY_ID;
        }
    }

    if (megaCount > (u32)megaLimit) {
        return FOLDER_ERR_MEGA_LIMIT;
    }
    if (gigaCount > (u32)gigaLimit) {
        return FOLDER_ERR_GIGA_LIMIT;
    }
    return FOLDER_OK;
}

/*
 * Return 1 if a chip requiring chipMb megabytes can be tagged as Regular
 * Chip given the player's regMemory capacity.
 */
u32 FolderCanSetRegular(u8 chipMb, u8 regMemory) {
    return (chipMb > 0u && chipMb <= regMemory) ? 1u : 0u;
}

/*
 * Shuffle a 30-slot battle deck index array [0..29] in place using MMBN4's
 * LCG RNG. If regIndex < 30, keeps regIndex pinned at deck[0] so the Regular
 * Chip is always drawn first on Turn 1.
 */
void FolderShuffleDeck(u8 deck[FOLDER_DECK_SIZE], u8 regIndex, u32 *rngSeed) {
    u32 i;
    u32 startIdx = 0u;

    for (i = 0; i < FOLDER_DECK_SIZE; i++) {
        deck[i] = (u8)i;
    }

    if (regIndex < FOLDER_DECK_SIZE) {
        u8 tmp = deck[0];
        deck[0] = deck[regIndex];
        deck[regIndex] = tmp;
        startIdx = 1u;
    }

    for (i = FOLDER_DECK_SIZE - 1u; i > startIdx; i--) {
        u32 span = (i - startIdx) + 1u;
        u32 j = startIdx + (StepRNG(rngSeed) % span);
        u8 tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}
