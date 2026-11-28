# Library listing for Duo and Grand Prix Power: pre-checks (research, 2026-11-27)

Task 3 of `docs/tasks-claude-agent-2026-11-27d.md`; follows `docs/pending/findings-2026-11-27.md` §1. Read-only research: `decomp/asm` plus the two US ROMs. **Nothing was built or changed.** The owner decides whether the chips should appear.

## Short answer

- **The proposed fix (byte `+9` bit `0x40` plus byte `+0x16` bits `0x04 | 0x08`) is not safe as written: it keeps the stars, but the chips still never appear.** The Mega and Giga *listing* loops do filter on `0x04`/`0x08`: they skip a chip that has the bit for the running version. With both bits set, Duo and GP Power are skipped in both versions.
- Setting only `+9` bit `0x40` (leaving `+0x16` as it is, `0x10`) is worse: the counts become 61 and 6 and the stars are lost.
- **Even with the filters passed, nothing would be listed.** The Mega and Giga loops place a chip by looking its Library number (`+0x1c`) up in a 300-entry slot table. Duo (292) and GP Power (291) have slot `0` in those tables, so the loop writes to slot `-1`, 4 bytes *before* the buffer. See Check 3.
- The other readers of `+0x16` don't care about bits `0x04`/`0x08` for these two chips, except one hard-coded exclusion (below).

## Facts about the two chips (both US ROMs, record table `0x080197EC`, stride `0x2C`, same address in RS and BM)

`BN4_GET_CHIP` = `sub_080190D0` (`code_01.s:14953`, same address in BM): `0x080197EC + id * 0x2C`.

| id | name | class (+8) | +9 | +0x16 | Library no. (+0x1c, u16) |
|---|---|---|---|---|---|
| `0x137` | Duo | 2 (Giga) | `0x07` (bit `0x40` clear) | `0x10` | 292 |
| `0x138` | GP Power | 1 (Mega) | `0x07` (bit `0x40` clear) | `0x10` | 291 |

The two records are byte-identical in RS and BM. `+0x16` is `0x10` for both: bits `0x01`, `0x04`, `0x08` clear. For comparison, the other Giga chips have `0x14` (RS-only, bit `0x04`) or `0x18` (BM-only, bit `0x08`).

## Which version bit is which

`VERSION_INDEX` is 0 in RS and 1 in BM. In every filter the chip is skipped when:

- RS (`VERSION_INDEX == 0`): `+0x16 & 0x08`;
- BM (`VERSION_INDEX != 0`): `+0x16 & 0x04`.

So bit `0x04` marks "Red Sun only" chips and bit `0x08` "Blue Moon only" chips. Setting both hides the chip in both versions. Check on the ROM data: 5 Giga chips have `0x04`, 5 have `0x08`; simulating the counts gives Standard 150, Mega 60, Giga 5 in both versions.

## Check 1: do the listing loops filter on `0x04`/`0x08`?

All in `code_03.s`. BM addresses are RS `+8` (the code region `0x08035088`–`0x0804F21C` is `+8`; neighbours confirmed in `symbols/map_rs_bm.txt`; function bytes match apart from literal-pool offsets and the version immediates).

| Loop | RS addr | BM addr | asm lines | Filters |
|---|---|---|---|---|
| Standard `sub_08037798` | `0x08037798` | `0x080377A0` | 4637–4690 | `+9 & 0x40`, skip if `+0x16 & 0x01`, class == 0. **No `0x04`/`0x08` filter.** Loops ids `1..0xBA` only, so it never reaches `0x137`/`0x138` anyway. |
| Mega `sub_08037824` | `0x08037824` | `0x0803782C` | 4712–4787 | `+9 & 0x40`, skip if `0x01`, **then skip on `0x08` (RS) / `0x04` (BM), lines 4730–4744**, class == 1 |
| Giga `sub_080378D8` | `0x080378D8` | `0x080378E0` | 4803–4897 | same as Mega (lines 4822–4836), class == 2 |
| PA `sub_08037990` | `0x08037990` | `0x08037998` | 4900–4960 | `+9 & 0x40`, `+0x16 & 0x01` **must be set**; no version bits |

Mega and Giga loops run ids `1..0x13F` (`_08037B3C` = `0x13F`), so they do reach `0x137`/`0x138`. The statement in findings §1 that the Mega and Giga listing loops don't filter on the version bits is **wrong**; only the Standard loop (the example it quoted) has no version filter.

Slot bookkeeping: entry address = `buffer + (slot - 1) * 4`, with `slot` from a halfword table indexed by the chip's `+0x1c`:

| Loop | Buffer | Table (RS / BM) |
|---|---|---|
| Standard | `0x0201A000` | none: slot = `+0x1c` |
| Mega | `0x0201A800` | `dat_08038B54` / `0x08038B5C` |
| Giga | `0x0201B000` | `dat_08038DAC` / `0x08038DB4` |
| PA | `0x0201B800` | `dat_08039004` / `0x0803900C` |

## Check 2: every other reader of chip byte `+0x16`

Method: every `ldrb rX, [rB, #0x16]` in `decomp/asm/code_*.s` (1270 hits) classified by whether `rB` is a chip record (the result of `BN4_GET_CHIP`). The rest are `r5`/`r7`/`sl`-based object, side and AI structs (`[r5, #0x16]` is the battle side), not chip records. No chip record is read at `+0x16` by any other form (no `adds #0x16` pointer arithmetic, no halfword or word reads of `+0x14`/`+0x16` after `BN4_GET_CHIP`). **Limit:** a static scan; a chip pointer handed through several calls to a function with no nearby `BN4_GET_CHIP` could be missed, but I found none.

| Function (RS addr; BM addr) | asm line | Bit | What it does | Duo / GP Power |
|---|---|---|---|---|
| `GetLibraryCountStandard` `sub_080373A8`; BM `0x080373B0` | `code_03.s:4147` | `0x01` | skip chip | not reached (class 0 only) |
| `GetLibraryCountMega` `sub_080373F0`; BM `0x080373F8` | 4188, 4195, 4201 | `0x01`, then `0x04`(BM)/`0x08`(RS) | skip | counted today only if `+9 & 0x40`: the count side of the fix |
| `GetLibraryCountGiga` `sub_08037450`; BM `0x08037458` | 4243, 4250, 4256 | same | skip | same |
| `sub_080374B0`; BM `0x080374B8` (into `[r5,#0x50]`) | 4298 | `0x01` | counts chips where it **is** set | no (bit clear) |
| Library listings | 4652 / 4730 / 4822 / 4916 | see Check 1 | see Check 1 | see Check 1 |
| `sub_08039AF0`; BM `0x08039AF8` (by the `+8` region; not in `map_rs_bm.txt`) | 8057 | `0x04`(BM)/`0x08`(RS) | scans ids `0..0x1FF` for the first chip whose flag `0x18C0+id` is set and whose `0x16C0+id` and `0x1AC0+id` are clear, skipping the other version's chips; reached through a pointer table, no direct `bl`. **Ids `0x137` and `0x138` are excluded by number** (literals `_08039BE8`, `_08039BEC`, lines 8105–8112) | already excluded; bits irrelevant |
| `sub_08030754`; BM about `0x08030758` (`+4` region) | `code_02.s:28777` | `0x01`; then `0x04`(BM)/`0x08`(RS) | loop from id `0x138` down to `1`: for each chip with `+9 & 0x40` and (bit `0x01` set, or no version bit for this version) calls `ChipGive(id, code, 0x1e)` for each of the 4 codes. Callers: `sub_0802E0F8` (`code_02.s:24332`) and the script handler `sub_0804F88C` (`code_04.s:947`) | **Skipped today (`0x40` clear).** With `0x40` set and the version bits left clear, it would **give 30 copies of each code** of both chips; with the proposed `0x0C` bits it skips them again |
| `sub_0803D6F8`; BM `0x0803D700` | `code_03.s:15695` | `0x10` | skips the chip when set. Builds a 0x20-byte-per-entry chip list at `0x0201C000` for ids `1..0xBA`, `0xC9..0x118`, `0x12D..0x138`; called from `sub_0803C65C` (`code_03.s:13568`) and `sub_0804BF48` (`code_03.s:43022`) (screens not identified) | already excluded (`0x10` set); keep it set |
| `sub_08022AFC`; BM `0x08022B00` | `code_02.s:3336` | `0x20` | battle chip-use check (called from `code_02.s:3164`, `3246`) | not a version or Library bit |
| `sub_0800C8C0`; same in BM | `code_00.s:25132` | `0x02` | battle: chip attack-end path | not a version or Library bit |

**Bits `0x04`/`0x08` on a chip record are read only in:** the three Library counts, the Mega and Giga Library listings, `sub_08030754` (chip-giving loop) and `sub_08039AF0` (the "first missing chip" scan). I found **no** read in drops, shops, traders, NetBattle code or the Custom screen. (The `+0x16` readers outside the Library test `0x10`, `0x20` and `0x02`.)

Readers of `+9` bit `0x40` (the "listed" bit): the four count functions, the four listing loops and `sub_08030754` (`code_02.s:28773`). No others.

## Check 3: where the two chips would appear

- **Tab:** by class. GP Power (class 1) in Mega, Duo (class 2) in Giga. The Standard tab can't show either (its loop ends at `0xBA`, and the class filter).
- **Library number:** the records carry 291 (GP Power) and 292 (Duo), but the Mega and Giga loops don't use that number directly; they map it through the slot tables. In both RS and BM, **`table[291]` and `table[292]` are `0` in the Mega, Giga and PA tables**, so the slot is `0 - 1 = -1` and the entry address is `buffer - 4`:
  - Mega: `0x0201A7FC`; Giga: `0x0201AFFC`. Those lie just before the real buffers; what else uses them is not verified.
  - So they would not show in the list; they would write a stray entry before the buffer.
- **Existing slots (from the tables):** Mega slots 1–60 use Library numbers 201–278 in both versions. Giga slots 1–5 use numbers 281–285 in RS and 286–290 in BM. PA slots 1–56. All contiguous; nothing reserved for 291/292.
- **For them to show,** a patch would need: (1) new slot entries (Mega slot 61, Giga slot 6) at index 291 and 292 in the tables of both versions; (2) the listing filters passed (version bits left clear for them, or the filters patched); (3) the star counts kept at 60 / 5, which means changing the count functions instead of using the version bits, since the counts and the listings share those bits. **Not verified:** how the Library screen derives its page count and cursor limits (`sub_08037CBC`, `sub_08037EE4`), and whether it assumes exactly 60 and 5 slots.

## Safety verdict on the proposed fix

- Stars: **preserved** (the counts skip chips with either version bit; simulated on both ROMs).
- Library: **no visible effect** (still hidden by the Mega and Giga listing filters), and it can't work without slot-table entries.
- Setting `+9` bit `0x40` without the version bits would make `sub_08030754` give 30 copies of each code of both chips, so the `0x0C` bits matter there.
- Setting `0x04 | 0x08` together is a combination no existing chip uses.

## Options for the owner (not decided)

1. Leave Duo and GP Power out of the Library (current behaviour, stars exact).
2. Show them: slot-table entries in both versions, a filter or count change, and an in-game test of the Library (page count, cursor, stars). Not a data-only change.

## How this was checked

Records and tables were read from `roms/bn4_rs_us.gba` and `bn4_bm_us.gba` (read-only): record `0x080197EC + id*0x2C`; slot tables at RS file offsets `0x38B54`, `0x38DAC`, `0x39004` (BM `+8`). No ROM bytes are stored in this file.
