# Violet Eclipse (MMBN4) TODO

Updated: 2026-11-24 by Gemini, after Round 13 (`b08a41e`).
This file is Gemini's (see [`AGENTS.md`](../AGENTS.md)).

Handoffs:
- [`docs/handoff-violet-eclipse.md`](docs/handoff-violet-eclipse.md) (Claude);
- [`decomp/notes/handoff-2026-11-24-gemini-round13.md`](decomp/notes/handoff-2026-11-24-gemini-round13.md) (Gemini).

Decisions waiting for the owner: [`docs/pending/`](docs/pending/). The patch packages and their test status: [`patch/README.md`](patch/README.md).

## Coordination

- **Gemini** owns `decomp/` and this file (and temporarily `patch/` and `docs/` until Friday 2026-11-27); Claude may also work in `decomp/`.
- **Claude** owns `plan/`, `docs/`, `patch/` and `ports/` (normal ownership resumes after Friday 2026-11-27).
- New game folders (BN1, 2, … and the Principia Trilogy) are Claude's.
- Stage by explicit path, never `git add -A`. Never commit ROM bytes.
- [ ] **Push to GitHub:** local `main` is 5 commits ahead of `origin/main` (`4de9524`..`b08a41e`). The owner decides when.
- [ ] **Merge Round 13 commits into `docs/handoff-violet-eclipse.md`** (Claude, when back).

## Done (through 2026-11-24, Round 13)

| Area | What | Where |
|---|---|---|
| Decomp | RS and BM build from one shared tree (DC-4); 59.4% coverage; names merged through Round 10 + full regeneration | `95c6e92`, `92538c0` |
| Patch: economy & drops | WP-A1 WoodMan fix; WP-A2 Zenny × 1.5; **WP-A3 key-item/upgrade shop prices at 40% (Option 2, 34 slots)**; WP-D2 random RS/BM drops; WP-D3 Bass gives both chips | PASS in-game (`4de9524`, `patch-test-results-6.md`) |
| Patch: Souls & Duo Soul | All 12 + **13th Duo Soul** (unite, MetalSoul sprite + custom Duo palette, Air/Float + B+Left Reflect passive, clean emotion window, MetalSoul icon); completion star needs 12; **3-page Status screen (L/R)**; Black Earth gate keeps 6 (owner, no patch); `FC 0C` grant-Soul command | PASS in-game (`4c71998`, `31261d5`, `patch-test-results-6.md`) |
| Patch: tournaments & friendly matches | WP-B1 canon pairs (AquaMan, Roll, ProtoMan) & deterministic bracket; Free Tournament fields all 12 Soul Navis (36 secret chips); **9 friendly tournament matches** (3 per venue, `FC 0E`, round-gated, retry on loss, win grants Soul & hides NPC, `b4 = 0x3D` results fix) | PASS in-game (`bac8ce9`, `c9d36cf`, `9ab3301`, `a6cf221`) |
| Patch: lobby, Duo & SP+ | 9 operators in Higsby's + 2 on street + debris-pile trash can (`list[7] 0x4A` -> `WtrGodComp1`); **lobby rematches climb V1 → V2 → V3 → SP → SP+** (`b4 = 0x3D`); **Duo rematches** in Meteor Comp 1 (`list[7] 0x07` crest, `FC 0D`) with **S-rank climb V1 → α → β → Ω → SP+** (`0x08007692`); **Duo story/rematch grant** (`dat_080C04F8[2]`, Duo Soul + Duo D); **SP+ difficulty** (4,095 HP, ATK ×1.5, Speed ×1.25, 80/20 15,000Z / 50 BF reward); battle-exit marker clear | PASS in-game (`76383e4`, `f8da808`, `532543d`, `acf65ad`, `a6cf221`) |
| Patch: chips, MD & QoL | Z-Saver BMD (Undernet 1); Grand Prix Power BMD (ACDC Area 2); Duo & GP Power chip names/descriptions (TextPet + e-Reader threshold `0x1000`) and local chip art (`--public` omits); Frequency Tuner (WP-B2); GMD follows Tuner (WP-B4); **Waiting Room Guard A** (`1 <= r6 <= 7`, WP-B6); NPC lists in free space; 2 verified `code_00` veneer pools (`sub_0800A144`, `sub_08001784`) | Built & verified; Z-Saver, Tuner, Guard A & WP-A3 PASS (`71d149e`, `patch-test-results-6.md`) |
| RE | Save, flags, Souls, shops, Mystery Data, encounters, tournaments, free space, every save slot, veneers, props, BattleModeFlags, Duo rank | `docs/re/`, `decomp/notes/` |

## Order of the remaining work

Owner decisions marked ⚠ come first.

| # | Task | Depends on / notes |
|---|---|---|
| 1 | **Fix LZ77-compressed NPC sprites (`Higsby 0x08` and `Sal 0x2C`)** | Both have `bit 31 = 1` in `BN4_SPRITE_LIST_6`, so they draw as white circles on maps that don't preload them (Higsby in DenDome Hall; Sal in Higsby's & Castillo WaitRm). Still talkable/functional; add to map preload lists or decompress at build time (`patch-test-results-6.md`) |
| 2 | **In-game smoke tests for remaining untested packages + minor cleanup:**<br>– test `wp_d2_gpp_md.asm` (ACDC Area 2 BMD), `wp_b4_gmd_tier.asm` (GMD tier override), `wp_d2_ereader_chips.asm` + `wp_d2_special_chip_art.asm` (Duo/GP Power names, descriptions & art in Library/pack), and roaming SP+ ghosts;<br>– remove or retire obsolete `wp_d2_duo_chip_reward.asm` (no-op since Duo skips results and `wp_d2_duo_story_grant.asm` already grants Duo D) | Quick verification & cleanup pass |
| 3 | **Load hook for existing saves** | Sync data copied only at New Game (WP-A3 shop stock in save slot `+0x58`, etc.) so pre-existing saves also get the 40% prices |
| 4 | ⚠ **Tier A leftovers & QoL:** story-battle rewards, ToyRobo, cutscene skip, and any remaining non-Soul boss rematches | Decide which Tier A items make 1.0 |
| 5 | ⚠ **Public-release chip art (`--public`):** placeholder/original art for Duo & Grand Prix Power | Local `ref/chip-art/` is Capcom art (gitignored); `--public` currently omits it |
| 6 | **Full-playthrough and completion testing (C1–C5)** on the PC port (RS & BM) | Validates every package together end-to-end |
| 7 | ⚠ **Ports:** EL-0 refactor → PC-2 (PET screen, touch Custom Screen, net map) → Wii U, 3DS, NDS | ⚠ Hardware owned; the GBARunner licence |
| 8 | **Decomp, long term:** carve mixed tables once decoded, reduce version blocks, DC-5 C | Ongoing, between steps |
| 9 | ⚠ **The collection:** BN1, 2, 3, 5 and 6 decomps → the MMBN Principia Trilogy (ports only) | After BN4 |

## Owner decisions still open

- [ ] RS/BM version choice in the merged ROM (`plan/DEFINITIVE-EDITION.md` §7 q1). The canon lineup and the friendlies assume one merged story.
- [ ] Tier A leftovers (story-battle rewards, ToyRobo, cutscene skip), Tier C content in 1.0, and D1–D9 ([`plan/00` §4](plan/00-decisions-and-feasibility.md)).
- [ ] Placeholder/original Duo & Grand Prix Power art for `--public` builds.
- [ ] Hardware owned for the console ports; the GBARunner licence ([`plan/emulation-layer/06`](plan/emulation-layer/06-milestones-and-risks.md)).
- [ ] Principia Trilogy: the base engine, which mechanics survive, how the story joins, the name.

## Before committing

- [ ] `git diff --check`; both forced byte-match builds (decomp); `python patch/build.py --check` (patch); coverage when the asm changes.
