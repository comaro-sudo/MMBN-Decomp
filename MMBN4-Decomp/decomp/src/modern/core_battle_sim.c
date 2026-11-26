/*
 * Portable C End-to-End Battle Frame Simulator Glue (Phase E2).
 *
 * Combines BattleTurnState, BattleActorState (player & enemy),
 * BattleStatusState, and BattlePanel[18] into a single cohesive frame
 * simulation step for headless battle verification & native port integration.
 */
#include "modern_api.h"

/*
 * Initialize a 1v1 headless battle simulation with player/enemy HP and
 * default 6x3 battle grid.
 */
void BattleSimInit(struct BattleSimState *sim, s16 playerHp, s16 enemyHp, u16 gaugeSpeed) {
    u32 i;
    u8 *raw = (u8 *)sim;

    for (i = 0; i < (u32)sizeof(*sim); i++) {
        raw[i] = 0u;
    }
    BattleGridInitDefault(sim->grid);
    BattleTurnInit(&sim->turn, 1u, gaugeSpeed);
    sim->player.hp = playerHp;
    sim->player.maxHp = playerHp;
    sim->enemy.hp = enemyHp;
    sim->enemy.maxHp = enemyHp;
}

/*
 * Apply a player attack of `damage` (with HitFlags) against the enemy actor,
 * propagating enemy deletion to `sim->turn.enemiesAlive`.
 */
s32 BattleSimPlayerAttack(struct BattleSimState *sim, s32 damage, u32 hitFlags) {
    s32 dealt = BattleActorApplyHit(&sim->enemy, damage, hitFlags);
    if (sim->enemy.deleted) {
        sim->turn.enemiesAlive = 0u;
    }
    return dealt;
}

/*
 * Apply an enemy attack of `damage` (with HitFlags) against the player actor,
 * propagating player deletion to `sim->turn.playerDeleted`.
 */
s32 BattleSimEnemyAttack(struct BattleSimState *sim, s32 damage, u32 hitFlags) {
    s32 dealt = BattleActorApplyHit(&sim->player, damage, hitFlags);
    if (sim->player.deleted) {
        sim->turn.playerDeleted = 1u;
    }
    return dealt;
}

/*
 * Advance the entire 1v1 battle simulation by 1 frame (timers, statuses,
 * turn coordinator & Custom Gauge). Returns 1 if Custom Gauge is ready.
 */
u32 BattleSimTick(struct BattleSimState *sim) {
    BattleActorTickTimers(&sim->player);
    BattleActorTickTimers(&sim->enemy);
    StatusTick(&sim->playerStatus);
    StatusTick(&sim->enemyStatus);
    return BattleTurnStepCombat(&sim->turn);
}
