# Project Goal: *Mega Man Battle Network 4: Violet Eclipse* on Every Port

*Set by the project owner, 2026-11-22. Name chosen by the owner, 2026-11-22: **Violet Eclipse** (Red Sun + Blue Moon → violet). It replaces the working title "Solar Eclipse" in [docs/design/design-document.md](../docs/design/design-document.md).*

> **Every port (PC, Wii U, 3DS, NDS, plus the patched GBA ROM) ships *Violet Eclipse*, the definitive version of MMBN4. In one save file, the player can get every Double Soul, collect every chip, and fight every boss.**

This goal outranks the rest of the plan. Phase 1 work packages, the emulation-layer ports and the decompilation are all judged by whether they move toward it.

## Status (2026-11-27)

What is built and tested, from [`patch/README.md`](../patch/README.md) (each package's test status) and the Done table in [`TODO.md`](../TODO.md). Test evidence: `decomp/notes/patch-test-results*.md` (Gemini). **Built and tested** = PASS in-game on the PC port, RS and BM unless noted.

| Criterion | State | Built and tested | Open |
|---|---|---|---|
| C1 Souls | **Built, tested** | All 12 Souls unite in both versions (WP-D4); Duo Soul as the 13th, granted by beating Duo; fixed tournament pairs and the Free Tournament fielding the 12 Soul Navis (WP-B1, WP-D4); 9 friendly matches grant Souls; 3-page Status screen; completion star needs 12 | Black Earth gate keeps 6 Souls (owner: no patch) |
| C2 Chips | **Mostly built** | Random RS/BM drops (WP-D2); Bass gives both chips (WP-D3); Free Tournament covers the 36 secret chips; Z-Saver and Grand Prix Power in Blue Mystery Data; Duo and GP Power names and descriptions; shop prices cut (WP-A3) | Duo and GP Power picture in `--public` builds (owner); chips from other event, code and accessory sources: recheck against [`docs/completion/chips.md`](../docs/completion/chips.md) |
| C3 Bosses and rematches | **Built, tested** for Souls' Navis, Duo and SP+ | 12 lobby operators + 10 roaming SP bosses, rematch climb V1 -> V2 -> V3 -> SP -> SP+ on S ranks; Duo rematch and the Duo story grant; SP+ difficulty (4,095 HP, ATK x1.5, Speed x1.25) | Tier A leftovers: story-battle rewards, ToyRobo, remaining non-Soul boss rematches ([`docs/pending/tier-a-leftovers-and-qol.md`](../docs/pending/tier-a-leftovers-and-qol.md)) |
| C4 No missables | **Partly** | Frequency Tuner (WP-B2) and Green Mystery Data by tier (WP-B4); Waiting Room guard (WP-B6, bounds check only) | **Persistent Mystery Data (WP-B3) is not built**; no completion test from a late save beyond the automated matrix |
| C5 Same on every port | **Open** | The content is in the ROM, so any port that runs the patched ROM has it. PC (Windows ARM64 and x64) and Wii U (in Cemu) boot and bench the patched ROM | Real Wii U `bench.log` (gate G-WU); New 3DS source written but not compiled; DSi/NDS not started (GBARunner licence, `D-EL5`) |
| Verification (§5) | **Partly** | Automated C1-C5 matrix 16/16 PASS (RS and BM); `build.py --check` | A full continuous playthrough per version; the same test on each port; re-run on the `--public` release build; a committed smoke test is in progress |

Also built: the WoodMan fix (WP-A1), Zenny x1.5 (WP-A2), on-demand sprite loading and NPC lists in free space (infrastructure). Release work (licence, public art, playthrough, hardware tests) is in [`docs/release-checklist.md`](../docs/release-checklist.md).

**Where each owner decision is recorded**

| Decision | Date | Recorded in |
|---|---|---|
| Goal and the name *Violet Eclipse* | 2026-11-22 | This file (top) |
| Cross-version approach: Option A (merged data in one ROM), delivered as **two complete patches, one per ROM (Option C)** (answers §7 q1) | 2026-11-25 | [`docs/pending/owner-decisions-2026-11-24.md`](../docs/pending/owner-decisions-2026-11-24.md); [`docs/release-checklist.md`](../docs/release-checklist.md) |
| Version drop codes and the Bass order (§7 q4) | 2026-11-23 | §7 below; [`docs/completion/bosses.md`](../docs/completion/bosses.md) §3 |
| Duo Soul as the 13th Soul | 2026-11-23 | [`docs/pending/duo-soul.md`](../docs/pending/duo-soul.md) (built) |
| Duo rematch climb on S ranks; key-item prices at 40% (WP-A3); Waiting Room guard A only | 2026-11-24 | [`docs/pending/owner-decisions-2026-11-24.md`](../docs/pending/owner-decisions-2026-11-24.md) |
| Hardware owned (`D-EL1`), ports licence GPL-3.0 (`D-EL3`), Principia Trilogy deferred | 2026-11-25 | [`docs/pending/owner-decisions-2026-11-24.md`](../docs/pending/owner-decisions-2026-11-24.md); [`plan/emulation-layer/06-milestones-and-risks.md`](emulation-layer/06-milestones-and-risks.md) |
| Open: patch licence; Duo/GP Power art for `--public`; Tier A leftovers; §7 q2 and q3; GBARunner licence (`D-EL5`) | n/a | [`docs/pending/`](../docs/pending/), [`TODO.md`](../TODO.md) "Owner decisions still open" |

## 1. Completion criteria (testable)

A port is "definitive" when a single save can reach all of these, without trading, link play, a second cartridge, event distributions or repeating the game three times:

| # | Criterion | How it's checked |
|---|---|---|
| C1 | **Every Double Soul** is obtainable, including those exclusive to the other version (Red Sun's and Blue Moon's) | Soul flags in the save (RE-05), all set |
| C2 | **Every chip** in both versions' chip tables is obtainable in-game: standard, Mega, Giga (both versions'), Dark Chips, Program Advances, and chips originally limited to events, codes, trading or accessories | Library/pack flags = full inventory (generated from the chip tables, §2) |
| C3 | **Every boss can be fought**: every Navi of both versions in every version/tier the game has (V1–V3 and any stronger variants), Duo, and the post-game bosses, with a way to **rematch** them after the story | Boss list (§2), each with a reachable encounter in a test save |
| C4 | **No missables**: everything above stays obtainable after any story point, and nothing is lost through New Game+ | Persistent Mystery Data (WP-B3), tier toggle (WP-B2), a completion test from a late save |
| C5 | **Same content on every port.** Platform extras (PET screen, touch, save states) may differ; content may not | Same completion test on each port (§5) |

## 2. Content inventory (first task)

The completion lists are **generated from the ROMs, not written from memory**:

- **Chips:** both chip tables (verified byte-identical with `mmbn-chip-tables`, about 389 entries each), merged and deduplicated. Flag which ones each version can actually obtain (GMD tables, shops, traders, battle drops, scripts) and which have no in-game source.
- **Souls:** from the Soul-grant logic and flags (RE-05), per version.
- **Bosses:** from the enemy/Navi records and encounter tables (RE-06; Cyberworld Endless maps BN4's enemy records), per version and tier.
- Output (started 2026-11-22): `patch/tools/inventory.py` writes `build/inventory/chips.csv` (item, kind, codes per version, sources, status). The CSV stays local because it holds game text and third-party facts. The conclusions are tracked in [docs/completion/](../docs/completion/README.md). This gives C1–C3 their exact contents.

## 3. What blocks completion today, and the fix for each

| Blocker in the original game | Fix | Where it lives |
|---|---|---|
| Souls depend on which opponents a playthrough draws; three runs needed | Single-run Soul acquisition, fixed brackets (WP-B1) | ROM patch |
| **Version-exclusive Souls, Navis, chips** | **Cross-version content** (§4): the other version's Souls, Navis and chips become reachable in one game | §4 decision |
| Virus/GMD tiers locked to playthrough count | Frequency Tuner (WP-B2), GMD by tier (WP-B4) | ROM patch |
| Missable Blue/Purple Mystery Data | Persistent Mystery Data (WP-B3) | ROM patch |
| Chips from trading, link battles, events, codes or accessories | Add in-game sources: shops, traders, Number Trader codes, BBS requests, boss drops | ROM patch (data + scripts) |
| Bosses only fought once (story) | **Rematch system**: a boss-rush/arena or BBS challenges for every Navi and tier, post-game included | ROM patch (scripts + encounter data); a PET menu on director ports can launch it too |
| Waiting Room can overwrite Soul data | WP-B6 | ROM patch |

**Rule:** completion content goes in the **ROM patch**, not in the director. The NDS port (hypervisor) and real GBA hardware can only run what's in the ROM, and C5 requires the same content everywhere. The director adds convenience, such as a PET checklist showing what's still missing.

## 4. The cross-version decision (must be made early)

*Update (RE-06, 2026-11-22):* both ROMs already contain **every** Navi's records, battles and roaming-ghost tables, and these are identical in RS and BM. What a version lacks is the game state that fields the other version's Navis (tournament pools, flags, scripts), plus 64 drop rows with version-specific codes ([docs/completion/bosses.md](../docs/completion/bosses.md)). So Option A needs less imported data than assumed below: mostly Soul data, graphics and scripts still to be checked.

Red Sun and Blue Moon each contain only their own exclusives. The fingerprint survey shows **79% of code bytes are identical** (5 shift regions), so the difference is mostly **data** (graphics, Navi records, scripts, chip availability). Three ways to get both versions' content into one game:

| Option | How | Works on | Cost |
|---|---|---|---|
| **A. Merged ROM** (*Violet Eclipse*, recommended) | One patched ROM built from Red Sun that **also contains Blue Moon's exclusive data** (Navi graphics, AI, records, Soul data, scripts), added in expanded ROM space (8 → 16 MB; GBA max 32 MB). A version choice at New Game, or both sets of exclusives in one story | **Every port, and GBA hardware** | Highest: needs the BM-only data identified and relocated. The disassembly's RS/BM variant work (DC-4) is exactly that inventory |
| B. Two-ROM runtime (Cyberworld Endless model) | The director loads both ROMs and pulls the other version's content in at runtime | PC / Wii U / 3DS only. **Not NDS, not GBA hardware** | Medium, but breaks C5 |
| C. Two patched ROMs, each complete | Each version patched to include the other's exclusives | Every port | About the same as A, twice the testing |

**Update 2026-11-22: much of A's data is already in both ROMs.** Both versions carry all twelve Souls' code, tables and sprites, and the chip table is identical. A Blue Moon Soul runs in Red Sun once a 12-byte table is filled ([docs/re/soul-merge-experiment.md](../docs/re/soul-merge-experiment.md)). Red Sun also fields Blue Moon's Navis in tournaments when it has their traded data ([docs/re/tournaments.md](../docs/re/tournaments.md)). What still has to move or be written is the grants, the encounters and the version-specific story data.

**Recommendation: A.** It is the only option that satisfies C5 on NDS and hardware. The decompilation stops being optional: DC-1 (done: RS/BM map), DC-2/DC-3 (names) and DC-4 (BM variant = the list of what differs) feed the merge directly.

Legal note: like everything else, the merged ROM is produced on the player's machine from **both** of their own ROMs (patch + build script). Nothing from either ROM is distributed.

## 5. Verification

- **Completion save test:** a scripted run (headless `--test`/bench modes on PC; the same scripts on console debug builds) from a late-game save that checks every flag in the inventory can be set, and every boss encounter loads. It runs on every port before a release.
- **Inventory diff:** after any patch change, regenerate `inventory.csv` and confirm no item lost its in-game source.

## 6. What changes in the plan

| Area | Change |
|---|---|
| [03 Phase 1](03-phase1-rom-overhaul.md) | New work packages: **WP-D1** content inventory, **WP-D2** in-game sources for every chip, **WP-D3** boss rematch system, **WP-D4** cross-version merge (Option A). WP-B1 now aims at *all* Souls of both versions |
| [decomp](decomp/README.md) | DC-4 (Blue Moon variant) becomes the cross-version diff for WP-D4. Priority up |
| [emulation-layer](emulation-layer/README.md) | Every port loads the definitive ROM. PET gets a completion checklist screen |
| [09 milestones](09-milestones.md) | "Definitive" release criteria = C1–C5 |

## 7. Open questions for the owner

1. ~~**Version choice**~~: **decided 2026-11-25: Option C, two complete patched ROMs** (see Status above). Original question: in the definitive ROM, does the player still pick Red Sun or Blue Moon (story and rival differ), with the other version's exclusives added as extra content? Or should both stories merge into one?
2. **Open.** **Stronger Navi tiers and post-game:** keep the original requirements (they are part of the challenge) as long as they're reachable in one save?
3. **Open.** **Dark Chips and Karma:** "every chip" includes Dark Chips, and using them changes Karma. Should collecting them be possible without being forced into Dark Karma?
4. ~~**Version drop codes**~~: **decided by the owner, 2026-11-23** ([docs/completion/bosses.md](../docs/completion/bosses.md) §3).
   - **Rule (a):** every drop that differs between versions (61 virus families, Bass, BassΩ, BassXX) gives the Red Sun or the Blue Moon item **at random, each time it drops**.
   - **Bass can be fought again and again**, so the player can collect all four Bass chips (Bass, BassAnly, BlakBarr, DeltaRay). This is part of the rematch system (WP-D3).
   - **Order for each Bass form** (owner, 2026-11-23; this replaces the orders written earlier):
     - **Main:** the first fight **drops both chips**.
     - **Fallback 1:** the fight drops one chip **plus an Unlocker** SubChip, and a **purple** Mystery Data nearby holds the other chip.
     - **Fallback 2:** after the first fight, a **blue** Mystery Data (Unlocker) and a **purple** Mystery Data (the other chip) appear nearby.
     - **Fallback 3:** that Bass form **roams the same area** and can be fought again.
   - **Earlier notes on the fallbacks** (superseded by the order above where they differ):
     - If Bass (V1) doesn't appear again after the story battle, make him roam Black Earth 2 like BassΩ. If that isn't possible, his story battle drops **both** Bass and BassAnly.
     - If BassΩ doesn't come back after a win, he drops **both** BlakBarr and DeltaRay. The same goes for BassXX.
     - If "drops both" is too hard or impossible: after the battle, a **Purple** Mystery Data appears nearby in Black Earth 2 with the chip the battle didn't give. The battle gives an **Unlocker** SubChip to open it. Fallback, if the battle can't give one: a **Blue** Mystery Data holding an Unlocker is pinned next to the purple one, where the player fights that Bass form. **Every win against Bass re-arms both Mystery Data.** If that can't be done, make sure the game supplies enough Unlockers for every purple Mystery Data (the 14 originals plus the new Bass ones).
