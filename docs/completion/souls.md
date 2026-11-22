# Double Souls (criterion C1)

The game-side details are in [`../re/save-and-flags.md`](../re/save-and-flags.md) §4 and [`../re/completion-and-new-game-plus.md`](../re/completion-and-new-game-plus.md) §1–2.

| Soul n | Soul | Navi | Version | Event flag | Pseudo-chip |
|---|---|---|---|---|---|
| 1 | RollSoul | Roll | Red Sun | `0x17` | `0x161` |
| 2 | GutsSoul | GutsMan | Red Sun | `0x18` | `0x162` |
| 3 | WindSoul | WindMan | Red Sun | `0x19` | `0x163` |
| 4 | SrchSoul | SearchMan | Red Sun | `0x1A` | `0x164` |
| 5 | FireSoul | FireMan | Red Sun | `0x1B` | `0x165` |
| 6 | ThunSoul | ThunderMan | Red Sun | `0x1C` | `0x166` |
| 7 | ProtSoul | ProtoMan | Blue Moon | `0x1D` | `0x167` |
| 8 | NumSoul | NumberMan | Blue Moon | `0x1E` | `0x168` |
| 9 | MetlSoul | MetalMan | Blue Moon | `0x1F` | `0x169` |
| 10 | JunkSoul | JunkMan | Blue Moon | `0x20` | `0x16A` |
| 11 | AquaSoul | AquaMan | Blue Moon | `0x21` | `0x16B` |
| 12 | WoodSoul | WoodMan | Blue Moon | `0x22` | `0x16C` |

The order comes from the chip table's names (`0x161`–`0x16C`) and CE's Soul tools; the flags from `BN4_SOUL_FLAGS` (`0x08020018`). Other Soul-like records also exist: `0x16D` DuoSoul, MegaSoul, FrteSoul and DarkSoul are forms, not collectible Souls.

## What one save needs

1. **Both versions' Navis to fight.** A Soul comes from the Navi you beat in a tournament, so the merged ROM must contain the other version's six Navis (AI, graphics, records). That's WP-D4, Option A.
2. **All twelve entries in the Soul flag table.** Write `1D`–`22` into Red Sun's `0xFF` entries, so the unite code (already 12-way) accepts them.
3. **The grant:** set the flag *and* write the pseudo-chip's key (`WriteChipKey`, `0x080063B4`). The leftover debug script 243 in Red Sun does exactly this for Blue Moon's six.
4. **The checks that count Souls must count twelve:** the completion star (`sub_08000D28`, list at `0x08000DB4`) and the `checkDoubleSouls` script command (`sub_0804F494`).
5. **No tournament luck:** in the original game, which Souls you get depends on who you face. WP-B1 (deterministic brackets) is still blocked on RE-04.
6. **Soul flags survive New Game+** (flags `0x000`–`0x0FF` are kept), so nothing to do there.

## Confirmed: a Blue Moon Soul works in Red Sun (2026-11-22)

With the 12-entry flag table (`patch/src/common/wp_d4_twelve_souls.asm`, now in both builds), Red Sun unites WoodSoul and fights in it. Both ROMs carry all twelve Souls' code, tables and sprites ([`../re/soul-merge-experiment.md`](../re/soul-merge-experiment.md)). Step 2 above is done, and step 1 shrinks to *getting* the other version's Souls: the grant, and the Navis' entries.

## Background: Red Sun may already hold Blue Moon's Navis (T)

Red Sun enters Blue Moon's Navis in tournaments when its traded-Navi list holds them, and a community Red Sun save has Blue Moon's Soul flags and all 36 secret chips ([`../re/tournaments.md`](../re/tournaments.md) §5, [`../re/community-saves.md`](../re/community-saves.md) §3). If the Soul forms are present too (CE's tool reads all twelve from Red Sun), step 1 above shrinks to filling that list or the entrant tables. The test: patch the Soul flag table, load save `22565`, and unite with ProtoSoul.

## Open

- Where the story sets a Soul's flag is still unknown. A runtime watch on the flag byte from a late-game save will find it.
