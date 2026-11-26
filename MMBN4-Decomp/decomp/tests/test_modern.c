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

/* core_world.c */
struct TournamentBracketSlot {
    u8 entrantId;
    u8 roundBits;
    u8 unk02;
    u8 unk03;
};
struct MysteryDataEntry {
    u8 type;
    u8 pad01;
    u16 flag;
    u32 arg1;
    u32 arg2;
};
u8 TournamentGetRound(
    const u8 *tournamentEntered,
    const struct TournamentBracketSlot (*brackets)[8],
    u8 tournamentId
);
int MysteryDataSetSlot(struct Toolkit *tk, u16 flag, u8 lo, u8 hi);
u16 MysteryDataGetSlot(const struct Toolkit *tk, u16 flag);
u32 MysteryDataResetList(struct Toolkit *tk, const struct MysteryDataEntry *entries);

/* core_shop.c */
struct ShopStockEntry {
    u8 type;
    u8 qty;
    u16 id;
    u8 code;
    u8 isNew;
    u16 price;
};
u32 ShopPriceToMoney(u32 shopKind, u16 rawPrice);
struct ShopStockEntry *ShopGetStock(struct Toolkit *tk, u8 shopIndex);
int ShopHasStock(const struct ShopStockEntry *slots, u32 slotCount);
int ShopTakeStock(struct ShopStockEntry *slots, u32 slotCount, u8 type, u16 id, u8 code, int shiftUp);
int ShopRestockItem(struct ShopStockEntry *slots, u32 slotCount, u16 id);

/* core_navicust.c */
void NaviCustClearGrid(struct Toolkit *tk);
int NaviCustSetCell(struct Toolkit *tk, u8 x, u8 y, u8 slotIndex);
u8 NaviCustGetCell(const struct Toolkit *tk, u8 x, u8 y);
u32 NaviCustCountPlaced(const struct Toolkit *tk);
int NaviCustOnCommandLine(const struct Toolkit *tk, u8 slotIndex);

/* core_battle_grid.c */
struct BattlePanel {
    u8 owner;
    u8 state;
    u16 crackTimer;
};
int BattleGridInBounds(u8 col, u8 row);
int BattleGridIndex(u8 col, u8 row);
void BattleGridInitDefault(struct BattlePanel grid[18]);
int BattleIsElementWeakness(u8 attackerElem, u8 defenderElem);
u16 BattleCalcHitDamage(u16 baseDamage, u8 attackerElem, u8 defenderElem, u8 panelState);
void BattlePanelCrack(struct BattlePanel *panel, int forceBreak);
int BattlePanelIsWalkable(const struct BattlePanel *panel, u8 hasAirShoes, u8 hasFloatShoes);

/* core_custom_screen.c */
struct CustomChipSlot {
    u16 chipId;
    u8 code;
    u8 selected;
};
struct ProgramAdvanceRecipe {
    u16 paChipId;
    u16 chips[3];
    u8 codes[3];
};
int CustomGaugeStep(u16 *gauge, u16 step);
int CustomCanSelectChip(const struct CustomChipSlot *selected, u32 selectedCount, u16 chipId, u8 code);
int CustomMatchesPA(const struct CustomChipSlot *seq, const struct ProgramAdvanceRecipe *recipe);
u16 CustomFindPA(const struct CustomChipSlot *selected, u32 selectedCount, const struct ProgramAdvanceRecipe *recipes, u32 recipeCount, u32 *outStart);

/* core_emotion.c */
enum EmotionState {
    EMOTION_NORMAL       = 0,
    EMOTION_FULL_SYNCHRO = 1,
    EMOTION_ANGER        = 2,
    EMOTION_ANXIOUS      = 3,
    EMOTION_EVIL         = 4,
    EMOTION_DARK_SOUL    = 5,
};
s16 EmotionAdjustKarma(s16 karma, s16 delta);
int EmotionIsEvilKarma(s16 karma);
enum EmotionState EmotionOnCounterHit(enum EmotionState cur, s16 karma);
enum EmotionState EmotionOnTakeHit(enum EmotionState cur, u16 damage, u16 maxHp);
u8 EmotionChipDamageMultiplier(enum EmotionState cur);

/* core_enemy_ai.c */
u16 BossCalcSpRecordBonus(u32 bestFrames);
u16 BossScaleSpPlusHp(u16 baseHp);
u16 BossScaleSpPlusAtk(u16 baseAtk);
u16 BossScaleSpPlusSpeed(u16 baseSpeed);
u8 BossAdvanceRematchRank(u8 currentRank, u8 bustingLevel, int requireSRank);

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

static void test_world_tournaments_and_md(void) {
    struct Toolkit tk;
    u8 entered[3] = {0, 1, 1};
    struct TournamentBracketSlot brackets[3][8];
    struct MysteryDataEntry mdList[4] = {
        {1, 0, 0x0D05, 0, 0},
        {5, 0, 0x0D06, 0, 0}, /* type 5 skipped */
        {2, 0, 0x0D07, 0, 0},
        {0, 0, 0, 0, 0},      /* terminator */
    };

    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    memset(brackets, 0, sizeof(brackets));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x040);

    /* TournamentGetRound */
    assert(TournamentGetRound(entered, brackets, 0) == 4); /* not entered */
    brackets[1][3].entrantId = 0x1E;
    brackets[1][3].roundBits = 0x00;
    assert(TournamentGetRound(entered, brackets, 1) == 0);
    brackets[1][3].roundBits = 0x01;
    assert(TournamentGetRound(entered, brackets, 1) == 1);
    brackets[1][3].roundBits = 0x03;
    assert(TournamentGetRound(entered, brackets, 1) == 2);
    brackets[1][3].roundBits = 0x07;
    assert(TournamentGetRound(entered, brackets, 1) == 3);

    /* Mystery Data slots & list reset */
    assert(MysteryDataSetSlot(&tk, 0x0D05, 0x34, 0x12) == 1);
    assert(MysteryDataGetSlot(&tk, 0x0D05) == 0x1234);
    assert(MysteryDataSetSlot(&tk, 0x0100, 0, 0) == 0); /* out of range */

    EventFlagSet(&tk, 0x0D05);
    EventFlagSet(&tk, 0x0D06);
    EventFlagSet(&tk, 0x0D07);
    assert(MysteryDataResetList(&tk, mdList) == 2);
    assert(!EventFlagTest(&tk, 0x0D05));
    assert(EventFlagTest(&tk, 0x0D06)); /* type 5 stayed set */
    assert(!EventFlagTest(&tk, 0x0D07));
}

static void test_shop_and_navicust(void) {
    struct Toolkit tk;
    struct ShopStockEntry *s0;

    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x020);

    /* Shop pricing & stock */
    assert(ShopPriceToMoney(0, 76) == 7600u);
    assert(ShopPriceToMoney(1, 100) == 100u);

    s0 = ShopGetStock(&tk, 0);
    s0[0].type = 1; s0[0].qty = 1; s0[0].id = 160; s0[0].code = 0xFF; s0[0].price = 25;
    s0[1].type = 2; s0[1].qty = 0xFF; s0[1].id = 1; s0[1].code = 0; s0[1].price = 8;
    assert(ShopHasStock(s0, 8) == 1);
    assert(ShopTakeStock(s0, 8, 1, 160, 0xFF, 1) == 1);
    assert(s0[0].type == 2 && s0[0].id == 1); /* shifted up */
    assert(ShopTakeStock(s0, 8, 2, 1, 0, 1) == 1);
    assert(s0[0].qty == 0xFF); /* unlimited stays 0xFF */
    s0[0].qty = 2;
    assert(ShopRestockItem(s0, 8, 1) == 1);
    assert(s0[0].qty == 3);

    /* NaviCust 5x5 grid & command line */
    NaviCustClearGrid(&tk);
    assert(NaviCustSetCell(&tk, 2, 2, 1) == 1);
    assert(NaviCustSetCell(&tk, 5, 0, 1) == 0);
    assert(NaviCustGetCell(&tk, 2, 2) == 1);
    assert(NaviCustOnCommandLine(&tk, 1) == 1);
    assert(NaviCustOnCommandLine(&tk, 2) == 0);
    tk.naviCustPlaced[0] = 0xC0;
    tk.naviCustPlaced[8] = 0xC1;
    assert(NaviCustCountPlaced(&tk) == 2);
}

static void test_battle_grid_and_damage(void) {
    struct BattlePanel grid[18];

    assert(!BattleGridInBounds(0, 1));
    assert(!BattleGridInBounds(7, 1));
    assert(BattleGridInBounds(1, 1) && BattleGridIndex(1, 1) == 0);
    assert(BattleGridInBounds(6, 3) && BattleGridIndex(6, 3) == 17);

    BattleGridInitDefault(grid);
    assert(grid[BattleGridIndex(3, 2)].owner == 0);
    assert(grid[BattleGridIndex(4, 2)].owner == 1);

    /* Element triangle & panel multipliers */
    assert(BattleIsElementWeakness(1, 4) == 1); /* Fire > Wood */
    assert(BattleIsElementWeakness(4, 3) == 1); /* Wood > Elec */
    assert(BattleIsElementWeakness(3, 2) == 1); /* Elec > Aqua */
    assert(BattleIsElementWeakness(2, 1) == 1); /* Aqua > Fire */
    assert(BattleIsElementWeakness(1, 2) == 0);

    assert(BattleCalcHitDamage(100, 1, 4, 0) == 200); /* Fire vs Wood */
    assert(BattleCalcHitDamage(100, 1, 4, 4) == 400); /* Fire vs Wood on Grass */
    assert(BattleCalcHitDamage(100, 3, 0, 5) == 200); /* Elec on Ice */
    assert(BattleCalcHitDamage(100, 0, 0, 7) == 50);  /* Holy panel halves */

    /* Panel cracking & AirShoes */
    BattlePanelCrack(&grid[0], 0);
    assert(grid[0].state == 1 && BattlePanelIsWalkable(&grid[0], 0, 0) == 1);
    BattlePanelCrack(&grid[0], 0);
    assert(grid[0].state == 2 && BattlePanelIsWalkable(&grid[0], 0, 0) == 0);
    assert(BattlePanelIsWalkable(&grid[0], 1, 0) == 1);
}

static void test_custom_screen_and_pa(void) {
    u16 gauge = 0x3FE0u;
    struct CustomChipSlot sel[4] = {
        {1, 0, 1},  /* Cannon A */
        {1, 1, 1},  /* Cannon B */
        {1, 2, 1},  /* Cannon C */
        {29, 26, 1} /* AreaGrab * */
    };
    struct ProgramAdvanceRecipe recipes[1] = {
        {500, {1, 1, 1}, {0xFF, 0xFF, 0xFF}}, /* GigaCannon (Cannon A-B-C) */
    };
    u32 paStart = 99;

    /* Custom Gauge */
    assert(CustomGaugeStep(&gauge, 16) == 0 && gauge == 0x3FF0u);
    assert(CustomGaugeStep(&gauge, 32) == 1 && gauge == 0x4000u);

    /* Selection compatibility */
    assert(CustomCanSelectChip(sel, 1, 1, 1) == 1);   /* same ID, diff code */
    assert(CustomCanSelectChip(sel, 1, 29, 0) == 1);  /* diff ID, same code A */
    assert(CustomCanSelectChip(sel, 1, 29, 26) == 1); /* diff ID, '*' wildcard */
    assert(CustomCanSelectChip(sel, 1, 29, 1) == 0);  /* diff ID, diff code B */
    assert(CustomCanSelectChip(sel, 2, 29, 26) == 0); /* already mixed codes A+B -> must stay same ID */

    /* Program Advance detection */
    assert(CustomFindPA(sel, 4, recipes, 1, &paStart) == 500 && paStart == 0);
    sel[2].code = 4; /* break A-B-C sequence */
    assert(CustomFindPA(sel, 4, recipes, 1, &paStart) == 0);
}

static void test_emotion_and_karma(void) {
    s16 karma = 512;

    karma = EmotionAdjustKarma(karma, -400);
    assert(karma == 112 && EmotionIsEvilKarma(karma) == 1);
    assert(EmotionOnCounterHit(EMOTION_NORMAL, karma) == EMOTION_NORMAL); /* Evil cannot Full Synchro */

    karma = EmotionAdjustKarma(karma, 500);
    assert(karma == 612 && EmotionIsEvilKarma(karma) == 0);
    assert(EmotionOnCounterHit(EMOTION_NORMAL, karma) == EMOTION_FULL_SYNCHRO);
    assert(EmotionChipDamageMultiplier(EMOTION_FULL_SYNCHRO) == 2);
    assert(EmotionOnTakeHit(EMOTION_FULL_SYNCHRO, 10, 100) == EMOTION_NORMAL);
    assert(EmotionOnTakeHit(EMOTION_NORMAL, 30, 100) == EMOTION_ANGER);
    assert(EmotionChipDamageMultiplier(EMOTION_ANGER) == 2);
    assert(EmotionChipDamageMultiplier(EMOTION_NORMAL) == 1);
}

static void test_enemy_ai_and_sp_scaling(void) {
    assert(BossCalcSpRecordBonus(0) == 0);
    assert(BossCalcSpRecordBonus(600) == 100);  /* 10.0s -> +100 */
    assert(BossCalcSpRecordBonus(1200) == 50);  /* 20.0s -> +50 */
    assert(BossCalcSpRecordBonus(1800) == 0);   /* 30.0s -> +0 */

    assert(BossScaleSpPlusHp(2500) == 4095);
    assert(BossScaleSpPlusHp(1200) == 2400);
    assert(BossScaleSpPlusAtk(200) == 300);
    assert(BossScaleSpPlusSpeed(100) == 125);

    assert(BossAdvanceRematchRank(0, 10, 1) == 0); /* needs S-rank */
    assert(BossAdvanceRematchRank(0, 11, 1) == 1);
    assert(BossAdvanceRematchRank(4, 11, 1) == 4); /* capped at SP+ */
}

struct TextScriptState {
    const u8 *buf;
    u32 len;
    u32 pos;
    u8 mugshotId;
    u8 textSpeed;
    u8 delayFrames;
    u8 lastOp;
    u16 lastSoundId;
    u16 lastExtArg;
    u32 lineCount;
};

void TextScriptInit(struct TextScriptState *st, const u8 *buf, u32 len);
u32 TextScriptInstrLen(const u8 *buf, u32 len, u32 pos);
u32 TextScriptStep(struct TextScriptState *st, struct Toolkit *tk, u8 *outGlyph);

static void test_text_script_decoder(void) {
    struct Toolkit tk;
    struct TextScriptState st;
    u8 glyph = 0;
    /* Mugshot(5), 'A'(0x0A), Newline, WaitKey, SetFlag(0x0123), Ext(0x0C, 0x0D), End */
    const u8 script[] = {
        0xEB, 0x05,
        0x0A,
        0xE7,
        0xE8,
        0xF2, 0x02, 0x23, 0x01,
        0xFC, 0x0C, 0x0D,
        0xE6
    };
    const u8 branchScript[] = {
        0xF2, 0x00, 0x23, 0x01, 0x42,
        0xE6
    };

    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x040);

    TextScriptInit(&st, script, sizeof(script));
    assert(TextScriptInstrLen(script, sizeof(script), 0) == 2);
    assert(TextScriptStep(&st, &tk, &glyph) == 1 && st.mugshotId == 5);
    assert(TextScriptStep(&st, &tk, &glyph) == 0 && glyph == 0x0A);
    assert(TextScriptStep(&st, &tk, &glyph) == 1 && st.lineCount == 1);
    assert(TextScriptStep(&st, &tk, &glyph) == 2); /* WaitKey */
    assert(TextScriptStep(&st, &tk, &glyph) == 1 && EventFlagTest(&tk, 0x0123) == 1);
    assert(TextScriptStep(&st, &tk, &glyph) == 1 && st.lastExtArg == 0x0C0Du);
    assert(TextScriptStep(&st, &tk, &glyph) == 3); /* End */
    assert(TextScriptStep(&st, &tk, &glyph) == 4); /* EOF error */

    /* Flag-set branch jumps to 0x42 */
    TextScriptInit(&st, branchScript, sizeof(branchScript));
    assert(TextScriptStep(&st, &tk, &glyph) == 3 && st.lastExtArg == 0x42u);
}

/* core_lz77.c */
u32 Lz77GetUncompressedSize(const u8 *src, u32 srcLen);
u32 Lz77Decompress(const u8 *src, u32 srcLen, u8 *dst, u32 dstCap);

/* core_battle_loop.c */
struct BattleActorState {
    s16 hp;
    s16 maxHp;
    u16 invFrames;
    u16 hitstunFrames;
    u16 invisFrames;
    u8 guardActive;
    u8 shadowActive;
    u8 barrierKind;
    s16 barrierHp;
    u8 deleted;
};

void BattleActorTickTimers(struct BattleActorState *actor);
s32 BattleActorApplyHit(struct BattleActorState *actor, s32 rawDamage, u32 flags);
s32 BattleActorHeal(struct BattleActorState *actor, s32 amount);

static void test_lz77_and_battle_loop(void) {
    /*
     * GBA LZ77 stream encoding "ABABABAB" (8 bytes):
     *   header: 0x10, 0x08, 0x00, 0x00
     *   flags:  0x20 (bit 0=0 literal 'A', bit 1=0 literal 'B',
     *                 bit 2=1 back-ref len=6 (nibble=3), disp=2 (raw=1))
     */
    const u8 lzStream[] = { 0x10, 0x08, 0x00, 0x00, 0x20, 'A', 'B', 0x30, 0x01 };
    u8 out[16];
    struct BattleActorState navi;

    assert(Lz77GetUncompressedSize(lzStream, sizeof(lzStream)) == 8u);
    assert(Lz77Decompress(lzStream, sizeof(lzStream), out, sizeof(out)) == 8u);
    assert(memcmp(out, "ABABABAB", 8) == 0);
    assert(Lz77Decompress(lzStream, sizeof(lzStream), out, 4) == 0u); /* dst too small */

    /* BattleActorState: Aura, Barrier, Invis, Wind, Flinch, Heal, Deletion */
    memset(&navi, 0, sizeof(navi));
    navi.hp = 500;
    navi.maxHp = 500;
    navi.barrierKind = 2; /* Aura 100 */
    navi.barrierHp = 100;

    assert(BattleActorApplyHit(&navi, 80, 0) == 0 && navi.barrierKind == 2);  /* Aura blocks < 100 */
    assert(BattleActorApplyHit(&navi, 100, 0) == 0 && navi.barrierKind == 0); /* Aura breaks at 100 */

    navi.barrierKind = 1; /* Barrier 50 */
    navi.barrierHp = 50;
    assert(BattleActorApplyHit(&navi, 40, (1u << 1)) == 40 && navi.hp == 460 && navi.barrierKind == 0); /* Wind strips & hits */

    assert(BattleActorApplyHit(&navi, 60, (1u << 4)) == 60 && navi.hp == 400 && navi.invFrames == 120u);
    assert(BattleActorApplyHit(&navi, 60, 0) == 0); /* flashing i-frames */
    BattleActorTickTimers(&navi);
    assert(navi.invFrames == 119u && navi.hitstunFrames == 15u);

    assert(BattleActorHeal(&navi, 150) == 100 && navi.hp == 500);
    navi.invFrames = 0;
    assert(BattleActorApplyHit(&navi, 999, 0) == 500 && navi.hp == 0 && navi.deleted == 1);
    assert(BattleActorHeal(&navi, 100) == 0);
}

int main(void) {
    test_progress_and_flags();
    test_input_and_memory();
    test_save_inventory_and_souls();
    test_battle_score_and_id_raise();
    test_world_tournaments_and_md();
    test_shop_and_navicust();
    test_battle_grid_and_damage();
    test_custom_screen_and_pa();
    test_emotion_and_karma();
    test_enemy_ai_and_sp_scaling();
    test_text_script_decoder();
    test_lz77_and_battle_loop();
    printf("modern-test: ALL PASSED\n");
    return 0;
}
