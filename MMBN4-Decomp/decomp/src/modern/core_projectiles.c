/*
 * Portable C Battle Projectile & Grid Stepper (Phase D2).
 *
 * Models grid-aligned battle projectiles (linear shots, waves, piercing
 * projectiles) moving across MMBN4's 6x3 battle grid (x in 1..6, y in 1..3).
 */
#include "gba_types.h"

u32 BattleGridInBounds(s32 x, s32 y);

struct BattleProjectile {
    u8 active;
    u8 owner;       /* 0 = player, 1 = enemy */
    u8 piercing;    /* 1 = continues moving after hitting a target */
    s8 dx;          /* +1 (right) or -1 (left) */
    s8 x;           /* 1..6 */
    s8 y;           /* 1..3 */
    u16 damage;
    u16 moveDelay;  /* frames per panel step */
    u16 timer;      /* countdown to next step */
};

/*
 * Spawn a horizontal projectile at (x, y) moving in direction dx (+1 / -1).
 */
void ProjectileSpawn(struct BattleProjectile *p, u8 owner, s8 x, s8 y,
                     s8 dx, u16 damage, u16 moveDelay, u8 piercing) {
    p->active = 1u;
    p->owner = owner;
    p->piercing = piercing;
    p->dx = dx;
    p->x = x;
    p->y = y;
    p->damage = damage;
    p->moveDelay = (moveDelay == 0u) ? 1u : moveDelay;
    p->timer = p->moveDelay;
}

/*
 * Advance projectile by 1 frame. Moves 1 panel when timer reaches 0 and
 * deactivates if it leaves the 6x3 battle grid.
 * Returns 1 if the projectile moved onto a new panel this frame, 0 otherwise.
 */
u32 ProjectileStep(struct BattleProjectile *p) {
    if (p == 0 || !p->active) {
        return 0;
    }
    if (p->timer > 0u) {
        p->timer--;
    }
    if (p->timer > 0u) {
        return 0;
    }
    p->x = (s8)(p->x + p->dx);
    if (!BattleGridInBounds((s32)p->x, (s32)p->y)) {
        p->active = 0u;
        return 0;
    }
    p->timer = p->moveDelay;
    return 1;
}

/*
 * Test whether an active projectile overlaps a target at (tx, ty) belonging
 * to `targetOwner`. If it hits a non-piercing projectile, deactivates it.
 * Returns projectile damage on hit, or 0 on miss.
 */
u16 ProjectileCheckHit(struct BattleProjectile *p, u8 targetOwner, s8 tx, s8 ty) {
    if (p == 0 || !p->active || p->owner == targetOwner) {
        return 0;
    }
    if (p->x != tx || p->y != ty) {
        return 0;
    }
    if (!p->piercing) {
        p->active = 0u;
    }
    return p->damage;
}
