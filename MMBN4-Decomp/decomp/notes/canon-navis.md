# Canon Soul Navis per BN4 Tournament (Research for Violet Eclipse)

Research for the owner's *Violet Eclipse* tournament design (`docs/pending/free-tournament-secret-chips.md`, "Exhibition matches, in detail"): each of the three story tournaments has **one story fight against the "canon" Soul Navi** and **three friendly matches** against the tournament's other three Soul Navis.

Evidence hierarchy used below:
1. **In-game code & story text (strongest):** verified directly from `bn4_rs_us.gba` and `bn4_bm_us.gba` (`sub_08043FE4` pair tables, `soul-grant.md`, and local script dumps of hub/finale archives).
2. **Later games (BN5 / BN6 continuity):** which BN4-introduced characters reappear with established familiarity.
3. **Reddit Mega Man communities (`r/BattleNetwork`, `r/Megaman`):** read-only synthesis of recurring canon threads.
4. **Secondary sources:** Mega Man Knowledge Base (MMKB), fan wikis, *MegaMan NT Warrior* (*Axess* / *Stream*) anime, and Ryo Takamisaki's *Rockman.EXE* manga.

---

## 1. Overarching community consensus on BN4 canon

- **No single official canon version:** Capcom never declared Red Sun or Blue Moon strictly canon. On `r/BattleNetwork` and `r/Megaman`, the prevailing view is a **composite timeline** — because BN4's story explicitly states that the Red Sun and Blue Moon tournaments run concurrently to find NetBattlers for the Duo asteroid mission, and because BN4's multi-playthrough / link-trade structure lets Lan meet both rosters, fans generally treat Lan as having met characters from **both** versions across the timeline ([r/BattleNetwork — canon version discussion](https://www.reddit.com/r/BattleNetwork/), [MMKB — Mega Man Battle Network 4](https://megaman.fandom.com/wiki/Mega_Man_Battle_Network_4)).
- **Two popular trilogy continuity tracks** cited across Reddit threads:
  - **Red Sun → BN5 Team ProtoMan → BN6 Gregar:** anchored by **Raika / SearchMan** (introduced in BN4 Red Sun and already known to Lan in BN5 Team ProtoMan) and **Mr. Match** (reappears in BN6 Gregar).
  - **Blue Moon → BN5 Team Colonel → BN6 Falzar:** anchored by **Shuko / AquaMan (SpoutMan)** (introduced in BN4 Blue Moon and reappearing as an established friend in BN6 Falzar), paired with *Team Colonel*'s Baryl/Colonel/Dingo continuity into BN6.
- **Anime & manga context (secondary):**
  - ***MegaMan NT Warrior Axess* / *Stream*:** drops the 3-tournament bracket structure in favor of Cross Fusion and Nebula, and gives MegaMan **all 12 BN4 Double Souls** organically over the course of *Axess* (Roll, Guts, Fire, Search, Wind, Metal, Number, Aqua/Spout, Wood, Thunder, Junk, Proto) ([MMKB — Double Soul](https://megaman.fandom.com/wiki/Double_Soul)).
  - ***Rockman.EXE* manga (Ryo Takamisaki):** bypasses the tournament loop entirely; **ProtoSoul** is given the most dramatic showcase as the debut Double Soul against Bass, originating from Serenade's program ([MMKB — MegaMan NT Warrior manga](https://megaman.fandom.com/wiki/MegaMan_NT_Warrior_(manga))).

---

## 2. Per-tournament, per-version analysis

### Tournament 1: Den Battle (`Tournament 0`)
*Pool: Red Sun = GutsMan, FireMan | Blue Moon = AquaMan, NumberMan*

| Version | Proposed canon Navi | Confidence | In-game code & story evidence | Community / later games / media |
|---|---|---|---|---|
| **Red Sun** | **GutsMan** (Dex) | **High** | **Hardcoded by the ROM on Playthrough 1:** `sub_08043FE4` (`0x08044044`) puts GutsMan (`0x00`) first and locks pair order until flag `0x15` is set (`docs/re/tournaments.md` §3). The mandatory Double Soul tutorial (`RS 0x08059EB6`, `soul-grant.md`) hardcodes `GutsSoul` (`0x18`, chip key `354`). | `r/BattleNetwork` threads consistently treat GutsMan as RS's signature first Soul opponent (Lan's best friend introducing Double Soul). Mr. Match / FireMan are mentioned in DenDome hub NPC chatter (`0x08782BA4`, `0x08784430`) and Match returns in BN6 Gregar, but FireMan cannot even be fought on Playthrough 1 in RS. |
| **Blue Moon** | **AquaMan** (Shuko) | **High** | **Hardcoded by the ROM on Playthrough 1:** `sub_08043FE4` (`0x0804404C`) puts AquaMan (`0x03`) first and locks pair order until flag `0x15` is set. The mandatory Double Soul tutorial (`BM 0x08059EC2`, `soul-grant.md`) hardcodes `AquaSoul` (`0x21`, chip key `363`). | **Strong BN6 continuity:** Shuko and AquaMan (SpoutMan) are introduced here and return in **BN6 Cybeast Falzar** as established friends of Lan — a headline argument on `r/BattleNetwork` for Blue Moon's canon footprint. Higsby/NumberMan return in BN5 Team Colonel, but Lan already knew Higsby from BN1–BN3. |

### Tournament 2: Eagle (RS) / Hawk (BM) (`Tournament 1`)
*Pool: Red Sun = Roll, WindMan | Blue Moon = MetalMan, WoodMan*

| Version | Proposed canon Navi | Confidence | In-game code & story evidence | Community / later games / media |
|---|---|---|---|---|
| **Red Sun (Eagle)** | **Roll** (Mayl) | **High** | `sub_08043FE4` (`0x08044044`) lists Roll (`0x04`) first (`04 05`). Castillo's main-story hub archives (`0x08785FCC`, `0x08786E94`, `0x08788A50`) center on Lan's Castillo visit with Mayl (*"You're here to meet Mayl, right?"*) and rescuing Roll. Roll/Mayl have the highest archive mention count in the game (45 / 41); WindMan (Lily) is never mentioned outside his own scenario. | `r/BattleNetwork` favors Roll as the emotional/story anchor of the Castillo arc and one of the core trio alongside GutsMan. Neither Roll nor WindMan creates a BN5/BN6 continuity conflict (WindMan never returns). |
| **Blue Moon (Hawk)** | **MetalMan** (Tamako) *or* **WoodMan** (Sal) | **Low–Medium** *(split)* | `sub_08043FE4` (`0x0804404C`) lists MetalMan (`0x06`) first (`06 07`), though Playthrough 1 picks randomly since flag `0x15` is already set. Castillo hub archive `0x08786E94` contains a WoodMan scenario hook (*"something happened to WoodMan"*). Neither Tamako nor Sal is referenced in the NAXA/Duo finale. | **No community consensus.** Both are returning BN1/BN3 allies with self-contained BN4 scenarios and no BN5/BN6 story callbacks. Slight mechanical tie-breaker to **MetalMan** (slot `+0` in `0x0804404C`), or **WoodMan** if preferring the Castillo-hub mention. |

### Tournament 3: Red Sun (RS) / Blue Moon (BM) (`Tournament 2`)
*Pool: Red Sun = SearchMan, ThunderMan | Blue Moon = ProtoMan, JunkMan*

| Version | Proposed canon Navi | Confidence | In-game code & story evidence | Community / later games / media |
|---|---|---|---|---|
| **Red Sun** | **SearchMan** (Raika) | **High** | Sharo SpaceCtr main-plot dialogue (`RS 0x0878EAC8` / `BM 0x0878E558`) explicitly references Raika right before the NAXA/Duo finale (*"You came all this way to see Raika? ... he's busy with an important mission"*). (`sub_08043FE4` lists `08 09` ThunderMan/SearchMan, randomized after flag `0x15`.) | **Strongest BN5 continuity anchor:** on `r/BattleNetwork`, Raika/SearchMan is the #1 cited reason for Red Sun's canon relevance, because Lan and Raika are already acquainted in **BN5 Team ProtoMan**. Raoul/ThunderMan (from BN2) never returns in BN5 or BN6. |
| **Blue Moon** | **ProtoMan** (Chaud) | **High** | `sub_08043FE4` (`0x0804404C`) lists ProtoMan (`0x0A`) first (`0A 0B`). **Chaud** appears directly in the NAXA / Duo finale in **both** versions (`RS 0x087C2E0C`, `0x087C3454` / `BM 0x087C28B0`, `0x087C2EF8`), saving Lan and confronting Regal. JunkMan is an autonomous one-off Navi with zero main-plot presence. | Chaud/ProtoMan is Lan's primary rival, co-lead of BN5 (*Team ProtoMan*), returns in BN6, and **ProtoSoul** is the signature debut Double Soul in the *Rockman.EXE* manga. |

---

## 3. Key disagreements for the owner to decide

1. **Red Sun vs. Blue Moon continuity split (`BN5`/`BN6`):**
   - **Den Battle:** GutsMan is hardcoded in RS and is Lan's iconic starter Soul; **AquaMan** is hardcoded in BM and is the only Den Battle Navi who *needs* a BN4 introduction to explain **BN6 Falzar** (Lan already knows Dex, Match, and Higsby from BN1–BN3).
   - **Red Sun / Blue Moon Tournament:** **SearchMan** (RS) is the only Tournament 3 Navi who *needs* a BN4 introduction to explain **BN5 Team ProtoMan** (Lan already knows Chaud from BN1–BN3, and Chaud appears in BN4's NAXA finale regardless). Conversely, **ProtoMan** (BM) is far more iconic to the fanbase and ties directly into the NAXA/Duo climax and the manga — though some `r/BattleNetwork` fans dislike ProtoMan's Dark-Chip-tainted heel turn in BN4 after BN3's resolution.
2. **Hawk Tournament (Blue Moon Tournament 2) has no clear canon winner:** MetalMan vs. WoodMan is a genuine toss-up (both are returning minor allies with no later-game callbacks).
3. **Randomization after Tournament 1:** In the retail ROMs, only Tournament 1 is deterministic on Playthrough 1; Tournaments 2 and 3 randomize their pair via `GetTertiaryRNG() & 1`. Therefore, the ROM itself does not crown a single Playthrough-1 winner for Tournaments 2 and 3 — story text and sequel continuity are the deciding factors.

---

## 4. Recommendations for *Violet Eclipse*

This depends on `plan/DEFINITIVE-EDITION.md` §7, question 1 (whether *Violet Eclipse* keeps a RS/BM version choice or merges into a single unified game):

### Option A — Keep a Red Sun / Blue Moon version choice (per-version canon)
| Tournament | Red Sun canon story fight | Blue Moon canon story fight | Friendly matches (the other 3) |
|---|---|---|---|
| **1st: Den Battle** | **GutsMan** | **AquaMan** | The remaining 3 of `{GutsMan, FireMan, AquaMan, NumberMan}` |
| **2nd: Eagle / Hawk** | **Roll** | **MetalMan** *(or **WoodMan**)* | The remaining 3 of `{Roll, WindMan, MetalMan, WoodMan}` |
| **3rd: Red Sun / Blue Moon** | **SearchMan** | **ProtoMan** | The remaining 3 of `{SearchMan, ThunderMan, ProtoMan, JunkMan}` |

### Option B — Single unified *Violet Eclipse* story (no RS/BM split)
| Tournament | Recommended canon story fight | Alternative | Why |
|---|---|---|---|
| **1st: Den Battle** | **GutsMan** | **AquaMan** | **GutsMan** is the most natural Double Soul tutorial with Lan's best friend (and RS's hardcoded Playthrough-1 fight). Pick **AquaMan** instead if prioritizing BN6 Falzar continuity (Shuko's first meeting). |
| **2nd: Eagle / Hawk** | **Roll** | *(none strong)* | **Roll** is the clear winner across all four Navis (`Roll, WindMan, MetalMan, WoodMan`): Mayl's Castillo date and Roll's rescue are embedded directly in Castillo's main-story hub text. |
| **3rd: Red Sun / Blue Moon** | **ProtoMan** *(for climax impact)* **or SearchMan** *(for BN5 continuity)* | — | **Owner's call:** **SearchMan** gives Raika his needed BN4 introduction before BN5 (and matches SpaceCtr's pre-NAXA Raika mention); **ProtoMan** gives the final tournament its biggest rival match right before Chaud joins Lan at NAXA/Duo. *(Remember: whichever is not the story fight is still fought as a friendly match before the final, so both Raika and Chaud are still met!)* |

---

## 5. Sources

### Reddit community discussions (read-only)
- [r/BattleNetwork](https://www.reddit.com/r/BattleNetwork/) — recurring threads on BN4 Red Sun vs. Blue Moon canon, the `Red Sun → Team ProtoMan → Gregar` vs. `Blue Moon → Team Colonel → Falzar` continuity tracks, Raika's BN4→BN5 familiarity, Shuko's BN4→BN6 Falzar familiarity, and the composite-canon consensus (accessed 2026-11-23 via web search).
- [r/Megaman](https://www.reddit.com/r/Megaman/) — cross-posted discussions on BN4–BN6 version canon and loose Capcom continuity (accessed 2026-11-23 via web search).

### Secondary wikis & official-media references
- Mega Man Knowledge Base (MMKB): [Mega Man Battle Network 4](https://megaman.fandom.com/wiki/Mega_Man_Battle_Network_4), [Double Soul](https://megaman.fandom.com/wiki/Double_Soul), [MegaMan NT Warrior (anime)](https://megaman.fandom.com/wiki/MegaMan_NT_Warrior), [MegaMan NT Warrior (manga)](https://megaman.fandom.com/wiki/MegaMan_NT_Warrior_(manga)).
- StrategyWiki / GameSpot walkthroughs confirming BN4 tournament rosters and 3-playthrough RNG structure.

### Primary in-game sources (local US ROMs & decomp notes)
- `MMBN4-Decomp/docs/re/tournaments.md` §3 (`sub_08043FE4`, pair tables `0x08044044` RS / `0x0804404C` BM, flag `0x15` lock).
- `MMBN4-Decomp/decomp/notes/soul-grant.md` (hardcoded Playthrough-1 tutorial grants: RS GutsSoul `0x08059EB6`, BM AquaSoul `0x08059EC2`).
- Local ROM text archives (`bn4_rs_us.gba` / `bn4_bm_us.gba`): DenDome hub (`0x08782BA4`, `0x08784430`), Castillo hub (`0x08785FCC`, `0x08786E94`, `0x08788A50`), Sharo SpaceCtr (`RS 0x0878EAC8` / `BM 0x0878E558`), NAXA / Duo finale (`RS 0x087C2E0C`, `0x087C3454` / `BM 0x087C28B0`, `0x087C2EF8`).

---

## 6. Owner decision (2026-11-23)

Confirmed unified *Violet Eclipse* canon lineup (1 canon story fight + 3 friendly matches per tournament):

| Tournament | Canon story fight | Friendly matches (3 per tournament) | Story / timing beats specified by owner |
|---|---|---|---|
| **1st: Den Battle** | **AquaMan** (Shuko) | **GutsMan** (Dex), **FireMan** (Mr. Match), **NumberMan** (Higsby) | **Dex / GutsMan** challenges Lan **after** the first canon fight (AquaMan), with dialogue along the lines of *"I lost my fight, but I really wanted to battle you."* |
| **2nd: Eagle / Hawk** | **Roll** (Mayl) | **WindMan** (Lily), **MetalMan** (Tamako), **WoodMan** (Sal) | Standard friendly-match pacing before the final. |
| **3rd: Red Sun / Blue Moon** | **ProtoMan** (Chaud) | **SearchMan** (Raika), **ThunderMan** (Raoul), **JunkMan** | **Raika / SearchMan** challenges Lan **before** the canon fight with Chaud / ProtoMan, with a similar friendly-challenge beat (establishing Lan & Raika's acquaintance for BN5 before the ProtoMan climax). |
