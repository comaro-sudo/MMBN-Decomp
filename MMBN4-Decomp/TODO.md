# Violet Eclipse (MMBN4) TODO

Updated: 2026-11-23 by Gemini (after Items 1–6 patch sprint: Status OAM fix, Z-Saver BMD, WP-B1 tournaments, Free Tournament 36-chip fix, lobby rematches + JunkMan trash can, WP-B2 Frequency Tuner).
The short, execution-oriented backlog. Detailed plans: [`plan/`](plan/README.md). Current handoffs:
[`docs/handoff-violet-eclipse.md`](docs/handoff-violet-eclipse.md) (Claude) and [`decomp/notes/handoff-2026-11-23-gemini-patch-sprint.md`](decomp/notes/handoff-2026-11-23-gemini-patch-sprint.md) (Gemini).
Open owner decisions: [`docs/pending/`](docs/pending/).

## Coordination

- Ownership: **Gemini** owns `decomp/` and this file (Claude may also work in `decomp/`). **Claude** owns `plan/`, `docs/`, `patch/` and `ports/`.
  New game folders (BN1, 2, …, the Principia Trilogy) are Claude's by default.
- Stage by explicit path, never `git add -A`. Never commit ROM bytes.
- [ ] **Push to GitHub:** local `main` is **13 commits ahead** of `origin/main` (plus this update). The owner decides when.

## Order of work (owner request, 2026-11-23)

Each step lists its current status and dependencies. Owner decisions marked ⚠ must be settled before their step.

| # | Status | Task | Why here / depends on |
|---|---|---|---|
| 1 | **Done** (`57ea3b9`) | Merge pending names from Gemini's rounds 3–5 + full regen | Done; small batch of round 6–7 names (`placements.md`, `wp-b1-design.md`, `soul-grant-patch-design.md`, `patch-test-results-2.md`, `special-chip-md-design.md`) left to merge |
| 2 | **Done & PASS** (`d0c64ea`) | **Status screen, two pages of 6 Souls** (`wp_d4_status_two_pages.asm`) | Fixed `dat_0810DABF` overwrite -> moved to `FS_STATUS_OAM` (`0x087FA0A0`); verified 6 icons Page 1 & 5 icons Page 2 on `mmbn4.exe` |
| 3 | **Designed** (`56f83fb`, `bee82f6`) | **Other version's Souls without a trade** (flag + `WriteChipKey`) | Designed in `soul-grant-patch-design.md` (11-byte inline `26`/`66` or `1C`/`3F` scripts, or 28-byte `GrantSoulByIndex`). Owner decided Black Earth gate keeps 6 Souls (**no `checkDoubleSouls` patch**) |
| 4 | **Done & PASS** (`bac8ce9`) | **Deterministic tournament pairs** (WP-B1) | `wp_b1_tournaments.asm`: unified canon pairs `{03,00}, {04,06}, {0A,09}`, disabled pair RNG & PT2 swap, deterministic `BN4_DRAW_BRACKET`, RS Den tutorial -> AquaSoul (`0x21`/`0x16B`) |
| 5 | **Done & PASS** (`90814b8`, `cc019c7`) | **Patch infrastructure for NPCs** (`wp_d0_npc_lists.asm`) | Higsby's `(00, 06)` and ACDC Town `(00, 00)` NPC lists relocated to free space (room for 16); verified PASS in RS & BM |
| 6 | **Pending** | **Tournaments: 9 friendly matches** (story canon pairs already wired in `bac8ce9`) | Needs 3 (Soul grant scripts); lineup (`AquaMan`, `Roll`, `ProtoMan` + 9 friendlies) & approved dialogue in `canon-navis.md` §6–§7 |
| 7 | **Done** (`56f83fb`) | Research: Higsby's walkable grid; Undernet dead end & ACDC corner for special-chip BMDs | All spots & free flags (`0x0D0B`, `0x0E04`) verified in `placements.md` |
| 8 | **Done & PASS** (`ce9d481`) | **Lobby rematches** (9 in Higsby's, 2 on ACDC Town; V1 -> V2 -> V3 -> SP climb + Secret Chips) | `wp_d3_lobby_rematches.asm`: verified GutsMan V1 battle launch from Dex in Higsby's (`mmbn4.exe`); SP+ (`4,095 HP`) tier & final operator sprite polish follow-up |
| 9 | **Done & PASS** (`ce9d481`) | **JunkMan's trash can** outside Higsby's `(-150, -90)` -> `WtrGodComp1 (8E, 00)` | `wp_d3_lobby_rematches.asm`: verified Trash Can jack-in -> `WtrGodComp1` face-to-face with JunkMan (`list[6] 0x5A`) on `mmbn4.exe` |
| 10 | **Partial — Z-Saver DONE** (`7c0f9b8`) | **Special chips:** Z-Saver (`0xDA`) **Done**; Duo (`0x137`) & Grand Prix Power (`0x138`) **Deferred** | `wp_d2_zsaver_md.asm` verified on `mmbn4.exe` (`Undernet 1` BMD `0x0E04`). Duo & GP Power deferred until JP ROM art copy + English names approach is settled |
| 11 | **Done & PASS** (`c9d36cf`) | **Free Tournament fix** for the 36 secret chips | `wp_d4_free_tournament.asm`: bypasses empty Waiting Room gate, missing-chips-first 7-Soul-Navi selector, type-`0x01` Soul Navi battles; verified MegaMan vs Roll V3 on `mmbn4.exe` |
| 12 | **Partial — WP-B2 DONE** | **Frequency Tuner (WP-B2)** **Done**; SP+ roaming ghosts, save protection, remaining boss rematches | `wp_b2_frequency_tuner.asm`: `BN4_ID_RAISE` hook + Higsby's Tuner NPC (`FC 0B 00`, flags `0x0E58..0x0E59`); verified on `mmbn4.exe`. SP+ ghosts & WP-B4 GMD hook remain |
| 13 | **Pending** ⚠ | Tier A leftovers: story-battle rewards, key-item prices, ToyRobo, cutscene skip | Key-item prices need a decision |
| 14 | **Pending** | **Existing-saves load hook** (new-game-copied data such as shop stock) | Last, once everything that lives in the save is known |
| 15 | **Pending** | **Full-playthrough and completion testing** (C1–C5) on the PC port | Validates 2–14 together |
| 16 | **Pending** ⚠ | Ports: EL-0 refactor → PC-2 (PET, touch Custom Screen, net map) → Wii U / 3DS / NDS | ⚠ hardware owned and GBARunner licence |
| 17 | **Ongoing** | Decomp, long term: carve mixed tables once decoded, reduce version blocks, DC-5 C | Ongoing, between steps |
| 18 | **Later** ⚠ | Collection: BN1, 2, 3, 5 and 6 decomps → ⚠ MMBN Principia Trilogy | After BN4 |

⚠ The RS/BM version choice for the merged ROM (`plan/DEFINITIVE-EDITION.md` §7 q1) affects steps 3, 6 and 12; settle it before step 6.

## Current verified state

- **Decomp:** US Red Sun and Blue Moon both rebuild byte-for-byte from **one shared `decomp/asm/` tree** (DC-4).
  - Symbols follow RS addresses; BM data addresses map through `symbols/datashift_rs_bm.txt`.
  - **132 `.if VERSION_*` blocks**; coverage **59.4%** (RS 700,816 / 1,179,456 code bytes).
- **Patch (9 packages built via `python patch/build.py`, needs both ROMs):**
  - **PASS in-game (5):** WP-A2 (battle Zenny × 1.5), WP-D2 (random version drops), WP-D3 (Bass drops both chips), WP-D4 star (12 Souls), WP-D0 (`wp_d0_npc_lists.asm` in RS & BM).
  - **Needs fix (1):** WP-D4 status 2-page (`wp_d4_status_two_pages.asm` — OAM table `dat_0810DABF` overwritten, 3/6 icons missing).
  - **Blocked (1):** WP-D2 Duo chip (`wp_d2_duo_chip_reward.asm` — Duo skips results screen & `0x137` is an e-Reader EWRAM-backed chip).
  - **Built / smoke-tested (2):** WP-A1 (WoodMan fix), WP-D4 twelve-Souls unite table (`wp_d4_twelve_souls.asm`).

## Decomp (DC-2 / DC-3 / DC-5)

- [x] DC-4: one shared RS/BM tree; segments split at true shift points; code- and data-region pointers as symbols.
- [x] DC-2 carving: encounter tables, 4 big and 204 small pointer tables (`db01bda`, `1c8fca7`, `43ad295`).
- [x] DC-3: names merged through Gemini's rounds 3–5 (`11f8a5b`, `57ea3b9`).
- [ ] Merge the names proposed in Gemini's round 6–7 notes (`placements.md`, `wp-b1-design.md`, `soul-grant-patch-design.md`, `patch-test-results-2.md`, `special-chip-md-design.md`) into `symbols/names.txt` and regenerate.
- [ ] Remaining `incrom` is mostly mixed data. Carve each table only once its record layout is decoded, as structured data, never as bulk hex.
- [ ] Reduce the version blocks: `code_02` and `code_05` pointers into `code_04` cutscene scripts need a per-range map of those scripts.
- [ ] DC-5 (later): hybrid C infrastructure, per `plan/decomp/05-milestones-and-risks.md`.

## Reverse engineering

- [x] Save format, flags, playthrough counter, Zenny/BugFrags, Souls, completion stars, New Game+.
- [x] RE-04 tournaments, RE-06 encounters/enemies/drops, RE-07 Mystery Data, RE-09 shops and Number Trader, all save slots, free ROM space.
- [x] Soul grant & full 12-Soul reader audit (`soul-grant.md`, `soul-grant-patch-design.md`).
- [x] Bass rematch feasibility, lobby NPCs & jack-ins, Free Tournament battle data, Status screen 2-page design, BN4 NPC cap (16).
- [x] Exact placements for 9 Higsby's operators, 2 street operators + trash can, ACDC Area 2 & Undernet 1 special-chip BMDs (`placements.md`).
- [x] Exact patch designs for WP-B1 (`wp-b1-design.md`), special-chip BMDs (`special-chip-md-design.md`), and lobby/trash-can/JunkMan NPCs (`lobby-npc-scripts-design.md`).
- [ ] Exact `list[7]` sprite indices for the 8 remaining story operators (Chaud, Mr. Match, Shuko, Lily, Tamako, Sal, Raika, Raoul) + trash-can sprite graphic.
- [ ] E-Reader chip data/art format for `Duo` (`0x137`) and `Grand Prix Power` (`0x138`) in US ROMs.
- [ ] Flag `0x14` (tournaments); other traders (`sub_0804C680`).

## Violet Eclipse patch: to build / fix

- [ ] **Fix `wp_d4_status_two_pages.asm`:** move 6-byte `dat_0810DABF` (`00 10 20 30 40 50`) to `FS_STATUS_SOULS + 0x30`, repoint `ICON_OAM_PTR` (`RS 0x0810DA7C / BM 0x0810DA90`), re-verify all 6 icons on Page 1 & 5 on Page 2.
- [ ] **Resolve Duo (`0x137`) & Grand Prix Power (`0x138`) e-Reader backing** + Duo results-screen skip (`+1 = 0x93` bit 7); build **Z-Saver (`0xDA`)** BMD in Undernet 1 (`special-chip-md-design.md`).
- [ ] **Deterministic tournaments (WP-B1) + 9 friendly matches** (`wp-b1-design.md`, `canon-navis.md` §6–§7, `soul-grant-patch-design.md`).
- [ ] **Lobby rematches** (9 in Higsby's + Higsby counter option, 2 on ACDC street; V1 → V2 → V3 → SP → **SP+** `4,095 HP` / 1.5× atk / 1.25× spd climb; appear after meeting the Navi; `lobby-npc-scripts-design.md`).
- [ ] **JunkMan's trash can** outside Higsby's `(-150, -90)` → cyber map `(8E, 00)` / `(88, 00)` with JunkMan NPC (`list[6] 0x5A`).
- [ ] **SP+ roaming ghosts** for bosses without an operator (`44a31c9`).
- [ ] **Free Tournament fix** for the 36 secret chips (owner decision pending in `docs/pending/free-tournament-secret-chips.md`).
- [ ] Frequency Tuner (WP-B2: `BN4_ID_RAISE` hook), save protection, remaining boss rematches (WP-D3).
- [ ] Tier A leftovers: story-battle rewards, key-item prices (owner decision pending), ToyRobo, cutscene skip ([`plan/03`](plan/03-phase1-rom-overhaul.md)).
- [ ] Existing saves: load hook for anything copied into the save at new game (shop stock).

## Ports

- [x] PC-1: ARM64 Windows app (embedded mGBA + SDL3, PET window, headless `--test`, `--dump`, `--state-in/out`).
- [ ] EL-0: refactor into `ports/common`. Then PC-2: BKPT hooks, PET state, touch Custom Screen, net map (original design inspired by MMBN1 DS / MMBN5 DS, no copied code or assets).
- [ ] Wii U, New 3DS and NDS routes ([`plan/emulation-layer/`](plan/emulation-layer/README.md)).

## Collection

- [ ] BN1, BN2, BN3, BN5 and BN6 decomps, after BN4. The second game moves shared tools/emulators to the repo root (`AGENTS.md` §4).
- [ ] [MMBN Principia Trilogy](../MMBN-Principia-Trilogy/README.md): BN1–3 merged into one game, for ports only (needs BN1–3 decomps first).

## Owner decisions still open

- [ ] How to handle **Duo (`0x137`) & Grand Prix Power (`0x138`)** being e-Reader EWRAM-backed chips in US ROMs (populate their chip data/art in ROM, or drop/replace them?).
- [ ] Use the Free Tournament fix for the 36 secret chips? (`docs/pending/free-tournament-secret-chips.md`)
- [ ] RS/BM version choice in the merged ROM (`plan/DEFINITIVE-EDITION.md` §7 q1). The canon lineup assumes one merged story.
- [ ] Key-item prices fixed or as in-game options; Tier C content in 1.0; D1–D9 ([`plan/00` §4](plan/00-decisions-and-feasibility.md)).
- [ ] Hardware owned for the console ports; the GBARunner licence ([`plan/emulation-layer/06`](plan/emulation-layer/06-milestones-and-risks.md)).
- [ ] Principia Trilogy: base engine, surviving mechanics, how the story joins, name.

## Before committing

- [ ] `git diff --check`; both forced byte-match builds; `python patch/build.py --check`; coverage when the asm changes.
