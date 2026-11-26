/*
 * MMBN4 Portable C Engine Public API (Phase E2 — libmmbn4_modern.a).
 *
 * Single self-contained header declaring the public structs, enums, and
 * 152 portable C functions compiled into build/modern/libmmbn4_modern.a
 * (35 modules under src/modern/).
 *
 * Designed for native host / PC / Wii U / New 3DS / future port consumers.
 */
#ifndef MMBN4_MODERN_API_H
#define MMBN4_MODERN_API_H

#include "battle.h"
#include "gba_io.h"
#include "toolkit.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- core_progress.c --- */
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

/* --- core_input.c --- */
void Joypad_Poll(struct JoypadState *jp, u16 keyInputReg);
int CheckSoftReset(struct Toolkit *tk, int canReset);
void ZeroFillByWord(void *dst, u32 byteLen);
void CopyBytes(const void *src, void *dst, u32 byteLen);
void CopyWords(const void *src, void *dst, u32 byteLen);

/* --- core_lz77.c --- */
u32 Lz77GetUncompressedSize(const u8 *src, u32 srcLen);
u32 Lz77Decompress(const u8 *src, u32 srcLen, u8 *dst, u32 dstCap);

/* --- core_oam_dma.c --- */
struct OamAttr {
    u16 attr0;
    u16 attr1;
    u16 attr2;
    s16 affine;
};

struct DmaQueueEntry {
    const void *src;
    void *dst;
    u32 control;
};

struct DmaQueue {
    struct DmaQueueEntry entries[32];
    u32 count;
};

void OamClearBuffer(struct OamAttr *oam, u32 count);
void OamSetSprite(struct OamAttr *entry, s16 x, s16 y, u8 shape, u8 size,
                  u16 tile, u8 pal, u8 prio, u8 hFlip, u8 vFlip);
void DmaQueueReset(struct DmaQueue *q);
u32 DmaQueuePush(struct DmaQueue *q, const void *src, void *dst, u32 control);

/* --- core_sound_mgr.c --- */
struct SoundMgrState {
    u8 activeBgm;
    u8 savedBgm;
    u8 muted;
    u8 fadeFrames;
    u8 fanfareActive;
};

void SoundMgrInit(struct SoundMgrState *sm);
u32 SoundMgrPlayBgm(struct SoundMgrState *sm, struct GameState *gs, u8 trackId);
void SoundMgrPlayFanfare(struct SoundMgrState *sm, u8 fanfareId);
u8 SoundMgrResumeBgm(struct SoundMgrState *sm, struct GameState *gs);
u32 SoundMgrTickFade(struct SoundMgrState *sm);

/* --- core_camera.c --- */
struct CameraState {
    s32 x;
    s32 y;
    s32 mapWidth;
    s32 mapHeight;
    u16 shakeFrames;
    s8 shakeAmp;
};

void CameraCenterOn(struct CameraState *cam, s32 targetX, s32 targetY);
void CameraStartShake(struct CameraState *cam, s8 amp, u16 frames);
s32 CameraTickShakeOffset(struct CameraState *cam);

/* --- core_fade.c --- */
u16 Rgb555Pack(u8 r, u8 g, u8 b);
u16 Rgb555FadeColor(u16 color, u8 coeff, u8 toWhite);
void Rgb555FadePalette(const u16 *src, u16 *dst, u32 count, u8 coeff, u8 toWhite);

/* --- core_fixed_math.c --- */
s32 Fx16Mul(s32 a, s32 b);
s32 Fx16Lerp(s32 a, s32 b, s32 t);
s32 MathAbsS32(s32 v);
s32 MathManhattan(s32 x0, s32 y0, s32 x1, s32 y1);
void MathFacingToDelta(u8 dir, s8 *outDx, s8 *outDy);

/* --- core_save.c --- */
void RebuildSavePointers(struct Toolkit *tk, u8 *saveBlockBase, u32 shift);
void XorSaveImage(u8 *ewramImage, u32 byteLen, u32 *keyWordSlot);
u32 ComputeSaveChecksum(const u8 *ewramImage, struct ProgressState *progress, u32 versionKey);
void WriteSaveMagic(struct ProgressState *progress);
int VerifySaveMagic(const struct ProgressState *progress);

/* --- core_inventory.c --- */
const struct BattleChipData *GetChipRecord(const struct BattleChipData *table, u16 chipId);
u16 FolderEntryChipId(u16 entry);
u8 FolderEntryChipCode(u16 entry);
u16 FolderMakeEntry(u16 chipId, u8 code);
u32 ChipPackCountById(const struct Toolkit *tk, u16 chipId);
u32 AddZenny(struct Toolkit *tk, s32 delta);
u32 AddBugFrags(struct Toolkit *tk, s32 delta);
u8 GetItemCount(const struct Toolkit *tk, u8 itemId);
u8 AddItemClamped(struct Toolkit *tk, u8 itemId, u8 amount, u8 cap);

/* --- core_folder.c --- */
struct FolderChipMeta {
    u16 chipId;
    u8 code;
    u8 chipClass;
};

u32 FolderValidate(const struct FolderChipMeta *chips, u32 count,
                   u8 megaLimit, u8 gigaLimit);
u32 FolderCanSetRegular(u8 chipMb, u8 regMemory);
void FolderShuffleDeck(u8 deck[30], u8 regIndex, u32 *rngSeed);

/* --- core_map.c --- */
struct MapWarpTarget {
    u8 mapGroup;
    u8 mapId;
    s16 x;
    s16 y;
    s16 z;
    u8 facing;
};

u16 MapPackKey(u8 mapGroup, u8 mapId);
u32 MapIsCyberworld(u8 mapGroup);
void MapApplyWarp(struct GameState *gs, const struct MapWarpTarget *warp);
u32 MapStepEncounter(u16 *stepCounter, u16 stepDelta, u16 threshold,
                     u8 sneakRunActive, u16 playerMaxHp, u16 enemyMaxHp);

/* --- core_world.c --- */
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

u8 TournamentGetRound(const u8 *tournamentEntered,
                      const struct TournamentBracketSlot (*brackets)[8],
                      u8 tournamentId);
int MysteryDataSetSlot(struct Toolkit *tk, u16 flag, u8 lo, u8 hi);
u16 MysteryDataGetSlot(const struct Toolkit *tk, u16 flag);
u32 MysteryDataResetList(struct Toolkit *tk, const struct MysteryDataEntry *entries);

/* --- core_shop.c --- */
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
int ShopTakeStock(struct ShopStockEntry *slots, u32 slotCount,
                  u8 type, u16 id, u8 code, int shiftUp);
int ShopRestockItem(struct ShopStockEntry *slots, u32 slotCount, u16 id);

/* --- core_subchips.c --- */
struct SubChipState {
    u8 qty[6];
    u8 maxPerSlot;
    u8 sneakRunActive;
    u8 untrapActive;
    u8 locEnemyActive;
};

u32 SubChipAdd(struct SubChipState *sc, u8 id, u8 amount);
u32 SubChipUseRecovery(struct SubChipState *sc, u8 id, u16 *curHp, u16 maxHp);
u32 SubChipActivateField(struct SubChipState *sc, u8 id);

/* --- core_chip_trader.c --- */
struct TraderPoolEntry {
    u16 chipId;
    u8 code;
    u8 weight;
};

u32 TraderRequiredCost(u8 kind);
u32 TraderCanSpin(u8 kind, u32 available);
const struct TraderPoolEntry *TraderRoll(const struct TraderPoolEntry *pool,
                                         u32 poolLen, u32 *rngSeed);

/* --- core_navicust.c --- */
void NaviCustClearGrid(struct Toolkit *tk);
int NaviCustSetCell(struct Toolkit *tk, u8 x, u8 y, u8 slotIndex);
u8 NaviCustGetCell(const struct Toolkit *tk, u8 x, u8 y);
u32 NaviCustCountPlaced(const struct Toolkit *tk);
int NaviCustOnCommandLine(const struct Toolkit *tk, u8 slotIndex);

/* --- core_navi_stats.c --- */
struct NaviBuildBonuses {
    u16 hpMemoryCount;
    s16 naviCustHpBonus;
    u16 darkChipPenalty;
    u8 customPlus;
    u8 megaFolderPlus;
    u8 gigaFolderPlus;
};

u16 NaviCalcMaxHp(const struct NaviBuildBonuses *b);
u8 NaviCalcCustomHandSize(const struct NaviBuildBonuses *b);
u8 NaviCalcMegaLimit(const struct NaviBuildBonuses *b);
u8 NaviCalcGigaLimit(const struct NaviBuildBonuses *b);

/* --- core_mail.c --- */
struct MailState {
    u8 received[8];
    u8 read[8];
    u8 claimed[8];
};

u32 MailDeliver(struct MailState *ms, u32 id);
u32 MailMarkRead(struct MailState *ms, u32 id);
u32 MailClaimAttachment(struct MailState *ms, u32 id);
u32 MailCountUnread(const struct MailState *ms);

/* --- core_text_script.c --- */
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

/* --- core_library.c --- */
u32 LibraryBitTest(const u8 *bits, u32 idx, u32 maxEntries);
u32 LibraryBitSet(u8 *bits, u32 idx, u32 maxEntries);
u32 LibraryCountRecorded(const u8 *bits, u32 maxEntries);
u32 LibraryHasSoulStar(u32 soulMask, u32 requiredCount);

/* --- core_battle_soul.c --- */
enum DoubleSoulId FindDoubleSoulForChip(const struct Toolkit *tk,
                                        const struct BattleChipData *chip,
                                        const u8 soulFlags[12],
                                        u16 usedSoulBits);
const u8 *GetAllDoubleSoulFlags(void);

/* --- core_battle_score.c --- */
u8 RaiseEnemyIdForPlaythrough(u8 enemyId, u8 enemyRankField, u8 playthrough, u8 battleFlags);
s32 ScoreBustingFlinch(s32 flinchCount);
s32 ScoreBustingMovement(s32 moveCount);
s32 ScoreBustingMultiDelete(u8 maxSimultaneousDeletes);
s32 ScoreBustingHpRatio(u16 currentHp, u16 maxHp);
u8 ClampBustingLevel(s32 rawScore);

/* --- core_battle_rewards.c --- */
struct SpPlusReward {
    u32 zenny;
    u32 bugFrags;
};

u8 BattleRewardTierFromBusting(u8 bustingLevel);
u32 BattleScaleZenny(u32 baseZenny, u8 violetEclipse);
struct SpPlusReward BattleRollSpPlusReward(u32 *rngSeed);

/* --- core_battle_grid.c --- */
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

/* --- core_battle_loop.c --- */
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

/* --- core_battle_turn.c --- */
struct BattleTurnState {
    u8 phase;
    u8 turnNumber;
    u8 activeSoul;
    u8 soulTurnsLeft;
    u16 customGauge;
    u16 gaugeSpeed;
    u32 totalCombatFrames;
    u8 enemiesAlive;
    u8 playerDeleted;
};

void BattleTurnInit(struct BattleTurnState *bt, u8 enemiesAlive, u16 gaugeSpeed);
void BattleTurnEnterCustom(struct BattleTurnState *bt);
void BattleTurnStartCombat(struct BattleTurnState *bt, u8 newSoulId);
u32 BattleTurnStepCombat(struct BattleTurnState *bt);

/* --- core_chips.c --- */
u8 BusterClampStat(s32 rawStat);
u16 BusterChargeDamage(u8 attackStat);
u16 BusterChargeFrames(u8 chargeStat);
u32 DarkChipApplyHpPenalty(u16 *maxHp, u16 *curHp);
u32 ChipCanSacrificeForSoul(u8 chipElement, u8 chipClass, u8 targetElem);

/* --- core_projectiles.c --- */
struct BattleProjectile {
    u8 active;
    u8 owner;
    u8 piercing;
    s8 dx;
    s8 x;
    s8 y;
    u16 damage;
    u16 moveDelay;
    u16 timer;
};

void ProjectileSpawn(struct BattleProjectile *p, u8 owner, s8 x, s8 y,
                     s8 dx, u16 damage, u16 moveDelay, u8 piercing);
u32 ProjectileStep(struct BattleProjectile *p);
u16 ProjectileCheckHit(struct BattleProjectile *p, u8 targetOwner, s8 tx, s8 ty);

/* --- core_custom_screen.c --- */
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
u16 CustomFindPA(const struct CustomChipSlot *selected, u32 selectedCount,
                 const struct ProgramAdvanceRecipe *recipes, u32 recipeCount, u32 *outStart);

/* --- core_emotion.c --- */
enum EmotionState {
    EMOTION_NORMAL       = 0,
    EMOTION_FULL_SYNCHRO = 1,
    EMOTION_ANGER        = 2,
    EMOTION_ANXIOUS      = 3,
    EMOTION_EVIL         = 4,
    EMOTION_DARK_SOUL    = 5
};

s16 EmotionAdjustKarma(s16 karma, s16 delta);
int EmotionIsEvilKarma(s16 karma);
enum EmotionState EmotionOnCounterHit(enum EmotionState cur, s16 karma);
enum EmotionState EmotionOnTakeHit(enum EmotionState cur, u16 damage, u16 maxHp);
u8 EmotionChipDamageMultiplier(enum EmotionState cur);

/* --- core_enemy_ai.c --- */
u16 BossCalcSpRecordBonus(u32 bestFrames);
u16 BossScaleSpPlusHp(u16 baseHp);
u16 BossScaleSpPlusAtk(u16 baseAtk);
u16 BossScaleSpPlusSpeed(u16 baseSpeed);
u8 BossAdvanceRematchRank(u8 currentRank, u8 bustingLevel, int requireSRank);

/* --- core_virus_ai.c --- */
struct VirusAiState {
    s8 x;
    s8 y;
    u16 attackCooldown;
    u16 moveCooldown;
    u8 guarding;
};

u32 VirusIsAlignedRow(const struct VirusAiState *v, s8 playerY);
u32 VirusTrackRow(struct VirusAiState *v, s8 playerY);
u32 VirusTickMettaurCycle(struct VirusAiState *v, u16 resetFrames);
void VirusPickRandomEnemyPanel(u32 *rngSeed, s8 *outX, s8 *outY);

/* --- core_boss_patterns.c --- */
struct BossPatternState {
    s16 hp;
    s16 maxHp;
    u8 phase;
    u16 auraRegenTimer;
    u16 auraRegenPeriod;
    u16 counterWindow;
};

u8 BossUpdatePhase(struct BossPatternState *b);
u32 BossTickAuraRegen(struct BossPatternState *b, u8 auraActive);
u32 BossCheckCounterHit(struct BossPatternState *b);

/* --- core_status_effects.c --- */
struct BattleStatusState {
    u8 activeStatus;
    u16 durationFrames;
    u8 statusGuard;
};

u32 StatusInflict(struct BattleStatusState *st, u8 statusKind, u16 frames);
void StatusTick(struct BattleStatusState *st);
u32 StatusIsImmobilized(const struct BattleStatusState *st);
s32 StatusApplyHitBonus(struct BattleStatusState *st, s32 damage, u8 isBreak, u8 element);

/* --- core_chip_effects.c --- */
u32 ChipApplyAreaGrab(struct BattlePanel grid[18], u8 attackerOwner);
void ChipApplyGeddon1(struct BattlePanel grid[18]);
u32 ChipApplySanctuary(struct BattlePanel grid[18], u8 owner);

/* --- core_soul_passives.c --- */
u32 SoulGetPassiveFlags(u8 soulId);
u16 SoulGetNullChipBonus(u8 soulId);

/* --- core_overworld_npc.c --- */
u32 OverworldAabbOverlap(s32 ax, s32 ay, s32 ahw, s32 ahh,
                         s32 bx, s32 by, s32 bhw, s32 bhh);
u32 OverworldCanTalkToNpc(s32 px, s32 py, u8 facing,
                          s32 nx, s32 ny, s32 reach, s32 npcRadius);

/* --- core_chip_combo.c --- */
u16 ChipApplyFlatModifiers(u16 baseDamage, u16 flatBonus, u8 panelsStolen);
u16 ChipCalcFinalDamage(u16 baseDamage, u16 flatBonus,
                        enum EmotionState emotion,
                        u8 attackerElem, u8 defenderElem, u8 panelState);

/* --- core_net_security.c --- */
u32 NetCanOpenBlackEarthGate(const struct Toolkit *tk, u32 soulMask,
                             u32 requiredSouls, u16 gateFlag);
u32 NetCanUnlockSecurityCube(const struct Toolkit *tk, u16 openedFlag, u8 keyItemId);
u8 NetResolveTunerTier(const struct Toolkit *tk, u8 tunerSetting);

/* --- core_link_battle.c --- */
u32 NetBattleResolveTiebreak(u16 p1Hp, u16 p1MaxHp, u16 p2Hp, u16 p2MaxHp);
u32 NetBattleIsTurnLimitReached(u8 turnNumber);
u32 NetBattleRecordRoundWin(u8 *p1Wins, u8 *p2Wins, u8 winner, u8 targetWins);

/* --- core_key_items.c --- */
u8 PetApplyRegUp(u8 *regMemory, u8 deltaMb);
u32 PetApplySubMem(u8 *subMemCap);
u32 PetCanRotateColor(u8 spinPartsMask, u8 colorIndex);

/* --- core_friendly_matches.c --- */
u32 FriendlyMatchIsAvailable(const struct Toolkit *tk, u8 currentRound,
                             u8 requiredRound, u16 clearedFlag);
u32 FriendlyMatchResolve(struct Toolkit *tk, u8 playerWon,
                         u16 clearedFlag, u16 soulFlag);
u8 RematchLadderOnWin(u8 currentTier, u8 bustingLevel, u8 isDuo);

/* --- core_navicust_bugs.c --- */
struct NaviCustPartDesc {
    u8 partId;
    u8 color;
    u8 isSolid;
};

u32 NaviCustHasColorClash(const u8 gridPartIds[25], const u8 gridColors[25]);
u32 NaviCustCountDistinctColors(const u8 gridColors[25]);
u32 NaviCustEvalPartBugs(const struct NaviCustPartDesc *desc,
                         u8 touchesCmdLine,
                         const u8 gridPartIds[25],
                         const u8 gridColors[25]);

#ifdef __cplusplus
}
#endif

#endif /* MMBN4_MODERN_API_H */
