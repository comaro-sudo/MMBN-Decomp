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
- All 12 Souls can be earned in either version. Each Navi you'd normally meet only in the other version now gives its Soul in a **friendly match** at the tournaments (3 per tournament, before the final). You can retry if you lose.
- A new 13th Soul, the **Duo Soul**, unlocks when you beat Duo. It has its own colours, a 180-damage fist charge shot, AirShoes + FloatShoes, and a Reflect shield.
- The status screen shows every Soul (R / L switches pages), and the Souls completion star needs all 12.

**Every chip**
- Drops that differ between versions now come from either version's table, 50/50.
- Bass gives both versions' Bass chips.
- The Free Tournament fields all 12 Soul Navis without link trades, so all 36 secret chips can be won.
- **Duo** and **Grand Prix Power**, once event-only chips, are now in the game with names, descriptions and pictures. The pictures are missing in public builds, see "Known issues".
- **Z-Saver** and **Grand Prix Power** each wait in a Blue Mystery Data in an out-of-the-way corner.

**Every boss, as often as you like**
- **Rematches in Higsby's shop:** the operators of all 12 Soul Navis wait in and outside Higsby's. JunkMan has no operator: jack into the trash can on the ACDC street.
  - Each Navi climbs V1 → V2 → V3 → SP → **SP+** as you beat it with an S rank.
- **Duo rematches:** after the ending, find Duo's crest in Meteor Comp 1. It climbs Duo → α → β → Ω → SP+ with S ranks.
- **SP+**, the new top difficulty: 4,095 HP, more damage, faster. Winning pays 15,000 Zenny (80%) or 50 BugFrags (20%).

**Quality of life**
- A **Frequency Tuner** in Higsby's sets the virus tier (Auto / V1 / V2 / V3). Green Mystery Data follow the same setting, so every reward is reachable on any playthrough.
- Battle Zenny ×1.5.
- HP Memory, NaviCust programs, Unlocker and LocEnemy cost 40% of their old price.
- The WoodMan scenario fix (by Prof. 9) is included.
- The Waiting Room rejects a corrupt link transfer instead of writing it into your save.

## Where to find the new content

| What | Where |
|---|---|
| Rematch operators (9) and the Frequency Tuner | Inside Higsby's shop, ACDC Town |
| 2 more operators and the trash can (JunkMan) | The ACDC Town street outside Higsby's |
| Friendly matches | DenDome Hall, the Castillo waiting room, the Colosseum lobby (during each tournament, before the final) |
| Duo's crest (rematches) | Meteor Comp 1, after the ending |
| Z-Saver | A Blue Mystery Data in a hidden dead end of Undernet 1 |
| Grand Prix Power | A Blue Mystery Data on a quiet round pad in ACDC Area 2 |

## Saves

Existing saves from the same version work. Shop prices update when you load one. Make a backup of your save first.

## Known issues

- **Public builds:** the Duo and Grand Prix Power chips have names and descriptions but no picture.

## Credits

*(To complete before release: Prof. 9's WoodMan fix; armips, TextPet, Flips; the community chip tables, symbol sources and test saves.)*
