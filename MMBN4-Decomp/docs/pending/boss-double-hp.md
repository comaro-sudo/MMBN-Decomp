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

## Owner decisions (2026-11-23)

1. **Option A: a 5th lobby step.** After an S rank against SP, the operator's next rematch is the extra version ("SP+", working name).
2. **Base: SP.**
3. **Updated by the owner (2026-11-23): every boss's SP+ has exactly 4,095 HP** (the 12-bit maximum), not 2× SP.
   - *(Earlier wording: SP+ HP = min(2 × SP HP, 4095).)*
   - Bosses without an operator reach SP+ through **option (a): their roaming SP ghosts become SP+ after an S rank** (see the issues below). This covers the 8 shared Navis; Duo and Bass reach it through their own rematch routes. The cap means the 7 overflowing bosses (above) stop at 4,095, so the 12-bit field is enough and **HP can be pure data** (no HP hook needed).
4. **Reward: a high amount of Zenny or BugFrags.** **Owner, 2026-11-23: random, 50/50** between **50 BugFrags** (`0xC032`) and **15,000 Zenny** (`0x7A98`). A reward halfword holds at most 16,383 (`RewardDecode`, Gemini's `sp-plus-design.md` §3).
5. **Attack × 1.5, speed × 1.25.**

## Issues these raise (Claude, for the owner and for research)

- **Resolved (owner, 2026-11-23): option (a).** Bosses without an operator get SP+ from their roaming SP ghost, after an S rank against it. Original issue: the lobby only has the 12 Soul Navis' operators (and JunkMan via the trash can). The **8 shared Navis** (TopMan, ShadeMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan), **Duo** and **Bass** have no operator in the lobby. **Owner to decide** how they reach SP+:
  - their roaming SP ghosts get an SP+ variant after an S rank;
  - more lobby NPCs (slots are tight: Higsby's 13/16, ACDC Town 16/16);
  - or a single "challenge" NPC or terminal that offers any of them.
- **Attack × 1.5 and speed × 1.25 aren't single fields.** Damage comes partly from the record's `base_atk` byte and partly from per-attack constants in each Navi's AI code. Speed is AI timing (frame counts per action), spread across each Navi's code.
  - Likely implementation: a battle-wide modifier active only in SP+ fights, scaling the damage MegaMan receives × 1.5 and shortening the enemy's AI wait timers to × 0.8 (= × 1.25 speed).
  - This needs research: where enemy damage to MegaMan is applied, and whether there's a common AI timer. **Feasibility to confirm before promising exact factors.**
- **New records or a hook?** SP+ needs its own identity (HP, reward). Either new enemy records (are there free enemy ids? 254 are used), or the SP record plus a "SP+ active" flag that a battle-start hook reads (HP from a small table, the damage and speed modifiers, the reward). The hook is likely, since attack and speed need code anyway.

## Open decisions for the owner (original list; 1–5 answered above)

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
