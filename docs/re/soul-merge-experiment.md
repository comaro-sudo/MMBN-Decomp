# Experiment: a Blue Moon Soul in Red Sun (2026-11-22)

**Result: confirmed.** With one 12-byte table changed, Red Sun lets MegaMan unite with **WoodSoul**, a Blue Moon Soul. He takes WoodSoul's form, the Soul's 3-turn counter starts, and the battle runs normally. With the original table, the same hand and save cannot unite. That's the control.

## Why it was expected

- **Unite code:** `BN4_UNITE_CHECK` (`0x0801FF84`) is identical in both versions and already handles 12 Souls. It refuses a Soul only if its entry in `BN4_SOUL_FLAGS` (`0x08020018`) is `0xFF`, its flag is clear, or it was used this battle ([save-and-flags.md](save-and-flags.md) §4).
- **Form tables:** `FORM_ENTERS`, `FORM_TURNS`, `FORM_ELEMENTS`, `FORM_COMPANIONS`, `FORM_LOADOUTS` and `FORM_FRAMES` are byte-identical in RS and BM, with real entries for all 12 forms. Form n = Soul n: elements 5 Fire, 6 Thunder, 11 Aqua, 12 Wood.
- **Sprites:** MegaMan's sprite list (list 0, `0x08027964` RS / `0x08027968` BM) points at the same 12 Soul sprites in both ROMs (`0x08214000`–`0x08287C94`). Those 0x73C94 bytes are identical.
- **The save:** Red Sun community save `22565` has Blue Moon's Soul flags and pseudo-chip keys set, in matching pairs ([community-saves.md](community-saves.md)).

## Method (reproducible, all local)

1. **ROM:** Red Sun US, with `BN4_SOUL_FLAGS` filled with `17`–`22`. This is now `patch/src/common/wp_d4_twelve_souls.asm`.
2. **Forced battle (test only):** `BN4_ENCOUNTER_CHECK` (`0x080F5370`) is replaced with `ldr r0, [pc]; bx lr` plus a battle record: group `0x84`, map 0, record 0, two Mettaur3. This is CE's `bn4_lab.py` method; the next encounter check starts that battle.
3. **Save:** `python patch/tools/savetool.py --convert ref/saves/*22565.sps`.
4. **Run:** the PC port's `--test` mode, chained with `--state-in`/`--state-out`.
   - load the save;
   - walk to the PC and jack in;
   - leave Lan's PC through the warp, so the encounter check runs;
   - the battle starts and the Custom screen opens (battle phase `0x08` at `0x02035811`).
5. **Unite:**
   - The hand holds Lance (a wood chip, element icon 3). In `BN4_SOUL_KINDS`, icon 3 belongs only to WoodSoul.
   - Picking Lance lights UNITE. The test is on the last chip picked.
   - UNITE shows the WoodSoul picture; then OK.
   - In battle (phase `0x0C`), MegaMan is in WoodSoul form with 3 turns.
6. **Control:** the same steps on a ROM with the original table leave UNITE greyed.

## Soul kinds = chip element icons (V, R)

`BN4_SOUL_KINDS` holds element-icon numbers (the chip record's `+0x07`). The pairing below matches each Soul's in-game rule:

| Soul | Icon | Chip type |
|---|---|---|
| Roll | 4 | Recovery |
| Guts | 8 | Panel destruction |
| Wind | 10 | Wind |
| Search | 7 | Invisibility |
| Fire | 0 | Fire |
| Thunder | 2 | Elec |
| Proto | 6 | Sword |
| Number | 5 | Bonus (T: the icon's name) |
| Metal | 11 | Break |
| Junk | 9 | Summon |
| Aqua | 1 | Aqua |
| Wood | 3 | Wood |

## What it means for Violet Eclipse

- **The merge (WP-D4, Option A) shrinks.** Both ROMs already carry all twelve Souls' battle code, tables and sprites. The Navi side points the same way: Red Sun already fields Blue Moon's Navis in tournaments when it has their traded data ([tournaments.md](tournaments.md)).
- **What remains is mostly *getting* the other version's Souls in one game:** the grant (flag plus pseudo-chip key), the Navis' tournament entries or a new event, and the counts (completion star, `checkDoubleSouls`).
- **The patch ships in both versions' builds:** `python patch/build.py`, 6 bytes per ROM.

## Not yet tested

- The mirror case: a Red Sun Soul in Blue Moon. Blue Moon save `4891` has Red Sun Soul flags but no keys in the layout Red Sun uses, so Blue Moon may store keys differently.
- Every Soul's full moveset, and the other five Blue Moon Souls. Same code path, so this is low risk.
- Fighting Blue Moon's Navis in Red Sun outside the Free Tournament path.
