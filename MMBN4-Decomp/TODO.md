# Violet Eclipse (MMBN4) TODO

Updated: 2026-11-25 by Gemini, after EL-0 ports refactor (`c934453`) + decomp table carving (`a57a907`).
This file is Gemini's (see [`AGENTS.md`](../AGENTS.md)).

Handoffs:
- [`docs/handoff-violet-eclipse.md`](docs/handoff-violet-eclipse.md) (Claude);
- [`decomp/notes/handoff-2026-11-25-gemini-el0-decomp.md`](decomp/notes/handoff-2026-11-25-gemini-el0-decomp.md) (Gemini).

Decisions waiting for the owner: [`docs/pending/`](docs/pending/). The patch packages and their test status: [`patch/README.md`](patch/README.md).

## Coordination

- **Gemini** owns `decomp/` and this file (and temporarily `patch/` and `docs/` until Friday 2026-11-27); Claude may also work in `decomp/`.
- **Claude** owns `plan/`, `docs/`, `patch/` and `ports/` (normal ownership resumes after Friday 2026-11-27).
- New game folders (BN1, 2, … and the Principia Trilogy) are Claude's.
- Stage by explicit path, never `git add -A`. Never commit ROM bytes.
- [x] **Push to GitHub:** `origin/main` is up to date through Round 13 + RS/BM C1–C5 matrix & decomp symbol merge.
- [ ] **Merge Round 13 commits into `docs/handoff-violet-eclipse.md`** (Claude, when back).

## Done (through 2026-11-24, Round 13)

| Area | What | Where |
|---|---|---|
| Decomp | RS and BM build from one shared tree (DC-4); **61.9% coverage** (`729,588 / 1,179,456` bytes, `make -B compare` OK on RS & BM); 7 mixed/pointer tables carved across `code_00/02/03/09` (+7,320B); `1,110` symbols | `95c6e92`, `a3884fb`, `a57a907` |
| Ports (PC / EL-0) | **EL-0 complete**: shared `ports/common/` (`gba_mem.h`, `emu_backend.h`, `emu_mgba.c` with idle-loop detection, `game.c` for **both RS & BM**, `ui_draw.h`, `pet_model.c`, `pet_ui.c`, `bench.c`) + thin SDL3 PC frontend (`ui_draw_sdl3.c`, `--bench`); verified on RS & BM (1,033–1,302 fps on ARM64) | `c934453` |
| Patch: economy & drops | WP-A1 WoodMan fix; WP-A2 Zenny × 1.5; **WP-A3 key-item/upgrade shop prices at 40% (Option 2, 34 slots + existing-save `SaveLoad` sync hook)**; WP-D2 random RS/BM drops; WP-D3 Bass gives both chips | PASS in-game (`4de9524`, `patch-test-results-6.md`) |
| Patch: Souls & Duo Soul | All 12 + **13th Duo Soul** (unite, MetalSoul sprite + custom Duo palette, Air/Float + B+Left Reflect passive, clean emotion window, MetalSoul icon); completion star needs 12; **3-page Status screen (L/R)**; Black Earth gate keeps 6 (owner, no patch); `FC 0C` grant-Soul command | PASS in-game (`4c71998`, `31261d5`, `patch-test-results-6.md`) |
| Patch: tournaments & friendly matches | WP-B1 canon pairs (AquaMan, Roll, ProtoMan) & deterministic bracket; Free Tournament fields all 12 Soul Navis (36 secret chips); **9 friendly tournament matches** (3 per venue, `FC 0E`, round-gated, retry on loss, win grants Soul & hides NPC, `b4 = 0x3D` results fix, Higsby & Sal lazy LZ77 sprite load) | PASS in-game (`bac8ce9`, `c9d36cf`, `9ab3301`, `a6cf221`) |
| Patch: lobby, Duo & SP+ | 9 operators in Higsby's + 2 on street + debris-pile trash can (`list[7] 0x4A` -> `WtrGodComp1`); **lobby rematches climb V1 → V2 → V3 → SP → SP+** (`b4 = 0x3D`); **Duo rematches** in Meteor Comp 1 (`list[7] 0x07` crest, `FC 0D`) with **S-rank climb V1 → α → β → Ω → SP+** (`0x08007692`); **Duo story/rematch grant** (`dat_080C04F8[2]`, Duo Soul + Duo D; obsolete `wp_d2_duo_chip_reward.asm` retired); **SP+ difficulty** (4,095 HP, ATK ×1.5, Speed ×1.25, 80/20 15,000Z / 50 BF reward); battle-exit marker clear | PASS in-game (`76383e4`, `f8da808`, `532543d`, `acf65ad`, `a6cf221`) |
| Patch: chips, MD & QoL | Z-Saver BMD (Undernet 1); **Grand Prix Power BMD (ACDC Area 2, PASS in-game)**; Duo & GP Power chip names/descriptions (TextPet + e-Reader threshold `0x1000`, PASS in-game) and local chip art (`--public` omits); Frequency Tuner (WP-B2); GMD follows Tuner (WP-B4); **Waiting Room Guard A** (`1 <= r6 <= 7`, WP-B6); NPC lists in free space; **on-demand LZ77 sprite decompression** (`wp_d0_sprite_lazy.asm` for Higsby `0x08` & Sal `0x2C`, PASS in-game); 2 verified `code_00` veneer pools (`sub_0800A144`, `sub_08001784`) | PASS in-game (`71d149e`, `patch-test-results-6.md`) |
| RE | Save, flags, Souls, shops, Mystery Data, encounters, tournaments, free space, every save slot, veneers, props, BattleModeFlags, Duo rank | `docs/re/`, `decomp/notes/` |

## Order of the remaining work

Owner decisions marked ⚠ come first.

| # | Task | Depends on / notes |
|---|---|---|
| 1 | ⚠ **Tier A leftovers & QoL:** story-battle rewards, ToyRobo, cutscene skip, and any remaining non-Soul boss rematches | Saved for later in [`docs/pending/tier-a-leftovers-and-qol.md`](docs/pending/tier-a-leftovers-and-qol.md) |
| 2 | ⚠ **Public-release chip art (`--public`):** placeholder/original art for Duo & Grand Prix Power | Local `ref/chip-art/` is Capcom art (gitignored); `--public` currently omits it |
| 3 | **Full-playthrough testing (human/long-run C1–C5)** on the PC port (RS & BM) | Automated 16/16 RS & BM C1–C5 matrix **PASS** (`patch-test-results-7-c1-c5.md`); full continuous playthrough still to run |
| 4 | **Ports:** PC-2 (PET screen tabs, touch Custom Screen, net map) → Wii U (GX2/GamePad) → New 3DS → DSi | **EL-0 done (`c934453`)**; hardware confirmed (**Wii U, New 3DS, DSi**); **GPL-3.0 OK** (`D-EL1`/`D-EL3`). ⚠ Only `D-EL5` (GBARunner licence for DSi/NDS) remains |
| 5 | **Decomp, long term:** carve mixed tables once decoded, reduce version blocks, DC-5 C | Ongoing, between steps |
| 6 | **The collection:** BN1, 2, 3, 5 and 6 decomps → the MMBN Principia Trilogy (ports only) | After BN4 (Principia Trilogy design deferred) |

## Owner decisions still open

- [ ] Tier A leftovers ([`docs/pending/tier-a-leftovers-and-qol.md`](docs/pending/tier-a-leftovers-and-qol.md), saved for later), Tier C content in 1.0, and D1–D9 ([`plan/00` §4](plan/00-decisions-and-feasibility.md)).
- [ ] Placeholder/original Duo & Grand Prix Power art for `--public` builds.
- [ ] Ask Gericom about the GBARunner licence for DSi/NDS (`D-EL5`, [`plan/emulation-layer/06`](plan/emulation-layer/06-milestones-and-risks.md)). *(Decided 2026-11-25: RS/BM = **Option C** two complete patches; hardware `D-EL1` = Wii U, New 3DS, DSi; ports licence `D-EL3` = GPL-3.0 OK; Principia Trilogy = later.)*

## Before committing

- [ ] `git diff --check`; both forced byte-match builds (decomp); `python patch/build.py --check` (patch); coverage when the asm changes.
