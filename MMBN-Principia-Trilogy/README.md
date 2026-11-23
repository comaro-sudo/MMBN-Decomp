# MMBN Principia Trilogy

**Status: vision and plan only (owner decision, 2026-11-23).** No code yet. It depends on the BN1, BN2 and BN3 decomps, which haven't started; the collection's order is BN4 first, then BN1, 2, 3 (see the root [README](../README.md) and [AGENTS.md](../AGENTS.md) §0).

## 1. The goal

*Mega Man Battle Network* 1, 2 and 3 become **one big, continuous game**: one engine, one save, one story from the WWW's first attack through the end of the Gospel and WWW arcs. All three games' content goes in: areas, scenarios, chips, viruses, Navis and bosses.

The owner chose **plan B, a true merge**, over plan A (an anthology: three separate games in one ROM with a chapter menu, which would have been far less work). This document plans plan B.

Like *Violet Eclipse*, it's built on the player's machine from **their own three ROMs**. Nothing from the games is distributed.

## 2. Why this is hard (and what makes it possible)

| Difficulty | Detail |
|---|---|
| **Three engines, not one** | BN2 and BN3 share an engine lineage, and BN3's is the most complete (Navi Customizer, style changes, the largest chip set). **BN1's engine is much older**: different battle code, chip ids, folder rules and save format, and no Navi Customizer. |
| **Content must be converted, not copied** | BN1's and BN2's maps, scripts, chips and enemies must be rewritten into BN3's formats. The same chip has different ids and stats across the games, and some mechanics differ (BN1's chip codes, BN2's styles, BN3's Navi Customizer). |
| **One story** | The three stories were written as sequels, with resets between them (MegaMan's power, Lan's progress). A merge must decide what carries over and how the later games' early chapters still make sense. |
| **Size** | Each game is an 8 MiB ROM; the GBA limit is 32 MiB. Merged content without duplication should fit, but that must be measured. |

**What makes it possible:** full decomps. Once each game builds from symbolic, understood source (as BN4 does now), its content can be extracted into structured data and converted by tools. The *Violet Eclipse* work proved the method: version merging, table carving, symbol maps across versions.

## 3. Architecture (proposed)

- **The base engine is BN3's** (proposal, owner to confirm). It's the latest and most capable of the three, and BN2's content is the closest to it.
- **Content pipeline:**
  1. Decompile each game to the point where its data tables (chips, enemies, encounters, shops, Mystery Data, maps, scripts) are carved and named.
  2. Write converters (Python) that read BN1/BN2 data from the player's ROMs and emit BN3-engine data: chips mapped to BN3 records, BN1/BN2 viruses ported to BN3's enemy system, maps and scripts rewritten to BN3's script commands.
  3. Build one ROM from BN3's source plus the converted content, generated at build time from the player's three ROMs.
- **Mechanics that exist only in one game** (e.g. BN2's styles vs BN3's styles, BN1's battle quirks) are kept, adapted or dropped case by case, decided per mechanic.
- **Targets:** the GBA ROM if it fits in 32 MiB, and every port via the emulation layer (as for *Violet Eclipse*). If the merged game doesn't fit 32 MiB, the ports can still run it (a larger-than-GBA image is a port-only build).

## 4. Phases

| Phase | Work | Done when |
|---|---|---|
| **P0** | BN1, BN2 and BN3 decomps to matching builds, with their data tables carved and named (the same milestones as BN4's DC-0 to DC-4) | All three build byte-identical from source |
| **P1** | Content inventory of all three games: every chip, enemy, area, scenario, boss | One merged list, with each item's source game and its BN3-engine equivalent |
| **P2** | The story design: one continuous script, what carries over between arcs, how the games' openings connect | An owner-approved story outline |
| **P3** | Converters: chips, enemies and encounters first, then maps and scripts | BN1's and BN2's battles play in BN3's engine |
| **P4** | The merged world: all areas reachable, scenarios connected in order | A full playthrough from start to end |
| **P5** | Balance, completion criteria (every chip, every boss, as in *Violet Eclipse*), testing, ports | Release criteria met |

## 5. Risks

| Risk | Mitigation |
|---|---|
| The scale: this is closer to making a new game than to a patch | Strict phases; P3 proves the converters on battles before any world-building |
| BN1's engine differences are too large to convert cleanly | Rebuild BN1's content in BN3's engine by hand where converters fail; accept BN3's mechanics for BN1's content |
| The story merge pleases nobody | P2 is an explicit owner-approved step before content work |
| The size exceeds 32 MiB | Measure in P1; port-only build as the fallback |
| Legal | The same model as every project here: built from the player's own ROMs, nothing distributed |

## 6. Open decisions for the owner

1. The base engine: BN3's (proposed) or another?
2. Which mechanics from BN1 and BN2 must survive, and which can be replaced by BN3's?
3. The story: one continuous arc that removes the resets between games, or three arcs kept as "chapters" of one save?
4. Must it run on real GBA hardware (≤ 32 MiB), or are ports enough?
5. The name: "MMBN Principia Trilogy" (working title from the owner).
