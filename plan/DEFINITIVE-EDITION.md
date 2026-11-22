# Project Goal: *Mega Man Battle Network 4: Violet Eclipse* on Every Port

*Set by the project owner, 2026-11-22. Name chosen by the owner, 2026-11-22: **Violet Eclipse** (Red Sun + Blue Moon → violet). It replaces the working title "Solar Eclipse" in [docs/design/design-document.md](../docs/design/design-document.md).*

> **Every port (PC, Wii U, 3DS, NDS, plus the patched GBA ROM) ships *Violet Eclipse*, the definitive version of MMBN4. In one save file, the player can get every Double Soul, collect every chip, and fight every boss.**

This goal outranks the rest of the plan. Phase 1 work packages, the emulation-layer ports and the decompilation are all judged by whether they move toward it.

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

Red Sun and Blue Moon each contain only their own exclusives. The fingerprint survey shows **79% of code bytes are identical** (5 shift regions), so the difference is mostly **data** (graphics, Navi records, scripts, chip availability). Three ways to get both versions' content into one game:

| Option | How | Works on | Cost |
|---|---|---|---|
| **A. Merged ROM** (*Violet Eclipse*, recommended) | One patched ROM built from Red Sun that **also contains Blue Moon's exclusive data** (Navi graphics, AI, records, Soul data, scripts), added in expanded ROM space (8 → 16 MB; GBA max 32 MB). A version choice at New Game, or both sets of exclusives in one story | **Every port, and GBA hardware** | Highest: needs the BM-only data identified and relocated. The disassembly's RS/BM variant work (DC-4) is exactly that inventory |
| B. Two-ROM runtime (Cyberworld Endless model) | The director loads both ROMs and pulls the other version's content in at runtime | PC / Wii U / 3DS only. **Not NDS, not GBA hardware** | Medium, but breaks C5 |
| C. Two patched ROMs, each complete | Each version patched to include the other's exclusives | Every port | About the same as A, twice the testing |

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

1. **Version choice:** in the definitive ROM, does the player still pick Red Sun or Blue Moon (story and rival differ), with the other version's exclusives added as extra content? Or should both stories merge into one?
2. **Stronger Navi tiers and post-game:** keep the original requirements (they are part of the challenge) as long as they're reachable in one save?
3. **Dark Chips and Karma:** "every chip" includes Dark Chips, and using them changes Karma. Should collecting them be possible without being forced into Dark Karma?
