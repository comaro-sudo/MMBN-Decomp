# Pending owner decision: the Free Tournament fix for the 36 secret chips

Written by Claude, 2026-11-23, from Gemini's RE-04 note ([`decomp/notes/re04-tournaments-remainder.md`](../../decomp/notes/re04-tournaments-remainder.md)) and RE-06. **The owner hasn't decided yet.** See also [`../completion/chips.md`](../completion/chips.md) §3.

## How the Free Tournament works today

The Free Tournament has 8 slots, and MegaMan always takes one. Each other slot is filled from the **Waiting Room**, which can hold up to 7 Navis:

- A Navi only gets into the Waiting Room through a **link-cable trade** with another cartridge.
- Each traded entry carries the Navi's id plus two blocks of battle data copied from the other player's game: its stats (save slot `+0x78`) and its AI/attack patterns (slot `+0x7C`).
- Any empty slots get generic fillers, NormalNavi and HealNavi, which only give Zenny or BugFrags.

So with no trades, you only ever fight fillers, and none of the 36 secret chips can drop (RollAro, GutPnch, PropBom, SeekBom, Meteors, Ligtnin, HawkCut, NumbrBl, MetlGer, PanlSht, AquaUp and GreenWd, each at V1–V3).

## The proposed fix

The patch changes `FreeTournamentSetup` (RS `0x080448C0` / BM `0x080448C8`), the routine that fills the 8 slots:

1. **Opponents:** stop reading the Waiting Room and fill the slots straight from the 12 Soul Navis: Roll, GutsMan, WindMan, SearchMan, FireMan, ThunderMan, ProtoMan, NumberMan, MetalMan, JunkMan, AquaMan, WoodMan. Only 7 fit in one tournament, so the obvious rule is to pick first the Navis whose secret chips you still lack.
2. **Battle data:** instead of the other player's traded stats and AI, each opponent uses the game's own record for that Navi. This is the data the story and the normal tournaments already use. Gemini's RE-06 showed that both ROMs contain all 12 Soul Navis' records, and that they're identical. Nothing has to be imported from the other version.
3. **Prizes stay as they are.** Beating a Soul Navi gives one of its 3 secret chips (`FreeTournamentSelectPrize`, `0x08045458`):

| Busting Level of the win | Chip |
|---|---|
| 1–3 | V1 (e.g. GutPnch1) |
| 4–6 | V2 |
| 7–9 | V3 |
| 10 or S | **the highest one you still lack** (`FreeTournamentPickMissingSecretChip`, `0x08045540`) |

That last rule is why S-rank wins are quick. Each S-rank win against a Navi gives a chip you don't have yet, so 36 S-rank wins give all 36. With the "Navis you still lack first" rule, every tournament is useful.

## What still needs checking

The open question is **what the Free Tournament battle actually reads** when it starts a fight against a traded Navi:

- **If it builds the fight from the Navi's id**, the same way the story tournaments do, the fix is simple: change who is picked, and the battle just works.
- **If it reads the stats and AI blocks in the save** (slots `+0x78` and `+0x7C`), there's no traded data to read. The patch would then also need to fill those blocks with each Navi's standard data.
  - A related option is to create "virtual" Waiting Room entries at new game. That works too, but with only 7 slots it would need rotating to cover all 12 Navis.

Gemini's notes don't settle which one it is yet. It's a short static read of the battle setup code, plus one test in mGBA.

## Owner addition (2026-11-23): rematches in the lobby

**If the player didn't win with Busting Level 10 or S, the Navis' operators stand in the Free Tournament lobby, and Lan and MegaMan can challenge them again and again.**

- Each of the 12 Soul Navis' operators is an NPC in the lobby. Talking to one starts a battle against that operator's Navi, as many times as the player wants.
- The prize follows the same Busting Level table as the tournament, so a 10/S win gives the highest secret chip still missing. This way a lower-ranked tournament win never costs a chip; the player can just try again.
- Implementation notes (T, from RE-04/RE-06):
  - each Soul Navi already has battle records in `BN4_STORY_BATTLES` (e.g. Roll: ids 64, 207, 208), identical in RS and BM;
  - the NPC script can start one of them and then call the Free Tournament prize routine (`FreeTournamentSelectPrize`, `0x08045458`) for that Navi;
  - placing 12 NPCs needs the lobby map's NPC list (not mapped yet), and their sprites (all in both ROMs, since every operator appears in the story).
- **Owner decision (2026-11-23):** each operator appears in the lobby **only after the player has met that Navi in the story**.
- **Caveat on "met in the story":** in the original game, one playthrough doesn't meet all 12 Soul Navis.
  - Each of a version's three tournaments draws **one of its two** Soul Navis (`docs/re/tournaments.md` §3), so a run meets 3 of the version's 6.
  - The other version's 6 never appear as story opponents.
  - Violet Eclipse already plans to fix this (WP-B1: every Soul in one run; WP-D4: the other version's Navis). Until those exist, the "met" condition would leave some operators out. Owner to confirm a rule (proposed below).
- **Scope (owner, 2026-11-23):** the climbing rule below applies **only to the lobby operators' rematches**. Story and tournament fights are independent of it and keep the game's own tiers.
- **Owner decision (2026-11-23), the rematch tier climbs with S ranks:** each operator's Navi starts at **V1**. After an S-rank win against V1, the next rematch is **V2**. After S against V2, it's **V3**. After S against V3, it's **SP**.
  - A win below S doesn't advance the tier.
  - Storage: 2 bits per Navi (V1/V2/V3/SP) × 12 Navis, in unused event flags.
  - The rematch battle must be started with the battle record's "no automatic raise" bit set (`+6` bit 7, RE-06), so the playthrough doesn't change the chosen tier.
  - (T: after SP the tier stays at SP. What an S win against SP gives once all 3 chips are owned isn't decided; the original rule falls back to V3.)

## Decisions for the owner

1. Use this fix for the 36 secret chips (C2)?
2. Which opponents per tournament: the Navis whose chips you still lack first (proposed), all at random, or a fixed rotation?
3. Keep the original Busting Level rule for the prize (proposed), or always give the highest missing chip?

---

## Decomp status at the time of writing (2026-11-23)

Not part of the decision; recorded together at the owner's request.

- Four pointer tables carved in `code_01` and `code_02` (commit `1c8fca7`): `code_01` `0x08016C04`, and `code_02` `0x0802793C`, `0x0802831C`, `0x0802891C` (BM +4). Both ROMs still build byte-identical.
- **Coverage 58.0%**, up from 57.2% (RS 683,504 of 1,179,456 code bytes in source). With the encounter tables carved just before it (`db01bda`), about 84 KiB was carved this round.
- Next candidates:
  - the 17 KiB block at `0x080197EC` in `code_01` (about 28% pointers);
  - two mixed tables in `code_02` and `code_03`.

  They mix pointers with other data, so they need their record layout read before carving.
- The cutscene scripts in `code_04c` stay as they are, because the version-specific insertions shift them differently in each version.
