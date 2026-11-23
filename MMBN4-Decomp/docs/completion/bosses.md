# Bosses (criterion C3)

**Preliminary.** This list is derived from the Navi chips: each Navi chip is earned by beating that Navi, so it gives the set of Navi fights and their tiers. The authoritative list will come from the ROM's enemy and Navi records and encounter tables (RE-06; CE already maps BN4's enemy records).

## Navis and tiers

Each Navi appears as a story fight (V1) and as stronger ghost versions roaming the net: V2, V3 and SP, with an S-rank chip from SP. The DS versions are in Black Earth 2 (post-game).

| Group | Navis | Tiers | Version |
|---|---|---|---|
| Soul Navis, Red Sun | Roll, GutsMan, WindMan, SearchMan, FireMan, ThunderMan | V1–V3, SP, DS | Red Sun only |
| Soul Navis, Blue Moon | ProtoMan, NumberMan, MetalMan, JunkMan, AquaMan, WoodMan | V1–V3, SP, DS | Blue Moon only |
| Shared Navis | TopMan, ShadeMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan | V1–V3, SP, DS (BurnerMan's DS chip has no listed source) | Both |
| Bass | Bass (Giga chip), BassAnly (Giga) | Black Earth 2 | Bass in Red Sun, BassAnly in Blue Moon |
| Duo | Final boss | Story tiers per playthrough | Both |

That makes 8 shared and 12 version-split Navis, each with up to 5 tiers: about **100 Navi fights**, plus Bass and Duo.

## What blocks completion

| Blocker | Fix |
|---|---|
| Six Navis per version are missing | Merged ROM (WP-D4): the other version's Navi data and its ghost encounters |
| Tiers V2/V3 depend on the playthrough count, and some Navis are only fought in tournaments | Frequency Tuner (WP-B2): the tier readers listed in [`../re/save-and-flags.md`](../re/save-and-flags.md) §3 |
| No way to fight a boss again after the story | Rematch system (WP-D3): a boss-rush arena or BBS challenges for every Navi and tier |
| The WoodMan bug | WP-A1: Prof. 9's fix, mechanism confirmed in both ROMs |

## Next

- RE-06: enemy records and encounter tables per area and tier, to replace this list with the ROM's own.
- Find where V2/V3/SP ghosts are placed (the net areas' NPC and encounter scripts) so rematches can reuse them.
