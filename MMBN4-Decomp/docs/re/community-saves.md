# What the community saves show

The owner added 15 GameShark SP (SharkPort) saves from GameFAQs (2004–2012). They're kept local in `ref/saves/` (see `ref/README.md`). Two are byte-identical copies, which leaves 13 unique: 5 Blue Moon and 8 Red Sun, from a first playthrough to a fifth. Read them with:

```powershell
python patch/tools/savetool.py [--convert] ref/saves/*.sps    # summary; --convert writes build/saves/*.sav
```

## 1. Format confirmations (R)

- **All 13 decode with valid checksums** using [save-and-flags.md](save-and-flags.md) §1.2: XOR key at `0x1554`, block shift at `0x1550`, magic, byte sum + `0x16` (RS) or `0x22` (BM). The version from the checksum matches the cartridge code in each SharkPort header (`B4WE` / `B4BE`) every time.
- SharkPort layout:
  1. a length-prefixed `"SharkPortSave"`;
  2. a u32 platform;
  3. three length-prefixed strings (title, date, notes);
  4. a u32 payload size (`0x1001C`) and a payload of a `0x1C`-byte header (game title and code at `+0x0C`) then 64 KB of SRAM;
  5. a 4-byte trailer.

  The game uses the first `0x73D2` bytes and the second block at `0x7F00`.

## 2. Fields identified from the saves (R)

| Field | Where | Evidence |
|---|---|---|
| Current HP | game state `+0x20` | Equals max HP in normal saves; 9999 against a 180 max in one cheated save |
| Max HP | progress `+0x14` | Equals game state `+0x22` |
| Max HP without NaviCust HP+ parts | progress `+0x16` | E.g. 1250 against 1000 (an HP+ program fitted) |
| Game time | progress `+0x2C` | Capped at 99:59:59 in one save (`0x014988F0` frames, as the code says) |
| Library flags | `0x16C0 + id` | Counts of 150 standard, 36 secret, 60+ Mega, 5–10 Giga and 30 PA match the uploaders' descriptions ("100% complete library", "all 10 giga chips") |
| Clear flag | `0x500` | Set exactly in the saves past the first playthrough |
| The tournament pools | `0x020073A0` | See [tournaments.md](tournaments.md) §2 |

## 3. The other version's Soul flags have a meaning (R)

Two saves have the other version's Soul flags set:
- Blue Moon `4891`: Roll, Search and Fire (`0x17`, `0x1A`, `0x1B`), with exactly **9 secret chips**, which is 3 Navis × 3 levels.
- Red Sun `22565`: Proto, Number, Metal, Junk and Wood, with all 36 secret chips.

Both also have the traded-Navi list at `0x02006F40` filled, and no other save does. So in the version that can't unite a Soul, its flag still records that the Navi's Soul data was received (or that Navi beaten in the Free Tournament), and that unlocks the Navi's secret chips. Red Sun's unite code refuses those Souls only through the `0xFF` entries of `BN4_SOUL_FLAGS`.

## 4. Useful test saves

For runtime RE in the PC port: `savetool.py --convert`, then `mmbn4.exe rom --test N --save build/saves/<name>.sav`.

| Need | Save |
|---|---|
| Red Sun, early (1 Soul, before tournament 2) | `4429` |
| Red Sun, end of playthrough 1 (3 Souls) | `4440` |
| Red Sun / Blue Moon, late game, playthrough 3–4 | `4752`, `4760` / `4569`, `4579` |
| Red Sun with Blue Moon Navi data (merge experiments) | `22565` |
| Blue Moon with Red Sun Navi data | `4891` |
| Blue Moon, first playthrough, early (cheated HP) | `4420` |
