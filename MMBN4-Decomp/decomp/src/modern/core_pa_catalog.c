/*
 * Portable C Canonical MMBN4 Program Advance Recipe Catalog (Phase D2).
 *
 * Exposes standard MMBN4 Program Advance recipes directly consumable by
 * CustomFindPA() (GigaCannon, LifeSword, TwinLeader, BodyGuard, PoisonPharaoh).
 */
#include "modern_api.h"

static const struct ProgramAdvanceRecipe kCorePaRecipes[] = {
    /* GigaCannon 1: Cannon A-B-C (sequential codes via 0xFF wildcard) */
    { 500u, { 1u, 1u, 1u }, { 0xFFu, 0xFFu, 0xFFu } },
    /* LifeSword: Sword S + WideSwrd S + LongSwrd S (code 'S' = 18) */
    { 501u, { 35u, 36u, 37u }, { 18u, 18u, 18u } },
    /* BodyGuard: AntiDmg M + AntiNavi M + AntiRecv M (code 'M' = 12) */
    { 502u, { 112u, 113u, 114u }, { 12u, 12u, 12u } },
    /* PoisonPharaoh: PoisMask A + PoisFace A + Anubis A (code 'A' = 0) */
    { 503u, { 120u, 121u, 122u }, { 0u, 0u, 0u } }
};

/*
 * Return pointer to the built-in MMBN4 P.A. recipe catalog and write its
 * entry count to `*outCount` if non-NULL.
 */
const struct ProgramAdvanceRecipe *PaCatalogGetRecipes(u32 *outCount) {
    if (outCount != 0) {
        *outCount = (u32)(sizeof(kCorePaRecipes) / sizeof(kCorePaRecipes[0]));
    }
    return kCorePaRecipes;
}

/*
 * Convenience wrapper scanning `selected[0..selectedCount)` against the
 * built-in MMBN4 Program Advance catalog. Returns the matched PA chip ID or 0.
 */
u16 PaCatalogMatch(const struct CustomChipSlot *selected, u32 selectedCount,
                   u32 *outStart) {
    u32 count = 0u;
    const struct ProgramAdvanceRecipe *recipes = PaCatalogGetRecipes(&count);
    return CustomFindPA(selected, selectedCount, recipes, count, outStart);
}
