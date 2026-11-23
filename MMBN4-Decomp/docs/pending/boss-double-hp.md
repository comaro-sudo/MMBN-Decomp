# Plan: an extra difficulty for every boss, with double HP (owner request, 2026-11-23)

**Status: plan, with open decisions.** The owner asked for "an additional difficulty for each boss, double the health".

## What it means

Every boss (the 20 Navis, Duo, Bass) gets **one more, harder version: the same boss with 2× HP**, on top of the existing V1 / V2 / V3 / SP. Attacks and AI stay as they are; only HP doubles, unless the owner later wants more.

## The technical constraint (V)

- Enemy HP is stored in **12 bits** of each `BN4_ENEMY_STATS` record (`hp_elem`: bits 0–11 HP, 12–15 element; RE-06), so the **maximum is 4095**.
- Doubling in the data works for most bosses, but **7 would overflow**: BassXX 4000, DuoΩ 3500, BassΩ 3000, Duoβ 3000, Duoα 2500, WoodManΩ 2500, LaserManΩ 2200.
- So the reliable way is **a small code hook at battle start** that doubles the boss's HP in the battle object (a 16-bit field in RAM, T: to confirm) when the extra difficulty is active. No new enemy records are needed; it works for every boss, including the 7.

## How the player reaches it (options)

| Option | How | Notes |
|---|---|---|
| **A. A 5th lobby tier** | The operator rematch climbs V1 → V2 → V3 → SP → **SP 2× HP** (after an S rank against SP) | Fits the lobby-rematch design; only Soul Navis have operators |
| **B. A global "Hard bosses" switch** | A toggle (e.g. in the Frequency Tuner, WP-B2) doubles every boss's HP everywhere: story, tournaments, roaming ghosts, rematches | Covers every boss at once; the story gets harder too |
| **C. A rematch-only mode** | Every rematch (lobby, Bass, Duo, roaming SP ghosts) can be chosen at 2× HP; story fights stay normal | Keeps the story's difficulty curve |

Claude's recommendation: **C, built on the hook**. It gives every boss an extra difficulty without changing the story. A is a subset of it (the lobby's 5th step).

## Open decisions for the owner

1. Which option: A, B or C (or a mix)?
2. **Which base version is doubled:** SP (the hardest, recommended), or whichever tier is being fought?
3. **Which bosses:** all 20 Navis + Duo + Bass (recommended), or also story bosses that aren't Navis?
4. **Rewards:** the same as that tier, or something extra (e.g. a guaranteed best drop, or a record in the Library or BBS)?
5. **Anything beyond HP** (attack power, speed)? Doubling HP alone is the simplest.

## Implementation sketch (after the decisions)

- Find where a battle copies the enemy record's HP into the battle object; RE-06's `BN4_ENEMY_RECORD` (`0x08011878`) is the entry point.
- Hook it: if the extra difficulty is active and the enemy is a boss (ids `0x80`–`0xE6`), double the HP. The hook goes in free space through a veneer, like the other packages.
- The flag for "extra difficulty active": set by the chosen option's script (lobby, rematch menu, or the Frequency Tuner).
- Test in the PC port: check the boss's HP in RAM, and that the HP bar shows the doubled value.
