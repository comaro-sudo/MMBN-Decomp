# Completion stars, New Game+, and how Souls are tracked (RE-03, RE-05)

Claude's reverse-engineering lane, 2026-11-22. Status: **V** = read in the code, **R** = seen in the running game, **T** = tentative. RS addresses; BM is the same in `code_00` unless noted. The save layout and the `eToolkit` slots used below are in [`save-and-flags.md`](save-and-flags.md).

## 1. Completion stars: `sub_08000D28` (V)

`sub_08000D28` returns `r0` = number of conditions met and `r1` = a bit mask:

| Bit | Condition |
|---|---|
| `0x80` | Flag `0x500` set (game cleared; set by NG+ and by the clear save `sub_08000DC2`) |
| `0x40` | All Souls in the zero-terminated u16 list at `0x08000DB4` are set, and the count is exactly 6. RS list `17 18 19 1A 1B 1C`, BM list `1D 1E 1F 20 21 22` |
| `0x20` | Standard Library count (`sub_080373A8`) = 150 |
| `0x10` | Mega Library count (`sub_080373F0`) = 60 |
| `0x08` | Giga Library count (`sub_08037450`) = 5 |
| `0x04` | Program Advance count (`sub_080374F0`) = 30 |
| `0x02` | Flag `0x6F` set |

These are probably the title-screen stars (T). **Violet Eclipse:** the Soul star needs a 12-entry list and `cmp r5, #12` (the list sits inside the function's literal area, so it must be moved). The Library targets need the merged counts if the other version's Giga chips are added.

`sub_08000DC2` is the clear save. It loads the save, sets flags `0x500` and `0x6E`, stores its argument as the clear time (progress `+0x2C`), and saves.

The script command `checkDoubleSouls` (`EE 2B`, handler `sub_0804F494` RS / `sub_0804F4A0` BM) tests flags `0x17`–`0x1C` (RS) or `0x1D`–`0x22` (BM) with `TestEventFlagRangeAll`, count 6. It needs the same 12-Soul change.

## 2. How a Soul is tracked (V; R for the debug scripts)

A Soul counts as obtained when two things are true:

1. **Its event flag** (`0x17`–`0x22`, from `BN4_SOUL_FLAGS`) is set.
2. **Its pseudo-chip key is written.** Soul *n* (1–12) is chip `352 + n`, so 353–358 for RS and 359–364 for BM. `sub_080063B4` writes the key: `[eToolkit+0x84][id] = [0x02000670 + id] XOR 0x64`. CE's `BN4_CHIP_GATE` refuses a Soul pseudo-chip whose key wasn't written.

The evidence is two leftover debug scripts in RS archive `0x757858` (scripts 243 and 244, with Japanese debug text). Each one sets flags 20 and 21 (`0x14`, `0x15`), then one version's six Soul flags, then gives that version's six pseudo-chips with `itemGiveChip` (code 255, amount 0). Script 243 grants **Blue Moon's** six Souls inside Red Sun. That's a working template for the merged ROM, provided the Soul data behind it exists.

**Where the story grants a Soul: not found yet.** It isn't a literal `flagSet` in any TextPet-indexed script (all 794 RS archives searched). Nothing in code sets these flags with a constant ID, and no other code reads `BN4_SOUL_FLAGS`. It's probably computed from the Navi id after the tournament, or set by a script outside the TextPet index. The fastest way to find it is a runtime watch on the flag byte (block `+0x118 + 2`) across a Soul event. That needs a late-game state.

## 3. New Game+: `sub_08000DE8` (V)

This is the only caller of `IncPlaythrough` (`sub_08000C5A`). Its steps:

1. Zero a scratch area: `0x0202A000`, `0x16000` bytes.
2. Fourteen **stash** routines plus `sub_08001680` append what is kept to the scratch area (cursor `r7`, word-aligned between groups).
3. New-game init: `sub_080049A8` and `sub_08004928`.
4. Fourteen **restore** routines plus `sub_080016F0` copy it back in the same order.
5. `IncPlaythrough`, then `sub_0802F6F0`, `sub_08035130`, `sub_0802D4C4`, `sub_080ADE5C`, `sub_080A5270` (not read yet). Then set flag `0x500` and clear flag `0x6E`.

**Kept across New Game+** (from the stash routines; destinations are the same slots):

| Stash / restore | What is kept |
|---|---|
| `sub_08000EC0` / `sub_08000F0C` | Slot `+0x54` fields: `+0x80`, `+0xA0`, `+0xB0` (6 bytes each) and `+0xC0` (`0xBD` bytes). T: probably NaviCust or BBS state |
| `sub_08000FEE` / `sub_08001028` | Flags `0x16C0`–`0x1CBF`: **the Library** (`0x16C0 + id`) and the two other chip ranges |
| `sub_0800107C` / `sub_08001092` | Flags `0x1EC0`–`0x1EFF` |
| `sub_080010B0` / `sub_0800111A` | Navi stats (`+0x78`, `0x200`), slot `+0x7C` (`0x700`), RAM `0x02001610` (`0x160`), `0x02007230` (`0x160`), `0x020001E0` (`0x58`), `0x02006F50` and `0x02001BB0` (`0x2E0` each), `0x02000000` (`0x180`) |
| `sub_080011BC` / `sub_080011E4` | RAM `0x020008A0` (`0xBC0`), `0x02000520` (`0xB8`), `0x02001770` (`0x20`) |
| `sub_08001224` / `sub_08001288` | Slots `+0x5C`–`+0x70` (sizes `0x20`, `0xC8`, `0x10`, 1, 2, `0xE`), RAM `0x02000190` (`0x20`) |
| `sub_08001308` / `sub_08001324` | RAM `0x02006F40` and `0x020014F0` (`0xE` each; `0x020014F0` is the Free Tournament's entrant list) |
| `sub_0800138C` / `sub_0800139E` | Slot `+0x58` (`0xDC0`) |
| `sub_080013B4` / `sub_080013EC` | **Folders** (`+0x4C`), slot `+0x50` (`0xF00`; T: the pack), RAM `0x020073D0` (2), `0x02001E9C` (4) |
| `sub_0800143C` / `sub_08001460` | Flags `0xD00`–`0x10FF`; slot `+0x74` (`0x800`) |
| `sub_08001490` / `sub_080014A6` | **Flags `0x000`–`0x0FF`**, which include every Soul flag `0x17`–`0x22` and flags `0x14`/`0x15` |
| `sub_08001350` / `sub_0800136A` | Only when flag `0x18` is set (a Red Sun Soul flag): RAM `0x020073A0`, `0x30` bytes (CE: `BN4_TOURNAMENT_POOLS`). T: what this means |
| `sub_08000F68` / `sub_08000FB0` | Conditional on flags from `0x15D0`, `0x2D` and `0x2E` (not decoded yet) |
| `sub_080014C4` / `sub_0800158C` | Anti-cheat state, so the kept data still validates: key tables `+0x80` and `+0x84` with their sources `0x02000240` and `0x02000670`, and the Zenny and BugFrag shadows with their keys `0x02000180` and `0x02001E94` |
| `sub_08001680` / `sub_080016F0` | Game-state and progress fields (the scan in `save-and-flags.md` shows both touching game state `+0x02`–`+0x22` and `+0x74`/`+0x78`, and progress `+0x06`, `+0x14`–`+0x2C`). T: field by field |

**What this means for C4 ("nothing lost through New Game+").** Souls, the Library, folders, the pack (T), Navi stats, Zenny and BugFrags (T, via `sub_08001680`) all survive NG+. Story flags outside the kept ranges are reset, which includes the Mystery Data range `0x15C0`–`0x163F` (T, `save-and-flags.md` §2). So C4's risk is limited to one-shot pickups and events in the reset ranges. The fix for persistent Mystery Data (WP-B3) is to add a stash/restore pair for that range here.

## 4. Data misread as code (for Gemini)

`sub_0805A58C`, `sub_0805A674`, `sub_0805A6AE`, `sub_08061836`, `sub_08066C0A` and `sub_08068B1C` in `decomp/asm/code_04.s` decode to nonsense: `bkpt`, shifts by 32, and an `incrom` in the middle of a "function". They are almost certainly data. They still byte-match, so nothing is broken; it only affects the coverage numbers and naming.
