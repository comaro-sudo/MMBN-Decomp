# Violet Eclipse (MMBN4) TODO

Updated: 2026-11-23 by Claude, at the owner's request, after Gemini's patch sprint and Claude's review.
This file is normally Gemini's (see [`AGENTS.md`](../AGENTS.md)).

Handoffs:
- [`docs/handoff-violet-eclipse.md`](docs/handoff-violet-eclipse.md) (Claude);
- [`decomp/notes/handoff-2026-11-23-gemini-patch-sprint.md`](decomp/notes/handoff-2026-11-23-gemini-patch-sprint.md) (Gemini).

Decisions waiting for the owner: [`docs/pending/`](docs/pending/). The patch packages and their test status: [`patch/README.md`](patch/README.md).

## Coordination

- **Gemini** owns `decomp/` and this file; Claude may also work in `decomp/`.
- **Claude** owns `plan/`, `docs/`, `patch/` and `ports/`. Gemini worked in `patch/` for the sprint with the owner's approval.
- New game folders (BN1, 2, … and the Principia Trilogy) are Claude's.
- Stage by explicit path, never `git add -A`. Never commit ROM bytes.
- [ ] **Push to GitHub:** local `main` is ahead of `origin/main`. The owner decides when.

## Done (2026-11-22/23)

| Area | What | Where |
|---|---|---|
| Decomp | RS and BM build from one shared tree (DC-4); 59.4% coverage; names merged through Gemini's round 5 | `57ea3b9` and earlier |
| Patch: economy and drops | WP-A1 WoodMan fix; WP-A2 Zenny × 1.5; WP-D2 random RS/BM drops; WP-D3 Bass gives both chips | PASS in-game |
| Patch: Souls | All 12 unite; the completion star needs 12; the status screen shows 12 on two pages (L/R); the Black Earth gate keeps 6 (owner, no patch) | PASS in-game |
| Patch: tournaments | WP-B1 fixed canon pairs (AquaMan, Roll, ProtoMan) and a deterministic bracket; the Free Tournament fields the 12 Soul Navis without trades (36 secret chips) | `bac8ce9`, `c9d36cf` |
| Patch: lobby | 9 operators in Higsby's, 2 on the street; rematches climb V1 → V2 → V3 → SP; JunkMan's trash can | `ce9d481` |
| Patch: other | Z-Saver Blue Mystery Data (Undernet 1); Frequency Tuner (WP-B2); NPC lists in free space | `7c0f9b8`, `ec5e084`, `90814b8` |
| RE | Save, flags, Souls, shops, Mystery Data, encounters, tournaments, free space, every save slot; all placements and patch designs | `docs/re/`, `decomp/notes/` |

## Order of the remaining work

Owner decisions marked ⚠ come first.

| # | Task | Depends on / notes |
|---|---|---|
| 1 | **Merge the names** from Gemini's round 6–7 notes, then a full regeneration | Quick decomp housekeeping |
| 2 | **Lobby fixes:**<br>– real sprites for the 7 operators using placeholders, plus a trash-can sprite;<br>– **confirm the "met" gate** (flags `0x0001 + id`: do they mean "met that Navi"?);<br>– check that JunkMan's map `WtrGodComp1 (8E, 00)` doesn't interfere with AquaMan's side story | Correctness of the lobby that's already built |
| 3 | **The other version's Souls**: a grant routine or script (`soul-grant-patch-design.md`) | Needed by step 4 |
| 4 | **The 9 friendly tournament matches**: all before the final, 2–3 before the canon fight, a win gives the Soul, retry on a loss, counts as "met" (`wp-b1-design.md`, `canon-navis.md` §6–§7) | Needs 3. ⚠ The RS/BM version choice (`plan/DEFINITIVE-EDITION.md` §7 q1) affects it. **New text:** the friendly-match dialogue needs TextPet insertion |
| 5 | **SP+, the extra boss difficulty** (`docs/pending/boss-double-hp.md`):<br>– HP 4,095 for every boss;<br>– a 5th lobby step after an S rank against SP;<br>– bosses without an operator get it from their roaming SP ghost after an S rank;<br>– attack × 1.5 and speed × 1.25 | Needs research first: where enemy damage and AI timing can be scaled. ⚠ The reward amount (Zenny or BugFrags) |
| 6 | **Duo and Grand Prix Power:**<br>– both have blank names and art in the US ROMs: copy the art from the player's **JP ROM** at build time, and write English names;<br>– Duo: a post-Duo grant, since the fight skips the results screen;<br>– Grand Prix Power: a Blue Mystery Data in ACDC Area 2 `(106, 326)`, flag `0x0D0B` | Then fix or remove `wp_d2_duo_chip_reward.asm` (no effect today) |
| 7 | WP-B4 GMD rewards by tier; save protection (WP-B6); the remaining boss rematches | After the content is in place |
| 8 | ⚠ Tier A leftovers: story-battle rewards, **key-item prices** (decision), ToyRobo, cutscene skip | |
| 9 | **A load hook for existing saves**: data copied in at new game (shop stock, …) | Last, once everything that lives in the save is known |
| 10 | **Full-playthrough and completion testing** (C1–C5) on the PC port | Validates everything together |
| 11 | ⚠ Ports: EL-0 refactor → PC-2 (PET screen, touch Custom Screen, net map) → Wii U, 3DS, NDS | ⚠ Hardware owned; the GBARunner licence |
| 12 | Decomp, long term: carve mixed tables once decoded, reduce the 132 version blocks, DC-5 C | Ongoing, between steps |
| 13 | ⚠ The collection: the BN1, 2, 3, 5 and 6 decomps → the MMBN Principia Trilogy (ports only) | After BN4 |

## Owner decisions still open

- [ ] RS/BM version choice in the merged ROM (`plan/DEFINITIVE-EDITION.md` §7 q1). The canon lineup and the friendlies assume one merged story.
- [ ] The SP+ reward amount (a high Zenny or BugFrag amount).
- [ ] Key-item prices fixed or as in-game options; Tier C content in 1.0; D1–D9 ([`plan/00` §4](plan/00-decisions-and-feasibility.md)).
- [ ] Hardware owned for the console ports; the GBARunner licence ([`plan/emulation-layer/06`](plan/emulation-layer/06-milestones-and-risks.md)).
- [ ] Principia Trilogy: the base engine, which mechanics survive, how the story joins, the name.

## Before committing

- [ ] `git diff --check`; both forced byte-match builds (decomp); `python patch/build.py --check` (patch); coverage when the asm changes.
