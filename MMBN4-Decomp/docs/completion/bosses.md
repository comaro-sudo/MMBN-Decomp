# Bosses (criterion C3)

**From the ROM** (RE-06, Gemini, 2026-11-22): [`../../decomp/notes/re06-encounters.md`](../../decomp/notes/re06-encounters.md) has every record and table. It replaces the earlier list, which was derived from the Navi chips. **V** = read from both US ROMs.

## The key finding: every boss is in both ROMs (V)

| Table | Contents | RS vs BM |
|---|---|---|
| `BN4_ENEMY_STATS` (`0x08011888`) | 254 records: viruses `0x01`–`0x7F`, Navis and bosses `0x80`–`0xFC` | identical |
| `BN4_STORY_BATTLES` (RS `0x080FA900` / BM `0x080FA90C`) | 245 story, tournament and boss battles | identical |
| `BN4_BATTLE_TABLES` (`0x08017FB8`) | 512 net-area encounter slots, including the wandering-ghost (`+0x10`) and story (`+0x20`/`+0x30`) variants | identical |
| `BN4_REWARD_ROWS` (RS `0x080F5F50` / BM `0x080F5F5C`) | 276 drop rows | **64 rows differ, in chip codes only** (§3) |

So the "version-exclusive" Navis (six per version) aren't missing from either ROM. Their records, battles and roaming-ghost tables are present and identical in both. **What keeps them out is game state:**
- which Soul Navis a version's tournaments field ([`../re/tournaments.md`](../re/tournaments.md): Red Sun already fields Blue Moon's Navis when they're in its traded list);
- the story flags that enable each ghost table (each `+0x10` table has an enabling flag, listed in RE-06 §4).

That makes the merged-content work (WP-D4) much smaller: no Navi data has to be imported. The work is in flags, tournament pools and scripts. (T until an in-game test fields an other-version Navi this way.)

## Navis and tiers (V)

- Each Navi has four records: V1, two stronger versions, and Ω. RE-06 writes them as `α`, `β` and `Ω`, which are the US V2, V3 and SP. For example, Roll is 700, 1000 and 1300 HP, then RollΩ 1500.
- **The playthrough raises the tier automatically.** `BN4_ID_RAISE` (`0x0800B036`) adds `min(playthrough, 2)` to a Navi's id (V1 → V2 → V3) and `2 × tier` to a virus's, unless the battle record's `+6` bit 7 is set. No separate Hard or Super Hard tables exist.
- **The Ω (SP) versions roam the net:** each one sits in one area's wandering-ghost table (`+0x10`), with an enabling flag.

| Group | Navis | Ω (SP) area |
|---|---|---|
| Red Sun's Soul Navis | Roll, GutsMan, WindMan, SearchMan, FireMan, ThunderMan | Town 3, ACDC 2, YumLand, Undernet 1, Town 2, Netopia |
| Blue Moon's Soul Navis | ProtoMan, NumberMan, MetalMan, JunkMan, AquaMan, WoodMan | Undernet 6, Town 4, Undernet 3, Undernet 2, ACDC 1, Park 1 |
| Shared | TopMan, ShadeMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan | Town 1, Undernet 4, Park 2, Sharo, ACDC 3, Undernet 5, NetFrica, Park 3 |
| Duo | Final boss, Meteor Comp 4; tiers by playthrough | — |
| Bass | Bass (Black Earth 2), BassΩ (roams Black Earth 2, flag `0x9F`), BassXX (story-override table) | — |

- **Dark Soul chips:** each of the 20 Navis has one "DS" drop row (`0x100`–`0x113`). It's used instead of the normal row when MegaMan's Dark Karma is past the threshold (`sub_0800F5A6() > 0x1D5`). These rows are identical in both versions.

## 3. What differs by version: drops (V)

- **Bass:** drops `Bass X` in Red Sun and `BassAnly X` in Blue Moon. BassΩ and BassXX drop `BlakBarr A` in RS and `DeltaRay Z` in BM (columns 3–4).
- **61 virus families drop a different code for the same chip**: e.g. Mettaur's `Guard1 L` (RS) vs `Guard1 V` (BM), Billy's `Thunder1 L` vs `P`, the Elmperor elements, BomBoy and the CircGuns. The full list is RE-06 §7.

**Owner decision (2026-11-23), `plan/DEFINITIVE-EDITION.md` §7 question 4:**
- **Random per drop.** Wherever the two versions' drop rows differ, a drop gives the Red Sun or the Blue Moon item at random (50/50), each time.
  - Implementation idea: keep both versions' rows. After `BN4_REWARD_PICK` (`0x080F580C`) picks its slot, choose the RS or BM row with one RNG bit. The table costs about 64 × 40 bytes (2.5 KiB) in free space (`docs/re/free-space.md`).
- **Bass is repeatable.** Bass, BassΩ and BassXX must be fightable as many times as the player wants, so that the random rule eventually gives all four chips: Bass, BassAnly, BlakBarr and DeltaRay.
  - Bass's every drop column gives his chip, so any Busting Level works. BlakBarr/DeltaRay need columns 3–4 (Busting Level 7 or higher, RE-06 §3).
  - Today Bass is a one-time story battle (`BN4_STORY_BATTLES` id 176). BassΩ roams Black Earth 2 behind flag `0x9F`. BassXX is in a story-override table.
  - The rematch (WP-D3) must re-arm all three. The simplest version is a repeatable script that starts battle 176 and the BassΩ/BassXX records again. (T: whether BassΩ already respawns after a win needs an in-game check.)
- (Considered and not chosen: the code follows the chosen version; extra drop slots at high Busting Levels.)

## What blocks completion (updated)

| Blocker | Fix |
|---|---|
| The other version's six Navis are never fielded | WP-D4, now smaller: tournament pools, flags and scripts (data already present) |
| Tiers depend on the playthrough count | Frequency Tuner (WP-B2): `BN4_ID_RAISE` is the single place to hook |
| The other version's drop codes | Random RS/BM item per drop (owner decision, above) |
| No rematches after the story | WP-D3: boss rush or BBS challenges. The 245 story-battle records can be reused as they are. **Bass, BassΩ and BassXX must be repeatable** (owner decision) |
| The WoodMan bug | WP-A1 (done) |

## Next

- Test in-game: set the traded-Navi list and the ghost flags so that one ROM fields the other version's Navi.
- Feed RE-06's drop rows into `patch/tools/inventory.py` as chip sources, then re-run the missing-chip count in [`chips.md`](chips.md).
