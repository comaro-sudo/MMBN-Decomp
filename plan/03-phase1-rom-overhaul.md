# 03 — Phase 1: ROM Overhaul (Work Packages)

Every item in Part I of the design doc maps to a work package (WP). Each one lists its RE prerequisites (see [02](02-reverse-engineering.md)), how it will be implemented, how it will be accepted, and its size (S ≈ days, M ≈ 1–3 weeks, L ≈ 1–2 months, XL ≈ 3+ months, part-time).

Implementation methods:
- **DATA** = table edits
- **SCRIPT** = TextPet event and text edits
- **ASM** = code hooks
- **CONTENT** = new maps, sprites or bosses

## Tier A — Quick wins (first public release, v0.x)

### WP-A1 — WoodMan softlock fix
- Prereq: RE-14
- Method: integrate or port `BN4-WoodMan-Fix` (credit the authors). Check whether it is Blue-Moon-only.
- Accept: fleeing battles during the vine sequence never softlocks (test on emulator and on GBARunner).
- Size: S

### WP-A2 — Zenny scaling (1.75x)
- Prereq: RE-08 (virus Zenny), RE-07 (GMD Zenny)
- Method: ASM multiplier in the reward routine (safer than editing every table). Clamp at the Zenny cap.
- Accept: a known virus gives 1.75x its original value, and GMD Zenny is scaled.
- Size: S

### WP-A3 — Rewards on scripted battles
- Prereq: RE-08
- Method: ASM. Bypass or clear the "no reward" condition for scenario battles and gauntlets. Exclude battles where a reward screen would break the script flow (test each one).
- Accept: every scripted battle shows a results screen with drops, and no script flow breaks.
- Size: M (the testing of each battle is the bulk of the work)

### WP-A4 — Shop price reduction for plot-essential items (60%)
- Prereq: RE-09
- Method: DATA
- Accept: the listed items cost 40% of their original price.
- Size: S

### WP-A5 — Cutscene / bracket-animation skip with START
- Prereq: RE-10
- Method: ASM hook in the script interpreter: on START, fast-forward to the script's end marker, or run text at max speed with no waits. Fast-forward is safer than a hard skip because every flag is still set.
- Accept: skipping any cutscene leaves the same flags and the same position as watching it (compare RAM dumps).
- Size: M

### WP-A6 — ToyRobo slide: 1 run instead of 3
- Prereq: RE-10
- Method: SCRIPT/ASM. Change the required count, or set the "completed" flag after the first run.
- Size: S

## Tier B — Progression rework (v1.0)

### WP-B1 — Single-playthrough Double Souls + deterministic brackets
- Prereq: RE-01..05
- Design (from the doc): Den → 2 Souls (QF, F); Eagle/Hawk → 2 (SF, F); RS/BM → 2 (QF, SF) + rival in the Final.
- Method: ASM to replace the bracket RNG with a fixed table per tournament. DATA for that table. SCRIPT to grant a Soul after the specified rounds (the original only grants after certain matches, so new grant events are needed).
- Sub-tasks:
  1. Document the original bracket and Soul rules (verify the design-doc claims).
  2. Write the fixed bracket table for each version.
  3. Hook the bracket generator.
  4. Add Soul-grant events and dialogue.
  5. Make sure each scenario opponent's chapter still plays its own story script (scenarios are tied to opponents).
- Accept: a fresh save reaches the credits with all 6 Souls, and the brackets are identical across fresh saves.
- Risk: **highest-risk WP.** The story scripts are built around which opponent you face. Scope it carefully.
- Size: L

### WP-B2 — Frequency Tuner (V1/V2/V3 virus tiers on demand)
- Prereq: RE-03, RE-06, RE-18
- Method: new RAM/save variable `tuner_tier`. Hook every read of the playthrough counter that is used for **encounters or GMD** (not story) so it reads `tuner_tier` instead. UI options:
  - (a) a NaviCust program colour/variant (least UI work)
  - (b) a new menu option at jack-in
  - Recommendation: (a) first.
- Accept: switching tier changes encounter tables immediately, and the story is unaffected.
- Size: M–L

### WP-B3 — Persistent Mystery Data
- Prereq: RE-07, RE-01
- Method: find the routine that resets or locks BMD/PMD at story milestones and disable it. Make sure the one-shot flags survive.
- Accept: a BMD skipped in chapter 1 can still be collected in the final chapter.
- Size: M

### WP-B4 — GMD loot tables follow the Tuner tier
- Prereq: WP-B2, RE-07
- Method: same hook as B2, applied to GMD table selection.
- Accept: every chip from every GMD table is obtainable with some tier setting, so none is missable. Verify with an extracted table audit script.
- Size: S–M

### WP-B5 — Optional NG+ (Hard Mode)
- Prereq: WP-B1, WP-B2
- Method: since Souls and tiers no longer need NG+, NG+ becomes: remixed virus placements (DATA), higher Zenny multiplier, key items kept (ASM on the NG+ init routine), Omega Navi tier (see 00 open question 4).
- Size: M (data-only Omega) / L+ (new bosses)

### WP-B6 — Waiting Room save protection
- Prereq: RE-13
- Method: relocate the transfer buffer to unused save space, or guard Soul flags from writes during transfer.
- Accept: link transfers in mGBA's multiplayer mode never change Soul flags.
- Size: M

### WP-B7 — Dark Karma redemption quest
- Prereq: RE-12, RE-15
- Method: a Job BBS request (reuses the BBS system) or an Undernet NPC. On completion, reset the karma counter and restore the max-HP penalty.
- Size: M

### WP-B8 — Duo refinements
- Prereq: RE-11
- Method: DATA/ASM to change hover timing (40% more time in the middle row) and add vulnerable windows for sword chips during recovery frames.
- Accept: a playtester with an average folder rates it as fair. Record before/after timing.
- Size: M

### WP-B9 — Generic Navis moved out of brackets and into the Job BBS
- Prereq: WP-B1, RE-15
- Method: they drop out of brackets automatically with B1's fixed tables. New BBS requests launch their battles and mini-games and give rewards.
- Size: M–L

## Tier C — Narrative & new content (v2.0, optional)

| WP | Item | Method | Size |
|---|---|---|---|
| C1 | Nebula operatives in tournaments + Dark Chip-supplied opponents | SCRIPT + battle setup flags | L |
| C2 | Forced Dark Chip temptation matches | SCRIPT + ASM (force-offer a Dark Chip in the Custom Screen) | M |
| C3 | Themed mini-dungeons replacing Park Area 3 backtracking (MetalMan factory, WoodMan greenhouse, JunkMan bank…) | CONTENT (maps, tilesets, warps, encounters) | XL |
| C4 | Black Earth: 3 new sub-areas | CONTENT | XL |
| C5 | Bass XX and Duo Omega super-bosses | CONTENT + boss AI | XL |
| C6 | Narrative resolution (origin of Dark Chips, Duo's judgment) | SCRIPT (writing plus events) | L |

Prerequisites for all of Tier C: RE-16, plus a decision on custom sprite tooling. Start a writing doc (`docs/narrative.md`) before any implementation.

## Suggested Tier A order

A1 → A2 → A4 → A6 → A3 → A5. The cheapest go first, which builds confidence in the pipeline. A5 last, because it needs the deepest understanding of the script engine.

## Phase 1 definition of done

- Both US versions build from one command and produce `.bps` patches.
- The full playthrough test (see [07](07-testing-and-qa.md)) passes on both versions.
- The changelog documents every change against the original.
