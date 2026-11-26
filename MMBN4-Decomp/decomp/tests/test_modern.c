/* Host unit test suite for MMBN4 portable C modules (Phase E2).
 * Built and executed natively via `make modern-test`. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "battle.h"
#include "gba_io.h"
#include "toolkit.h"

/* core_progress.c */
void CapIncrementGameTimeFrames(struct Toolkit *tk);
u8 GetPlaythrough(const struct Toolkit *tk);
u8 GetPlaythroughTier(const struct Toolkit *tk);
void IncPlaythrough(struct Toolkit *tk);
void SeedRNG(u32 *rng);
u32 StepRNG(u32 *rng);
s32 StepPositiveSignedRNG(u32 *rng);
void EventFlagSet(struct Toolkit *tk, u16 flag);
void EventFlagClear(struct Toolkit *tk, u16 flag);
void EventFlagToggle(struct Toolkit *tk, u16 flag);
int EventFlagTest(const struct Toolkit *tk, u16 flag);

/* core_input.c */
void Joypad_Poll(struct JoypadState *jp, u16 keyInputReg);
int CheckSoftReset(struct Toolkit *tk, int canReset);
void ZeroFillByWord(void *dst, u32 byteLen);
void CopyBytes(const void *src, void *dst, u32 byteLen);
void CopyWords(const void *src, void *dst, u32 byteLen);

/* core_save.c */
void RebuildSavePointers(struct Toolkit *tk, u8 *saveBlockBase, u32 shift);
void XorSaveImage(u8 *ewramImage, u32 byteLen, u32 *keyWordSlot);
u32 ComputeSaveChecksum(const u8 *ewramImage, struct ProgressState *progress, u32 versionKey);
void WriteSaveMagic(struct ProgressState *progress);
int VerifySaveMagic(const struct ProgressState *progress);

/* core_inventory.c */
const struct BattleChipData *GetChipRecord(const struct BattleChipData *table, u16 chipId);
u16 FolderEntryChipId(u16 entry);
u8 FolderEntryChipCode(u16 entry);
u16 FolderMakeEntry(u16 chipId, u8 code);
u32 ChipPackCountById(const struct Toolkit *tk, u16 chipId);
u32 AddZenny(struct Toolkit *tk, s32 delta);
u32 AddBugFrags(struct Toolkit *tk, s32 delta);
u8 GetItemCount(const struct Toolkit *tk, u8 itemId);
u8 AddItemClamped(struct Toolkit *tk, u8 itemId, u8 amount, u8 cap);

/* core_battle_soul.c */
enum DoubleSoulId FindDoubleSoulForChip(
    const struct Toolkit *tk,
    const struct BattleChipData *chip,
    const u8 soulFlags[12],
    u16 usedSoulBits
);
const u8 *GetAllDoubleSoulFlags(void);

/* core_battle_score.c */
u8 RaiseEnemyIdForPlaythrough(u8 enemyId, u8 enemyRankField, u8 playthrough, u8 battleFlags);
s32 ScoreBustingFlinch(s32 flinchCount);
s32 ScoreBustingMovement(s32 moveCount);
s32 ScoreBustingMultiDelete(u8 maxSimultaneousDeletes);
s32 ScoreBustingHpRatio(u16 currentHp, u16 maxHp);
u8 ClampBustingLevel(s32 rawScore);

static u8 gTestSaveBlock[SAVE_BLOCK_SIZE + 0x200];
static u8 gTestEwramImage[0x73D2];

static void test_progress_and_flags(void) {
    struct Toolkit tk;
    u32 rng = 0;

    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x16C);

    /* Verify save-block relocation offsets */
    assert((u8 *)tk.gameState == gTestSaveBlock + 0x16C + 0x0000);
    assert((u8 *)tk.progress  == gTestSaveBlock + 0x16C + 0x0084);
    assert(tk.eventFlags      == gTestSaveBlock + 0x16C + 0x0118);
    assert((u8 *)tk.naviStatShadow == gTestSaveBlock + 0x16C + 0x3CE8);

    /* Playthrough tier capping */
    assert(GetPlaythrough(&tk) == 0 && GetPlaythroughTier(&tk) == 0);
    IncPlaythrough(&tk);
    assert(GetPlaythrough(&tk) == 1 && GetPlaythroughTier(&tk) == 1);
    IncPlaythrough(&tk);
    assert(GetPlaythrough(&tk) == 2 && GetPlaythroughTier(&tk) == 2);
    IncPlaythrough(&tk);
    assert(GetPlaythrough(&tk) == 3 && GetPlaythroughTier(&tk) == 2);

    /* Game time capping */
    tk.progress->gameTimeFrames = 0x014988EFu;
    CapIncrementGameTimeFrames(&tk);
    assert(tk.progress->gameTimeFrames == 0x014988F0u);
    CapIncrementGameTimeFrames(&tk);
    assert(tk.progress->gameTimeFrames == 0x014988F0u);

    /* MSB-first event flag bitfield */
    assert(!EventFlagTest(&tk, 0x0017));
    EventFlagSet(&tk, 0x0017);
    assert(EventFlagTest(&tk, 0x0017));
    assert(tk.eventFlags[0x0017 >> 3] == (0x80u >> (0x0017 & 7)));
    EventFlagToggle(&tk, 0x0017);
    assert(!EventFlagTest(&tk, 0x0017));
    EventFlagSet(&tk, 0x1166);
    assert(EventFlagTest(&tk, 0x1166));
    EventFlagClear(&tk, 0x1166);
    assert(!EventFlagTest(&tk, 0x1166));

    /* RNG exact bit-rotation + XOR step */
    SeedRNG(&rng);
    assert(rng == RNG_INIT_SEED);
    assert(StepRNG(&rng) == ((((RNG_INIT_SEED << 1) | (RNG_INIT_SEED >> 31)) + 1u) ^ RNG_XOR_MASK));
    assert(StepPositiveSignedRNG(&rng) >= 0);
}

static void test_input_and_memory(void) {
    struct Toolkit tk;
    struct MainState ms;
    struct JoypadState jp;
    u32 src[4] = {0x11223344u, 0x55667788u, 0x99AABBCCu, 0xDDEEFF00u};
    u32 dst[4] = {0};

    memset(&tk, 0, sizeof(tk));
    memset(&ms, 0, sizeof(ms));
    memset(&jp, 0, sizeof(jp));
    tk.mainState = &ms;
    tk.joypad = &jp;

    /* Press A+B+Start+Select (active-low in REG_KEYINPUT) */
    Joypad_Poll(&jp, (u16)~(KEY_A | KEY_B | KEY_SELECT | KEY_START));
    assert(jp.held == (KEY_A | KEY_B | KEY_SELECT | KEY_START));
    assert(jp.pressed == (KEY_A | KEY_B | KEY_SELECT | KEY_START));
    assert(CheckSoftReset(&tk, 1) == 1);
    assert(ms.softResetTimer == 10);
    assert(CheckSoftReset(&tk, 1) == 0); /* cooled down */

    CopyWords(src, dst, sizeof(src));
    assert(memcmp(src, dst, sizeof(src)) == 0);
    ZeroFillByWord(dst, sizeof(dst));
    assert(dst[0] == 0 && dst[3] == 0);
}

static void test_save_inventory_and_souls(void) {
    struct Toolkit tk;
    struct BattleChipData chip;
    u32 keySlot = 0x1234565Au;
    u32 sum1, sum2;

    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    memset(gTestEwramImage, 0, sizeof(gTestEwramImage));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x0BC);

    /* Save magic + checksum + XOR round-trip */
    WriteSaveMagic(tk.progress);
    assert(VerifySaveMagic(tk.progress) == 1);
    gTestEwramImage[0] = 0x42;
    gTestEwramImage[0x73D1] = 0x10;
    sum1 = ComputeSaveChecksum(gTestEwramImage, tk.progress, 0x16);
    assert(sum1 == 0x42 + 0x10 + 0x16);

    XorSaveImage(gTestEwramImage, sizeof(gTestEwramImage), &keySlot);
    assert(gTestEwramImage[0] == (0x42 ^ 0x5A));
    XorSaveImage(gTestEwramImage, sizeof(gTestEwramImage), &keySlot);
    assert(gTestEwramImage[0] == 0x42);
    sum2 = ComputeSaveChecksum(gTestEwramImage, tk.progress, 0x16);
    assert(sum2 == sum1);

    /* Folder encoding & Zenny/BugFrags/Pack */
    u16 entry = FolderMakeEntry(149, 26); /* chip 149, '*' code */
    assert(FolderEntryChipId(entry) == 149);
    assert(FolderEntryChipCode(entry) == 26);

    tk.chipPack[1][0] = 2;
    tk.chipPack[1][1] = 3;
    assert(ChipPackCountById(&tk, 1) == 5);
    assert(AddZenny(&tk, 1500000) == 999999u);
    assert(AddBugFrags(&tk, 50000) == 9999u);
    assert(AddItemClamped(&tk, 0xB0, 10, 4) == 4);
    assert(GetItemCount(&tk, 0xB0) == 4);

    /* Double Soul unite check (Roll Soul = flag 0x17, kind 0x04; Wood Soul = flag 0x22, kind 0x03) */
    memset(&chip, 0, sizeof(chip));
    chip.kind = 0x04; /* Roll Soul */
    assert(FindDoubleSoulForChip(&tk, &chip, GetAllDoubleSoulFlags(), 0) == SOUL_NONE);
    EventFlagSet(&tk, 0x17);
    assert(FindDoubleSoulForChip(&tk, &chip, GetAllDoubleSoulFlags(), 0) == SOUL_ROLL);
    assert(FindDoubleSoulForChip(&tk, &chip, GetAllDoubleSoulFlags(), 1u << 0) == SOUL_NONE);

    chip.kind = 0x03; /* Wood Soul */
    EventFlagSet(&tk, 0x22);
    assert(FindDoubleSoulForChip(&tk, &chip, GetAllDoubleSoulFlags(), 0) == SOUL_WOOD);
}

static void test_battle_score_and_id_raise(void) {
    /* Virus scaling: +2 per tier, capped by (5 - rankField) */
    assert(RaiseEnemyIdForPlaythrough(0x10, 0, 0, 0) == 0x10);
    assert(RaiseEnemyIdForPlaythrough(0x10, 0, 1, 0) == 0x12);
    assert(RaiseEnemyIdForPlaythrough(0x10, 0, 2, 0) == 0x14);
    assert(RaiseEnemyIdForPlaythrough(0x10, 0, 5, 0) == 0x14); /* tier capped at 2 */
    assert(RaiseEnemyIdForPlaythrough(0x10, 4, 2, 0) == 0x11); /* maxRaise = 1 */
    assert(RaiseEnemyIdForPlaythrough(0x10, 0, 2, 0x80) == 0x10); /* fixed-tier flag */

    /* Navi scaling: +1 per tier, capped by (2 - rankField) */
    assert(RaiseEnemyIdForPlaythrough(0x90, 0, 1, 0) == 0x91);
    assert(RaiseEnemyIdForPlaythrough(0x90, 0, 2, 0) == 0x92);
    assert(RaiseEnemyIdForPlaythrough(0x90, 1, 2, 0) == 0x91);
    assert(RaiseEnemyIdForPlaythrough(0xE4, 0, 2, 0) == 0xE4); /* >= 0xE4 untouched */

    /* Busting Level sub-scores */
    assert(ScoreBustingFlinch(0) == 1);
    assert(ScoreBustingFlinch(1) == 0);
    assert(ScoreBustingFlinch(4) == -3);
    assert(ScoreBustingMovement(2) == 1);
    assert(ScoreBustingMovement(3) == 0);
    assert(ScoreBustingMultiDelete(1) == 0);
    assert(ScoreBustingMultiDelete(3) == 4);
    assert(ScoreBustingHpRatio(49, 100) == 0);
    assert(ScoreBustingHpRatio(50, 100) == 1);
    assert(ScoreBustingHpRatio(75, 100) == 2);
    assert(ScoreBustingHpRatio(100, 100) == 3);
    assert(ClampBustingLevel(-5) == 1);
    assert(ClampBustingLevel(15) == 11);
}

int main(void) {
    test_progress_and_flags();
    test_input_and_memory();
    test_save_inventory_and_souls();
    test_battle_score_and_id_raise();
    printf("modern-test: ALL PASSED\n");
    return 0;
}
