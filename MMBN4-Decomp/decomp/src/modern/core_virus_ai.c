/*
 * Portable C Virus AI Pattern & Grid Movement Helpers (Phase D3).
 *
 * Models common MMBN4 virus behaviors on the 6x3 battle grid (code_08):
 *   - Row-alignment check (Canodumb / Mettaur line-of-sight targeting)
 *   - Step-toward-row vertical tracking
 *   - Cooldown-driven attack readiness & Mettaur guard-state toggle
 *   - Random valid enemy-side panel hop selection
 */
#include "gba_types.h"

u32 StepRNG(u32 *rng);
u32 BattleGridInBounds(s32 x, s32 y);

struct VirusAiState {
    s8 x;
    s8 y;
    u16 attackCooldown;
    u16 moveCooldown;
    u8 guarding;
};

/*
 * Return 1 if the virus shares the same row as the player (y == playerY).
 */
u32 VirusIsAlignedRow(const struct VirusAiState *v, s8 playerY) {
    return (v != 0 && v->y == playerY) ? 1u : 0u;
}

/*
 * Move the virus 1 row closer to `playerY` (clamped to rows 1..3).
 * Returns 1 if the virus changed rows, 0 if already aligned.
 */
u32 VirusTrackRow(struct VirusAiState *v, s8 playerY) {
    s8 nextY;

    if (v == 0 || v->y == playerY) {
        return 0;
    }
    nextY = (v->y < playerY) ? (s8)(v->y + 1) : (s8)(v->y - 1);
    if (!BattleGridInBounds((s32)v->x, (s32)nextY)) {
        return 0;
    }
    v->y = nextY;
    return 1;
}

/*
 * Tick Mettaur-style attack cooldown: while cooling down, virus guards;
 * when cooldown hits 0, drops guard and returns 1 (fire shockwave).
 */
u32 VirusTickMettaurCycle(struct VirusAiState *v, u16 resetFrames) {
    if (v == 0) {
        return 0;
    }
    if (v->attackCooldown > 0u) {
        v->attackCooldown--;
        v->guarding = 1u;
        return 0;
    }
    v->guarding = 0u;
    v->attackCooldown = resetFrames;
    return 1;
}

/*
 * Pick a random panel on the enemy side (x in 4..6, y in 1..3) using StepRNG.
 */
void VirusPickRandomEnemyPanel(u32 *rngSeed, s8 *outX, s8 *outY) {
    u32 r = StepRNG(rngSeed);
    *outX = (s8)(4 + (r % 3u));
    *outY = (s8)(1 + ((r >> 4) % 3u));
}
