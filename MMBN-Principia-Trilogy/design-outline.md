# Principia Trilogy: design outline

Written 2026-11-27. Expands [README.md](README.md) (plan B: BN1, BN2 and BN3 merged into one game on the BN3 engine, ports only). Nothing here is decided: owner decisions are listed in §8 as questions, and where a section names a "proposal" it is the README's or a default to be confirmed.

Facts about the games' internals below are **not yet verified**: no BN1-3 decomp exists. Statements about game content come from general knowledge of the series and are marked "to verify". Each is checked in P1 (content inventory) against the player's own ROMs.

## 1. Premise and scope

One game, one save, one continuous story from BN1's opening to the end of BN3. Built at build time from the player's own three ROMs; nothing from the games is distributed. Ports only (PC, NDS, 3DS, PSP, PS Vita, Wii U, Android); no real-GBA requirement.

### Scope per game

| Source | What goes in | Main conversion work | Main difficulty |
|---|---|---|---|
| **BN1** | Its story arc, areas, viruses, Navis and bosses, chip set, items | Everything: oldest engine, different battle code, chip ids, folder rules, save format; no Navi Customizer | Largest conversion; BN1 mechanics that BN3 lacks must be kept, adapted or dropped |
| **BN2** | Its story arc, areas, viruses, Navis and bosses, chips, style system content | Closer to BN3 (same engine lineage, to verify): data converts, scripts and maps are rewritten | BN2's style system versus BN3's styles; overlapping content with BN3 |
| **BN3** (White or Blue) | The base engine and its content: arc, areas, chips, Navi Customizer, styles | Least conversion; the engine is the host | White vs Blue: one version, both merged (as Violet Eclipse did for BN4), or a choice. This is an owner question |

Content classes inside each game (the P1 inventory): areas and maps, scenarios and events, scripts and dialogue, chips (and codes), viruses and encounter tables, Navis and bosses, shops and Mystery Data, items and key items, music and sprites referenced by the above, save-persisted state (flags, counts, unlocks).

## 2. Engine baseline

Proposal in the README: BN3's engine, because it is the latest of the three and the most complete. Open points:
- BN3 exists in two versions (White, Blue) that differ in exclusive content (to verify the exact list in P1). The same RS/BM problem BN4's Violet Eclipse solved by a **two-version merge** applies here; the BN4 pipeline (a shared tree with `.if VERSION_*` blocks) is a model.
- "BN3's engine" means a decomp of BN3 that builds, with data tables carved and named, before any conversion. The converters write into BN3's formats, so BN3's data formats must be fully understood first.

## 3. Saves and progression

### The core problem

The three games are sequels with partial resets between them (to verify: which stats, chips and items reset or carry over in the originals). A merged game has one save. Options, listed not chosen:

| Model | How progression works | Consequence |
|---|---|---|
| **A. True continuity** | One progression: BN1's chapter, then BN2's, then BN3's; the player's HP, chips, Navi power and items carry through, scaled so later content stays challenging | Requires rebalancing every enemy and boss; a story that explains the no-resets; most work |
| **B. Chapters with soft resets** | Three arcs in one save; at each chapter boundary some things carry over (chips collection, Zenny, library) and some reset (HP, levels) as the originals did | Closer to the originals; simpler balance; "one story" is looser |
| **C. Parallel unlocks** | The world is open from the start in a hub, arcs unlock by progress, with a shared collection | Needs a new world structure; furthest from the originals |

The README's open question 3 is this choice. Everything below has to work for whichever is picked.

### What a unified save must record

- **Player state:** name, HP, Navi stats, folder, pack, items, Zenny, BugFrags (BN2, to verify), library, mail, key items, party/Navi data. Each field's source game and meaning must be unified, e.g. one chip id space (see §5).
- **Story flags:** one flag space for all three arcs. BN4's save-and-flags notes (`MMBN4-Decomp/docs/re/save-and-flags.md`) show the shape of that work. Flag collisions are impossible by construction only if the converters assign ids from one allocator.
- **Per-game systems:** Navi Customizer layout (BN3), styles (BN2/BN3), BN1's mechanics if kept.
- **Save format and checksum:** the merged game defines its own; the BN3 engine's save routine is the base and its size limits (SRAM, to verify) bound what fits. With ports only, the format is free to be larger than a GBA cartridge's save, but only if the engine's save code is rewritten or the emulation layer provides more storage. Owner/engineering decision.
- **Imports:** importing existing saves from the three original games is a possible feature. It needs each game's save format decoded (a decomp output) and a mapping to the merged state; it is optional and not assumed.
- **Versioning:** a save-format version field from day one, so later fixes do not orphan saves.

## 4. What each game's decomp must provide (inputs to the converters)

| Need | BN1 | BN2 | BN3 |
|---|---|---|---|
| Byte-matching build from the player's ROM | yes (P0) | yes (P0) | yes (P0) |
| Chip table: id, name, element, damage, codes, MB, effect id, icon refs | decoded as data | decoded as data | decoded as data (target format) |
| Enemy and virus data: stats, AI id, drops, sprites refs | decoded | decoded | decoded |
| Encounter tables per area | decoded | decoded | decoded |
| Map data: tilemaps, collision, warps, props | decoded | decoded | decoded |
| Script bytecode: opcode set, the command semantics, text format | disassembler for the script VM | same | same, plus the VM spec as the target |
| Dialogue text and its encoding (TextPet-like) | extract/insert | extract/insert | extract/insert |
| Battle engine: how a chip effect, an enemy AI and a boss pattern run | enough to re-implement BN1-only behaviour | enough to re-implement BN2-only behaviour | the host; fully understood |
| Save layout and flag meanings | needed | needed | needed (target) |
| Shop and Mystery Data tables | decoded | decoded | decoded |
| Sprite, palette and music references (not their bytes in git) | index only | index only | index only |

Everything extracted is read from the player's ROMs at build time; the repository holds only code, converters and structured *descriptions* of formats, never the game data (the rule that governs `MMBN4-Decomp/`).

The BN1 plan is [../MMBN1-Decomp/plan/README.md](../MMBN1-Decomp/plan/README.md). BN2 and BN3 plans do not exist yet; they need the same kind of bootstrap plan, and BN3's is the most critical of the three since it is the host.

## 5. Conversion rules (to be designed)

- **One id space** for chips, items and flags, assigned by an allocator in the converter, with a published mapping table per game.
- **Duplicates:** the same chip across games (same name, different stats/ids) become one chip or distinct variants? Same for recurring viruses and Navis. Needs a rule and a table; an owner question.
- **Mechanics present in only one game** (e.g. BN1 quirks, BN2 styles): keep, adapt or drop, case by case, recorded in a decision log.
- **Scripts and maps** are rewritten by converters where the formats are close, by hand where they are not.
- **Progression-gated content** (items that open doors, level caps, key chips): re-gated against the unified progression model chosen in §3.

## 6. Phases

As in the README (P0 to P5). This outline adds exit criteria for P0 and P1:
- **P0 done** when the three decomps build byte-identical and each exposes the tables in §4.
- **P1 done** when every item in every content class has a row in one inventory: source game, source id, BN3-engine equivalent or "to build", and a conversion method (auto, hand, drop).

The ports' emulation layer or native route is a P5 matter; the NDS memory ceiling should be benchmarked in P1 because a bigger-than-GBA image is only useful if the target can load it.

## 7. Risks

| Risk | Why it matters | Mitigation |
|---|---|---|
| Scale | Closer to a new game than a patch | Strict phases; prove converters on battles before the world |
| BN3 is the host and must be fully understood first | A shallow BN3 decomp blocks everything | Prioritise BN3's decomp quality over speed; consider ordering BN3 before BN2 and BN1 if the owner wants Principia early |
| Order conflict | Collection order is BN4, 1, 2, 3, but the host is BN3 | Owner question below |
| BN1 engine gap | Conversion may not be mechanical | Rebuild BN1 content by hand where needed; accept BN3's mechanics for it |
| Story merge | Three resets, three tones | P2 owner-approved story outline before content work |
| Balance | Three difficulty curves in one | Models in §3; playtest with test saves, as in BN4's QA plan |
| Save size and format | One save must hold the union of the games' state | Design the format early in P1; version it |
| Memory/speed on NDS | Weakest target | Benchmark early; the owner already named NDS as a target |
| Legal | Same model as everything here | Build from the player's own ROMs; no game data in git; confirm licence of any imported community code |
| Effort versus finishing BN4 and the other games | Competes with BN5 and BN6's decomps | Principia is later than BN1-3; it must not block the collection |

## 8. Questions the owner must answer

Not decided here.

1. **Base engine:** confirm BN3's. And BN3 White, Blue, or both merged (as Violet Eclipse did for BN4)?
2. **Progression model (§3):** true continuity, chapters with soft resets, or parallel unlocks? What carries over between arcs and what resets?
3. **Mechanics to keep:** which BN1 and BN2 mechanics must survive (BN1's battle quirks, BN2's styles and their relation to BN3's styles)?
4. **Duplicates:** when a chip, virus or Navi appears in several games, one merged entry or distinct variants?
5. **Order of work:** keep BN4, 1, 2, 3 (BN3 last, though it is the host), or decomp BN3 earlier for the Principia's sake?
6. **Saves:** should original saves from BN1-3 be importable? Is a larger-than-GBA save acceptable?
7. **Completion goal:** "every chip, every boss" as in Violet Eclipse, across all three games?
8. **Story:** who writes the connective story, and how much of the original dialogue is rewritten versus kept?
9. **Ports:** which ports are required for a first release (the README lists seven; PSP, PS Vita and Android are new targets with no plan)? Which is the lead port?
10. **Licensing:** the licence of the converters and tools (the BN4 ports use GPL-3.0, `D-EL3`). Same here?
11. **Name:** is "MMBN Principia Trilogy" final?
12. **Scope guard:** should the first milestone be a vertical slice (one area from each game, one save) before committing to the full merge?
