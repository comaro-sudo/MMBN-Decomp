# Mega Man Battle Network 4: Violet Eclipse

*Draft player README (2026-11-27). Final wording, credits and licence are still open: see `docs/release-checklist.md`.*

Violet Eclipse is a fan patch for **Mega Man Battle Network 4**. It lets you finish the game, with every Double Soul, every chip and every boss, in **one save**: no second cartridge, no link cable, no e-Reader cards. There is one patch for **Red Sun** and one for **Blue Moon**. Each is a complete game.

## What you need

- A legally dumped US ROM of the game. The patch checks it:

  | Patch | ROM | SHA-1 |
  |---|---|---|
  | `bn4_rs_overhaul.bps` | Mega Man Battle Network 4: Red Sun (USA) | `a97e96a7da03abd70f7953328e511b9fa29179f1` |
  | `bn4_bm_overhaul.bps` | Mega Man Battle Network 4: Blue Moon (USA) | `5e017c803ddc768efb8010314b46bc17e9757f71` |

- A BPS patcher, such as Flips: open the `.bps`, choose your ROM, save the patched copy. Keep your original ROM.

## What changes

**All 13 Double Souls in one game**
- All 12 Souls can be earned in either version. Nine **friendly matches** at the tournaments (3 per tournament, in the rounds before the final) give the Soul of the Navi you fight: Guts, Fire, Number, Wind, Metal, Wood, Search, Thunder and Junk. A Navi stops appearing once you own its Soul. You can retry if you lose. The other three (Aqua, Roll, Proto) are the fixed Navi of each tournament, the same in both versions.
- A new 13th Soul, the **Duo Soul**, is granted when you beat Duo (the story fight or any rematch), together with the Duo chip (D, once). It has its own colours, a 180-damage fist charge shot, AirShoes + FloatShoes, and a Reflect shield.
- The status screen shows every Soul (R / L switches pages), and the Souls completion star needs all 12.

**Every chip**
- Drops that differ between versions now come from either version's table, 50/50.
- Bass gives both versions' Bass chips (the results screen names only one; the other is added silently).
- The Free Tournament fields all 12 Soul Navis without link trades, so all 36 secret chips can be won.
- **Duo** and **Grand Prix Power**, once event-only chips, are now in the game with names and descriptions. Public builds have no picture for them, see "Known issues".
- **Z-Saver** and **Grand Prix Power** each wait in a Blue Mystery Data in an out-of-the-way corner.

**Every boss, as often as you like**
- **Rematches in Higsby's shop:** the operators of all 12 Soul Navis wait in and outside Higsby's. Each one appears once you own that Navi's Soul. JunkMan has no operator: jack into the trash can on the ACDC street.
  - Each Navi climbs V1 → V2 → V3 → SP → **SP+** as you beat it with an S rank.
- **Duo rematches:** after the ending, find Duo's signal in Meteor Comp 1. It climbs Duo → α → β → Ω → SP+ with S ranks.
- **SP+**, the new top difficulty, after an S rank on the SP (or Duo Ω) fight: 4,095 HP, damage to you ×1.5, speed ×1.25. It also covers the roaming SP bosses. Winning pays 15,000 Zenny (80%) or 50 BugFrags (20%).

**Quality of life**
- A **Frequency Tuner** in Higsby's sets the virus tier (Auto / V1 / V2 / V3). Green Mystery Data follow the same setting, so every reward is reachable on any playthrough.
- Battle Zenny ×1.5.
- HP Memory, the Zenny NaviCust programs, Unlocker and LocEnemy cost about 40% of their old price (rounded per item).
- The WoodMan scenario fix (by Prof. 9) is included.
- The Waiting Room rejects a link transfer with an out-of-range Navi slot instead of writing it into your save.

## Where to find the new content

| What | Where |
|---|---|
| Rematch operators (9) and the Frequency Tuner | Inside Higsby's shop, ACDC Town |
| 2 more operators and the trash can (JunkMan) | ACDC Town street; the trash can is on the sidewalk at the front-right corner of Higsby's, under the awning |
| Friendly matches (9: three per tournament) | DenDome Hall (Den Battle), the Castillo waiting room (Eagle/Hawk), the Colosseum lobby (Red Sun/Blue Moon). They stand there during the first two rounds of each tournament (Dex only after the AquaMan fight, Raika only before the Chaud fight) and disappear once you own that Navi's Soul |
| Duo's signal, a crest-shaped marker (rematches) | Meteor Comp 1, right by the jack-in point, after the ending (jack in at the Space Center) |
| Z-Saver | A Blue Mystery Data in a hidden red dead-end spur of Undernet 1, tucked under the western yellow bridge |
| Grand Prix Power | A Blue Mystery Data on the quiet round dead-end pad on the lower-left peninsula of ACDC Area 2 |

## Saves

Existing saves from the same version work. Shop prices update when you load one. Make a backup of your save first.

## Known issues

- **Public builds:** the Duo and Grand Prix Power chips have names and descriptions but no picture.
- **Chip Library:** Duo and Grand Prix Power can be obtained and used in battle, but they do not appear in the Chip Library (they are not required for any completion star).
- **Bass drops:** beating Bass awards both versions' Bass chips, though the results screen only names one (the other is added silently).

Full list (including port issues): `docs/known-issues.md`.

## Credits

- **Prof. 9**: the WoodMan scenario fix ("WoodMan Glitch Fix v2.0", free to use), and **TextPet** (MIT), the text tool the patch is built with.
- **Kingcom**: **armips** (MIT), the assembler.
- **Alcaro**: **Flips** (GPL-3.0 or later), which creates and applies `.bps` patches.
- **StraDaMa**: the community Mega Man Battle Network chip tables, used as a build-time check.
- **Sascha Becker** (*Cyberworld Endless*, MIT), the **nettai** project and the **dism-exe** disassembly community: the symbol names and engine notes that made finding addresses possible.
- The uploaders of the community GameShark saves on GameFAQs, used for testing.
- **Jeffrey Pfau** and the mGBA contributors, whose emulator the test builds run on.

*Mega Man Battle Network* is a trademark of **Capcom**. Violet Eclipse is an unofficial fan patch, not affiliated with or endorsed by Capcom. It contains no Capcom data: you supply your own ROM. The patch's own licence is still to be chosen.

Full list with licences: `docs/credits.md`.
