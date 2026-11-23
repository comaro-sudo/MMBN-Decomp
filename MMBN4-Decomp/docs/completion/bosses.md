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
  - **Implemented (2026-11-23):** `patch/src/common/wp_d2_random_version_drops.asm`.
    - `RewardGetRowPtr`, which every reward path uses, returns the row from this ROM's table or, on one RNG bit, from the other version's table.
    - `build.py` copies the other version's table from the player's other ROM at build time (11 KiB at `0x087F7420`; the BPS stays about 1 KB). So **building now needs both ROMs.**
    - Checked by disassembly in both versions. (T: not yet tested in-game.)
  - Implementation idea: keep both versions' rows. After `BN4_REWARD_PICK` (`0x080F580C`) picks its slot, choose the RS or BM row with one RNG bit. The table costs about 64 × 40 bytes (2.5 KiB) in free space (`docs/re/free-space.md`).
- **Bass is repeatable.** Bass, BassΩ and BassXX must be fightable as many times as the player wants, so that the random rule eventually gives all four chips: Bass, BassAnly, BlakBarr and DeltaRay.
  - Bass's every drop column gives his chip, so any Busting Level works. BlakBarr/DeltaRay need columns 3–4 (Busting Level 7 or higher, RE-06 §3).
  - Today Bass is a one-time story battle (`BN4_STORY_BATTLES` id 176). BassΩ roams Black Earth 2 behind flag `0x9F`. BassXX is in a story-override table.
  - The rematch (WP-D3) must re-arm all three. The simplest version is a repeatable script that starts battle 176 and the BassΩ/BassXX records again. (T: whether BassΩ already respawns after a win needs an in-game check.)
  - **Priority (owner, 2026-11-23). This is the final order; it replaces the orders in the notes below.** Use the first one that can be implemented:

    | # | What the player gets |
    |---|---|
    | Main | The first fight **drops both chips** |
    | Fallback 1 | The fight drops one chip **+ an Unlocker** SubChip; a **purple** Mystery Data nearby holds the other chip |
    | Fallback 2 | After the first fight, a **blue** Mystery Data (Unlocker) and a **purple** Mystery Data (the other chip) appear nearby |
    | Fallback 3 | The Bass form **roams the same area** (Black Earth 2) and can be fought again |

    This applies to Bass (Bass/BassAnly), BassΩ and BassXX (BlakBarr/DeltaRay).

    **Main option implemented (2026-11-23):** `patch/src/common/wp_d3_bass_both_chips.asm`, based on Gemini's `decomp/notes/bass-rematch.md`.
    - The results screen's chip grant (`bl CHIP_GIVE` at RS `0x080216E2` / BM `0x080216E6`) now calls a helper. The helper does the original grant, then grants the other version's pair: Bass ↔ BassAnly X, BlakBarr A ↔ DeltaRay Z.
    - The helper sits in a dead 86-byte block of `code_03` (RS `0x08037C58` / BM `0x08037C60`), which nothing branches to or points at.
    - Builds in both versions. The emitted code and the table were checked by disassembly.
    - (T: not yet tested in-game. The results screen still names only the battle's own chip, and the pair is added silently.)
    - Also from Gemini's note: **BassΩ and BassXX already roam Black Earth 2 forever** after Bass V1 sets flag `0x9F` (BassXX appears instead of BassΩ while a Mod Card is loaded). Only Bass V1 is a one-time fight. With this patch, one Bass V1 win plus one BassΩ/BassXX win at Busting Level 7+ gives all four chips. The notes below give implementation details. Where they set a different order, the table wins.
  - **Earlier notes (2026-11-23):**
    - **Bass (V1):** if he doesn't appear again after the story battle, add him to Black Earth 2's roaming table, the same area as BassΩ (a new entry in the `+0x10` ghost table, or a second `+0x10`-style table behind its own flag). If that isn't possible, his story battle **drops both chips**, Bass and BassAnly.
    - **BassΩ:** if the in-game check shows he doesn't come back after a win, he **drops both chips**, BlakBarr and DeltaRay.
    - "Drops both" needs a small patch: after the normal reward, grant the other version's chip too (through the same chip-add path the reward screen uses; to be found from `BN4_REWARD_PICK` / `RewardDecode`, RE-06 §2).
    - **BassXX:** the same as BassΩ. If he can't be fought again, he **drops both chips**, BlakBarr and DeltaRay.
    - **Last fallback, if "drops both" is too hard or impossible:** after the battle, a **Purple Mystery Data appears nearby** in Black Earth 2 and holds the chip the battle didn't give. (The owner chose purple over blue on 2026-11-23, in keeping with purple holding rare items.)
      - **First choice: the Bass battle gives the Unlocker** as a reward, together with its normal drop.
      - **Fallback, if the battle can't give it** (owner, 2026-11-23): a **Blue Mystery Data holding an Unlocker** is pinned at each place the player fights a Bass form, next to that form's purple one. It's possible: Blue Mystery Data can hold an Unlocker; CardComp, ColoNBMComp and VendMchComp already have one (RE-07 §3).
      - **Every win against a Bass form re-arms its Mystery Data**: the purple one (the missing chip), plus the blue one when the fallback is used.
        - On each win, clear their "taken" flags (in `0x0D00`–`0x10FF`), so both reappear even if they were opened before.
        - All of the blue one's content slots are Unlocker, so New Game+ advancing it changes nothing.
        - The purple one's content hook gives the chip of the pair the player still lacks (Library flags). (T: if the player already has both, give one of the two at random.)
      - If giving an Unlocker from the battle isn't possible, **make sure the game supplies enough Unlockers** for every purple Mystery Data: the 14 originals (RE-07) plus up to 3 new Bass ones.
        - **Already the case (V):** the six SubChips are items `0xB0`–`0xB5` (176–181; save slot `+0x54`, Gemini's `save-slots.md`). The six SubChip shops (stock records 6–11, `docs/re/shops.md`) sell all six with unlimited quantity (`0xFF`), so the Unlocker supply is unlimited.
        - Carry limit: each SubChip is capped at the SubMemory count (item `0xA5`, starting at 4).
        - Still to confirm: which of 176–181 is the Unlocker (item name table, Gemini's round-2 task 1), and that a SubChip shop is reachable in the post-game.
      - How it could work (from RE-07, `decomp/notes/re07-mystery-data.md`): add a placement to Black Earth 2's Mystery Data list. Use an unused flag in `0x0D00`–`0x10FF`; 123 of the 1,024 slots are used. Make it visible only after the battle's win flag.
      - Its content must be "the chip of the pair you don't have yet". It checks the Library flags (`0x16C0 + id`): if you lack BassAnly, it gives BassAnly; if you lack Bass, it gives Bass. Same for BlakBarr and DeltaRay. That needs a small content hook, since normal Blue Mystery Data hold one fixed item.
      - If even that hook is too much, use one Mystery Data per missing chip, each visible only while its chip isn't in the Library.
- (Considered and not chosen: the code follows the chosen version; extra drop slots at high Busting Levels.)

## Duo rematches (owner decision, 2026-11-23)

- **The player can fight every version of Duo, climbing like the lobby:** each S-rank win against Duo makes the next rematch the next version: **Duo (V1) → Duoα → Duoβ → DuoΩ**, then the SP+ step from `docs/pending/boss-double-hp.md`.
  - The same model as the operator rematches: a 2-bit tier counter in free flags (next free after the lobby's `0x0E40`–`0x0E59`, e.g. `0x0E5A`–`0x0E5B`), raised only on an S rank.
  - The rematch battle uses the "no automatic tier raise" bit.
  - The four Duo records are enemy ids `0xE0`–`0xE3`, with story battles 93 and 94 (RE-06).
- **Where (owner, 2026-11-23): back in the Meteor Comp**, where Duo is fought in the story, after the ending (post-game).
  - To research: the exact Meteor Comp map, which flag marks the ending/post-game (Gemini found Duo-defeated flag `0x006F`), and a free spot for a Duo NPC or object there that starts the rematch at the current tier.
  - *(Not chosen: a terminal in Higsby's lobby; a lobby "challenge" menu.)*
  - **The rematch replays the final battle** (owner, 2026-11-23): story battle records 93/94 with the current Duo version, started as a **plain battle** (as the lobby rematches do), so the ending and credits don't replay and the results screen is reached.
- **Side effect:** rematches go through a normal battle start, so they **reach the results screen** (the story's final battle doesn't). The existing `wp_d2_duo_chip_reward.asm` (Duoβ/DuoΩ drop the Duo chip) therefore works in rematches, so the Duo chip comes from beating Duoβ or DuoΩ in a rematch.
- **Built (2026-11-23):** the post-Duo grant for players who beat Duo only in the story: `patch/src/common/wp_d2_duo_story_grant.asm` hooks Gemini's `sub_080C4780` and gives Duo D once, when flag `0x006F` is first set. In-game test pending.

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
