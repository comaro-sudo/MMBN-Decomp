# RE-04: tournaments and brackets

Claude's reverse-engineering lane, 2026-11-22. **V** = read in the code (RS addresses; `code_03` is +8 in BM). **R** = checked against the community saves (`ref/saves/`, see [community-saves.md](community-saves.md)). **T** = tentative.

## 1. Entry point (V)

`BN4_START_TOURNAMENT` (`sub_08043FA0`) is called with `r0` = the tournament:
- It stores `r0` at `0x0200ACC0 + 5`.
- Tournament **3 is the Free Tournament** and goes to `sub_080448C0` (not read yet).
- Tournaments **0–2** (the three story tournaments) run five steps:
  - `sub_08043FE4`: the Soul Navi pair (§3);
  - `sub_08044058`: the other entrants (§4);
  - `sub_0804422C`, `sub_0804428C`, `sub_080442EC`: not read yet; CE says they fill the pool and draw the bracket.
- It then marks every entrant (`sub_08043EEC`) and, except for the Free Tournament, sets MegaMan's next opponent (`BN4_NEXT_OPPONENT`, `sub_08044744`).

## 2. State: the tournament pools at `0x020073A0` (V, R)

`0x020073A0 + t × 0xC`, three records (CE's `BN4_TOURNAMENT_POOLS`):

| Bytes | Contents |
|---|---|
| `+0`, `+1` | The tournament's **two Soul Navis**, in the order they'll be met (§3) |
| `+2`–`+6` | The other entrants, `0xFF`-terminated (§4) |
| `+7`–`+0xB` | A second list of shared Navis: `14 16 15 17` / `18 1A 19` / `1C 1B 1D` in every save (T: the rounds' other fights or the scenario Navis) |

New Game+ keeps the pools only when flag `0x18` is set (`sub_08001350`; see [completion-and-new-game-plus.md](completion-and-new-game-plus.md) §3).

## 3. The Soul Navi pair: `sub_08043FE4` (V, R)

- If the pool already has a pair (`+0` is not `0xFF`), it keeps it, swapping the two under a condition (`sub_0804480C`, not read yet).
- Otherwise it copies the pair from a per-tournament table (`0x08044044` RS / `0x0804404C` BM):

| Tournament | Red Sun | Blue Moon |
|---|---|---|
| 0 | `00 01` | `03 02` |
| 1 | `04 05` | `06 07` |
| 2 | `08 09` | `0A 0B` |

- **Randomness:** the order stays fixed until **flag `0x15`** is set; after that, `GetTertiaryRNG() & 1` picks it. Flag `0x15` is set in every save past the first tournament (R), and the leftover debug "give Souls" scripts set it (and `0x14`).
- The saves show the order differs between playthroughs: RS tournament 0 has `00 01` in playthroughs 1, 3 and 4 but `01 00` in playthrough 2.

**Ids are global across versions (R):** Red Sun's Soul Navis are 0, 1, 4, 5, 8 and 9; Blue Moon's are 2, 3, 6, 7, 10 and 11. Matching them to Souls is T: probably tournament 0 = Roll/GutsMan (RS) and ProtoMan/NumberMan (BM), tournament 1 = Wind/Search and Metal/Junk, tournament 2 = Fire/Thunder and Aqua/Wood. This needs a runtime check.

## 4. The other entrants: `sub_08044058` (V, R)

1. It starts from a per-tournament list (`0x080441EC` RS / `0x080441F4` BM): two shared Navis plus the *other* version's pair.

   | Tournament | Red Sun | Blue Moon |
   |---|---|---|
   | 0 | `0C 0D 02 03` | `0C 0D 00 01` |
   | 1 | `10 0F 06 07` | `10 0F 04 05` |
   | 2 | `11 0E 0A 0B` | `11 0E 09 08` |

2. A Soul Navi (id below `0xC`) is kept **only if it is in the traded-Navi list** at `0x02006F40` (`sub_08044618`: 7 two-byte entries, the id in the second byte). That list is empty (`FF`) in every save except the two with traded Soul data. There, it holds the other version's six ids, and the pools include them: RS save `22565`, tournament 0 = `00 01 02 0D 0C 03` (R).
3. It shuffles the list with `GetTertiaryRNG`: 0–3 swaps of a random entry with its neighbour.
4. It sorts entrants whose flag `id + 1` is set behind those whose flag is clear (flags `0x01`–`0x12`; meaning T: "met already").
5. Then it checks the result against a per-tournament required pair (`0x080441FC` RS / `0x08044204` BM: `02 03`, `06 07`, `0A 0B` in RS) (rest not read yet).

## 5. What this means for Violet Eclipse

- **WP-B1, deterministic brackets.** The randomness has exactly two sources:
  - the pair-order coin flip (§3), gated by flag `0x15`;
  - the entrant shuffle (§4.3).
  Both use `GetTertiaryRNG` (`sub_08001B60`). A patch can make the pair order follow the playthrough count (both Souls of a tournament over two runs), or let the player choose. And it can drop the shuffle.
- **WP-D4, the merge.** Tournament ids already cover all twelve Soul Navis, and Red Sun already enters Blue Moon's Navis when the traded list holds them. Red Sun must therefore contain enough of Blue Moon's Navis to fight them. Red Sun save `22565` has Blue Moon's Soul flags set and all 36 secret chips, which fits. CE's `bn4_souls.py` also reads all twelve Souls' form tables from Red Sun.
  - **If true, the merge is far smaller than planned:** fill the traded list (or rewrite the entrant tables), then open the Soul flag table (`souls.md`).
  - **Tested (R):** with the 12-entry Soul flag table and save `22565`, Red Sun unites WoodSoul ([soul-merge-experiment.md](soul-merge-experiment.md)).
- **The secret chips** come through the same path: beating a traded Navi in the Free Tournament (§1, `sub_080448C0`).

## 6. Next

1. Read `sub_080448C0` (Free Tournament), `sub_0804480C` (pair swap), `sub_0804422C`/`sub_0804428C`/`sub_080442EC` (bracket draw).
2. Find who writes `0x02006F40` (the link-trade receive code) and flags `0x14`/`0x15`.
3. The id → Navi/Soul table: confirm at runtime by loading a save at a tournament.
4. The ProtoSoul-in-Red-Sun experiment (§5).
