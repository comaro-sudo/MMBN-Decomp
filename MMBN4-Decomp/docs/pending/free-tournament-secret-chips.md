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
- **Meeting all 12 Soul Navis in one playthrough (owner, 2026-11-23):**
  1. **Preferred:** the story tournaments let MegaMan fight every Soul Navi in one playthrough. Claude's proposed way:
     - with deterministic brackets (WP-B1), MegaMan's three opponents in each tournament are three of its four Soul Navis (the tournament's entrant list already includes both versions' pairs, `docs/re/tournaments.md` §3);
     - plus one scripted **exhibition match** after the final against the fourth one, using that Navi's existing battle record;
     - that's 4 × 3 = all 12, with no change to the 8-slot bracket screen.
     - A true 16-entrant bracket is the alternative, but it's a much bigger patch (bracket screen, graphics, round logic).
  2. **If that isn't possible:** (a) + (c). The operators keep the "met in the story" rule, and every operator not yet met **appears after the story ends**, as a post-game catch-up, so none is ever left out.
- **Scope (owner, 2026-11-23):** the climbing rule below applies **only to the lobby operators' rematches**. Story and tournament fights are independent of it and keep the game's own tiers.
- **Owner decision (2026-11-23), the rematch tier climbs with S ranks:** each operator's Navi starts at **V1**. After an S-rank win against V1, the next rematch is **V2**. After S against V2, it's **V3**. After S against V3, it's **SP**.
  - A win below S doesn't advance the tier.
  - Storage: 2 bits per Navi (V1/V2/V3/SP) × 12 Navis, in unused event flags.
  - The rematch battle must be started with the battle record's "no automatic raise" bit set (`+6` bit 7, RE-06), so the playthrough doesn't change the chosen tier.
  - (T: after SP the tier stays at SP. What an S win against SP gives once all 3 chips are owned isn't decided; the original rule falls back to V3.)

## Exhibition matches, in detail (owner likes this option, 2026-11-23)

**Goal:** MegaMan fights all 12 Soul Navis in one playthrough, through the story tournaments, without rebuilding the 8-slot bracket.

**Who is in each tournament** (`decomp/notes/re04-tournaments-remainder.md` §1). Each story tournament has four Soul Navis: the version's own pair plus the other version's pair.

| Tournament | Red Sun's pair | Blue Moon's pair |
|---|---|---|
| 1st: Den Battle | GutsMan, FireMan | NumberMan, AquaMan |
| 2nd: Eagle (RS) / Hawk (BM) | Roll, WindMan | MetalMan, WoodMan |
| 3rd: Red Sun (RS) / Blue Moon (BM) | ThunderMan, SearchMan | ProtoMan, JunkMan |

**How a tournament runs:**
1. **Bracket rounds** (WP-B1, deterministic): MegaMan's three opponents (round 1, semifinal, final) are three of those four Soul Navis. The other entrants are shared and generic Navis, as today.
2. **The exhibition match**, right after MegaMan wins the final, in the same venue. The announcer calls a special match against the fourth Soul Navi.
   - It's a scripted battle using that Navi's existing record in `BN4_STORY_BATTLES` (every Soul Navi has records there, identical in RS and BM).
   - No bracket screen is involved.
3. That's 4 Soul Navis × 3 tournaments = **all 12 in one playthrough**.

**Owner decisions (2026-11-23), these replace the proposals below where they differ:**
- **The "canon" Navi fights in the story.** In each tournament, the story fight is against the one Soul Navi that fits the story best. The other three Soul Navis of that tournament become **three exhibition (friendly) matches**.
  - Example only: if FireMan is the canon Navi of the 1st tournament, the friendly matches are GutsMan, NumberMan and AquaMan.
  - So each tournament has 1 story fight + 3 friendly matches, and the playthrough gets 3 + 9 = all 12.
  - **Which Navi is canon in each tournament is still open.** Proposal: decide from the game's own story text (which Navi the tournament's cutscenes and the version's plot refer to; TextPet script dumps, RE), then confirm with the owner. RS and BM may give different answers; the merged-version decision (§7 question 1) affects this.
- **Accepted for the friendly matches:**
  - tier: the game's normal tier for that playthrough, like the other story fights;
  - **winning gives that Navi's Soul**, so all 12 Souls are obtainable in one run;
  - losing can be retried right away, with no game over;
  - the match counts as **meeting** the Navi, win or lose, so its operator appears in the lobby.
- **When the friendly matches happen (owner, 2026-11-23):**
  - **all before the tournament's final;**
  - **2 or 3 before the first canon fight;**
  - the rest spread across the tournament day.
- **Which Navi is canon: decided** by the owner with Gemini (2026-11-23), for one unified story. See [`decomp/notes/canon-navis.md`](../../decomp/notes/canon-navis.md) §6, with friendly-match dialogue drafts in §7:

  | Tournament | Canon story fight | Friendly matches |
  |---|---|---|
  | 1st: Den Battle | AquaMan (Shuko) | GutsMan (Dex; challenges Lan after the canon fight), FireMan (Mr. Match), NumberMan (Higsby) |
  | 2nd: Eagle / Hawk | Roll (Mayl) | WindMan (Lily), MetalMan (Tamako), WoodMan (Sal) |
  | 3rd: Red Sun / Blue Moon | ProtoMan (Chaud) | SearchMan (Raika; challenges Lan before the Chaud fight), ThunderMan (Raoul), JunkMan |

  This lineup implies one merged story, no RS/BM split for the tournaments (`plan/DEFINITIVE-EDITION.md` §7 question 1).
- **JunkMan has no operator (owner decision, 2026-11-23):** he **waits in the lobby's network**. Lan jacks in, and JunkMan stands in a small cyber-area as an NPC; talking to him starts the rematch.
  - **Owner's preference: the jack-in point is a trash can** in the lobby or nearby, fitting JunkMan ("born from data the world threw away"). Whether one exists and can be jacked into is part of Gemini's round-3 task 1. Otherwise, a lobby PC. It matches the friendly-match draft, where MegaMan "picks up a strange Navi signal".
  - **JunkMan's overworld (field) sprite exists in both ROMs (V, checked by Claude, 2026-11-23).**
    - Sprite list 6 (the Navis' overworld sprites; the list table at RS `0x0802793C` / BM `0x08027940`, per Cyberworld Endless's `docs/rom/bn4.md` and `bn4_navis.h`), entry `0x5A`, points to `0x08486FC8` in **both** versions, and its first 8 KiB are byte-identical.
    - Roll (`0x41`), WoodMan (`0x47`), AquaMan (`0x51`) and SearchMan (`0x59`) are likewise at the same address in both, so the lobby NPCs need no imported graphics.
    - His battle sprite also exists in both.

**Details to settle (earlier proposals; see the owner decisions above):**
- **Which Navi goes in the exhibition?** Proposal: one of the other version's pair, so the version's own story stays as it was. The final keeps its original opponent.
- **Its tier:** the game's own tier (V1 on the first playthrough, raised by the playthrough as usual), the same as the other story fights.
- **Does winning give that Navi's Soul?** That would give all 12 Souls in one run (criterion C1, WP-B1). Proposal: yes. How the story grants a Soul is Gemini's round-2 task 2.
- **Losing:** the original tournaments end the game on a loss. Proposal: a lost exhibition can be retried right away, with no game over, since it's a bonus match.
- **"Met" for the lobby operators:** winning or losing the exhibition counts as meeting that Navi, so its operator appears in the lobby.
- **Scenarios:** each Soul Navi has its own side scenario (e.g. AquaMan's water-god comps, RE-07). The exhibition only adds a battle. Whether the unchosen Navis' scenarios also become playable is part of WP-D4.

**Implementation sketch (T):**
- a script after the tournament-win cutscene: text, then `startBattle` with the Navi's record, then the Soul grant if chosen, then set flags (exhibition done, Navi met);
- one flag per tournament so it happens once;
- the bracket logic itself is untouched.

**The variant with a hidden first round** (owner's idea for Option 1): a 16-entrant tournament whose first round isn't shown, with the normal 8-slot bracket only once 8 Navis remain. In practice that's the same as an exhibition **before** the bracket: MegaMan plays a qualifying match against the fourth Soul Navi, then the usual three rounds. It's just as cheap, because the qualifier is a scripted battle too. The difference is only presentation and story order: "qualifier, then bracket" versus "bracket, then exhibition". Either works with the same code.

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
