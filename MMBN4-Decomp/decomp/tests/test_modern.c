/* Host unit test suite for MMBN4 portable C modules (Phase E2).
 * Built and executed natively via `make modern-test`. */

#include <assert.h>
#include <stdio.h>
#include <string.h>


#include "modern_api.h"

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

static void test_chips_and_map(void) {
    u16 maxHp = 100, curHp = 100;
    struct GameState gs;
    struct MapWarpTarget warp = { 0x90, 0x02, 10, 20, 0, 1 };
    u16 steps = 0;

    /* Buster stats & DarkChip penalty */
    assert(BusterClampStat(0) == 1 && BusterClampStat(9) == 5);
    assert(BusterChargeDamage(3) == 30u);
    assert(BusterChargeFrames(1) == 90u && BusterChargeFrames(5) == 30u);
    assert(DarkChipApplyHpPenalty(&maxHp, &curHp) == 1 && maxHp == 99u && curHp == 99u);
    maxHp = 1;
    assert(DarkChipApplyHpPenalty(&maxHp, &curHp) == 0 && maxHp == 1u);

    /* DoubleSoul sacrifice eligibility */
    assert(ChipCanSacrificeForSoul(1, 0, 1) == 1u); /* Fire Standard for FireSoul */
    assert(ChipCanSacrificeForSoul(1, 1, 1) == 0u); /* Mega chip rejected */
    assert(ChipCanSacrificeForSoul(2, 0, 1) == 0u); /* Wrong element */

    /* Map key, cyberworld check, warp, encounter counter & SneakRun */
    assert(MapPackKey(0x90, 0x02) == 0x9002u);
    assert(MapIsCyberworld(0x01) == 0u && MapIsCyberworld(0x90) == 1u);
    memset(&gs, 0, sizeof(gs));
    MapApplyWarp(&gs, &warp);
    assert(gs.mapGroup == 0x90 && gs.mapNumber == 0x02);

    assert(MapStepEncounter(&steps, 40, 100, 0, 500, 200) == 0u && steps == 40u);
    assert(MapStepEncounter(&steps, 70, 100, 1, 500, 200) == 0u && steps == 0u); /* SneakRun suppresses */
    assert(MapStepEncounter(&steps, 100, 100, 1, 200, 500) == 1u); /* stronger enemy triggers */
}

static void test_oam_dma_and_folder(void) {
    struct OamAttr oam[4];
    struct DmaQueue dq;
    struct FolderChipMeta folder[30];
    u8 deck[30];
    u8 seen[30];
    u32 seed = 0x12345678u;
    u32 i;

    /* OAM & DMA queue */
    OamClearBuffer(oam, 4);
    assert(oam[0].attr0 == 0x0200u && oam[3].attr0 == 0x0200u);
    OamSetSprite(&oam[0], 120, 80, 1, 2, 0x40, 3, 1, 1, 0);
    assert((oam[0].attr0 & 0xFFu) == 80u && (oam[0].attr0 >> 14) == 1u);
    assert((oam[0].attr1 & 0x1FFu) == 120u && (oam[0].attr1 & (1u << 12)) != 0u);
    assert((oam[0].attr2 & 0x3FFu) == 0x40u && (oam[0].attr2 >> 12) == 3u);

    DmaQueueReset(&dq);
    assert(DmaQueuePush(&dq, oam, oam, 0x84000010u) == 1u && dq.count == 1u);

    /* Folder validation */
    for (i = 0; i < 30; i++) {
        folder[i].chipId = (u16)(1 + (i / 4)); /* max 4 copies each */
        folder[i].code = 0;
        folder[i].chipClass = 0;
    }
    folder[28].chipId = 200; folder[28].chipClass = 1; /* 1 Mega */
    folder[29].chipId = 250; folder[29].chipClass = 2; /* 1 Giga */
    assert(FolderValidate(folder, 30, 5, 1) == 0u);

    folder[27].chipId = 251; folder[27].chipClass = 2; /* 2nd Giga -> rejected */
    assert(FolderValidate(folder, 30, 5, 1) == 4u);
    folder[27].chipClass = 3; /* DarkChip -> banned */
    assert(FolderValidate(folder, 30, 5, 1) == 5u);

    assert(FolderCanSetRegular(12, 16) == 1u);
    assert(FolderCanSetRegular(20, 16) == 0u);

    /* Deck shuffle preserves Regular Chip at slot 0 & is a valid permutation */
    FolderShuffleDeck(deck, 17, &seed);
    assert(deck[0] == 17u);
    memset(seen, 0, sizeof(seen));
    for (i = 0; i < 30; i++) {
        assert(deck[i] < 30u);
        seen[deck[i]]++;
    }
    for (i = 0; i < 30; i++) {
        assert(seen[i] == 1u);
    }
}

static void test_library_and_status(void) {
    u8 stdBits[20];
    struct BattleStatusState st;

    /* Library bitfields & Violet Eclipse 12-Soul star */
    memset(stdBits, 0, sizeof(stdBits));
    assert(LibraryBitSet(stdBits, 0, 150) == 1u);
    assert(LibraryBitSet(stdBits, 0, 150) == 0u); /* duplicate */
    assert(LibraryBitSet(stdBits, 149, 150) == 1u);
    assert(LibraryBitSet(stdBits, 150, 150) == 0u); /* out of bounds */
    assert(LibraryBitTest(stdBits, 149, 150) == 1u);
    assert(LibraryCountRecorded(stdBits, 150) == 2u);

    assert(LibraryHasSoulStar(0x003Fu, 6) == 1u);  /* 6 Souls (vanilla) */
    assert(LibraryHasSoulStar(0x0FFEu, 12) == 0u); /* 11 Souls */
    assert(LibraryHasSoulStar(0x0FFFu, 12) == 1u); /* 12 Souls (Violet Eclipse) */
    assert(LibraryHasSoulStar(0x1FFEu, 12) == 0u); /* Duo Soul bit 12 doesn't replace base Soul */

    /* Status effects, StatusGuard & Freeze+Break / Bubble+Elec 2x shatter */
    memset(&st, 0, sizeof(st));
    st.statusGuard = 1;
    assert(StatusInflict(&st, 1, 60) == 0u); /* blocked by StatusGuard */
    st.statusGuard = 0;

    assert(StatusInflict(&st, 2, 2) == 1u); /* Freeze for 2 frames */
    assert(StatusIsImmobilized(&st) == 1u);
    assert(StatusApplyHitBonus(&st, 100, 1, 0) == 200 && st.activeStatus == 0u); /* Break shatters */

    assert(StatusInflict(&st, 3, 2) == 1u); /* Bubble for 2 frames */
    assert(StatusApplyHitBonus(&st, 80, 0, 3) == 160 && st.activeStatus == 0u); /* Elec pops */

    assert(StatusInflict(&st, 4, 2) == 1u); /* Confuse (not immobilized) */
    assert(StatusIsImmobilized(&st) == 0u);
    StatusTick(&st);
    assert(st.activeStatus == 4u && st.durationFrames == 1u);
    StatusTick(&st);
    assert(st.activeStatus == 0u);
}

static void test_subchips_and_battle_turn(void) {
    struct SubChipState sc;
    u16 hp = 120;
    struct BattleTurnState bt;

    /* SubChips */
    memset(&sc, 0, sizeof(sc));
    sc.maxPerSlot = 4;
    assert(SubChipAdd(&sc, 0, 6) == 4u && sc.qty[0] == 4u); /* MiniEnrg clamped to 4 */
    assert(SubChipUseRecovery(&sc, 0, &hp, 200) == 1u && hp == 170u && sc.qty[0] == 3u);
    assert(SubChipAdd(&sc, 1, 1) == 1u); /* FullEnrg */
    assert(SubChipUseRecovery(&sc, 1, &hp, 200) == 1u && hp == 200u && sc.qty[1] == 0u);
    assert(SubChipUseRecovery(&sc, 0, &hp, 200) == 0u); /* already at max HP */
    assert(SubChipAdd(&sc, 2, 1) == 1u); /* SneakRun */
    assert(SubChipActivateField(&sc, 2) == 1u && sc.sneakRunActive == 1u && sc.qty[2] == 0u);

    /* Battle Turn Coordinator & DoubleSoul 3-turn expiration */
    BattleTurnInit(&bt, 2, 0x2000u);
    assert(bt.phase == 0u);
    BattleTurnEnterCustom(&bt);
    assert(bt.phase == 1u && bt.turnNumber == 1u);
    BattleTurnStartCombat(&bt, 13); /* Duo Soul (3 turns) */
    assert(bt.phase == 2u && bt.activeSoul == 13u && bt.soulTurnsLeft == 3u);
    assert(BattleTurnStepCombat(&bt) == 0u && bt.totalCombatFrames == 1u);
    assert(BattleTurnStepCombat(&bt) == 1u && bt.customGauge == 0x4000u);

    /* Turn 2 -> 3 -> 4 reverts Duo Soul */
    BattleTurnEnterCustom(&bt); assert(bt.soulTurnsLeft == 2u && bt.activeSoul == 13u);
    BattleTurnStartCombat(&bt, 0);
    BattleTurnEnterCustom(&bt); assert(bt.soulTurnsLeft == 1u && bt.activeSoul == 13u);
    BattleTurnStartCombat(&bt, 0);
    BattleTurnEnterCustom(&bt); assert(bt.soulTurnsLeft == 0u && bt.activeSoul == 0u);

    /* Victory transition when enemiesAlive hits 0 */
    BattleTurnStartCombat(&bt, 0);
    bt.enemiesAlive = 0;
    assert(BattleTurnStepCombat(&bt) == 0u && bt.phase == 3u);
}

static void test_sound_and_trader(void) {
    struct SoundMgrState sm;
    struct GameState gs;
    const struct TraderPoolEntry pool[3] = {
        { 1, 0, 50 },
        { 29, 26, 30 },
        { 200, 1, 20 }
    };
    u32 seed = 0xCAFEBABEu;
    const struct TraderPoolEntry *pick;

    /* Sound manager: play, dedup, fanfare save/resume, fade */
    memset(&gs, 0, sizeof(gs));
    SoundMgrInit(&sm);
    assert(SoundMgrPlayBgm(&sm, &gs, 0x05) == 1u && gs.currentBgm == 0x05);
    assert(SoundMgrPlayBgm(&sm, &gs, 0x05) == 0u); /* already playing */
    SoundMgrPlayFanfare(&sm, 0x20);
    assert(sm.activeBgm == 0x20 && sm.fanfareActive == 1u);
    assert(SoundMgrResumeBgm(&sm, &gs) == 0x05 && gs.currentBgm == 0x05);
    sm.fadeFrames = 2;
    assert(SoundMgrTickFade(&sm) == 0u);
    assert(SoundMgrTickFade(&sm) == 1u && sm.activeBgm == 0xFFu);

    /* Chip Trader */
    assert(TraderRequiredCost(0) == 3u && TraderRequiredCost(1) == 10u && TraderRequiredCost(2) == 10u);
    assert(TraderCanSpin(0, 2) == 0u && TraderCanSpin(0, 3) == 1u);
    assert(TraderCanSpin(2, 10) == 1u);
    pick = TraderRoll(pool, 3, &seed);
    assert(pick != 0 && (pick->chipId == 1 || pick->chipId == 29 || pick->chipId == 200));
}

static void test_mail_stats_and_projectiles(void) {
    struct MailState ms;
    struct NaviBuildBonuses b;
    struct BattleProjectile proj;

    /* PET Mail */
    memset(&ms, 0, sizeof(ms));
    assert(MailDeliver(&ms, 3) == 1u && MailDeliver(&ms, 3) == 0u);
    assert(MailDeliver(&ms, 7) == 1u && MailCountUnread(&ms) == 2u);
    assert(MailMarkRead(&ms, 3) == 1u && MailCountUnread(&ms) == 1u);
    assert(MailClaimAttachment(&ms, 3) == 1u && MailClaimAttachment(&ms, 3) == 0u);

    /* MegaMan Stat Aggregation */
    memset(&b, 0, sizeof(b));
    b.hpMemoryCount = 45;     /* +900 -> 1000 */
    b.naviCustHpBonus = 500;  /* 1500 */
    b.darkChipPenalty = 10;   /* 1490 */
    b.customPlus = 2;         /* 7 */
    b.megaFolderPlus = 2;     /* 7 */
    b.gigaFolderPlus = 1;     /* 2 */
    assert(NaviCalcMaxHp(&b) == 1490u);
    assert(NaviCalcCustomHandSize(&b) == 7u);
    assert(NaviCalcMegaLimit(&b) == 7u);
    assert(NaviCalcGigaLimit(&b) == 2u);

    /* Battle Projectiles */
    ProjectileSpawn(&proj, 0, 2, 2, 1, 60, 2, 0);
    assert(ProjectileStep(&proj) == 0u && proj.x == 2); /* timer 2->1 */
    assert(ProjectileStep(&proj) == 1u && proj.x == 3); /* moves to x=3 */
    assert(ProjectileCheckHit(&proj, 0, 3, 2) == 0u);   /* friendly fire ignored */
    assert(ProjectileCheckHit(&proj, 1, 3, 2) == 60u && proj.active == 0u);

    /* Piercing wave exits right edge (x=6 -> x=7 deactivates) */
    ProjectileSpawn(&proj, 0, 5, 1, 1, 80, 1, 1);
    assert(ProjectileStep(&proj) == 1u && proj.x == 6);
    assert(ProjectileCheckHit(&proj, 1, 6, 1) == 80u && proj.active == 1u);
    assert(ProjectileStep(&proj) == 0u && proj.active == 0u);
}

static void test_camera_rewards_and_virus_ai(void) {
    struct CameraState cam;
    struct SpPlusReward rew;
    struct VirusAiState met;
    u32 seed = 0x13572468u;
    s8 rx = 0, ry = 0;

    /* Camera clamping & screen shake */
    memset(&cam, 0, sizeof(cam));
    cam.mapWidth = 480;
    cam.mapHeight = 320;
    CameraCenterOn(&cam, 50, 40);
    assert(cam.x == 0 && cam.y == 0);
    CameraCenterOn(&cam, 450, 300);
    assert(cam.x == 240 && cam.y == 160);
    CameraStartShake(&cam, 4, 2);
    assert(CameraTickShakeOffset(&cam) == -4);
    assert(CameraTickShakeOffset(&cam) == 4);
    assert(CameraTickShakeOffset(&cam) == 0);

    /* Battle rewards & Violet Eclipse 1.5x Zenny / SP+ roll */
    assert(BattleRewardTierFromBusting(3) == 0u);
    assert(BattleRewardTierFromBusting(6) == 1u);
    assert(BattleRewardTierFromBusting(8) == 2u);
    assert(BattleRewardTierFromBusting(10) == 3u);
    assert(BattleRewardTierFromBusting(11) == 4u);
    assert(BattleScaleZenny(1000u, 0) == 1000u);
    assert(BattleScaleZenny(1000u, 1) == 1500u);
    rew = BattleRollSpPlusReward(&seed);
    assert((rew.zenny == 15000u && rew.bugFrags == 0u)
        || (rew.zenny == 0u && rew.bugFrags == 50u));

    /* Virus AI row tracking, Mettaur guard cycle & enemy-side hop */
    memset(&met, 0, sizeof(met));
    met.x = 5; met.y = 1; met.attackCooldown = 1;
    assert(VirusIsAlignedRow(&met, 3) == 0u);
    assert(VirusTrackRow(&met, 3) == 1u && met.y == 2);
    assert(VirusTrackRow(&met, 3) == 1u && met.y == 3);
    assert(VirusIsAlignedRow(&met, 3) == 1u);
    assert(VirusTickMettaurCycle(&met, 60) == 0u && met.guarding == 1u);
    assert(VirusTickMettaurCycle(&met, 60) == 1u && met.guarding == 0u && met.attackCooldown == 60u);
    VirusPickRandomEnemyPanel(&seed, &rx, &ry);
    assert(rx >= 4 && rx <= 6 && ry >= 1 && ry <= 3);
}

static void test_boss_patterns_and_fade(void) {
    struct BossPatternState boss;
    u16 palSrc[2];
    u16 palDst[2];

    /* Boss phase-2 enrage, Bass Aura regen, counter-hit window */
    memset(&boss, 0, sizeof(boss));
    boss.hp = 1200; boss.maxHp = 2000;
    boss.auraRegenTimer = 2; boss.auraRegenPeriod = 180;
    boss.counterWindow = 8;
    assert(BossUpdatePhase(&boss) == 0u);
    boss.hp = 1000;
    assert(BossUpdatePhase(&boss) == 1u);
    assert(BossTickAuraRegen(&boss, 1) == 0u); /* Aura still active */
    assert(BossTickAuraRegen(&boss, 0) == 0u && boss.auraRegenTimer == 1u);
    assert(BossTickAuraRegen(&boss, 0) == 1u && boss.auraRegenTimer == 180u);
    assert(BossCheckCounterHit(&boss) == 1u && boss.counterWindow == 0u);
    assert(BossCheckCounterHit(&boss) == 0u);

    /* RGB555 palette fade */
    palSrc[0] = Rgb555Pack(31, 0, 0);
    palSrc[1] = Rgb555Pack(0, 16, 31);
    assert(Rgb555FadeColor(palSrc[0], 0, 0) == palSrc[0]);
    assert(Rgb555FadeColor(palSrc[0], 16, 0) == 0u);
    assert(Rgb555FadeColor(palSrc[0], 16, 1) == 0x7FFFu);
    Rgb555FadePalette(palSrc, palDst, 2, 8, 0);
    assert(palDst[0] == Rgb555Pack(15, 0, 0) && palDst[1] == Rgb555Pack(0, 8, 15));
}

static void test_fixed_math(void) {
    s8 dx = 0, dy = 0;

    assert(Fx16Mul(1 << 16, 3 << 16) == (3 << 16));
    assert(Fx16Mul(3 << 15, 2 << 16) == (3 << 16)); /* 1.5 * 2.0 = 3.0 */
    assert(Fx16Lerp(10 << 16, 20 << 16, 1 << 15) == (15 << 16));
    assert(MathAbsS32(-42) == 42 && MathAbsS32(42) == 42);
    assert(MathManhattan(1, 1, 6, 3) == 7);
    MathFacingToDelta(1, &dx, &dy); /* NE */
    assert(dx == 1 && dy == -1);
    MathFacingToDelta(5, &dx, &dy); /* SW */
    assert(dx == -1 && dy == 1);
}

static void test_chip_effects_souls_and_npc(void) {
    struct BattlePanel grid[18];

    /* AreaGrab, Geddon1, Sanctuary */
    BattleGridInitDefault(grid);
    assert(ChipApplyAreaGrab(grid, 0) == 3u); /* steals col 4 */
    assert(grid[BattleGridIndex(4, 2)].owner == 0u);
    assert(ChipApplyAreaGrab(grid, 0) == 3u); /* steals col 5 */
    assert(ChipApplyAreaGrab(grid, 0) == 0u); /* col 6 preserved for enemy */
    assert(ChipApplySanctuary(grid, 0) == 15u);
    assert(grid[BattleGridIndex(1, 1)].state == 7u);
    ChipApplyGeddon1(grid);
    assert(grid[BattleGridIndex(1, 1)].state == 1u); /* cracked */

    /* DoubleSoul & Violet Eclipse Duo Soul passives */
    assert((SoulGetPassiveFlags(13) & ((1u << 0) | (1u << 1) | (1u << 3)))
        == ((1u << 0) | (1u << 1) | (1u << 3))); /* Duo: Air+Float+Reflect */
    assert(SoulGetNullChipBonus(SOUL_GUTS) == 30u);
    assert(SoulGetNullChipBonus(SOUL_NUMBER) == 10u);

    /* Overworld AABB & NPC talk-ray */
    assert(OverworldAabbOverlap(10, 10, 8, 8, 20, 10, 8, 8) == 1u);
    assert(OverworldAabbOverlap(10, 10, 4, 4, 30, 10, 4, 4) == 0u);
    assert(OverworldCanTalkToNpc(100, 100, 2, 114, 100, 12, 8) == 1u); /* facing East toward NPC */
    assert(OverworldCanTalkToNpc(100, 100, 6, 114, 100, 12, 8) == 0u); /* facing West away from NPC */
}

static void test_chip_combo_and_net_security(void) {
    struct Toolkit tk;

    /* Chip combo damage pipeline: (100 + 30) * 2 (FullSynchro) * 2 (Fire vs Wood) * 2 (Grass) = 1040 */
    assert(ChipApplyFlatModifiers(100, 30, 3) == 160u);
    assert(ChipCalcFinalDamage(100, 30, EMOTION_FULL_SYNCHRO, 1, 4, 4) == 1040u);
    assert(ChipCalcFinalDamage(100, 0, EMOTION_NORMAL, 0, 0, 7) == 50u); /* Holy halves */

    /* Black Earth gate, security cube & Violet Eclipse Frequency Tuner */
    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x040);

    assert(NetCanOpenBlackEarthGate(&tk, 0x001Fu, 6, 0) == 0u); /* only 5 Souls */
    assert(NetCanOpenBlackEarthGate(&tk, 0x003Fu, 6, 0) == 1u); /* 6 Souls OK */
    assert(NetCanUnlockSecurityCube(&tk, 0x0200, 12) == 0u);
    AddItemClamped(&tk, 12, 1, 1);
    assert(NetCanUnlockSecurityCube(&tk, 0x0200, 12) == 1u);

    tk.progress->playthrough = 0;
    assert(NetResolveTunerTier(&tk, 0) == 0u); /* Auto -> 1st run */
    assert(NetResolveTunerTier(&tk, 3) == 2u); /* Forced SuperHard -> tier 2 */
}

static void test_link_battle_and_pet_upgrades(void) {
    u8 p1w = 0, p2w = 0;
    u8 regMem = 48;
    u8 subCap = 7;

    /* NetBattle tiebreak & best-of-3 */
    assert(NetBattleResolveTiebreak(300, 1000, 400, 1000) == 2u);
    assert(NetBattleResolveTiebreak(500, 500, 800, 1000) == 1u);
    assert(NetBattleResolveTiebreak(500, 1000, 250, 500) == 0u);
    assert(NetBattleIsTurnLimitReached(14) == 0u && NetBattleIsTurnLimitReached(15) == 1u);
    assert(NetBattleRecordRoundWin(&p1w, &p2w, 1, 2) == 0u);
    assert(NetBattleRecordRoundWin(&p1w, &p2w, 1, 2) == 1u);

    /* PET RegUP, SubMem & SpinParts */
    assert(PetApplyRegUp(&regMem, 3) == 50u && regMem == 50u);
    assert(PetApplySubMem(&subCap) == 1u && subCap == 8u);
    assert(PetApplySubMem(&subCap) == 0u);
    assert(PetCanRotateColor(0x05u, 0) == 1u);
    assert(PetCanRotateColor(0x05u, 1) == 0u);
    assert(PetCanRotateColor(0x05u, 2) == 1u);
}

static void test_friendly_matches_and_navicust_bugs(void) {
    struct Toolkit tk;
    u8 pids[25];
    u8 cols[25];
    struct NaviCustPartDesc prog = { 1, 1, 0 };  /* textured program */
    struct NaviCustPartDesc solid = { 2, 2, 1 }; /* solid stat part */

    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x040);

    /* Friendly matches: round-gated, retry-on-loss, win grants Soul & hides NPC */
    assert(FriendlyMatchIsAvailable(&tk, 0, 1, 0x0310) == 0u);
    assert(FriendlyMatchIsAvailable(&tk, 1, 1, 0x0310) == 1u);
    assert(FriendlyMatchResolve(&tk, 0, 0x0310, 0x0320) == 0u); /* loss -> retry */
    assert(FriendlyMatchIsAvailable(&tk, 1, 1, 0x0310) == 1u);
    assert(FriendlyMatchResolve(&tk, 1, 0x0310, 0x0320) == 1u); /* win -> grants Soul */
    assert(FriendlyMatchIsAvailable(&tk, 1, 1, 0x0310) == 0u);
    assert(EventFlagTest(&tk, 0x0320) == 1);

    /* Lobby vs Duo rematch ladders */
    assert(RematchLadderOnWin(0, 8, 0) == 1u);  /* Lobby advances on any win */
    assert(RematchLadderOnWin(0, 10, 1) == 0u); /* Duo needs S-rank (11) */
    assert(RematchLadderOnWin(0, 11, 1) == 1u);

    /* NaviCust bug detection */
    memset(pids, 0, sizeof(pids));
    memset(cols, 0, sizeof(cols));
    pids[12] = 1; cols[12] = 1; /* center (row 2) */
    assert(NaviCustEvalPartBugs(&prog, 1, pids, cols) == 0u);  /* OK on command line */
    assert(NaviCustEvalPartBugs(&prog, 0, pids, cols) == (1u << 1)); /* off cmd line bug */
    assert(NaviCustEvalPartBugs(&solid, 1, pids, cols) == (1u << 2)); /* solid on cmd line bug */

    /* Same-color adjacent parts clash */
    pids[13] = 2; cols[13] = 1;
    assert(NaviCustHasColorClash(pids, cols) == 1u);

    /* 5 distinct colors -> too many colors bug */
    cols[0] = 1; cols[1] = 2; cols[2] = 3; cols[3] = 4; cols[4] = 5;
    assert(NaviCustCountDistinctColors(cols) == 5u);
    assert((NaviCustEvalPartBugs(&prog, 1, pids, cols) & (1u << 3)) != 0u);
}

static void test_anim_bracket_and_counter(void) {
    struct SpriteAnimState anim;
    const u16 durs[3] = { 2, 1, 2 };
    struct TournamentBracketSlot br[8];
    struct BossPatternState boss;
    struct BattleStatusState bst;
    enum EmotionState emo = EMOTION_NORMAL;

    /* SpriteAnim */
    SpriteAnimInit(&anim, 3, durs[0], 0);
    assert(SpriteAnimStep(&anim, durs) == 0u && anim.frameIndex == 0u);
    assert(SpriteAnimStep(&anim, durs) == 1u && anim.frameIndex == 1u);
    assert(SpriteAnimStep(&anim, durs) == 1u && anim.frameIndex == 2u);
    assert(SpriteAnimStep(&anim, durs) == 0u && anim.finished == 0u);
    assert(SpriteAnimStep(&anim, durs) == 0u && anim.finished == 1u);

    /* Tournament bracket */
    memset(br, 0, sizeof(br));
    br[2].entrantId = 0x1E;
    assert(BracketQfOpponentSlot(2) == 3u);
    assert(BracketGetChampion(br) == 0xFFu);
    BracketMarkRoundWin(br, 2, 0);
    BracketMarkRoundWin(br, 2, 1);
    BracketMarkRoundWin(br, 2, 2);
    assert(BracketHasWonRound(br, 2, 2) == 1u);
    assert(BracketGetChampion(br) == 0x1Eu);

    /* DarkChip offer & Counter-Hit resolution */
    assert(DarkChipShouldOffer(600, 1000, EMOTION_NORMAL, 0) == 0u);
    assert(DarkChipShouldOffer(500, 1000, EMOTION_NORMAL, 0) == 1u);
    assert(DarkChipShouldOffer(500, 1000, EMOTION_NORMAL, 1) == 0u); /* banned */
    assert(BattleCalcHpPercent(250, 1000) == 25u && BattleIsLowHp(250, 1000) == 1u);

    memset(&boss, 0, sizeof(boss));
    memset(&bst, 0, sizeof(bst));
    boss.counterWindow = 5;
    assert(BattleResolveCounterHit(&boss, &bst, &emo, 512) == 1u);
    assert(bst.activeStatus == 1u && bst.durationFrames == 90u);
    assert(emo == EMOTION_FULL_SYNCHRO && BattleIsDoubleDamageEmotion(emo) == 1u);
    assert(BattleShouldTriggerDarkSoul(100, 1) == 1u);
}

static void test_obstacle_save_slots_and_cutscene_vm(void) {
    struct BattleObstacle cube;
    u8 hrs = 0, mins = 0, secs = 0;
    struct ProgressState prog;
    struct CutsceneVmState vm;
    /* Wait 2 frames, then Halt (0x00) */
    const u8 script[] = { 0x02, 0x02, 0x00, 0x00 };

    /* RockCube obstacle damage & AirShot 200-damage push collision */
    ObstacleSpawn(&cube, 3, 2, 200, 200);
    assert(ObstacleTakeDamage(&cube, 50) == 0u && cube.hp == 150);
    assert(ObstaclePushSlide(&cube, 1, 5, 2) == 200u && cube.active == 0u);

    ObstacleSpawn(&cube, 2, 1, 100, 200);
    assert(ObstacleTakeDamage(&cube, 100) == 1u && cube.active == 0u);

    /* Save playtime formatting, integrity check & title stars */
    SaveFormatPlaytime((3661u) * 60u, &hrs, &mins, &secs);
    assert(hrs == 1u && mins == 1u && secs == 1u);
    SaveFormatPlaytime(0x0FFFFFFFu, &hrs, &mins, &secs);
    assert(hrs == 99u && mins == 59u && secs == 59u);

    memset(&prog, 0, sizeof(prog));
    memset(gTestEwramImage, 0, sizeof(gTestEwramImage));
    WriteSaveMagic(&prog);
    prog.checksum = ComputeSaveChecksum(gTestEwramImage, &prog, 0x16u);
    assert(SaveValidateIntegrity(gTestEwramImage, &prog, 0x16u) == 1u);
    prog.checksum ^= 1u;
    assert(SaveValidateIntegrity(gTestEwramImage, &prog, 0x16u) == 0u);
    assert(SaveCountTitleStars(0x3Fu) == 6u);

    /* Cutscene VM */
    CutsceneVmInit(&vm, script, sizeof(script));
    assert(CutsceneVmStep(&vm) == 0u && CutsceneVmGetWaitFrames(&vm) == 2u);
    assert(CutsceneVmStep(&vm) == 0u && CutsceneVmGetWaitFrames(&vm) == 1u);
    assert(CutsceneVmStep(&vm) == 0u && CutsceneVmGetWaitFrames(&vm) == 0u);
    assert(CutsceneVmStep(&vm) == 1u && CutsceneVmIsHalted(&vm) == 1u);
    CutsceneVmReset(&vm);
    assert(CutsceneVmIsHalted(&vm) == 0u);
    CutsceneVmHalt(&vm);
    assert(CutsceneVmIsHalted(&vm) == 1u);
}

static void test_chip_attacks_and_duo_bass(void) {
    const s8 tx[3] = { 6, 4, 5 };
    const s8 ty[3] = { 2, 2, 1 };
    const u8 alive[3] = { 1, 1, 1 };
    struct BattlePanel grid[18];

    /* Concrete chip hitboxes: Cannon, WideSword, LongSword, MiniBomb */
    assert(ChipCannonFindTarget(2, 2, tx, ty, alive, 3) == 1); /* hits closest x=4 before x=6 */
    assert(ChipCannonFindTarget(2, 3, tx, ty, alive, 3) == -1);
    assert(ChipWideSwordHits(3, 2, 4, 1) == 1u && ChipWideSwordHits(3, 2, 4, 3) == 1u);
    assert(ChipWideSwordHits(3, 2, 5, 2) == 0u);
    assert(ChipLongSwordHits(3, 2, 4, 2) == 1u && ChipLongSwordHits(3, 2, 5, 2) == 1u);
    assert(ChipLongSwordHits(3, 2, 4, 1) == 0u);
    assert(ChipLobBombHits(2, 2, 3, 5, 2) == 1u);
    assert(ChipLobBombHits(4, 2, 3, 7, 2) == 0u); /* out of bounds */

    /* Duo & Bass attack patterns */
    BattleGridInitDefault(grid);
    assert(BossDuoMeteorImpact(grid, 2, 2, 2, 2) == 1u);
    assert(grid[BattleGridIndex(2, 2)].state == 1u); /* panel cracked */
    assert(BossDuoGigaLaserHits(2, 3, 2) == 1u && BossDuoGigaLaserHits(2, 3, 1) == 0u);
    assert(BossBassDarkArmBladeHits(4, 2, 1) == 1u && BossBassDarkArmBladeHits(4, 1, 1) == 0u);
    assert(BossBassAuraBreaks(180, 200, 0) == 0u);
    assert(BossBassAuraBreaks(200, 200, 0) == 1u);
    assert(BossBassAuraBreaks(10, 200, 1) == 1u); /* Wind strips Aura */
}

static void test_map_collision_and_pet_menu(void) {
    /* 3x3 overworld tile map: 0=walk, 1=wall, 2=warp */
    const u8 tiles[9] = {
        0, 1, 2,
        0, 0, 0,
        1, 0, 0
    };
    struct MapCollisionGrid map = { tiles, 3, 3 };
    s32 px = 0, py = 1;
    struct MenuCursorState mc;

    /* Map collision & step */
    assert(MapGridIsWalkable(&map, 1, 0) == 0u);
    assert(MapGridIsWarpTile(&map, 2, 0) == 1u);
    assert(MapGridTryStep(&map, &px, &py, 2) == 1u && px == 1 && py == 1); /* East */
    assert(MapGridTryStep(&map, &px, &py, 0) == 0u && px == 1 && py == 1); /* North blocked by wall */
    assert(MapGridTryStep(&map, &px, &py, 2) == 1u && px == 2 && py == 1); /* East */
    assert(MapGridTryStep(&map, &px, &py, 0) == 1u && px == 2 && py == 0); /* North -> warp tile */
    assert(MapGridIsWarpTile(&map, px, py) == 1u);

    /* PET scrolling list cursor */
    MenuCursorInit(&mc, 12, 5);
    MenuCursorMove(&mc, -1); /* wrap to bottom */
    assert(mc.cursor == 11u && mc.scrollTop == 7u && MenuCursorVisibleRow(&mc) == 4u);
    MenuCursorMove(&mc, 1); /* wrap to top */
    assert(mc.cursor == 0u && mc.scrollTop == 0u);
    MenuCursorPage(&mc, 1); /* R page down */
    assert(mc.cursor == 5u && mc.scrollTop == 1u && MenuCursorVisibleRow(&mc) == 4u);
}

static void test_soul_navi_ai_and_npc_vm(void) {
    struct BattlePanel grid[18];
    s8 px = 0, py = 0;
    struct Toolkit tk;
    struct NpcVmState npc;
    /* SetSprite(0x10), SetPos(0x38, 0x28, 0x40), SetFacing(1), SetTalk(3), SetFlag(0x0210), End */
    const u8 npcScript[] = {
        0x17, 0x10,
        0x14, 0x38, 0x00, 0x28, 0x00, 0x40, 0x00,
        0x16, 0x01,
        0x18, 0x03,
        0x06, 0x10, 0x02,
        0x00
    };

    /* Soul Navi AI helpers */
    BattleGridInitDefault(grid);
    assert(BossGutsPunchCrack(grid, 4, 2) == 2u);
    assert(grid[BattleGridIndex(3, 2)].state == 1u && grid[BattleGridIndex(2, 2)].state == 1u);
    assert(BossProtoStepTarget(2, 2, &px, &py) == 1u && px == 3 && py == 2);
    assert(BossProtoStepTarget(6, 2, &px, &py) == 0u);
    assert(BossCrossExplosionHits(2, 2, 2, 1) == 1u && BossCrossExplosionHits(2, 2, 3, 3) == 0u);
    assert(BossSearchScopeLocks(2, 3, 2, 3) == 1u);

    /* NpcRunScript VM */
    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x040);

    NpcVmInit(&npc, npcScript, sizeof(npcScript));
    assert(NpcVmRun(&npc, &tk, 32) == 6u);
    assert(npc.halted == 1u && npc.spriteId == 0x10u);
    assert(npc.x == 0x38 && npc.y == 0x28 && npc.z == 0x40);
    assert(npc.facing == 1u && npc.talkIdx == 3u);
    assert(EventFlagTest(&tk, 0x0210) == 1);
}

static void test_map_vm_pa_catalog_and_battle_sim(void) {
    struct Toolkit tk;
    struct MapScriptVmState mvm;
    /* SetFlag(0x0155), ClearFlag(0x0156), End */
    const u8 mscript[] = { 0x1C, 0xFF, 0x55, 0x01, 0x1D, 0xFF, 0x56, 0x01, 0x00 };
    const struct CustomChipSlot ls[3] = { { 35, 18, 1 }, { 36, 18, 1 }, { 37, 18, 1 } };
    u32 paStart = 99;
    s8 ox = 5, oy = 1;
    u8 chips = 3;
    u32 seed = 0x55AA1234u;
    struct BattleSimState sim;

    /* MapScript VM */
    memset(&tk, 0, sizeof(tk));
    memset(gTestSaveBlock, 0, sizeof(gTestSaveBlock));
    RebuildSavePointers(&tk, gTestSaveBlock, 0x040);
    EventFlagSet(&tk, 0x0156);
    MapScriptVmInit(&mvm, mscript, sizeof(mscript));
    assert(MapScriptVmRun(&mvm, &tk, 16) == 3u && mvm.flagsTouched == 2u);
    assert(EventFlagTest(&tk, 0x0155) == 1 && EventFlagTest(&tk, 0x0156) == 0);

    /* Built-in P.A. Catalog (LifeSword) */
    assert(PaCatalogMatch(ls, 3, &paStart) == 501u && paStart == 0u);

    /* Extended Soul Navi AI */
    assert(BossWindApplyGust(3, 1) == 3 && BossWindApplyGust(2, -1) == 1);
    BossThunderOrbStep(&ox, &oy, 2, 2);
    assert(ox == 4 && oy == 1);
    assert(BossRollDestroyChip(&chips) == 1u && chips == 2u);
    assert(BossNumberRollDiceDamage(&seed, 30) >= 30u);
    assert(BossAquaHoseHits(3, 2, 2, 2) == 1u && BossAquaHoseHits(3, 2, 1, 2) == 0u);

    /* End-to-end 1v1 headless battle simulation */
    BattleSimInit(&sim, 500, 300, 0x2000u);
    BattleTurnEnterCustom(&sim.turn);
    BattleTurnStartCombat(&sim.turn, 0);
    assert(BattleSimTick(&sim) == 0u);
    assert(BattleSimPlayerAttack(&sim, 300, 0) == 300 && sim.enemy.deleted == 1u);
    assert(BattleSimTick(&sim) == 0u && sim.turn.phase == 3u); /* VICTORY */
}

static void test_chip_attacks2(void) {
    struct BattlePanel grid[18];
    s8 wx = 1, bx = 1, by = 3;
    s16 barrier = 100;
    u16 invis = 2;

    /* AreaGrab steals col 4, then col 5, preserves col 6 unless allowLastColumn=1 */
    BattleGridInitDefault(grid);
    assert(ChipAreaGrabStealColumn(grid, 0) == 3u);
    assert(ChipAreaGrabStealColumn(grid, 0) == 3u);
    assert(ChipAreaGrabStealColumn(grid, 0) == 0u);
    assert(ChipAreaGrabStealColumn(grid, 1) == 3u);

    /* ShockWave stops on broken panel */
    BattleGridInitDefault(grid);
    grid[BattleGridIndex(3, 2)].state = 2u;
    assert(ChipShockWaveStep(grid, &wx, 2, 1) == 1u && wx == 2);
    assert(ChipShockWaveStep(grid, &wx, 2, 1) == 0u && wx == 2);

    /* Boomerang perimeter sweep: (1,3)->(6,3)->(6,1)->(1,1) = 12 steps then 0 */
    {
        u32 steps = 0u;
        while (ChipBoomerangPerimeterStep(&bx, &by)) {
            steps++;
        }
        assert(steps == 12u && bx == 1 && by == 1);
    }

    /* Barrier absorb + overflow + wind shatter */
    assert(ChipBarrierAbsorbHit(&barrier, 40, 0) == 0 && barrier == 60);
    assert(ChipBarrierAbsorbHit(&barrier, 80, 0) == 20 && barrier == 0);
    barrier = 200;
    assert(ChipBarrierAbsorbHit(&barrier, 10, 1) == 0 && barrier == 0); /* Wind shatters */

    /* Invisible timer */
    assert(ChipInvisTick(&invis) == 1u && invis == 1u);
    assert(ChipInvisTick(&invis) == 0u && invis == 0u);
}

static void test_panel_hazards(void) {
    struct BattlePanel grid[18];
    s8 px = 1, py = 2;
    s16 hp = 120;
    u16 timer = 0;
    u32 i;

    BattleGridInitDefault(grid);

    /* Cracked -> Broken on step-off */
    grid[BattleGridIndex(1, 1)].state = 1u;
    assert(PanelStepOffCrack(grid, 1, 1) == 1u);
    assert(grid[BattleGridIndex(1, 1)].state == 2u);
    assert(PanelStepOffCrack(grid, 1, 1) == 0u);

    /* Ice slide across (1,2) -> (2,2) -> (3,2) stopping at Normal (3,2) */
    grid[BattleGridIndex(1, 2)].state = 3u;
    grid[BattleGridIndex(2, 2)].state = 3u;
    assert(PanelIceSlideStep(grid, &px, &py, 1, 0, 0, 0) == 2u);
    assert(px == 3 && py == 2);
    /* FloatShoes prevents sliding */
    px = 1;
    assert(PanelIceSlideStep(grid, &px, &py, 1, 0, 0, 1) == 0u && px == 1);

    /* Lava deals 50 unless Fire (1) or FloatShoes */
    grid[BattleGridIndex(2, 3)].state = 5u;
    assert(PanelApplyLavaDamage(grid, 2, 3, &hp, 0, 0) == 50 && hp == 70);
    assert(PanelApplyLavaDamage(grid, 2, 3, &hp, 1, 0) == 0 && hp == 70);
    assert(PanelApplyLavaDamage(grid, 2, 3, &hp, 0, 1) == 0 && hp == 70);

    /* Poison drains 1 HP every 10 frames */
    grid[BattleGridIndex(3, 3)].state = 6u;
    for (i = 0; i < 9; i++) {
        assert(PanelTickPoisonDamage(grid, 3, 3, &hp, &timer, 0) == 0u);
    }
    assert(PanelTickPoisonDamage(grid, 3, 3, &hp, &timer, 0) == 1u && hp == 69);

    /* Grass heals Wood-element (4) 1 HP every 10 frames */
    grid[BattleGridIndex(1, 3)].state = 4u;
    timer = 0;
    for (i = 0; i < 9; i++) {
        assert(PanelTickGrassHeal(grid, 1, 3, &hp, 100, &timer, 4) == 0u);
    }
    assert(PanelTickGrassHeal(grid, 1, 3, &hp, 100, &timer, 4) == 1u && hp == 70);
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
    test_chips_and_map();
    test_oam_dma_and_folder();
    test_library_and_status();
    test_subchips_and_battle_turn();
    test_sound_and_trader();
    test_mail_stats_and_projectiles();
    test_camera_rewards_and_virus_ai();
    test_boss_patterns_and_fade();
    test_fixed_math();
    test_chip_effects_souls_and_npc();
    test_chip_combo_and_net_security();
    test_link_battle_and_pet_upgrades();
    test_friendly_matches_and_navicust_bugs();
    test_anim_bracket_and_counter();
    test_obstacle_save_slots_and_cutscene_vm();
    test_chip_attacks_and_duo_bass();
    test_map_collision_and_pet_menu();
    test_soul_navi_ai_and_npc_vm();
    test_map_vm_pa_catalog_and_battle_sim();
    test_chip_attacks2();
    test_panel_hazards();
    printf("modern-test: ALL PASSED\n");
    return 0;
}
