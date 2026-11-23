# Task for Gemini: which Soul Navi is "canon" in each BN4 tournament (research)

Assigned by the owner on 2026-11-23, written by Claude.

## Why

In *Violet Eclipse* each story tournament will have **one story fight against the "canon" Soul Navi**, the one that fits the story best. The tournament's other three Soul Navis become friendly matches. The owner's design is in `docs/pending/free-tournament-secret-chips.md`, "Exhibition matches, in detail".

The owner wants the canon choice to follow what the **Mega Man Battle Network community** considers canon, backed by the game's own story.

The four Soul Navis per tournament (from `decomp/notes/re04-tournaments-remainder.md` §1):

| Tournament | Red Sun's pair | Blue Moon's pair |
|---|---|---|
| 1st: Den Battle | GutsMan, FireMan | NumberMan, AquaMan |
| 2nd: Eagle (RS) / Hawk (BM) | Roll, WindMan | MetalMan, WoodMan |
| 3rd: Red Sun (RS) / Blue Moon (BM) | ThunderMan, SearchMan | ProtoMan, JunkMan |

## What to research

1. **Reddit Mega Man communities** (read only; never post or comment): r/BattleNetwork, r/Megaman and related ones. Search for discussions of:
   - which Soul Navi is the "canon" or "true" opponent in each BN4 tournament;
   - which version (Red Sun or Blue Moon) is considered canon, and what later games say (BN5 and BN6 references to BN4 events);
   - how the anime (*Mega Man NT Warrior / Rockman.EXE Stream*) and the manga treat these tournaments, if posts discuss them;
   - which Navi fits each tournament's story (e.g. the Navi whose operator matters to that chapter's plot).
2. **Other sources**, to cross-check: the Mega Man Knowledge Base / fan wikis, Rockman.EXE official material, and guides. Treat them as secondary, and say so.
3. **The game's own story text** (stronger evidence): which Navis each tournament's cutscenes and the version's plot actually mention.
   - TextPet is in `tools/textpet/` and its BN4 plugin is in `tools/textpet/plugins/` (both local).
   - Read the script dumps locally. Never commit extracted text; quote at most a short line per point.

## What to produce

**`decomp/notes/canon-navis.md`** (new), with:

1. A table per tournament and per version:
   - the proposed canon Navi;
   - the evidence for it: community threads (link, date, rough vote count or level of agreement), story text (map or script id, a short paraphrase), the anime or manga, later games;
   - your confidence (high / medium / low).
2. The disagreements: where the community is split, or where Red Sun and Blue Moon give different answers.
3. A recommendation for each tournament, keeping in mind that the merged game may or may not keep a Red Sun/Blue Moon choice (`plan/DEFINITIVE-EDITION.md` §7, question 1: undecided).

## Constraints

- **Read-only on the web.** Don't post, sign in, or send any personal or project details to any site. Use a generic client.
- **Cite every source** with a link. Summarize; don't paste long quotes or whole threads.
- **In the repo:** create only `decomp/notes/canon-navis.md` and your handoff. Don't edit anything else, don't run `make`, and don't push.
- Never commit game text, extracted scripts or guide data beyond a short quote.

## Done when

Each of the 3 tournaments (× 2 versions) has a proposed canon Navi with cited evidence and a confidence level, and the disagreements are listed for the owner to decide.
