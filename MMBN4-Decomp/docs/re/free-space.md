# Free ROM space (2026-11-22)

**V** = checked in both US ROMs. **T** = tentative, needs a runtime check.

Both ROMs are a full 8 MiB (`0x08000000`–`0x087FFFFF`). Any new code, table or text for Phase 1 needs room, so this note lists what is safe to use.

## 1. Safe now: the 0xFF tail, 35.9 KiB (V)

| Version | Range | Size |
|---|---|---|
| RS | `0x087F739C`–`0x08800000` | `0x8C64` (35,940 bytes) |
| BM | `0x087F739C`–`0x08800000` | the same: the BM data region is back at delta `+0x0` here (`decomp/symbols/datashift_rs_bm.txt`) |

- It's the cartridge's erase fill after the last asset (the title-screen and version-logo block ends before it).
- **Nothing points into it.** A scan of every aligned word in both ROMs found no pointer into the range, with or without bit 31. The values that look like pointers turned out to be:
  - `0x08800000` / `0x88800000`: the end of the ROM, used as an end marker 237 times;
  - `0x087F786F`: a coincidence inside Thumb code at RS `0x080263C4` and its copies.
- The same addresses work in both versions, so one patch file can use it for RS and BM.
- **Use `0x087F7400`–`0x08800000`** (aligned, a small margin after the last asset). Give each work package its own sub-range in `patch/src/common/defines.asm`.

## 2. Not free (V)

| Range (RS) | Why not |
|---|---|
| `0x08211B2C`–`0x08212700` (3 KiB, 0xFF) | It ends exactly at `0x08212700`, which `crt0` loads (literal at `0x080001F0`): the IWRAM payload's copy bounds. Several pointers point inside it. |
| The 0x00 runs (34 of 256 B or more, 32.6 KiB in all; the largest is `0x0868FA14`, 19.8 KiB) | They're inside graphics, tilemaps and palettes, where zeros are real pixels or empty tiles. |

## 3. Candidates that need a runtime check (T)

`code_09` holds 0xFF runs between tables, with no pointer into them: RS `0x080F8FC4`–`0x080F9240` (636 B), `0x080F92C6`–`0x080F9540` (634 B) and `0x080F95C8`–`0x080F9780` (440 B), BM +0xC. They may be unused table slots that are still read by index. Don't use them unless an mGBA watch over a playthrough shows no reads.

## 4. More room: expand the ROM to 16 MiB (T, recommended for Violet Eclipse)

The GBA maps cartridge ROM up to 32 MiB (`0x08000000`–`0x09FFFFFF`). Padding the image to 16 MiB gives **8 MiB of free space at `0x08800000`**.

- The patch pipeline already handles it: armips can write past the end, and BPS stores the new size.
- **Risk to check first:** the ROM uses `0x08800000` (and `0x88800000`) as an end marker. If any loader compares a pointer against that marker, data placed exactly at `0x08800000` would be read as "none". Start expanded data at `0x08800100` or later, and find the marker's readers before relying on the range.
- Targets: mGBA, flashcarts and the PC port take 16 MiB images. Wii U VC injection and GBARunner3 should be checked (plan/04).

The 35.9 KiB tail is enough for the Tier A patches. The definitive edition's merged content (both versions' Souls and Navis, new text) will likely need the expansion.
